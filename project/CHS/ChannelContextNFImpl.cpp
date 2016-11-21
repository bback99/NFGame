
#include "stdafx.h"

//#include "ChannelContextNFImpl.h"

#include "Channel.h"


CChannelContextNFImpl theChannelContextNFImpl;

#define BEGIN_SWITCH_NF	switch(pld.mTagID) {
#define END_SWITCH_NF		default: \
	LOG(INF_UK, "CHS_CChannelContextNFImpl"_LK, "+++++ received unknown message from Client +++++ : PID=[", pld.mTagID, "]"); \
	break; \
}
#define BEGIN_CASE_NF(ADATA) case PayloadCliCHS_NF::msg##ADATA##_Tag:
#define MIDDLE_CASE_NF(ADATA) ; break; \
	case PayloadCliCHS_NF::msg##ADATA##_Tag:
#define END_CASE_NF	break;	


string GetStringRoomID(RoomID rid)
{
	return ::format("SSN/CID/CAT/RID = %d/%d/%d/%d", rid.m_lSSN, rid.m_dwGCIID, rid.m_dwCategory, rid.m_dwGRIID);	
}

CChannelContextNFImpl::CChannelContextNFImpl()
{

}

CChannelContextNFImpl::~CChannelContextNFImpl()
{

}

BOOL CChannelContextNFImpl::ProcessMessage(const CNFDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, string& strMsg)
{
	GBuf buf((LPVOID)(strMsg.c_str()), strMsg.length());
	PayloadCliCHS_NF pld;
	pld.BLoad(buf);

 	BEGIN_SWITCH_NF
 		BEGIN_CASE_NF(GRListPageReq)
 			OnGRListPageReq(jdmd, channelCtx, user, pld.un.m_msgGRListPageReq);
 		END_CASE_NF
 	END_SWITCH_NF

	return TRUE;
}

// NF에서는 리스트를 보여주는 방의 옵션을 선택할 수 있기 때문에 기존의 WaitRoomList ADL은 사용하지 않고 따로 만들어서 사용한다.
BOOL CChannelContextNFImpl::OnGRListPageReq(const CNFDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, MsgCliCHS_GRListPageReq* pMsg)
{
	MsgCHSCli_GRListPageAns ans;
	NFRoomInfoInChannelList totlstRoom = channelCtx.GetRoomList();
	NFRoomInfoInChannelList lstPlayingRoom;
	long lCNT = 0;

	ForEachCElmt(NFRoomInfoInChannelList, totlstRoom, it, ij)
	{
		NFRoomInfoInChannel rinfo = *it;

		if(rinfo.m_lRoomState == RSTATE_PLAYER_READY)// && rinfo.m_roomOption.m_lMaxUserCnt > (LONG)(rinfo.m_lstUserBaseInfo.size())) 
		{
			// Page * 8개의 갯수를 체크한다.
			if (++lCNT > (pMsg->m_lPage-1) * 8)
				ans.m_lstRoomInfo.push_back(rinfo);
		}
		else
			lstPlayingRoom.push_back(rinfo);

		if(ans.m_lstRoomInfo.size() >= 8 || lstPlayingRoom.size() >= 8)
			break;
	}

	if (ans.m_lstRoomInfo.size() < 8 && lstPlayingRoom.size() >= 1)
	{
		ForEachCElmt(NFRoomInfoInChannelList, lstPlayingRoom, it, ij)
			ans.m_lstRoomInfo.push_back(*it);
	}

	// nf
	PayloadCHSCli_NF	pld(PayloadCHSCli_NF::msgGRListPageAns_Tag, ans);
	GBuf buf;
	pld.BStore(buf);
	
	string strdata;
	strdata.assign((char*)buf.GetData(), buf.GetLength());

	MsgCHSCli_RegionMsgAns	ans2(jdmd.m_nRegion, jdmd.m_nMsgID, jdmd.m_nMsgType, strdata, 0);

	// chs
	PayloadCHSCli		pld2(PayloadCHSCli::msgRegionMsgAns_Tag, ans2);
	channelCtx.SendToUser(user.GetUSN(), pld2);
	
	return TRUE;
}

void CChannelContextNFImpl::InsertUserFromMap(LONG lUSN, LONG lGRIID)
{
	TLock lo(this);
	m_mapUserRoomID[lUSN] = lGRIID;
}

void CChannelContextNFImpl::DeleteUserFromMap(LONG lUSN)
{
	TLock lo(this);
	m_mapUserRoomID.erase(lUSN);
}

DWORD CChannelContextNFImpl::GetUserRoomIDFromMap(LONG lUSN)
{
	TLock lo(this);
	MAP_USERROOMID::iterator iter = m_mapUserRoomID.find(lUSN);
	if(iter == m_mapUserRoomID.end())
		return 0;
	else
		return iter->second;
}