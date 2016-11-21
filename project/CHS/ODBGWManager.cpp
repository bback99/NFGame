#include "stdafx.h"
#include "ODBGWManager.h"
#include "LRBHandler.h"
#include "Control.h"
#include "ChannelDir.h"
#include "Common.h"
#include "LRBHandler.h"
#ifdef _JACKPOT_INDICATION_
#include "JackpotManager.h"
#endif
#define theLRBManager theLRBHandler

CODBGWManager theODBGWMgr;


#define ODBGWQUERY_VIPROOMLIST	0x01

CODBGWManager::CODBGWManager() : m_bJackpotFlag(FALSE)
{
}

CODBGWManager::~CODBGWManager()
{
}

BOOL CODBGWManager::SendRegisterServiceReq(const LRBAddress& addrODBGW)
{
	MsgCliODBGW_RegisterServiceReq req;
	PayloadCliODBGW pld(PayloadCliODBGW::msgRegisterServiceReq_Tag, req);
	if(FALSE == theLRBHandler.AsyncSend(pld, &addrODBGW))
	{
		LOG(ERR_UK, __FUNCTION__, "Fail to AsyncSend");
		return FALSE;
	}
	
	return TRUE;
}

#ifdef _JACKPOT_INDICATION_
BOOL CODBGWManager::SendSelectJackpotMoney()
#else
BOOL CODBGWManager::SendSelectJackpotMoney(LONG lSSN)
#endif
{
	adl::LRBAddress dest;
	dest.SetAddress("AODBJACKPOT");

	MsgCliODBGW_ExecutePlainQueryReq req;
	req.m_needAnswer = TRUE;
#ifdef _JACKPOT_INDICATION_
	req.m_strQuery = ::format("SELECT ssn, jackpotmoney FROM jackpot_totalmoney");
	req.m_bufKey = "JACKPOTMONEY";
#else
	req.m_strQuery = ::format("SELECT ssn, jackpotmoney FROM jackpot_totalmoney WHERE ssn = %d", lSSN);
	req.m_bufKey = "JACKPOT";
#endif
	PayloadCliODBGW pld(PayloadCliODBGW::msgPlainReq_Tag, req);
	if(FALSE == theLRBHandler.AsyncSend(pld, &dest))
	{
		LOG(ERR_UK, __FUNCTION__, " Fail to AsyncSend");
		return FALSE;
	}
	
	return TRUE;
}

void CODBGWManager::OnLRBRegister(const LRBAddress &addr)
{
	LONG lSSN;
	sscanf(addr.GetString().c_str(), "MCHSVIP%d", &lSSN);

	string strQueryID = ::format("SP_SELECT_VIP_ROOM_INFO");
	
	//AODBVIPSSN 로 쿼리를 전송한다.
	MsgCliODBGW_ExecuteBindQueryReq msg;
	msg.m_needAnswer	= TRUE;
	msg.m_bufKey		= ::format("VIP|%u|%u", (LONG)ODBGWQUERY_VIPROOMLIST,(LONG)lSSN);
	msg.m_strQueryID	= strQueryID;

	msg.m_vecParams.clear();
	msg.m_vecParams.push_back(::format("%u",lSSN));									// SSN

	PayloadCliODBGW pld(PayloadCliODBGW::msgBindReq_Tag, msg);
	string strODBGW = ::format("AODBVIP%d",lSSN);
	LRBAddress addrAODBGW;
	addrAODBGW.SetAddress(strODBGW.c_str());

	GBuf buf;	
	if (pld.BStore(buf) == FALSE)
	{
		LOG(INF_UK, "CHS_BStoreError,CODBGWManager::OnLRBRegister(). addrAODBGW=", strODBGW);
		 return;
	}

	theLRBHandler.SendToODBGW(addrAODBGW, buf);

	return;
}

void CODBGWManager::OnRegisterServiceNtf(const LRBAddress &src, const LRBAddress &dst)
{
	TLock lo(this);
	if (strstr(dst.GetString().c_str(), "MCHSRMM"))
	{
		LONG lSSN;
		sscanf(dst.GetString().c_str(), "MCHSRMM%d", &lSSN);
		m_setRMMSSN.insert(lSSN);
	}
	else if (strstr(dst.GetString().c_str(), "MCHSVIP"))
	{
		LONG lSSN;
		sscanf(dst.GetString().c_str(), "MCHSVIP%d", &lSSN);
		m_setVIPSSN.insert(lSSN);
	}
	else if(strstr(dst.GetString().c_str(), "MCHSJACKPOT"))
	{
		SetJackpotServiceOn(TRUE);
	}
}

void CODBGWManager::OnRegisterServiceAns(const LRBAddress &src, const LRBAddress &dst)
{	
	//TLock(this);
	TLock lo(this);
	if (strstr(src.GetString().c_str(), "AODBRMM"))
	{
		LONG lSSN;
		sscanf(src.GetString().c_str(), "AODBRMM%d", &lSSN);
		m_setRMMSSN.insert(lSSN);
	}
   	else if (strstr(src.GetString().c_str(), "AODBVIP"))
	{
		LONG lSSN;
		sscanf(src.GetString().c_str(), "AODBVIP%d", &lSSN);
		m_setVIPSSN.insert(lSSN);
	}
	else if(strstr(src.GetString().c_str(), "AODBJACKPOT"))
	{
		SetJackpotServiceOn(TRUE);
	}
}



void CODBGWManager::OnQueryAns(const LRBAddress &src, const LRBAddress &dst, MsgODBGWCli_QueryAns *pAns)
{
	if (pAns->m_lErrCode != 0)
	{
		// 로그 남긴다.
		LOG(WAR_UK, " ", __FUNCTION__, " ODBGW Error Answer, lErrCode:", pAns->m_lErrCode);
	}

	if (strncmp(pAns->m_bufKey.c_str(), "VIP", strlen("VIP")) == 0 )
	{
		// Type, SSN 을 알아낸다.
		LONG lType = -1, lSSN = -1;
		::sscanf(pAns->m_bufKey.c_str(), "VIP|%d|%d", &lType, &lSSN);

		switch(lType)
		{
		case ODBGWQUERY_VIPROOMLIST:
			{	
				UpdateVIPList(lSSN, pAns->m_vecResult);			
			}
			break;
		default: // 에러로그 남기시오.	
			break;
		}
	}
	else if (strncmp(pAns->m_bufKey.c_str(), "RMM", strlen("RMM")) == 0 )
	{
		// 체널 아이디, USN 을 알아낸다.
		ChannelID cid;
		LONG lUSN;
		
		DWORD dwSendTime;
		::sscanf(pAns->m_bufKey.c_str(), "RMM|%d|%u|%u|%d|%u", &cid.m_lSSN, &cid.m_dwCategory, &cid.m_dwGCIID, &lUSN, &dwSendTime);

		DWORD dwNow = ::GetTickCount();
		if (5*1000 < dwNow - dwSendTime) // 5초 이상인 경우 Time 로그를 남긴다.
			LOG(WAR_UK, "CLRBHandler::OnQueryAns() Too late RMM Select Answer : Time:", dwNow - dwSendTime, "(ms), USN:", lUSN);

		CChannelPtr spChannel;
		if(!theChannelDir.GetChannel(cid, &spChannel))
		{	
			LOG(WAR_UK, "CHS_NotFoundChannel,CLRBHandler::OnQueryAns() GetChannel fail ChannelID : [", cid.m_dwGCIID , " ], USN:", lUSN);			
			return; // 에러로그 남기시오.
		}

		MsgCHSCli_MatchedRoomListAns *pToCliAns = new MsgCHSCli_MatchedRoomListAns;
		if (pAns->m_lErrCode != 0)
		{
			pToCliAns->m_lErrorCode = -1; // 이렇게 하지 말고 에러 코드 정의하시오.
			::XsigQueueSignal(::GetChannelThreadPool(), spChannel, CHANNELSIGNAL_RMMANS, (WPARAM)lUSN, (LPARAM)pToCliAns);
			return;	
		}

		pToCliAns->m_lErrorCode = 0x00;

		// SELECT ROOMID, GLSIP, GLSPORT
		vector<string> &vecResult = pAns->m_vecResult;
		int nRoomNum = vecResult.size() / 4;

//		theLog.Put(ERR_UK, "Size : ", vecResult.size(), " : ", nRoomNum, "ERR : ", pAns->m_lErrCode);

		for (int i = 0; i < nRoomNum; i++)
		{
			MatchRoomInfo room;
            room.m_roomID				= Str64ToRoomID(vecResult[i*4]);			
			sscanf(vecResult[i*4 + 1].c_str(), "%u", &room.m_nsapGLS.m_dwIP);
			sscanf(vecResult[i*4 + 2].c_str(), "%u", &room.m_nsapGLS.m_dwPort);			
			room.m_strGameOption = vecResult[i*4 + 3];

			pToCliAns->m_vecRoom.push_back(room);

			LOG(DEV_UK, "DirectRoomMatchMaking::RoomID:",vecResult[i*4],",GLS IP:", room.m_nsapGLS.m_dwIP, ", GLS PORT:", room.m_nsapGLS.m_dwPort, ",Game Option:", room.m_strGameOption);
		}
		::XsigQueueSignal(::GetChannelThreadPool(), spChannel, CHANNELSIGNAL_RMMANS, (WPARAM)lUSN, (LPARAM)pToCliAns);
	}
#ifdef _JACKPOT_INDICATION_
	else if (string("JACKPOTMONEY") == pAns->m_bufKey)
	{
		OnRcvSelectJackpotTotalMoneyAns(pAns);
	}
#else
	else if (0 == strncmp(pAns->m_bufKey.c_str(), "JACKPOT", strlen("JACKPOT")))
	{
		if(2 == pAns->m_vecResult.size())
		{
			LONG lSSN = atoi(pAns->m_vecResult[0].c_str());
			INT64 llJackpotMoney = _atoi64(pAns->m_vecResult[1].c_str());
			theChannelDir.GetJackpotMoneyData().SetJackpotMoney(lSSN, llJackpotMoney);
		}
		else
			theLog.Put(DEV_UK, __FUNCTION__, "Receving query answer for jackpot is \"not\" ok");
	}
#endif
#ifdef _HOPE_JACKPOT_
	else if (0 == ::strncmp(pAns->m_bufKey.c_str(), "HOPEJACKPOT_SELECTINFO", ::strlen("HOPEJACKPOT_SELECTINFO")))
	{
		if (4 != pAns->m_vecResult.size())
		{
			theLog.Put(WAR_UK, "CHS_HopeJackpot, CODBGWManager::OnQueryAns() - HOPEJACKPOT_SELECTINFO. Wrong Parameter. Size: ", pAns->m_vecResult.size());
			return;
		}
		int nDisplayType = ::atoi(pAns->m_vecResult[0].c_str());
		LONG lTotalWinningMoney = ::atoi(pAns->m_vecResult[1].c_str());
		LONG lTotalJackpotMoney = ::atoi(pAns->m_vecResult[2].c_str());
		int nTodayRemainWinner = ::atoi(pAns->m_vecResult[3].c_str());
		theHopeJackpotManager.SetJackpotInfo(nDisplayType, lTotalWinningMoney, lTotalJackpotMoney, nTodayRemainWinner);
		theChannelDir.SendHopeJackpotInfoNtf();
		theLog.Put(INF_UK, "CHS_HopeJackpot, CODBGWManager::OnQueryAns() - HopeJackpot Select Info. Info: (", nDisplayType, ",", lTotalWinningMoney, ",", lTotalJackpotMoney, ",", nTodayRemainWinner, ")");
	}
#endif	// _HOPE_JACKPOT_
}

void CODBGWManager::OnChangedNtf(const LRBAddress &src, const LRBAddress &dst, MsgODBGWCli_ChangedNtf *pNtf)
{
	if("SP_SELECT_VIP_ROOM_INFO" == pNtf->m_strNotiID)
	{			
		// SSN을 알아낸다.
		LONG lSSN;
		sscanf(dst.GetString().c_str(), "MCHSVIP%d", &lSSN);
		theLog.Put(DEV_UK, "CODBGWManager::OnVIPChangeNtf(), src=", src.GetString(), ", dst=", dst.GetString());

		UpdateVIPList(lSSN, pNtf->m_vecResult);
		return;
	}
#ifdef _JACKPOT_INDICATION_
	else if("SP_NOTIFY_JACKPOT_INDICATION" == pNtf->m_strNotiID)
	{
		if(1 != pNtf->m_vecResult.size())
		{
			LOG(ERR, "Game Jackpot Indication Message Return Parameter Error");
			return;
		}

		LONG lSSN = ::atoi(pNtf->m_vecResult[0].c_str());
		theChannelDir.SendJackpotIndication(lSSN);
		return;
	}
#endif
#ifdef _HOPE_JACKPOT_
	// Jackpot 초기화
	else if ("SP_HOPEJACKPOT_NOTIFY_INIT" == pNtf->m_strNotiID)
	{
		if (4 != pNtf->m_vecResult.size())
		{
			theLog.Put(WAR_UK, "CHS_HopeJackpot, CODBGWManager::OnChangedNtf() - SP_HOPEJACKPOT_NOTIFY_INIT. Wrong Parameter. Size: ", pNtf->m_vecResult.size());
			return;
		}
		int nDisplayType = ::atoi(pNtf->m_vecResult[3].c_str());
		LONG lTotalWinningMoney = ::atoi(pNtf->m_vecResult[0].c_str());
		LONG lTotalJackpotMoney = ::atoi(pNtf->m_vecResult[1].c_str());
		int nTodayRemainWinner = ::atoi(pNtf->m_vecResult[2].c_str());
		theHopeJackpotManager.SetJackpotInfo(nDisplayType, lTotalWinningMoney, lTotalJackpotMoney, nTodayRemainWinner);
		theChannelDir.SendHopeJackpotInfoNtf();
		theLog.Put(INF_UK, "CHS_HopeJackpot, CODBGWManager::OnChangedNtf() - HopeJackpot Notify Init. Info: (", nDisplayType, ",", lTotalWinningMoney, ",", lTotalJackpotMoney, ",", nTodayRemainWinner, ")");
		return;
	}
	// Jackpot 예시/당첨
	else if ("SP_HOPEJACKPOT_NOTIFY_WINNER" == pNtf->m_strNotiID)
	{
		theLog.Put(INF_UK, "CHS_HopeJackpot, CODBGWManager::OnChangedNtf() - HopeJackpot Notify Winner.");
		return;
	}
#endif	// _HOPE_JACKPOT_
	else 
	{
		theLog.Put(WAR_UK, __FUNCTION__, " Invalid NotiID, pNtf->m_NotiID:", pNtf->m_strNotiID);
	}
}



//SELECT ROOMID,GLSIP,GLSPORT,GLSLRBADDRESS,REGPASSWD,ENTRANCEMONEY,DISPLAYINFO from VIP_ROOM_INFO_

void CODBGWManager::UpdateVIPList(LONG lSSN, vector<string> &queryAns)
{
	TLock lo(this);
	VIPRoomList *pVIPList = NULL;
	VIPMap::iterator i = m_mapVIP.find(lSSN);

	if (i == m_mapVIP.end())
	{
		pVIPList = new VIPRoomList();
		m_mapVIP[lSSN] =  pVIPList;
	}
	else
		pVIPList = i->second;
	const int nFieldNum = 7;
	pVIPList->m_sizeList = queryAns.size() / nFieldNum;
	theLog.Put(DEV_UK, "ButterFlyDebug, Received VIPRoomListUpdateMsg. RoomNum=", pVIPList->m_sizeList);
	{
		pVIPList->m_vecRoomID.clear();
		pVIPList->m_vecGLSIP.clear();		
		pVIPList->m_vecGLSPort.clear();		
		pVIPList->m_vecGLSAddr.clear();		
		pVIPList->m_vecRegPasswd.clear();
		pVIPList->m_vecEntranceMoney.clear();
		pVIPList->m_vecDisplayInfo.clear();
	}

	for (int i = 0; i < pVIPList->m_sizeList ; i++)
	{
		{// For Debug Log		
			string strRoomID;
			Str64ToRoomID(queryAns[nFieldNum*i + 0].c_str()).GetInstanceID(strRoomID);
			theLog.Put(DEV_UK,"VIPRoom[",i+1,"], RoomID:", strRoomID);
		}
		pVIPList->m_vecRoomID.push_back(queryAns[nFieldNum*i + 0]);
		pVIPList->m_vecGLSIP.push_back(queryAns[nFieldNum*i + 1]);
		pVIPList->m_vecGLSPort.push_back(queryAns[nFieldNum*i + 2]);
		pVIPList->m_vecGLSAddr.push_back(queryAns[nFieldNum*i + 3]);
		pVIPList->m_vecRegPasswd.push_back(queryAns[nFieldNum*i + 4]);
		pVIPList->m_vecEntranceMoney.push_back(queryAns[nFieldNum*i + 5]);
		pVIPList->m_vecDisplayInfo.push_back(queryAns[nFieldNum*i + 6]);
	}
	return;
}



void CODBGWManager::GetVIPRoomList(LONG lSSN, MsgCHSCli_VIPRoomListAns &ans)
{

	VIPRoomList *pVIPList = NULL;
	ans.m_lErrorCode = 0x00;

	TLock lo(this);

	VIPMap::iterator i = m_mapVIP.find(lSSN);
	if (i != m_mapVIP.end())
		pVIPList = i->second;
	
	if (pVIPList == NULL || pVIPList->m_sizeList == 0)
		return;
		
	for (int i = 0 ; i < pVIPList->m_sizeList; i++)
	{	

		VIPRoomInfo room;
		room.m_roomID				= Str64ToRoomID(pVIPList->m_vecRoomID[i]);
		sscanf(pVIPList->m_vecGLSIP[i].c_str(),		"%u", &room.m_nsapGLS.m_dwIP);
		sscanf(pVIPList->m_vecGLSPort[i].c_str(),	"%u", &room.m_nsapGLS.m_dwPort);		
		room.m_vecGameRoomInfo.push_back(pVIPList->m_vecRegPasswd[i]);
		room.m_vecGameRoomInfo.push_back(pVIPList->m_vecEntranceMoney[i]);
		room.m_vecGameRoomInfo.push_back(pVIPList->m_vecDisplayInfo[i]);
		ans.m_vecRoom.push_back(room);
	}

	return;
}



BOOL CODBGWManager::GetRoomLRBAddr(LONG lSSN, RoomID &rid, LRBAddress &addrGLS)
{
	VIPRoomList *pVIPList = NULL;

	TLock lo(this);
	VIPMap::iterator i = m_mapVIP.find(lSSN);

	if (i != m_mapVIP.end())
		pVIPList = i->second;

	if (pVIPList == NULL || pVIPList->m_sizeList == 0)
		return FALSE;

	string str64Rid = RoomIDToStr64(rid);
	for (int i = 0 ; i < pVIPList->m_sizeList; i++)
	{
		if (pVIPList->m_vecRoomID[i].compare(str64Rid) ==0)
		{
			addrGLS.SetAddress(pVIPList->m_vecGLSAddr[i].c_str());
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CODBGWManager::RegisterServiceReq()
{
	TLock lo(this);
	static BOOL bRegisterSuccess = FALSE;
	if (bRegisterSuccess == TRUE)
		return TRUE;

	// RMM 서비스를 위한 SSN을 읽어 온다.
	char szSSN[1024] = {0x00};
	//::GetPrivateProfileStringA("RMM", "SSN", "", szSSN, sizeof(szSSN)/sizeof(char), theControl.m_confPath.GetConfPath());
	::GetPrivateProfileStringA("OS3", "SSN", "", szSSN, sizeof(szSSN)/sizeof(char), theControl.m_confPath.GetConfPath());

	//set<LONG> setRegisteredSSN;

	BOOL bIsRMMService = FALSE;
	BOOL bIsVIPService = FALSE;

	char *token = strtok(szSSN, " ,\t");
	while (token != NULL)
	{
		LONG lSSN = atoi(token);

		// RMM은 무조건 VIP 하기로 됨. 아래 코드 무시 - 나중에 삭제 -
		// LONG lMSN = theControl.SSN2MSN(lSSN);
		token = strtok(NULL, " ,\t");

		// 서비스하지 않는 SSN 이면 그냥 무시
		if (theChannelDir.m_ssnTable.find(lSSN) == theChannelDir.m_ssnTable.end())
			continue;
		
		// Room Match Making 처리			
		RegisterRMMServiceReq(lSSN);
		bIsRMMService = TRUE;

		// VIP Room 처리
		RegisterVIPServiceReq(lSSN);
		bIsVIPService = TRUE;
	}


	if(FALSE == RegisterJackpotServiceReq())
	{
		LOG(ERR_UK, __FUNCTION__, "::Fail to RegisterJackpotServiceReq");
		return FALSE;
	}
	bRegisterSuccess = TRUE;

	return TRUE;
}

void CODBGWManager::OnRcvMsg(const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if(wMessageType & MESSAGE_NACK)
	{
		DeleteServiceNumber(src);
		LOG(INF_UK, __FUNCTION__, ", receive NACK message from ODBGW. src:", src.GetString(), ", dest:", dest.GetString());
		return;
	}

	PayloadODBGWCli pld;
	if (!::BLoad(pld, buf))
	{
		LOG(INF_UK, __FUNCTION__, ", !!!!!!!! OnRcvODBGWMsg : BLoad Error"); // INF_UK와 ERR_UK가 고민스럽지만 다른 핸들러 함수에서 모두 INF_UK를 사용했으므로 일단 이렇게.
		return;
	}

	switch(pld.mTagID)
	{
	case PayloadODBGWCli::msgRegisterServiceNtf_Tag:
		OnRegisterServiceNtf(src, dest);
		break;
	case PayloadODBGWCli::msgRegisterServiceAns_Tag:
		OnRegisterServiceAns(src, dest);
		break;
	case PayloadODBGWCli::msgChangedNtf_Tag:		
		OnChangedNtf(src, dest, pld.un.m_msgChangedNtf);
		break;
	case PayloadODBGWCli::msgQueryAns_Tag:
		OnQueryAns(src, dest, pld.un.m_msgQueryAns);
		break;
	}
}

BOOL CODBGWManager::RegisterJackpotServiceReq()
{
	LRBAddress addrMCHS;
	addrMCHS.SetAddress("MCHSJACKPOT");
	// kukuta if (::XLRBConnectorRegister(&addr, this) == FALSE)
	if(FALSE == theLRBHandler.RegisterAddress(addrMCHS))
	{
		LOG(ERR_UK, "CHS_CLRBHandler,*** Fail to Register an Address [", addrMCHS.GetString(), "]");
		return FALSE;
	}
	else
	{
		LOG(INF_UK, "CHS_CLRBHandler,*** Register an Address [", addrMCHS.GetString(), "]");
	}
	
	LRBAddress addrAODBGW;
	addrAODBGW.SetAddress("AODBJACKPOT");
	if(FALSE == SendRegisterServiceReq(addrAODBGW))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CODBGWManager::RegisterRMMServiceReq(LONG lSSN)
{
	char szAddr[20];
	sprintf(szAddr, "MCHSRMM%d", lSSN);

	LRBAddress addr;
	addr.SetAddress(szAddr);
	          	
	if(FALSE == theLRBHandler.RegisterAddress(addr))
	{
		LOG(ERR_UK, "CHS_CLRBHandler,*** Fail to Register an Address [", szAddr, "]");
		return FALSE;
	}
	else
	{
		LOG(INF_UK, "CHS_CLRBHandler,*** Register an Address [", szAddr, "]");
	}

	sprintf(szAddr, "AODBRMM%d", lSSN);
	addr.SetAddress(szAddr);
	if(FALSE == SendRegisterServiceReq(addr))
	{
		LOG(INF_UK, "CHS_CLRBHandler,*** SendRegisterServiceReq [", szAddr, "]");
		return FALSE;
	}
	return TRUE;
}

BOOL CODBGWManager::RegisterVIPServiceReq(LONG lMSN)
{
	char szAddr[20];
	sprintf(szAddr, "MCHSVIP%d", lMSN);

	LRBAddress addr;
	addr.SetAddress(szAddr);

	
	if(FALSE == theLRBHandler.RegisterAddress(addr))
	{
		LOG(ERR_UK, "CHS_CLRBHandler,*** Fail to Register an Address [", szAddr, "]");
		return FALSE;
	}
	else
	{
		LOG(INF_UK, "CHS_CLRBHandler,*** Register an Address [", szAddr, "]");
	}


	sprintf(szAddr, "AODBVIP%d", lMSN);
	addr.SetAddress(szAddr);
	if(FALSE == SendRegisterServiceReq(addr))
	{
		LOG(INF_UK, "CHS_CLRBHandler,*** SendRegisterServiceReq [", szAddr, "]");
		return FALSE;
	}
	
	return TRUE;
}

#ifdef _HOPE_JACKPOT_
BOOL CODBGWManager::SendHopeJackpotSelectInfoReq()
{
	if (FALSE == theHopeJackpotManager.IsServiceOn())
	{
		LOG(WAR_UK, "CHS_HopeJackpot, Warning!! Hope Jackpot is off.");
		return FALSE;
	}

	MsgCliODBGW_ExecutePlainQueryReq req;
	req.m_needAnswer = TRUE;
	req.m_bufKey = "HOPEJACKPOT_SELECTINFO";
	req.m_strQuery = ::format("SELECT Display_Type, Total_Winning_Money, Remain_Jackpot_Money, Today_Remain_Winner FROM HopeJackpot_Info_State");

	PayloadCliODBGW pld(PayloadCliODBGW::msgPlainReq_Tag, req);
	LRBAddress addrODBGW = theHopeJackpotManager.GetODBGWAddress();
	theLRBHandler.AsyncSend(pld, &addrODBGW);

	LOG(DEV_UK, "CHS_HopeJackpot, CODBGWManager::SendHopeJackptSelectInfoReq()");
	return TRUE;
}

CHopeJackpotManager theHopeJackpotManager;

CHopeJackpotManager::CHopeJackpotManager()
{
	m_bHopeJackpotFlag = FALSE;
	m_bHopeJackpotGroupFlag = FALSE;
	m_setSSN.clear();
	m_setSSNGroup.clear();
	m_nDisplayType = 2;
	m_lTotalWinningMoney = 0;
	m_lTotalJackpotMoney = 0;
	m_nTodayRemainWinner = 0;
}

CHopeJackpotManager::~CHopeJackpotManager()
{
	m_setSSN.clear();
	m_setSSNGroup.clear();
}

BOOL CHopeJackpotManager::Init()
{
	// Service Target SSN
	m_setSSN.insert(2);
	m_setSSN.insert(3);
	m_setSSN.insert(17);
	m_setSSN.insert(40);

	// Service Group SSN
	m_setSSNGroup.insert(2);
	m_setSSNGroup.insert(3);
	m_setSSNGroup.insert(17);
	m_setSSNGroup.insert(40);
	m_setSSNGroup.insert(18);
	m_setSSNGroup.insert(23);
	m_setSSNGroup.insert(44);
	m_setSSNGroup.insert(53);

	char szTempAddr[LRBADDRESS_SIZE + 1];
	::memset(szTempAddr, 0, sizeof(szTempAddr));
	if (0 != ::GetPrivateProfileStringA("HopeJackpot", "JackpotAddress", "MHOPEJP", szTempAddr, LRBADDRESS_SIZE, theControl.m_confPath.GetConfPath()))
	{
		if (0 < ::strlen(szTempAddr))
			m_addrMulticastJackpot.SetAddress(szTempAddr);
		else
		{
			LOG(WAR_UK, "CHS_HopeJackpot, CHopeJackpotManager::Init() - Wrong JackpotAddress");
			return FALSE;
		}
	}
	else
	{
		LOG(WAR_UK, "CHS_HopeJackpot, CHopeJackpotManager::Init() - No JackpotAddress");
		return FALSE;
	}
	::memset(szTempAddr, 0, sizeof(szTempAddr));
	if (0 != ::GetPrivateProfileStringA("HopeJackpot", "ODBGWAddress", "AODBHOPEJP", szTempAddr, LRBADDRESS_SIZE, theControl.m_confPath.GetConfPath()))
	{
		if (0 < ::strlen(szTempAddr))
			m_addrAnycastODBGW.SetAddress(szTempAddr);
		else
		{
			LOG(WAR_UK, "CHS_HopeJackpot, CHopeJackpotManager::Init() - Wrong ODBGWAddress");
			return FALSE;
		}
	}
	else
	{
		LOG(WAR_UK, "CHS_HopeJackpot, CHopeJackpotManager::Init() - No ODBGWAddress");
		return FALSE;
	}
	::memset(szTempAddr, 0, sizeof(szTempAddr));
	if (0 != ::GetPrivateProfileStringA("HopeJackpot", "CHSAddress", "MCHSHOPEJP", szTempAddr, LRBADDRESS_SIZE, theControl.m_confPath.GetConfPath()))
	{
		if (0 < ::strlen(szTempAddr))
			m_addrMulticastCHS.SetAddress(szTempAddr);
		else
		{
			LOG(WAR_UK, "CHS_HopeJackpot, CHopeJackpotManager::Init() - Wrong CHSAddress");
			return FALSE;
		}
	}
	else
	{
		LOG(WAR_UK, "CHS_HopeJackpot, CHopeJackpotManager::Init() - No CHSAddress");
		return FALSE;
	}

	ForEachElmt(set<LONG>, m_setSSN, i, j)
	{
		LONG lSSN = *i;
		if (theChannelDir.m_ssnTable.end() != theChannelDir.m_ssnTable.find(lSSN))
		{
			m_bHopeJackpotFlag = TRUE;
			LOG(INF_UK, "CHS_HopeJackpot, Service is On by SSN: ", lSSN);
		}
	}

	ForEachElmt(set<LONG>, m_setSSNGroup, i, j)
	{
		LONG lSSN = *i;
		if (theChannelDir.m_ssnTable.end() != theChannelDir.m_ssnTable.find(lSSN))
		{
			m_bHopeJackpotGroupFlag = TRUE;
			LOG(INF_UK, "CHS_HopeJackpot, Service is On by SSN: ", lSSN);
		}
	}

	if (FALSE == RegisterAddress())
	{
		LOG(INF_UK, "CHS_HopeJackpot, CHopeJackpotManager::Init() - RegisterAddress() failed.");
		return FALSE;
	}

	LOG(INF_UK, "CHS_HopeJackpot, CHopeJackpotManager::Init() - Success.");
	return TRUE;
}

BOOL CHopeJackpotManager::IsServiceOn()
{
	return m_bHopeJackpotFlag;
}

BOOL CHopeJackpotManager::IsServiceOn(LONG lSSN)
{
	if (FALSE == m_bHopeJackpotFlag)
		return FALSE;
	if (m_setSSN.end() == m_setSSN.find(lSSN))
		return FALSE;
	return TRUE;
}

BOOL CHopeJackpotManager::IsServiceGroup()
{
	return m_bHopeJackpotGroupFlag;
}

BOOL CHopeJackpotManager::IsServiceGroup(LONG lSSN)
{
	if (FALSE == m_bHopeJackpotGroupFlag)
		return FALSE;
	if (m_setSSNGroup.end() == m_setSSNGroup.find(lSSN))
		return FALSE;
	return TRUE;
}

LRBAddress CHopeJackpotManager::GetODBGWAddress()
{
	return m_addrAnycastODBGW;
}

void CHopeJackpotManager::GetJackpotInfo(int& nDisplayType, LONG& lTotalWinningMoney, LONG& lTotalJackpotMoney, int& nTodayRemainWinner)
{
	nDisplayType = m_nDisplayType;
	lTotalWinningMoney = m_lTotalWinningMoney;
	lTotalJackpotMoney = m_lTotalJackpotMoney;
	nTodayRemainWinner = m_nTodayRemainWinner;
}

void CHopeJackpotManager::SetJackpotInfo(int nDisplayType, LONG lTotalWinningMoney, LONG lTotalJackpotMoney, int nTodayRemainWinner)
{
	m_nDisplayType = nDisplayType;
	m_lTotalWinningMoney = lTotalWinningMoney;
	m_lTotalJackpotMoney = lTotalJackpotMoney;
	m_nTodayRemainWinner = nTodayRemainWinner;
}

BOOL CHopeJackpotManager::RegisterAddress()
{
	if (TRUE == m_bHopeJackpotFlag)
	{
		// for Communication with ODBGW
		if (FALSE == theLRBHandler.RegisterAddress(m_addrMulticastJackpot))
		{
			LOG(WAR_UK, "CHS_HopeJackpot, Hope Jackpot Regiester Address Failed. JackpotAddress:", m_addrMulticastJackpot.GetString().c_str());
			return FALSE;
		}
		MsgCliODBGW_RegisterServiceReq req;
		PayloadCliODBGW pld(PayloadCliODBGW::msgRegisterServiceReq_Tag, req);
		if (FALSE == theLRBHandler.AsyncSend(pld, &m_addrAnycastODBGW))
		{
			LOG(WAR_UK, "CHS_HopeJackpot, Hope Jackpot Send Regiester Service Request Failed. ODBGWAddress:", m_addrAnycastODBGW.GetString().c_str());
			return FALSE;
		}
		if (FALSE == theODBGWMgr.SendHopeJackpotSelectInfoReq())
		{
			LOG(WAR_UK, "CHS_HopeJackpot, Hope Jackpot Send Select Info Request Failed.");
			return FALSE;
		}
	}
	// Service Group 인 경우에는 Multicast 는 받기
	if (TRUE == m_bHopeJackpotGroupFlag)
	{
		// for Multicast from GLS
		if (FALSE == theLRBHandler.RegisterAddress(m_addrMulticastCHS))
		{
			LOG(WAR_UK, "CHS_HopeJackpot, Hope Jackpot Regiester Address Failed. CHSAddress:", m_addrMulticastCHS.GetString().c_str());
			return FALSE;
		}
	}

	return TRUE;
}
#endif	// _HOPE_JACKPOT_

#ifdef _JACKPOT_INDICATION_
void CODBGWManager::OnRcvSelectJackpotTotalMoneyAns(MsgODBGWCli_QueryAns* pAns)
{
	const int COLUMN_COUNT = 2;	
	if(0 != (pAns->m_vecResult.size() % COLUMN_COUNT))
	{
		LOG(ERR, "Jackpot Money Parameter Count Error");
		return;
	}

	if(0 == pAns->m_vecResult.size())
	{
		
	}

	for(unsigned int i=0; i<pAns->m_vecResult.size(); i+=COLUMN_COUNT)
	{
		LONG lSSN = ::atoi(pAns->m_vecResult[i+0].c_str());
		string strJackpotMoney = pAns->m_vecResult[i+1];
		if(TRUE == theJackpotManager.IsServiceOn(lSSN))
		{
			theJackpotManager.SetJackpotMoneyBySSN(lSSN, strJackpotMoney);
		}
	}      	
}
#endif
