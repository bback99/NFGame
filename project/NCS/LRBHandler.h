#pragma once

#include <NF/ADL/MsgCHSNCS.h>
#include <ADL/MsgGBSCli.h>
#include <NF/ADL/MsgNLSCli.h>
#include <NF/ADL/MsgNCSNGS.h>
#include <NF/ADL/MsgNASCli.h>


const int SYNC_CALL_TIMEOUT = 5000;		//	5 초


class CLRBHandler: public XLRBHandlerBase
{
public:
	typedef XLRBHandlerBase TBase;
public:
	CLRBHandler();
	virtual ~CLRBHandler();
public:
	//	필수 구현
	virtual void OnXLRBError(LONG lError);
	virtual void OnXLRBRegister(LONG lErrorCode, LRBAddress& addr);
	virtual void OnXLRBRcvMsg(const DWORD dwMID,const adl::LRBAddress& src, const adl::LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);
	virtual void OnXLRBTerminateNtf(ListAddress& lstAddr);
	virtual void OnXLRBUnknownEvent(UINT pEvent, LONG lErrorCode, LPXBUF ppXBuf, LRBAddress& srcAddr, LRBAddress& destAddr);

	// Address 관련 
private:
	BOOL		m_bRegMultiAddr;
	BOOL		m_bRegAnyAddr;
	DWORD		m_dwServiceTypeID;			// 자신이 지정해야 하는 서비스 타입 id 
	LRBAddress  m_serverAddr;				// UNICAST 를 위한 주소
	LRBAddress  m_serviceAddr[2];			// MULTI , ANYCAST 를 위한 주소
	LRBAddress	m_NLSMulticastAddr;
	LRBAddress	m_CHSMulticastAddr;			
	LRBAddress  m_NGSMulticastAddr;
	LRBAddress	m_NASMulticastAddr;

	string m_sComputerName;
public:
	void OnSendNCSStart();
	void SetComputerName(LPCSTR szName, DWORD dwLength);
	DWORD GetTypeID() { return m_dwServiceTypeID; }

	// address get/set
	const LRBAddress& GetMyAddr() const { return m_serverAddr; }
	const LRBAddress& GetNLSAddr() const { return m_NLSMulticastAddr; }
	const LRBAddress& GetCHSAddr() const { return m_CHSMulticastAddr; }
	const LRBAddress& GetNGSAddr() const { return m_NGSMulticastAddr; }
	const LRBAddress& GetNASAddr() const { return m_NASMulticastAddr; }
public:
	BOOL IsRegistered();
	BOOL RegisterAddress();
	void SetSvrAddress(void);

	// operation
	BOOL Init();
	BOOL Run();
	void Stop();

	// Initialize 관련 
	BOOL FinalInit(void);
	BOOL m_bFICalled;

	// BLRBMsgHandlerEx.cpp
public:
	// src 를 보고 핸들러를 구분한다 
	void OnRcvCHSMsg(const PayloadInfo& pldInfo, GBuf& buf);
	void OnRcvNGSMsg(const PayloadInfo& pldInfo, GBuf& buf);
	void OnRcvGBSMsg(const PayloadInfo& pldInfo, GBuf& buf);
	void OnRcvNASMsg(const PayloadInfo& pldInfo, GBuf& buf);

public:
	void SendToCHS(const PayloadNCSCHS& pld, const PayloadInfo& pldInfo);
	void SendToNGS(const PayloadNCSNGS& pld, const PayloadInfo& pldInfo);

	void SendToNLS(const PayloadNLSCLI& pld, const PayloadInfo& pldInfo);
	void SendToNLS(const LRBAddress& src, const PayloadInfo& dest, PayloadCLINLS& pld);

	void SendToNAS(const PayloadCLINAS& pld, const PayloadInfo& pldInfo);

	void SendToGBS(const PayloadGBSCLI& pld, const PayloadInfo& pldInfo);
	void SendToGBS(const LRBAddress& src, const PayloadInfo& dest, PayloadCLIGBS& pld);
	
	void ConnectorSendTo(DWORD dwMID, GBuf& buf, WORD wProtocol, LRBAddress& SrcAddr, const LRBAddress& DestAddr);

	//CHS
	BOOL OnRegisterServiceNtf(MsgCHSNCS_RegisterServiceNtf* pMsg, const PayloadInfo& pldInfo);
	BOOL OnRegisterServiceAns(MsgCHSNCS_RegisterServiceAns* pMsg, const PayloadInfo& pldInfo);
	BOOL OnChannelListReq(MsgCHSNCS_RChannelListReq* pMsg, const PayloadInfo& pldInfo);		// any channel or all category
	BOOL OnCHSInfoNtf(MsgCHSNCS_CHSInfoNtf* pMsg);
	BOOL OnNGSInfoReq(MsgCHSNCS_NGSInfoReq* pMsg, const PayloadInfo& pldInfo);
	BOOL OnNtfNFFriendAccept(MsgCHSNCS_NtfNFFriendAccept* pMsg, const PayloadInfo& pldInfo);
	//NGS
	BOOL OnRegisterServiceNtf(MsgNGSNCS_RegisterServiceNtf* pMsg, const PayloadInfo& pldInfo);
	BOOL OnRegisterServiceAns(MsgNGSNCS_RegisterServiceAns* pMsg, const PayloadInfo& pldInfo);
	BOOL OnNGSInfoNtf(MsgNGSNCS_NGSInfoNtf* pMsg);
	BOOL OnNtfNFFriendAdd(MsgNGSNCS_NtfNFFriendAdd* pMsg);
	BOOL OnNtfNFFriendAccept(MsgNGSNCS_NtfNFFriendAccept* pMsg);
	BOOL OnNtfNFLetterReceive(MsgNGSNCS_NtfNFLetterReceive* pMsg);

	// 서버 종료 메시지
	void OnSvrTerminateNtf(LRBAddress& lrbAddr);

protected:
	BOOL GetRealIPAddressFile(string& sIP);
	void SendToCharLobby(LONG lCSN, HSIGNAL hSignal, WPARAM w, LPARAM l);
};

extern CLRBHandler theLRBHandler;