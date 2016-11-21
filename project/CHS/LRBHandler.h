// LRBHANDLER.h
//

#ifndef LRBHANDLER_H
#define LRBHANDLER_H

#pragma once
#include "Common.h"

//#include "ADL/MsgLCSCommon2.h"
#include "NF/ADL/MsgCHSNGS.h"
#include <NF/ADL/MsgCHSNCS.h>
#include <NF/ADL/MsgNASCli.h>

#ifdef _CHSNLS
#include <NLSManager.h>
#endif


enum {
	COMPARE_SSN = 1, 
	COMPARE_CATEGORY, 
	COMPARE_GCIID
};
enum 
{
	ALLSERVER = 0, 
	NCS_MULTI,
	NCS_ANY,
	NGS_ANY, 
	BLS_ANY,
    IBB_ANY,
	RKS_MULTI,
	RKS_ANY,
	PCG_MULTI,
	MDS_MULTI,
	NAS_MULTI,
	NAS_ANY,
	MAX_ADDR_CNT = 12
};

enum E_REGISTER_MYADDR
{
	CHS_UNI = 0,
	CHS_MUL,
	CHS_ANY,
	RRC_MUL,
	MDC_MUL,
	MAX_MYADDR_CNT
};


class CLRBHandler : public XLRBHandlerBase
{
	IMPLEMENT_TISAFEREFCNT(CLRBHandler)
public:
	typedef XLRBHandlerBase TBase;
public:
	CLRBHandler(void);
	virtual ~CLRBHandler(void);

	BOOL Init();
	BOOL GetINIFileAddressInfo();
	BOOL GetINIFileFixedRoom();
	BOOL RunLRBHandler();
	void StopLRBHandler();

	BOOL TryServiceRegistToNCS(NSAP & nsapCHS);
	BOOL SendChannelIDListToNGS();

public: 
	virtual void OnXLRBError(LONG lError);
	virtual void OnXLRBRegister(LONG lErrorCode, LRBAddress& addr);
	virtual void OnXLRBRcvMsg(const DWORD dwMID, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	virtual void OnXLRBTerminateNtf(ListAddress& lstAddr);
	virtual void OnXLRBUnknownEvent(UINT pEvent, LONG lErrorCode, LPXBUF ppXBuf, LRBAddress& srcAddr, LRBAddress& destAddr);

public:
	// src 를 보고 핸들러를 구분한다 
	void OnRcvNGSMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvNCSMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvIBBMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	void OnRcvNASMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);

	// send message
	void MakeAddress(LRBAddress & addr, DWORD dwCastType, DWORD dwCategory);

	BOOL CheckStartUp() { return m_bLRBStartup; }

	BOOL RegisterAddress(const LRBAddress& addr);
protected:
	BOOL RegisterAddress();

	void OnRcvNGS(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);

	//from GLS
	void OnRoomCreateNtf(PayloadNGSCHS & pld, const PayloadInfo & dest);
	void OnUserRoomLeaveNtf(PayloadNGSCHS & pld);	
	void OnUserRoomJoinNtf(PayloadNGSCHS & pld, const PayloadInfo & dest);
	void OnRoomDeleteNtf(PayloadNGSCHS & pld)	;
	void OnRoomStatusChangeNtf(PayloadNGSCHS & pld);	
	void OnUserInfoInRoomChangeNtf(PayloadNGSCHS & pld)	;
	void OnChangeRoomOptionNtf(PayloadNGSCHS & pld);		
	void OnChangeGameOptionNtf(PayloadNGSCHS & pld);
	void OnGLSReconnectNtf(PayloadNGSCHS & pld, const PayloadInfo & dest);
	void OnChangeLoginStateNtf(PayloadNGSCHS & pld);
	void OnChannelIDListReq(PayloadNGSCHS & pld, const PayloadInfo & dest);
	void OnGameRoomListAns(PayloadNGSCHS & pld, const PayloadInfo & dest);
	void OnReqFreeRoomInfo(PayloadNGSCHS & pld, const PayloadInfo & dest);
	void OnNtfNFFriendAdd(PayloadNGSCHS& pld, const PayloadInfo& dest);
	void OnNtfNFFriendAccept(PayloadNGSCHS& pld, const PayloadInfo& dest);
	void OnNtfNFLetterReceive(PayloadNGSCHS& pld, const PayloadInfo& dest);
	

	//from NCS
	void OnRChannelListAns(MsgNCSCHS_RChannelListAns *pPld);
	void OnGLSInfoAns(MsgNCSCHS_NGSInfoAns *pPld);
	//void OnFRGLSInfoAns(MsgNCSCHS_FRGLSInfoAns *pPld);
	void OnRegisterServiceReq(MsgNCSCHS_RegisterServiceReq * pPld, const PayloadInfo & dest);
	void OnNtfNFFriendAdd(MsgNCSCHS_NtfNFFriendAdd* pPld);
	void OnNtfNFFriendAccept(MsgNCSCHS_NtfNFFriendAccept* pPld);
	void OnNtfNFLetterReceive(MsgNCSCHS_NtfNFLetterReceive* pPld);

	////from LB NACK
	void OnRecvNCS_NACK(GBuf & buf);
	void OnNGSInfoReq_NACK(MsgCHSNCS_NGSInfoReq *pPld);
	//void OnFRNGSInfoReq_NACK(MsgCHSNCS_FRNGSInfoReq *pPld);
	void OnRChannelListReq_NACK(MsgCHSNCS_RChannelListReq *pPld);
	void OnRegisterServiceNtf_NACK(MsgCHSNCS_RegisterServiceNtf * pPld);

	// from IBB
	void OnRecvIBB_NACK(GBuf & buf);

public:
	// Send
	void ConnectorSendTo(GBuf& buf, WORD wProtocol, LRBAddress& SrcAddr, const PayloadInfo& DestAddr);

	template <class PAYLOADT>
	BOOL AsyncSend(const PAYLOADT& pld, const LRBAddress* dest)
	{
		GBuf buf;
		if(!BStore(buf, pld))
		{
			LOG(ERR_UK, __FUNCTION__, "Fail to BStore");
			return FALSE;
		}
		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (LRBAddress*)&m_svrAddr, (LRBAddress*)dest);
		return TRUE;
	}

	// Send GLS
	void SendToNGS(const PayloadCHSNGS& pld, const PayloadInfo& dest);
	void SendToPCCNGS(const PayloadCHSNGS& pld, const PayloadInfo& dest);
	void SendToAllPCCNGS(const PayloadCHSNGS& pld);
	
	// Send ELB
	void SendToNCS(const PayloadCHSNCS& pld, const PayloadInfo& dest);
	void SendToNCS(const PayloadCHSNCS& pld);
	void SendToAllNCS(const PayloadCHSNCS& pld);

	// Send RealRanking
	void SendToRKS(const LRBAddress& src, const LRBAddress& dest, GBuf& buf);
	void SendToODBGW(const LRBAddress& dest, GBuf& buf);
	void SendToBMS(const LRBAddress& dest, GBuf& buf);

	// Send LCS
	void SendToNLS(const PayloadCLINLS& pld, const PayloadInfo& dest);
	void SendToNLS(const LRBAddress& src, const PayloadInfo& dest, PayloadCLINLS& pld);
	void SendToEchoMsg();

	// Send NAS
	void SendToNAS(const PayloadCLINAS& pld, const PayloadInfo& dest);
	void SendToNAS(const PayloadCLINAS& pld);
	void SendToAllNAS(const PayloadCLINAS& pld);

	//
	LRBAddress & GetMyAddress() { return m_svrAddr; }

	/// for LCS
	const LRBAddress& GetNLSAddr() const { return m_NLSMulticastAddr; }
	void SetNLSAddr(const LRBAddress& addr) { TLock lo(this); m_NLSMulticastAddr = addr; }

/////////////////////////////////////////////////////
private:
	BOOL m_bRegistered;
	DWORD		m_dwServiceTypeID;				// 자신이 지정해야 하는 서비스 타입 id 
	LRBAddress  m_svrAddr;						// 서버 단위 주소
	LRBAddress  m_svcAddr[MAX_ADDR_CNT];		// send 대상, MULTI , ANYCAST 를 위한 주소
	LRBAddress	m_svcMyAddr[MAX_MYADDR_CNT];	// recv를 위해 LrB에 등록해야 할 Address

	BOOL m_bLRBStartup;

	BOOL m_bIsFixedRoom;						// FixedRoom 체크하기 위한 값

	/// for LCS
	LRBAddress m_NLSMulticastAddr;
public:

protected:
	LRBMessageCnt m_MsgCount;	

};

extern CLRBHandler theLRBHandler;
#endif
