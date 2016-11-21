//
// Listener.cpp
//

#include "stdafx.h"
#include "common.h"
#include "Listener.h"
#include "Channel.h"
#include "ChannelDir.h"
#include "LRBHandler.h"
#include <NFVariant/NFDBManager.h>

#include <md5.h>

#ifdef _CHSNLS
	#include <NLSManager.h>
#endif

const long lMajorVersion = 7;
const long lMinorVersion = 0; 

extern long AddApplyUseItem(NFInvenSlot& useInven);
extern void AddAblitity(NFUser& NFUser, LONG lPartIndex, LONG lItemSRL);
extern LONG GetNFCharInfo(CUser* pUser, LONG lReqSSN = 0, string* pOutString = NULL);
///////////////////////////////////////////////////////////////////////////////////
// CSLinkAdlMgrT
BOOL CSLinkAdlMgrT::AddSLink(CLink* pLink) {
	//return TRUE;
	TLock lo(this);
	/*
	map<long, CLink*>::iterator iter;
	if((iter = m_mapAddrLink.find(pLink->GetIP())) != m_mapAddrLink.end())
	{
		theErr.LOG(1, "CHS_CSLinkAdlMgrT:AddSLink", "====> Listener Link Error : [%d]", pLink->GetIP());
		RemoveSLink(iter->second);
		m_pListener->DestroyLink(iter->second);
	}	
	m_mapAddrLink[pLink->GetIP()] = pLink; //return FALSE;
	*/

	SYSTEMTIME sys; memset(&sys, 0, sizeof(SYSTEMTIME)); ::GetLocalTime(&sys);
	FILETIME ftime; SystemTimeToFileTime(&sys, &ftime);

	MAP_LINK_TIME::iterator iter1;
	if((iter1 = m_mapLinkLastRcvTime.find(pLink)) != m_mapLinkLastRcvTime.end()) {
		RemoveSLink(iter1->first);
		m_pListener->DestroyLink(iter1->first);
	}
	m_mapLinkLastRcvTime[pLink] = ftime;
	return TRUE;
}
BOOL CSLinkAdlMgrT::RemoveSLink(CLink* pLink) {
	//return TRUE;
	TLock lo(this);
	m_mapLinkLastRcvTime.erase(pLink);
	//m_mapAddrLink.erase(pLink->GetIP());
	return TRUE;
}
void CSLinkAdlMgrT::CheckSLink() {	
	//return;
	TLock lo(this);
	SYSTEMTIME sys; memset(&sys, 0, sizeof(SYSTEMTIME)); ::GetLocalTime(&sys);
	FILETIME ftime; SystemTimeToFileTime(&sys, &ftime);
//	theErr.LOG(1, "Time1", "Time High/low = [%6d:%6d]/[%6d:%6d]", \
//		HIWORD(ftime.dwHighDateTime), LOWORD(ftime.dwHighDateTime), \
//		HIWORD(ftime.dwLowDateTime), LOWORD(ftime.dwLowDateTime));					

	FILETIME diffFT;
	ForEachCElmt( MAP_LINK_TIME, m_mapLinkLastRcvTime, iter, iter2) {
//		theErr.LOG(1, "Time2", "===> Time High/low = [%6d:%6d]/[%6d:%6d]", \
//			HIWORD(iter->second.dwHighDateTime), LOWORD(iter->second.dwHighDateTime), \
//			HIWORD(iter->second.dwLowDateTime), LOWORD(iter->second.dwLowDateTime));

		if( DiffFileTime(ftime, iter->second, diffFT) == 1) {	///// over 2 minutes				
			LOG(INF_UK, "CHS_CSLinkAdlMgrT"_LK, "====> CheckSLink::Listener Link Error : [", iter->first->GetIP(), "]");
			RemoveSLink(iter->first);
			m_pListener->DestroyLink(iter->first);								
		}
	}
}

void CSLinkAdlMgrT::UpdateTime(CLink *pLink)
{
	//return;
	TLock lo(this);
	SYSTEMTIME sys; memset(&sys, 0, sizeof(SYSTEMTIME)); ::GetLocalTime(&sys);
	FILETIME ftime; SystemTimeToFileTime(&sys, &ftime);
	MAP_LINK_TIME::iterator iter;
	if((iter = m_mapLinkLastRcvTime.find(pLink)) != m_mapLinkLastRcvTime.end())
		iter->second = ftime;
}

int CSLinkAdlMgrT::DiffFileTime(FILETIME aFT, FILETIME bFT, FILETIME& retFT)		/////// aFT - bFT
{		
	//return 1;
	if(aFT.dwLowDateTime >= bFT.dwLowDateTime)
	{
		retFT.dwLowDateTime = aFT.dwLowDateTime - bFT.dwLowDateTime;
		retFT.dwHighDateTime = aFT.dwHighDateTime - bFT.dwHighDateTime;
		if(retFT.dwHighDateTime > 0 || HIWORD(retFT.dwLowDateTime) > LINK_TIME_GAME*100)
			return 1;
	}
	else
	{
		retFT.dwLowDateTime = aFT.dwLowDateTime + 0xffff - bFT.dwLowDateTime + 1;
		retFT.dwHighDateTime = aFT.dwHighDateTime - bFT.dwHighDateTime -1;

		if(retFT.dwHighDateTime > 0 || HIWORD(retFT.dwLowDateTime) > LINK_TIME_GAME*100)
			return 1;
	}
	return -1;
}


///////////////////////////////////////////////////////////////////////////////////
// CListener

CListener theListener;

//// 채널 재접속을 위한 게임 설정 /// 고스톱, 맞고, 맞고 플러스, 정정당당 맞고, 호구 로봇, new 맞고.
map<long, long> gVecChannelRejoinGame;
BOOL GetParsingData(char* strParsing, vector<long>* vecData)
{
	string str = strParsing;
	string tempstr;

	long nData;	
	for(unsigned int index = 0 ; index < str.size() ; index++)
	{		
		char chData = str.at(index);
		if(chData >= '0' && chData <= '9')
			tempstr.append(1, chData);
		else if(tempstr != "")
		{
			nData = atol(tempstr.c_str());
			vecData->push_back(nData);
			tempstr.erase();
		}
	}
	////////// last ////////////
	if(tempstr != "")
	{
		nData = atol(tempstr.c_str());
		vecData->push_back(nData);
	}
	return TRUE;
}

void __stdcall _ListenerCallback(DWORD dwHandle, SOCKET hSocket, int nErrorCode, LPCSTR lpRemoteAddr, LONG lRemotePort, LPVOID lpContext)
{
	theListener.OnListenerAccept(hSocket, nErrorCode, lpRemoteAddr, lRemotePort);
}

CListener::CListener() : m_dwListener(0)
{
	m_bStating = FALSE;	
}
 
CListener::~CListener()
{
}

BOOL CListener::RunListen(int nPort)
{
	TLock lo(this);
	char szTemp[256] = {0, };
	DWORD dwRet = ::GetPrivateProfileStringA("TIMER", "STATECHK", "", szTemp, sizeof(szTemp), theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/);
	if(dwRet == 0) {
		TLOG0("Not found CHSConfig.INI file or Valid value \n");
		return FALSE;
	}

//	::ZeroMemory(szTemp, sizeof(szTemp));
//	dwRet = ::GetPrivateProfileString("TIMER", "ALIVECHK", _T(""), szTemp, sizeof(szTemp), "CHSConfig.INI");
//	if(dwRet == 0) {
//		TLOG0("Not found CHSConfig.INI file or Valid value \n");
//		return FALSE;
//	}
//	LONG lALIVECHK = atoi(szTemp);

    ///////////////// Setting Channel Rejoin Game !
	gVecChannelRejoinGame.clear();
	gVecChannelRejoinGame[1]   = FLAG_CHANNEL_REJOIN;		/// 고스톱	
	gVecChannelRejoinGame[14]  = FLAG_CHANNEL_REJOIN;		/// 맞고
	gVecChannelRejoinGame[19]  = FLAG_CHANNEL_REJOIN;		/// 맞고 플러스
	gVecChannelRejoinGame[22]  = FLAG_CHANNEL_REJOIN;		/// 정정당당 맞고
	gVecChannelRejoinGame[24]  = FLAG_CHANNEL_REJOIN;		/// 뉴 맞고
	gVecChannelRejoinGame[43]  = FLAG_CHANNEL_REJOIN;		/// 트롯맞고 
	gVecChannelRejoinGame[121] = FLAG_CHANNEL_REJOIN;		/// 호구로봇
	gVecChannelRejoinGame[25] = FLAG_CHANNEL_REJOIN;		/// 섯다
	gVecChannelRejoinGame[26] = FLAG_CHANNEL_REJOIN;		/// 도리짓고땡
	gVecChannelRejoinGame[54] = FLAG_CHANNEL_REJOIN;		/// 구슬모으기
    gVecChannelRejoinGame[45] = FLAG_CHANNEL_REJOIN;		/// 대부호
	
	//gVecChannelRejoinGame[2] = FLAG_CHANNEL_REJOIN;			/// 포커
	//gVecChannelRejoinGame[3] = FLAG_CHANNEL_REJOIN;			/// 하이로우
	//gVecChannelRejoinGame[17] = FLAG_CHANNEL_REJOIN;		/// 로우바둑이
	//gVecChannelRejoinGame[18] = FLAG_CHANNEL_REJOIN;		/// 와일드훌라
	//gVecChannelRejoinGame[23] = FLAG_CHANNEL_REJOIN;		/// 맞포커
	//gVecChannelRejoinGame[40] = FLAG_CHANNEL_REJOIN;		/// 뉴포커


	char sGetData[1024] = {0x00};
	BOOL bRetData = ::GetPrivateProfileStringA("CHANNEL_REJOIN", "GAME", "", sGetData, sizeof(sGetData)/sizeof(char), theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/);
	if(bRetData)
	{
		vector<long> vecData;
		::GetParsingData(sGetData, &vecData);
		for(vector<long>::iterator iter = vecData.begin() ; iter != vecData.end() ; iter++)
			gVecChannelRejoinGame[*iter] = FLAG_CHANNEL_REJOIN;
	}
	///////////////////////////////////////////////
	BOOL bRet = TRUE;
	bRet = bRet && ::XlstnCreate(&m_dwListener, &_ListenerCallback, nPort, NULL, NULL);
	VALIDATE(bRet);

	bRet = bRet && m_timerAlive.Activate(GetThreadPool(), this, 10000, 10000);//lALIVECHK, lALIVECHK);
	bRet = bRet && m_timerResetUserCnt.Activate(GetThreadPool(), this, 2000, 2000);	

	m_SLinkAdlMgrT.SetListener(this);

	VALIDATE(bRet);

	m_bStating = TRUE;
	return TRUE;
}

BOOL CListener::StopListen()
{
	TLock lo(this);

	if(m_dwListener)
	{
		::XlstnDestroy(m_dwListener);
		m_dwListener = 0;
	}

	ForEachElmt(TLinkMap, mLinkMap, i, j)
	{
		TLink* pLink = i->second;
		DestroyLink(pLink);
	}
	mLinkMap.clear();

	m_bStating = FALSE;
	return TRUE;
}

STDMETHODIMP_(void) CListener::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if(hSignal == 0)
	{
		TLock lo(this);
		TBase::OnSignal(hSignal, wParam, lParam);		
	}
	else if (hSignal == HSIGNAL_ONACCEPT)
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
			LOG(WAR_UK, "CHS_CListener, pLink->Register() Fail!!. GetLastError()=", GetLastError());
			return;
		}

		bRet = AddLink(pLink);
		if (!bRet)
		{
			pLink->Unregister();
			delete pLink;
			LOG(WAR_UK, "CHS_CListener, pLink->Unregister() Fail!!. GetLastError()=", GetLastError());
			return;
		}
		m_SLinkAdlMgrT.AddSLink(pLink);
	}
	else if(hSignal == LISTENER_JOINCHANNELANS)
	{
		TLock lo(this);
		CLink* pLink = (CLink*)wParam;		
		OnChannelJoinAns(pLink, lParam);
		return;
	}
	else if(hSignal == LISTENER_INVITECHANNELANS)
	{
		TLock lo(this);
		CUser * pUser = (CUser *)wParam;
		CLink* pLink = pUser->GetLink();
		if(!pLink) return;
		OnChannelInviteAns(pLink, lParam);
		return;
	}
	else if(hSignal == LISTENER_DJOINCHANNELANS)
	{
		TLock lo(this);
		CUser * pUser = (CUser *)wParam;
		CLink* pLink = pUser->GetLink();
		if(!pLink) return;
		OnChannelDirectJoinAns(pLink, lParam);
		return;
	}
// 	else if(hSignal == LISTENER_DUPLICATEJOINCHANNELANS)
// 	{
// 		TLock lo(this);		
// #ifdef _CHSNLS
// 		CUser * pUser = (CUser *)wParam;
// 		if(!pUser) return;
// 		CLink* pLink = pUser->GetLink();
// 		if(!pLink) return;
// 		
// 		AddLink(pLink);
// 		m_SLinkAdlMgrT.AddSLink(pLink);
// 
// 		CRoomID_For_ChannelReJoin* rFC = (CRoomID_For_ChannelReJoin*)lParam;
// 		if(rFC == NULL) {
// 			LOG(INF_UK, "CHS_CListener"_LK, "CListener::OnSignal (LISTENER_DUPLICATEJOINCHANNELANS) LParam = NULL!");
// 			return;
// 		}
// 		RoomID* rID = &(rFC->m_roomID);
// 		if(rID == NULL) {
// 			LOG(INF_UK, "CHS_CListener"_LK, "OnSignal::LISTENER_DUPLICATEJOINCHANNELANS : RoomID is Invalid!");
// 			delete rFC;
// 			return;
// 		}		
// 
// 		if(rFC->m_lMsgType == LCSMSGTYPE_NJOINCHANNEL)
// 		{
// 			ChannelInfo chInfo;
// 			NFUser	cbInfo;
// 			NFCharBaseInfoList ubInfo;
// #ifndef _NFGAME_
// 			RoomInfoInChannelList rInfo;
// #else
// 			NFRoomInfoInChannelList rInfo;
// #endif
// 			PayloadCHSCli pld(PayloadCHSCli::msgChannelJoinAns_Tag, \
// 				MsgCHSCli_ChannelJoinAns(ERR_CHANNEL_BANISH, chInfo, cbInfo, ubInfo, rInfo, *rID));
// 
// 			SendMsg(pLink, pld);			
// 			
// 			if(pUser != NULL)
// 			{
// 				string logstr = format("===== CListener::OnSignal: LISTENER_DUPLICATEJOINCHANNELANS : LCSMSGTYPE_NJOINCHANNEL : UserID = [%s], USN = [%d] !! ", \
// 					pUser->GetUserData().m_nfUserBaseInfo.m_sUID.c_str(), pUser->GetUserData().m_nfUserBaseInfo.m_lUSN);
// 				LOG(INF_UK, "CHS_CListener"_LK, logstr);
// 			}
// 			else
// 				LOG(INF_UK, "CHS_CListener"_LK, " ===== CListener::OnSignal: LISTENER_DUPLICATEJOINCHANNELANS : LCSMSGTYPE_NJOINCHANNEL : CUer = NULL");
// 
// 			// For Duplication Check - 고스톱/포커류 일 경우에만 :: 이어치기 관련
// 			RoomID rid;
// 			pUser->NLSGetRoomID(rid);
// //			if(IS_SINGLE_JOIN_SSN(rid.m_lSSN))
//             pLink->SetDiconCheck(TRUE);
// 		}
// 		else if(rFC->m_lMsgType == LCSMSGTYPE_DJOINCHANNEL)
// 		{			
// 			LRBAddress lrbAddr;
// 			NSAP nsapData;
// 			PayloadCHSCli pld(PayloadCHSCli::msgGRDirectCreateAns_Tag, \
// 				MsgCHSCli_GRDirectCreateAns(nsapData, 0, lrbAddr, _X(" "), _X(" "), ERR_CHANNEL_BANISH, *rID));
// 			SendMsg(pLink, pld);
// 			LOG(INF_UK, "CHS_CListener"_LK, "===== CListener::OnSignal: LISTENER_DUPLICATEJOINCHANNELANS : LCSMSGTYPE_DJOINCHANNEL");
// 		}
// 		else if(rFC->m_lMsgType == LCSMSGTYPE_INVITCHANNEL)
// 		{
// 			NSAP nsapData;
// 			PayloadCHSCli pld(PayloadCHSCli::msgGRInviteInfoAns_Tag, \
// 				MsgCHSCli_GRInviteInfoAns(nsapData, 0, _X(" "), _X(" "), ERR_CHANNEL_BANISH, *rID));
// 			SendMsg(pLink, pld);			
// 			LOG(INF_UK, "CHS_CListener"_LK, "===== CListener::OnSignal: LISTENER_DUPLICATEJOINCHANNELANS : LCSMSGTYPE_INVITCHANNEL");
// 		}
// 
// 		delete rFC;
// 		///// 서버에서 먼저 끊게 만든다.
// //		pLink->PostGracefulClose();
// 
// #endif
// 		return;
// 	}
	else if(hSignal == LISTENER_RESET_USERMSGCNT)
	{		
		theChannelDir.ResetUserMsgCnt();		
		//m_SLinkAdlMgrT.CheckSLink();		/////////// for test ////////////////
		return;
	}	
	else
	{
		if(m_timerAlive.IsHandle(hSignal))	
		{
			TLock lo(this);
			// 일정 시간동안의 접속 횟수룰 로그에 남긴다.
			m_LMsgCount.StampLogListen(2L);

			// Heartbeat flag seting
			theHeartBeat.SetListenFlag();
			theHeartBeat.SetLRBFlag();
		}
		else if(m_timerResetUserCnt.IsHandle(hSignal))	
		{
//			TLock lo(this);
			::XsigQueueSignal(::GetChannelThreadPool(), this, LISTENER_RESET_USERMSGCNT, (WPARAM)NULL, (LPARAM)NULL);
		}
		else
		{
			TLock lo(this);
			TBase::OnSignal(hSignal, wParam, lParam);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////
// CListener

void CListener::SendMsg(TLink* pSocket, const PayloadCHSCli& pld)
{
	pSocket->DoSendMsg(pld);
}

BOOL CListener::OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR lpAddr, LONG lPort)
{
	TRACE2("CListener::OnListenerAccept( %d, 0x%08x )\n", hSocket, nErrorCode );
	if(nErrorCode)
	{
		LOG(INF_UK, "CHS_CListener"_LK, "CListener::OnListenerAccept ErrorCode=", nErrorCode, ", HANDLE=", hSocket);
		return FALSE;
	}
	if (hSocket == INVALID_SOCKET)
	{
		LOG(INF_UK, "CHS_CListener"_LK, "CListener::OnListenerAccept Invalid Socket - Port is ", lPort);
		return FALSE;
	}

	CLink* pLink = new CLink;
	VALIDATE(pLink);
	pLink->SetIP(::inet_addr(lpAddr));

	BOOL bRet = ::XsigQueueSignal(GetThreadPool(), this, HSIGNAL_ONACCEPT, (WPARAM)pLink, LPARAM(hSocket));
	VALIDATE(bRet);

	return TRUE;
}

void CListener::DestroyLink(CLink* pLink)
{
	CUser* pUser = pLink->GetUser();
	if(pUser)
	{
//#ifdef _CHSNLS
		RoomID rid;
//		pUser->NLSGetRoomID(rid);
		TKey key(pUser->GetUSN(), pUser->GetCSN());
		theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, rid, pUser->GetUserData().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel);
//#endif
		pUser->SetLink(NULL);
		pLink->SetUser(NULL);
		delete(pUser);
	}
	m_SLinkAdlMgrT.RemoveSLink(pLink);
	RemoveLink(pLink);	
	pLink->Unregister();
	delete(pLink);
}

BOOL CListener::OnError(CLink* pSocket, long lEvent, int nErrorCode)
{
	string logstr = format("CListener::OnError(%d) Event=%ld, ErrorCode=%d", pSocket->GetHandle(), lEvent, nErrorCode);
	LOG(INF_UK, "CHS_CListener"_LK, logstr);
	DestroyLink(pSocket);
	return FALSE;
}

BOOL CListener::OnRcvMsg(CLink* pLink, PayloadCliCHS& pld)
{
	BOOL bRet = TRUE;
	m_SLinkAdlMgrT.UpdateTime(pLink);
	switch(pld.mTagID)
	{		
	case PayloadCliCHS::msgJoinChannelReq_Tag:
		bRet = OnRcvJoinChannelReq(pLink, pld.un.m_msgJoinChannelReq);	
		//message count 
		m_LMsgCount.m_dwJoin++;
		break;
	case PayloadCliCHS::msgGRInviteReq_Tag:	
		bRet = OnGRInviteReq(pLink, pld.un.m_msgGRInviteReq);	
		//message count 
		m_LMsgCount.m_dwInvite++;
		break;
	case PayloadCliCHS::msgGRDirectCreateReq_Tag:	
		bRet = OnGRDirectCreateReq(pLink, pld.un.m_msgGRDirectCreateReq);
		//message count
		m_LMsgCount.m_dwDirect++;
		break;	
	case PayloadCliCHS::msgGRJoinInfoReq_Tag:	
	default:
		LOG(INF_UK, "CHS_CListener"_LK, "++++ CListener : OnRcvmsg : Invalid Message Tag : [", pld.mTagID, "]");
		bRet = OnError(pLink, FD_READ, pld.mTagID /*INVALID_MSG*/);
		break;
	}
	return bRet;
}

BOOL CListener::OnGRInviteReq(CLink* pLink, MsgCliCHS_GRInviteReq* pMsg) 
{
	CUser* pUser = pLink->GetUser();
	if(pUser)
	{
		return OnError(pLink, FD_READ, PayloadCliCHS::msgGRInviteReq_Tag);
	}
	if(!ISVALID_USN(pMsg->m_userInfo.m_lUSN))
	{
		LOG(INF_UK, "CHS_CListener"_LK, "+++++ ASSERT : OnGRInviteReq  USN : ", pMsg->m_userInfo.m_lUSN, "+++++++");
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// User의 fingerprint check
	// pMsg->m_userInfo.m_sReserved1 => TimeStamp|HashedFingerprint
	// HashedFingerprint== md5hash_say(string::format("%d%d", pMsg->m_userInfo.m_lUSN, TimeStamp)+ "gOGo !!pmANg")
	// TimeStamp> time()- 60*60*4
	// 이 조건이 맞지 않을 경우, Game Client로 Noti
	BOOL bSuccessFlag = FALSE;
	int nErrorFlag = 0;
	int nFIdx = pMsg->m_userInfo.m_sReserved1.find('|');
	if(nFIdx != -1)
	{
		string sTimeStamp = pMsg->m_userInfo.m_sReserved1.substr(0, nFIdx);
		string sHashedFingerprint = pMsg->m_userInfo.m_sReserved1.substr(nFIdx+1, pMsg->m_userInfo.m_sReserved1.size() - nFIdx);
		char sEncData[1024] = {0x00};
		sprintf(sEncData, "%d%s%s", pMsg->m_userInfo.m_lUSN, sTimeStamp.c_str(), MD5_KEY);		
		char sResult[1024] = {0x00};		
		md5hash_say(sResult, sEncData);

		ULONG lCurTime = time(NULL);
		ULONG lJoinTime = atol(sTimeStamp.c_str());
		if(strcmp(sHashedFingerprint.c_str(), sResult) == 0)
		{
			if(lJoinTime > lCurTime - MD5_TIME_GAP_DOWN && lJoinTime < lCurTime + MD5_TIME_GAP_UP )
				bSuccessFlag = TRUE;
			else
				nErrorFlag = 3;
		}
		else
			nErrorFlag = 2;
	}
	else
		nErrorFlag = 1;
	if(bSuccessFlag == FALSE)
	{
		string logstr = format("+++++ ASSERT : OnSignal_InviteReq : Not Matching UID/USN = [%s]/[%d], ErrorFlag = [%d], [%s] ++++ ", \
			pMsg->m_userInfo.m_sUID.c_str(), pMsg->m_userInfo.m_lUSN, nErrorFlag, \
			pMsg->m_userInfo.m_sReserved1.c_str());
		LOG(INF_UK, "CHS_CListener"_LK, logstr);	

		NSAP nsap;
		LONG lErr = ERR_CHANNEL_NOTFOUND;
		PayloadCHSCli pld(PayloadCHSCli::msgGRInviteInfoAns_Tag, 
			MsgCHSCli_GRInviteInfoAns(nsap, pMsg->m_roomID.m_dwGRIID, _X(" "), _X(" "), lErr));
		SendMsg(pLink, pld);
		return TRUE;
	}
	///////////////////////////////////////////////////////////////////////////////////////////

	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(pMsg->m_roomID, &spChannel)) 
	{
		LONG lErr = 0L;

		NSAP nsap;
		lErr = ERR_CHANNEL_NOTFOUND;
		PayloadCHSCli pld(PayloadCHSCli::msgGRInviteInfoAns_Tag, 
			MsgCHSCli_GRInviteInfoAns(nsap, pMsg->m_roomID.m_dwGRIID, _X(" "), _X(" "), lErr));
		SendMsg(pLink, pld);

		return TRUE;	
	}

	m_SLinkAdlMgrT.RemoveSLink(pLink);
	RemoveLink(pLink);	

//#ifdef _CHSNLS

//	if(IS_SINGLE_JOIN_SSN(pMsg->m_roomID.m_lSSN))
	{
		///////// LCS 로 갈때는 CUser 단위로 처리해서 CUser를 생성
		// #### 수정해야 함 2010/6/22 ####
		//pUser = new CUser(pMsg->m_userInfo);
		VALIDATE(pUser);
		pUser->SetChannelID(spChannel->GetChannelID());
		pUser->SetLink(pLink);
		pLink->SetUser(pUser);
		/////////////////////////////////////////////////////////////////////////////////////
		string sOpt = "CHS_USER";
		pUser->m_dwGRIID = pMsg->m_roomID.m_dwGRIID;
		theNLSManager.AddUserToNLS((LPNLSOBJECT)pUser, pLink->GetIP(), sOpt, NLSMSGTYPE_INVITCHANNEL, NLSCLISTATUS_NFCHANNELSERVER, spChannel);
	}
//#else	
//	else
//		::XsigQueueSignal(::GetChannelThreadPool(), spChannel, CHANNELSIGNAL_INVITEREQ,  (WPARAM)pLink, (LPARAM)(pMsg->m_roomID.m_dwGRIID));
//#endif
	return FALSE;
}

BOOL CListener::OnGRDirectCreateReq(CLink* pLink, MsgCliCHS_GRDirectCreateReq* pMsg)
{
	RCPtrT<CChannel> spChannel;
	if(!theChannelDir.GetChannel(pMsg->m_channelID, &spChannel)) 
	{
		string logstr = format("Listener : Not found channel _Direct: [%d:%d:%d]SSN:CTEGORY:GCIID [%d]USN", \
			pMsg->m_channelID.m_lSSN, \
			pMsg->m_channelID.m_dwCategory,\
			pMsg->m_channelID.m_dwGCIID, \
			pMsg->m_userInfo.m_lUSN);
		LOG(INF_UK, "CHS_CListener"_LK, logstr);
		SendListenerMsg(*pLink, ERR_CHANNEL_NOTFOUND);
		return TRUE;
	} 
	ASSERT(spChannel != NULL);

	CUser* pUser = pLink->GetUser();
	ASSERT(!pUser);
	if(pUser)
	{
		LOG(INF_UK, "CHS_CListener"_LK, "+++++ ASSERT : OnGRDirectCreateReq+++++");
		return TRUE;
	}
	if(!ISVALID_USN(pMsg->m_userInfo.m_lUSN))
	{
		LOG(INF_UK, "CHS_CListener"_LK, "+++++ ASSERT : OnGRDirectCreateReq USN : ", pMsg->m_userInfo.m_lUSN, "++++++");	
		return TRUE;
	}

	// #### 수정해야 함 2010/6/22 ####
	//pUser = new CUser(pMsg->m_userInfo);
	VALIDATE(pUser);
	pUser->SetChannelID(pMsg->m_channelID);
	pUser->SetLink(pLink);
	pLink->SetUser(pUser);

	m_SLinkAdlMgrT.RemoveSLink(pLink);
	RemoveLink(pLink);	

	PayloadCliCHS msg(PayloadCliCHS::msgGRDirectCreateReq_Tag, MsgCliCHS_GRDirectCreateReq(*pMsg));
	GBuf buf;
	VALIDATE(::BStore(buf, msg));
	LPXBUF pXBuf = buf.Detach();
	//LPXBUF pXBuf = ::XbufCreate(buf.GetXBuf());
//	LPXBUF pXBuf = NULL;
//	VALIDATE(::BStore(pXBuf, msg));
	//pUser->PushMsg(msg);

	::XsigQueueSignal(GetChannelThreadPool(), spChannel, CHANNEL_DIRECTCREATE, (WPARAM)pXBuf, (LPARAM)pUser);

	return FALSE;
}
void CListener::SendListenerMsg(CLink & link, LONG lErrCode)
{
	PayloadCHSCli pld(PayloadCHSCli::msgChannelJoinAns_Tag, MsgCHSCli_ChannelJoinAns(lErrCode));
	SendMsg(&link, pld);
}

BOOL CListener::OnRcvJoinChannelReq(CLink* pLink, MsgCliCHS_JoinChannelReq* pMsg)	
{
	RCPtrT<CChannel> spChannel;
	if(!theChannelDir.GetChannel(pMsg->m_channelID, &spChannel)) 
	{
		string logstr = format("Listener : Not found channel : [%d:%d:%d]SSN:CTEGORY:GCIID [%d]USN", \
			pMsg->m_channelID.m_lSSN, \
			pMsg->m_channelID.m_dwCategory, \
			pMsg->m_channelID.m_dwGCIID, \
			pMsg->m_userInfo.m_lUSN);
		LOG(INF_UK, "CHS_CListener"_LK, logstr);
		SendListenerMsg(*pLink, ERR_CHANNEL_NOTFOUND);
		return TRUE;
	} 
	ASSERT(spChannel != NULL);

	CUser* pUser = pLink->GetUser();
	if(pUser)
	{
		LOG(INF_UK, "CHS_CListener"_LK, "+++++ ASSERT :OnRcvJoinChannelReq +++++");
		return TRUE;
	}
	if(!ISVALID_USN(pMsg->m_userInfo.m_lUSN))
	{
		LOG(INF_UK, "CHS_CListener"_LK, "+++++ ASSERT : OnRcvJoinChannelReq USN : ", pMsg->m_userInfo.m_lUSN, "+++++++");	
		return TRUE;
	}

	// #### 수정해야 함 2010/6/22 ####
	pUser = new CUser(pMsg->m_userInfo);
	VALIDATE(pUser);
	pUser->SetChannelID(pMsg->m_channelID);
	pUser->SetBLSData(pMsg->m_sBLSPayload);
	pUser->SetGameMode(pMsg->m_lGameMode);
	pUser->SetLink(pLink);
	pLink->SetUser(pUser);

	m_SLinkAdlMgrT.RemoveSLink(pLink);
	RemoveLink(pLink);

	// NF게임에서는 모든 게임들을 NLS로 등록한다.
	string sOpt = "CHS_USER";
	theNLSManager.AddUserToNLS((LPNLSOBJECT)pUser, pLink->GetIP(), sOpt, NLSMSGTYPE_NJOINCHANNEL, NLSCLISTATUS_NFCHANNELSERVER, spChannel);
	return FALSE;
}



void CListener::SendToAll(const PayloadCHSCli& pld)
{
	GBuf buf;
	VALIDATE(::LStore(buf, pld));
	ForEachElmt(TLinkMap, mLinkMap, i, j) 
	{
		TLink *pLink = i->second;
		ASSERT(pLink);
		pLink->DoSend(buf);
	}
}

void CListener::OnChannelJoinAns(TLink* pLink, LONG lErrCode)
{
	if(!pLink) return;
	AddLink(pLink);
	m_SLinkAdlMgrT.AddSLink(pLink);
	SendListenerMsg(*pLink, lErrCode);
	LOG(INF_UK, "CHS_CListener"_LK, "======== Join channel Error : Not found channel or Failed DB Query !! Error =  ", lErrCode);
}


void CListener::OnChannelInviteAns(CLink * pLink, LONG lErrCode)
{
	if(!pLink) return;
//  이건 따로 메세지를 정의하지 않았음. 
	AddLink(pLink);
	m_SLinkAdlMgrT.AddSLink(pLink);
	PayloadCHSCli pld(PayloadCHSCli::msgGRInviteInfoAns_Tag);
	pld.un.m_msgGRInviteInfoAns->m_lErrorCode = lErrCode;
	SendMsg(pLink, pld);
	LOG(INF_UK, "CHS_CListener"_LK, "======== GR Invite channel Error : Not found channel or Failed DB Query !! Error = ", lErrCode);
}

void CListener::OnChannelDirectJoinAns(CLink * pLink, LONG lErrCode)
{
	if(!pLink) return;
	AddLink(pLink);
	m_SLinkAdlMgrT.AddSLink(pLink);
	PayloadCHSCli pld(PayloadCHSCli::msgGRDirectCreateAns_Tag);
	pld.un.m_msgGRDirectCreateAns->m_lErrorCode = lErrCode;
	SendMsg(pLink, pld);
	LOG(INF_UK, "CHS_CListener"_LK, "======== D Join channel Error : Not found channel or Failed DB Query !! Error = ", lErrCode);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// CInviteListener

CInviteListener theInviteListener;

void __stdcall _InviteListenerCallback(DWORD dwHandle, SOCKET hSocket, int nErrorCode, LPCSTR lpRemoteAddr, LONG lRemotePort, LPVOID lpContext)
{
	theInviteListener.OnListenerAccept(hSocket, nErrorCode, lpRemoteAddr, lRemotePort);
}

CInviteListener::CInviteListener() : m_dwListener(0)
{
	m_bStating = FALSE;
}
 
CInviteListener::~CInviteListener()
{
}

BOOL CInviteListener::RunListen(int nPort)
{
	TLock lo(this);

	BOOL bRet = TRUE;
	bRet = bRet && ::XlstnCreate(&m_dwListener, &_InviteListenerCallback, nPort, NULL, NULL);
	VALIDATE(bRet);

	m_bStating = TRUE;
	return TRUE;
}

BOOL CInviteListener::StopListen()
{
	TLock lo(this);

	if(m_dwListener)
	{
		::XlstnDestroy(m_dwListener);
		m_dwListener = 0;
	}

	ForEachElmt(TLinkMap, mLinkMap, i, j)
	{
		TLink* pLink = i->second;
		DestroyLink(pLink);
	}
	mLinkMap.clear();

	m_bStating = FALSE;
	return TRUE;
}

STDMETHODIMP_(void) CInviteListener::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if(hSignal == HSIGNAL_ONACCEPT)
	{
		TLink* pLink = (TLink *) wParam;
		SOCKET hSocket = (SOCKET) lParam;

		if ((NULL == pLink) || (hSocket == INVALID_SOCKET))
			return;

		TLock lo(this);

		BOOL bRet = pLink->Register(hSocket, NULL, HSIGNAL_XLINKHANDLER);
		if (!bRet)
		{
			::closesocket(hSocket);
			delete(pLink);
			return;
		}

		bRet = AddLink(pLink);
		if (!bRet)
		{
			delete pLink;
			return;
		}
	}
	else
	{
		TLock lo(this);
		TBase::OnSignal(hSignal, wParam, lParam);
	}
}


///////////////////////////////////////////////////////////////////////////////////
// CInviteListener
void CInviteListener::SendMsg(CInviteLink* pSocket, const PayloadInvitation& pld)
{
	pSocket->DoSendMsg(pld);
}

BOOL CInviteListener::OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR lpAddr, LONG lPort)
{
	TRACE2("CInviteListener::OnListenerAccept( %d, 0x%08x )\n", hSocket, nErrorCode );
	if(nErrorCode)
	{
		string logstr = format("CInviteListener::OnListenerAccept ErrorCode=%d, HANDLE=0x%08x", nErrorCode, hSocket);
		LOG(INF_UK, "CHS_CInviteListener"_LK, logstr);
		return FALSE;
	}
	if (hSocket == INVALID_SOCKET)
	{
		LOG(INF_UK, "CHS_CInviteListener"_LK, "CInviteListener::OnListenerAccept Invalid Socket - Port is ", lPort);
		return FALSE;
	}

	CInviteLink* pLink = new CInviteLink;
	VALIDATE(pLink);

	BOOL bRet = ::XsigQueueSignal(GetThreadPool(), this, HSIGNAL_ONACCEPT, (WPARAM)pLink, LPARAM(hSocket));
	VALIDATE(bRet);

	return TRUE;
}

void CInviteListener::DestroyLink(CInviteLink* pLink)
{
	RemoveLink(pLink);
	pLink->Unregister();
	delete(pLink);
}

BOOL CInviteListener::OnError(CInviteLink* pSocket, long lEvent, int nErrorCode)
{
	LOG(INF_UK, "CHS_CInviteListener"_LK, "+++++ Invite Listener OnError ++++++");
	TRACE1("CInviteListener:: OnError(%d)\n", nErrorCode);
	DestroyLink(pSocket);
	return FALSE;
}

BOOL CInviteListener::OnRcvMsg(CInviteLink* pLink, PayloadInvitation& pld)
{
	switch(pld.mTagID)
	{
	case PayloadInvitation::msgInvitationInfoReq_Tag :
		return OnRcvInvitationInfoReq(pLink, pld.un.m_msgInvitationInfoReq);
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInviteListener::OnRcvInvitationInfoReq(CInviteLink* pLink, MsgInvitationInfoReq* pMsg)
{
	if(!ISVALID_USN(pMsg->m_userInfo.m_lUSN))
	{
		LOG(INF_UK, "CHS_CInviteListener"_LK, "+++++ ASSERT : OnRcvInvitationInfoReq. USN : ", pMsg->m_userInfo.m_lUSN);	
		PayloadInvitation pld1(PayloadInvitation::msgInvitationInfoNtf_Tag);
		MsgInvitationInfoNtf& rMsg = *(pld1.un.m_msgInvitationInfoNtf);
		rMsg.m_lErrorCode = INVITE_ERR_INVALIDUSN;
		SendMsg(pLink, pld1);
		return TRUE;
	}

	RCPtrT<CChannel> spChannel;
	if(!theChannelDir.GetChannel(pMsg->m_channelID, &spChannel)) 
	{
		string logstr = format("Invite Listener : Not found channel : [%d:%d:%d]SSN:CTEGORY:GCIID [%d]USN", \
			pMsg->m_channelID.m_lSSN, \
			pMsg->m_channelID.m_dwCategory, \
			pMsg->m_channelID.m_dwGCIID, \
			pMsg->m_userInfo.m_lUSN);
		LOG(INF_UK, "CHS_CInviteListener"_LK, logstr);

		PayloadInvitation pld1(PayloadInvitation::msgInvitationInfoNtf_Tag);
		MsgInvitationInfoNtf& rMsg = *(pld1.un.m_msgInvitationInfoNtf);
		rMsg.m_lErrorCode = INVITE_ERR_CHANNELNOTFOUND;
		SendMsg(pLink, pld1);
		return TRUE;
	} 
	ASSERT(spChannel != NULL);

	NFUserBaseInfo ubi(pMsg->m_userInfo, 0);
	pLink->SetUserInfo(ubi);
	pLink->SetRoomID(pMsg->m_roomID);
	pLink->SetNSAP(pMsg->m_nsapGLS);

	RemoveLink(pLink);
	::XsigQueueSignal(GetChannelThreadPool(), spChannel, 0, (WPARAM)CHANNEL_INVITEUSERJOIN, (LPARAM) pLink);

	return FALSE;
}

void CInviteListener::SendToAll(const PayloadInvitation& pld)
{
	GBuf buf;
	VALIDATE(::LStore(buf, pld));
	ForEachElmt(TLinkMap, mLinkMap, i, j) 
	{
		CInviteLink *pLink = i->second;
		ASSERT(pLink);
		pLink->DoSend(buf);
	}
}

void CInviteListener::SendMsg(CInviteLink* pLink, PayloadInvitation& pld)
{
	GBuf buf;
	::LStore(buf, pld);

	pLink->DoSend(buf);
}
