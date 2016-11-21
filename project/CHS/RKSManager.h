#pragma once

#include "LrbHandler.h"


enum RK_REQUEST_TYPE
{
	RK_REQ_JOINUSER,
	RK_REQ_GETRANK
};

static char* RK_REQ_STRING[] = {
	"JOIN_USER",
		"REQUESTS_RANK"
};


class RKSManager : public IXObject
{
public:
	IMPLEMENT_TISAFE(RKSManager);

	RKSManager() {m_dwRefCnt = 0; memset(m_dwRRS, 0xFF, sizeof(m_dwRRS));}				

	void OnRegisterNtf	(const LRBAddress &addr, LONG lSSN, LONG lType);
	void OnRecvAns	(LPXBUF pXBufID, LPXBUF pXBufPld);
	void OnNackReturned	(const LRBAddress &addr);

public:
	void SendRksReqMsgForJoin(LONG lSSN, LONG lRankType, ChannelID& CID, LONG lUSN);
	void SendRksReqMsgForReq(LONG lSSN, LONG lRankType, GBuf& buf, ChannelID& CID, LONG lSrcUSN );

	BOOL IsServiceSSN(DWORD ssn) 
	{
		return TRUE;
	}

	LRBAddress& SrcAddr()
	{
		return theLrbManager.GetMyAddress();
	}

	BOOL SendToLRB(const LRBAddress& src, const LRBAddress& dest, GBuf& buf)
	{
		theLrbManager.SendToRKS(src, dest, buf);
		return TRUE;
	}

	BOOL SetRankingMsgID(string strMsgType, long lUSN, ChannelID& cid, string& strrkmsg);
	BOOL GetRankingMsgID(string& strrkmsg, string& strMsgType, long& lUSN, ChannelID& cid);

protected:

protected:
	DWORD m_dwRefCnt;
	STDMETHOD_(ULONG, AddRef)() { return (DWORD)::InterlockedIncrement((LPLONG)&m_dwRefCnt);};
	STDMETHOD_(ULONG, Release)(){ return (DWORD)::InterlockedDecrement((LPLONG)&m_dwRefCnt);};
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

	BOOL IsSetRRS	(long lSSN, long lType) { lSSN %=500; /* Japan's SSN not small 500; koran's SSN not big 500;*/ return  *(m_dwRRS + (lSSN<<2) + (lType>>5)) &   (0x80000000U >>(lType%32)) ? TRUE : FALSE ;}
	VOID SetRRS		(long lSSN, long lType) { lSSN %=500; /* Japan's SSN not small 500; koran's SSN not big 500;*/		   *(m_dwRRS + (lSSN<<2) + (lType>>5)) |=  (0x80000000U >>(lType%32));}
	VOID ClearRRS	(long lSSN, long lType) { lSSN %=500; /* Japan's SSN not small 500; koran's SSN not big 500;*/		   *(m_dwRRS + (lSSN<<2) + (lType>>5)) &= ~(0x80000000U >>(lType%32));}

	DWORD m_dwRRS[500*4]; // Too Large. But only small memory is used when running. 16byte used per SSN. Korean used only 1 DWORD. Japan's 10 SSN use 40 DWORD.
};

extern RKSManager theRKSManager;