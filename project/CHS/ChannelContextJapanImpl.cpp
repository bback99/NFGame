#include "stdafx.h"
/**
#include "ChannelContextJapanImpl.h"

#ifdef _GLOBALIZATION_

#include "Channel.h"


CChannelContextJapanImpl theChannelContextJapanImpl;


#define BEGIN_SWITCH_JI		switch(pld.mTagID) {
#define END_SWITCH_JI		default: \
	LOG(INF_UK, "CHS_CChannelContextJapanImpl"_LK, "+++++ received unknown message from Client +++++ : PID=[", pld.mTagID, "]"); \	
	break; \
}
#define BEGIN_CASE_JI(ADATA)	case PayloadCliCHS_japan::msg##ADATA##_Tag:
#define MIDDLE_CASE_JI(ADATA)	; break; \
	case PayloadCliCHS_japan::msg##ADATA##_Tag:
#define END_CASE_JI	break;	

string GetStringRoomID(RoomID rid)
{
	return ::format("SSN/CID/CAT/RID = %d/%d/%d/%d", rid.m_lSSN, rid.m_dwGCIID, rid.m_dwCategory, rid.m_dwGRIID);	
}

CChannelContextJapanImpl::CChannelContextJapanImpl()
{

}

CChannelContextJapanImpl::~CChannelContextJapanImpl()
{

}

BOOL CChannelContextJapanImpl::ProcessMessage(const CJapanDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, string& strMsg)
{
	GBuf buf((LPVOID)(strMsg.c_str()), strMsg.length());
	PayloadCliCHS_japan pld;
	pld.BLoad(buf);

	BEGIN_SWITCH_JI
		BEGIN_CASE_JI(PckRoomJoinReq)
			OnPckRoomJoinReq(jdmd, channelCtx, user, pld.un.m_msgPckRoomJoinReq);
		MIDDLE_CASE_JI(PckRoomOutReq)
			OnPckRoomOutReq(jdmd, channelCtx, user, pld.un.m_msgPckRoomOutReq);
		END_CASE_JI
	END_SWITCH_JI
 
	return TRUE;
}

BOOL CChannelContextJapanImpl::OnPckRoomJoinReq(const CJapanDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, MsgCliCHSJapan_PckRoomJoinReq* pMsg)
{
	/////////// 사용자가 혹시라도 다른방에 옮겨갈때 ///////////
	/////////// MsgCliCHSJapan_PckRoomOutReq 메시지 안보내고 나갈수도 있으니 추가.	
	DWORD dwGRIID = GetUserRoomIDFromMap(user.GetUSN());
	DWORD dwGRIID_InRoomList = channelCtx.FinUserInRoom(user.GetUSN());
	if(dwGRIID != 0 || dwGRIID_InRoomList != 0)
	{
		printf(" ~~~~~~~~~ Exist room : [%d]\n", dwGRIID);
		DeleteUserFromMap(user.GetUSN());
		if(dwGRIID != 0)
			channelCtx.OnUserRoomLeave(dwGRIID, user.GetUSN());
		if(dwGRIID_InRoomList != 0)
			channelCtx.OnUserRoomLeave(dwGRIID_InRoomList, user.GetUSN());
	}
	///////////////////////////////////////////////////////////

	int nRet = channelCtx.IsEmptyRoom(pMsg->m_roomID.m_dwGRIID);
	if(nRet == 0)
	{
		MsgCHSCliJapan_PckRoomJoinAns mpjans(pMsg->m_roomID, user.GetUSN(), 0);
		PayloadCHSCli_japan pld_ans(PayloadCHSCli_japan::msgPckRoomJoinAns_Tag, mpjans);
		GBuf buf;
		pld_ans.BStore(buf);
		string strdata;
		strdata.assign((char*)buf.GetData(), buf.GetLength());
		MsgCHSCli_RegionMsgAns msgregans(jdmd.m_nRegion, jdmd.m_nMsgID, jdmd.m_nMsgType, strdata, 0);
		PayloadCHSCli pldchscli(PayloadCHSCli::msgRegionMsgAns_Tag, msgregans);		
		channelCtx.SendToUser(user.GetUSN(), pldchscli);

		channelCtx.OnUserRoomJoin(pMsg->m_roomID.m_dwGRIID, user.GetUserData());
		InsertUserFromMap(user.GetUSN(), pMsg->m_roomID.m_dwGRIID);
	}
	else
	{
		if(nRet == 101)
			LOG(INF_UK, "CHS_InsertUserInRoom_Error"_LK, "[InsertUserInRoom] User is not only 1");
		else
			LOG(INF_UK, "CHS_InsertUserInRoom_Error"_LK, "[InsertUserInRoom] Not room : ", ::GetStringRoomID(pMsg->m_roomID).c_str());

		MsgCHSCliJapan_PckRoomJoinAns mpjans(pMsg->m_roomID, user.GetUSN(), nRet);
		PayloadCHSCli_japan pld_ans(PayloadCHSCli_japan::msgPckRoomJoinAns_Tag, mpjans);
		GBuf buf;
		pld_ans.BStore(buf);
		string strdata;
		strdata.assign((char*)buf.GetData(), buf.GetLength());
		MsgCHSCli_RegionMsgAns msgregans(jdmd.m_nRegion, jdmd.m_nMsgID, jdmd.m_nMsgType, strdata, 100);
		PayloadCHSCli pldchscli(PayloadCHSCli::msgRegionMsgAns_Tag, msgregans);
		channelCtx.SendToUser(user.GetUSN(), pldchscli);
	}
	return TRUE;
}

BOOL CChannelContextJapanImpl::OnPckRoomOutReq(const CJapanDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, MsgCliCHSJapan_PckRoomOutReq* pMsg)
{
	RoomInfoInChannel rinfo;
	if(!channelCtx.FindRoom(pMsg->m_roomID.m_dwGRIID))
		return FALSE;

	channelCtx.OnUserRoomLeave(pMsg->m_roomID.m_dwGRIID, user.GetUSN());
	DeleteUserFromMap(user.GetUSN());
	{
		MsgCHSCliJapan_PckRoomOutAns mpoans(pMsg->m_roomID, 0);
		PayloadCHSCli_japan pld_ans(PayloadCHSCli_japan::msgPckRoomOutAns_Tag, mpoans);
		GBuf buf;
		pld_ans.BStore(buf);
		string strdata;
		strdata.assign((char*)buf.GetData(), buf.GetLength());
		MsgCHSCli_RegionMsgAns msgregans(jdmd.m_nRegion, jdmd.m_nMsgID, jdmd.m_nMsgType, strdata, 0);
		PayloadCHSCli pldchscli(PayloadCHSCli::msgRegionMsgAns_Tag, msgregans);
		channelCtx.SendToUser(user.GetUSN(), pldchscli);		
	}
	return TRUE;
}

void CChannelContextJapanImpl::InsertUserFromMap(LONG lUSN, LONG lGRIID)
{
	TLock lo(this);
	m_mapUserRoomID[lUSN] = lGRIID;
}

void CChannelContextJapanImpl::DeleteUserFromMap(LONG lUSN)
{
	TLock lo(this);
	m_mapUserRoomID.erase(lUSN);
}

DWORD CChannelContextJapanImpl::GetUserRoomIDFromMap(LONG lUSN)
{
	TLock lo(this);
	MAP_USERROOMID::iterator iter = m_mapUserRoomID.find(lUSN);
	if(iter == m_mapUserRoomID.end())
		return 0;
	else
		return iter->second;
}
#endif
*/