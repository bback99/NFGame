#include "stdafx.h"
#include "LRBHandler.h"
#include "Manager.h"
#include <PMSConnector.h>
#include <NLSManager.h>

// 고려 사항 
// nack / ack 구분해서 핸들링
// protocol 검사하는 routine 넣기

CManager		theManager;
CLRBHandler		theLRBHandler;


CLRBHandler::CLRBHandler()
{
	// 정확한 값 넣기
	m_dwServiceTypeID	= MAKELONG(SVCTYP_ANY, SVCCAT_NCS);
	m_bRegMultiAddr		= FALSE;
	m_bRegAnyAddr		= FALSE;
	m_bFICalled			= FALSE;
}

CLRBHandler::~CLRBHandler(void)
{
	Stop();
}

void CLRBHandler::SetComputerName(LPCSTR szName, DWORD dwLength)
{
	TLock lo(this);

	m_sComputerName.assign(szName, dwLength);
}

BOOL CLRBHandler::Init()
{
	string sInternalIP;

#ifdef _WNLB_APPLY_

	if (!GetRealIPAddressFile(sInternalIP))
	{
		LOG (INF, "GetRealIPAddressFile Failed!!!!!!!!!!!!");

		sInternalIP = theManager.GetIP();
	}

#else

	sInternalIP = theManager.GetIP();

#endif

	DWORD dwIP = inet_addr(sInternalIP.c_str());
	string sID = adl::LRBAddress::GetIDFromIP(dwIP);

	LOG (INF, "CLRBHandler : IP Address=", sID);


	
	// BLRBConnector Initialize
	ASSERT(CManager::m_hThreadPool);

	BOOL bRet = ::XLRBConnectorInit(CManager::m_hThreadPool);
	if (!bRet)
	{
		LOG	(ERR, "*** Fail to initialize BLRBConnector ***");
		return FALSE;
	}
	

	DWORD nSize = MAX_COMPUTERNAME_LENGTH + 1;
	char szCName[MAX_COMPUTERNAME_LENGTH + 1];
	bRet = ::GetComputerNameA(&szCName[0], &nSize);
	if (!bRet) {
		LOG (ERR, "*** Fail to get computer name ***");
		return FALSE;
	}

	SetComputerName(szCName, nSize);
	// My Address (NCS)
	m_serverAddr.SetAddress(CASTTYPE_UNICAST, SVCCAT_NCS, sID);
	m_serviceAddr[0].SetAddress(CASTTYPE_ANYCAST, SVCCAT_NCS, sID);
	m_serviceAddr[1].SetAddress(CASTTYPE_MULTICAST, SVCCAT_NCS, sID);

	// Set NLS Address
	m_NLSMulticastAddr.SetAddress(CASTTYPE_MULTICAST, SVCCAT_NLS, sID);

	// Multicast address
	m_CHSMulticastAddr.SetAddress(CASTTYPE_MULTICAST, SVCCAT_CHS, sID);
	m_NGSMulticastAddr.SetAddress(CASTTYPE_MULTICAST, SVCCAT_NGS, sID);
	m_NASMulticastAddr.SetAddress(CASTTYPE_MULTICAST, SVCCAT_NAS, sID);

	return TRUE;
}

BOOL CLRBHandler::Run()
{
	BOOL bRet = RegisterAddress();
	if ( bRet )
		LOG (INF, "*** Success : Connected to LRB ***");
	else
		LOG (INF, "*** FAiled : Connected Failed to LRB ***");

	return bRet;
}

void CLRBHandler::Stop(void)
{
	TLock lo(this);

	m_bRegMultiAddr = FALSE;
	m_bRegAnyAddr   = FALSE;
	m_bFICalled	    = TRUE;
	::XLRBConnectorShutdown();			// 여러번 불릴수 있다
}

BOOL CLRBHandler::IsRegistered()
{ 
	return ( m_bRegMultiAddr && m_bRegAnyAddr ) ;
} 

BOOL CLRBHandler::RegisterAddress()
{

	TLock lo(this);
	// Server Instance의 Address를 등록. BLRB에 connect() 

	BOOL bRet = FALSE;
	for(int i  = 1; i < XLRB_RETRYCOUNT; i++)
	{
		if (::XLRBConnectorStartup(&m_serverAddr, this))
		{
			// Service Instance의 Address를 등록
			BOOL bRet = ::XLRBConnectorRegister(&m_serviceAddr[0], this);
			if (!bRet)
			{
				LOG (ERR, "CLRBHandler : *** Fail to Register an Address 1 [",GetLastError(),"] ****");
				return FALSE;
			}

			bRet = ::XLRBConnectorRegister(&m_serviceAddr[1], this);
			if (!bRet)
			{
				LOG (ERR, "CLRBHandler : *** Fail to Register an Address 2 [",GetLastError(),"] ****");
				return FALSE;
			}
			return TRUE;
		}
		else
		{
			LOG (ERR, "CLRBHandler : FAIL -- BLRBConnector Startup FAIL, Retry Count is ",i,", Err =  ",GetLastError());
		}
	}

	return bRet;

}
/////////////////////////////////////////////////////////////////////
//enum ERROR
//{
//	ERROR_UNKNOWN = 0,
//	ERROR_REGISTERROR = 1,
//	ERROR_DEREGISTERROR = 2,
//	ERROR_CONNECTIONCLOSE = 10
//};
// 에러 발생시 BLRB와의 연결을 끊고, 주기적으로 재접속을 시도한다.
void CLRBHandler::OnXLRBError(LONG lError)
{
	LOG (ERR,"*** CBLRBCMsgHandler::OnBLRBEvent, Error Code ",lError," ***");

	{
		// Local Address를 얻어냄. - PMS에게 제공하기 위함.
		string sHostAddr, sLRBIP;
		GetHostAddress(sHostAddr, TRUE);
		LPSTR lpszLRBIP = GetLRBIP();

		PMSAWarningNtf msgNtf;
		if(lpszLRBIP != NULL)
		{
			msgNtf.m_sWarnMsg  = ::format("ELB is disconnected from LRB. [IP:%s] [LRBIP:%s]\n", sHostAddr.c_str(), lpszLRBIP);
		}
		else
		{
			msgNtf.m_sWarnMsg  = ::format("ELB is disconnected from LRB. [IP:%s] [LRBIP:??]\n", sHostAddr.c_str());
		}
		msgNtf.m_sTreatMsg = ::format("Check the LRB Servers \n");
		msgNtf.m_lErrLevel = FL_CRITICAL;
		msgNtf.m_unitID.m_dwSSN = 0;
		msgNtf.m_unitID.m_dwCategory = 0;
		msgNtf.m_unitID.m_dwGSID = thePMSConnector.GetGSID();
		PayloadHA pld(PayloadHA::msgPMSWarningNtf_Tag,msgNtf);

		thePMSConnector.SendMsg(pld);
	}

	//	if ((lError == ERROR_CONNECTIONCLOSE) )
	//	{
	Stop();	

	//	}
}
/*
enum REGISTRATIONANSINDEX
{
RDA_SUCCESS,
RDA_NTF,		// LRB에서 다른 서비스로 알리는 경우
RDA_RECONNECT,	// 잠시(5초?) 후에 다시 시도하라는 뜻
RDA_SYSTEM,
RDA_RECONNECTOTHER,		// 다른 LRB로 연결 요청을 하라는 뜻
RDA_FAIL,
RDA_ALREADY,			// 이미 등록된 Address
RDA_INVALIDROUTETYPE, 
RDA_LRB = 1000		// LRB 간의 replication용 메시지
};
*/
void CLRBHandler::OnXLRBRegister(LONG lErrorCode, LRBAddress& addr)
{

	LOG (INF, "*** LRBConnectorHandler::OnBLRBRegister, Error Code ",lErrorCode," ***");

	switch ( lErrorCode )
	{
	case RDA_SUCCESS:
		{
			if ( addr.GetCastType() == CASTTYPE_MULTICAST )
				m_bRegMultiAddr = TRUE;
			else if ( addr.GetCastType() == CASTTYPE_ANYCAST )
				m_bRegAnyAddr = TRUE;

			if (IsRegistered() )
				FinalInit();

			break;
		}
	case RDA_RECONNECT:						
	case RDA_RECONNECTOTHER:
	default:
		{
			Stop();							// shutdown 호출해주고
			Sleep(5000);					// 5초 후 재접속
			RegisterAddress();
		}
		break;

	}


	// register 된 주소 만큼 m_addrRegistered 에 추가한다 .
	// m_addrRegistered = addr;
}

void CLRBHandler::OnXLRBRcvMsg(const DWORD dwMID, const adl::LRBAddress& src, const adl::LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	// ELB가 쓰는 프로토콜은 PROTOCOL_LDTP 하나임
	if ( wProtocol != PROTOCOL_LDTP || wMessageType & MESSAGE_NACK)
	{
		string sSrc;
		string sDest;
		src.GetStringFormat(sSrc);
		dest.GetStringFormat(sDest);

		if ( wProtocol != PROTOCOL_LDTP )
			LOG (INF, "CLRBHandler::OnBLRBRcvMsg, type ",wMessageType,", protocol ",wProtocol,", src ",sSrc,", dest ",sDest);
		else
			LOG (INF, "CLRBHandler::OnBLRBRcvMsg - NACK message src ",sSrc,", dest",sDest);
		return;
	}

	BYTE cSvcCat = src.GetServiceCategory();   //src.addr[15];
	PayloadInfo pldInfo(dwMID, src);

	switch(cSvcCat)
	{
	case SVCCAT_GBS:
		OnRcvGBSMsg( pldInfo, buf );
		break;
	case SVCCAT_NLS:
		{
			if (wMessageType != MESSAGE_ACK)
			{
#ifdef _USENLS
				LOG (ERR, "CHS_CLRBHandler", "### CLrbHandler::OnXLRBRcvMsg - NLS Nack!!! dwCat = ", SVCCAT_NLS);
#endif
				return;
			}
			PayloadNLSCLI pld;
			VALIDATE(::BLoad(pld, buf));
			theNLSManager.RecvNLSMessage(&pld, src);
			break;
		}
	case SVCCAT_CHS:
		OnRcvCHSMsg( pldInfo, buf );
		break;
	case SVCCAT_NGS:
		OnRcvNGSMsg( pldInfo, buf);
		break;
	case SVCCAT_NAS:
		OnRcvNASMsg(pldInfo, buf);
		break;
	default:
		{
			string sSrc;
			string sDest;
			src.GetStringFormat(sSrc);
			dest.GetStringFormat(sDest);

			LOG	(ERR,  "--LRBConnectorHandler::OnBLRBRcvMsg - Unknown src ",sSrc,", dest ", sDest);		
		}
	}
}

void CLRBHandler::OnXLRBTerminateNtf(ListAddress& lstAddr)
{
	LOG (INF,  "*** LRBConnectorHandler::OnBLRBTerminateNtf, Size ",lstAddr.m_lstAddr.size()," ***");

	TLock lo(this);
	//Server Instance를 처리하는 Routine이 필요함
	ForEachElmt(LRBAddressList, lstAddr.m_lstAddr, i, j)
	{
		LRBAddress& addr = *i;
		OnSvrTerminateNtf(addr);
	}

}

void CLRBHandler::OnXLRBUnknownEvent(UINT pEvent, LONG lErrorCode, LPXBUF ppXBuf, LRBAddress& srcAddr, LRBAddress& destAddr)
{
	LOG (ERR, "*** CLRBHandler::OnBLRBUnknownEvent, Event Code ",pEvent,", Error Code ",lErrorCode," ***");
	//Stop();
}

void CLRBHandler::OnSendNCSStart()
{
	LOG (ERR, "*** CLRBHandler::OnSendNCSStart, with CHS , NGS ***");

	//to CHS
	MsgNCSCHS_RegisterServiceReq mCHSReq;
	PayloadNCSCHS pld(PayloadNCSCHS::msgRegisterServiceReq_Tag, mCHSReq);

	SendToCHS(pld, PayloadInfo(m_CHSMulticastAddr));

	//to NGS
	MsgNCSNGS_RegisterServiceReq mNGSReq;
	PayloadNCSNGS pld2(PayloadNCSNGS::msgRegisterServiceReq_Tag, mNGSReq);

	SendToNGS(pld2, PayloadInfo(m_NGSMulticastAddr));
}

/////////////////////////////////////////////////////////////////////////////////////
//from CHS
void CLRBHandler::OnRcvCHSMsg(const PayloadInfo& pldInfo, GBuf& buf) 
{
	PayloadCHSNCS pld;
	VALIDATE(::BLoad(pld, buf));

	switch(pld.mTagID)
	{
	case PayloadCHSNCS::msgRegisterServiceNtf_Tag:
		OnRegisterServiceNtf(pld.un.m_msgRegisterServiceNtf, pldInfo);
		break;
	case PayloadCHSNCS::msgRegisterServiceAns_Tag:
		OnRegisterServiceAns(pld.un.m_msgRegisterServiceAns, pldInfo);
		break;
	case PayloadCHSNCS::msgRChannelListReq_Tag:
		OnChannelListReq(pld.un.m_msgRChannelListReq, pldInfo);
		break;
	case PayloadCHSNCS::msgCHSInfoNtf_Tag:
		OnCHSInfoNtf(pld.un.m_msgCHSInfoNtf);
		break;
	case PayloadCHSNCS::msgNGSInfoReq_Tag:
		OnNGSInfoReq(pld.un.m_msgNGSInfoReq, pldInfo);
		break;
	case PayloadCHSNCS::msgNtfNFFriendAccept_Tag:
		OnNtfNFFriendAccept(pld.un.m_msgNtfNFFriendAccept, pldInfo);
		break;
	default:
		{
			LOG (ERR, "LRBConnector::OnRcvCHSMsg - Unknown message(Tag:",pld.mTagID,")");
		}
		break;
	}
}

//from NGS
void CLRBHandler::OnRcvNGSMsg(const PayloadInfo& pldInfo, GBuf& buf)
{
	PayloadNGSNCS pld;
	VALIDATE(::BLoad(pld, buf));

	switch(pld.mTagID)
	{
	case PayloadNGSNCS::msgRegisterServiceNtf_Tag:
		OnRegisterServiceNtf(pld.un.m_msgRegisterServiceNtf, pldInfo);
		break;
	case PayloadNGSNCS::msgRegisterServiceAns_Tag:
		OnRegisterServiceAns(pld.un.m_msgRegisterServiceAns, pldInfo);
		break;
	case PayloadNGSNCS::msgNGSInfoNtf_Tag:
		OnNGSInfoNtf(pld.un.m_msgNGSInfoNtf);
		break;
	case PayloadNGSNCS::msgNtfNFFriendAdd_Tag:
		OnNtfNFFriendAdd(pld.un.m_msgNtfNFFriendAdd);
		break;
	case PayloadNGSNCS::msgNtfNFFriendAccept_Tag:
		OnNtfNFFriendAccept(pld.un.m_msgNtfNFFriendAccept);
		break;
	case PayloadNGSNCS::msgNtfNFLetterReceive_Tag:
		OnNtfNFLetterReceive(pld.un.m_msgNtfNFLetterReceive);
		break;
	default:
		{
			LOG (ERR, "LRBConnector::OnRcvNGSMsg - Unknown message(Tag:",pld.mTagID,")");
		}
		break;
	}
}

// from GBS
void CLRBHandler::OnRcvGBSMsg(const PayloadInfo& pldInfo, GBuf& buf)
{
}

// from NAS
void CLRBHandler::OnRcvNASMsg(const PayloadInfo& pldInfo, GBuf& buf)
{
	PayloadNASCLI pld;
	VALIDATE(::BLoad(pld, buf));

	switch(pld.mTagID)
	{
	default:
		{
			LOG (ERR, "LRBConnector::OnRcvNASMsg - Unknown message(Tag:",pld.mTagID,")");
		}
		break;
	}
}

BOOL CLRBHandler::FinalInit(void)	//	여러번불릴수 있다..
{
	if ( m_bFICalled )
		return TRUE;

	m_bFICalled = TRUE;
	theManager.SetRegInitEvent();

	OnSendNCSStart();

	return 0;
}