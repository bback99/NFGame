#pragma once


#include "RKSManager.h"
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

class RKSManagerImple: public RKSManager
{
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
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
};

extern RKSManagerImple theRKSManager;

