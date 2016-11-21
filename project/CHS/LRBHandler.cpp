
#include "stdafx.h"

#include "Common.h"
#include "LRBHandler.h"
#include "ChannelDir.h"
#include "Control.h"
#include "Listener.h"
#include "StatisticsTable.h"
#include "Reporter.h"

#include <PMSConnObject.h>
#include <RankUtility.h>
#include <adl/MsgXLRBLCMP.h>
#ifdef _USE_STATMSG
#include "StatMsgMgr.h"
#endif

CLRBHandler theLRBHandler;
extern string GetParseData(string& sTarget, string sToken);

CLRBHandler::CLRBHandler(void)
{
	m_bRegistered = FALSE;

	m_bLRBStartup = FALSE;
	m_bIsFixedRoom = FALSE;
	Init();
}

CLRBHandler::~CLRBHandler(void)
{
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
		ASSERT(1);				break;
	case 'A':
		sApp = "ANYCAST";		break;
	//case 'B':
	//	sApp = "BROADCAST";		break;
	default:
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "Unknown CastType .. MakeAddress : CastType = ", dwCastType);
	}							

	switch(dwCategory)
	{
	case SVCCAT_NCS:
		sKey = "NCS";		break;
	case SVCCAT_NGS:
		sKey = "NGS";		break;
	case SVCCAT_CHS:
		sKey = "CHS";		break;
	case SVCCAT_AMS:
		sKey = "AMS";		break;
	case SVCCAT_NLS:
		sKey = "NLS";		break;
	case SVCCAT_BLS:
		sKey = "BLS";		break;
	case SVCCAT_IBB:
		sKey = "IBB";		break;
	case SVCCAT_RTRKS:
		sKey = "RRC";		break;
	case SVCCAT_NAS:
		sKey = "NAS";		break;
	default:
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "Unknown Category .. MakeAddress 1 : CastType = ", dwCategory);
		break;
	}

	string sID = "";
	if(dwCastType == BYTE('U'))
	{
		DWORD dwIP = GSocketUtil::GetHostIP();
		string strIP = GSocketUtil::GetHostIPString(); 
		sID = LRBAddress::GetIDFromIP(dwIP);
	}
	addr.SetAddress((BYTE)dwCastType, (BYTE)dwCategory, sID); 
}

BOOL CLRBHandler::Init()	// address 생성.
{
	MakeAddress(m_svrAddr, 'U', SVCCAT_CHS);

	// for send address
	MakeAddress(m_svcAddr[NCS_MULTI],	'M', SVCCAT_NCS);
	MakeAddress(m_svcAddr[NCS_ANY],		'A', SVCCAT_NCS);
	MakeAddress(m_svcAddr[NGS_ANY],		'A', SVCCAT_NGS);
	MakeAddress(m_svcAddr[NAS_MULTI],	'M', SVCCAT_NAS);
	MakeAddress(m_svcAddr[NAS_ANY],		'A', SVCCAT_NAS);

	MakeAddress(m_svcAddr[BLS_ANY],		'A', SVCCAT_BLS);
	MakeAddress(m_svcAddr[IBB_ANY],		'A', SVCCAT_IBB);
	MakeAddress(m_svcAddr[RKS_MULTI],	'M', SVCCAT_RTRKS);

	// for recv address
	MakeAddress(m_svcMyAddr[CHS_UNI],	'U', SVCCAT_CHS);
	MakeAddress(m_svcMyAddr[CHS_MUL],	'M', SVCCAT_CHS);
	MakeAddress(m_svcMyAddr[CHS_ANY],	'A', SVCCAT_CHS);

	//// for RankingServer
	memcpy(m_svcMyAddr[RRC_MUL].addr, "MRRC", strlen("MRRC"));

	//// for MDS
	memcpy(m_svcMyAddr[MDC_MUL].addr, "MMDC", strlen("MMDC"));

	LRBAddress lcsAddr;
	MakeAddress(lcsAddr, 'M', SVCCAT_NLS);
	SetNLSAddr(lcsAddr);


	// 혹시 INI파일에 Address주소가 있다면 ... Address 주소 읽어오기
	GetINIFileAddressInfo();

	return FALSE;
}

BOOL CLRBHandler::GetINIFileFixedRoom()
{
	char szFixedRoom[256] = {0x00};

	DWORD dwRet = ::GetPrivateProfileStringA("FIXED_ROOM", "FLAG", "0" , szFixedRoom, sizeof(szFixedRoom)/sizeof(char), theControl.m_confPath.GetConfPath());
	if (dwRet)
	{
		LONG lResult = atol(szFixedRoom);

		if (lResult)
			m_bIsFixedRoom = TRUE;
		
		LOG(INF_UK, "CHS_LRBHndl_Run"_LK, "*** Success to initialize GetINIFileFixedRoom : Type -> ", m_bIsFixedRoom);
	}
	else
		LOG(INF_UK, "CHS_LRBHndl_Run"_LK, "*** Failed to initialize GetINIFileFixedRoom");

	return TRUE;
}

BOOL CLRBHandler::GetINIFileAddressInfo()
{
	// 시간이 없어서 INI파일에서 읽어오는거 일단 보류
	/*	char szCastingType[256] = {0x00};
	char szAddrIndex[512] = {0x00};
	DWORD dwRet = ::GetPrivateProfileStringA("MULTICAST", NULL, NULL , szCastingType, sizeof(szCastingType)/sizeof(char), theControl.m_confPath.GetConfPath());
	DWORD dwRead = 0;

	while(dwRead < dwRet)
	{
		//GetPrivateProfileStringA("MULTICAST", "", "0", szAddrIndex, sizeof(szAddrIndex)/sizeof(char), theControl.m_confPath.GetConfPath());

		LONG lAddIndex = atol(szAddrIndex);
		// SSN을 리스트로 쫙 읽어온다.

		if (lAddIndex >= 0)
		{
			MakeAddress(m_svcMyAddr[lAddIndex], 'M', lAddIndex);
		}

		LONG lLength = strlen(pszIndex) + 1;
		dwRead += lLength;
		pszIndex += lLength;
	}*/
	return TRUE;
}

BOOL CLRBHandler::RunLRBHandler()
{
	TLock lo(this);
	if(!m_bLRBStartup)
	{
		BOOL bRet = ::XLRBConnectorInit(::GetLRBThreadPool());
		if (!bRet)
		{
			LOG(INF_UK, "CHS_LRBHndl_Run_Err"_LK, "*** Fail to initialize XLRBConnector ErrorCode is ", GetLastError());
			return FALSE;
		}
		m_bLRBStartup = TRUE;
	}

#ifdef _USE_STATMSG
	theStatMsgMgr.SetModulerValue(3);
	theStatMsgMgr.InitData(1000, 600, 2000, 1200, 120000);
#endif

	if(!RegisterAddress())
	{
		LOG(INF_UK, "CHS_LRBHndl_Run_Err"_LK, "!!!!!!!!!! -,.- CLRBHandler::RunLRBHandler() : FAIL !!!!!!!!!! ");
		return FALSE;
	}

	LOG(INF_UK, "CHS_LRBHndl_Run"_LK, "!!!!!!!!!! ^^ CLRBHandler::RunLRBHandler() : SUCCESS !!!!!!!!!! ");

//#ifdef _FIXED_ROOM_
	// 고정방의 경우 INI에서 읽어서 설정하고 새로운 GRID를 부여하지 않는다.
	GetINIFileFixedRoom();
//#endif

	return TRUE;
}

void CLRBHandler::StopLRBHandler()
{
	LOG(INF_UK, "CHS_LRBHndl_Stop"_LK, "Stop LRBHandler... ");
	::XLRBConnectorShutdown();
}

BOOL CLRBHandler::RegisterAddress(const LRBAddress& addr)
{   	
	if('U' == addr.GetCastType())
	{
		DWORD dwIP = GSocketUtil::GetHostIP();
		string sID = LRBAddress::GetIDFromIP(dwIP);
		string strAddr = std::string("U") + addr.ExtractServiceName() + sID;
		m_svrAddr.SetAddress(strAddr.c_str());
		for(int i=0; i<5; i++)
		{
			if(::XLRBConnectorStartup((const LPLRBADDRESS)&m_svrAddr, this)) 
			{
				return TRUE;
			}
			theLog.Put(ERR_UK, __FUNCTION__, "::Fail to Register UNICAST address, but try again ", i);
		}
		theLog.Put(ERR_UK, __FUNCTION__, "::Fail to Register UNICAST address");
		return FALSE;
	}
	else { 
		if(!::XLRBConnectorRegister((const LPLRBADDRESS)&addr, this))
		{
			theLog.Put(ERR_UK, __FUNCTION__, "::Fail to Register address");
			return FALSE;
		}
	}
	theLog.Put(INF_UK, "Success to  Register address ", addr.GetString());
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
			::Sleep(100);
			LOG(INF_UK, "CHS_CLRBHandler"_LK, "*** Retry Start up CLRBHandler Error = ", GetLastError());
			continue;
		}
		else 
			break;
	}
	if(!bRet)
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "*** Fail to Start up CLRBHandler Error = ", GetLastError());
		return FALSE;
	}

	// Service Instance의 Address를 등록
	// 등록 대상(받기)과 보내기 전용은 구분해
	for(int i = 0; i < MAX_MYADDR_CNT ; i++)	// broad casting은 지원하지 않음.
	{
		bRet = ::XLRBConnectorRegister(&m_svcMyAddr[i], this);
		Sleep(100);
		if (!bRet)
		{
			LOG(INF_UK, "CHS_CLRBHandler"_LK, "*** Fail to Register an Address [", i, "]type [", GetLastError(), "]err ***");
			continue;
		}
		else
			LOG(INF_UK, "CHS_CLRBHandler"_LK, "*** Succeed to Register an Address [", m_svcMyAddr[i].GetString(), "] *** ");

	}

	//.....................

	return bRet;
}

BOOL CLRBHandler::TryServiceRegistToNCS(NSAP & nsapCHS)
{
	TLock lo(this);

	CHSNCSRegister reg_info(nsapCHS, m_svrAddr, theChannelDir.m_lstChannelInfoForLBReg);
	PayloadCHSNCS pld(PayloadCHSNCS::msgRegisterServiceNtf_Tag, MsgCHSNCS_RegisterServiceNtf(reg_info));
	SendToAllNCS(pld);

	return TRUE;
}

BOOL CLRBHandler::SendChannelIDListToNGS()
{
	TLock	lo(this);
	
	MsgCHSNGS_ChannelIDList		msgChannelIDList;
	theChannelDir.OnChannelDListReq(-1, msgChannelIDList);  // SSN을 -1로 보내면 INI파일에서 읽어서 ChannelIDList를 찾음
	PayloadCHSNGS pld(PayloadCHSNGS::msgChannelIDList_Tag, MsgCHSNGS_ChannelIDList(msgChannelIDList));

	SendToAllPCCNGS(pld);

	return TRUE;
}


extern void StopCHSService();

///////////////////////////////////////////////////////////////////////////////////////////////
//// 
void CLRBHandler::OnXLRBError(LONG lError)
{
	LOG(INF_UK, "CHS_LRBHndl_OnXLRBError"_LK, "*** CXLRBCMsgHandler::OnXLRBEvent, Error Code : ", lError);

	{
		// Local Address를 얻어냄. - AMS에게 제공하기 위함.
		string sHostAddr, sLRBIP;
		GetHostAddress(sHostAddr, TRUE);
		LPSTR lpszLRBIP = GetLRBIP();

		PMSAWarningNtf msgNtf;
		if(lpszLRBIP != NULL)
		{
			msgNtf.m_sWarnMsg  = ::format("CHS is disconnected from LRB. [IP:%s] [LRBIP:%s]\n", sHostAddr.c_str(), lpszLRBIP);
		}
		else
		{
			msgNtf.m_sWarnMsg  = ::format("CHS is disconnected from LRB. [IP:%s] [LRBIP:??]\n", sHostAddr.c_str());
		}
		msgNtf.m_sTreatMsg = ::format("Check the LRB Servers \n");
		msgNtf.m_lErrLevel = FL_CRITICAL;
		msgNtf.m_unitID.m_dwSSN = 0;
		msgNtf.m_unitID.m_dwCategory = 0;


		thePMSConnector.SendWarningMsg(msgNtf.m_lErrLevel, msgNtf.m_sWarnMsg.c_str(), msgNtf.m_sTreatMsg.c_str(), 0, 0);
	}

	StopLRBHandler();	

	m_bLRBStartup = FALSE;
	StopCHSService();
}

void CLRBHandler::OnXLRBRegister(LONG lErrorCode, LRBAddress& addr)
{
	LOG(INF_UK, "CHS_LRBHndl_OnXLRBRegister, *** LRBConnectorHandler::OnXLRBRegister, Result Code = ", lErrorCode, ". addr=", addr.GetString() );

	switch ( lErrorCode )
	{
	case RDA_SUCCESS:
		m_bRegistered = TRUE;				// 두번 받아야 되지만 임시처리
		break;
	case RDA_RECONNECT:						
	case RDA_RECONNECTOTHER:
		{
			StopLRBHandler();							// shutdown 호출해주고 
			RegisterAddress();
		}
		break;

	}

	if (lErrorCode == RDA_SUCCESS && strncmp(addr.GetString().c_str(), "ACHS", strlen("ACHS")) == 0)
	{
		return;
	}
	if (lErrorCode == RDA_SUCCESS && strncmp(addr.GetString().c_str(), "MCHSVIP", strlen("MCHSVIP")) == 0)
	{
	}
	// register 된 주소 만큼 m_addrRegistered 에 추가한다 .
	// m_addrRegistered = addr;
}

void CLRBHandler::OnXLRBRcvMsg(const DWORD dwMID, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if(wProtocol != PROTOCOL_LDTP)
	{
		///////////////////////// for ECHO ////////////////////////
		if(wProtocol == PROTOCOL_LCMP)
		{
			PayloadLCMP pld;
			if(!pld.BLoad(buf))
			{
				LOG(INF_UK, "CHS_CLRBHandler"_LK, "*** LRBConnectorHandler::OnXLRBRcvMsg, BLoad Error !!!!!!!");
				return;
			}

			if( pld.mTagID == PayloadLCMP::msgEchoAns_Tag)
				LOG(INF_UK, "CHS_CLRBHandler"_LK , "^^ Echo Message Received!!!");
		}
		///////////////////////////////////////////////////////////
		else
			LOG(INF_UK, "CHS_CLRBHandler"_LK, "*** LRBConnectorHandler::OnXLRBRcvMsg, Message Type : ", wMessageType, ", Protocol : ", wProtocol, " ***");
		return;
	}

	BYTE btCat = src.GetServiceCategory();
	PayloadInfo pldInfo(dwMID, src);

	switch(btCat)
	{
	case SVCCAT_NCS : 
		OnRcvNCSMsg( pldInfo, buf, wMessageType, wProtocol );
		break;
	case SVCCAT_NGS : 
		OnRcvNGSMsg( pldInfo, buf, wMessageType, wProtocol );
		break;
	case SVCCAT_IBB :
		OnRcvIBBMsg( pldInfo, buf, wMessageType, wProtocol );
		break;
	case SVCCAT_RTRKS :
		break;
	case SVCCAT_ODBGW :
		break;
	case SVCCAT_BMS :
		break;
	case SVCCAT_BCS :
		break;
	case SVCCAT_NAS:
		OnRcvNASMsg(pldInfo, buf, wMessageType, wProtocol);
		break;
#ifdef _CHSNLS
	case SVCCAT_NLS :
		{
			if (wMessageType != MESSAGE_ACK)
			{
				LOG(INF_UK, "CHS_CLRBHandler"_LK, "CLrbHandler::OnXLRBRcvMsg - Nack!!! dwCat = ", SVCCAT_NLS);
				TLOG0("CLrbHandler::OnXLRBRcvMsg - Nack!!!\n");
				return;
			}
			PayloadNLSCLI pld;
			VALIDATE(::BLoad(pld, buf));
			theNLSManager.RecvNLSMessage(&pld, src);
			break;
		}
#endif
	default:
		{
			string logstr = format("Unknown service type..source[%s], dest[%s] ", src.GetString().c_str(), dest.GetString().c_str());
			LOG(INF_UK, "CHS_CLRBHandler"_LK, logstr);
		}
	}
}

void CLRBHandler::OnXLRBTerminateNtf(ListAddress& lstAddr)
{
	LOG(INF_UK, "CHS_LRBHndl_OnXLRBTerminateNtf"_LK, "*** LRBConnectorHandler::OnXLRBTerminateNtf, Size = ", lstAddr.m_lstAddr.size());

	//Server Instance를 처리하는 Routine이 필요함
	ForEachElmt(LRBAddressList, lstAddr.m_lstAddr, i, j)
	{
		LRBAddress& addr = *i;
		string sAddr;
		addr.GetStringFormat(sAddr);
	
		LOG(INF_UK, "CHS_CLRBHandler"_LK, " Terminated Service LRBAddress = ", sAddr);
			
		DWORD dwCat = addr.GetServiceCategory();
		if (dwCat == SVCCAT_NGS)
		{
			theChannelDir.RemGLSLogicalAddr(addr);
		}
		else if(dwCat == SVCCAT_NLS)
		{
			theNLSManager.NLSTerminateNtf(addr);
		}
		else if(dwCat == SVCCAT_BMS)
		{
		}
	}
}

void CLRBHandler::OnXLRBUnknownEvent(UINT pEvent, LONG lErrorCode, LPXBUF ppXBuf, LRBAddress& srcAddr, LRBAddress& destAddr)
{
	LOG(INF_UK, "CHS_LRBHndl_OnXLRBUnknownEvnt"_LK, "*** LRBConnectorHandler::OnXLRBUnknownEvent, Event Code : ", pEvent, " , Error Code : ", lErrorCode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// message 
void CLRBHandler::OnRcvNGSMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if(wMessageType & MESSAGE_NACK)
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "receive NACK message from NGS");
		return;
	}
	OnRcvNGS(dest, buf, wMessageType, wProtocol);
}

void CLRBHandler::OnRcvNCSMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if(wMessageType & MESSAGE_NACK)
	{
		OnRecvNCS_NACK(buf);
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "receive NACK message from NCS");
		return;
	}

	PayloadNCSCHS pld;
	if(!::BLoad(pld, buf))
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, " !!!!!!!! OnRcvNCSMsg : BLoad Error");
		return;
	}

	switch(pld.mTagID) {
	case PayloadNCSCHS::msgRegisterServiceReq_Tag:
		OnRegisterServiceReq(pld.un.m_msgRegisterServiceReq, dest);
		break;
	case PayloadNCSCHS::msgRChannelListAns_Tag:
		OnRChannelListAns(pld.un.m_msgRChannelListAns);
		// count message
		m_MsgCount.m_dwRCList++;
		break;
	case PayloadNCSCHS::msgNGSInfoAns_Tag:
		OnGLSInfoAns(pld.un.m_msgNGSInfoAns);
		break;
	case PayloadNCSCHS::msgNtfNFFriendAdd_Tag:
		OnNtfNFFriendAdd(pld.un.m_msgNtfNFFriendAdd);
		break;
	case PayloadNCSCHS::msgNtfNFFriendAccept_Tag:
		OnNtfNFFriendAccept(pld.un.m_msgNtfNFFriendAccept);
		break;
	case PayloadNCSCHS::msgNtfNFLetterReceive_Tag:
		OnNtfNFLetterReceive(pld.un.m_msgNtfNFLetterReceive);
		break;
	default:
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "+++++ Received Unknown message from NCS +++++ ");
		break;
	}
}

void CLRBHandler::OnRcvIBBMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol) 
{
	if(wMessageType & MESSAGE_NACK)
	{
		OnRecvIBB_NACK(buf);
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "receive NACK message from IBB - rcv msg");
		return;
	}
}

void CLRBHandler::OnRcvNASMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if(wMessageType & MESSAGE_NACK)
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "receive NACK message from NAS");
		return;
	}

	PayloadNASCLI pld;
	if(!::BLoad(pld, buf))
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, " !!!!!!!! OnRcvNASMsg : BLoad Error");
		return;
	}

	switch(pld.mTagID)
	{
	default:
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "+++++ Received Unknown message from NAS +++++ ");
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FromNCS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLRBHandler::OnRChannelListAns(MsgNCSCHS_RChannelListAns *pPld)
{	
	ASSERT(pPld);
	if(!pPld)
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "+++++ ASSERT :OnRChannelListAns +++++");
		return;
	}
	
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(pPld->m_channelID, &spChannel))
		return;

	PayloadCHSCli msg(PayloadCHSCli::msgChannelListAns_Tag,	
		MsgCHSCli_ChannelListAns(
			pPld->m_lstChannelInfoList, 
			pPld->m_lTotalUserCount, 
			pPld->m_lTotalChannelCount, 
			pPld->m_lIsAllCategory
		)
	);
	GBuf buf;
	::LStore(buf, msg);

	spChannel->PostChannelListAns(pPld->m_lCSN, buf);
}

void CLRBHandler::OnGLSInfoAns(MsgNCSCHS_NGSInfoAns *pPld)
{
	ASSERT(pPld);
	if(!pPld)
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "+++++ ASSERT :OnGLSInfoAns +++++");
		return;
	}

	if(pPld->m_lType == CONNECTTYPE_DIRECT)	
	{
		RCPtrT<CChannel> spChannel;
		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
		{
			DWORD dwNewGRIID = spChannel->GetRoomID();

			PayloadCHSCli msg1(PayloadCHSCli::msgGRDirectCreateAns_Tag, 
				MsgCHSCli_GRDirectCreateAns(pPld->m_nsapNGS, dwNewGRIID, m_svrAddr, _X(" "), _X(" "), pPld->m_lErrorCode));

			GBuf buf; 
			::LStore(buf, msg1);
			spChannel->PostGRDirectCreateAns(pPld->m_lCSN, buf);
			return;
		}
	}
	else if(pPld->m_lType == CONNECTTYPE_NORMAL)
	{
		RCPtrT<CChannel> spChannel;
		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
		{
			DWORD dwNewGRIID = spChannel->GetRoomID();

			PayloadCHSCli msg1(PayloadCHSCli::msgNGSInfoAns_Tag, 
				MsgCHSCli_GLSInfoAns(pPld->m_nsapNGS, dwNewGRIID, m_svrAddr, pPld->m_lErrorCode));

			GBuf buf; 
			::LStore(buf, msg1);
			spChannel->PostGLSInfoAns(pPld->m_lCSN, buf);
			return;
		}
	}
	else if(pPld->m_lType == CONNECTTYPE_FREEMODEROOM)
	{
		RCPtrT<CChannel> spChannel;
		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
		{
			DWORD dwNewGRIID = spChannel->GetRoomIDbyFreeRoom();
			LOG(ERR_UK, "CHS_CLRBHandler"_LK, "OnGLSInfoAns : GRIID", dwNewGRIID);

			PayloadCHSCli msg1(PayloadCHSCli::msgNGSInfoAns_Tag, 
				MsgCHSCli_GLSInfoAns(pPld->m_nsapNGS, dwNewGRIID, m_svrAddr, pPld->m_lErrorCode));

			GBuf buf; 
			::LStore(buf, msg1);
			spChannel->PostGLSInfoAns(pPld->m_lCSN, buf);
			return;
		}
	}
	else
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "++++++ UnKnown LB->CHS GLSInfoAns Type =====");
}

void CLRBHandler::OnRegisterServiceReq(MsgNCSCHS_RegisterServiceReq * pPld, const PayloadInfo & dest)
{
	TLock lo(this);
	CHSNCSRegister reg_info(theChannelDir.GetCHSNsap(), m_svrAddr, theChannelDir.m_lstChannelInfoForLBReg);
	PayloadCHSNCS pld(PayloadCHSNCS::msgRegisterServiceAns_Tag, MsgCHSNCS_RegisterServiceAns(reg_info));

	SendToNCS(pld, dest);// SendToUniLB(..)

	// Boot State 설정.
	theReporter.SetAllDiffFlag();
	LOG(INF_UK, "CHS_CLRBHandler"_LK, "OnRegisterServiceReq : Listener Run by LB");
	//Channel List를 전송.. by report signaling.. 
}

void CLRBHandler::OnNtfNFFriendAdd(MsgNCSCHS_NtfNFFriendAdd* pPld)
{
	TLock lo(this);

	RCPtrT<CChannel> spChannel;
	if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
	{
		MsgCHSCli_NtfNFFriendAdd ntf;
		ntf.m_strCharName = pPld->m_strSender;

		GBuf buf;
		::LStore(buf, ntf);
		spChannel->PostNtfNFFriendAdd(pPld->m_lReceiverCSN, buf);
	}
}

void CLRBHandler::OnNtfNFFriendAccept(MsgNCSCHS_NtfNFFriendAccept* pPld)
{
	TLock lo(this);

	RCPtrT<CChannel> spChannel;
	if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
	{
		MsgCHSCli_NtfNFFriendAccept ntf;
		ntf.m_nfFriend = pPld->m_nfFriend;

		GBuf buf;
		::LStore(buf, ntf);
		spChannel->PostNtfNFFriendAccept(pPld->m_lReceiverCSN, buf);
	}
}

void CLRBHandler::OnNtfNFLetterReceive(MsgNCSCHS_NtfNFLetterReceive* pPld)
{
	TLock lo(this);

	RCPtrT<CChannel> spChannel;
	if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
	{
		spChannel->PostNtfNFLetterReceive(pPld->m_lReceiverCSN);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// send message
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLRBHandler::ConnectorSendTo(GBuf& buf, WORD wProtocol, LRBAddress& SrcAddr, const PayloadInfo& DestAddr)
{
	::XLRBConnectorAsyncSendTo(DestAddr.dwMID, buf, wProtocol, &SrcAddr, (LRBAddress*)&(DestAddr.addr));
}


// ------------------------------------ GLS ------------------------------------ //
void CLRBHandler::SendToNGS(const PayloadCHSNGS& pld, const PayloadInfo& dest)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSGLSMsg BStore FAIL--\n");
		return;
	}
	if(dest.addr.IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToGLS"_LK, "!!! Destination LRBAddress is NULL ::: ", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, dest ) ;	
}

void CLRBHandler::SendToPCCNGS(const PayloadCHSNGS& pld, const PayloadInfo& dest)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSPCCGLSMsg BStore FAIL--\n");
		return;
	}
	if(dest.addr.IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToPCCGLS"_LK, "!!! Destination LRBAddress is NULL ::: ", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, dest ) ;
}

void CLRBHandler::SendToAllPCCNGS(const PayloadCHSNGS& pld)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSPCCGLSMsg BStore FAIL--\n");
		return;
	}
	if(m_svcAddr[PCG_MULTI].IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToAllPCCGLS"_LK, "!!! Destination LRBAddress is NULL ::: ", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, m_svcAddr[PCG_MULTI] ) ;
}

// ----------------------------------------------------------------------------- //



// ------------------------------------ NCS ------------------------------------ //
void CLRBHandler::SendToNCS(const PayloadCHSNCS& pld, const PayloadInfo& dest)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSLBMsg BStore FAIL--\n");
		return;
	}
	if(dest.addr.IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToELB"_LK, "!!! Destination LRBAddress is NULL :::", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, dest ) ;
}

void CLRBHandler::SendToNCS(const PayloadCHSNCS& pld)		// any casting 
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSLBMsg BStore FAIL--\n");
		return;
	}
	if(m_svcAddr[NCS_ANY].IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToELB"_LK, "!!! Destination LRBAddress is NULL :::", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, m_svcAddr[NCS_ANY] ) ;
}

void CLRBHandler::SendToAllNCS(const PayloadCHSNCS& pld)	
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSLBMsg BStore FAIL--\n");
		return;
	}
	if(m_svcAddr[NCS_MULTI].IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToAllELB"_LK, "!!! Destination LRBAddress is NULL :::", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, m_svcAddr[NCS_MULTI] ) ;	
}
// ----------------------------------------------------------------------------- //


void CLRBHandler::SendToNAS(const PayloadCLINAS& pld, const PayloadInfo& dest)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSLBMsg BStore FAIL--\n");
		return;
	}
	if(dest.addr.IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToNAS"_LK, "!!! Destination LRBAddress is NULL :::", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, dest );
}

void CLRBHandler::SendToNAS(const PayloadCLINAS& pld) // any casting
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSLBMsg BStore FAIL--\n");
		return;
	}
	if(m_svcAddr[NAS_ANY].IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToNAS"_LK, "!!! Destination LRBAddress is NULL :::", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, m_svcAddr[NAS_ANY] ) ;
}

void CLRBHandler::SendToAllNAS(const PayloadCLINAS& pld)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		TLOG0("--Error CHSLBMsg BStore FAIL--\n");
		return;
	}
	if(m_svcAddr[NAS_MULTI].IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToAllNAS"_LK, "!!! Destination LRBAddress is NULL :::", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, m_svcAddr[NAS_MULTI] ) ;
}


// ------------------------------------ RKS ------------------------------------ //
void CLRBHandler::SendToRKS(const LRBAddress& src, const LRBAddress& dest, GBuf& buf)
{
	::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
}

// ------------------------------------ ODBGW ------------------------------------ // 하는일이 없으므로 위의 SendToKRS와 합쳐서 SendToServer 로 해도 될 듯.
void CLRBHandler::SendToODBGW(const LRBAddress& dest, GBuf& buf)
{
	LRBAddress src = GetMyAddress();
	::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
}

void CLRBHandler::SendToBMS(const LRBAddress& dest, GBuf& buf)
{
	LRBAddress src = GetMyAddress();
	::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
}

// ------------------------------------ LCS ------------------------------------ //
void CLRBHandler::SendToNLS(const PayloadCLINLS& pld, const PayloadInfo& dest)
{
	GBuf buf;
	BOOL bRet = ::BStore(buf, pld);
	if (!bRet) {
		return;
	}
	if(dest.addr.IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToNLS"_LK, "!!! Destination LRBAddress is NULL :::", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, dest ) ;
}

void CLRBHandler::SendToNLS(const LRBAddress& src, const PayloadInfo& dest, PayloadCLINLS& pld)
{
	GBuf buf;
	BOOL bRet = ::BStore(buf, pld);
	if (!bRet) {
		return;
	}
	if(dest.addr.IsNULL())
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToNLS"_LK, "!!! Destination LRBAddress is NULL :::", __FILE__, ":", __LINE__);
		return;
	}
	ConnectorSendTo( buf, PROTOCOL_LDTP, m_svrAddr, dest ) ;	
}
// ----------------------------------------------------------------------------- //


void CLRBHandler::SendToEchoMsg()
{
	MsgEchoReq msgEchoReq(1);
	PayloadLCMP pldlcmp(PayloadLCMP::msgEchoReq_Tag, msgEchoReq);
	GBuf buf1; 
	if(!pldlcmp.BStore(buf1))
	{
		LOG(INF_UK, "CHS_CLRBHandler_SendToEchoMsg"_LK, "CLRBHandler::SendToEchoMsg BStore Error!!!");
		return;
	}
	ConnectorSendTo( buf1, PROTOCOL_LCMP, m_svrAddr, m_svrAddr );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// from LB NACK
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLRBHandler::OnRecvIBB_NACK(GBuf & buf)
{
	theControl.ResetIBBRunFlag();
	LOG(INF_UK, "CHS_LRBHndl_OnRcvIBB_NACK"_LK, " NACK :  Received NACK message from IBB ");
}


void CLRBHandler::OnRecvNCS_NACK(GBuf & buf)//const MsgService * pMsg)
{
	PayloadCHSNCS pld;
	VALIDATE(::BLoad(pld, buf));

	switch(pld.mTagID) {
	case PayloadCHSNCS::msgRegisterServiceNtf_Tag:
		OnRegisterServiceNtf_NACK(pld.un.m_msgRegisterServiceNtf);
		break;
	case PayloadCHSNCS::msgRChannelListReq_Tag:
		OnRChannelListReq_NACK(pld.un.m_msgRChannelListReq);
		break;
	case PayloadCHSNCS::msgNGSInfoReq_Tag:
		OnNGSInfoReq_NACK(pld.un.m_msgNGSInfoReq);
		break;
	default:
		LOG(INF_UK, "CHS_CLRBHandler_OnRcvLB_NACK"_LK, " NACK :  Received Unknown message from LB  ");
	}
}

void CLRBHandler::OnRChannelListReq_NACK(MsgCHSNCS_RChannelListReq *pPld)
{	
	if(!pPld)
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "+++++ ASSERT : NACK : OnRChannelListAns +++++");
		return;
	}
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(pPld->m_channelID, &spChannel))
		return;

	ChannelInfoList lstChannel;

	PayloadCHSCli msg(PayloadCHSCli::msgChannelListAns_Tag,	
		MsgCHSCli_ChannelListAns(
			lstChannel, 
			0L, 
			0L, 
			CHS_UNKNOWN
		)
	);
	GBuf buf;
	::LStore(buf, msg);

	spChannel->PostChannelListAns(pPld->m_lCSN, buf);
}

void CLRBHandler::OnNGSInfoReq_NACK(MsgCHSNCS_NGSInfoReq *pPld)
{
	ASSERT(pPld);
	if(!pPld)
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "+++++ ASSERT : NACK : OnGLSInfoAns +++++");
		return;
	}
	if(pPld->m_lType == CONNECTTYPE_DIRECT)	
	{
		RCPtrT<CChannel> spChannel;
		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
		{
			NSAP nsapGLS;
			PayloadCHSCli msg1(PayloadCHSCli::msgGRDirectCreateAns_Tag, 
				MsgCHSCli_GRDirectCreateAns(nsapGLS, 0UL, m_svrAddr, _X(" "), _X(" "), CHS_UNKNOWN));

			GBuf buf; 
			::LStore(buf, msg1);
			spChannel->PostGRDirectCreateAns(pPld->m_lCSN, buf);
			
			LOG(INF_UK, "CHS_CLRBHandler"_LK, " NACK : Send CHS->Cli GRdirectCreateReq ");
			return;
		}

	}
	else if(pPld->m_lType == CONNECTTYPE_NORMAL)
	{
		RCPtrT<CChannel> spChannel;
		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
		{
			NSAP nsapGLS;
			PayloadCHSCli msg1(PayloadCHSCli::msgNGSInfoAns_Tag, 
				MsgCHSCli_GLSInfoAns(nsapGLS, 0UL, m_svrAddr, CHS_UNKNOWN));

			GBuf buf; 
			::LStore(buf, msg1);
			spChannel->PostGRDirectCreateAns(pPld->m_lCSN, buf);
			
			LOG(INF_UK, "CHS_CLRBHandler"_LK, " NACK : Send CHS->Cli NGSInfoReq ");
			return;
		}
	}
	else 
		LOG(INF_UK, "CHS_CLRBHandler"_LK, " NACK : UnKnown LB->CHS GLSInfoAns Type ");
}

void CLRBHandler::OnRegisterServiceNtf_NACK(MsgCHSNCS_RegisterServiceNtf * pPld)
{
	// 여기서는 boot Sequence가 바뀐다..
	if(theControl.GetBootState() != CHS_REGING_LB)
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, "NACK : Invalid OnRegisterServiceNtf_NACK");
		return;
	}
	//
	//LB가 몇개인지 알지 못함, 
	theControl.SetBootState(CHS_REG_LB);
	LOG(INF_UK, "CHS_CLRBHandler"_LK, " NACK : OnRegisterServiceNtf_NACK ");
}
