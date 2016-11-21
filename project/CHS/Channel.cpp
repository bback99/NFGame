//
// Channel.cpp
//

#include "stdafx.h"
#include "Agent.h"
#include "CHSInfoDir.h"
#include "Channel.h"
#include "ChannelDir.h"
#include "ChatAgent.h"
#include "Control.h"
#include "ErrorDefine.h"
#include "Listener.h"
#include "LRBHandler.h"
#include "RankUtility.h"
#include "StatisticsTable.h"
#include <NFVariant/NFDBManager.h>
#include <NFVariant/NFMenu.h>

#include <atlbase.h>

#ifdef _CHSNLS
#include <NLSManager.h>
#endif

#ifdef _USE_STATMSG
#include "StatMsgMgr.h"
#endif

extern void md5hash_say(char* tgt,const char* src);
//////////////////////////////////////////////////////////////////////////////////
// CChannel
string GetParseData(string& sTarget, string sToken)
{
	string sRet;
	int nIndex = sTarget.find_first_of(sToken.c_str());
	if ( nIndex != (int)NPOS )
	{
		sRet = sTarget.substr(0, nIndex);
		sTarget.erase( 0, nIndex + 1 ); // sToken도 지운다
	}
	else
	{
		sRet = sTarget;
		sTarget.erase();
	}
	return sRet;
}

string GetParseDatawithTokenRvs(string& sTarget, string sToken)
{
	string sRet;
	int nIndex = sTarget.rfind(sToken.c_str());
	if ( nIndex != (int)NPOS )
	{
		sRet = sTarget.substr(nIndex); // nIndex 이후 모든 스트링을 substr. sToken 안지운다
		sTarget.erase( nIndex ); // nIndex 이후 모든 스트링을 erase
	}
	// 없으면 NULL
	return sRet;
}

string GetNextTokenData(string sTarget, string sToken, string sFind)
{
	string sRet;

	if(sTarget == "")
		return string("");
	while(sTarget.size() > 0)
	{
		int nIndex = sTarget.find_first_of(sToken);
		if ( nIndex != (int)NPOS )
		{
			sRet = sTarget.substr(0, nIndex);
			sTarget.erase( 0, nIndex + 1 );
		}
		else
		{
			sRet = sTarget;
			sTarget.erase();
		}
		if(sRet.find(sFind) == 0)
		{
			int nf = sRet.find("=");			
			return sRet.substr(nf+1, sRet.size() - nf - 1);
		}
	}
	return string("");
}

LONG GetNFCharInfo(CUser* pUser, LONG lReqSSN = 0, string* pOutString = NULL)
{
	//////////////////////////////////////////////////////////////////////////
	// ###################### NFUser 정보 읽어오는 부분 ######################
	NFUserBaseInfo NFUBI = pUser->GetUserData().m_nfUserBaseInfo;
	NFUser& NFUI = pUser->GetNFUser();

	//////////////////////////////////////////////////////////////////////////
	// 1. CSN에 해당하는 NFChar정보 읽어오기...
	LONG lErrorCode = theNFDBMgr.SelectNFCharBaseNExteriorInfo(NFUI.m_nfCharInfoExt, NFUBI.m_lGSN, NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
	if (lErrorCode != 1)
	{
		pUser->SetErrorCode(ERR_CRF_NOT_EXIST_CHAR);
		theLog.Put(ERR_UK, "GetNFCharInfo"_LK, "SelectNFCharByCSN is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
		return FALSE;
	}

	LONG lMaxExp = 0;
	theNFDataItemMgr.GetNFExp(NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel, lMaxExp);
	NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lExpMax = lMaxExp;

	//////////////////////////////////////////////////////////////////////////
	// 2. CSN에 해당하는 NFInvenSlot정보 읽어오기
	TlstInvenSlot lst;
	if (!theNFDBMgr.SelectNFCharInven(lst, NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_strLastestLogOutDate, NFUI.m_nfCharInfoExt.m_nfCharInven, pUser->GetGSN(), NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN, lErrorCode))
	{
		pUser->SetErrorCode(ERR_CRF_NOT_EXIST_INVEN);
		theLog.Put(ERR_UK, "GetNFCharInfo"_LK, "SelectNFCharInvenByCSN is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
		return FALSE;
	}
	else
	{
		if (NF::G_NF_ERR_SUCCESS != lErrorCode)
			theLog.Put(ERR_UK, "GetNFCharInfo"_LK, "SelectNFCharInvenByCSN is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN, ", Err : ", lErrorCode);
	}

	// 수족관 물고기 정보
	if (!theNFMenu.GetAquaFish(pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfAquaFish))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(ERR, "theNFMenu.GetAquaFish is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	// WORKING(acepm83@neowiz.com) 능력치를 셋팅하기전에 수족관 정보를 읽어야 한다.(Beacause 수족관 버프)

	LONG lElapsedClearHour = 0; // 청소 경과시간
	LONG lElapsedFeedHour = 0;	// 밥준 경과시간
	
	if (!theNFDBMgr.SelectNFCharAqua(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfAqua, lElapsedClearHour, lElapsedFeedHour))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(ERR, "SelectNFCharAqua is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}
	else
	{
		theNFMenu.CalcNFAquaGauge(pUser->GetNFCharInfoExt(), lElapsedClearHour, lElapsedFeedHour);
	}

	//////////////////////////////////////////////////////////////////////////
	// 3. CSN에 해당하는 Inven_use + 기본 캐릭터 Ability 읽어오기...
	lErrorCode = theNFDBMgr.SelectNFAbilityByCSN(pUser->GetNFCharInfoExt(), NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
	if (lErrorCode != NF::G_NF_ERR_SUCCESS)
	{
		pUser->SetErrorCode(ERR_CRF_NOT_EXIST_ABILITY);
		theLog.Put(ERR_UK, "GetNFCharInfo"_LK, "SelectNFAbilityByCSN is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
		return FALSE;
	}

	NFAbility	nfBasicAbility;
	if (!theNFDataItemMgr.GetNFAbility(NFUI.m_nfCharInfoExt.m_nfCharExteriorInfo.m_lBasicCharSRL, nfBasicAbility))
	{
		pUser->SetErrorCode(ERR_CRF_NOT_EXIST_ABILITY);
		theLog.Put(ERR_UK, "GetNFCharInfo"_LK, "GetNFAbility is Fail!!!, Char DefaultCSN: ", NFUI.m_nfCharInfoExt.m_nfCharExteriorInfo.m_lBasicCharSRL, ", CSN : ", NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
		return FALSE;
	}

	NFUI.m_nfCharInfoExt.m_nfAbility += nfBasicAbility;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
ULONG __stdcall ChannelLinkGroup::AddRef()
{
	return m_pChannel->AddRef();
}
ULONG __stdcall ChannelLinkGroup::Release()
{
	return m_pChannel->Release();
}
void ChannelLinkGroup::Lock()
{
	m_pChannel->Lock();
}
void ChannelLinkGroup::Unlock()
{
	m_pChannel->Unlock();
}
void __stdcall ChannelLinkGroup::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	Lock();
	if(hSignal == 0)
	{
		LOG(INF_UK, "CHS_ChannelLinkGroup"_LK, "Unknown signal handle");
	}
	else 
	{
		TBase::OnSignal(hSignal, wParam, lParam);
	}
	Unlock();
}

BOOL ChannelLinkGroup::Deactivate()
{
	ForEachElmt(TLinkMap, mLinkMap, i, j)
	{
		TLink* pLink = i->second;
		DestroyLink(pLink);
	}

	mLinkMap.clear();
	return TRUE;
}

void ChannelLinkGroup::DestroyLink(CLink* pLink)
{
	CUser* pUser = pLink->GetUser();
	if(pUser)
	{
		//m_pChannelContext->RemoveUser(pUser);
		pUser->SetLink(NULL);
		pLink->SetUser(NULL);
		delete(pUser);
	}
	RemoveLink(pLink);
	pLink->Unregister();
	delete(pLink);
}

BOOL ChannelLinkGroup::OnRcvMsg(CLink* pSocket, PayloadCliCHS& rPld)
{
	CUser* pUser = pSocket->GetUser();
	if(!pUser)
	{
		LOG(INF_UK, "CHS_ChannelLinkGroup"_LK, "++++ OnRcvMsg : Not found User ++++");
		return OnError(pSocket, FD_READ, -100);
	}
	m_pChannel->m_pChannelContext->OnUserMsg(pUser, rPld);
	
	return TRUE;
}

BOOL ChannelLinkGroup::OnError(TLink* pSocket, long lEvent, int nErrorCode)
{
	CUser* pUser = pSocket->GetUser();
	
	if (ERROR_BUFFER_OVERFLOW == nErrorCode)
	{
		LOG(INF_UK, "CHS_ChannelLinkGroup_BufOF"_LK, "******* Buffer Overflow : Event=", lEvent, ", ErrorCode=", nErrorCode);
		if (pUser)
			LOG(INF_UK, "CHS_ChannelLinkGroup_BufOF"_LK, "\tClosed Link: USN = ",pUser->GetUSN(), ", ID = ", pUser->GetUserID().c_str() );
	}
	if(pUser)
	{
		RoomID rid;
		pUser->NLSGetRoomID(rid);
//		if(IS_SINGLE_JOIN_SSN(rid.m_lSSN)) 
		{
			TKey key(pUser->GetUSN(), pUser->GetCSN());
			theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, rid, pUser->GetUserData().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel);
		}
		m_pChannel->m_pChannelContext->RemoveUser(pUser);

#ifdef USE_CRCREPORTER
		m_pChannel->m_pChannelContext->m_RCReporter.RemCRCUSN(pUser->GetUSN());
#endif

		delete(pUser);
	}
	else 
	{
		// Invite에 의해 처리된 socket은 CUser Objedt를 만들지 않았음.
		RemoveLink(pSocket);
		pSocket->Unregister();
		delete pSocket;
	}
	return FALSE;
}

void ChannelLinkGroup::UserLinkCut(DWORD dwIndex)
{
	ForEachElmt(TLinkMap, mLinkMap, it, jt)
	{
		TLink* pLink = it->second;
		if(pLink->GetIndex() == dwIndex)
			break;
	}
	if(it != mLinkMap.end())
	{
		TLink* pLink = it->second;
		LOG(INF_UK, "CHS_ChannelLinkGroup"_LK, "++++++ UserLinkCut :SendBuf overflow : User Disconnect +++++++ [", pLink->GetIndex(), "]");
		OnError(pLink, FD_WRITE, 588);
	}
}

ULONG __stdcall InviteLinkGroup::AddRef()
{
	return m_pChannel->AddRef();
}
ULONG __stdcall InviteLinkGroup::Release()
{
	return m_pChannel->Release();
}
void InviteLinkGroup::Lock()
{
	m_pChannel->Lock();
}
void InviteLinkGroup::Unlock()
{
	m_pChannel->Unlock();
}

void __stdcall InviteLinkGroup::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	Lock();
	if(hSignal == 0)
	{
		LOG(INF_UK, "CHS_InviteLinkGroup"_LK, "Unknown signal handle");
	}
	else 
		TBase::OnSignal(hSignal, wParam, lParam);
	Unlock();
}

BOOL InviteLinkGroup::Deactivate()
{
	ForEachElmt(TLinkMap, mLinkMap, i, j)
	{
		TLink* pLink = i->second;
		DestroyLink(pLink);
	}

	mLinkMap.clear();
	return TRUE;
}

void InviteLinkGroup::DestroyLink(CInviteLink* pLink)
{
	RemoveLink(pLink);
	pLink->Unregister();
	delete (pLink);
}

BOOL InviteLinkGroup::OnRcvMsg(CInviteLink* pSocket, PayloadInvitation& rPld)
{
	ASSERT(pSocket != NULL);

	switch(rPld.mTagID)
	{
	case PayloadInvitation::msgInvitationReq_Tag :
		{
			BOOL bRet = OnRcvInvitationReq(pSocket, rPld.un.m_msgInvitationReq);
			return bRet;
		}
	default :
		break;
	}
	return TRUE;
}

BOOL InviteLinkGroup::OnError(TLink* pSocket, long lEvent, int nErrorCode)
{
	RemoveLink(pSocket);
	pSocket->Unregister();
	delete (pSocket);
	return FALSE;
}

void InviteLinkGroup::OnAddLink(CInviteLink* pLink)
{
	ASSERT(pLink != NULL);
	AddLink(pLink);

	PayloadInvitation pld(PayloadInvitation::msgInvitationInfoNtf_Tag);
	MsgInvitationInfoNtf& msg = *pld.un.m_msgInvitationInfoNtf;

	msg.m_lErrorCode = INVITE_ERR_NOERROR;

	// #### 수정해야 함 2010.6.22
	//m_pChannel->m_pChannelContext->GetUserList(msg.m_lstUserInfo);

	SendMsg(pLink, pld);
}

void InviteLinkGroup::SendMsg(CInviteLink* pLink, PayloadInvitation& pld)
{
	GBuf buf;
	::LStore(buf, pld);

	pLink->DoSend(buf);

}

void InviteLinkGroup::SendToAll(const GBuf& buf)
{
	ForEachElmt(TLinkMap, mLinkMap, it, jt)
	{
		CInviteLink* pLink = it->second;
		if (pLink != NULL)
			pLink->DoSend(buf);
	}
}

BOOL InviteLinkGroup::OnRcvInvitationReq(CInviteLink* pLink, MsgInvitationReq* pMsg)
{
	ASSERT(pLink != NULL);

	PayloadInvitation pld(PayloadInvitation::msgInvitationAns_Tag);
	MsgInvitationAns& rMsg = *(pld.un.m_msgInvitationAns);
	rMsg.m_usnList.BCopy(pMsg->m_usnList);

	m_pChannel->OnRcvInvitationReq(pLink->GetNSAP(), pLink->GetRoomID(), pLink->GetUserInfo(), pMsg->m_sPasswd, pMsg->m_sMessage, pMsg->m_usnList, rMsg.m_lstErrorCode);

	SendMsg(pLink, pld);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

CChannel::CChannel(const ChannelID& channelID, const ChannelBaseInfo& binfo, long lChannelType)
{
	m_lRoomID = 0;

	m_ChannelID = channelID;
	m_lChannelType = lChannelType;
	m_pChannelContext = new CChannelContext(this, binfo);
	VALIDATE(m_pChannelContext);
	m_apChannelLinkGroup = new ChannelLinkGroup(this);
	m_apInviteLinkGroup = new InviteLinkGroup(this);

	// BanishUser들을 KickOut 하기 위한 Timer
	m_timerBanishUser.Activate(GetThreadPool(), this, 2000, 2000);
}
CChannel::~CChannel() 
{

}

BOOL CChannel::Run()
{
	TLock lo(this);

	ChannelInfoPCRoom& info = m_pChannelContext->m_ChannelInfo;
#ifdef _PMS_CODE_SEPERATION_
	theCHSInfoDir.InsertCHSInfo(info.m_channelID, info.m_lUserCount, info.m_lWaitingUserCount, info.m_lRoomCount, info.m_lPCRoomUserCount);
#else
	theCHSInfoDir.AddCHSInfo(info.m_channelID, info.m_lUserCount, info.m_lWaitingUserCount, info.m_lRoomCount, info.m_lPCRoomUserCount);
#endif
	BOOL bRet = TRUE;

#ifdef USE_CRCREPORTER
	m_pChannelContext->m_RCReporter.RunCRC();
#endif

	return bRet;
}

BOOL CChannel::Stop()
{
	TLock lo(this);
#ifdef _PMS_CODE_SEPERATION_
	theCHSInfoDir.DeleteCHSInfo(m_pChannelContext->m_ChannelInfo.m_channelID);
#else
	theCHSInfoDir.RemoveCHSInfo(m_pChannelContext->m_ChannelInfo.m_channelID);
#endif

#ifdef USE_CRCREPORTER
	m_pChannelContext->m_RCReporter.StopCRC();
#endif

	if (m_apChannelLinkGroup->IsActive())
		m_apChannelLinkGroup->Deactivate();
	if (m_apInviteLinkGroup->IsActive())
		m_apInviteLinkGroup->Deactivate();

	return TRUE;
}

void CChannel::SendToLink(CLink* pLink, const GBuf& buf)
{
	pLink->DoSend(buf);

}

void CChannel::SendToInviteAll(const GBuf& buf)
{
	m_apInviteLinkGroup->SendToAll(buf);
}

BOOL CChannel::AddSocket(CLink* pLink)
{
	return m_apChannelLinkGroup->AddLink(pLink);
}

void CChannel::RemoveSocket(CLink* pLink)
{
	m_apChannelLinkGroup->RemoveLink(pLink);
}

void CChannel::OnKickOutBanishUser()
{
	ForEachElmt(TMapBanishUser, m_mapBanishUser, it, ij)
	{
		DWORD dwCurTime = GetTickCount();
		if (dwCurTime - (DWORD)((*it).second >= 2000))
		{
			LOG(INF_UK, "CHS_CChannel_Kickout, CChannel::OnSignal() Delete USN :", (*it).first, ", TimeGap :", dwCurTime - (DWORD)((*it).second));
			m_pChannelContext->OnUserDisconnectFromNLS((*it).first, NULL);
			m_mapBanishUser.erase((*it).first);
		}
	}
}

STDMETHODIMP_(void) CChannel::OnSignal(HSIGNAL hObj, WPARAM wP, LPARAM lP)
{
	if (m_timerBanishUser.IsHandle(hObj))
	{
		TLock lo(this);
		OnKickOutBanishUser();
	}
	else if (hObj == 0) 
	{
		if((LONG)wP == CHANNEL_SMGWMSG)
		{
			xstring* pUID = (xstring*)lP;
			{
				TLock lo(this);
				m_pChannelContext->OnUserChatPart(*pUID);
			} 
			delete pUID;		
		}
		else if((LONG)wP == CHANNEL_ANNOUNCEMSG)
		{
			LPXBUF pXBuf = (LPXBUF)lP;
			TLock lo(this);
			{
				GBuf buf(pXBuf, FALSE);
				OnAnnounceMsgFromPMS(buf);
			}
		}
		else if((LONG)wP == CHANNEL_ALIVECHK)
		{
			LOG(INF_UK, "CHS_CChannel_OnSignal"_LK, "************CChannel OnSignal : [", hObj, "/", wP, "/", lP, "] *************\n");
		}
		else if ((LONG)wP == CHANNEL_INVITEUSERJOIN)
		{
			CInviteLink* pLink = (CInviteLink *)lP;
			ASSERT(pLink != NULL);
			TLock lo(this);
			m_apInviteLinkGroup->OnAddLink(pLink);
		}
	}
	else if(hObj == CHANNEL_USERLINKCUT_SIG)
	{
		TLock lo(this);
		DWORD dwIndex = lP;
		if(dwIndex > 0)
			m_apChannelLinkGroup->UserLinkCut(dwIndex);
		else if(wP > 0)
		{
			m_pChannelContext->OnUserDisconnect(wP);
		}
	}
	else if(hObj == CHANNEL_USERLINKCUT_SIG_FROM_NLS)
	{
		TLock lo(this);
		if(wP > 0 && lP > 0)
		{
			m_pChannelContext->OnUserDisconnectFromNLS(wP, (CUser*)lP);
		}
		else
		{
			CUser* pOldUser = FindUser((LONG)wP);
			if (!pOldUser)
				return;

			// 여기서 메세지를 보내야 끊기기 전에 메세지를 제대로 보낼 수 있다.... 2010/9/14 -bback99 
			MsgCHSCli_ReqBanishUser		req;
			PayloadCHSCli	pld(PayloadCHSCli::msgReqBanishUser_Tag, req);
			m_pChannelContext->SendToUser(pOldUser, pld);

			// 2초후에 끊기 위해서 Timer에 등록한다.
			m_mapBanishUser[(LONG)wP] = GetTickCount();		
			LOG(INF_UK, "CHS_CChannel_OnSignal, CChannel::OnSignal() USN :", (LONG)wP, ", RegTime :", m_mapBanishUser[(LONG)wP]);
		}
	}
	else if(hObj == CHANNEL_PROCESS_NF_FRIEND_ACCEPT) // 친구요청을 받은 캐릭터가 수락했을 때 처리
	{
		TLock lo(this);
		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wP))
			return;

		// 수락자
		CUser* pAcceptor = FindUser( msg.m_lCSN );
		if(!pAcceptor)
			return;

		// 수락자 정보
		RoomID roomID;
		pAcceptor->NLSGetRoomID(roomID);
		CNFFriend nfFriendAcceptor;
		nfFriendAcceptor.m_bIsOnline = TRUE;
		nfFriendAcceptor.m_lLevel = pAcceptor->GetUserData().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel;
		nfFriendAcceptor.m_strCharName = pAcceptor->GetUserNick();
		nfFriendAcceptor.m_roomID = roomID;
		nfFriendAcceptor.m_lStatus = NLSCLISTATUS_NFCHANNELSERVER;

		// 수락자의 친구 리스트
		CONT_NF_FRIEND kContAcceptorFriend;
		pAcceptor->FindNFFriend(kContAcceptorFriend);

		// 신청자 정보
		CNFFriend nfFriendApplicant;
		nfFriendApplicant.m_strCharName = msg.m_strCharName;

		ArcVectorT< NLSBaseInfo >::iterator applicant_iter = msg.m_kContNLSBaseInfo.begin();
		if( msg.m_kContNLSBaseInfo.end() != applicant_iter )
		{
			// 수락자의 친구정보 중에서 신청자의 레벨, 위치를 업데이트
			CONT_NF_FRIEND::iterator acceptor_friend_iter = kContAcceptorFriend.find( applicant_iter->m_Key );
			if( acceptor_friend_iter != kContAcceptorFriend.end() )
			{
				CONT_NF_FRIEND::mapped_type& element = acceptor_friend_iter->second;
				element.m_lLevel = applicant_iter->m_lLevel;
				element.m_roomID = applicant_iter->m_roomID;
				element.m_bIsOnline = TRUE;
				element.m_lStatus = applicant_iter->m_lStatus;
				nfFriendApplicant = element;
			}

			// 신청자의 위치
			switch( applicant_iter->m_lStatus )
			{
				case NLSCLISTATUS_NFCHARLOBBY: // NCS
					{
						// NCS로 전달
						MsgCHSNCS_NtfNFFriendAccept ntf;
						ntf.m_lReceiverCSN = applicant_iter->m_Key.m_lSubKey;
						ntf.m_nfFriend = nfFriendAcceptor;
						PayloadCHSNCS pld(PayloadCHSNCS::msgNtfNFFriendAccept_Tag, ntf);
						theLRBHandler.SendToNCS(pld, applicant_iter->m_serverLRBAddr);
					}break;
				case NLSCLISTATUS_NFCHANNELSERVER: // CHS
					{
						ChannelID channelID;
						channelID = applicant_iter->m_roomID;
						CChannelPtr spChannel;
						if(!theChannelDir.GetChannel(channelID, &spChannel))
							return;

						// 신청자에게 상대방이 수락했음을 알린다.
						CUser* pApplicant = spChannel->FindUser(applicant_iter->m_Key.m_lSubKey);
						if( pApplicant )
						{
							pApplicant->AddNFFriend( TKey( msg.m_lUSN, msg.m_lCSN ), nfFriendAcceptor );

							MsgCHSCli_NtfNFFriendAccept ntf;
							ntf.m_nfFriend = nfFriendAcceptor;
							PayloadCHSCli pld(PayloadCHSCli::msgNtfNFFriendAccept_Tag, ntf);
							spChannel->m_pChannelContext->SendToUser(pApplicant, pld);
						}
					}break;
				case NLSCLISTATUS_NFGAMESERVER: // NGS
					{
						// NGS로 전달
						MsgCHSNGS_NtfNFFriendAccept ntf;
						ntf.m_lReceiverCSN = applicant_iter->m_Key.m_lSubKey;
						ntf.m_roomID = applicant_iter->m_roomID;
						ntf.m_nfFriend = nfFriendAcceptor;
						PayloadCHSNGS pld(PayloadCHSNGS::msgNtfNFFriendAccept_Tag, ntf);
						theLRBHandler.SendToNGS(pld, applicant_iter->m_serverLRBAddr);
					}break;					
				default:
					{
					}break;
			}
		}

		// 수락자에게 신청자의 정보를 보내준다.
		MsgCHSCli_AnsNFFriendAccept ans;
		ans.m_lErrorCode = EC_FE_SUCCESS;
		ans.m_nfFriend = nfFriendApplicant;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAccept_Tag, ans);
		m_pChannelContext->SendToUser(pAcceptor, pld);
	}
	else if( hObj == CHANNEL_PROCESS_NF_FRIEND_ADD )
	{
		TLock lo(this);

		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wP))
			return;

		// 신청자
		CUser* pApplicant = FindUser( msg.m_lCSN );
		if( !pApplicant )
		{
			return;
		}

		std::string strApplicantName = pApplicant->GetUserNick();

		ArcVectorT< NLSBaseInfo >::const_iterator acceptor_iter = msg.m_kContNLSBaseInfo.begin();
		if( msg.m_kContNLSBaseInfo.end() != acceptor_iter )
		{
			// 수락자의 위치
			switch( acceptor_iter->m_lStatus )
			{
			case NLSCLISTATUS_NFCHARLOBBY://NCS
				{
					MsgCHSNCS_NtfNFFriendAdd ntf;
					ntf.m_lReceiverCSN = acceptor_iter->m_Key.m_lSubKey;
					ntf.m_strSender = strApplicantName;
					PayloadCHSNCS pld(PayloadCHSNCS::msgNtfNFFriendAdd_Tag, ntf);
					theLRBHandler.SendToNCS(pld, acceptor_iter->m_serverLRBAddr);
				}break;
			case NLSCLISTATUS_NFGAMESERVER://NGS
				{
					MsgCHSNGS_NtfNFFriendAdd ntf;
					ntf.m_lReceiverUSN = acceptor_iter->m_Key.m_lMainKey;
					ntf.m_roomID = acceptor_iter->m_roomID;
					ntf.m_strSender = strApplicantName;
					PayloadCHSNGS pld(PayloadCHSNGS::msgNtfNFFriendAdd_Tag, ntf);
					theLRBHandler.SendToNGS(pld, acceptor_iter->m_serverLRBAddr);
				}break;
			}
		}

		MsgCHSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = EC_FE_SUCCESS;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
		m_pChannelContext->SendToUser(pApplicant, pld);
	}
	else if( hObj == CHANNEL_PROCESS_FOLLOW_USER )
	{
		TLock lo(this);

		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wP))
			return;

		//
		CUser* pUser = FindUser( msg.m_lCSN );
		if( !pUser )
		{
			return;
		}

		MsgCHSCli_AnsFollowUser ans;
		ans.Clear();

		ArcVectorT< NLSBaseInfo >::const_iterator iter = msg.m_kContNLSBaseInfo.begin();
		if( msg.m_kContNLSBaseInfo.end() != iter )
		{
			switch( iter->m_lStatus )
			{
			case NLSCLISTATUS_NFCHARLOBBY://NCS
				{
					ans.m_lServerType = NLSCLISTATUS_NFCHARLOBBY;
					ans.m_addr = iter->m_nsap;
				}break;
			case NLSCLISTATUS_NFCHANNELSERVER://CHS
				{
					ans.m_lServerType = NLSCLISTATUS_NFCHANNELSERVER;
					ans.m_roomID = iter->m_roomID;
					ans.m_lGameMode = iter->m_lGameMode;
					ans.m_addr = iter->m_nsap;
				}break;
			case NLSCLISTATUS_NFGAMESERVER://NGS
				{
					ans.m_lServerType = NLSCLISTATUS_NFGAMESERVER;
					ans.m_roomID = iter->m_roomID;
					ans.m_lGameMode = iter->m_lGameMode;
					ans.m_addr = iter->m_nsap;
				}break;
			}
		}

		PayloadCHSCli pld(PayloadCHSCli::msgAnsFollowUser_Tag, ans);
		m_pChannelContext->SendToUser(pUser, pld);

	}
	else if(hObj == CHANNEL_USERJOIN)
	{
		OnSignal_UserJoin(wP, lP);
	}
	else if(hObj == CHANNEL_DIRECTCREATE)
	{
		OnSignal_DirectCreate(wP, lP);
	}
	else if(hObj == CHANNELSIGNAL_REMGLS)
	{
		OnSignal_RemNGS(wP, lP);
	}
	else if(hObj == CHANNELSIGNAL_CHANNELLISTANS)
	{
		OnSignal_ChannelListAns(wP, lP);
	}
	else if(hObj == CHANNELSIGNAL_GLSINFOANS)
	{
		OnSignal_GLSInfoAns(wP, lP);
	}
	else if(hObj == CHANNELSIGNAL_GRDIRECTCREATEANS)
	{
		OnSignal_GRDirectCreateAns(wP, lP);
	}
	else if(hObj == CHANNELSIGNAL_INVITEREQ)
	{
		OnSignal_InviteReq(wP, lP);
	}
	else if(hObj == CHANNELSIGNAL_FROMGLS)
	{
		OnSignalEx(hObj, wP, lP);
	}
    else if(hObj == CHANNELSIGNAL_NLSANSWER)
	{
		TLock lo(this);
		CUser* pUser = (CUser*)lP;
		m_pChannelContext->ChannelJoinFromNLS(pUser, (LONG)wP);		
	}
	else if(hObj == CHANNELSIGNAL_IBBANS)
	{
		TLock lo(this);
		OnSignal_ForwardIBBMsg(wP, lP);		
	}

#ifdef _CHS_ACCUSE
	else if(hObj == ACCUSESIGNAL_AGENTANSWER)
	{
		TLock lo(this);
		CAccuseResultAnswer dataARAns;
		if(lP == 0)
		{
			if(!theAccuseAgent.m_AccuseResultAns.Get((long)wP, dataARAns))
				return;
		}
		else if(lP == 1)
		{
			if(!theHTTPAgent.m_AccuseResultAns.Get((long)wP, dataARAns))
				return;
		}
		else
			return;

		CUser* pUser = m_pChannelContext->FindUser(wP);
		if(pUser == NULL || pUser->GetLink() == NULL)
			return;
		PayloadCHSCli pld(PayloadCHSCli::msgAccuseAns_Tag, MsgCHSCli_AccuseAns(dataARAns.GetAccusedID().c_str(), dataARAns.GetErrorCode()));
		GBuf buf;
		if(!::LStore(buf, pld)) return;
		SendToLink(pUser->GetLink(), buf);
	}
#endif
	///////////// 사용자 메시지 전송 횟수 체크를 위한 타이머.
	else if(hObj == CHANNEL_RESET_USERMSGCNT)
	{
		TLock lo(this);		
		m_pChannelContext->ResetRcvMsgCnt();
	}
	else
	{
		TLock lo(this);
		TBase::OnSignal(hObj, wP, lP);
	}
}

LONG CChannel::IncRoomID()
{
	TLock lo(this);
	if(m_lRoomID == MAXLONG)
		m_lRoomID = 1L;
	return m_lRoomID = ::InterlockedIncrement((LPLONG)&m_lRoomID);
}
LONG CChannel::DecRoomID()
{
	TLock lo(this);
	return 	m_lRoomID = ::InterlockedDecrement((LPLONG)&m_lRoomID);
}
DWORD CChannel::GetRoomID()
{
	return (DWORD)IncRoomID();
}
DWORD CChannel::GetRoomIDbyFreeRoom()
{
	DWORD dRoomIndex = 0;
	dRoomIndex = m_pChannelContext->AllocFreeRoomIndex();
	if (0 == dRoomIndex)		// 빈RoomIndex 없음
	{
		return (DWORD)IncRoomID();
	}
	else
		return dRoomIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// from Listener
void CChannel::OnSignal_UserJoin(WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);	// 채널에서 DB Query 동기화

	CUser * pUser = (CUser *)lParam;
	if(!pUser) {
		LOG(INF_UK, "CHS_CChannel_OnSignal_UserJoin"_LK, "+++++ ASSERT : OnSignal_UserJoin : pUser is NULL!!!");
		return;
	}
	CLink * pLink = pUser->GetLink();

	//LOG(ERR_UK, "OnSignal_UserJoin // Type : ", (pUser->GetUserData()).m_lUserType);

	///////////////////////////////////////////////////////////////////////////////////////////
	// User의 fingerprint check
	// pMsg->m_userInfo.m_sReserved1 => TimeStamp|HashedFingerprint
	// HashedFingerprint== md5hash_say(string::format("%d%d", pMsg->m_userInfo.m_lUSN, TimeStamp)+ "gOGo !!pmANg")
	// TimeStamp> time()- 60*60*4
	// 이 조건이 맞지 않을 경우, Game Client로 Noti
//	BOOL bSuccessFlag = FALSE;
//	int nErrorFlag = 0;
//	int nFIdx = pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.find('|');

	// 암호화 넣어줘야 함!!! -bback99
// 	if(nFIdx != -1)
// 	{
// 		string sTimeStamp = pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.substr(0, nFIdx);
// 		string sHashedFingerprint = pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.substr(nFIdx+1, pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.size() - nFIdx);
// 		char sEncData[1024] = {0x00};
// 		sprintf(sEncData, "%d%s%s", pUser->GetUserData().m_nfUserBaseInfo.m_lUSN, sTimeStamp.c_str(), MD5_KEY);
// 		char sResult[1024] = {0x00};		
// 		md5hash_say(sResult, sEncData);
// 
// 		ULONG lCurTime = time(NULL);
// 		ULONG lJoinTime = atol(sTimeStamp.c_str());
// 		if(strcmp(sHashedFingerprint.c_str(), sResult) == 0)
// 		{
// 			if(lJoinTime > lCurTime - MD5_TIME_GAP_DOWN && lJoinTime < lCurTime + MD5_TIME_GAP_UP )
// 				bSuccessFlag = TRUE;
// 			else
// 				nErrorFlag = 3;
// 		}
// 		else
// 			nErrorFlag = 2;
// 	}	
// 	else
// 		nErrorFlag = 1;
// 
// 	if(bSuccessFlag == FALSE)
// 	{
// 		//	pUser->GetUserData().m_sUID.c_str(), pUser->GetUserData().m_lUSN, nErrorFlag, \
// 		//	pUser->GetUserData().m_sReserved1.c_str());
// 		string strlog = format("+++++ ASSERT : OnRcvJoinChannelReq : Not Matching UID/USN = [%s]/[%d], ErrorFlag = [%d], [%s] ++++ ", \
// 			pUser->GetUserData().m_nfUserBaseInfo.m_sUID.c_str(), pUser->GetUserData().m_nfUserBaseInfo.m_lUSN, nErrorFlag, \
// 			pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.c_str());
// 		LOG(INF_UK, "CHS_CChannel_OnSignal_UserJoin"_LK, strlog);
// 
// 		pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.clear();
// 
// 		::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_JOINCHANNELANS, (WPARAM)pLink, ERR_CHANNEL_DUPLICATE + 100);
// 		return;
// 	}

	pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.clear();

	///////////////////////////////////////////////////////////////////////////////////////////
	BOOL dbRet = TRUE;
	if (pUser->GetGameMode() != 1)
		dbRet = GetNFCharInfo(pUser);
	if(dbRet < 0/* || pUser->GetUSN() == 31545677*/)
	{
		LOG(INF_UK, "CHS_CChannel_OnSignal_UserJoin"_LK, "++++++ Fail DB Query for GameData : [",pUser->GetUSN(), "][", pUser->GetUserID(), "]USN/UID +++++");
		::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_JOINCHANNELANS, (WPARAM)pLink, ERR_CHANNEL_GAMEDATA);
		return;
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	// 기존에 위에 있었지만, 삭제 하고 여기를 수정한 이유
	// 기존에는 기존 접속 종료 후, 새로운 접속까지 먹통 되게 만들었다..
	// 새로운 접속은 유효하게 만들도록 하기 위해서 채널 안에서 중복 접속일 경우, 새로운 유저 연결은 유지되도록 한다.
	CUser* pOldUser = m_pChannelContext->FindUser(pUser->GetCSN());
	if(pOldUser)
	{
		//m_pChannelContext->OnUserDisconnect(pUser->GetUSN());

		// 여기서 메세지를 보내야 끊기기 전에 메세지를 제대로 보낼 수 있다.... 2010/9/14 -bback99 
		MsgCHSCli_ReqBanishUser		req;
		PayloadCHSCli	pld(PayloadCHSCli::msgReqBanishUser_Tag, req);
		m_pChannelContext->SendToUser(pOldUser, pld);

		// 2초후에 끊기 위해서 Timer에 등록한다.
		m_mapBanishUser[(LONG)pUser->GetUSN()] = GetTickCount();		
		LOG(INF_UK, "CHS_CChannel_OnSignal_UserJoin, CChannel::OnSignal() USN :", (LONG)pUser->GetUSN(), ", RegTime :", m_mapBanishUser[(LONG)pUser->GetUSN()]);
	}

	// 새로운 유저 연결...
	m_pChannelContext->AddUser(pUser);
	
	// FreeMode인지 BattleMode인지 여기서 결정한다... 보내야 하는 Ans 메세지를 구분하기 위해서
	// 99 : ZeroTool로 무조건 방만들어서 접속하기 위해서...
	if (pUser->GetGameMode() != 1 && pUser->GetGameMode() != 99)
		m_pChannelContext->OnUserJoinReq(NULL, *pUser);
	else
		m_pChannelContext->OnUserJoinReq_FreeMode(*pUser);

	return;
}

void CChannel::OnSignal_DirectCreate(WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);
	CUser * pUser = (CUser *)lParam;

	///////////////////////////////////////////////////////////////////////////////////////////
	// User의 fingerprint check
	// pMsg->m_userInfo.m_sReserved1 => TimeStamp|HashedFingerprint
	// HashedFingerprint== md5hash_say(string::format("%d%d", pMsg->m_userInfo.m_lUSN, TimeStamp)+ "gOGo !!pmANg")
	// TimeStamp> time()- 60*60*4
	// 이 조건이 맞지 않을 경우, Game Client로 Noti
	BOOL bSuccessFlag = FALSE;
	int nErrorFlag = 0;
	int nFIdx = pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.find('|');
	if(nFIdx != -1)
	{
		string sTimeStamp = pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.substr(0, nFIdx);
		string sHashedFingerprint = pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.substr(nFIdx+1, pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.size() - nFIdx);
		char sEncData[1024] = {0x00};
		sprintf(sEncData, "%d%s%s", pUser->GetUserData().m_nfUserBaseInfo.m_lUSN, sTimeStamp.c_str(), MD5_KEY);
		char sResult[1024] = {0x00};		
		md5hash_say(sResult, sEncData);

		ULONG lCurTime = time(NULL);
		ULONG lJoinTime = atol(sTimeStamp.c_str());
		if(strcmp(sHashedFingerprint.c_str(), sResult) == 0)
		{
			if(lJoinTime > lCurTime - MD5_TIME_GAP_DOWN && lJoinTime < lCurTime + MD5_TIME_GAP_UP )
				bSuccessFlag = TRUE;
			else
				nErrorFlag = 3;
		}
		else
			nErrorFlag = 2;
	}
	else
		nErrorFlag = 1;
	if(bSuccessFlag == FALSE)
	{			
		string strlog = format("+++++ ASSERT : OnSignal_DirectCreate : Not Matching UID/USN = [%s]/[%d], ErrorFlag = [%d], [%s] ++++ ", \
			pUser->GetUserData().m_nfUserBaseInfo.m_sUID.c_str(), pUser->GetUserData().m_nfUserBaseInfo.m_lUSN, nErrorFlag, \
			pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.c_str());
		LOG(INF_UK, "CHS_CChannel_OnSignal_DirectCreate"_LK, strlog);	
		
		m_pChannelContext->RemoveUser(pUser);
		delete pUser;
		return;
	}

	pUser->GetUserData().m_nfUserBaseInfo.m_sReserved1.clear();

	/////////////////////////////////////////////////////////////////////////////////////////
	// 위에서 해준다. - bback99 (2010/9/14)
	CUser* pOldUser = m_pChannelContext->FindUser(pUser->GetCSN());
	if(pOldUser)
	{
		//m_pChannelContext->OnUserDisconnect(pUser->GetUSN());

		// 여기서 메세지를 보내야 끊기기 전에 메세지를 제대로 보낼 수 있다.... 2010/9/14 -bback99 
		MsgCHSCli_ReqBanishUser		req;
		PayloadCHSCli	pld(PayloadCHSCli::msgReqBanishUser_Tag, req);
		m_pChannelContext->SendToUser(pOldUser, pld);

		// 2초후에 끊기 위해서 Timer에 등록한다.
		m_mapBanishUser[(LONG)pUser->GetUSN()] = GetTickCount();		
		LOG(INF_UK, "CHS_CChannel_OnSignal_UserJoin, CChannel::OnSignal() USN :", (LONG)pUser->GetUSN(), ", RegTime :", m_mapBanishUser[(LONG)pUser->GetUSN()]);
	}

	m_pChannelContext->AddUser(pUser);

	
	LPXBUF pXBuf = (LPXBUF)wParam;
	ASSERT(pXBuf);
	GBuf buf(pXBuf, FALSE);
	PayloadCliCHS pld;
	VALIDATE(::BLoad(pld, buf));
	m_pChannelContext->OnUserMsg(pUser, pld);
	return;
}

void CChannel::PostRemNGS(const NSAP& nsap)
{
	NSAP* pNSAP = new NSAP;
	VALIDATE(pNSAP);
	pNSAP->BCopy(nsap);

	::XsigQueueSignal(::GetChannelThreadPool(), this, CHANNELSIGNAL_REMGLS,  (WPARAM)pNSAP, (LPARAM)0);
}

void CChannel::OnSignal_RemNGS(WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);
	NSAP* pNSAP = (NSAP*)wParam;
	ASSERT(pNSAP);

	if(m_pChannelContext->m_ChannelInfo.m_lUserCount < 1) 
	{
		delete(pNSAP);
		return;
	}

	m_pChannelContext->OnRemNGS(*pNSAP);
	delete(pNSAP);
}

///////////////////////////////////////////////////////////////////////////////////
// From IBB
///////////////////////////////////////////////////////////////////////////////////
void CChannel::OnSignal_ForwardIBBMsg(WPARAM wParam, LPARAM lParam)
{
	//  자신에게만 응답을 보낸다.. unique ..  
	IBBDataBus * pBus = (IBBDataBus *)wParam;
	GBuf bufPayload((LPXBUF)lParam, FALSE);

	CUser * pUser = m_pChannelContext->m_UsersMap.Find(pBus->m_lUSN); // TODO(acepm83@neowiz.com) 현재 CSN기반으로 바꿀 수 없다.
	if(!pUser) 
	{
		delete pBus;
		return;
	}
	
	if(pBus->m_bUserGameDataChanged == TRUE)
	{
		if(pBus->m_sGameData != "error!!!")
			pUser->UpdateGameData(pBus->m_sGameData);	
		m_pChannelContext->SendToAll(bufPayload);
	}
	else
	{
		if(pUser->GetBLSStatFlag() == -1L )
		{
			m_pChannelContext->SendToAll(bufPayload);
			pUser->SetBLSStatFlag(1L);
		}
		else 
			m_pChannelContext->SendToUser(pBus->m_lUSN, bufPayload); // TODO(acepm83@neowiz.com) 현재 CSN기반으로 바꿀 수 없다.
	}

	delete pBus;
}

///////////////////////////////////////////////////////////////////////////////////
// From LB 
///////////////////////////////////////////////////////////////////////////////////

void CChannel::PostChannelListAns(long lCSN, GBuf& buf)
{
	LPXBUF pXBuf = buf.Detach();
	//LPXBUF pXBuf = ::XbufCreate(buf.GetXBuf());
	VALIDATE(pXBuf);
	::XsigQueueSignal(::GetChannelThreadPool(), this, CHANNELSIGNAL_CHANNELLISTANS,  (WPARAM)lCSN, (LPARAM)pXBuf);
}

void CChannel::OnSignal_ChannelListAns(WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);

	LONG lCSN = (LONG)wParam;
	LPXBUF pXBuf = (LPXBUF)lParam;
	ASSERT(pXBuf);
	GBuf buf(pXBuf, FALSE);

#ifdef USE_CRCREPORTER
	//
	// 채널 리스트 요구자와 대기자 모두에게 전달.
	//
	GBuf buf2(buf);
	vector<LONG> vecGSN;
	m_pChannelContext->m_RCReporter.GetSendUSNList(vecGSN);
	DWORD _i = 0;
	while(_i < vecGSN.size())
	{
		m_pChannelContext->SendToUser(vecGSN[_i], buf2); // TODO(acepm83@neowiz.com) 승호님에게 문의
		_i++;
	}

	//자신이 속한 채널은 최신정보로 갱신
	PayloadCHSCli pld;
	::LLoad(pld, buf2);
	ChannelInfoList & cinfo = pld.un.m_msgChannelListAns->m_lstChannelInfoList;
	ForEachElmt(ChannelInfoList, cinfo, it, jt)
	{
		ChannelInfo & info = *it;
		if(info.m_channelID == m_pChannelContext->m_ChannelInfo.m_channelID)
		{
			info.m_lUserCount = m_pChannelContext->m_ChannelInfo.m_lUserCount;
			info.m_lWaitingUserCount = m_pChannelContext->m_ChannelInfo.m_lWaitingUserCount;
			info.m_lRoomCount = m_pChannelContext->m_ChannelInfo.m_lRoomCount;
		}
	}

#endif
	
	m_pChannelContext->SendToUser(lCSN, buf);
}

void CChannel::PostGLSInfoAns(long lCSN, GBuf& buf)
{
	LPXBUF pXBuf = buf.Detach();
	//LPXBUF pXBuf = ::XbufCreate(buf.GetXBuf());
	VALIDATE(pXBuf);
	::XsigQueueSignal(::GetChannelThreadPool(), this, CHANNELSIGNAL_GLSINFOANS,  (WPARAM)lCSN, (LPARAM)pXBuf);
}

void CChannel::OnSignal_GLSInfoAns(WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);

	LONG lCSN = (LONG)wParam;
	LPXBUF pXBuf = (LPXBUF)lParam;
	ASSERT(pXBuf);
	if(!pXBuf)
		return;
	GBuf buf(pXBuf, FALSE);

	m_pChannelContext->SendToUser(lCSN, buf);
}

void CChannel::PostNtfNFFriendAdd(long lCSN, GBuf& buf)
{
	CUser* pUser = m_pChannelContext->m_UsersMap.Find(lCSN);
	if( pUser )
	{
		MsgCHSCli_NtfNFFriendAdd ntf;
		if( !::LLoad(ntf, buf) )
			return;

		PayloadCHSCli pld(PayloadCHSCli::msgNtfNFFriendAdd_Tag, ntf);
		m_pChannelContext->SendToUser(pUser, pld);
	}
}

void CChannel::PostNtfNFFriendAccept(long lCSN, GBuf& buf)
{
	CUser* pUser = m_pChannelContext->m_UsersMap.Find(lCSN);
	if( pUser )
	{
		MsgCHSCli_NtfNFFriendAccept ntf;
		if( !::LLoad(ntf, buf) )
			return;

		PayloadCHSCli pld(PayloadCHSCli::msgNtfNFFriendAccept_Tag, ntf);
		m_pChannelContext->SendToUser(pUser, pld);
	}
}

void CChannel::PostNtfNFLetterReceive(long lCSN)
{
	CUser* pUser = m_pChannelContext->m_UsersMap.Find(lCSN);
	if( pUser )
	{
		MsgCHSCli_NtfNFLetterReceive ntf;
		PayloadCHSCli pld(PayloadCHSCli::msgNtfNFLetterReceive_Tag, ntf);
		m_pChannelContext->SendToUser(pUser, pld);
	}
}

void CChannel::PostGRDirectCreateAns(long lCSN, GBuf& buf)
{
	//LPXBUF pXBuf = ::XbufCreate(buf.GetXBuf());
	LPXBUF pXBuf = buf.Detach();
	VALIDATE(pXBuf);
	::XsigQueueSignal(::GetChannelThreadPool(), this, CHANNELSIGNAL_GRDIRECTCREATEANS,  (WPARAM)lCSN, (LPARAM)pXBuf);
}

void CChannel::OnSignal_GRDirectCreateAns(WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);

	LONG lCSN = (LONG)wParam;
	LPXBUF pXBuf = (LPXBUF)lParam;
	GBuf buf(pXBuf, FALSE);

	PayloadCHSCli pld;
	::LLoad(pld, buf);
	pld.un.m_msgGRDirectCreateAns->m_sCategoryName = m_pChannelContext->m_ChannelInfo.m_sCategoryName.c_str();
	pld.un.m_msgGRDirectCreateAns->m_sChannelName = m_pChannelContext->m_ChannelInfo.m_sTitle.c_str();
	GBuf buf2;
	::LStore(buf2, pld);

	m_pChannelContext->SendToUser(lCSN, buf2);
}

BOOL CChannel::OnRcvInvitationReq(const NSAP& nsap, const RoomID& roomID, const UserBaseInfo& userInfo, const xstring& sPasswd, const xstring& sMsg, const USNList& usnList, ArcListT<LONG>& lErrorList)
{
	MsgCHSCli_GRInvitationReq rMsg;

	rMsg.m_nsapNGS.BCopy(nsap);
	rMsg.m_roomID.BCopy(roomID);
	rMsg.m_userInfo.BCopy(userInfo);
	rMsg.m_sPasswd.assign(sPasswd.c_str(), sPasswd.length());
	rMsg.m_sMessage.assign(sMsg.c_str(), sMsg.length());

	ForEachCElmt(USNList, usnList, i, j)
	{
		LONG lErrorCode = m_pChannelContext->OnInvitationReq(*i, rMsg);
		lErrorList.push_back(lErrorCode);
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// From PMS 
///////////////////////////////////////////////////////////////////////////////////
void CChannel::OnAnnounceMsgFromPMS(const GBuf& buf)
{
	m_pChannelContext->SendToAll(buf);
}


///////////////////////////////////////////////////////////////////////////////////
// CChannelContext
///////////////////////////////////////////////////////////////////////////////////

CChannelContext::CChannelContext(CChannel* pChannel, const ChannelBaseInfo& binfo) : m_pChannel(pChannel)
{
	m_ChannelInfo.Clear();
	m_roomlist.clear();
	m_roomlist_Test.clear();
	m_lstRoomIndexFree.clear();
	m_ChannelInfo.Clear();
	m_ChannelInfo.ChannelBaseInfo::BCopy(binfo);
	m_ChannelInfo.m_nsapCHS.BCopy(theChannelDir.GetCHSNsap());
} 

CUser* CChannelContext::FindUser(long lCSN) 
{
	return m_UsersMap.Find(lCSN);
}

CUser * CChannelContext::FindUser(xstring sUID)
{	
	return m_UsersMap.Find(sUID); 
}

void CChannelContext::SendToUser(long lCSN, const PayloadCHSCli& pld)
{
	CUser * pUser = m_UsersMap.Find(lCSN);
	if(!pUser) 
		return;
	SendToUser(pUser, pld);
}
void CChannelContext::SendToUser(long lCSN, const GBuf & buf)
{
	CUser * pUser = m_UsersMap.Find(lCSN);
	if(!pUser) 
		return;
	SendToUser(pUser, buf);
}

BOOL CChannelContext::IsUserConnected(LONG lCSN)
{
	CUser * pUser = m_UsersMap.Find(lCSN);
	if(!pUser) 
		return FALSE;

	CLink *pLink = pUser->GetLink();
	if (!pLink)
		return FALSE;

	return m_pChannel->IsLinkRegistered(pLink);
}

void CChannelContext::SendToUser(CUser* pUser, const PayloadCHSCli& pld)
{
	GBuf buf;
	BOOL bRet = ::LStore(buf, pld);
	VALIDATE(bRet);
	SendToUser(pUser, buf);
}
void CChannelContext::SendToUser(CUser* pUser, const GBuf & buf)
{
	if(!pUser) return;
	CLink *pLink = pUser->GetLink();
	if(!pLink) return;
	m_pChannel->SendToLink(pLink, buf);
}

void CChannelContext::SendToAll(const PayloadCHSCli& pld, long lUSN)
{
	GBuf buf; 
	::LStore(buf, pld);

	ForEachElmt(CUserMap, m_UsersMap, i, j) 
	{
		CUser * pUser = *i;
		if(!pUser)
			continue;
		if(pUser->GetUSN() == lUSN)
			continue;
		CLink * pLink = pUser->GetLink();
		if(!pLink)
			continue;
		m_pChannel->SendToLink(pLink, buf);
	}
}

void CChannelContext::SendToAll(const GBuf& buf)
{
	ForEachElmt(CUserMap, m_UsersMap, i, j) 
	{
		CUser* pUser = *i;
		if(!pUser)
			continue;
		CLink *pLink = pUser->GetLink();
		if(!pLink) 
			continue;
		m_pChannel->SendToLink(pLink, buf);
	}
}

void CChannelContext::SendToInviteAll(PayloadInvitation& pld)
{
	GBuf buf;
	::LStore(buf, pld);

	m_pChannel->SendToInviteAll(buf);
}

void CChannelContext::AddUser(CUser* pUser)
{
	CLink* pLink = pUser->GetLink();
	if(!pUser || !pLink) {
		LOG(INF_UK, "CHS_CChannelContext"_LK, "++++ AddUser : ChannelContext : Null Point or Not found Link ++++");
		return;
	}
	m_UsersMap.Add(pUser);
//	AddLatentUserCount();
	m_ChannelInfo.m_lUserCount++;
	// 이값은 이 채널에 접속했다 Room으로 이동한 사용자를 포함
	m_ChannelInfo.m_lWaitingUserCount++;

	// 통계를 위한 테이블 갱신.
	UserStatInfo info;
	info.Copy(pUser->GetUserData().m_nfUserBaseInfo);

	//LOG(ERR_UK, "CHS_CChannelContext"_LK, "PCRoomUser2 : ", (pUser->GetUserData()).m_lUserType);

	if (info.m_lIsPCRoomUser != 0)
	{
		m_ChannelInfo.m_lPCRoomUserCount++;
//		LOG(ERR_UK, "CHS_CChannelContext"_LK, "PCRoomUser : ", info.m_lIsPCRoomUser, "CNT : ", m_ChannelInfo.m_lPCRoomUserCount);
	}

	if(!m_pChannel->AddSocket(pLink))
	{
		LOG(INF_UK, "CHS_CChannelContext"_LK, "fail AddSocket in AddUser : ChannelContext ");
		OnUserDisconnect(pUser->GetUSN());
	}
#ifdef _PMS_CODE_SEPERATION_
	theStatTable.UpdateSessionStatInfoDelta(m_ChannelInfo.m_channelID, info, 1, 1);
#endif
	theCHSInfoDir.UpdateCHSInfo(m_ChannelInfo.m_channelID, m_ChannelInfo.m_lUserCount, m_ChannelInfo.m_lWaitingUserCount, m_ChannelInfo.m_lRoomCount, m_ChannelInfo.m_lPCRoomUserCount);

	theStatTable.SetStatInfo(m_ChannelInfo.m_channelID.m_lSSN, info/*pUser->GetUserData()*/, 1);
	
	if (info.m_lIsPCRoomUser != 0)
		theStatTable.SetStatInfoPC(m_ChannelInfo.m_channelID.m_lSSN, info/*pUser->GetUserData()*/, 1);



#ifdef _PMS_PCROOM_TEST_
	theStatTable.SetStatInfoPC(m_ChannelInfo.m_channelID.m_lSSN, info/*pUser->GetUserData()*/, 1);
#endif
	

#ifdef _USE_MSGCOUNT
	theStatTable.AddMsgCount(m_ChannelInfo.m_channelID.m_lSSN);
#endif
}

void CChannelContext::RemoveUser(CUser* pUser)
{
	if(!pUser)
		return;

	// send to all : user exit notificatio message
	long lUSN = pUser->GetUSN();
	if(!ISVALID_USN(lUSN))
	{
		LOG(INF_UK, "CHS_CChannelContext"_LK, "+++++ ASSERT :RemoveUser +++++");
		return;
	}
	ASSERT(ISVALID_USN(lUSN));

	PayloadCHSCli msg(PayloadCHSCli::msgGCExitNtf_Tag, 
		MsgCHSCli_GCExitNtf(	m_ChannelInfo.m_lUserCount, 
								m_ChannelInfo.m_lWaitingUserCount, 
								lUSN
								)
		);
	SendToAll(msg, pUser->GetUSN());

	{
		PayloadInvitation pldInvitation(PayloadInvitation::msgUserExitNtf_Tag);
		MsgUserExitNtf& rMsg = *(pldInvitation.un.m_msgUserExitNtf);
		rMsg.m_lUSN = lUSN;

		SendToInviteAll(pldInvitation);
	}

	CLink* pLink = pUser->GetLink();
	if(pLink)
	{
		pUser->SetLink(NULL);
		pLink->SetUser(NULL);
		if (pLink->IsValid())
		{
			m_pChannel->RemoveSocket(pLink);
			pLink->Unregister();
		}
		delete(pLink);
	}
	m_UsersMap.Remove(pUser);	
	m_ChannelInfo.m_lUserCount--;
	m_ChannelInfo.m_lWaitingUserCount--;

	// 통계를 위한 테이블 갱신.
	UserStatInfo info;
	info.Copy(pUser->GetUserData().m_nfUserBaseInfo);

	if (info.m_lIsPCRoomUser != 0)
		m_ChannelInfo.m_lPCRoomUserCount--;

#ifdef _PMS_CODE_SEPERATION_
	theStatTable.UpdateSessionStatInfoDelta(m_ChannelInfo.m_channelID, info, -1, -1);
#endif
	theCHSInfoDir.UpdateCHSInfo(m_ChannelInfo.m_channelID, m_ChannelInfo.m_lUserCount, m_ChannelInfo.m_lWaitingUserCount, m_ChannelInfo.m_lRoomCount, m_ChannelInfo.m_lPCRoomUserCount);
	
	theStatTable.SetStatInfo(m_ChannelInfo.m_channelID.m_lSSN, info/*pUser->GetUserData()*/, -1);

	if (info.m_lIsPCRoomUser != 0)
		theStatTable.SetStatInfoPC(m_ChannelInfo.m_channelID.m_lSSN, info/*pUser->GetUserData()*/, -1);
}




// 같은 CHS내의 중복이라던지, NLS로부터 중복이 체크되어 끊으라고 요청이 올 경우 호출 되는 함수
// 아직 Link에 ADD 되기 전이기 때문에 USN으로 FindUser로 찾아도 하나밖에 검색되지 않는다.
void CChannelContext::OnUserDisconnectFromNLS(long lCSN, CUser* pNewUser)
{	
	// 기존 유저 삭제
	CUser* pOldUser = FindUser(lCSN);
	if(!pOldUser)
		return;

	RemoveUser(pOldUser);	
	delete pOldUser;
}


void CChannelContext::OnUserChatPart(xstring sUID)
{
	CUser* pUser = FindUser(sUID);
	if(!pUser) 
	{
		return;
	}
	RemoveUser(pUser);	
	LOG(INF_UK, "CHS_CChannelContext"_LK, "===== Disconnect User : from Chat Part : [", sUID, "]UID =====");
	delete pUser;
}

void CChannelContext::OnUserChangeChannel(long lCSN)
{
	CUser* pUser = FindUser(lCSN);
	if(!pUser)
		return;

	RemoveUser(pUser);
	delete pUser;
}

//void CChannelContext::SMGWChangeChannel(LstUID & lstUID)
//{
//#pragma oMSG("전달되지 않음.. 삭제됨....")
//	LONG size = lstUID.size();
//	theErr.LOG(2, "CHS_CChannelContext", "++++++ SMGW : Change Channel : lstUID size [%d] +++++++", size);
//
//	vector<long> vecKickUSN;
//	ForEachElmt(CUserMap, m_UsersMap, it1, jt1)
//	{
//		CUser* p = *it1;
//		// check if p in lstUID
//		ForEachElmt(LstUID, lstUID, itu, jtu)
//		{
//			string& sUID = *itu;
//			if(p->GetUserID() == sUID)
//				break;
//		}
//		if(itu == lstUID.end())
//			vecKickUSN.push_back(p->GetUSN());
//	}
//
//	for(unsigned i = 0; i < vecKickUSN.size(); ++i)
//	{
//		long lUSN = vecKickUSN[i];
//		OnUserChangeChannel(lUSN);
//	}
//}

//switch(pld.mTagID)
//{
//case PayloadCliCHS::msgJoinChannelReq_Tag:	// Listener에서 .. msg를 직접 넘기는 것으로 처리..
//	OnUserJoinReq(pld.un.m_msgJoinChannelReq, *pUser);	
//	break;	
//
//default: 
//	LOG(INF_UK, "CHS_CChannelContext"_LK, "+++++ received unknown message from Client +++++ : PID=[", pld.mTagID, "]"); 
//	OnUserDisconnect(lUSN);		
//	break;
//}

#define BEGIN_SWITCH	switch(pld.mTagID) {
#define END_SWITCH		default: \
	LOG(INF_UK, "CHS_CChannelContext"_LK, "+++++ received unknown message from Client +++++ : PID=[", pld.mTagID, "] CSN=[", lCSN, "]"); \
	m_pChannel->RemoveSocket(pLink); \
	pLink->Unregister();		\
	break; \
	}
#define BEGIN_CASE(ADATA) case PayloadCliCHS::msg##ADATA##_Tag:
#define MIDDLE_CASE(ADATA) ; break; \
	case PayloadCliCHS::msg##ADATA##_Tag:
#define END_CASE	break;	

void CChannelContext::OnUserMsg(CUser * pUser, PayloadCliCHS& pld)
{
	LONG lCSN = pUser->GetCSN();
	if(!ISVALID_CSN(lCSN))	// INVALID_CSN
		return;

	/////////// 사용자 메시지 수신 횟수 체크...
	//// 5초안에 15개 이상 들어오면 무시한다. 5초안에 50개 이상오면 짤라버림.
	static stlMinCount = 15;
	static stlMaxCount = 50;

	stlMinCount = 500;
	stlMaxCount = 1500;
	if(pUser->m_lRcvMsgCnt > stlMaxCount)
	{
		LOG(INF_UK, "CHS_CChannelContext_MsgCnt_50"_LK, " !!! User Send Msg Overflow (50 over): CSN : [%d], PayloadType = [", lCSN, ", PayloadType = [", pld.mTagID, "]");
		CLink* pLink = pUser->GetLink();
		GBuf buf;
		buf.SetLength(0);
		pLink->DoSend(buf);
		return;
	}
	if(pUser->m_lRcvMsgCnt > stlMinCount)
	{
		if(pUser->m_lRcvMsgCnt == stlMinCount+1)
			LOG(INF_UK, "CHS_CChannelContext_MsgCnt_15"_LK, " !!! User Send Msg Overflow (15 over): CSN : [%d], PayloadType = [", lCSN, ", PayloadType = [", pld.mTagID, "]");
		pUser->m_lRcvMsgCnt++;
		return;
	}
	else
		pUser->m_lRcvMsgCnt++;
	
	// 이어치기 관련 어뷰징 코드 추가
	CLink* pLink = pUser->GetLink();
	if (pLink->GetDiconCheck() == TRUE)
	{
		LOG(INF_UK, "CHS_CChannelContext_DisconCheck"_LK, " ++++++++++++++++++ Disconnect Flag TRUE Setting :: CSN : [", pUser->GetCSN(), "], PayloadType = [", pld.mTagID, "]" );
		return;
	}


	BEGIN_SWITCH
		BEGIN_CASE(JoinChannelReq)
			OnUserJoinReq(pld.un.m_msgJoinChannelReq, *pUser);
		MIDDLE_CASE(GRInviteReq)
			OnDirectCreateReq(pld.un.m_msgGRDirectCreateReq, lCSN);
		MIDDLE_CASE(GRJoinInfoReq)
			OnDirectCreateReq(pld.un.m_msgGRDirectCreateReq, lCSN);
		MIDDLE_CASE(GRDirectCreateReq)
			OnDirectCreateReq(pld.un.m_msgGRDirectCreateReq, lCSN);
		MIDDLE_CASE(NGSInfoReq)
			OnNGSInfoReq(pld.un.m_msgNGSInfoReq, lCSN, CONNECTTYPE_NORMAL);
		MIDDLE_CASE(ChannelListReq)
			OnChannelListReq(lCSN, pld.un.m_msgChannelListReq->m_lCategory)
		MIDDLE_CASE(GCExitReq)
			OnUserDisconnect(lCSN)
		MIDDLE_CASE(DirectInviteReq)
			OnDirectInviteReq(pld.un.m_msgDirectInviteReq, pUser->GetUserID())

		MIDDLE_CASE(ChatMsgTo)
			OnChatMsgTo(pld.un.m_msgChatMsgTo, pUser->GetCSN())
		MIDDLE_CASE(ChatMsgAll)
			OnChatMsgAll(pld.un.m_msgChatMsgAll, pUser->GetCSN())
		MIDDLE_CASE(LoginStateChangeNtf)
			OnLoginStateChangeNtf(pld.un.m_msgLoginStateChangeNtf, pUser->GetCSN())
		MIDDLE_CASE(AccuseReq)
			OnAccuseReq(pld.un.m_msgAccuseReq, *pUser);
			OnLoginStateChangeNtf(pld.un.m_msgLoginStateChangeNtf, pUser->GetCSN());
		MIDDLE_CASE(GetUserGameInfoReq)
			OnGetUserGameInfoReq(pld.un.m_msgGetUserGameInfoReq, *pUser);	
		MIDDLE_CASE(GetUserRankInfoReq)
			OnGetRankInfoReq(pld.un.m_msgGetUserRankInfoReq, *pUser);

		MIDDLE_CASE(ReqRoomList)
			OnReqRoomList(pld.un.m_msgReqRoomList, *pUser);
		MIDDLE_CASE(ReqChangeParts)
			OnReqChangeParts(pld.un.m_msgReqChangeParts, *pUser);
		MIDDLE_CASE(ReqChangeCardSlot)
			OnReqChangeCardSlot(pld.un.m_msgReqChangeCardSlot, *pUser);
		MIDDLE_CASE(ReqExchangeCards)
			OnReqExchangeCards(pld.un.m_msgReqExchangeCards, *pUser);
		MIDDLE_CASE(ReqBuyItem)
			OnReqBuyItem(pld.un.m_msgReqBuyItem, *pUser);
		MIDDLE_CASE(ReqRemoveItem)
			OnReqRemoveItem(pld.un.m_msgReqRemoveItem, *pUser);
		MIDDLE_CASE(ReqOpenCardPack)
			OnReqOpenCardPack(pld.un.m_msgReqOpenCardPack, *pUser);
		MIDDLE_CASE(ReqChangeQuickSlot)
			OnReqChangeQuickSlot(pld.un.m_msgReqChangeQuickSlot, *pUser);
		MIDDLE_CASE(ReqAchvInfo)
			OnReqAchieveInfo(pld.un.m_msgReqAchvInfo, *pUser);
		MIDDLE_CASE(ReqNFFriendAccept)
			OnReqNFFriendAccept(pld.un.m_msgReqNFFriendAccept, *pUser);
		MIDDLE_CASE(ReqAwaiterList)
			OnReqAwaiterList(pld.un.m_msgReqAwaiterList, *pUser);
		MIDDLE_CASE(ReqNFFriendAdd)
			OnReqNFFriendAdd(pld.un.m_msgReqNFFriendAdd, *pUser);
		MIDDLE_CASE(ReqNFBlockOrUnBlock)
			OnReqNFBlockOrUnBlock(pld.un.m_msgReqNFBlockOrUnBlock, *pUser);
		MIDDLE_CASE(ReqFollowUser)
			OnReqFollowUser(pld.un.m_msgReqFollowUser, *pUser);
		MIDDLE_CASE(ReqNextEnchantInfo)
			OnReqNextEnchantInfo(pld.un.m_msgReqNextEnchantInfo, *pUser);
		MIDDLE_CASE(ReqItemEnchant)
			OnReqItemEnchant(pld.un.m_msgReqItemEnchant, *pUser);
		MIDDLE_CASE(ReqAquaFish)
			OnReqAquaFish(pld.un.m_msgReqAquaFish, *pUser);
		END_CASE
	END_SWITCH
}

void CChannelContext::ChannelJoinFromNLS(CUser * pUser, LONG lMsgType)
{
#ifdef _CHSNLS
	if(!pUser) {
		LOG(INF_UK, "CHS_CChannelContext"_LK, "fail CChannel OnSignal : ChannelJoinFromNLS.. : pUser is NULL");
		return;
	}

	// NF에서는 필요 없는 코드 이므로 삭제한다. -bback99 (2010/9/15)
	//// 기존 유저가 있는지 찾는다.
	//// 같은 USN으로 찾더라도, pUser는 아직 m_Users 에 추가 되지 않은 상태이므로, pOldUSer는 기존 유저이다. 끊어버리자 -ㅅ-;; -bback99
	//CUser* pOldUser = FindUser(pUser->GetUSN());
	//if (pOldUser) {
	//	CLink * pLink = pOldUser->GetLink();
	//	DWORD dwIndex = 0UL;
	//	if(pLink)
	//		dwIndex = pLink->GetIndex();
	//	LOG(INF_UK, "CHS_CChannelContext"_LK, "Connected User : JoinChannelReq USN:Link Index = ", pOldUser->GetUSN(), ":", dwIndex, "!!!");
	//	OnUserDisconnect(pUser->GetUSN());
	//	return;
	//}

	if(lMsgType == NLSMSGTYPE_NJOINCHANNEL)	// message 구분..  normal join & direct join
	{
		LONG lErrorCode = pUser->GetErrorCode();
		// 이 응답을 받기 전에 Queck update mode, slow update mode로 메세지를 보낼건지 결정해야 한다. 
		// 게임 한판이 끝나기 전에 접속을 해제하지 않는 게임의 경우.. slow update mode로 진행해야 한다.
		// 원한다면 client에게 다른 접속이 있었음을 알려줄 수도 있다.
		if(lErrorCode == E_NLS_EXISTKEY || lErrorCode == E_NLS_EXISTVALUE)
		{
		// 이 Error code는 Slow Update Mode일 경우에만 수신된다. 이 경우 Quick 지우고자 할 경우 quick update로 다시 메세지를 전송해야 한다.
		// 이 부분에 대해서는 아직 client와 협의된 바가 없어, 모든 경우 Quick update 한다.
		}
		else if(lErrorCode == E_NLS_NOTEXIST || lErrorCode == E_NLS_NOTDEFINE)
		{
			pUser->SetErrorCode(CHS_UNKNOWN);	
			LOG(INF_UK, "CHS_CChannelContext"_LK, "check Unknown Symbol.. NJoinChannel");
			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_JOINCHANNELANS, (WPARAM)pUser, (LPARAM)ERR_CHANNEL_RELOGON);
			return;
		}
		pUser->SetErrorCode(CHS_SOK);
		m_pChannel->OnSignal_UserJoin(NULL, (LPARAM)pUser);
	}
	else if(lMsgType == NLSMSGTYPE_INVITCHANNEL)
	{
		LONG lErrorCode = pUser->GetErrorCode();
		if(lErrorCode == E_NLS_EXISTKEY || lErrorCode == E_NLS_EXISTVALUE)
		{
		// 이 Error code는 Slow Update Mode일 경우에만 수신된다. 이 경우 Quick 지우고자 할 경우 quick update로 다시 메세지를 전송해야 한다.
		// 이 부분에 대해서는 아직 client와 협의된 바가 없어, 모든 경우 Quick update 한다.
		}
		else if(lErrorCode == E_NLS_NOTEXIST || lErrorCode == E_NLS_NOTDEFINE)
		{
			pUser->SetErrorCode(CHS_UNKNOWN);	
			LOG(INF_UK, "CHS_CChannelContext"_LK, "check Unknown Symbol.. InviteChannel");
			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_INVITECHANNELANS, (WPARAM)pUser, (LPARAM)ERR_CHANNEL_RELOGON);
			return;
		}
		//////// CUser NULL 로 안만들어주면 사용자 인원수 감소한다. 
		if(pUser == NULL)
		{
			LOG(INF_UK, "CHS_CChannelContext"_LK, "CChannelContext::ChannelJoinFromNLS[LCSMSGTYPE_INVITCHANNEL] pUser is NULL");
			return;
		}
		
		/*//////// 이거 안해주면 안빠짐. //////////
		RoomID rid;
		pUser->NLSGetRoomID(rid);
		if(IS_SINGLE_JOIN_SSN(rid.m_lSSN))
			theNLSManager.ReqRemUser(pUser->GetUSN(), rid);

		DWORD dwGRIID = pUser->m_dwGRIID;
		CLink* pLink = pUser->GetLink();
		pUser->SetLink(NULL);
		pLink->SetUser(NULL);
		delete(pUser);
		///////////////////////////////////////////////////////////
		m_pChannel->OnSignal_InviteReq((WPARAM)pLink, dwGRIID);	*/	
		m_pChannel->OnSignal_InviteReq((WPARAM)pUser->GetLink(), pUser->m_dwGRIID);
	}
	else if(lMsgType == NLSMSGTYPE_DJOINCHANNEL)	// message 구분.. normal join & direct join
	{
		LONG lErrorCode = pUser->GetErrorCode();
		if(lErrorCode == E_NLS_EXISTKEY || lErrorCode == E_NLS_EXISTVALUE)
		{
		// 이 Error code는 Slow Update Mode일 경우에만 수신된다. 이 경우 Quick 지우고자 할 경우 quick update로 다시 메세지를 전송해야 한다.
		// 이 부분에 대해서는 아직 client와 협의된 바가 없어, 모든 경우 Quick update 한다.
		}
		else if(lErrorCode == E_NLS_NOTEXIST || lErrorCode == E_NLS_NOTDEFINE)
		{
			pUser->SetErrorCode(CHS_UNKNOWN);	// 임시로 DB Error로 처리..
			LOG(INF_UK, "CHS_CChannelContext"_LK, "check Unknown Symbol.. DJoinChannel");
			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_DJOINCHANNELANS, (WPARAM)pUser, (LPARAM)ERR_CHANNEL_RELOGON);
			return;
		}
		m_pChannel->OnSignal_DirectCreate(NULL, (LPARAM)pUser);
	}

#endif
}



LONG CChannelContext::OnUserJoinReq_FreeMode(CUser& user)
{
	NFRoomInfoInChannelList	lstRoom;

	// 방을 생성하거나 조인 시킨 결과를 보낸다.. FreeMode 이므로... (2011/5/17)
	GetWaitRoomList(m_pChannel->GetChannelID(), lstRoom, 5);
	if (lstRoom.size() <= 0)
	{
		// 방생성해서 생성 결과를 전송
		MsgCliCHS_NGSInfoReq msg(m_pChannel->GetChannelID());
		OnNGSInfoReq(&msg, user.GetCSN(), CONNECTTYPE_FREEMODEROOM);
	}
	else
	{
		// WaitRoom이 있으면 대략 5개 정도로 추려서 보낸다...(타이밍상 풀이 될 수도 있으므로)
		PayloadCHSCli pld(PayloadCHSCli::msgAnsRoomList_Tag, MsgCHSCli_AnsRoomList(NF::G_NF_ERR_SUCCESS, lstRoom));
		SendToUser(&user, pld);
	}
	return 0;	
}

BOOL CChannelContext::OnDirectCreateReq_LCS(CUser & user)
{
	MsgCliCHS_NGSInfoReq msg(user.GetChannelID());
	OnNGSInfoReq(&msg, user.GetCSN(), CONNECTTYPE_DIRECT);
	return TRUE;
}

tstring CChannelContext::GetCurrentDate()
{
	TCHAR buf[18] = {0};
	SYSTEMTIME st;
	GetLocalTime(&st);
	int ret = GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, _T("yyyyMMdd"), buf, sizeof(buf));
	if (0 == ret)
	{
		theLog.Put(ERR_UK, "GetDateFormat fail. GetLastError=", GetLastError());
		return _T("");
	}
	return buf;
}

//////// 이거 필요없다. ///////////
void CChannel::PostInviteReq(CLink* pLink, const DWORD dwGRIID)
{
	::XsigQueueSignal(::GetChannelThreadPool(), this, CHANNELSIGNAL_INVITEREQ,  (WPARAM)pLink, (LPARAM)dwGRIID);
}

void CChannel::OnSignal_InviteReq(WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);

	CLink* pLink = (CLink*)wParam;
	DWORD dwGRIID = (DWORD)lParam;

	if(pLink)
	{	
		///////// 이거 안해주면 안빠짐. //////////
		long lDeleteUSN = 0;
		CUser* pUser = pLink->GetUser();
		RoomID rid;

		if(pUser)
		{
			lDeleteUSN = pUser->GetUSN();
			pUser->NLSGetRoomID(rid);

			RoomID rid;
			pUser->NLSGetRoomID(rid);
			TKey key(pUser->GetUSN(), pUser->GetCSN());
			theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, rid, pUser->GetUserData().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel);
			
			//CLink* pLink = pUser->GetLink();
			pUser->SetLink(NULL);
			pLink->SetUser(NULL);
			delete(pUser);
		}
		///////////////////////////////////////////////////////////

		AddSocket(pLink);
		
		NFRoomInfoInChannel rinfo;
		if(m_pChannelContext->GetRoomWithRoomID(dwGRIID, &rinfo))
		{
			PayloadCHSCli pld(PayloadCHSCli::msgGRInviteInfoAns_Tag, 
				MsgCHSCli_GRInviteInfoAns(
					rinfo.m_nsapGLS,
					rinfo.m_dwGRIID,
					m_pChannelContext->m_ChannelInfo.m_sCategoryName,
					m_pChannelContext->m_ChannelInfo.m_sTitle,
					0
					)
				);
			pLink->DoSendMsg(pld);
			
		}
		else
		{
			LOG(WAR_UK, __FUNCTION__, "(USN: ", lDeleteUSN, ", SSN: ", rid.m_lSSN, ", dwGRIID:", dwGRIID, ")");
			//////////////// 사용자가 들어간 방이 없어졌다. 이때는 무조건 LCS에서 위치 정보 지우기.
			//////////////// ABSOLUTE_DELETE_VALUE(0xffffffff) 으로 RoomID::m_dwGRIID 를 설정하면 무조건 지운다.
			//////////////// 방에서 게임 하던 사용자가 무슨 이유에서건 제대로 지워지지 않고 LCS 서버에 남아 있는 경우에 이곳에서 처리한다.
			if(lDeleteUSN > 0)
			{
				RoomID rDeleteID;
				//rDeleteID.m_dwGRIID = ABSOLUTE_DELETE_VALUE;
				rDeleteID.m_lSSN = m_ChannelID.m_lSSN;
				rDeleteID.m_dwCategory = m_ChannelID.m_dwCategory;
				rDeleteID.m_dwGCIID = m_ChannelID.m_dwGCIID;

				if (dwGRIID != ABSOLUTE_DELETE_VALUE)
					rDeleteID.m_dwGRIID = dwGRIID;
				else
					rDeleteID.m_dwGRIID = 0;

				RoomID rid;
				pUser->NLSGetRoomID(rid);
				TKey key(pUser->GetUSN(), pUser->GetCSN());
				theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, rid, pUser->GetUserData().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel);
			}
			///////////////////////////////////////////////

			NSAP nsap;
			PayloadCHSCli pld(PayloadCHSCli::msgGRInviteInfoAns_Tag, 
				MsgCHSCli_GRInviteInfoAns(
				nsap, 
				dwGRIID,
				_X(" "),
				_X(" "),
				ERR_CHANNEL_NOTFOUNDROOM)
				);
			pLink->DoSendMsg(pld);
		}
	}
	else
		LOG(INF_UK, "CHS_CChannelContext"_LK, "!!!!!!!!!!! CChannel::OnSignal_InviteReq : pLink is NULL !!!!!!!!!!!!!");
}

void CChannelContext::UserAccuseReq(MsgCliCHS_AccuseReq * pMsg, CUser & user)
{
	LONG lECode = 0L;
	AccuseBus bus;
	bus.m_sType = pMsg->m_sType;	// 'g' : mean game..
	bus.m_sAccusedID = pMsg->m_sAccusedID;
	bus.m_lAccusedUSN = pMsg->m_lAccusedUSN;
	bus.m_lUSN = user.GetUSN();
	bus.m_sUserID = user.GetUserID();
	bus.m_bRoomTitleAccuse = FALSE;
//	이미 나간 사용자 신고로 인해 변경.
//	CUser * pUser = FindUser(bus.m_sAccusedID);
//	if(pUser)
//	{
//		bus.m_lAccusedUSN = pUser->GetUSN();
//	}

	xstring _temp;
	LONG len = m_ChatHistory.GetHistory(user.GetUSN(), bus.m_lAccusedUSN, _temp);
	if(len < 3) 
	{
		lECode = ERRAGENT_NOCHATDATA;
		PayloadCHSCli pld(PayloadCHSCli::msgAccuseAns_Tag, MsgCHSCli_AccuseAns(bus.m_sAccusedID.c_str(), lECode));
		SendToUser(bus.m_lUSN, pld); // TODO(acepm83@neowiz.com) 현재 CSN기반으로 바꿀 수 없다.
	} 
	else 
	{
		SYSTEMTIME systime;
		::GetLocalTime(&systime);
		bus.m_sDate = ::format("%04d-%02d-%02d %02d:%02d:%02d ", 
					systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

		bus.m_sReason = pMsg->m_sReason;
		bus.m_sContent = _X("신고 이유 : ") + pMsg->m_sContent + _X("\n\n") + _temp;
		MakeContentHeader(bus, pMsg->m_sGameName);

		string sContent1;
		if(pMsg->m_sType == "g2")
			bus.m_sContent1 = m_ChannelInfo.m_sTitle + _X(" : ") + pMsg->m_sContent;
		else 
			bus.m_sContent1 = m_ChannelInfo.m_sTitle;

		time_t _lt;
		::time(&_lt);
		RoomID roomID(m_pChannel->m_ChannelID, 0L);
		lECode = theAccuseAgent.PostAccusationMsg(bus, (LONG)_lt, roomID);
		// client에 응답을 보낸다. 그러나 여기서의 응답은 신고가 DB와 FileServe에 완전히 기록되었음을 보장하지 않는다.
		// 단지, 중복 되지 않은 신고이며, Agent에 신고 process를 수행하라고 메세지를 전달했음만을 보장한다.
	}

//	PayloadCHSCli pld(PayloadCHSCli::msgAccuseAns_Tag, MsgCHSCli_AccuseAns(bus.m_sAccusedID.c_str(), lECode));
//	SendToUser(bus.m_lUSN, pld);
}

// 방제 신고..(2003/01/21)
void CChannelContext::RoomTitleAccuseReq(MsgCliCHS_AccuseReq * pMsg, CUser & user)
{
	LONG lECode = 0L;
	AccuseBus bus;
	bus.m_sType = pMsg->m_sType;					// 'g?' : mean game..방제 신고에 해당하는 값.
	bus.m_sAccusedID = pMsg->m_sAccusedID.c_str();	// 방제 신고일때 이값은 방제를 생성한 사람이다.
	bus.m_lAccusedUSN = pMsg->m_lAccusedUSN;		// 방제신고에서 USN 검색이 곤란하여 이 값을 전달받는다.
	bus.m_lUSN = user.GetUSN();
	bus.m_sUserID = user.GetUserID();
	bus.m_bRoomTitleAccuse = TRUE;
	xstring s_roomTitle = pMsg->m_sRoomTitle.c_str();
	
	{
		SYSTEMTIME systime;
		::GetLocalTime(&systime);
		bus.m_sDate = ::format("%04d-%02d-%02d %02d:%02d:%02d ", 
					systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

		bus.m_sReason = pMsg->m_sReason;
		bus.m_sContent = _X("신고 이유 : ") + pMsg->m_sContent;
		bus.m_sContent1 = str2xstr(pMsg->m_sGameName) + _X(" / ") + m_ChannelInfo.m_sTitle + 
			_X(" : [방제] ") + s_roomTitle + _X(" : ") + pMsg->m_sContent;

		time_t _lt;
		::time(&_lt);
		RoomID roomID(m_pChannel->m_ChannelID, 0L);
		lECode = theAccuseAgent.PostAccusationMsg(bus, (LONG)_lt, roomID, &xstr2str(s_roomTitle));
	}
//	PayloadCHSCli pld(PayloadCHSCli::msgAccuseAns_Tag, MsgCHSCli_AccuseAns(bus.m_sAccusedID.c_str(), lECode));
//	SendToUser(bus.m_lUSN, pld);
}

////////////// 짜고 치기 2005.06.20
void CChannelContext::PrearrangeGameAccuseReq(MsgCliCHS_AccuseReq * pMsg, CUser & user)
{
	LONG lECode = 0L;
	AccuseBus bus;
	bus.m_sType = pMsg->m_sType;					// "a2" : 짜고치기 신고

	bus.m_sAccusedID = pMsg->m_sAccusedID.c_str();	// 방만든 사람

	bus.m_lAccusedUSN = pMsg->m_lAccusedUSN;		
	bus.m_lUSN = user.GetUSN();
	bus.m_sUserID = user.GetUserID();
	bus.m_bRoomTitleAccuse = TRUE;

	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	bus.m_sDate = ::format("%04d-%02d-%02d %02d:%02d:%02d ", 
		systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	bus.m_sReason = pMsg->m_sReason;

	string sMacAddr = "UNKNOWN_MAC";
	string sContent = pMsg->m_sContent;

/*	if (it != m_macTable.end())
	{
		sMacAddr = it->second;

		if (pMsg->m_sContent.size() > 0)
		{
			int nIdx = sContent.find("MACADDRESS");
			if (nIdx > 0)
				sContent = sContent.substr(0, nIdx) + sMacAddr + sContent.substr(nIdx+10, sContent.size() - nIdx - 10 + 2);
		}

		bus.m_sContent = sContent;
		bus.m_sContent1 = sContent;
	}
	else
	{
		bus.m_sContent = pMsg->m_sContent;
		bus.m_sContent1 = pMsg->m_sContent;
	}*/

	bus.m_sContent = pMsg->m_sContent;
	bus.m_sContent1 = pMsg->m_sContent;

	time_t _lt;
	::time(&_lt);
	RoomID roomID(m_pChannel->m_ChannelID, 0L);
	lECode = theAccuseAgent.PostAccusationMsg(bus, (LONG)_lt, roomID);

	//	PayloadCHSCli pld(PayloadCHSCli::msgAccuseAns_Tag, MsgCHSCli_AccuseAns(bus.m_sAccusedID.c_str(), lECode));
	//	SendToUser(bus.m_lUSN, pld);
}

void CChannelContext::MakeContentHeader(AccuseBus & bus, string & game_name)
{
	xstring token(_X(" \r\n"));
	xstring temp_content;	
	temp_content = _X("[위치 정보] \r\n") + str2xstr(game_name) + token + m_ChannelInfo.m_sCategoryName + token + m_ChannelInfo.m_sTitle + 
		token + token + bus.m_sContent + token;
	bus.m_sContent = temp_content;
}

LONG CChannelContext::OnInvitationReq(LONG lCSN, MsgCHSCli_GRInvitationReq& rMsg)
{
	CUser* pUser = m_UsersMap.Find(lCSN);
	if (pUser == NULL)
	{
		return INVITE_ERR_NOUSER;
	}
	else
	{
		PayloadCHSCli pld(PayloadCHSCli::msgGRInvitationReq_Tag, rMsg);
		SendToUser(pUser, pld);
		return INVITE_ERR_NOERROR;
	}
}

void CChannelContext::ResetRcvMsgCnt()
{
	ForEachElmt(CUserMap, m_UsersMap, i, j) 
	{
		CUser * pUser = *i;
		pUser->m_lRcvMsgCnt = 0;
	}
}

void CChannelContext::OnAddRankData(long lCSN, string& strRank)
{
	CUser* pUser = m_UsersMap.Find(lCSN);
	if (pUser)
	{
		// format : USN=1234|RANKING=9999
		UserBaseInfo& BaseInfo = pUser->GetUserData().m_nfUserBaseInfo;
		BaseInfo.m_sReserved2 += strRank;
	}
}