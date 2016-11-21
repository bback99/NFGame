#include "stdafx.h"
#include "ADL/MsgCommonStruct.h"
#include "LRBHandler.h"
#include "RKSManagerImple.h"
#include "ADL/MsgRTRKSCli.h"
#include "ADL/MsgSVRRTRKS.h"

extern string GetParseData(string& sTarget, string sToken);

RKSManagerImple theRKSManager;
// nothing to do
STDMETHODIMP_(void) RKSManagerImple::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)		
{
}

void RKSManagerImple::SendRksReqMsgForJoin(LONG lSSN, LONG lRankType, ChannelID& CID, LONG lUSN)
{
	MsgCliRTRKS_UserToRankingReq msgranking;
	msgranking.m_lSSN = lSSN;
	msgranking.m_lUSN = lUSN;  // i wonder this usn's ranking.
	msgranking.m_lRankingType = lRankType;
	
	PayloadCliRTRKS pldcli(PayloadCliRTRKS::msgUserToRankingReq_Tag, msgranking);

	GBuf rankBuf;
	if (!pldcli.BStore(rankBuf))
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

void RKSManagerImple::SendRksReqMsgForReq(LONG lSSN, LONG lRankType, GBuf& buf, ChannelID& CID, LONG lSrcUSN )
{
	string strChannelID;
	CID.GetInstanceID(strChannelID);

	MsgSvrRTRKS_Req msg;
	SetRankingMsgID(string(RK_REQ_STRING[RK_REQ_GETRANK]), lSrcUSN, CID, msg.m_strMsgID);
	msg.m_buf = buf;	

	PayloadSvrRTRKS pld(PayloadSvrRTRKS::msgReq_Tag, msg);	
	GBuf pldBuf;
	if (!pld.BStore(pldBuf))
	{
		LOG(INF_UK, "SendRksReqMsg"_LK, "BStore failed2 SSN = ", lSSN, ", RankType = ", lRankType, "SrcUSN = ", lSrcUSN);
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
	theLog.Put(DEV, "SendRksReqMsg(). ssn:", lSSN, ", type:", lRankType, ", ChannelID: ", strChannelID);
}

BOOL RKSManagerImple::SetRankingMsgID(string strMsgType, long lUSN, ChannelID& cid, string& strrkmsg)
{
	char strbuf[1024] = {0x00};
	string strChannelID;
	cid.GetInstanceID(strChannelID);	
	sprintf(strbuf, "%s|%d|%s", strMsgType.c_str(), lUSN, strChannelID.c_str());
	strrkmsg = strbuf;
	return TRUE;
}
BOOL RKSManagerImple::GetRankingMsgID(string& strrkmsg, string& strMsgType, long& lUSN, ChannelID& cid)
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
