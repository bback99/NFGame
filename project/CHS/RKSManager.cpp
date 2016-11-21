#include "stdafx.h"
#include "rksmanager.h"

#ifndef _NFGAME_
#include "ADL/MsgCommonStruct.h"
#else
#include "ADL/MsgNFCommonStruct.h"
#endif

#include "LRBHandler.h"
#include "RankUtility.h"
#include "ADL/MsgSVRRTRKS.h"

RKSManager theRKSManager;

extern string GetParseData(string& sTarget, string sToken);

STDMETHODIMP_(void) RKSManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)		
{
}


VOID RKSManager::OnNackReturned(const LRBAddress &addr) 
{	
	theLog.Put(WAR_UK,"RRS_Warning, Nack Returned From :", addr.GetString());
	LONG lSSN = 0, lType = 0;
	sscanf(addr.ExtractIdentifier().c_str(), "%06d%02d",&lSSN, &lType);

	if ((lSSN < 0) || (999 < lSSN) || (lType < 0) || (99 < lType))
		return;

	ClearRRS(lSSN, lType);
}

VOID RKSManager::OnRegisterNtf (const LRBAddress &addr, LONG lSSN, LONG lType) 
{
	theLog.Put(INF_UK,"RRS_Inf, Register RRS addr:", addr.GetString(), ", lSSN:", lSSN, ", lType:", lType);

	if ((lSSN < 0) || (999 < lSSN) || (lType < 0) || (99 < lType))
		return;

	SetRRS(lSSN, lType);
}

void RKSManager::SendRksReqMsgForJoin(LONG lSSN, LONG lRankType, ChannelID& CID, LONG lUSN)
{
	GBuf rankBuf;
	Rank::CRankUtilityForClient RUtil;
	string strRankData;
	if ( RUtil.EncodeUSNToRankingReq(lSSN, lRankType, lUSN, 0, strRankData) )
		rankBuf.AddRight((LPVOID)strRankData.c_str(), strRankData.length());
	else 
	{
		LOG(INF_UK, "SendRksReqMsg"_LK, "BStore failed SSN = ", lSSN, ", TargetUSN = ", lUSN, ", RankType = ", lRankType);
		return;
	}

	MsgSvrRTRKS_Req msg;
	SetRankingMsgID(string(RK_REQ_STRING[RK_REQ_JOINUSER]), lUSN, CID, msg.m_strMsgID);
	msg.m_buf = rankBuf;	

	PayloadSvrRTRKS pld(PayloadSvrRTRKS::msgReq_Tag, msg);	
	GBuf pldBuf;
	if (!pld.BStore(pldBuf))
	{
		LOG(INF_UK, "SendRksReqMsg"_LK, "BStore failed2 SSN = ", lSSN, ", TargetUSN = ", lUSN, ", RankType = ", lRankType);
		return;
	}
	//----------------------------------------------------//
	// Check RRS
	if (IsSetRRS(lSSN, lRankType) == FALSE)
	{
		theLog.Put(DEV,"RKSManager::SendMsg() Faile. IsSetRRS(SSN:",lSSN,", Type:",lRankType,") return FALSE");
		return ;
	}

	// Make dest Address
	LRBAddress dest;	
	char szAddr[24];	
	sprintf(szAddr,"%cRRS%06d%02d", 'A',  lSSN, lRankType);
	dest.SetAddress(szAddr);

	// Send
	theLrbManager.SendToRKS(theLRBHandler.GetMyAddress(), dest, pldBuf);
	//----------------------------------------------------//

	theLog.Put(DEV, "SendRksReqMsg(). ssn:", lSSN, ", type:", lRankType);
}

void RKSManager::SendRksReqMsgForReq(LONG lSSN, LONG lRankType, GBuf& buf, ChannelID& CID, LONG lSrcUSN )
{
}

BOOL RKSManager::SetRankingMsgID(string strMsgType, long lUSN, ChannelID& cid, string& strrkmsg)
{
	char strbuf[1024] = {0x00};
	string strChannelID;
	cid.GetInstanceID(strChannelID);	
	sprintf(strbuf, "%s|%d|%s", strMsgType.c_str(), lUSN, strChannelID.c_str());
	strrkmsg = strbuf;
	return TRUE;
}
BOOL RKSManager::GetRankingMsgID(string& strrkmsg, string& strMsgType, long& lUSN, ChannelID& cid)
{
	strMsgType = GetParseData(strrkmsg, string("|"));
	string strUSN = GetParseData(strrkmsg, string("|"));
	lUSN = atol(strUSN.c_str());

	string strCHSID = GetParseData(strrkmsg, string("|"));
	cid.m_lSSN = atol(GetParseData(strCHSID, string(" ")).c_str());
	cid.m_dwCategory = atol(GetParseData(strCHSID, string(" ")).c_str());
	cid.m_dwGCIID = atol(GetParseData(strCHSID, string(" ")).c_str());

	return TRUE;
}
