
#include "stdafx.h"
#include "Control.h"
#include "ChatAgent.h"
#include "ChannelDir.h"

LONG CChannelContext::OnUserJoinReq(MsgCliCHS_JoinChannelReq * pMsg, CUser & user)
{
	// 채널 접속이 완료되면.. BLS와 통신.. 
	// UserBaseInfo 의 m_lChannelJoinState 인자를 IN/OUT으로 사용, BLS와 통신이 필요한지 아닌지를 구분.	
	if(theControl.IsRunIBB() && (user.GetUserData()).m_nfUserBaseInfo.m_lChannelJoinState == CJS_QUERYBLS)
	{
		if(user.GetBLSData() != string(""))
		{
		}
		else
		{ 
			if(user.GetUserID().find(_X("pmang_mon")) == xstring::npos && user.GetUSN()%7 == 0)
			{
				string logstr = format("Empty BLS Data : USN/UID = [%d/%s], SSN = %d", user.GetUSN(), \
					user.GetUserID().c_str(), m_ChannelInfo.m_channelID.m_lSSN);
				LOG(INF_UK, "CHS_CChannelContext_EmptyBLSData"_LK, logstr);
			}
			user.ChangeUserState(CJS_RUNBLS);
		}
#ifdef _USE_STATMSG
		ENTER_STAT_MSG(user.GetUSN(), 1);
#endif
	}
	else
	{
		user.ChangeUserState(CJS_RUNBLS);
	}


	LONG lErrCode = 0L;
	{
		PayloadCHSCli pld(PayloadCHSCli::msgChannelJoinAns_Tag);
		MsgCHSCli_ChannelJoinAns& msg = *pld.un.m_msgChannelJoinAns;
		msg.m_lErrorCode = lErrCode;

		// PC방 추가 떄문에 Code 수정
		// 기존 코드 msg.m_channelInfo = m_ChannelInfo;
		msg.m_channelInfo.Copy( m_ChannelInfo );
		msg.m_nfUser.BCopy(user.GetNFUser()); 

		{
			AUTO_LOCK(&m_gcs);
			msg.m_lstRoomInfo = m_roomlist;
		}
		msg.m_dwCHSLogicalAddr = theLRBHandler.GetMyAddress();

		SYSTEMTIME sysTime;
		::GetLocalTime(&sysTime);
		msg.m_lServerTime = (LONG)sysTime.wYear*10000 + (LONG)sysTime.wMonth*100 + (LONG)sysTime.wDay;
		SendToUser(&user, pld);
	}

	//다른 접속자에게 전송.
	{
		LONG lTotWaitUser = m_ChannelInfo.m_lWaitingUserCount;
		LONG lTotRoomUser = m_ChannelInfo.m_lUserCount - m_ChannelInfo.m_lWaitingUserCount;
		if(lTotRoomUser < 0) {
			LOG(INF_UK, "CHS_CChannelContext_OnUserJoinReq"_LK, "++++ UserCountingError : [---] +++++");
			lTotRoomUser = 0;
		}

		PayloadCHSCli pldAll(PayloadCHSCli::msgGCJoinNtf_Tag, MsgCHSCli_GCJoinNtf(lTotRoomUser+lTotWaitUser, lTotWaitUser, user.GetUserData().m_nfUserBaseInfo));
		SendToAll(pldAll, user.GetCSN());	// pUser를 제외한 모두에게..
	}

	// 현재 룸에서, 초대하기 창을 띠워놓고 있는 사용자들에게도 새로운 사용자가 들어왔음을 알린다. 
	{
		PayloadInvitation pldInvitation(PayloadInvitation::msgUserJoinNtf_Tag);
		MsgUserJoinNtf& msg = *pldInvitation.un.m_msgUserJoinNtf;
		msg.m_userInfo.BCopy(user.GetUserData().m_nfUserBaseInfo);
		SendToInviteAll(pldInvitation);
	}

	return TRUE;
}

void CChannelContext::OnDirectCreateReq(MsgCliCHS_GRDirectCreateReq * pMsg, LONG lCSN)
{
	MsgCliCHS_NGSInfoReq msg(pMsg->m_channelID);
	OnNGSInfoReq(&msg, lCSN, CONNECTTYPE_DIRECT);
}

void CChannelContext::OnNGSInfoReq(MsgCliCHS_NGSInfoReq * pMsg, LONG lCSN, LONG lMsgType) 
{
	PayloadCHSNCS pld(PayloadCHSNCS::msgNGSInfoReq_Tag, MsgCHSNCS_NGSInfoReq(m_ChannelInfo.m_channelID, lCSN, lMsgType));
	theLRBHandler.SendToNCS(pld);		
}

void CChannelContext::OnChannelListReq(LONG lCSN, LONG lCategory) 
{
#ifdef USE_CRCREPORTER
	// CRCReporter에 채널 리스트 요구를 대행 시킨다. 
	if(lCategory == m_ChannelInfo.m_channelID.m_dwCategory && m_RCReporter.OnCRCInfoReq(lCSN))
	{
		return;
	}
#endif
	ChannelID cID = m_ChannelInfo.m_channelID;
	PayloadCHSNCS pld(PayloadCHSNCS::msgRChannelListReq_Tag, MsgCHSNCS_RChannelListReq(cID, lCSN, lCategory));
	theLRBHandler.SendToNCS(pld);	
}

void CChannelContext::OnUserDisconnect(long lCSN)
{	
	CUser* pUser = FindUser(lCSN);
	if(!pUser)
		return;

	RoomID rid;
	pUser->NLSGetRoomID(rid);
	TKey key(pUser->GetUSN(), pUser->GetCSN());
	theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, rid, pUser->GetUserData().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel);

	RemoveUser(pUser);	
	delete pUser;
}

void CChannelContext::OnDirectInviteReq(MsgCliCHS_DirectInviteReq * pMsg, xstring sUserID)
{
	PayloadCHSCli msg(PayloadCHSCli::msgDirectInviteNtf_Tag);
	msg.un.m_msgDirectInviteNtf->m_lErrorCode = 0L;
	msg.un.m_msgDirectInviteNtf->m_dwGRIID = pMsg->m_dwGRIID;
	msg.un.m_msgDirectInviteNtf->m_sUserID = sUserID.c_str();
	msg.un.m_msgDirectInviteNtf->m_vecPosition = pMsg->m_vecPosition;

	GBuf buf;
	BOOL bRet = ::LStore(buf, msg);
	VALIDATE(bRet);

	ForEachElmt(lstCSN, /*CSN*/pMsg->m_lstCSN, i, j)
	{
		LONG lCSN = (*i);
		CUser * pUser = m_UsersMap.Find(lCSN);
		if(!pUser)
			continue;

		SendToUser(pUser, buf);
	}	
}

void CChannelContext::OnChatMsgTo(MsgCliCHS_ChatMsgTo * pMsg, LONG lCSN)
{
	CUser * pUser = m_UsersMap.Find(lCSN);
	if(!pUser)
		return;
	xstring sUID = pUser->GetUserID();
	xstring sNick = pUser->GetUserNick();
	CUser * pUser2 = m_UsersMap.Find(pMsg->m_lToCSN);
	if(!pUser2)	
		return;
	xstring sUID2 = pUser2->GetUserID();
	LONG lCSN2 = pUser2->GetCSN();
	xstring sNick2 = pUser2->GetUserNick();

	m_ChatHistory.AddNewMsg(lCSN, lCSN2, sUID, sNick, sUID2, pMsg->m_sText);


	PayloadCHSCli msg(PayloadCHSCli::msgChatMsgTo_Tag);
	msg.un.m_msgChatMsgTo->m_dwColor = pMsg->m_dwColor;
	msg.un.m_msgChatMsgTo->m_lFontSize = pMsg->m_lFontSize;
	msg.un.m_msgChatMsgTo->m_lFromCSN = lCSN;
	msg.un.m_msgChatMsgTo->m_lToCSN = pMsg->m_lToCSN;		// 이거 필요한가?

	msg.un.m_msgChatMsgTo->m_sText.assign(pMsg->m_sText.c_str(), pMsg->m_sText.length());	// 이거 assign 해야... 

	SendToUser(pMsg->m_lToCSN, msg); 

	// for Chat Agent
	ChannelID & cid = m_pChannel->GetChannelID();
	if(theChatAgent.IsCVConnected() && theChatAgent.IsCategory(cid.m_dwCategory) && theChatAgent.IsSecret() )
	{
		theChatAgent.PushChatMsg(cid, lCSN, sUID, sNick, pMsg->m_sText, lCSN2, sUID2, sNick2);		
	}

	// 전투바둑이만 로그를 남긴다.
	if(m_pChannel->GetChannelID().m_lSSN ==  17)
	{

		// 바둑이(전체), 채팅 기록 남기기
		// 일시 | 귓속말 여부 | 채널명(서버명) | 
		// 송신자 USN | ID | 별명 | IP |
		// 수신자 USN | ID | 별명 | IP |
		// 대화내용 
		in_addr addr1;
		addr1.S_un.S_addr = pUser->GetLink()->GetIP();
		in_addr addr2;
		addr2.S_un.S_addr = pUser2->GetLink()->GetIP();

		LOG(INF_UK, "CHS_CHAT_INFO, ", "1", " | ", m_ChannelInfo.m_channelID.m_dwGCIID, " | ",
			lCSN, " | ", sUID.c_str(), " | ", sNick.c_str(), " | ", inet_ntoa(addr1), " | ", 
			pMsg->m_lToCSN, " | ", pUser2->GetUserID().c_str(), " | ", pUser2->GetUserNick().c_str(), " | ", inet_ntoa(addr2), " | ", 
			pMsg->m_sText.c_str());
	}
}

void CChannelContext::OnChatMsgAll(MsgCliCHS_ChatMsgAll * pMsg, LONG lCSN)
{
	CUser * pUser = m_UsersMap.Find(lCSN);
	if(!pUser)
		return;
	xstring sUID = pUser->GetUserID();
	xstring sNick = pUser->GetUserNick();
	xstring temp =  _X(" ");
	m_ChatHistory.AddNewMsg(lCSN, -1L, sUID, sNick, temp, pMsg->m_sText);


	PayloadCHSCli msg(PayloadCHSCli::msgChatMsgAll_Tag);
	msg.un.m_msgChatMsgAll->m_sNickName = sNick;
	msg.un.m_msgChatMsgAll->m_dwColor = pMsg->m_dwColor;
	msg.un.m_msgChatMsgAll->m_lFontSize = pMsg->m_lFontSize;
	msg.un.m_msgChatMsgAll->m_lFromCSN = lCSN;
	msg.un.m_msgChatMsgAll->m_sText.assign(pMsg->m_sText.c_str(), pMsg->m_sText.length());

	SendToAll(msg);

	// for Chat Agent
	ChannelID & cid = m_pChannel->GetChannelID();
	if(theChatAgent.IsCVConnected() && theChatAgent.IsCategory(cid.m_dwCategory) )
	{
		theChatAgent.PushChatMsg(cid, lCSN, sUID, sNick, pMsg->m_sText);
	}

	// 전투바둑이만 로그를 남긴다.
	if(m_pChannel->GetChannelID().m_lSSN ==  17)
	{
		in_addr addr1;
		addr1.S_un.S_addr = pUser->GetLink()->GetIP();

		// 바둑이(전체), 채팅 기록 남기기
		// 일시 | 귓속말 여부 | 채널명(서버명) | 
		// 송신자 USN | ID | 별명 | IP |
		// 수신자 USN | ID | 별명 | IP |
		// 대화내용 
		LOG(INF_UK, "CHS_CHAT_INFO, ", "0", " | ", m_ChannelInfo.m_channelID.m_dwGCIID, " | ",
			lCSN, " | ", sUID.c_str(), " | ", sNick.c_str(), " | ", inet_ntoa(addr1), " | ", 
			" |  |  |  | ", 
			pMsg->m_sText.c_str());
	}
}

void CChannelContext::OnLoginStateChangeNtf(MsgCliCHS_LoginStateChangeNtf * pMsg, LONG lCSN)
{
	CUser * pUser = m_UsersMap.Find(lCSN);
	if(!pUser)
		return;
	pUser->UpdateLoginState(pMsg->m_lLoginState);

	//RoomID _roomid(m_ChannelInfo.m_channelID, 0L);
	PayloadCHSCli pld( PayloadCHSCli::msgLoginStateChangeNtf_Tag, 
		MsgCHSCli_LoginStateChangeNtf( 
		0L,
		lCSN,
		pMsg->m_lLoginState
		)
		);
	SendToAll(pld);
}

int gnIBBSendMsgCnt1 = 0;
int gnIBBSendMsgCnt2 = 0;
void CChannelContext::OnAccuseReq(MsgCliCHS_AccuseReq * pMsg, CUser & user)
{
	LOG(DEV_UK, "Accuse",
		", AccusedUSN: ", pMsg->m_lAccusedUSN,
		", AccusedID: ", pMsg->m_sAccusedID,
		", AccuserUSN: ", user.GetUSN(),
		", Type: ", pMsg->m_sType,
		", Reason: ", pMsg->m_sReason,
		", Content: ", pMsg->m_sContent,
		", RoomTitle: ", pMsg->m_sRoomTitle
		);

	if(pMsg->m_sType == "g1" || pMsg->m_sType == "k1")
	{
		UserAccuseReq(pMsg, user);
	}
	else if(pMsg->m_sType == "gt" || pMsg->m_sType == "kt")
	{
		RoomTitleAccuseReq(pMsg, user);
	}
	else if(pMsg->m_sType == "a2")
	{
		PrearrangeGameAccuseReq(pMsg, user);
	}
	else 
		LOG(INF_UK, "CHS_CChannelContext"_LK, "Unknown Accuse Type.. ", pMsg->m_sType);
}

// 사용자의 게임정보를 보낸다.
void CChannelContext::OnGetUserGameInfoReq(MsgCliCHS_GetUserGameInfoReq* pMsg, CUser& user) 
{

}

void CChannelContext::OnGetRankInfoReq(MsgCliCHS_GetUserRankInfoReq * pMsg, CUser & pUser)
{

}

//void CChannelContext::AchvReportCallback(LONG GSN, LONG CSN, int achv_ID, const achv::EventItem_T *pEvtItem)
//{
//	theLog.Put(ERR_UK, "NGS_Logic, AchvReportCallback(), CSN :", CSN, ", RoomID :", pEvtItem->roomID.m_dwGRIID, ", Event :", pEvtItem->evt, ", Achv_ID :", achv_ID);
//
	// 업적 성공 클라이언트로 알림
// 	MsgNGSCli_NotifyCompleteAchieveInfo		ntf;
// 	ntf.m_lCSN = CSN;
// 
// 	CRoomPtr spRoom;
// 	BOOL bRet = theRoomTable.FindRoom(pEvtItem->roomID, &spRoom);
// 	if (!bRet)
// 	{
// 		theLog.Put(ERR_UK, "NGS_Logic, AchvReportCallback(), Not Found RoomID : ", pEvtItem->roomID.m_dwGRIID, ", CSN :", CSN);
// 		return;
// 	}
// 
// 	LONG lGSN = spRoom->GetAchvUserInfo(CSN, ntf.m_strNickName);
// 	if (lGSN <= 0)
// 	{
// 		theLog.Put(ERR_UK, "NGS_Logic, AchvReportCallback(), Not Found User CSN :", CSN);
// 		return;
// 	}
// 
// 	int nErrorCode = 0;
// 
// 	Achievement	completeACHV;
// 	completeACHV.Clear();
// 	completeACHV.m_lAchieveSRL = achv_ID;
// 	completeACHV.m_dGauge = pEvtItem->val;
// 
// 	SYSTEMTIME systime;
// 	::GetLocalTime(&systime);
// 	completeACHV.m_strEndDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
// 
// 	ntf.m_listGetLately.push_back(completeACHV);
// 	ntf.m_lErrorCode = nErrorCode;
// 
// 
// 	// 보상은 따로 지급.. 여기서는 알리기만 하면 됨	(알리는것도 레벨에 따라서 알린다...- 나중에 추가)	
// 	PayloadNGSCli pld(PayloadNGSCli::msgNotifyCompleteAchieveInfo_Tag, ntf);
// 	spRoom->SendToUser(CSN, pld);
//}
