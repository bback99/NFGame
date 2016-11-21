// CHSLRBManager.cpp
#include "stdafx.h"
#include "LRBHandler.h"
#include "Control.h"

#include "LCInfoMap.h"

//#include "PeerCtl.h"
#include <PMSConnObject.h>


#define ABSOLUTE_DELETE_VALUE	0xffffffff		//// RoomID::m_dwGRIID 를 ABSOLUTE_DELETE_VALUE로 하면 무조건 지운다.


BOOL GetHostAddr(string& rAddress, BOOL bExternalFirst)
{
	rAddress.erase();
	char buf[1024];
	if(::gethostname(buf, 1024) != 0) return FALSE;
	HOSTENT* phe = ::gethostbyname(buf);
	if(!phe) return FALSE;
	char * paddr = NULL;

	// 먼저 10.X.Y.Z의 형태가 아닌 놈을 찾는다.
	if(bExternalFirst) {
		if(!phe->h_addr_list) {
			return FALSE;
		}
		for(int i = 0; ; i++) {
			char * pa = phe->h_addr_list[i];
			if(pa == NULL) break;
			char * p = ::inet_ntoa(*(in_addr*)(pa));
			if(p) {
				if(::strncmp(p, "10.", 3)) {
					paddr = p;
					break;
				}
			}
		}
	}

	if(!paddr) {
		if(!(phe->h_addr)) {
			return FALSE;
		}
		paddr = ::inet_ntoa(*(in_addr *)(phe->h_addr));
		if(!paddr) {
			return FALSE;
		}
	}

	rAddress = paddr;
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// CLRBLink
///////////////////////////////////////////////////////////////////////////////////
CLRBLink::CLRBLink()
{
}

CLRBLink::~CLRBLink()
{
}

DWORD CLRBLink::GetRecvBufSize()
{
	return 1;
}

DWORD CLRBLink::GetSendBufQSize()
{
	return 1;
}
 
///////////////////////////////////////////////////////////////////////////////
// CLRBHandler

CLRBHandler theLRBHandler;

CLRBHandler::CLRBHandler()
{
}

CLRBHandler::~CLRBHandler() 
{
}

BOOL CLRBHandler::Run(HTHREADPOOL hThreadPool)
{
	SetupLRBAddress();

	BOOL bRet = ::XLRBConnectorInit(hThreadPool);
	if (!bRet)
	{
		LOG(ERR_UK, "CLRBHandler,",format("*** Fail to initialize XNLSConnector Error No (%ld)***\n",::GetLastError()));
		return FALSE;
	}

	if(RegisterAddress())
	{
		LOG(INF_UK, "CLRBHandler,############# SUCCESS : Run()  ~~~~~~~~~~!!! ################\n");
		return TRUE;
	}

	return FALSE;
}

BOOL CLRBHandler::Stop()
{
	LOG(INF_UK, "CLRBHandler,Stop XNLS::LRBHandler... ");
	::XLRBConnectorShutdown();
	return TRUE;
}

BOOL CLRBHandler::RegisterAddress()
{
	TLock lo(this);

	// Server Instance의 Address를 등록. XLRB에 connect() 
	BOOL bRet = FALSE;
	for(int i = 0; i < 4; i++)
	{
		bRet = ::XLRBConnectorStartup(&m_svrAddr, this);
		if (!bRet)
		{
			::Sleep(500);
			LOG(INF_UK, "CLRBHandler,",format("*** Retry Start up CLRBHandler [%d] ****\n", GetLastError()));
			continue;
		}
		else
			break;
	}
	if(!bRet)
	{
		LOG(ERR_UK, "CLRBHandler,",format("*** Fail to Start up CLRBHandler [%d] ****\n", GetLastError()));
		return FALSE;
	}
	// Service Instance의 Address를 등록
	for(int i = 0; i < MAX_MYADDR_CNT ; i++)	
	{
		bRet = ::XLRBConnectorRegister(&m_svcMyAddr[i], this);
		Sleep(500);
		if (!bRet)
		{
			LOG(ERR_UK, "CLRBHandler,",format("*** Fail to Register an Address [%d]type [%d]err ****\n",  i, GetLastError()));
			continue;
		}
	}
	//.....................

	return bRet;
}

BOOL CLRBHandler::SetupLRBAddress()	// address 생성.
{	
	MakeAddress(m_svrAddr, 'U', SVCCAT_NLS);

	// for send address	
	MakeAddress(m_svcAddr[NLS_UNI], 'U', SVCCAT_NLS);
	MakeAddress(m_svcAddr[NLS_ANY], 'A', SVCCAT_NLS);
	MakeAddress(m_svcAddr[NLS_MUL], 'M', SVCCAT_NLS);

	// for recv address
	MakeAddress(m_svcMyAddr[NLSME_UNI], 'U', SVCCAT_NLS);
	MakeAddress(m_svcMyAddr[NLSME_MUL], 'M', SVCCAT_NLS);
	MakeAddress(m_svcMyAddr[NLSME_ANY], 'A', SVCCAT_NLS);

	return FALSE;
}

void CLRBHandler::MakeAddress(LRBAddress & addr, DWORD dwCastType, DWORD dwCategory)
{
	string sApp, sKey;
	switch((LONG)dwCastType)
	{
	case 'M':
		sApp = "MULTICAST";		break;
	case 'U':
		//sApp = "UNICAST";		// unicasting address는 자체에서 만들수 없다..
		//break;
		ASSERT(1);				break;
	case 'A':
		sApp = "ANYCAST";		break;
	case 'B':
		sApp = "BROADCAST";		break;
	default:
		LOG(ERR_UK, "CLRBHandler,", format("Unknown CastType .. %d from LRBHandler Init. plz Check INI file", dwCastType));
	}

	switch(dwCategory)
	{
	case SVCCAT_NGS:
        sKey = "NGS";		break;
	case SVCCAT_CHS:
		sKey = "CHS";		break;
	case SVCCAT_ELB:
		sKey = "ELB";		break;
	case SVCCAT_AMS:
		sKey = "AMS";		break;
	case SVCCAT_NLS:
		sKey = "NLS";		break;
	case SVCCAT_BLS:
		sKey = "BLS";		break;
	case SVCCAT_MGS:
		sKey = "MGS";		break;
	default:
		LOG(ERR_UK, "CLRBHandler,",format("Unknown Category .. %d from LRBHandler Init.", dwCategory));
	}

	CHAR szTempAddr[LRBADDRESS_SIZE+1] = {0, };
	memset(szTempAddr, ' ', LRBADDRESS_SIZE);
	CHAR szDefAddr[LRBADDRESS_SIZE/2] = {0, };
	memset(szDefAddr, ' ', LRBADDRESS_SIZE/2);
	addr.Clear();
	::GetPrivateProfileStringA(sApp.c_str(), sKey.c_str(), sKey.c_str(), szDefAddr, LRBADDRESS_SIZE, "LRB_ADDR.ini");
	
	string sID = "";
	if(dwCastType == BYTE('U'))
	{
		DWORD dwIP = GSocketUtil::GetHostIP();
		sID = LRBAddress::GetIDFromIP(dwIP);
	}
	addr.SetAddress((BYTE)dwCastType, (BYTE)dwCategory, sID); 

	LOG(ERR_UK, "CLRBHandler Address ", addr.addr);
}


/////////////////////// for get addr //////////////////
static DWORD GetHostIPWithHostName(LPCSTR strHostName, bool bFindPrivateIP = false)
{
	HOSTENT* phe = ::gethostbyname(strHostName);
	if(!phe) return FALSE;
	DWORD dwIP = 0;
	char* paddr = NULL;

	// 먼저 10.X.Y.Z의 형태가 아닌 놈을 찾는다.
	if(!phe->h_addr_list)  return FALSE;
	for(int i = 0; ; i++) 
	{
		char * pa = phe->h_addr_list[i];
		if(pa == NULL) break;
		dwIP = ((in_addr*)(pa))->S_un.S_addr;
		char * p = ::inet_ntoa(*((in_addr*)(pa)));
		if(p) 
		{
			int nRes = ::strncmp(p, "10.", 3);
			if(bFindPrivateIP) {
				if (!nRes) {
					paddr = p;
					break;
				}
			}
			else {
				if (nRes) {
					paddr = p;
					break;
				}
			}
		}
	}

	if(!paddr) 
	{
		if(!(phe->h_addr)) 
			return 0;
		dwIP = ((in_addr*)(phe->h_addr))->S_un.S_addr;
		paddr = ::inet_ntoa(*(in_addr *)(phe->h_addr));
		if(!paddr) 
			return 0;			
	}
	return dwIP;
}
///////////////////////////////////////////////////////


void CLRBHandler::MakeOtherAddress(LRBAddress & addr, string m_sServerName, DWORD dwCastType, DWORD dwCategory)
{
	string sApp, sKey;
	switch((LONG)dwCastType)
	{
	case 'M':
		sApp = "MULTICAST";		break;
	case 'U':
//		sApp = "UNICAST";		break; // unicasting address는 자체에서 만들수 없다..
		ASSERT(1);				break;
	case 'A':
		sApp = "ANYCAST";		break;
	case 'B':
		sApp = "BROADCAST";		break;
	default:
		//theErr.LOG(1, "CLRBHandler", "Unknown CastType .. %d from LRBHandler Init.", dwCastType);
		LOG(ERR_UK, "CLRBHandler,", format("Unknown CastType .. %d from LRBHandler Init.", dwCastType));
	}

	switch(dwCategory)
	{
	case SVCCAT_NGS:
        sKey = "NGS";		break;
	case SVCCAT_CHS:
		sKey = "CHS";		break;
	case SVCCAT_ELB:
		sKey = "ELB";		break;
	case SVCCAT_AMS:
		sKey = "AMS";		break;
	case SVCCAT_NLS:
		sKey = "NLS";		break;
	case SVCCAT_BLS:
		sKey = "BLS";		break;
	default:
		LOG(ERR_UK, "CLRBHandler,",format( "Unknown Category .. %d from LRBHandler Init.", dwCategory));
	}

	string sID = "";
	if(dwCastType == BYTE('U'))
	{
		DWORD dwIP = GetHostIPWithHostName(m_sServerName.c_str());
		sID = LRBAddress::GetIDFromIP(dwIP);
	}
	addr.SetAddress((BYTE)dwCastType, (BYTE)dwCategory, sID);
}

void CLRBHandler::OnXLRBError(LONG lError)
{
	LOG(ERR_UK, "CLRBHandler,",format( "*** CXLRBCMsgHandler::OnXLRBEvent, Error Code %d ***\n", lError));

	{
		// Local Address를 얻어냄. - PMS에게 제공하기 위함.
		string sHostAddr, sLRBIP;
		GetHostAddress(sHostAddr, TRUE);
		LPSTR lpszLRBIP = GetLRBIP();

		PMSAWarningNtf msgNtf;
		if(lpszLRBIP != NULL)
		{
			msgNtf.m_sWarnMsg  = ::format("NLS is disconnected from LRB. [IP:%s] [LRBIP:%s]\n", sHostAddr.c_str(), lpszLRBIP);
		}
		else
		{
			msgNtf.m_sWarnMsg  = ::format("NLS is disconnected from LRB. [IP:%s] [LRBIP:??]\n", sHostAddr.c_str());
		}
		msgNtf.m_sTreatMsg = ::format("Check the LRB Servers \n");
		msgNtf.m_lErrLevel = FL_CRITICAL;
		msgNtf.m_unitID.m_dwSSN = 0;
		msgNtf.m_unitID.m_dwCategory = 0;
		PayloadHA pld(PayloadHA::msgPMSWarningNtf_Tag,msgNtf);
		
		thePMSConnector.SendWarningMsg(msgNtf.m_lErrLevel, msgNtf.m_sWarnMsg.c_str(), msgNtf.m_sTreatMsg.c_str(), msgNtf.m_unitID.m_dwSSN, msgNtf.m_unitID.m_dwCategory);
	}

	Stop();	
}

void CLRBHandler::OnXLRBRegister(LONG lErrorCode, LRBAddress& addr)
{
	if(lErrorCode == RDA_SUCCESS) {
		LOG(INF_UK, "CLRBHandler,+++++ Success Server Register +++++ \n");
		return;
	} 
	LOG(ERR_UK, "CLRBHandler," ,format( "+++++ Fail Service Register +++++ : lErrorCode = [%d]\n", lErrorCode));
}

void CLRBHandler::OnXLRBRcvMsg(const DWORD dwMID, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	PayloadInfo pldInfo(dwMID, src);

	BYTE btCast = src.GetServiceCategory();

	// 일단 전부 NLS 메시지로 가정. 
	if(SVCCAT_NGS == btCast || SVCCAT_CHS == btCast || SVCCAT_NCS == btCast)
		OnRcvServerMsg( pldInfo, buf, wMessageType, wProtocol , src);
	else
		LOG(ERR_UK, "CLRBHandler,", format("Unknown service type..source[%s], dest[%s] ", src.GetString().c_str(), dest.GetString().c_str()));	
}

void CLRBHandler::OnXLRBTerminateNtf(ListAddress& lstAddr)
{
	TerminateService(lstAddr);
}

void CLRBHandler::OnXLRBUnknownEvent(UINT pEvent, LONG lErrorCode, LPXBUF ppXBuf, LRBAddress& srcAddr, LRBAddress& destAddr)
{
}

void CLRBHandler::OnRcvServerMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol, const LRBAddress& src)
{
	if(wMessageType & MESSAGE_NACK)
	{
		LOG(ERR_UK ,"CLRBHandler," ,format("receive NACK message from CHS, GLS, MGS!, Addr = [%s] \n", dest.addr.GetString().c_str()));
		return;
	}

	PayloadCLINLS pld;

	BOOL bRet = BLoad(pld, buf);

	if(!bRet)	{
		LOG(ERR_UK, "CLRBHANDLER_OnRcvServerMsg,    !!! LLoad Error");
		return; 
	}
	

	switch(pld.mTagID)
	{
	case PayloadCLINLS::msgInsertCharacterReq_Tag:
		OnInsertCharacterReq(pld.un.m_msgInsertCharacterReq, src, dest);
		break;
	case PayloadCLINLS::msgUpdateCharacterReq_Tag:
		OnUpdateCharacterReq(pld.un.m_msgUpdateCharacterReq, src, dest);
		break;
	case PayloadCLINLS::msgDeleteCharacterReq_Tag:
		OnDeleteCharacterReq(pld.un.m_msgDeleteCharacterReq, src, dest);
		break;
	case PayloadCLINLS::msgGetCharacterReq_Tag:
		OnGetCharacterReq(pld.un.m_msgGetCharacterReq, src, dest);
		break;
	case PayloadCLINLS::msgGetRegionStatReq_Tag:
		OnGetRegionStatReq(pld.un.m_msgGetRegionStatReq, src, dest);
		break;
	case PayloadCLINLS::msgReqLocation_Tag:
		OnReqLocation(pld.un.m_msgReqLocation, src, dest);
		break;
	default:
		LOG(ERR_UK, "CLRBHandler,++++ Received Unknown Message from Svr ++++ (", pld.mTagID,")\n");
		break;
	}
}

void CLRBHandler::TerminateService(ListAddress& lstAddr)
{
	ForEachElmt(LRBAddressList, lstAddr.m_lstAddr, i, j)
	{
		LRBAddress _addr = *i;

		if (CASTTYPE_UNICAST == _addr.GetCastType())
		{
			string sAddr;
			_addr.GetStringFormat(sAddr);

			string strIP;
			NSAP nsapItem;

			BYTE cSvcCat = _addr.GetServiceCategory();

			switch(cSvcCat)
			{
			case SVCCAT_CHS:				
			case SVCCAT_NCS:
			case SVCCAT_NGS:
				{
					theLocationMainDB.RemoveValue(_addr);
				}
			default:
					LOG(ERR_UK, "CLRBHandler,++++ TerminateService ++++ (", cSvcCat,")\n");
				break;
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 여기서 부터 실제 Action 코드들 들어감.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLRBHandler::OnInsertCharacterReq(MsgCLINLS_InsertCharacterReq *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	TKey key(inputPld->m_Key);
	NLSBaseInfo tempBaseInfo;
	tempBaseInfo.Clear();

	// Location DB에 사용자 Insert 한다.
	NLSBaseInfo newBaseInfo(inputPld->m_Key, inputPld->m_strCharName, inputPld->m_lLevel, inputPld->m_roomID, inputPld->m_lGameMode, inputPld->m_nsapServerCurrent, inputPld->m_lStatus, 0, inputPld->m_serverLRBAddr);

	string strRoomID;
	inputPld->m_roomID.GetInstanceID(strRoomID);

	LONG lErr = theLocationMainDB.AddItem(key, newBaseInfo, tempBaseInfo);
	if (E_INSERT_USEREXIST == lErr) {
		// 어뷰징이다!!! 기존에 접속되어 있는 서버로 해당 유저를 내쫓으라고 알려준다.
		LOG(ERR_UK, "!!!@@@###$$$%%%^^^&&&*** Abusing!!! @@@###$$$%%%^^^&&&***");
		LOG(ERR_UK, "Received MsgCLINLS_InsertCharacterReq ### MainKey : ", inputPld->m_Key.m_lMainKey, ", SubKey : ", inputPld->m_Key.m_lSubKey);
		LOG(ERR_UK, "Received MsgCLINLS_InsertCharacterReq ### RoomID : ", strRoomID, ", Status : ", inputPld->m_lStatus);
		
		// 기존 유저의 CSN을 찾아야 함.
		NLSBaseInfo kickOutUser;
		if( FALSE == theLocationMainDB.GetItem(key, kickOutUser) )
		{
			LOG(ERR_UK, "GetItem Error!!!!!!");
		}

		PayloadNLSCLI	pld2(PayloadNLSCLI::msgDisconnectUserReq_Tag);
		pld2.un.m_msgDisconnectUserReq->m_Key = kickOutUser.m_Key;
		pld2.un.m_msgDisconnectUserReq->m_roomID = tempBaseInfo.m_roomID;
		SendToClient(pld2, tempBaseInfo.m_serverLRBAddr);

		// 유저가 있든 어뷰징이든, 새로 접속한 유저의 정보로 셋팅한다.
		theLocationMainDB.UpdateItem(inputPld->m_Key, newBaseInfo);
	}
	else {
		LOG(INF_UK, "Received MsgCLINLS_InsertCharacterReq ### MainKey : ", inputPld->m_Key.m_lMainKey, ", SubKey : ", inputPld->m_Key.m_lSubKey);
		LOG(INF_UK, "Received MsgCLINLS_InsertCharacterReq ### RoomID : ", strRoomID, ", Status : ", inputPld->m_lStatus);
	}

	// Send AnsMsg
	PayloadNLSCLI	pld(PayloadNLSCLI::msgInsertCharacterAns_Tag);
	pld.un.m_msgInsertCharacterAns->m_lErrorCode = lErr;
	pld.un.m_msgInsertCharacterAns->m_roomID = newBaseInfo.m_roomID;
	pld.un.m_msgInsertCharacterAns->m_Key = key;
	SendToClient(pld, dest);

	// 통계 Data를 업데이트 한다.
	// theLocationMainDB.AddRegionStat(temp_Info.m_tcli.m_RegionID);
}

void CLRBHandler::OnUpdateCharacterReq(MsgCLINLS_UpdateCharacterReq *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	LOG(INF_UK, "Received MsgCLINLS_UpdateCharacterReq MainKey : ", inputPld->m_Key.m_lMainKey, ", SubKey : ", inputPld->m_Key.m_lSubKey);

	// Location DB에 내용을 업데이트 한다.
	NLSBaseInfo temp_Info(inputPld->m_Key, inputPld->m_strCharName, inputPld->m_lLevel, inputPld->m_roomID, inputPld->m_lGameMode, inputPld->m_nsapServerCurrent, inputPld->m_lStatus, 0, inputPld->m_serverLRBAddr);
	theLocationMainDB.UpdateItem(inputPld->m_Key, temp_Info);

	// 통계 Data를 업데이트 한다.
	// theLocationMainDB.SubstractRegionStat(inputPld->m_tcliPrevious.m_RegionID);
	// theLocationMainDB.AddRegionStat(inputPld->m_tcliCurrent.m_RegionID);

	// 응답 보낸다.
	PayloadNLSCLI	pld(PayloadNLSCLI::msgUpdateCharacterAns_Tag );

	pld.un.m_msgUpdateCharacterAns->m_lErrorCode = 0;
	pld.un.m_msgUpdateCharacterAns->m_Key = inputPld->m_Key;

	SendToClient(pld, dest);
}


// void CLRBHandler::OnInsertCharacterReq(MsgCLINLS_InsertCharacterReq *inputPld, const LRBAddress& src, const PayloadInfo& dest)
// {
// 	TKey key(inputPld->m_Key);
// 
// 	// 먼저 Data가 존재하는 지 확인. 
// 	BOOL	bRet;
// 	NLSBaseInfo temp_Info_get;
// 	bRet = theLocationMainDB.GetItem(key, temp_Info_get);
// 
// 	// 사용자 존재! 문제 발생. 가져온 Data Return 
// 	if (bRet)	
// 	{
// 		PayloadNLSCLI	pld(PayloadNLSCLI::msgInsertCharacterAns_Tag );
// 
// 		// 유저가 어느 서버에서 나가서 DISCONNECT 셋팅이 되어 있는 상태에서, Insert 요청이 오면 Update한다.
// 		if (temp_Info_get.m_lStatus == NLSCLISTATUS_DISCONNECT) {
// 			temp_Info_get.m_lStatus = inputPld->m_lStatus;
// 
// 			pld.un.m_msgInsertCharacterAns->m_lErrorCode = E_INSERT_USEREXIST;
// 		}
// 		else {
// 			// 어뷰징이다!!! 기존에 접속되어 있는 서버로 해당 유저를 내쫓으라고 알려준다.
// 			pld.un.m_msgInsertCharacterAns->m_lErrorCode = E_INSERT_UNKNOWN;
// 
// 			LOG(INF_UK, "Abusing!!!@@@###$$$ MsgCLINLS_InsertCharacterReq MainKey : ", inputPld->m_Key.m_lMainKey, ", SubKey : ", inputPld->m_Key.m_lSubKey);
// 
// 			PayloadNLSCLI	pld2(PayloadNLSCLI::msgDisconnectUserReq_Tag);
// 			pld2.un.m_msgDisconnectUserReq->m_Key = key;
// 			SendToClient(pld2, temp_Info_get.m_serverLRBAddr);
// 		}
// 
// 		// 유저가 있든 어뷰징이든, 새로 접속한 유저의 정보로 셋팅한다.
// 		theLocationMainDB.UpdateItem(inputPld->m_Key, temp_Info_get);
// 
// 		pld.un.m_msgInsertCharacterAns->m_Key = key;
// 		pld.un.m_msgInsertCharacterAns->m_nsapServer.m_dwIP = temp_Info_get.m_nsap.m_dwIP;
// 		pld.un.m_msgInsertCharacterAns->m_nsapServer.m_dwPort = temp_Info_get.m_nsap.m_dwPort;
// 		pld.un.m_msgInsertCharacterAns->m_serverLRBAddr = temp_Info_get.m_serverLRBAddr;
// 				
// 		SendToClient(pld, dest);
// 
// 		return;
// 	}
// 
// 	// Location DB에 사용자 Insert 한다.
// 	NLSBaseInfo temp_Info;
// 	TKey keyItem;
// 	keyItem = key;
// 
// 	temp_Info.m_Key = inputPld->m_Key.m_lSubKey;
// 	temp_Info.m_roomID = inputPld->m_roomID;
// 	temp_Info.m_lStatus = inputPld->m_lStatus;
// 	temp_Info.m_nsap.m_dwIP = inputPld->m_nsapServerCurrent.m_dwIP;
// 	temp_Info.m_nsap.m_dwPort = inputPld-> m_nsapServerCurrent.m_dwPort;
// 	temp_Info.m_serverLRBAddr = inputPld->m_serverLRBAddr;
// 
// 	theLocationMainDB.AddItem(keyItem, temp_Info);
// 
// 	LOG(INF_UK, "Received MsgCLINLS_InsertCharacterReq MainKey : ", inputPld->m_Key.m_lMainKey, ", SubKey : ", inputPld->m_Key.m_lSubKey);
// 
// 	// 통계 Data를 업데이트 한다.
// 	// theLocationMainDB.AddRegionStat(temp_Info.m_tcli.m_RegionID);
// 
// 
// 	// 응답 보낸다.
// 	PayloadNLSCLI	pld(PayloadNLSCLI::msgInsertCharacterAns_Tag );
// 	
// 	pld.un.m_msgInsertCharacterAns->m_lErrorCode = S_INSERT_SUCCESS;
// 	pld.un.m_msgInsertCharacterAns->m_roomID = temp_Info.m_roomID;
// 	pld.un.m_msgInsertCharacterAns->m_Key = key;
// 	
// 	SendToClient(pld, dest);
// }






void CLRBHandler::OnDeleteCharacterReq(MsgCLINLS_DeleteCharacterReq *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	LOG(INF_UK, "Received MsgCLINLS_DeleteCharacterReq MainKey : ", inputPld->m_Key.m_lMainKey, ", SubKey : ", inputPld->m_Key.m_lSubKey);

	// 통계 Data를 업데이트 한다.
	// 지운뒤 통계 Data 업데이트 하면 업데이트 안 됨.
	// theLocationMainDB.SubstractRegionStat(inputPld->m_tcliCurrent.m_RegionID);

	// Location DB의 내용을 삭제한다.
	theLocationMainDB.RemItem(inputPld->m_Key);



	// 응답 보낸다.
	PayloadNLSCLI	pld(PayloadNLSCLI::msgDeleteCharacterAns_Tag );

	pld.un.m_msgDeleteCharacterAns->m_lErrorCode = 0;
	pld.un.m_msgDeleteCharacterAns->m_Key = inputPld->m_Key;

	SendToClient(pld, dest);
}





void CLRBHandler::OnGetCharacterReq(MsgCLINLS_GetCharacterReq *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	//LOG(INF_UK, "Received MsgCLINLS_GetCharacterReq MainKey : ", inputPld->m_Key.m_lMainKey, ", SubKey : ", inputPld->m_Key.m_lSubKey);

	// 응답 보낸다.
	PayloadNLSCLI	pld(PayloadNLSCLI::msgGetCharacterAns_Tag );

	// Location DB에서 정보를 가져 온다.
	ForEachElmt(list<TKey>, inputPld->m_lstKet, it, ij)
	{
		NLSBaseInfo temp_Info;
		theLocationMainDB.GetItem((*it), temp_Info);
		pld.un.m_msgGetCharacterAns->m_lstNLBBaseInfo.push_back(temp_Info);
	}

	pld.un.m_msgGetCharacterAns->m_lErrorCode = 0;

	SendToClient(pld, dest);
}



void CLRBHandler::OnGetRegionStatReq(MsgCLINLS_GetRegionStatReq *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	long result = 0;

	// LOG(INF_UK, "Received MsgCLINLS_GetRegionStatReq - RegionID:", int(inputPld->m_RegionID));

	// result = theLocationMainDB.GetRegionStat(inputPld->m_RegionID);

	// 응답 보낸다.
	PayloadNLSCLI	pld(PayloadNLSCLI::msgGetRegionStatAns_Tag );

	// pld.un.m_msgGetRegionStatAns->m_RegionID = inputPld->m_RegionID;
	pld.un.m_msgGetRegionStatAns->m_lCurrentUserNum = result;

	SendToClient(pld, dest);
}



void CLRBHandler::OnSetLeavingFlagNtf(MsgCLINLS_SetLeavingFlagNtf *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	NLSBaseInfo temp_Info;

	theLocationMainDB.GetItem(inputPld->m_Key, temp_Info);
	temp_Info.m_lStatus = FLAG_LEAVING;

	theLocationMainDB.UpdateItem(inputPld->m_Key, temp_Info);
	LOG(INF_UK, "Received MsgCLINLS_SetLeavingFlagNtf ");
}

void CLRBHandler::OnSetQuittingFlagNtf(MsgCLINLS_SetQuittingFlagNtf *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	NLSBaseInfo temp_Info;

	theLocationMainDB.GetItem(inputPld->m_Key, temp_Info);
	temp_Info.m_lStatus = FLAG_QUITTING;

	theLocationMainDB.UpdateItem(inputPld->m_Key, temp_Info);

	LOG(INF_UK, "Received MsgCLINLS_SetLeavingFlagNtf ");
}

void CLRBHandler::OnSetDisconnectedFlagNtf(MsgCLINLS_SetDisconnectedFlagNtf *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	NLSBaseInfo temp_Info;

	theLocationMainDB.GetItem(inputPld->m_Key, temp_Info);
	temp_Info.m_lStatus = FLAG_DISCONNECTED;

	theLocationMainDB.UpdateItem(inputPld->m_Key, temp_Info);

	LOG(INF_UK, "Received MsgCLINLS_SetLeavingFlagNtf ");
}




void CLRBHandler::SendToClient(const PayloadNLSCLI& pld, const PayloadInfo& m_dest)
{
	GBuf buf;
	BOOL bRet = BStore(buf, pld);
//	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		LOG(ERR_UK, "CLRBHandler,--Error PayloadNLSCLI BStore FAIL--\n");
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, m_dest ) ;
}

int glQuickUpdateUserCnt = 0;


void CLRBHandler::OnFindUserReq(MsgCLINLS_GetCharacterReq * pPld, const PayloadInfo & header)
{
	return;
}

void CLRBHandler::OnReqLocation(MsgCLINLS_ReqLocation *inputPld, const LRBAddress& src, const PayloadInfo& dest)
{
	ArcVectorT< NLSBaseInfo > kContNLSBaseInfo;

	// 요청한 데이터
	ArcVectorT< TKey >::iterator iter = inputPld->m_kContKey.begin();
	while( iter != inputPld->m_kContKey.end() )
	{	
		NLSBaseInfo nlsBaseInfo;
		if( theLocationMainDB.GetItem( (*iter), nlsBaseInfo ) )
		{
			kContNLSBaseInfo.push_back(nlsBaseInfo);
		}
		
		++iter;
	}

	// 요청한 유저의 RoomID(NGS에서필요)
	NLSBaseInfo reqUserInfo;
	TKey key;
	key.m_lMainKey = inputPld->m_lUSN;
	key.m_lSubKey = inputPld->m_lCSN;	
	if( !theLocationMainDB.GetItem( key, reqUserInfo ) )
	{
		return;
	}
	
	PayloadNLSCLI	pld(PayloadNLSCLI::msgAnsLocation_Tag );
	pld.un.m_msgAnsLocation->m_roomID = reqUserInfo.m_roomID;
	pld.un.m_msgAnsLocation->m_lUSN = inputPld->m_lUSN;
	pld.un.m_msgAnsLocation->m_lCSN = inputPld->m_lCSN;
	pld.un.m_msgAnsLocation->m_kContNLSBaseInfo = kContNLSBaseInfo;
	pld.un.m_msgAnsLocation->m_lCause = inputPld->m_lCause;
	pld.un.m_msgAnsLocation->m_strCharName = inputPld->m_strCharName;
	SendToClient(pld, dest);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// send message
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLRBHandler::ConnectorSendTo(GBuf& buf, WORD wProtocol, LRBAddress& SrcAddr, const PayloadInfo& DestAddr)
{
	::XLRBConnectorAsyncSendTo(DestAddr.dwMID, buf, wProtocol, (LRBAddress*)&SrcAddr, (LRBAddress*)&(DestAddr.addr));
	
	LOG(INF_UK, "Sending  Data");
	//DWORD dwMID, LPXBUF pXBuf, WORD wProtocol, const LPLRBADDRESS lpSrcAddr, const LPLRBADDRESS lpDestAddrz
}

