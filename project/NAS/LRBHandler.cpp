// CHSLRBManager.cpp
#include "stdafx.h"
#include "LRBHandler.h"
#include "Control.h"
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
	MakeAddress(m_svrAddr, 'U', SVCCAT_NAS);

	// for send address	
	MakeAddress(m_svcAddr[NAS_UNI], 'U', SVCCAT_NAS);
	MakeAddress(m_svcAddr[NAS_ANY], 'A', SVCCAT_NAS);
	MakeAddress(m_svcAddr[NAS_MUL], 'M', SVCCAT_NAS);

	// for recv address
	MakeAddress(m_svcMyAddr[NASME_UNI], 'U', SVCCAT_NAS);
	MakeAddress(m_svcMyAddr[NASME_MUL], 'M', SVCCAT_NAS);
	MakeAddress(m_svcMyAddr[NASME_ANY], 'A', SVCCAT_NAS);

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
	case SVCCAT_NAS:
		sKey = "NAS";		break;
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
	case SVCCAT_NAS:
		sKey = "NAS";		break;
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
			msgNtf.m_sWarnMsg  = ::format("NAS is disconnected from LRB. [IP:%s] [LRBIP:%s]\n", sHostAddr.c_str(), lpszLRBIP);
		}
		else
		{
			msgNtf.m_sWarnMsg  = ::format("NAS is disconnected from LRB. [IP:%s] [LRBIP:??]\n", sHostAddr.c_str());
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

	// 일단 전부 NAS 메시지로 가정. 
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
		LOG(ERR_UK ,"CLRBHandler," ,format("receive NACK message from CHS, NLS, NGS!, Addr = [%s] \n", dest.addr.GetString().c_str()));
		return;
	}
	
	PayloadCLINAS pld;
	BOOL bRet = BLoad(pld, buf);

	if(!bRet)
	{
		LOG(ERR_UK, "CLRBHANDLER_OnRcvServerMsg,    !!! LLoad Error");
		return; 
	}

	switch(pld.mTagID)
	{
	case PayloadCLINAS::msgReqTest_Tag:
		LOG(INF, "CLRBHandler,+++ Test Value: ", pld.un.m_msgReqTest->m_strMemo.c_str(),"\n");
		break;
	case PayloadCLINAS::msgReqAchvCaching_Tag:
		OnReqAchvCaching(pld.un.m_msgReqAchvCaching, src, dest);
		break;
	case PayloadCLINAS::msgReqCheckAchv_Tag:
		OnReqCheckAchv(pld.un.m_msgReqCheckAchv, src, dest);
		break;
	default:
		LOG(ERR_UK, "CLRBHandler,++++ Received Unknown Message from Svr ++++ (", pld.mTagID,")\n");
		break;
	}
}

void CLRBHandler::TerminateService(ListAddress& lstAddr)
{
	// 다른 서버가 종료되었을 때 호출됨
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 여기서 부터 실제 Action 코드들 들어감.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLRBHandler::OnReqAchvCaching(MsgCLINAS_ReqAchvCaching* pPld, const LRBAddress& src, const PayloadInfo& dest)
{
	//g_kAchvDef.LoadCharAchvState(pPld->m_lGSN, pPld->m_lCSN);
}

void CLRBHandler::OnReqCheckAchv(MsgCLINAS_ReqCheckAchv* pPld, const LRBAddress& src, const PayloadInfo& dest)
{
	//g_kAchvDef.CheckAchv(pPld->m_lGSN, pPld->m_lCSN, pPld->m_lAchvEvent, pPld->m_mapFactorVal);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// send message
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLRBHandler::SendToClient(const PayloadNASCLI& pld, const PayloadInfo& m_dest)
{
	GBuf buf;
	BOOL bRet = BStore(buf, pld);
	
	if(!bRet)
	{
		LOG(ERR_UK, "CLRBHandler,--Error PayloadNASCLI BStore FAIL--\n");
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, m_dest ) ;
}

void CLRBHandler::ConnectorSendTo(GBuf& buf, WORD wProtocol, LRBAddress& SrcAddr, const PayloadInfo& DestAddr)
{
	::XLRBConnectorAsyncSendTo(DestAddr.dwMID, buf, wProtocol, (LRBAddress*)&SrcAddr, (LRBAddress*)&(DestAddr.addr));

	LOG(INF_UK, "Sending  Data");
}

