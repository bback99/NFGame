////
//// LRBMANAGER.h
////
//
//#ifndef LRBMANAGER_H
//#define LRBMANAGER_H
//
//#include "Common.h"
//
//enum {
//	COMPARE_SSN = 1, 
//	COMPARE_CATEGORY, 
//	COMPARE_GCIID
//};
//class CLRBManager;
//class CLRBManager : public XSigAdlLinkSingleT<CLRBLink, PayloadLRBService>
//{
//	IMPLEMENT_TISAFE(CLRBManager)
//public:
//	typedef XSigAdlLinkSingleT<CLRBLink, PayloadLRBService> TBase;
//	typedef PayloadLRBService TMsg;
//private:
//	CLRBManager(const CLRBManager&);
//public:
//	CLRBManager();
//	virtual ~CLRBManager();
//	STDMETHOD_(ULONG, AddRef)();
//	STDMETHOD_(ULONG, Release)();
//	ULONG GetRefCnt() { return m_dwRefCnt; }
//	virtual BOOL RunLRBManager();
//	virtual BOOL StopLRBManager();
//
//	BOOL TryServiceRegistToLB(NSAP & nsap);
//	void TryServiceRegistToLRB();
//	BOOL GetNextLRBAddr();
//	BOOL ChangeLRBAddr();
//	void RecoverLRBManager();
//	void OnRegisterServiceAns(MsgRegisterServiceAns & msg);
//protected:
//	virtual void DestroyLink(CLRBLink* pLink);
//	BOOL OnConnect(TSocket* pSocket, int nErrorCode);
//	virtual BOOL OnClose(TSocket* pSocket, int nErrorCode);
//	BOOL OnError(CLRBLink* pLink, long lEvent, int nErrorCode);
//	BOOL OnRcvMsg(TSocket* pSocket, PayloadLRBService& pld);
//	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);	
//
//public:
//	void OnRecvLRBMessage(PayloadLRBService& pld);
//	void OnRcvServerRegistAns(PayloadLRBService *pPld);
//	void RegisterInstanceReq(ChannelID channelID);
//	void RegisterInstanceAns(MsgRegisterInstanceAns & msg);
//	void DeRegisterInstanceReq(ChannelID channelID);
//	void DeRegisterInstanceAns(MsgDeregisterInstanceAns & msg);
//	void OnTerminateServiceNtf(MsgTerminateServiceNtf & msg);
//
//	// Send function
//	void DirectSendToLRB(const PayloadServiceLRB& pld);
//	void SendToAllGLS(PayloadCHSGLS& pld);
//	void SendToAMS(PayloadCHSAMS & pld);
//	void SendToLB(PayloadCHSLB& pld);
//	void SendToUniLB(PayloadCHSLB& pld, DWORD dwAddr);
//	void SendToAllLB(PayloadCHSLB& pld);
//	void OnSendToLRB();
//	void PostSendToLRB(const PayloadServiceLRB& pld);
//
//	void OnRecvGLS(const MsgService * pld);
//	void OnRecvCHS(const MsgService * pld);
//	void OnRecvLB(const MsgService * pld);
//	void OnRecvLB_NACK(const MsgService * pld);
//	void OnRecvAMS(const MsgService * pMsg);
//	void OnRecvSMGW(const MsgService * pMsg);
//
//	//from GLS
//	void OnRoomCreateNtf(PayloadGLSCHS & pld, DWORD dwLogicalAddr);
//	void OnUserRoomLeaveNtf(PayloadGLSCHS & pld);	
//	void OnUserRoomJoinNtf(PayloadGLSCHS & pld, DWORD dwLogicalAddr)	;
//	void OnRoomDeleteNtf(PayloadGLSCHS & pld)	;
//	void OnRoomStatusChangeNtf(PayloadGLSCHS & pld);	
//	void OnUserInfoInRoomChangeNtf(PayloadGLSCHS & pld)	;
//	void OnChangeAvatarNtf(PayloadGLSCHS & pld);
//	void OnChangeNickNameNtf(PayloadGLSCHS & pld);
//	void OnChangeItemNtf(PayloadGLSCHS & pld);
//	void OnChangeRoomOptionNtf(PayloadGLSCHS & pld);		
//	void OnChangeGameOptionNtf(PayloadGLSCHS & pld);
//	void OnGLSReconnectNtf(PayloadGLSCHS & pld, DWORD dwLogicalAddr);
//	void OnChangeLoginStateNtf(PayloadGLSCHS & pld);
//
//	//from LB
//	void OnRChannelListAns(MsgLBCHS_RChannelListAns *pPld);
//	void OnGLSInfoAns(MsgLBCHS_GLSInfoAns *pPld);
//	void OnRegisterServiceReq(MsgLBCHS_RegisterServiceReq * pPld, DWORD dwAddr = 0UL);
//
//	//from LB NACK
//	void OnGLSInfoReq_NACK(MsgCHSLB_GLSInfoReq *pPld);
//	void OnRChannelListReq_NACK(MsgCHSLB_RChannelListReq *pPld);
//	void OnRegisterServiceNtf_NACK(MsgCHSLB_RegisterServiceNtf * pPld);
//
//	//from AMS
//	void OnAMSAnnounceReq(MsgAMSCHS_AnnounceReq * pPld);
//	void OnAMSServiceStopReq(MsgAMSCHS_ServiceStopReq * pPld);
//	void OnAMSHeartBeatReq(MsgAMSCHS_HeartBeatReq * pPld);
//	void OnAMSRoomListReq(MsgAMSCHS_RoomListReq * pPld);
//	void OnAMSStatisticReq(MsgAMSCHS_StatisticReq * pPld);
//	void OnComputerNameReq();
//
//	//from SMGW
////	void OnSMGWUserLeaveNtf(MsgSMGWCHS_UserLeaveNtf * pPld);
////	void OnSMGWChannelInfoNtf(MsgSMGWCHS_ChannelInfoNtf * pPld);
//
//	void MakeHeader(MessageHeader & header, WORD wTargetAddr = 0, char cRtype = ROUTTYPE_ANYCASTING, char ctype = MSGTYPE_ACK);
//	void MakeUniHeader(MessageHeader & header, DWORD dwTargetAddr);
//protected:
//	DWORD m_dwRefCnt;
//public:
//	LONG m_LRBADDRESS;
//	GSafeBufQ m_LRBMsgQ;
//	LONG m_lAMSSourceAddr;
//
//	LONG m_lNextLRBIP;
//	LONG m_lRetryCnt;
//	BOOL m_bFirstStart;
//	BOOL m_bLRBRun;
//
//	GXSigTimer m_timerAlive2;
//
//protected:
//	LRBMessageCnt m_MsgCount;
//	string m_sLRBIP;
//};
//
//extern CLRBManager theLRBManager;
//#endif //!LRBMANAGER_H
