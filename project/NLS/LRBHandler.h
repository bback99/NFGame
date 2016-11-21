//
// LRBHandler.h
//

#ifndef LRBHANDLER_H
#define LRBHANDLER_H

#include "Config.h"
#include "XLRBHandlerBase.h"
#include "LCInfoMap.h"

enum {
	COMPARE_SSN = 1, 
	COMPARE_CATEGORY, 
	COMPARE_GCIID
};

enum 
{
	ALLSERVER = 0, 
	NLS_UNI,	
	NLS_ANY,	
	NLS_MUL,	
	MAX_ADDR_CNT
};

enum 
{
	NLSME_UNI = 0,
	NLSME_MUL,
	NLSME_ANY,
	MAX_MYADDR_CNT
};

enum
{
	LRBGW_ERR_NOTEXISTUSER = -200,
	LRBGW_ERR_NOTEXISTSERVER = -201,
	LRBGW_ERR_NOTMATCHSSN = -202
};

enum
{
	LRBGW_RES_EXISTUSER = -1,
	LRBGW_RES_NOTEXISTUSER = 1,
};


//////////////////////////////////////////////////////////////////////
// CLRBLink

class CLRBLink : public GBufSocket
{
public:
	typedef GBufSocket TBase;
public:
	CLRBLink();
	virtual ~CLRBLink();
	NSAP GetnsapLRB() { return m_nsapLRB; }
	void SetnsapLRB(NSAP nsapLRB); 

	DWORD GetRecvBufSize();
	DWORD GetSendBufQSize();
protected:
	NSAP m_nsapLRB;
};



//////////////////////////////////////////////////////////////////////
// CLRBHandler

class CLRBHandler : public XLRBHandlerBase
{
	IMPLEMENT_TISAFEREFCNT(CLRBHandler)
public:
	typedef XLRBHandlerBase TBase;
	
	typedef pair<LRBAddress, LRBAddress> pairSvrT;
	typedef vector<pairSvrT> vecT;

private:
	CLRBHandler(const CLRBHandler&);
public:
	CLRBHandler();
	virtual ~CLRBHandler();
public:	
	BOOL Run(HTHREADPOOL hThreadPool);
	BOOL Stop();

	BOOL RegisterAddress();
	BOOL SetupLRBAddress();

	void MakeAddress(LRBAddress & addr, DWORD dwCastType, DWORD dwCategory);
	void MakeOtherAddress(LRBAddress & addr, string m_sServerName, DWORD dwCastType, DWORD dwCategory);

	virtual void OnXLRBError(LONG lError);
	virtual void OnXLRBRegister(LONG lErrorCode, LRBAddress& addr);
	virtual void OnXLRBRcvMsg(const DWORD dwMID, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	virtual void OnXLRBTerminateNtf(ListAddress& lstAddr);
	virtual void OnXLRBUnknownEvent(UINT pEvent, LONG lErrorCode, LPXBUF ppXBuf, LRBAddress& srcAddr, LRBAddress& destAddr);

public:
	void OnRcvServerMsg(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol, const LRBAddress& src);

		
public:
	void TerminateService(ListAddress& lstAddr);
	
	BOOL RecoverUserList();
	void ReqConnectedUserList();
	void ReqLCSRegister();
	void ReqDumpData(LRBAddress dwAddr);
	BOOL IsCompleteDumpDataRecv();
	void QueuingMsg(GBuf & buf);
	LONG GetWaitMsgQueueSize();
	void ResetAll();

	void SendToService(const PayloadNLSCLI& pld, const PayloadInfo& m_dest);

// Send function
protected:
	void PostSendToLRB(const GBuf & buf, LRBAddress & header);
	void SendToNLS(const PayloadNLSCLI& pld, const PayloadInfo& m_dest);
	void SendToClient(const PayloadNLSCLI& pld, const PayloadInfo& m_dest);
	void ConnectorSendTo(GBuf& buf, WORD wProtocol, LRBAddress& SrcAddr, const PayloadInfo& DestAddr);

public:
	//from Server
	void OnInsertCharacterReq(MsgCLINLS_InsertCharacterReq * pPld, const LRBAddress& src, const PayloadInfo& m_dest);
	void OnUpdateCharacterReq(MsgCLINLS_UpdateCharacterReq * pPld, const LRBAddress& src, const PayloadInfo& m_dest);
	void OnDeleteCharacterReq(MsgCLINLS_DeleteCharacterReq * pPld, const LRBAddress& src, const PayloadInfo& m_dest);
	void OnGetCharacterReq(MsgCLINLS_GetCharacterReq * pPld, const LRBAddress& src, const PayloadInfo& m_dest);
	void OnGetRegionStatReq(MsgCLINLS_GetRegionStatReq * pPld, const LRBAddress& src, const PayloadInfo& m_dest);
	void OnSetLeavingFlagNtf(MsgCLINLS_SetLeavingFlagNtf * pPld, const LRBAddress& src, const PayloadInfo& m_dest);
	void OnSetQuittingFlagNtf(MsgCLINLS_SetQuittingFlagNtf * pPld, const LRBAddress& src, const PayloadInfo& m_dest);
	void OnSetDisconnectedFlagNtf(MsgCLINLS_SetDisconnectedFlagNtf * pPld, const LRBAddress& src, const PayloadInfo& m_dest);
	void OnFindUserReq(MsgCLINLS_GetCharacterReq * pPld, const PayloadInfo & header);
	void OnRemUserReq(MsgCLINLS_DeleteCharacterReq * pPld, const PayloadInfo& header);

	void OnReqLocation(MsgCLINLS_ReqLocation * pPld, const LRBAddress& src, const PayloadInfo& m_dest);

public:
	DWORD m_dwLogicalAddr;

protected:
	char m_szLRBIP[16];
	LONG m_lLRBPort;

	LRBAddress  m_svrAddr;						// 서버 단위 주소
	LRBAddress  m_svcAddr[MAX_ADDR_CNT];		// send 대상, MULTI , ANYCAST 를 위한 주소
	LRBAddress	m_svcMyAddr[MAX_MYADDR_CNT];	// recv를 위해 LrB에 등록해야 할 Address
};

extern CLRBHandler theLRBHandler;

#endif //!LRBHANDLER_H
