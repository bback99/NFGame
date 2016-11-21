//
//// LRBManager.cpp
//
//
//#include "stdafx.h"
//#include "Common.h"
//#include "LRBManager.h"
//#include "ChannelDir.h"
//// #include "ChannelTable.h"
//#include "Control.h"
//#include "Listener.h"
//#include "StatisticsTable.h"
//#include "Reporter.h"
//
/////////////////////////////////////////////////////////////////////////////////
//// CLRBManager
//
//CLRBManager theLRBManager;
//
//CLRBManager::CLRBManager() : m_dwRefCnt(0)
//{
//	m_lAMSSourceAddr = 0L;
//	m_LRBADDRESS = -999L;
//	m_lNextLRBIP = 0L;
//	m_lRetryCnt = 0L;
//	m_bFirstStart = FALSE;
//	m_bLRBRun = FALSE;
//
//	m_apSocket = new TSocket;
//}
//
//CLRBManager::~CLRBManager() 
//{
//	StopLRBManager();
//}
//
//STDMETHODIMP_(ULONG) CLRBManager::AddRef()
//{
//	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
//	return dwRefCnt;
//}
//
//STDMETHODIMP_(ULONG) CLRBManager::Release()
//{
//	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
//	if(dwRefCnt == 0)
//	{
////		FreeRoom(this);
//	}
//	return dwRefCnt;
//}
//BOOL CLRBManager::ChangeLRBAddr()
//{
//	//1~2분 정도 하나의 IP로 접속을 시도한다.
//	TCHAR RETRY_CNT[30] = {0, };
//	LONG lRetryCnt = 0L;
//	if(!m_bFirstStart)
//	{
//		if(!::GetPrivateProfileString("TIMER", "RETRYCNT", _T(""), RETRY_CNT, sizeof(RETRY_CNT), "CHSConfig.INI"))
//		{
//			theErr.LOG(1, "Not Found INI file or invalide Retry Count Value ");
//			return FALSE;
//		}
//		lRetryCnt = atoi(RETRY_CNT);
//		if(lRetryCnt < 5) 
//			lRetryCnt = 20;
//		if(m_lRetryCnt > lRetryCnt)
//		{
//			return GetNextLRBAddr();
//		}
//	} 
//	else
//	{
//		return GetNextLRBAddr();
//	}
//	m_lRetryCnt += 1;
//
//	return TRUE;
//}
//
//BOOL CLRBManager::GetNextLRBAddr()
//{
//	TCHAR LRBIP[30] = {0, };
//	char KEYWORD[30] = {0, };
//	m_lRetryCnt = 0L;
//	sprintf(KEYWORD, "LRBIP%d", m_lNextLRBIP);
//	m_lNextLRBIP += 1;
//	BOOL dwRet = ::GetPrivateProfileString("LRB", KEYWORD, _T(""), LRBIP, sizeof(LRBIP), "CHSConfig.INI");
//	if(dwRet) 
//	{
//		//theChannelDir.SetLRBIP(LRBIP);
//		m_sLRBIP = LRBIP;
//		theErr.LOG(1, "+++++++ LRBManger Run : Next LRB IP Setting [%s] IP +++++++++", LRBIP);
//		return TRUE;
//	} 
//	else 
//	{
//		dwRet = ::GetPrivateProfileString("LRB", "LRBIP0", _T(""), LRBIP, sizeof(LRBIP), "CHSConfig.INI");
//		if(!dwRet)
//			return FALSE;
//		//theChannelDir.SetLRBIP(LRBIP);
//		m_sLRBIP = LRBIP;
//		m_lNextLRBIP = 0L;
//		theErr.LOG(1, "*** Not found Next LRB Ip, Reset First LRB IP *** ");
//		return TRUE;
//	}	
//	return FALSE;
//}
//
//BOOL CLRBManager::RunLRBManager()
//{
//	TLock lo(this);
//	theErr.LOG(2, "RunLRBManager.....");
//	TRACE0("LRBManager - Run()\n");
//
//	BOOL bRet = TRUE;
//
//	// avoid assert..
//	StopLRBManager();
//	
//	CLRBLink* pLink = GetSocket();
//	if(!pLink) 
//		return FALSE;
//	if (!pLink->Socket()) {
//		return FALSE;
//	}
//	///////////////////////////
//	//ChangeLRBAddr();
//	///////////////////////////
//
//	if(!pLink->Connect(m_sLRBIP, PORT_SYSTEM)) 
//	{
//		if(::WSAGetLastError() != WSAEWOULDBLOCK) 
//		{
//			TRACE1("LRBManager - fail to connect(%s)\n", WSAGetErrorDescription(::WSAGetLastError()));
//			StopLRBManager();
//			return FALSE;
//		}
//	}
//	bRet = bRet && EventSelect(pLink->GetHandle(), FD_ALL_EVENTS);
//	VALIDATE(bRet);
//	if (!bRet) {
//		StopLRBManager();
//		return FALSE;
//	}
//	bRet = bRet && ActivateSocket(GetLRBThreadPool());	// 외부와 연결은 전용의 thread pool을 사용 
//	VALIDATE(bRet);
//	if(!bRet)
//	{
//		StopLRBManager();
//		return FALSE;
//	}
//
//	return TRUE;
//}
//
//BOOL CLRBManager::StopLRBManager()
//{
//	TLock lo(this);
//	TRACE0("LRBManager - Stop()\n");
//	if (m_timerAlive2.IsActive())	
//		m_timerAlive2.Deactivate();
//	DestroyLink(NULL);
//	return TRUE;
//}
//
////BOOL CLRBManager::TryServiceRegistToLB(NSAP & nsapCHS)
////{
////	TLock lo(this);
////	LONG lServiceID = m_LRBADDRESS;
////	PayloadCHSLB pld(PayloadCHSLB::msgRegisterServiceReq_Tag, MsgCHSLB_RegisterServiceReq(nsapCHS, lServiceID));
////	SendToLB(pld);
////	return TRUE;
////}
//
//BOOL CLRBManager::TryServiceRegistToLB(NSAP & nsapCHS)
//{
//	TLock lo(this);
//
//	CHSLBRegister reg_info(nsapCHS, m_LRBADDRESS, theChannelDir.m_lstChannelInfoForLBReg);
//	PayloadCHSLB pld(PayloadCHSLB::msgRegisterServiceNtf_Tag, MsgCHSLB_RegisterServiceNtf(reg_info));
//	SendToAllLB(pld);
//	return TRUE;
//}
//
//void CLRBManager::TryServiceRegistToLRB()
//{
//	PayloadServiceLRB msg(PayloadServiceLRB::msgRegisterServiceReq_Tag,
//							MsgRegisterServiceReq(SvcTypeID((WORD)SVCCAT_CHS, (WORD)theControl.m_lCHSID)));
//	DirectSendToLRB(msg);
//}
//
//void CLRBManager::DestroyLink(CLRBLink* pLink)
//{
//	TLock lo(this);
//	DeactivateSocket();
//	m_apSocket->Close();
//}
//
//BOOL CLRBManager::OnConnect(TSocket* pSocket, int nErrorCode)
//{
//	theErr.LOG(2, "+++++ LRBManager OnConnect function call [%d]ErrCode +++++", nErrorCode);
//	TryServiceRegistToLRB();
//	return TRUE;
//}
//
//BOOL CLRBManager::OnClose(TSocket* pSocket, int nErrorCode)
//{
//	theErr.LOG(1, "+++++ OnClose LRBManager : [%d] ErrorCode ", nErrorCode);
//	StopLRBManager();
//
//
//	m_bLRBRun = FALSE;
//	RecoverLRBManager();
//
//	BOOL bRet2 = m_timerAlive2.Deactivate();
//	if(!bRet2)
//	{
//		return FALSE;
//	}
//	return TRUE;
//}
//
//void CLRBManager::RecoverLRBManager()
//{
//	theErr.LOG(1, "*** Try to Recover connection LRB Manager ***");
//
//	theControl.SetBootStateOut(CHS_CONNECT_LRB);
//}
//
//BOOL CLRBManager::OnError(CLRBLink* pLink, long lEvent, int nErrorCode)
//{
//	TRACE1("LRBManager-OnError(%d)\n", pLink->GetHandle());
//	theErr.LOG(1, "+++++ OnCloser LRBManager : [%d]lEvent [%d]ErrorCode ", lEvent, nErrorCode);
//	DestroyLink(pLink);	
//	
//	//다시 기동을 시도한다.
//	RecoverLRBManager();
//
//	return FALSE;
//}
//
//STDMETHODIMP_(void) CLRBManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
//{
//	if((LONG)hSignal == 0L) // user define event , OutThreadPool을 사용.
//	{	
//		if((LONG)wParam == CHANNEL_LRBMSG) 	
//		{
//			TLock lo(this);
//			OnSendToLRB();
//		}
//	}
//	else 
//	{
//		if(m_timerAlive2.IsHandle(hSignal))
//		{
//			TLock lo(this);
//			// 수신한 명령의 누적 카운트를 Alive message와 겸해서 로그에 남김.
//			m_MsgCount.StampLogGLS(2L);
//
//			// Heartbeat flag setting
//			theHeartBeat.SetLRBFlag();
//
//			TBase::OnSignal(hSignal, wParam, lParam);
//		}
//		else 
//		{
//			TLock lo(this);
//			TBase::OnSignal(hSignal, wParam, lParam);
//		}
//	}
//}
//
//BOOL CLRBManager::OnRcvMsg(TSocket* pSocket, PayloadLRBService& pld)
//{
////	DWORD r_size = pSocket->GetRecvBufSize();
////	if(r_size > 1024 * 4000)
////		theErr.LOG(2, "LRBManager's Recv Buffer Size : [%d] byte", r_size);
//
//	switch(pld.mTagID) {
//	case PayloadLRBService::msgRegisterServiceAns_Tag:	
//		OnRegisterServiceAns(*(pld.un.m_msgRegisterServiceAns));
//		break;
//	case PayloadLRBService::msgRegisterInstanceAns_Tag:
//		RegisterInstanceAns(*(pld.un.m_msgRegisterInstanceAns));
//		break;
//	case PayloadLRBService::msgDeregisterInstanceAns_Tag:
//		DeRegisterInstanceAns(*(pld.un.m_msgDeregisterInstanceAns));
//		break;
//	case PayloadLRBService::msgService_Tag:
//		OnRecvLRBMessage(pld);
//		break;
//	case PayloadLRBService::msgTerminateServiceNtf_Tag:
//		OnTerminateServiceNtf(*(pld.un.m_msgTerminateServiceNtf));
//		break;
//	default:
//		TLOG0("OnRcvMsg from LRB :  Unknown msg \n");
//		theErr.LOG(1, "+++ OnRcvMsg from LRB :  Unknown msg +++");
//	}
//	return TRUE;
//}
//////////////////////////
//// neo1
////
////GLS에 이상이 발생했을 경우 해당 GLS에 속한 Room들과 그 속에 속한 user들에 대한 절적한 처리가 필요. 
//////////////////////////
//// TSN_SERVICE	// 해당 서비스가 종료되었음
//// TSN_LRB		//서비스가 연결된 LRB가 종료되었음.
//////////////////////////
//void CLRBManager::OnTerminateServiceNtf(MsgTerminateServiceNtf & msg)
//{
//	theErr.LOG(2, "===== TerminateServiceNtf: [%d]LinkID [%d]LRBID [%d]SVCCAT [%d]REASON======", msg.m_logicalAddr.m_wLinkID, msg.m_logicalAddr.m_wLRBID, msg.m_serviceTypeID.m_wSvcCat, msg.m_lErrorCode);
//	if(msg.m_serviceTypeID.m_wSvcCat == SVCCAT_GLS && msg.m_lErrorCode == TSN_SERVICE) {
//		DWORD dwLogical = (DWORD)MAKELONG(msg.m_logicalAddr.m_wLinkID, msg.m_logicalAddr.m_wLRBID);
//
//		NSAP nsap;
//		if(!theChannelDir.GetGLSNsap(dwLogical, nsap))
//		{
//			theErr.LOG(2, "TerminateServiceNtf : Not found [%d]LinkID [%d]LRBID", msg.m_logicalAddr.m_wLinkID, msg.m_logicalAddr.m_wLRBID);
//			return;
//		}
//		string sIP;
//		nsap.GetIP(sIP);
//		theErr.LOG(1, "++++ GLS Stoped ...[%d] SVCCAT [%s]IP ++++", msg.m_serviceTypeID.m_wSvcCat, sIP.c_str());
//		theChannelDir.RemGLS(nsap);	//neo1
//
//		theChannelDir.RemGLSLogicalAddr(dwLogical);
//	}
//	// GLS가 연결된 LRB가 죽었을 경우에 해당함.
//	else if(msg.m_serviceTypeID.m_wSvcCat == SVCCAT_GLS && msg.m_lErrorCode == TSN_LRB)
//	{
//		DWORD dwLogical = (DWORD)MAKELONG(msg.m_logicalAddr.m_wLinkID, msg.m_logicalAddr.m_wLRBID);
//		theChannelDir.RemGLSLogicalAddr(dwLogical);
//		theErr.LOG(1, "++++ LRB Stoped : connected GLS...[%d] SVCCAT ++++", msg.m_serviceTypeID.m_wSvcCat);
//	}
//	else if(msg.m_serviceTypeID.m_wSvcCat == SVCCAT_LB)
//	{
//		theErr.LOG(2, "TerminateServiceNtf : LB... ");
//		if(theControl.GetBootState() != CHS_RUN)
//		{
//			theErr.LOG(1, "TerminateServiceNtf : Invalid State ");
//			return;
//		}
//		theControl.SetBootStateOut(CHS_REG_LB);
//	}
//	else
//	{
//		theErr.LOG(1, "+++++++ Service Stoped : [%d] SVCCAT ID [%d]REASON++++++++", msg.m_serviceTypeID.m_wSvcCat, msg.m_lErrorCode);
//	}
//}
//
//void CLRBManager::OnRegisterServiceAns(MsgRegisterServiceAns & msg)
//{
//	if(msg.m_lErrorCode == RDA_RECONNECTOTHER)
//	{
//		if(!GetNextLRBAddr())
//		{
//			theErr.LOG(1, " fali GetNextLRBAddr : OnRegisterServiceAns ");
//		}
//		if(theControl.GetBootState() == CHS_CONNECTING_LRB)
//		{
//			theControl.SetBootState(CHS_CONNECT_LRB);
//		}
//		theErr.LOG(2, " Recv OnRegisterServiceAns (LRB) ");
//		return;
//	}
//	else if(msg.m_lErrorCode == RDA_SUCCESS) 
//	{
////		theConstSetFlag.m_bLRBRun = TRUE;
//		if(theControl.GetBootState() == CHS_CONNECTING_LRB)
//		{
//			theControl.SetBootState(CHS_REG_INSTANCE);
//		}
//
//		LONG pre_LRBADDRESS  = m_LRBADDRESS;
//		m_LRBADDRESS = MAKELONG(msg.m_logicalAddr.m_wLinkID, msg.m_logicalAddr.m_wLRBID);
//		theErr.LOG(1, "Success Service Register : [%d]SvcCat, [%d]SvcTypeID ", msg.m_serviceTypeID.m_wSvcCat, msg.m_serviceTypeID.m_wSvcTypeID);
//		theErr.LOG(1, "Success Service Register : [%d]LinkID, [%d]LRBID ", msg.m_logicalAddr.m_wLinkID, msg.m_logicalAddr.m_wLRBID);
//
//		m_lRetryCnt = 0L;
////		m_bFirstStart = TRUE;
//		m_bLRBRun = TRUE;
//
//		if(!m_timerAlive2.Activate(GetLRBThreadPool(), this, 10000, 10000))
//			return;
//	
//		// 아래는 초기접속인 경우에 호출됨.
//		if(m_LRBADDRESS != pre_LRBADDRESS && pre_LRBADDRESS != -999)
//		{
//			PayloadCHSGLS pld(PayloadCHSGLS::msgChangeAddrNtf_Tag);
//			pld.un.m_msgChangeAddrNtf->m_dwOldAddr = pre_LRBADDRESS;
//			pld.un.m_msgChangeAddrNtf->m_dwNewAddr = m_LRBADDRESS;
//#ifdef USE_DIRECTSEND
//			MessageHeader header;
//			header.Clear();
//			theLRBManager.MakeHeader(header, SVCCAT_GLS, ROUTTYPE_STATICMULTICASTING, MSGTYPE_ACK);
//
//			GBuf buf;
//			if(!::BStore(buf, pld))
//				return;
//			PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//			msg.un.m_msgService->m_header = header;
//			msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//			DirectSendToLRB(msg);	// neo direct
//#else
//			SendToAllGLS(pld);
//#endif
//		}
//
//		return ;
//	}
//	theErr.LOG(1, "+++++ Failed Service Registraton to LRB ++++++");
//}
//
//void CLRBManager::RegisterInstanceReq(ChannelID channelID)
//{
//	string s_iid;
//	channelID.GetInstanceID(s_iid);
//	InstanceID instance(s_iid);
//	LogicalAddr logical(HIWORD(m_LRBADDRESS), LOWORD(m_LRBADDRESS));
//	PayloadServiceLRB msg(PayloadServiceLRB::msgRegisterInstanceReq_Tag,
//							MsgRegisterInstanceReq(instance, logical, 0L));
//	PostSendToLRB(msg);
//}
//
//static DWORD s_RegInstNACKCnt = 0UL;
//void CLRBManager::RegisterInstanceAns(MsgRegisterInstanceAns & msg)
//{
//	if(theControl.GetBootState() != CHS_REGING_INSTANCE)
//	{
////		theErr.LOG(1, "RegisterInstanceAns : Invalid ");
//		return;
//	}
//	if(s_RegInstNACKCnt > 100)
//	{
//		theControl.SetBootState(CHS_STOP);
//		theErr.LOG(1, "+++++ CHS Stop : fail RegisterInstanceReq +++++"); 
//	}
//	if(msg.m_lErrorCode != RDA_SUCCESS) {
//		s_RegInstNACKCnt++;
//		theErr.LOG(1, "+++++ FAIL : RegisterInstanceAns : [%d] Instance ID +++++", s_RegInstNACKCnt);
//	}
//
//}
//
//void CLRBManager::DeRegisterInstanceReq(ChannelID channelID)
//{
//	string s_iid;
//	channelID.GetInstanceID(s_iid);
//	InstanceID instance(s_iid);
//	LogicalAddr logical(HIWORD(m_LRBADDRESS), LOWORD(m_LRBADDRESS));
//	PayloadServiceLRB msg(PayloadServiceLRB::msgDeregisterInstanceReq_Tag,
//							MsgDeregisterInstanceReq(instance, logical, 0L));
//	PostSendToLRB(msg);
//	theErr.LOG(3, "DeRegisterInstanceReq : %s Instance ID", s_iid.c_str());
//}
//
//void CLRBManager::DeRegisterInstanceAns(MsgDeregisterInstanceAns & msg)
//{
//	if(msg.m_lErrorCode != RDA_SUCCESS) {
//		theErr.LOG(1, "+++++ FAIL : DeRegisterInstanceAns : [%d] Instance ID +++++", msg.m_lErrorCode);
//	}
//	theErr.LOG(3, "Success DeRegisterInstanceAns : [%s] Instance ID", msg.m_instanceID.m_sID.c_str());	
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
////
//void CLRBManager::OnRecvLRBMessage(PayloadLRBService& pld)
//{	//여기서 head를 분석하고 
//	MsgService* pMsg = NULL;
//	WORD wSvcCat = 0;
//	switch(pld.mTagID)
//	{
//	case PayloadLRBService::msgService_Tag:
//		pMsg = pld.un.m_msgService;
//		if(pMsg->m_header.m_cMessageType == MSGTYPE_NACK) 
//		{
//			// 모든 메세지를 다 처리 할 필요는 없다.. 일부 서비스로 전송한 것만 처리.. 
//			wSvcCat = HIWORD(pMsg->m_header.m_dwSourceTypeID);	
//			switch(wSvcCat) 
//			{
//			case SVCCAT_LB:
//				OnRecvLB_NACK(pMsg);
//				break;
//			default:
//				TLOG1("NACK ===== OnRecvLRBMessage : [%d]ServiceCategory \n", HIWORD(pMsg->m_header.m_dwTargetTypeID));
//				theErr.LOG(2, "NACK ===== OnRecvLRBMessage : [%d]ServiceCategory ", HIWORD(pMsg->m_header.m_dwTargetTypeID));
//				break;
//			}	
//			return;
//		}
//		wSvcCat = HIWORD(pMsg->m_header.m_dwSourceTypeID);	
//		switch(wSvcCat) 
//		{
//		case SVCCAT_GLS:
//			OnRecvGLS(pMsg);
//			break;
//		case SVCCAT_LB:
//			OnRecvLB(pMsg);
//			break;
//		case SVCCAT_AMS:
//			OnRecvAMS(pMsg);
//			break;
//		case SVCCAT_SMGW:
//			OnRecvSMGW(pMsg);
//			break;
//		default:
//			break;
//		}	
//		break;
//	default: 
//		theErr.LOG(1, "++++ OnRecvLRBMessage : Receive unknown Message ++++"); 
//		break;
//	}
//}
//
//void CLRBManager::OnRecvLB(const MsgService * pMsg)
//{
//	GBuf buf( (LPVOID) pMsg->m_sRealMsg.c_str(), pMsg->m_sRealMsg.length() );
//	PayloadLBCHS pld;
//	VALIDATE(::BLoad(pld, buf));
//
//	switch(pld.mTagID) {
//	case PayloadLBCHS::msgRegisterServiceReq_Tag:
//		OnRegisterServiceReq(pld.un.m_msgRegisterServiceReq, pMsg->m_header.m_dwSourceAddr);
//		break;
//	case PayloadLBCHS::msgRChannelListAns_Tag:
//		OnRChannelListAns(pld.un.m_msgRChannelListAns);
//		// count message
//		m_MsgCount.m_dwRCList++;
//		break;
//	case PayloadLBCHS::msgGLSInfoAns_Tag:
//		OnGLSInfoAns(pld.un.m_msgGLSInfoAns);
//		break;
//
//	default:
//		TLOG0("Received Unknown message from LB \n");
//		theErr.LOG(1, "+++++ Received Unknown message from LB +++++ \n ");
//	}
//}
//
//void CLRBManager::OnRecvLB_NACK(const MsgService * pMsg)
//{
//	GBuf buf( (LPVOID) pMsg->m_sRealMsg.c_str(), pMsg->m_sRealMsg.length() );
//	PayloadCHSLB pld;
//	VALIDATE(::BLoad(pld, buf));
//
//	switch(pld.mTagID) {
//	case PayloadCHSLB::msgRegisterServiceNtf_Tag:
//		OnRegisterServiceNtf_NACK(pld.un.m_msgRegisterServiceNtf);
//		break;
//	case PayloadCHSLB::msgRChannelListReq_Tag:
//		OnRChannelListReq_NACK(pld.un.m_msgRChannelListReq);
//		break;
//	case PayloadCHSLB::msgGLSInfoReq_Tag:
//		OnGLSInfoReq_NACK(pld.un.m_msgGLSInfoReq);
//		break;
//
//	default:
//		theErr.LOG(1, " NACK :  Received Unknown message from LB  \n ");
//	}
//}
//
//void CLRBManager::OnRecvAMS(const MsgService * pMsg)
//{
//	GBuf buf( (LPVOID) pMsg->m_sRealMsg.c_str(), pMsg->m_sRealMsg.length() );
//	PayloadAMSCHS pld;
//	VALIDATE(::BLoad(pld, buf));
//
//	m_lAMSSourceAddr = pMsg->m_header.m_dwSourceAddr;
//	switch(pld.mTagID) {
////	case PayloadAMSCHS::msgChannelDeleteReq_Tag:
////		OnAMSChannelDeleteReq(pld.un.m_msgChannelDeleteReq);
////		break;
//	case PayloadAMSCHS::msgAnnounceReq_Tag:
//		OnAMSAnnounceReq(pld.un.m_msgAnnounceReq);
//		break;
//	case PayloadAMSCHS::msgServiceStopReq_Tag:
//		OnAMSServiceStopReq(pld.un.m_msgServiceStopReq);
//		break;
//	case PayloadAMSCHS::msgHeartBeatReq_Tag:
//		OnAMSHeartBeatReq(pld.un.m_msgHeartBeatReq);
//		break;
//	case PayloadAMSCHS::msgRoomListReq_Tag:
//		OnAMSRoomListReq(pld.un.m_msgRoomListReq);
//		break;
//	case PayloadAMSCHS::msgStatisticReq_Tag:
//		OnAMSStatisticReq(pld.un.m_msgStatisticReq);
//		break;
//	case PayloadAMSCHS::msgComputerNameReq_Tag:
//		OnComputerNameReq();
//		break;
//	default:
//		TLOG0("Received Unknown Message from AMS \n");
//		theErr.LOG(1, "++++++ Received Unknown Message from AMS ++++++ ");
//	}
//}
//
//void CLRBManager::OnRecvSMGW(const MsgService * pMsg) 
//{
////	GBuf buf( (LPVOID) pMsg->m_sRealMsg.c_str(), pMsg->m_sRealMsg.length() );
////	PayloadSMGWCHS pld;
////	VALIDATE(::BLoad(pld, buf));
////
////	switch(pld.mTagID) {
////	case PayloadSMGWCHS::msgUserLeaveNtf_Tag:
////		OnSMGWUserLeaveNtf(pld.un.m_msgUserLeaveNtf);
////		break;
////	case PayloadSMGWCHS::msgChannelInfoNtf_Tag:
////		OnSMGWChannelInfoNtf(pld.un.m_msgChannelInfoNtf);
////		break;
////	default:
////		TLOG0("Received Unknown Message from SMGW \n");
////		theErr.LOG(1, "+++++++ Received Unknown Message from SMGW ++++++");
////	}
//}
//
//void CLRBManager::OnRcvServerRegistAns(PayloadLRBService *pPld)
//{
//	ASSERT(pPld);
//	if(!pPld)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnRcvServerRegistAns+++++");
//		return;
//	}
//
//	if(pPld->un.m_msgRegisterServiceAns->m_lErrorCode != SVR_REGIST_SUCCESS) {
//		theErr.LOG(2, "Success Server Register");
//		return;
//	} 
//	theErr.LOG(1, "+++++ Fail Service Register +++++ \n");
//}
//////////////////////////////////////////////////////////////////////////////////////////////////
//// Send function
//void CLRBManager::DirectSendToLRB(const PayloadServiceLRB& pld)
//{
//	GBuf buf;
//	if(!::LStore(buf, pld))
//		return;
//	CLRBLink* pLink = (CLRBLink*)GetSocket();
//	if (!pLink) 
//		return;
//	pLink->DoSend(buf);	
//}
//
//void CLRBManager::SendToLB(PayloadCHSLB& pld)
//{
//	MessageHeader header;
//	header.Clear();
//	theLRBManager.MakeHeader(header, SVCCAT_LB);
//
//	GBuf buf;
//	if(!::BStore(buf, pld)) {
//		theErr.LOG(2, "Fail SendToLB : empty payload ");
//		return;
//	}
//	PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//	msg.un.m_msgService->m_header = header;
//	msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	PostSendToLRB(msg);
//}
//
//void CLRBManager::SendToUniLB(PayloadCHSLB& pld, DWORD dwAddr)
//{
//	MessageHeader header;
//	header.Clear();
//	theLRBManager.MakeUniHeader(header, dwAddr);
//
//	GBuf buf;
//	if(!::BStore(buf, pld)) {
//		theErr.LOG(2, "Fail SendToUniLB : empty payload ");
//		return;
//	}
//	PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//	msg.un.m_msgService->m_header = header;
//	msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	PostSendToLRB(msg);
//}
//
//void CLRBManager::SendToAllLB(PayloadCHSLB& pld)
//{
//	MessageHeader header;
//	header.Clear();
//	theLRBManager.MakeHeader(header, SVCCAT_LB, ROUTTYPE_STATICMULTICASTING, MSGTYPE_ACK);
//
//	GBuf buf;
//	if(!::BStore(buf, pld)) {
//		theErr.LOG(2, "Fail SendToAllLB : empty payload ");
//		return;
//	}
//	PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//	msg.un.m_msgService->m_header = header;
//	msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	PostSendToLRB(msg);
//}
//
//void CLRBManager::SendToAMS(PayloadCHSAMS & pld)
//{
//	MessageHeader header;
//	MakeHeader(header, SVCCAT_AMS, ROUTTYPE_TARGET_UNICASTING);
//	header.m_dwTargetAddr = m_lAMSSourceAddr;
//
//	GBuf buf;
//	if(!::BStore(buf, pld))
//		return;
//	PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//	msg.un.m_msgService->m_header = header;
//	msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	PostSendToLRB(msg);
//}
//
//void CLRBManager::SendToAllGLS(PayloadCHSGLS& pld)
//{
//	MessageHeader header;
//	header.Clear();
//	theLRBManager.MakeHeader(header, SVCCAT_GLS, ROUTTYPE_STATICMULTICASTING, MSGTYPE_ACK);
//
//	GBuf buf;
//	if(!::BStore(buf, pld))
//		return;
//	PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//	msg.un.m_msgService->m_header = header;
//	msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	PostSendToLRB(msg);
//}
//
//void CLRBManager::PostSendToLRB(const PayloadServiceLRB& pld)
//{
//	GBuf buf;
//	BOOL bRet = ::LStore(buf, pld);
//	VALIDATE(bRet);
//
//	m_LRBMsgQ.Push(buf);
//	::XsigQueueSignal(::GetLRBThreadPool(), this, 0, (WPARAM)CHANNEL_LRBMSG, 0);
//}
//
//void CLRBManager::OnSendToLRB()
//{
//	CLRBLink* pLink = (CLRBLink*)GetSocket();
//	if (!pLink) 
//		return;
//	
////	DWORD s_size = pLink->GetSendBufQSize();
////	if(s_size > 1024 * 100)
////		theErr.LOG(2, "LRBManager's Send Buffer Queue size : [%d] byte ", s_size);
//
//	GBuf buf;
//	while(m_LRBMsgQ.Pop(buf))
//	{
//		pLink->DoSend(buf);
//	}
//}
//
//void CLRBManager::MakeHeader(MessageHeader & header, WORD wTargetAddr, char cRtype, char ctype)
//{
//	header.m_cMessageType = ctype;
//	header.m_cRoutingType = cRtype;
//	header.m_dwSourceTypeID = MAKELONG(SVCTYP_ANY, SVCCAT_CHS);
//	header.m_dwTargetTypeID = MAKELONG(SVCTYP_ANY, wTargetAddr);
//	header.m_dwSourceAddr = m_LRBADDRESS;
//	header.m_dwTargetAddr = MAKELONG(SVCTYP_ANY, wTargetAddr);
//}
//
//void CLRBManager::MakeUniHeader(MessageHeader & header, DWORD dwTargetAddr)
//{
//	header.m_cMessageType = MSGTYPE_ACK;
//	header.m_cRoutingType = ROUTTYPE_TARGET_UNICASTING;
//	header.m_dwSourceTypeID = MAKELONG(SVCTYP_ANY, SVCCAT_CHS);
//	header.m_dwTargetTypeID = MAKELONG(SVCTYP_ANY, SVCCAT_LB);
//	header.m_dwSourceAddr = m_LRBADDRESS;
//	header.m_dwTargetAddr = dwTargetAddr;
//}
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// FromLB
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//void CLRBManager::OnRChannelListAns(MsgLBCHS_RChannelListAns *pPld)
//{	
//	ASSERT(pPld);
//	if(!pPld)
//	{
//		theErr.LOG(1, "+++++ ASSERT :OnRChannelListAns +++++");
//		return;
//	}
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(pPld->m_channelID, &spChannel))
//		return;
//
//	theErr.LOG(3, "Send CHS->Cli OnRChannelListAns");
//
//	PayloadCHSCli msg(PayloadCHSCli::msgChannelListAns_Tag,	
//		MsgCHSCli_ChannelListAns(
//			pPld->m_lstChannelInfoList, 
//			pPld->m_lTotalUserCount, 
//			pPld->m_lTotalChannelCount, 
//			0L
//		)
//	);
//	GBuf buf;
//	::LStore(buf, msg);
//
//	spChannel->PostChannelListAns(pPld->m_lUSN, buf);
//}
//
//void CLRBManager::OnGLSInfoAns(MsgLBCHS_GLSInfoAns *pPld)
//{
//	ASSERT(pPld);
//	if(!pPld)
//	{
//		theErr.LOG(1, "+++++ ASSERT :OnGLSInfoAns +++++");
//		return;
//	}
//	if(pPld->m_lType == CONNECTTYPE_DIRECT)	
//	{
//		RCPtrT<CChannel> spChannel;
//		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
//		{
//			DWORD dwNewGRIID = spChannel->GetRoomID();
//			DWORD logcAddr = m_LRBADDRESS;
//			PayloadCHSCli msg1(PayloadCHSCli::msgGRDirectCreateAns_Tag, 
//				MsgCHSCli_GRDirectCreateAns(pPld->m_nsapGLS, dwNewGRIID, logcAddr, " ", " ", CHS_SOK));
//
//			GBuf buf; 
//			::LStore(buf, msg1);
//			spChannel->PostGRDirectCreateAns(pPld->m_lUSN, buf);
//			
//			theErr.LOG(3, "Send CHS->Cli GRdirectCreateAns ");
//			return;
//		}
//
//	}
//	else if(pPld->m_lType == CONNECTTYPE_NORMAL)
//	{
//		RCPtrT<CChannel> spChannel;
//		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
//		{
//			DWORD dwNewGRIID = spChannel->GetRoomID();
//			DWORD logcAddr = m_LRBADDRESS;
//			PayloadCHSCli msg1(PayloadCHSCli::msgGLSInfoAns_Tag, 
//				MsgCHSCli_GLSInfoAns(pPld->m_nsapGLS, dwNewGRIID, logcAddr, CHS_SOK));
//
//			GBuf buf; 
//			::LStore(buf, msg1);
//			spChannel->PostGLSInfoAns(pPld->m_lUSN, buf);
//			
//			theErr.LOG(3, "Send CHS->Cli GLSInfoAns  =====");
//			return;
//		}
//	}
//	else 
//		theErr.LOG(2, "++++++ UnKnown LB->CHS GLSInfoAns Type =====");
//}
//
//
//void CLRBManager::OnRegisterServiceReq(MsgLBCHS_RegisterServiceReq * pPld, DWORD dwAddr)
//{
//	TLock lo(this);
//	CHSLBRegister reg_info(theChannelDir.GetCHSNsap(), m_LRBADDRESS, theChannelDir.m_lstChannelInfoForLBReg);
//	PayloadCHSLB pld(PayloadCHSLB::msgRegisterServiceAns_Tag, MsgCHSLB_RegisterServiceAns(reg_info));
//
//	SendToUniLB(pld, dwAddr);
//
//	// Boot State 설정.
//	//theControl.SetBootStateOut(CHS_START_LISTENER);
//	theReporter.SetAllDiffFlag();
//	theErr.LOG(2, "OnRegisterServiceReq : Listener Run by LB");
//	//Channel List를 전송.. by report signaling.. 
//}
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// from LB NACK
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void CLRBManager::OnRChannelListReq_NACK(MsgCHSLB_RChannelListReq *pPld)
//{	
//	if(!pPld)
//	{
//		theErr.LOG(1, "+++++ ASSERT : NACK : OnRChannelListAns +++++");
//		return;
//	}
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(pPld->m_channelID, &spChannel))
//		return;
//
//	theErr.LOG(3, "NACK : Send CHS->Cli OnRChannelListAns");
//
//	ChannelInfoList lstChannel;
//
//	PayloadCHSCli msg(PayloadCHSCli::msgChannelListAns_Tag,	
//		MsgCHSCli_ChannelListAns(
//			lstChannel, 
//			0L, 
//			0L, 
//			CHS_UNKNOWN
//		)
//	);
//	GBuf buf;
//	::LStore(buf, msg);
//
//	spChannel->PostChannelListAns(pPld->m_lUSN, buf);
//}
//
//void CLRBManager::OnGLSInfoReq_NACK(MsgCHSLB_GLSInfoReq *pPld)
//{
//	ASSERT(pPld);
//	if(!pPld)
//	{
//		theErr.LOG(1, "+++++ ASSERT : NACK : OnGLSInfoAns +++++");
//		return;
//	}
//	if(pPld->m_lType == CONNECTTYPE_DIRECT)	
//	{
//		RCPtrT<CChannel> spChannel;
//		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
//		{
//			DWORD logcAddr = m_LRBADDRESS;
//			NSAP nsapGLS;
//			PayloadCHSCli msg1(PayloadCHSCli::msgGRDirectCreateAns_Tag, 
//				MsgCHSCli_GRDirectCreateAns(nsapGLS, 0UL, logcAddr, " ", " ", CHS_UNKNOWN));
//
//			GBuf buf; 
//			::LStore(buf, msg1);
//			spChannel->PostGRDirectCreateAns(pPld->m_lUSN, buf);
//			
//			theErr.LOG(2, " NACK : Send CHS->Cli GRdirectCreateReq ");
//			return;
//		}
//
//	}
//	else if(pPld->m_lType == CONNECTTYPE_NORMAL)
//	{
//		RCPtrT<CChannel> spChannel;
//		if(theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
//		{
//			DWORD logcAddr = m_LRBADDRESS;
//			NSAP nsapGLS;
//			PayloadCHSCli msg1(PayloadCHSCli::msgGLSInfoAns_Tag, 
//				MsgCHSCli_GLSInfoAns(nsapGLS, 0UL, logcAddr, CHS_UNKNOWN));
//
//			GBuf buf; 
//			::LStore(buf, msg1);
//			spChannel->PostGLSInfoAns(pPld->m_lUSN, buf);
//			
//			theErr.LOG(2, " NACK : Send CHS->Cli GLSInfoReq ");
//			return;
//		}
//	}
//	else 
//		theErr.LOG(2, " NACK : UnKnown LB->CHS GLSInfoAns Type ");
//}
//
//void CLRBManager::OnRegisterServiceNtf_NACK(MsgCHSLB_RegisterServiceNtf * pPld)
//{
//	// 여기서는 boot Sequence가 바뀐다..
//	if(theControl.GetBootState() != CHS_REGING_LB)
//	{
//		theErr.LOG(1, "NACK : Invalid OnRegisterServiceNtf_NACK");
//		return;
//	}
//	//
//	//LB가 몇개인지 알지 못함, 
//	//
////	theControl.SetBootStateOut(CHS_WAITING_LB);
//	theControl.SetBootState(CHS_REG_LB);
//	theErr.LOG(1, " NACK : OnRegisterServiceNtf_NACK ");
//}
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// from AMS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//void CLRBManager::OnAMSAnnounceReq(MsgAMSCHS_AnnounceReq * pPld)
//{
//	ASSERT(pPld);
//	if(!pPld)
//	{
//		theErr.LOG(1, "+++++ ASSERT :OnAMSAnnounceReq +++++");
//		return;
//	}
//	switch(pPld->m_lAnnounceType)
//	{
//	case ANNOUNCE_ALLSERVICE:
//		theChannelDir.AMSAnnounceReqSystem(pPld->m_sAnnouncMsg);
//		break;
//	case ANNOUNCE_SSN:
//		theChannelDir.AMSAnnounceReqSSN(pPld->m_roomID, pPld->m_sAnnouncMsg);
//		break;
//	case ANNOUNCE_CATEGORY:
//		theChannelDir.AMSAnnounceReqCategory(pPld->m_roomID, pPld->m_sAnnouncMsg);
//		break;
//	case ANNOUNCE_CHANNEL:
//		theChannelDir.AMSAnnounceReqChannel(pPld->m_roomID, pPld->m_sAnnouncMsg);
//		break;
//	default:
//		break;
//	}
//	theErr.LOG(3, "AMS : OnAMSAnnounceReq %d:type", pPld->m_lAnnounceType);
//}
//
//void CLRBManager::OnAMSServiceStopReq(MsgAMSCHS_ServiceStopReq * pPld)
//{
//	ASSERT(pPld);
//	if(!pPld)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnAMSServiceStopReq+++++");
//		return;
//	}
//	theErr.LOG(1, "== AMS ServiceStopReq ==");
//	PayloadCHSAMS pld(PayloadCHSAMS::msgServiceStopAns_Tag, pPld);
//	pld.un.m_msgServiceStopAns->m_lErrorCode = 0L;
//
//#ifdef USE_DIRECTSEND
//	MessageHeader header;
//	MakeHeader(header, SVCCAT_AMS, ROUTTYPE_TARGET_UNICASTING);
//	header.m_dwTargetAddr = m_lAMSSourceAddr;
//
//	GBuf buf;
//	if(!::BStore(buf, pld))
//		return;
//	PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//	msg.un.m_msgService->m_header = header;
//	msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	DirectSendToLRB(msg);
//#else
//	SendToAMS(pld);		// neo direct
//#endif
//	theControl.StopControl();
//}
//void CLRBManager::OnAMSHeartBeatReq(MsgAMSCHS_HeartBeatReq * pPld)
//{
//	ASSERT(pPld);
//	if(!pPld)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnAMSHeartBeatReq+++++");
//		return;
//	}
//	LONG lErrCode = 0L;
//	if(!theHeartBeat.IsAlive())	// Error를 판단하고 flag를 reset.
//	{
//		lErrCode = AMSCHS_ERR_HEARTBEAT;
//		theErr.LOG(2, "Heartbeat Err : Listener or LRBManager is %x RunFlag", theHeartBeat.GetFlagValue());
//	} 
//
//	NSAP nsap = theChannelDir.GetCHSNsap();
//	LONG lLogical = m_LRBADDRESS;
//	LONG lServiceType = MAKELONG((WORD)theControl.m_lCHSID, (WORD)SVCCAT_CHS);
//	MsgCHSAMS_HeartBeatAns msg(pPld->m_lStep, nsap, lLogical, lServiceType, lErrCode);
//	PayloadCHSAMS pld(PayloadCHSAMS::msgHeartBeatAns_Tag, MsgCHSAMS_HeartBeatAns(msg));
//
//#ifdef USE_DIRECTSEND
//	MessageHeader header;
//	MakeHeader(header, SVCCAT_AMS, ROUTTYPE_TARGET_UNICASTING);
//	header.m_dwTargetAddr = m_lAMSSourceAddr;
//
//	GBuf buf;
//	if(!::BStore(buf, pld))
//		return;
//	PayloadServiceLRB msg2(PayloadServiceLRB::msgService_Tag);
//	msg2.un.m_msgService->m_header = header;
//	msg2.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	DirectSendToLRB(msg2);
//#else
//	SendToAMS(pld);		// neo direct
//#endif
//}
//
//#pragma oMSG("TODO - CRITICAL !!! - now send RoomList to AMS as empty table !!!")
//
//void CLRBManager::OnAMSRoomListReq(MsgAMSCHS_RoomListReq * pPld)
//{
//	RoomsInChannelList lstRoomsInCHS;
//	lstRoomsInCHS.clear();
//	LONG lErrCode = 0L;
//
//	//lErrCode = theChannelDir.GetRoomList(lstRoomsInCHS);
//
//	PayloadCHSAMS pld(PayloadCHSAMS::msgRoomListAns_Tag, MsgCHSAMS_RoomListAns(m_LRBADDRESS, lstRoomsInCHS, lErrCode));
//#ifdef USE_DIRECTSEND
//	MessageHeader header;
//	MakeHeader(header, SVCCAT_AMS, ROUTTYPE_TARGET_UNICASTING);
//	header.m_dwTargetAddr = m_lAMSSourceAddr;
//
//	GBuf buf;
//	if(!::BStore(buf, pld))
//		return;
//	PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//	msg.un.m_msgService->m_header = header;
//	msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	DirectSendToLRB(msg);
//#else
//	SendToAMS(pld);		// neo direct
//#endif
//}
//
//void CLRBManager::OnAMSStatisticReq(MsgAMSCHS_StatisticReq * pPld)
//{
//	StatInfoList lstStat;
//	lstStat.clear();
//	if(!theStatTable.GetStatInfo(lstStat))
//	{
//		theErr.LOG(1, "++++++ OnAMSStatisticReq : fail to get Statistics Infomation +++++++");
//		return;
//	}
//	PayloadCHSAMS pld(PayloadCHSAMS::msgStatisticAns_Tag, MsgCHSAMS_StatisticAns(pPld->m_lStep, m_LRBADDRESS, lstStat));
//#ifdef USE_DIRECTSEND
//	MessageHeader header;
//	MakeHeader(header, SVCCAT_AMS, ROUTTYPE_TARGET_UNICASTING);
//	header.m_dwTargetAddr = m_lAMSSourceAddr;
//
//	GBuf buf;
//	if(!::BStore(buf, pld))
//		return;
//	PayloadServiceLRB msg(PayloadServiceLRB::msgService_Tag);
//	msg.un.m_msgService->m_header = header;
//	msg.un.m_msgService->m_sRealMsg.assign((LPCTSTR)buf.GetData(), buf.GetLength());
//	DirectSendToLRB(msg);
//#else
//	SendToAMS(pld);		// neo direct
//#endif
//}
//
//void CLRBManager::OnComputerNameReq()
//{
//	string name;
//	if(!theControl.GetComputerName(name))
//	{
//		PayloadCHSAMS pld(PayloadCHSAMS::msgComputerNameAns_Tag, 
//			MsgCHSAMS_ComputerNameAns(theChannelDir.GetCHSNsap(), name, -1L));
//#pragma oMSG("Error code 정의 할것.. with AMS...")
//		SendToAMS(pld);		
//	}
//	PayloadCHSAMS pld(PayloadCHSAMS::msgComputerNameAns_Tag, 
//		MsgCHSAMS_ComputerNameAns(theChannelDir.GetCHSNsap(), name, 0L));
//	SendToAMS(pld);
//
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// from SMGW
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////void CLRBManager::OnSMGWUserLeaveNtf(MsgSMGWCHS_UserLeaveNtf * pPld)
////{
////	ASSERT(pPld);
////	if(!pPld)
////	{
////		theErr.LOG(1, "+++++ ASSERT : OnSMGWUserLeaveNtf+++++");
////		return;
////	}
////	
////	CChannelPtr spChannel;
////	if(!theChannelDir.GetChannel(pPld->m_channelID, &spChannel)) 
////		return;
////
////	string* pUID = new string( pPld->m_sUID.c_str() );
////	VALIDATE(pUID);
////
////	m_MsgCount.m_dwChatPart++;
////	::XsigQueueSignal(GetChannelThreadPool(), spChannel, 0, (WPARAM)CHANNEL_SMGWMSG, (LPARAM)pUID);
////}
////
////void CLRBManager::OnSMGWChannelInfoNtf(MsgSMGWCHS_ChannelInfoNtf * pPld)
////{
////	if(!pPld)
////	{
////		theErr.LOG(1, "+++++ ASSERT : OnSMGWChannelInfoNtf+++++");
////	//	return;
////	}
////	ASSERT(pPld);
////
////	LONG lSize = pPld->m_lstUID.size();
////	theErr.LOG(1, "SMGW ChangeChannelNtf : UID List size is [%d]", lSize);
////	if(lSize < 1) 
////		return;
////
////	ChannelID channelID(pPld->m_channelID.m_lSSN, pPld->m_channelID.m_dwCategory, pPld->m_channelID.m_dwGCIID);
////
////	RCPtrT<CChannel> spChannel;
////	if(!theChannelDir.GetChannel(channelID, &spChannel)) 
////	{
////		theErr.LOG(1, "++++ SMGWChangeChannelNtr : Not found channel ++++");
////		return;
////	}
////
////	LstUID * plistUID = new LstUID;
////	VALIDATE(plistUID);
////	plistUID->BCopy(pPld->m_lstUID);
////	
////	::XsigQueueSignal(GetChannelThreadPool(), spChannel, 0, (WPARAM)CHANNEL_SMGWCMD, (LPARAM)plistUID);
////}
