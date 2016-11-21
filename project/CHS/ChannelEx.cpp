//
// ChannelEx.cpp
//

#include "stdafx.h"
#include "Channel.h"

#include "CHSInfoDir.h"

//#pragma warning(disable:4706)
//#include "MsgNDLGameDB2.h"
//#pragma warning(default:4706)
#ifdef _PMS_CODE_SEPERATION_
#include "StatisticsTable.h"
#endif


/*
// 파칭코는 채널과 게임에 동시에 접속한다.
// 게임에 접속할 경우, 채널+게임에 중복 접속이 되서 ELB에 2명으로 보이게 됨.
// 그래서 void CChannelContext::OnUserRoomJoin(DWORD dwRSN, const UserBaseInfo& uinfo) 에서 사용자 추가시 체크해야 함.
// 501 : 파칭코.
*/

extern string GetParseDatawithTokenRvs(string& sTarget, string sToken);
extern string GetParseData(string& sTarget, string sToken);
///////////////////////////////////////////////////////////////////////////////////
// CChannel
void CChannel::PushNGSMessage(GBuf & msg)
{
	m_Queue.Push(msg);

	::XsigQueueSignal(::GetThreadPool(), this, CHANNELSIGNAL_FROMGLS,  (WPARAM)0, (LPARAM)0);
}

BOOL CChannel::PopGLSMessage(GBuf & msg)
{
	return m_Queue.Pop(msg);
}

void CChannel::OnSignalEx(HSIGNAL hObj, WPARAM wP, LPARAM lP)
{
	TLock lo(this);
	GBuf buf;
	if(!PopGLSMessage(buf))
		return;
	PayloadNGSCHS msg;
	VALIDATE(::BLoad(msg, buf));

	switch(msg.mTagID)
	{
	case PayloadNGSCHS::msgRoomCreateNtf_Tag:
		OnRoomCreateFromNGS(*msg.un.m_msgRoomCreateNtf);
		break;
	case PayloadNGSCHS::msgRoomDeleteNtf_Tag:
		OnRoomDeleteFromNGS(*msg.un.m_msgRoomDeleteNtf);
		break;
	case PayloadNGSCHS::msgRoomStatusChangeNtf_Tag:
		OnRoomStatusChangeFromNGS(*msg.un.m_msgRoomStatusChangeNtf);
		break;
	case PayloadNGSCHS::msgUserRoomJoinNtf_Tag:
		OnUserRoomJoinFromNGS(*msg.un.m_msgUserRoomJoinNtf);
		break;
	case PayloadNGSCHS::msgUserRoomLeaveNtf_Tag:
		OnUserRoomLeaveFromNGS(*msg.un.m_msgUserRoomLeaveNtf);
		break;
	case PayloadNGSCHS::msgUserInfoInRoomChangeNtf_Tag:
		OnUserRoomChangeFromNGS(*msg.un.m_msgUserInfoInRoomChangeNtf);
		break;
	case PayloadNGSCHS::msgChangeRoomOptionNtf_Tag:
		OnChangeRoomOptionFromNGS(*msg.un.m_msgChangeRoomOptionNtf);
		break;
	case PayloadNGSCHS::msgChangeGameOptionNtf_Tag:
		OnChangeGameOptionFromNGS(*msg.un.m_msgChangeGameOptionNtf);
		break;
	case PayloadNGSCHS::msgChangeLoginStateNtf_Tag:
		OnChangeLoginStateFromNGS(*msg.un.m_msgChangeLoginStateNtf);
		break;
	case PayloadNGSCHS::msgGameRoomListAns_Tag:
		OnGameRoomListAnsFromNGS(*msg.un.m_msgGameRoomListAns);
		break;
	case PayloadNGSCHS::msgReqFreeRoomInfo_Tag:
		OnReqFreeRoomInfoFromNGS(*msg.un.m_msgReqFreeRoomInfo);
		break;
	default:
		LOG(INF_UK, "CHS_CChannelEx_OnSignalEx"_LK, "Receive Unknown GLS Msg \n");
		break;
	}
}

// from GLS
void CChannel::OnRoomCreateFromNGS(MsgNGSCHS_RoomCreateNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnRoomCreate(msg.m_roomID.m_dwGRIID, msg.m_nfRoomBaseInfo);
}

void CChannel::OnRoomDeleteFromNGS(MsgNGSCHS_RoomDeleteNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnRoomDelete(msg.m_roomID.m_dwGRIID);
}

void CChannel::OnRoomStatusChangeFromNGS(MsgNGSCHS_RoomStatusChangeNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnRoomStatusChange(msg.m_roomID.m_dwGRIID, msg.m_lRoomState);
}

void CChannel::OnUserRoomJoinFromNGS(MsgNGSCHS_UserRoomJoinNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnUserRoomJoin(msg.m_roomID.m_dwGRIID, msg.m_nfUserBaseInfo);
}

void CChannel::OnUserRoomLeaveFromNGS(MsgNGSCHS_UserRoomLeaveNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnUserRoomLeave(msg.m_roomID.m_dwGRIID, msg.m_lCSN);
}

void CChannel::OnUserRoomChangeFromNGS(MsgNGSCHS_UserInfoInRoomChangeNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnUserRoomChange(msg.m_roomID.m_dwGRIID, msg.m_lstUserGameData);
}

void CChannel::OnChangeRoomOptionFromNGS(MsgNGSCHS_ChangeRoomOptionNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnChangeRoomOption(msg.m_roomID.m_dwGRIID, msg.m_nfRoomOption);
}

void CChannel::OnChangeGameOptionFromNGS(MsgNGSCHS_ChangeGameOptionNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnChangeGameOption(msg.m_roomID.m_dwGRIID, msg.m_sGameOption);
}

void CChannel::OnChangeLoginStateFromNGS(MsgNGSCHS_ChangeLoginStateNtf & msg)
{
	TLock lo(this);

	m_pChannelContext->OnChangeLoginState(msg.m_roomID.m_dwGRIID, msg.m_lCSN, msg.m_lLoginState);
}

void CChannel::OnGameRoomListAnsFromNGS(MsgNGSCHS_GameRoomListAns & msg)
{
	TLock lo(this);

	m_pChannelContext->OnAddRoomList(msg.m_channelID, msg.m_lstRoomInfo);
}

void CChannel::OnReqFreeRoomInfoFromNGS(MsgNGSCHS_ReqFreeRoomInfo & msg)
{
	TLock lo(this);

	m_pChannelContext->OnReqFreeRoomInfo(msg.m_lCSN, msg.m_roomID, msg.m_addrNGS);
}

BOOL CChannel::GetWaitRoomList(ChannelID cid, NFRoomInfoInChannelList& lstRoom, UINT nGetCnt, LONG lType)
{
	return m_pChannelContext->GetWaitRoomList(cid, lstRoom, nGetCnt, lType);	
}

///////////////////////////////////////////////////////////////////////////////////
// CChannelContext
void CChannelContext::OnRemNGS(const NSAP& nsap)
{
	AUTO_LOCK(&m_gcs);

	ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
	{
		NFRoomInfoInChannel& rinfo = *it;

		if(rinfo.m_nsapGLS == nsap)
		{
			//
			m_ChannelInfo.m_lRoomCount--;
			m_ChannelInfo.m_lUserCount -= rinfo.m_lstNFUserBaseInfo.size();
			//

//#ifdef _PMS_CODE_SEPERATION_
//			for(UserBaseInfoList::iterator itr = rinfo.m_lstUserBaseInfo.begin(); itr != rinfo.m_lstUserBaseInfo.end(); itr++)
//			{
//				UserBaseInfo& ubi = *itr;
//				UserStatInfo usi;
//				usi.Copy(ubi);
//				theStatTable.UpdateSessionStatInfoDelta(m_ChannelInfo.m_channelID, usi, -1, 0);
//			}
//#endif

			LOG(INF_UK, __FUNCTION__ , "::OnRemoveGLS RoomList Delete :", rinfo.m_dwGRIID);
			m_roomlist.erase(it);
		}
	}
}

void CChannelContext::OnRoomCreate(DWORD dwRSN, const NFRoomBaseInfo& info)
{
	NFRoomInfoInChannel rinfo;
	UserBaseInfoList lstUserInfo;
	rinfo.m_dwGRIID = info.m_dwGRIID;
	rinfo.m_lRoomState = info.m_lRoomState;
	rinfo.m_lRoomType = info.m_lRoomType;
	rinfo.m_nsapGLS = info.m_nsapGLS;
	rinfo.m_roomOption = info.m_roomOption;
	rinfo.m_sGameOption.assign(info.m_sGameOption.c_str(), info.m_sGameOption.length());
	rinfo.m_sReserve1.assign(info.m_sReserve1.c_str(), info.m_sReserve1.length());

	{
		AUTO_LOCK(&m_gcs);
		m_roomlist.push_back(rinfo);
	}

	m_ChannelInfo.m_lRoomCount++;

	theCHSInfoDir.UpdateCHSInfo(m_ChannelInfo.m_channelID, m_ChannelInfo.m_lUserCount, m_ChannelInfo.m_lWaitingUserCount, m_ChannelInfo.m_lRoomCount, m_ChannelInfo.m_lPCRoomUserCount);
}

void CChannelContext::OnRoomDelete(DWORD dwRSN)
{
	AUTO_LOCK(&m_gcs);

	LOG(ERR_UK, __FUNCTION__ , "::CAUTION_MSG::OnRoomDelete, ", dwRSN);

	ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
	{
		NFRoomInfoInChannel& info = *it;
		if(info.m_dwGRIID == dwRSN)
			break;
	}
	if(it == m_roomlist.end())
		return;

	NFRoomInfoInChannel& info = *it;

	m_ChannelInfo.m_lRoomCount--;
	m_ChannelInfo.m_lUserCount -= info.m_lstNFUserBaseInfo.size();
	theCHSInfoDir.UpdateCHSInfo(m_ChannelInfo.m_channelID, m_ChannelInfo.m_lUserCount, m_ChannelInfo.m_lWaitingUserCount, m_ChannelInfo.m_lRoomCount, m_ChannelInfo.m_lPCRoomUserCount);

#ifdef _PMS_CODE_SEPERATION_
	for(NFUserBaseInfoList::iterator itr = info.m_lstNFUserBaseInfo.begin(); itr != info.m_lstNFUserBaseInfo.end(); itr++)
	{
		NFUserBaseInfo& ubi = *itr;
		UserStatInfo usi;
		usi.Copy(ubi);
		theStatTable.UpdateSessionStatInfoDelta(m_ChannelInfo.m_channelID, usi, -1, 0);
		LOG(INF_UK, __FUNCTION__ , "::CAUTION_MSG::Receive Room Delete Msg Before All User Leave The Room");
	}
#endif
	//

	if (info.m_roomOption.m_lPlayType == 1)		// if roomType == FreeRoom then
		m_lstRoomIndexFree.push_front(info.m_dwGRIID);

	m_roomlist.erase(it);
}

void CChannelContext::OnRoomStatusChange(DWORD dwRSN, LONG lRoomState)
{
	{
		AUTO_LOCK(&m_gcs);
		ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
		{
			NFRoomInfoInChannel& rinfo = *it;
			if(rinfo.m_dwGRIID == dwRSN)
				break;
		}
		if(it == m_roomlist.end())
			return;

		NFRoomInfoInChannel& rinfo = *it;
		rinfo.m_roomOption.m_lRoomStatus = lRoomState;
	}
}

void CChannelContext::OnUserRoomJoin(DWORD dwRSN, const NFUserBaseInfo& uinfo)
{
	{
		AUTO_LOCK(&m_gcs);
		ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
		{
			NFRoomInfoInChannel& info = *it;
			if(info.m_dwGRIID == dwRSN)
				break;
		}
		if(it == m_roomlist.end())
		{
			return;
		}

		NFRoomInfoInChannel& info = *it;
		info.m_lstNFUserBaseInfo.push_back(uinfo);
	}
	m_ChannelInfo.m_lUserCount++;
	theCHSInfoDir.UpdateCHSInfo(m_ChannelInfo.m_channelID, m_ChannelInfo.m_lUserCount, m_ChannelInfo.m_lWaitingUserCount, m_ChannelInfo.m_lRoomCount, m_ChannelInfo.m_lPCRoomUserCount);

#ifdef _PMS_CODE_SEPERATION_
	UserStatInfo usi;
	usi.Copy(uinfo);
	theStatTable.UpdateSessionStatInfoDelta(m_ChannelInfo.m_channelID, usi, 1, 0); // PC방 사용자는 각 사 채널링 별로 분류 하지 않는다
#endif
}

void CChannelContext::OnUserRoomLeave(DWORD dwRSN, long lUSN)
{
	{
		AUTO_LOCK(&m_gcs);
		ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
		{
			NFRoomInfoInChannel& rinfo = *it;
			if(rinfo.m_dwGRIID == dwRSN)
				break;
		}
		if(it == m_roomlist.end())
			return;

		NFRoomInfoInChannel& rinfo = *it;

		ForEachElmt(NFUserBaseInfoList, rinfo.m_lstNFUserBaseInfo, it2, jt2)
		{
			NFUserBaseInfo& uinfo = *it2;
			if(uinfo.m_lLastPlayCSN == lUSN)
				break;
		}
		if(it2 == rinfo.m_lstNFUserBaseInfo.end())
			return;
	
		rinfo.m_lstNFUserBaseInfo.erase(it2);
									
#ifdef _PMS_CODE_SEPERATION_
		UserStatInfo usi;
		UserBaseInfo& ubi = *it2;
		usi.Copy(ubi);
		theStatTable.UpdateSessionStatInfoDelta(m_ChannelInfo.m_channelID, usi, -1, 0); // PC방 사용자는 각 사 채널링 별로 분류 하지 않는다
#endif
	}

	m_ChannelInfo.m_lUserCount--;
	theCHSInfoDir.UpdateCHSInfo(m_ChannelInfo.m_channelID, m_ChannelInfo.m_lUserCount, m_ChannelInfo.m_lWaitingUserCount, m_ChannelInfo.m_lRoomCount, m_ChannelInfo.m_lPCRoomUserCount);
}

void CChannelContext::OnUserRoomChange(DWORD dwRSN, ChangeUserGameDataList & lstGUD)
{
	{
		AUTO_LOCK(&m_gcs);
		ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
		{
			NFRoomInfoInChannel& rinfo = *it;
			if(rinfo.m_dwGRIID == dwRSN)
				break;
		}
		if(it == m_roomlist.end())
			return;

		// change usergamedata : 1user -> multi user change..
		ForEachElmt(ChangeUserGameDataList, lstGUD, it2, jt2)
		{
			ChangeUserInfo(dwRSN, *it, *it2);
		}
	}

	PayloadCHSCli pld(PayloadCHSCli::msgUserInfoInRoomChangeNtf_Tag,
		MsgCHSCli_UserInfoInRoomChangeNtf(dwRSN, lstGUD));
	SendToAll(pld);
}

void CChannelContext::ChangeUserInfo(DWORD dwRSN, NFRoomInfoInChannel& rinfo, ChangeUserGameData & ugd)
{
	ForEachElmt(NFUserBaseInfoList, rinfo.m_lstNFUserBaseInfo, it2, jt2)
	{
		NFUserBaseInfo& ruinfo = *it2;
		if(ruinfo.m_lUSN == ugd.m_lUSN)
			break;
	}
	if(it2 == rinfo.m_lstNFUserBaseInfo.end())
		return;

	NFUserBaseInfo& ruinfo = *it2;
	ruinfo.m_sUserGameData.assign(ugd.m_sUserGameData.c_str(), ugd.m_sUserGameData.length());
}

void CChannelContext::OnChangeRoomOption(DWORD dwRSN, const NFRoomOption& option)
{
	AUTO_LOCK(&m_gcs);
	ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
	{
		NFRoomInfoInChannel& rinfo = *it;
		if(rinfo.m_dwGRIID == dwRSN)
			break;
	}
	if(it == m_roomlist.end())
		return;
	NFRoomInfoInChannel& rinfo = *it;
	rinfo.m_roomOption.BCopy(option);

	PayloadCHSCli pld(PayloadCHSCli::msgChangeRoomOption_Tag,MsgCHSCli_ChangeRoomOptionNtf(rinfo.m_dwGRIID, option));
	SendToAll(pld);
}

void CChannelContext::OnChangeGameOption(DWORD dwRSN, const string& option)
{
	{
		AUTO_LOCK(&m_gcs);
		ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
		{
			NFRoomInfoInChannel& rinfo = *it;
			if(rinfo.m_dwGRIID == dwRSN)
				break;
		}
		if(it == m_roomlist.end())
			return;
		NFRoomInfoInChannel& rinfo = *it;
		rinfo.m_sGameOption.assign(option.c_str(), option.length());
	}

	PayloadCHSCli pld(PayloadCHSCli::msgChangeGameOption_Tag, MsgCHSCli_ChangeGameOptionNtf(dwRSN,option));
	SendToAll(pld);
}

void CChannelContext::OnChangeLoginState(DWORD dwRSN, LONG lUSN, LONG lLoginState)
{
	{
		AUTO_LOCK(&m_gcs);
		// find rinfo
		ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
		{
			NFRoomInfoInChannel& rinfo = *it;
			if(rinfo.m_dwGRIID == dwRSN)
				break;
		}
		if(it == m_roomlist.end())
			return;
		NFRoomInfoInChannel& rinfo = *it;

		// find ruinfo
		ForEachElmt(NFUserBaseInfoList, rinfo.m_lstNFUserBaseInfo, it2, jt2)
		{
			NFUserBaseInfo& ruinfo = *it2;
			if(ruinfo.m_lUSN == lUSN)
				break;
		}
		if(it2 == rinfo.m_lstNFUserBaseInfo.end())
			return;
		NFUserBaseInfo& ruinfo = *it2;

		ruinfo.m_lLoginState = lLoginState;
	}

	PayloadCHSCli pld(PayloadCHSCli::msgLoginStateChangeNtf_Tag);
	pld.un.m_msgLoginStateChangeNtf->m_lCSN = lUSN;
	pld.un.m_msgLoginStateChangeNtf->m_dwGRIID = dwRSN;
	pld.un.m_msgLoginStateChangeNtf->m_lLoginState = lLoginState;
	SendToAll(pld);
}

void CChannelContext::OnReqFreeRoomInfo(LONG lCSN, const RoomID& roomID, const LRBAddress& addrNGS)
{
	PayloadCHSNGS pld(PayloadCHSNGS::msgAnsFreeRoomList_Tag);
	pld.un.m_msgAnsFreeRoomList->m_lCSN = lCSN;
	pld.un.m_msgAnsFreeRoomList->m_roomID = roomID;

	{
		AUTO_LOCK(&m_gcs);
		
		ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
		{
			NFRoomInfoInChannel& rinfo = *it;

			if ((rinfo.m_dwGRIID != roomID.m_dwGRIID) && (rinfo.m_roomOption.m_lMaxUserCnt > (LONG)(rinfo.m_lstNFUserBaseInfo.size())))
				pld.un.m_msgAnsFreeRoomList->m_lstRoomInfo.push_back(rinfo);
		}
	}

	theLRBHandler.SendToNGS(pld, addrNGS);
}

void CChannelContext::GetUserList(NFUserInfoList& userBaseInfoList)
{
	ForEachElmt(CUserMap, m_UsersMap, itm, jtm)
	{
		CUser* p = *itm;
		userBaseInfoList.push_back(p->GetUserData());
	}
}

// lType = 1, 일반방
// lType = 2, 3 대전방
BOOL CChannelContext::GetWaitRoomList(ChannelID cid, NFRoomInfoInChannelList& lstRoom, UINT nGetCnt, LONG lType)
{
	AUTO_LOCK(&m_gcs);
	lstRoom.clear();

	ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
	{
		NFRoomInfoInChannel& rinfo = *it;
		if( rinfo.m_lRoomState == ROOMSTATE_RUN && rinfo.m_roomOption.m_lMaxUserCnt > (LONG)(rinfo.m_lstNFUserBaseInfo.size()) && lType == rinfo.m_roomOption.m_lPlayType)
		{
			if(GetTickCount() % 2 == 0)
				lstRoom.push_back(rinfo);
			else 
				lstRoom.push_front(rinfo);

			if(lstRoom.size() >= nGetCnt)
				break;
		}
	}
	return TRUE;
}

BOOL CChannelContext::GetRoomWithRoomID(DWORD dwGRIID, NFRoomInfoInChannel* rinfo)
{
	AUTO_LOCK(&m_gcs);
	ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
	{	
		NFRoomInfoInChannel& ri = *it;
		if(ri.m_dwGRIID == dwGRIID)
		{			
			rinfo->Copy(ri);
			return TRUE;
		}
	}
	return FALSE;

}

BOOL CChannelContext::FindRoom(DWORD dwGRIID)
{
	AUTO_LOCK(&m_gcs);
	ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it, jt)
	{	
		NFRoomInfoInChannel& ri = *it;
		if(ri.m_dwGRIID == dwGRIID)
			return TRUE;
	}
	return FALSE;			
}

void CChannelContext::OnAddRoomList(ChannelID cid, NFRoomInfoInChannelList& lstRoom)
{
	AUTO_LOCK(&m_gcs);

	ForEachElmt(NFRoomInfoInChannelList, lstRoom, it, ij)
	{
		NFRoomInfoInChannel& ri = (*it);

		int size = m_roomlist.size();
		if (size > 0)
		{
			bool bCheck = true;
			ForEachElmt(NFRoomInfoInChannelList, m_roomlist, it2, ij2)
			{
				NFRoomInfoInChannel& ri2 = (*it2);
				//// 기존방 있으면 업데이트
				if (ri2.m_dwGRIID == ri.m_dwGRIID)
				{
					ri2 = ri;
					bCheck = false;
					break;
				}
			}
			//// 새로운 방이면 추가.
			if (bCheck)
			{
				m_roomlist.push_back(ri);
				// 추가된 방정보 업데이트
				m_ChannelInfo.m_lRoomCount++;
				theCHSInfoDir.UpdateCHSInfo(m_ChannelInfo.m_channelID, m_ChannelInfo.m_lUserCount, m_ChannelInfo.m_lWaitingUserCount, m_ChannelInfo.m_lRoomCount, m_ChannelInfo.m_lPCRoomUserCount);
			}
		}
		else
		{
			m_roomlist.push_back(ri);

			// 추가된 방정보 업데이트
			m_ChannelInfo.m_lRoomCount++;
			theCHSInfoDir.UpdateCHSInfo(m_ChannelInfo.m_channelID, m_ChannelInfo.m_lUserCount, m_ChannelInfo.m_lWaitingUserCount, m_ChannelInfo.m_lRoomCount, m_ChannelInfo.m_lPCRoomUserCount);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////