//
// Listener.cpp
//

#include "stdafx.h"
#include "Listener.h"
#include "Room.h"
#include "RoomTable.h"

#include <NF/ADL/MsgCHSNGS.h>

const long lMajorVersion = 7;
const long lMinorVersion = 0;

#include "LrbConnector.h"

#include <md5.h>
#define MD5_TIME_GAP_DOWN		(60*60*8)			///	8  hour
#define MD5_TIME_GAP_UP			(60*5)				/// 5  min
#define MD5_KEY					"gOGo !!pmANg"

#include "ErrorLog.h"
#include <NLSManager.h>
///////////////////////////////////////////////////////////////////////////////////
CListener theListener;

void __stdcall _ListenerCallbackFunc(DWORD dwHandle, SOCKET hSocket, int nErrorCode, LPCSTR lpRemoteAddr, LONG lRemotePort, LPVOID lpContext)
{
	theListener.OnListenerAccept(hSocket, nErrorCode, lpRemoteAddr, lRemotePort);
}

///////////////////////////////////////////////////////////////////////////////////
// CListener
CListener::CListener() : m_dwListenerHandle(0)

{

}
 
CListener::~CListener()
{
}

BOOL CListener::Run(int nPort)
{
	TLock lo(this);

	BOOL bRet = TRUE;

	bRet = bRet && ::XlstnCreate(&m_dwListenerHandle, &_ListenerCallbackFunc, nPort, NULL, NULL);
	VALIDATE(bRet);

	if (!bRet)
		return FALSE;

#ifdef _DEBUG
	bRet = bRet && m_timerAlive.Activate(GetThreadPool(), this, ALIVE_INTERVAL, ALIVE_INTERVAL);
#endif

#ifdef _TESTTOOL_DEBUG_
	m_timerPrintRoomCnt.Activate( GetThreadPool(), this, 0, 60000 );
#endif
	if(!bRet)
	{
		Stop();
		return FALSE;
	}

	if (theRoomTable.IsRoomTitleMonitoringOn())
	{
		bRet = bRet && m_timerRoomTitleMonitor.Activate(GetThreadPool(), this, 300000, 300000);
		if (!bRet)
		{
			theLog.Put(ERR_UK, "Listener Running Failure: fail activating Room Title Monitor Timer");
			Stop();
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CListener::Stop()
{
	TLock lo(this);

	if(m_dwListenerHandle)
	{
		::XlstnDestroy(m_dwListenerHandle);
		m_dwListenerHandle = 0;
	}

	ForEachElmt(TLinkMap, mLinkMap, i, j)
	{
		TLink* pLink = i->second;
		DestroyLink(pLink);
	}

	mLinkMap.clear();

#ifdef _DEBUG
	m_timerAlive.Deactivate();
#endif

	if (theRoomTable.IsRoomTitleMonitoringOn())
		m_timerRoomTitleMonitor.Deactivate();

	return TRUE;
}

////////////////////////// for test ////////////////////////
//#include "Control.h"
//#include <pmsconnector.h>
////////////////////////// for test ////////////////////////

STDMETHODIMP_(void) CListener::OnSignal(HSIGNAL hObj, WPARAM wParam, LPARAM lParam)
{
	if(hObj == 0)
	{
		if(wParam == LISTENERMSG_CREATEANS)
		{
			TLock lo(this);

			CUser* pUser = (CUser*)lParam;
			if (!pUser) return;
			CLink* pLink = pUser->GetLink();
			if (!pLink) return;
			long lErrorCode = pUser->GetErrorCode();

			if (AddLink(pLink)) {
				PayloadNGSCli pld(PayloadNGSCli::msgCreateRoomNtf_Tag);
				pld.un.m_msgCreateRoomNtf->Clear();
				pld.un.m_msgCreateRoomNtf->m_lErrorCode = lErrorCode;
				SendMsg(pLink, pld);
			}

			TBase::OnSignal(hObj, wParam, lParam);
		}
		else if(wParam == LISTENERMSG_JOINANS)
		{
			TLock lo(this);

			CUser* pUser = (CUser*)lParam;
			if (!pUser) return;
			CLink* pLink = pUser->GetLink();
			if (!pLink) return;
			long lErrorCode = pUser->GetErrorCode();
//			long lErrorCode = JRF_SYSTEM;

			if (AddLink(pLink)) {
				PayloadNGSCli pld(PayloadNGSCli::msgJoinRoomAns_Tag);
				pld.un.m_msgJoinRoomAns->Clear();
				pld.un.m_msgJoinRoomAns->m_lErrorCode = lErrorCode;
				SendMsg(pLink, pld);
			}

			TBase::OnSignal(hObj, wParam, lParam);
		}
		else
		{
			TLock lo(this);
			TBase::OnSignal(hObj, wParam, lParam);
		}
	}
	else if (hObj == HSIGNAL_ONACCEPT)
	{
		TLock lo(this);

		CLink* pLink = (CLink *) wParam;
		SOCKET hSocket = (SOCKET) lParam;

		if ((NULL == pLink) || (hSocket == INVALID_SOCKET))
			return;

		BOOL bRet = pLink->Register(hSocket, NULL, HSIGNAL_XLINKHANDLER);
		if (!bRet)
		{
			::closesocket(hSocket);
			delete(pLink);
			return;
		}


	//////////////////////////////////////////////////////////////////////////
		LINGER  lingerStruct; 

		lingerStruct.l_onoff = 1; 
		lingerStruct.l_linger = 2; 
		::setsockopt((SOCKET)hSocket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));

		//////////////////////////////////////////////////////////////////////////

		bRet = AddLink(pLink);
		if (!bRet)
		{
			delete pLink;
			return;
		}
	}
	else if (m_timerRoomTitleMonitor.IsHandle(hObj))
	{
		LRBAddress addrNGS = theRoomTable.GetAddr();

		int nIsRoom = 0;

		//string sQuery = "LogDB|Q|INSERT ALL ";
		string sQuery = "LogDB|Q|INSERT INTO LOG_ROOMLIST (USN, SSN, CHANNEL, CREDATE, ROOMNAME, GLS_IP) VALUES (?, ?, ?, TO_CHAR(SYSDATE, 'YYYYMMDDHH24MISS'), ?, ?)";
		RoomList roomList;

		theRoomTable.GetRoomList(roomList);

		ForEachElmt(RoomList, roomList, i, j)
		{
			CRoom* pRoom = (*i);
			string sPartQuery;
			if (!pRoom) continue;

			RoomID roomID;
			roomID = pRoom->GetRoomID();

			LONG lSSN = roomID.m_lSSN;
			DWORD dwCategory = roomID.m_dwCategory;

			if (((lSSN == 2 || lSSN == 40 || lSSN == 3 || lSSN == 44 || lSSN == 17) && (dwCategory == 6 || dwCategory == 7 || dwCategory == 19)) || (lSSN == 18 && dwCategory == 6))
			{
				NFRoomOption nfRoomOption;
				pRoom->GetNFRoomOption(nfRoomOption);
				sPartQuery = ::format("|%ld|%ld|%ld|%s|%s", nfRoomOption.m_lCapUSN, lSSN, dwCategory, nfRoomOption.m_sRoomTitle.c_str(), addrNGS.GetString().c_str());

				sQuery += sPartQuery;
				nIsRoom = 1;
			}

			sPartQuery.clear();

			pRoom->Release();
		}
		//sQuery += "SELECT 1 FROM DUAL";

		//string sDeleteQuery = ::format("LogDB|Q|DELETE FROM LOG_ROOMLIST WHERE GLS_IP = '%s'", addrNGS.GetString().c_str());
		string sDeleteQuery = ::format("LogDB|Q|DELETE FROM LOG_ROOMLIST WHERE GLS_IP = ?|%s", addrNGS.GetString().c_str());

		BOOL bRet;
		DBGW_XString strDelResult;
		int nRet;
		bRet = ExecuteQuery(1, sDeleteQuery.c_str(), &strDelResult, &nRet);
		if (!bRet)
			theLog.Put(ERR_UK, "RoomTitleMonitor Error: Deleting RoomTitle Failed. Error = ", nRet);

		theLog.Put(DEV_UK, "Delete Query Test: ", sDeleteQuery);

		if (nIsRoom)
		{
			DBGW_XString strResult;
			bRet = ExecuteQuery(1, sQuery.c_str(), &strDelResult, &nRet);
			if (!bRet)
				theLog.Put(ERR_UK, "RoomTitleMonitor Error: Inserting RoomTitle Failed. Error = ", nRet, ", Query: ", sQuery);

			theLog.Put(DEV_UK, "Query Test: ", sQuery);
		}
	}
	else
	{
		TLock lo(this);
#ifdef _DEBUG
		if(m_timerAlive.IsHandle(hObj)) {
			TLOGAlive();
		}
#endif
		TBase::OnSignal(hObj, wParam, lParam);
	}
}

void CListener::TLOGAlive()
{
#ifdef _DEBUG
	SYSTEMTIME sys;
	memset(&sys, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&sys); 

	//theLog.Put(DEV, "-----------------------------------------------------");
	//string sLogTemp = ::format("CListener is Alive! [%02d/%02d, %02d:%02d ]", sys.wMonth, sys.wDay, sys.wHour, sys.wMinute);
	//theLog.Put(DEV, sLogTemp.c_str());
	//theLog.Put(DEV, "-----------------------------------------------------");
#endif
}

///////////////////////////////////////////////////////////////////////////////////
// CListener

void CListener::SendMsg(CLink* pSocket, const PayloadNGSCli& pld)
{
	// @@ Encrypt
	// Check using Encryption
	//if (TRUE == theEncryptMgr.IsMsgEncryptNeeded(pld))
	//{
	//	PayloadNGSCli pldEncrypted;
	//	int nRet = ::EncryptPld<PayloadNGSCli, EncryptedMsg>(pldEncrypted, pld, PayloadNGSCli::msgEncryptedMsg_Tag, theEncryptMgr.GetEncryptKey()); 
	//	if (0 != nRet)
	//	{
	//		theLog.Put(WAR_UK, "NGS_PacketEncrypt"_COMMA, "CListener::SendMsg - Message Encryption is failed. (Tag:", pld.mTagID, ", Error:", GetEncryptPldErrorString(nRet), ")");
	//		return;
	//	}
	//	pSocket->DoSendMsg(pldEncrypted);
	//	return;
	//}
	pSocket->DoSendMsg(pld);
}

BOOL CListener::OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR szAddr, LONG lPort)
{
	if ((nErrorCode) || (hSocket == INVALID_SOCKET))
		return FALSE;

	CLink* pLink = new CLink;
	VALIDATE(pLink);
	pLink->SetIP(::inet_addr(szAddr));

	BOOL bRet = ::XsigQueueSignal(GetThreadPool(), this, HSIGNAL_ONACCEPT, (WPARAM)pLink, LPARAM(hSocket));
	VALIDATE(bRet);

	theLog.Put(DEV, "CListener::OnListenerAccept", "Successs --------------------------------");

	return TRUE;
}

void CListener::DestroyLink(CLink* pLink)
{
	CUser* pUser = pLink->GetUser();
	if(pUser)
	{
		if (!pUser->IsPlaying()) {
			TKey key(pUser->GetUSN(), pUser->GetCSN());
			theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, pUser->GetRoomID(), 0);
		}

		pUser->SetLink(NULL);
		pLink->SetUser(NULL);
		delete(pUser);
	}

	RemoveLink(pLink);
	pLink->Unregister();

	delete(pLink);
}

BOOL CListener::OnError(CLink* pSocket, long lEvent, int nErrorCode)
{
	if (pSocket != NULL)
	{
		theLog.Put(DEV, "CListener::OnError(", pSocket->GetHandle(), ")", " Event : ", lEvent, " ErrorCode : ", nErrorCode );
		DestroyLink(pSocket);
	}
	return FALSE;
}

BOOL CListener::OnRcvMsg(CLink* pLink, PayloadCliNGS& pld)
{
	// @@ Encrypt
	// if Encrypted Message...
	//if (PayloadCliNGS::msgEncryptedMsg_Tag == pld.mTagID)
	//{
	//	PayloadCliNGS pldPlain;
	//	int nRet = ::DecryptPld<PayloadCliNGS, EncryptedMsg>(pldPlain, pld, theEncryptMgr.GetEncryptKey());
	//	if (0 != nRet)
	//	{
	//		theLog.Put(WAR_UK, "NGS_PacketEncrypt"_COMMA, "CListener::OnRcvMsg - Message Decryption is failed. (Error:", GetEncryptPldErrorString(nRet), ")");
	//		return FALSE;
	//	}
	//	theLog.Put(DEV, "CListener::OnRcvMsg - Original Message (Tag:", pldPlain.mTagID, ")");
	//	// Check using Encryption
	//	if (TRUE == theEncryptMgr.IsMsgEncryptNeeded(pldPlain))
	//	{
	//		return OnRcvMsgOrg(pLink, pldPlain);
	//	}
	//	else
	//	{
	//		theLog.Put(WAR_UK, "NGS_PacketEncrypt"_COMMA, "CListener::OnRcvMsg - This Message should not be Encrypted. (Tag:", pldPlain.mTagID, ")");
	//		return FALSE;
	//	}
	//}
	//// if Plain Message...
	//else
	//{
	//	// Check using Encryption
	//	if (TRUE == theEncryptMgr.IsMsgEncryptNeeded(pld))
	//	{
	//		theLog.Put(WAR_UK, "NGS_PacketEncrypt"_COMMA, "CListener::OnRcvMsg - This Message should be Encrypted. (Tag:", pld.mTagID, ")");
	//		return FALSE;
	//	}
	//}

	return OnRcvMsgOrg(pLink, pld);
}

BOOL CListener::OnRcvMsgOrg(CLink* pLink, PayloadCliNGS& pld)
{
	switch(pld.mTagID)
	{
	case PayloadCliNGS::msgCreateRoomReq_Tag:
		return OnRcvCreateRoomReq(pLink, pld.un.m_msgCreateRoomReq);
	case PayloadCliNGS::msgJoinRoomReq_Tag:
		return OnRcvJoinRoomReq(pLink, pld.un.m_msgJoinRoomReq);
	default:
		{
			theLog.Put(DEV, "CListener::OnRcvMsg - Unknown message(Tag:", pld.mTagID, ")");
		}
		break;
	}
	return OnError(pLink, FD_READ, -999);
}

void CListener::SendToAll(const PayloadNGSCli& pld)
{
	GBuf buf;
	VALIDATE(::LStore(buf, pld));
	ForEachElmt(TLinkMap, mLinkMap, it, jt) 
	{
		TLink *pLink = it->second;
		ASSERT(pLink);
		pLink->DoSend(buf);
	}
}

BOOL CListener::OnRcvCreateRoomReq(CLink* pLink, MsgCliNGS_CreateRoomReq* pMsg)
{
	RoomID roomID = pMsg->m_roomID;
	LONG lSSN = roomID.m_lSSN;

	if ( (g_wSvcType != (WORD)lSSN) && ( (g_wSvcType != (WORD)SVCTYP_ANY)) ) {
		theLog.Put(WAR_UK, "NGS_CreateRoom_Error, invalid SSN in CListener::OnRcvCreateRoomReq(). RoomID:", RoomID2Str(pMsg->m_roomID));
		PayloadNGSCli pld(PayloadNGSCli::msgCreateRoomNtf_Tag);
		pld.un.m_msgCreateRoomNtf->Clear();
		pld.un.m_msgCreateRoomNtf->m_lErrorCode = CRF_WRONGCATEGORY;
		SendMsg(pLink, pld);
		return TRUE;
	}

	CUser* pUser = pLink->GetUser();
	VERIFY(!pUser);
	if (pUser) {
		pLink->SetUser(NULL);
		pUser->SetLink(NULL);
		delete(pUser);
		pUser = NULL;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// User의 fingerprint check
	// pMsg->m_userInfo.m_sReserved1 => TimeStamp|HashedFingerprint
	// HashedFingerprint== md5hash_say(string::format("%d%d", pMsg->m_userInfo.m_lUSN, TimeStamp)+ "gOGo !!pmANg")
	// TimeStamp> time()- 60*60*4
	// 이 조건이 맞지 않을 경우, Game Client로 Noti
	BOOL bSuccessFlag = TRUE;
	// @@ NGS Renewal
	int nErrorFlag = 0;
	//int nFIdx = pMsg->m_userinfo.m_sReserved1.find('|');
	//if(nFIdx != -1)
	//{
	//	string sTimeStamp = pMsg->m_userinfo.m_sReserved1.substr(0, nFIdx);
	//	string sHashedFingerprint = pMsg->m_userinfo.m_sReserved1.substr(nFIdx+1, pMsg->m_userinfo.m_sReserved1.size() - nFIdx);
	//	char sEncData[1024] = {0x00};
	//	sprintf(sEncData, "%d%s%s", pMsg->m_userinfo.m_lUSN, sTimeStamp.c_str(), MD5_KEY);
	//	char sResult[1024] = {0x00};		
	//	md5hash_say(sResult, sEncData);

	//	ULONG lCurTime = time(NULL);
	//	ULONG lJoinTime = atol(sTimeStamp.c_str());
	//	if(strcmp(sHashedFingerprint.c_str(), sResult) == 0)
	//	{
	//		if(lJoinTime > lCurTime - MD5_TIME_GAP_DOWN && lJoinTime < lCurTime + MD5_TIME_GAP_UP )
	//			bSuccessFlag = TRUE;
	//		else
	//			nErrorFlag = 3;
	//	}
	//	else
	//		nErrorFlag = 2;
	//}
	//else
	//	nErrorFlag = 1;

	if (roomID.m_dwGRIID == ABSOLUTE_DELETE_VALUE)
	{
		theLog.Put(WAR_UK, "NGS_CreateRoom_Error, ABSOLUTE_DELETE_VALUE GRIID, USN = ", pMsg->m_nfUBI.m_lUSN);
		bSuccessFlag = FALSE;
	}

	if(bSuccessFlag == FALSE)
	{
		string sLogTemp = ::format("Not Matching UID / USN: %s / %ld. Error: %d in OnRcvCreateRoomReq", \
				pMsg->m_nfUBI.m_sNickName.c_str(), pMsg->m_nfUBI.m_lUSN, nErrorFlag);
		theLog.Put(WAR_UK, "NGS_CreateRoom_Error"_COMMA, sLogTemp.c_str());

		PayloadNGSCli pld(PayloadNGSCli::msgCreateRoomNtf_Tag);
		pld.un.m_msgCreateRoomNtf->Clear();
		pld.un.m_msgCreateRoomNtf->m_lErrorCode = CRF_SYSTEM;
		SendMsg(pLink, pld);

		return TRUE;
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	NFUserBaseInfo ud(pMsg->m_nfUBI);
	pLink->SetGatewayIP(pMsg->m_nfUBI.m_lGatewayIp);
	pLink->SetMacAddr(pMsg->m_nfUBI.m_sReserved2);
	
	ud.m_lGatewayIp = 0L;
	ud.m_sReserved1.erase();	
	ud.m_sReserved2.erase();

	pUser = new CUser(pMsg->m_nfUBI);
	VALIDATE(pUser);

	pUser->SetErrorCode(CRF_SUCCESS);
	pUser->SetRoomID(roomID);
	
	if (pMsg->m_dwCHSLogicalAddr.GetCastType() == CASTTYPE_INVALID)
	{
		string sLogTemp = ::format("Invalid CHS Address from client: USN = %ld, Room ID: %ld in OnRcvCreateRoomReq", \
			pUser->GetUSN(), roomID.m_dwGRIID);
		theLog.Put(WAR_UK, "NGS_CreateRoom_Error"_COMMA, sLogTemp.c_str());
	}
	pUser->SetCHSAddr(pMsg->m_dwCHSLogicalAddr);
	pLink->SetUser(pUser);
	pUser->SetLink(pLink);

	// 방제 신고에 따른 변경 - client의 코드 수정을 막기위해 여기서 처리
	// RoomOption 변경 - 클라이언트의 요청으로 인해서 여기서 넣음...
	pMsg->m_nfRoomOption.m_lCapUSN = pMsg->m_nfUBI.m_lLastPlayCSN;
	pMsg->m_nfRoomOption.m_lCreatorUSN = pMsg->m_nfUBI.m_lLastPlayCSN;

	if (1 == pMsg->m_nfRoomOption.m_lPlayType)
	{
		// 방제목을 서버에서 생성해야 한다.
		char szRoomIndex[1024] = {0,};
		sprintf(szRoomIndex, "-%.3d", roomID.m_dwGRIID);
		pMsg->m_nfRoomOption.m_sRoomTitle + szRoomIndex;
	}

	pUser->SetRoomOption(pMsg->m_nfUBI.m_lLastPlayCSN, pUser->GetUserID(), pMsg->m_nfRoomOption);
	pUser->SetState(US_ALIVE);

	CRoomPtr spRoom;
	BOOL bRet = theRoomTable.Create(pUser->GetRoomID(), &spRoom );
	if(bRet)
	{
		BOOL bRet2 = spRoom->Run(pUser->GetRoomID(), theRoomTable.GetAddr(), theRoomTable.GetTypeID(), pUser->GetNFRoomOption(), pMsg->m_sGameOption);
		if (bRet2) {
			RemoveLink(pLink);
			// For chat monitoring
			if (theRoomTable.GetChatMonSSN() > 0 && theRoomTable.GetChatMonSSN() == roomID.m_lSSN && theRoomTable.GetChatMonCategory() == LONG(roomID.m_dwCategory))
				spRoom->SetChatForward();

			theNLSManager.AddUserToNLS(pUser, ud.m_lClientIp, pMsg->m_nfUBI.m_sUserGameData, NLSMSGTYPE_CREATEROOM, NLSCLISTATUS_NFGAMESERVER, spRoom);
			return FALSE;
		}
		else
		{
			PayloadNGSCli pld(PayloadNGSCli::msgCreateRoomNtf_Tag);
			pld.un.m_msgCreateRoomNtf->Clear();
			pld.un.m_msgCreateRoomNtf->m_lErrorCode = CRF_DBERROR;
			SendMsg(pLink, pld);

			return TRUE;
		}
	}

	PayloadNGSCli pld(PayloadNGSCli::msgCreateRoomNtf_Tag);
	pld.un.m_msgCreateRoomNtf->Clear();
	pld.un.m_msgCreateRoomNtf->m_lErrorCode = CRF_ALREADY;
	
	SendMsg(pLink, pld);
	
	return TRUE;
}

BOOL CListener::OnRcvJoinRoomReq(CLink* pLink, MsgCliNGS_JoinRoomReq* pMsg)
{
	RoomID roomID = pMsg->m_roomID;
	
	LRBAddress addrCHS = pMsg->m_dwCHSLogicalAddr;

	CUser* pUser = pLink->GetUser();
	if (pUser) {
		pLink->SetUser(NULL);
		pUser->SetLink(NULL);
		delete(pUser);
		pUser = NULL;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// @@ NGS Renewal
	// User의 fingerprint check
	// pMsg->m_userInfo.m_sReserved1 => TimeStamp|HashedFingerprint
	// HashedFingerprint== md5hash_say(string::format("%d%d", pMsg->m_userInfo.m_lUSN, TimeStamp)+ "gOGo !!pmANg")
	// TimeStamp> time()- 60*60*4
	// 이 조건이 맞지 않을 경우, Game Client로 Noti
	BOOL bSuccessFlag = TRUE;
	int nErrorFlag = 0;
	//int nFIdx = pMsg->m_userinfo.m_sReserved1.find('|');
	//if(nFIdx != -1)
	//{
	//	string sTimeStamp = pMsg->m_userinfo.m_sReserved1.substr(0, nFIdx);
	//	string sHashedFingerprint = pMsg->m_userinfo.m_sReserved1.substr(nFIdx+1, pMsg->m_userinfo.m_sReserved1.size() - nFIdx);
	//	char sEncData[1024] = {0x00};
	//	sprintf(sEncData, "%d%s%s", pMsg->m_userinfo.m_lUSN, sTimeStamp.c_str(), MD5_KEY);
	//	char sResult[1024] = {0x00};		
	//	md5hash_say(sResult, sEncData);

	//	ULONG lCurTime = time(NULL);
	//	ULONG lJoinTime = atol(sTimeStamp.c_str());
	//	if(strcmp(sHashedFingerprint.c_str(), sResult) == 0)
	//	{
	//		if(lJoinTime > lCurTime - MD5_TIME_GAP_DOWN && lJoinTime < lCurTime + MD5_TIME_GAP_UP )
	//			bSuccessFlag = TRUE;
	//		else
	//			nErrorFlag = 3;
	//	}
	//	else
	//		nErrorFlag = 2;
	//}
	//else
	//	nErrorFlag = 1;

	if (roomID.m_dwGRIID == ABSOLUTE_DELETE_VALUE)
	{
		theLog.Put(WAR_UK, "NGS_JoinRoom_Error, ABSOLUTE_DELETE_VALUE GRIID, USN = ", pMsg->m_nfUBI.m_lUSN);
		bSuccessFlag = FALSE;
	}

	if(bSuccessFlag == FALSE)
	{
		string sLogTemp = ::format("Not Matching UID / USN: %s / %ld, Error: %d, String: %s in OnRcvJoinRoomReq", \
				pMsg->m_nfUBI.m_sNickName.c_str(), pMsg->m_nfUBI.m_lUSN, nErrorFlag);
		theLog.Put(WAR_UK, "NGS_CreateRoom_Error"_COMMA, sLogTemp.c_str());

		PayloadNGSCli pld(PayloadNGSCli::msgJoinRoomAns_Tag);
		pld.un.m_msgJoinRoomAns->Clear();
		pld.un.m_msgJoinRoomAns->m_lErrorCode = JRF_SYSTEM;
		SendMsg(pLink, pld);

		return TRUE;
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	NFUserBaseInfo ud(pMsg->m_nfUBI);
	pLink->SetGatewayIP(pMsg->m_nfUBI.m_lGatewayIp);
	pLink->SetMacAddr(pMsg->m_nfUBI.m_sReserved2);
	
	ud.m_lGatewayIp = 0L;
	ud.m_sReserved1.erase();	
	ud.m_sReserved2.erase();

	pUser = new CUser(pMsg->m_nfUBI);
	VALIDATE(pUser);

	pUser->SetErrorCode(JRF_SUCCESS);
	pUser->SetRoomID(roomID);

	LRBAddress tmpAddr;
	tmpAddr.Clear();
	pUser->SetCHSAddr(tmpAddr);

	pLink->SetUser(pUser);
	pUser->SetLink(pLink);
	pUser->SetPassword(pMsg->m_sPasswd);
	pUser->SetState(US_ALIVE);
	pUser->SetPlaying(FALSE);

	CRoomPtr spRoom;
	BOOL bRet = theRoomTable.FindRoom(roomID, &spRoom );
	if(bRet)
	{
		RemoveLink(pLink);

		if (spRoom->GetUserState(pUser->GetCSN(), 1))
			pUser->SetPlaying(TRUE);

		theNLSManager.AddUserToNLS(pUser, ud.m_lClientIp, pMsg->m_nfUBI.m_sUserGameData, NLSMSGTYPE_JOINROOM, NLSCLISTATUS_NFGAMESERVER, spRoom);

		return FALSE;
	}

	////// 요 밑에 있는건 안써. (..)
	PayloadNGSCli pld(PayloadNGSCli::msgJoinRoomAns_Tag);
	pld.un.m_msgJoinRoomAns->Clear();
	pld.un.m_msgJoinRoomAns->m_lErrorCode = JRF_NOTFOUND;
	SendMsg(pLink, pld);

	///////////////////// 유령방 삭제. 2004.3.16 kimsk ////////////////
	PayloadNGSCHS pld1(PayloadNGSCHS::msgRoomDeleteNtf_Tag);
	pld1.un.m_msgRoomDeleteNtf->m_roomID = roomID;

	theLrbManager.SendToCHS(theRoomTable.GetAddr(), addrCHS, pld1);
	theLog.Put(WAR_UK, "GLS_JoinRoom_Error"_COMMA, "Can't Find Room ID: ", RoomID2Str(pUser->GetRoomID()));
	///////////////////////////////////////////////////////////////////

	return TRUE;
}

