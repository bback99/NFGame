//
// LrbConnector.h
//

#ifndef LRBCONNECTOR_H
#define LRBCONNECTOR_H

#include "Common.h"


///////////////////////////////////////////////////////////////////////////////////
#define NGSINFO_INTERVAL 10000

///////////////////////////////////////////////////////////////////////////////////
class PayloadGLSAMS;
class PayloadNGSNCS;
class MsgLBGLS_RegisterServiceReq;
class MsgAMSGLS_RoomDeleteReq;
class MsgAMSGLS_AnnounceReq;
class MsgAMSGLS_ServiceStopReq;
class MsgAMSGLS_HeartBeatReq;
class MsgAMSGLS_StatisticReq;
class MsgAMSGLS_ComputerNameReq;
class MsgCHSNGS_ChangeAddrNtf;
class MsgCHSNGS_ChannelIDList;
class MsgCHSNGS_AnsFreeRoomList;
class MsgNCSNGS_NtfNFFriendAdd;
class MsgNCSNGS_NtfNFFriendAccept;
class MsgNCSNGS_NtfNFLetterReceive;

class PayloadNGSCHS;
class PayloadCLINLS;
class PayloadGLSGLS;
class PayloadCLINAS;

///////////////////////////////////////////////////////////////////////////////////
// CLrbHandler

class CLrbManager;

enum
{
	LRBMSG_SENDTOLRB = 1
};

class CLrbHandler : public XLRBHandlerBase
{
public:
	typedef XLRBHandlerBase TBase;
public:
	CLrbHandler(CLrbManager* pManager);
	virtual ~CLrbHandler();
	// Handling Method
public:
	virtual void OnXLRBError(LONG lError);
	virtual void OnXLRBRegister(LONG lErrorCode, LRBAddress& addr);
	virtual void OnXLRBRcvMsg(const DWORD dwMID, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	virtual void OnXLRBTerminateNtf(ListAddress& lstAddr);
	virtual void OnXLRBUnknownEvent(UINT pEvent, LONG lErrorCode, LPXBUF ppXBuf, LRBAddress& srcAddr, LRBAddress& destAddr);
protected:
	// 서버별 수신 메시지 처리 Routine
	void OnRcvAMSMsg(const PayloadInfo pldInfo, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvNCSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvCHSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvNLSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvNGSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvPLSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvIBBMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvRKSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvODBGWMsg(const PayloadInfo pldInfo, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvBCSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvNASMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol);
	
protected: // adl receive related
	// adl from AMS
	void OnRcvRoomDeleteReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_RoomDeleteReq* pMsg);
	void OnRcvAnnounceReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_AnnounceReq* pMsg);
	void OnRcvServiceStopReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_ServiceStopReq* pMsg);
	void OnRcvHeartBeatReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_HeartBeatReq* pMsg);
	void OnRcvStatisticsReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_StatisticReq* pMsg);
	void OnRcvComputerNameReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_ComputerNameReq* pMsg);
	// adl from NCS
	void OnRcvRegisterServiceReqFromLB(const LRBAddress& src);
	void OnRcvNtfFriendAdd(MsgNCSNGS_NtfNFFriendAdd* pMsg);
	void OnRcvFromNCSNtfFriendAccept(MsgNCSNGS_NtfNFFriendAccept* pMsg);
	void OnRcvFromNCSNtfNFLetterReceive(MsgNCSNGS_NtfNFLetterReceive* pMsg);
	// adl from CHS
	void OnRcvChangeCHSAddrFromCHS(MsgCHSNGS_ChangeAddrNtf* pMsg);	
	void OnRcvCannelIDListFromCHS(MsgCHSNGS_ChannelIDList* pMsg, const LRBAddress &src);
	void OnRcvAnsFreeRoomList(MsgCHSNGS_AnsFreeRoomList* pMsg);
	void OnRcvFromCHSNtfFriendAccept(MsgCHSNGS_NtfNFFriendAccept* pMsg);
	// adl from GLS
	void OnNGSMulticastNotify(LONG lSSN, GBuf& gBuf);
	// adl from MDS
private:
	CLrbManager* m_pManager;
};

// kukuta 아무곳에서 사용하는 곳이 없어 삭제 2월 점검 이후 주석도 지워 버릴 것!! typedef AutoPtrT<CLrbHandler> APLRBHANDLER;

///////////////////////////////////////////////////////////////////////////////////
// CLrbManager

class CLrbManager : public IXObject
{
	IMPLEMENT_TISAFE(CLrbManager)
protected:
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

public:
	CLrbManager();
	virtual ~CLrbManager();
public:
	// Control Method
	BOOL Init();
	BOOL Run();
	BOOL Stop();
public:
	void InitComplete()				{	m_evInit.SetEvent(); }
	BOOL OnLrbRegistered();
public:
	DWORD GetServiceTypeID()		{	return m_dwServiceTypeID;	}
//protected: // adl related
	// adl to CHS	
	void SendToCHS(const LRBAddress& src, const LRBAddress& dest, PayloadNGSCHS& pld, BOOL bInvalidAddrLog = TRUE);
	// adl to AMS
	void SendToAMS(const LRBAddress& src, const LRBAddress& dest, PayloadGLSAMS& pld);
	// adl to NCS
	void SendToNCS(const LRBAddress& src, const LRBAddress& dest, PayloadNGSNCS& pld);
	// adl to NLS
	void SendToNLS(const LRBAddress& src, const LRBAddress& dest, PayloadCLINLS& pld);
	// adl to NAS
	void SendToNAS(const LRBAddress& src, const LRBAddress& dest, PayloadCLINAS& pld);
	// adl to other GLS
	void SendToMGLS(const LRBAddress& src, const LRBAddress& dest, PayloadGLSGLS& pld);
	// adl to PLS
	void SendToPLS(const LRBAddress& src, const LRBAddress& dest, GBuf& buf);
	// to IBB
	void SendToIBB(const LRBAddress& src, const LRBAddress& dest, GBuf& buf);
	// to RKS
	BOOL SendToRKS(const LRBAddress& src, const LRBAddress& dest, GBuf& buf);
	// to MDS

	template <class PAYLOADT>
	BOOL AsyncSend(const PAYLOADT& pld, const LRBAddress* src, const LRBAddress* dest)
	{
		if(CASTTYPE_INVALID == dest->GetCastType())
		{
			theLog.Put(WAR_UK, "NGS_theLRBManager_Error"_COMMA, "Invalid Destination Message: ");
			return FALSE;
		}

		GBuf buf;
		if(!BStore(buf, pld))
		{
			theLog.Put(ERR_UK, __FUNCTION__, "::BStore Error");
			return FALSE;
		}
		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (LRBAddress*)src, (LRBAddress*)dest);
		return TRUE;
	}

	void SendToServer(const LRBAddress& src, const LRBAddress& dest, GBuf &buf);

	void SendHeartBeatAnsToAMS(const LRBAddress& src, const LRBAddress& dest, LONG lStep);
	void SendStatisticsAnsToAMS(const LRBAddress& src, const LRBAddress& dest, LONG lStep);
	void SendComputerNameAnsToAMS(const LRBAddress& src, const LRBAddress& dest);	

	void SendRegisterServiceNtfToNCS();
	void SendRegisterServiceAnsToLB(const LRBAddress& src);
	void NotifyNGSInfoToNCS(const LRBAddress& src);

	void RegisterMultiAddr(char *szMAddr);
	BOOL RegisterAddress(const LRBAddress& addr);
protected:
	GXSigTimer m_timerNGSInfo;
	DWORD m_dwRefCnt;

private:
	GEvent			m_evInit;
	DWORD			m_dwServiceTypeID;			// Server Type
	BOOL			m_bInitialized;
	CLrbHandler*	m_pHandler;
public:
	friend class CLrbHandler;
};

extern CLrbManager theLrbManager;

#endif //!LRBCONNECTOR_H
