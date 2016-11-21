
#include "stdafx.h"
#include <NFVariant/NFGameData.h>
#include <NFVariant/NFDBManager.h>

const LONG g_lRoomCntByPage = 10;

// 1. m_lRoomType : RoomType(PlayType) =  2 or 3
// 2. m_lRoomStatus : Wait or All Room
// 3. m_lPage : Page 
// 4. lRoomCnt : Default(10개)

void CChannelContext::OnReqRoomList(MsgCliCHS_ReqRoomList* pMsg, CUser & user)
{
	MsgCHSCli_AnsRoomList ans;
	ans.m_lErrorCode = 1;

	long lCNT = 0;
	long lMaxRoomCNT = g_lRoomCntByPage;

// 	if (pMsg->m_lRoomType != 2 && pMsg->m_lRoomType != 3)
// 		ans.m_lErrorCode = -1;
// 	if (pMsg->m_lRoomStatus != 1 && pMsg->m_lRoomStatus != 2)
// 		ans.m_lErrorCode = -2;
// 	if (pMsg->m_lPage <= 0)
// 		ans.m_lErrorCode = -3;
// 	if (pMsg->m_lRoomCnt < 0 && pMsg->m_lRoomCnt > g_lRoomCntByPage)
// 		ans.m_lErrorCode = -4;

	if (pMsg->m_lTestType == 1 && m_roomlist_Test.size() <= 0)			// 테스트 용으로 방을 만든다...
	{
		// Mix 테스트
		// battle single : wait room
		NFRoomInfoInChannel info;
		info.Clear();

		NFUserBaseInfo nfUBI;
		nfUBI.Clear();

		for(int i=0; i<3; i++)
			info.m_lstNFUserBaseInfo.push_back(nfUBI);

		info.m_roomOption.m_lRoomStatus = ROOMSTATE_RUN;
		info.m_roomOption.m_lPlayType = 2;
		info.m_roomOption.m_lMaxUserCnt = 16;

		// battle team : wait room 
		NFRoomInfoInChannel info2;
		info2.Clear();

		NFUserBaseInfo nfUBI2;
		nfUBI2.Clear();

		for(int i=0; i<5; i++)
			info2.m_lstNFUserBaseInfo.push_back(nfUBI2);

		info2.m_roomOption.m_lRoomStatus = ROOMSTATE_RUN;
		info2.m_roomOption.m_lPlayType = 3;
		info2.m_roomOption.m_lMaxUserCnt = 16;

		// battle single : full room
		NFRoomInfoInChannel info3;
		info3.Clear();

		NFUserBaseInfo nfUBI3;
		nfUBI3.Clear();

		for(int i=0; i<16; i++)
			info3.m_lstNFUserBaseInfo.push_back(nfUBI3);

		info3.m_roomOption.m_lRoomStatus = ROOMSTATE_START;
		info3.m_roomOption.m_lPlayType = 2;
		info3.m_roomOption.m_lMaxUserCnt = 16;

		// battle team : full room
		NFRoomInfoInChannel info4;
		info4.Clear();

		NFUserBaseInfo nfUBI4;
		nfUBI4.Clear();

		for(int i=0; i<16; i++)
			info4.m_lstNFUserBaseInfo.push_back(nfUBI4);

		info4.m_roomOption.m_lRoomStatus = ROOMSTATE_START;
		info4.m_roomOption.m_lPlayType = 3;
		info4.m_roomOption.m_lMaxUserCnt = 16;

		AUTO_LOCK(&m_gcs);
		{
			for(int i=0; i<50; i++)
			{
				LONG lRand = rand() % 4;
				switch(lRand)
				{
				case 0:	m_roomlist_Test.push_back(info);		break;
				case 1:	m_roomlist_Test.push_back(info2);	break;
				case 2:	m_roomlist_Test.push_back(info3);	break;
				case 3:	m_roomlist_Test.push_back(info4);	break;
				default: break;
				}
			}
		}
	}

	if (ans.m_lErrorCode == 1)
	{
		// MAXRoomCnt 설정
		if (pMsg->m_lRoomCnt > 0)
			lMaxRoomCNT = pMsg->m_lRoomCnt;	

		////////// 
		// 2011/7/19 RoomList 테스트 끝나면 지워야 하는 코드.. 
		if (pMsg->m_lTestType == 1)		
		{
			AUTO_LOCK(&m_gcs);
			ForEachCElmt(NFRoomInfoInChannelList, m_roomlist_Test, it, ij)
			{
				NFRoomInfoInChannel rinfo = *it;

				if(pMsg->m_lRoomType == rinfo.m_roomOption.m_lPlayType)		// PlayType이 1순위...
				{
					// Page * lRoomCntByPage 개의 갯수를 체크한다.
					if (++lCNT > (pMsg->m_lPage-1) * g_lRoomCntByPage)		// Page 2순위
					{
						if (1 == pMsg->m_lRoomStatus)						// RoomStatus : wait or all 3순위
						{
							// Wait or Not Wait
							if (rinfo.m_roomOption.m_lMaxUserCnt > (LONG)(rinfo.m_lstNFUserBaseInfo.size()))		// if wait...
								ans.m_lstRoomInfo.push_back(rinfo);
						}
						else
							ans.m_lstRoomInfo.push_back(rinfo);
					}
				}

				if ((LONG)ans.m_lstRoomInfo.size() >= lMaxRoomCNT)
					break;
			}
		}
		else
		{
			AUTO_LOCK(&m_gcs);
			ForEachCElmt(NFRoomInfoInChannelList, m_roomlist, it, ij)
			{
				NFRoomInfoInChannel rinfo = *it;

				if(pMsg->m_lRoomType == rinfo.m_roomOption.m_lPlayType)		// PlayType이 1순위...
				{
					// Page * lRoomCntByPage 개의 갯수를 체크한다.
					if (++lCNT > (pMsg->m_lPage-1) * g_lRoomCntByPage)		// Page 2순위
					{
						if (1 == pMsg->m_lRoomStatus)						// RoomStatus : wait or all 3순위
						{
							// Wait or Not Wait
							if (rinfo.m_roomOption.m_lMaxUserCnt > (LONG)(rinfo.m_lstNFUserBaseInfo.size()))		// if wait...
								ans.m_lstRoomInfo.push_back(rinfo);
						}
						else
							ans.m_lstRoomInfo.push_back(rinfo);
					}
				}

				if ((LONG)ans.m_lstRoomInfo.size() >= lMaxRoomCNT)
					break;
			}
		}
	}
	else
		theLog.Put(ERR_UK, "NGS_General"_COMMA, "OnReqRoomList ErrorCode :", ans.m_lErrorCode);
		
	// nf
	PayloadCHSCli	pld(PayloadCHSCli::msgAnsRoomList_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqAchieveInfo(MsgCliCHS_ReqAchvInfo* pMsg, CUser & user)
{
	NFUser& nfUser = user.GetNFUser();

	MsgCHSCli_AnsAchvInfo ans;
	ans.m_nfAchievementPoint = nfUser.m_nfCharInfoExt.m_nfCharAchievement.m_nfCharAP;
	//	ans.m_mapArchievement = nfUser.m_nfCharInfoExt.m_nfCharAchievement.m_nfCharAchieve;

	PayloadCHSCli pld(PayloadCHSCli::msgAnsAchvInfo_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

// 대기실 유저 목록 요청
void CChannelContext::OnReqAwaiterList(MsgCliCHS_ReqAwaiterList* pMsg, CUser& user)
{
	MsgCHSCli_AnsAwaiterList ans;

	ForEachElmt(CUserMap, m_UsersMap, i, j) 
	{
		CUser* pUser = *i;
		if(!pUser)
			continue;

		AwaiterInfo awaiterInfo;
		awaiterInfo.m_lLevel = pUser->GetUserData().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel;
		awaiterInfo.m_strCharName = pUser->GetUserNick();
		ans.m_kContAwaiterList.push_back( awaiterInfo );
	}

	PayloadCHSCli pld(PayloadCHSCli::msgAnsAwaiterList_Tag, ans);
	SendToUser( user.GetCSN(), pld);
}

void CChannelContext::OnReqFollowUser(MsgCliCHS_ReqFollowUser* pMsg, CUser& user)
{
	LONG lUSN = user.GetUSN();
	LONG lCSN = user.GetCSN();

	TKey key;
	if( theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strCharName, key ) )
	{
		ArcVectorT<TKey> kContKey;
		kContKey.push_back( key );

		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = lUSN;
		pld.un.m_msgReqLocation->m_lCSN = lCSN;
		pld.un.m_msgReqLocation->m_kContKey = kContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_FOLLOW_USER;
		pld.un.m_msgReqLocation->m_strCharName = pMsg->m_strCharName;
		theNLSManager.GetUserLocation(pld);
	}
}

void CChannelContext::OnReqNFBlockOrUnBlock(MsgCliCHS_ReqNFBlockOrUnBlock* pMsg, CUser& user)
{
	MsgCHSCli_AnsNFBlockOrUnBlock ans;

	// 캐릭터가 존재하는지 체크( USN, CSN 얻기 )
	TKey key;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strCharName, key ) )
	{
		ans.m_lErrorCode = EC_FE_NOT_EXIST_CHARACTER;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFBlockOrUnBlock_Tag, ans);	
		SendToUser( user.GetCSN(), pld);
		return;
	}

	CONT_NF_FRIEND_NICK kContNFBlock = user.GetNFBlockList();
	if( kContNFBlock.empty() )
	{
		if( FALSE == theNFDBMgr.SelectNFFriendNickByStatus( user.GetCSN(), kContNFBlock, FR_BLOCK ) )	
		{
			ans.m_lErrorCode = EC_FE_DB_ERROR;
			PayloadCHSCli pld(PayloadCHSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
			SendToUser( user.GetCSN(), pld);
			return;
		}

		user.SetNFBlock( kContNFBlock );
	}

	if( FOT_BLOCK == pMsg->m_lOrderType) // 차단
	{
		// 이미 차단인지?
		CONT_NF_FRIEND_NICK::const_iterator block_iter = std::find(kContNFBlock.begin(), kContNFBlock.end(), pMsg->m_strCharName);
		if( block_iter != kContNFBlock.end() )
		{
			ans.m_lErrorCode = EC_FE_ALREADY_BLOCK;
			PayloadCHSCli pld(PayloadCHSCli::msgAnsNFBlockOrUnBlock_Tag, ans);	
			SendToUser( user.GetCSN(), pld);
			return;
		}

		if( FALSE == theNFDBMgr.InsertNFFriend( user.GetCSN(), key.m_lSubKey, FR_BLOCK ) )
		{
			ans.m_lErrorCode = EC_FE_ALREADY_BLOCK;
			PayloadCHSCli pld(PayloadCHSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
			SendToUser( user.GetCSN(), pld);
			return;
		}
		else
		{
			user.AddNFBlock( pMsg->m_strCharName );
		}
	}
	else if( FOT_UNBLOCK == pMsg->m_lOrderType ) //차단 해제
	{
		if( FALSE == theNFDBMgr.DeleteNFFriend( user.GetUserNick(), pMsg->m_strCharName, FR_BLOCK ) )
		{
			ans.m_lErrorCode = EC_FE_DB_ERROR;
			PayloadCHSCli pld(PayloadCHSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
			SendToUser( user.GetCSN(), pld);
			return;
		}
		else
		{
			user.DeleteNFBlock( pMsg->m_strCharName );
		}
	}

	ans.m_lErrorCode = EC_FE_SUCCESS;
	ans.m_lOrderType = pMsg->m_lOrderType;
	PayloadCHSCli pld(PayloadCHSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
	SendToUser( user.GetCSN(), pld);
}

void CChannelContext::OnReqNFFriendAdd(MsgCliCHS_ReqNFFriendAdd* pMsg, CUser& user)
{	
	const LONG lUSN = user.GetUSN();
	const LONG lCSN = user.GetCSN();

	// 자신한테 친구 추가 못함.
	if( pMsg->m_strCharName == user.GetUserNick() )
	{
		MsgCHSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = EC_FE_SELF;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	CONT_NF_FRIEND kContNFFriend;
	if( FALSE == user.FindNFFriend( kContNFFriend ) )
	{
		// 친구목록이 없으면 DB에서 읽어온다.
		if( FALSE == theNFDBMgr.SelectNFFriendInfo( lCSN, kContNFFriend, FR_FRIEND ) )
		{
			MsgCHSCli_AnsNFFriendAdd ans;
			ans.m_lErrorCode = EC_FE_DB_ERROR;
			PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		user.SetNFFriend( kContNFFriend );
	}

	// 이미 친구 인지 검사
	CONT_NF_FRIEND::const_iterator friend_iter = kContNFFriend.begin();
	while( kContNFFriend.end() != friend_iter )
	{
		CONT_NF_FRIEND::mapped_type nfFriend = friend_iter->second;
		if( pMsg->m_strCharName == nfFriend.m_strCharName )
		{
			MsgCHSCli_AnsNFFriendAdd ans;
			ans.m_lErrorCode = EC_FE_ALREADY_FRIEND;
			PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		++friend_iter;
	}

	// 친구 최대 등록인원 100명 체크
	if( NF_FRIEND_MAX_CNT <= kContNFFriend.size() )
	{
		MsgCHSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = EC_FE_FRIEND_OVERFLOW;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	// 캐릭터가 존재하는지 체크( USN, CSN 얻기 )
	TKey key;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strCharName, key ) )
	{
		MsgCHSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = EC_FE_NOT_EXIST_CHARACTER;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);	
		SendToUser(lCSN, pld);
		return;
	}

	LONG lStatus = FR_NONE;
	if( TRUE == theNFDBMgr.SelectNFFriendRelation( key.m_lSubKey, lCSN, FR_BLOCK, lStatus) )
	{
		// 내가 상대방의 차단에 등록되어 있는지?
		if( FR_BLOCK == lStatus )
		{
			MsgCHSCli_AnsNFFriendAdd ans;
			ans.m_lErrorCode = EC_FE_BLOCK;
			PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);	
			SendToUser(lCSN, pld);
			return;
		}

		if( TRUE == theNFDBMgr.SelectNFFriendRelation( key.m_lSubKey, lCSN, FR_FRIEND, lStatus) )
		{
			// 상대방은 이미 내가 친구 상태인가?( 나만 예전에 친구 삭제 했었고, 지금 다시 신청하는 경우 )
			if( FR_FRIEND == lStatus )
			{
				// 신청자만 수락자(수락하지는 않지만)를 친구로 등록한다. 상대방한테 알릴 필요도 없다.				
				if( theNFDBMgr.InsertNFFriend( lCSN, key.m_lSubKey, FR_FRIEND ) )
				{
					CNFFriend nfFriend;
					nfFriend.m_strCharName = pMsg->m_strCharName;
					nfFriend.m_bIsOnline = FALSE; // 아직 온라인인지 모르니까.
					user.AddNFFriend(key, nfFriend);

					{// 친구요청이 됐음을 알리고, noti는 따로 준다.
						MsgCHSCli_AnsNFFriendAdd ans;
						ans.m_lErrorCode = EC_FE_SUCCESS;
						PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
						SendToUser(lCSN, pld);
					}

					// 자동 수락자의 정보를 NSL에 요청
					ArcVectorT<TKey> kContKey;
					kContKey.push_back(key);
					PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
					pld.un.m_msgReqLocation->m_lUSN = lUSN;
					pld.un.m_msgReqLocation->m_lCSN = lCSN;
					pld.un.m_msgReqLocation->m_kContKey = kContKey;
					pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_AUTO_ACCEPT_FRIEND;
					theNLSManager.GetUserLocation(pld);
					return;
				}
			}
		}
	}
	else
	{
		MsgCHSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = EC_FE_DB_ERROR;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);	
		SendToUser(lCSN, pld);
		return;
	}

	BOOL bIsSuccessBoth = FALSE; // 친구추가 쿼리 둘다 성공했는지.	
	if( theNFDBMgr.InsertNFFriend( lCSN, key.m_lSubKey, FR_FRIEND_SEND ) )
	{
		if( theNFDBMgr.InsertNFFriend( key.m_lSubKey, lCSN, FR_FRIEND_RECV ) )
		{
			bIsSuccessBoth = TRUE;
		}
		else
		{
			// 롤백.
			theNFDBMgr.DeleteNFFriend( user.GetUserNick(), pMsg->m_strCharName );
		}
	}
	else
	{
		// 이미 친구 신청함
		MsgCHSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = EC_FE_ALREADY_APPLICATE;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);	
		SendToUser(lCSN, pld);
		return;
	}

	if( FALSE == bIsSuccessBoth)
	{
		MsgCHSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = EC_FE_DB_ERROR;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	CUser* pAcceptor = FindUser( key.m_lSubKey );
	if( pAcceptor )
	{
		MsgCHSCli_NtfNFFriendAdd ntf;
		ntf.m_strCharName = user.GetUserNick();
		PayloadCHSCli pld(PayloadCHSCli::msgNtfNFFriendAdd_Tag, ntf);
		SendToUser(pAcceptor->GetCSN(), pld);
	}
	else
	{
		ArcVectorT<TKey> kContKey;
		kContKey.push_back(key);

		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = lUSN;
		pld.un.m_msgReqLocation->m_lCSN = lCSN;
		pld.un.m_msgReqLocation->m_kContKey = kContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_ADD_FRIEND;
		pld.un.m_msgReqLocation->m_strCharName = pMsg->m_strCharName;
		theNLSManager.GetUserLocation(pld);
	}
}

// 친구요청을 받은 사람이 수락을 했을 때
void CChannelContext::OnReqNFFriendAccept(MsgCliCHS_ReqNFFriendAccept* pMsg, CUser& user)
{	
	const LONG lUSN = user.GetUSN();
	const LONG lCSN = user.GetCSN();

	// 수락자의 친구목록
	CONT_NF_FRIEND kContNFFriend;
	if( FALSE == user.FindNFFriend( kContNFFriend ) )
	{
		// 친구목록이 없으면 DB에서 읽어온다.
		if( FALSE == theNFDBMgr.SelectNFFriendInfo( lCSN, kContNFFriend, FR_FRIEND ) )
		{
			MsgCHSCli_AnsNFFriendAccept ans;
			ans.m_lErrorCode = EC_FE_DB_ERROR;

			PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAccept_Tag, ans);
			SendToUser(lCSN, pld);

			return;
		}

		user.SetNFFriend( kContNFFriend );
	}

	// 친구 최대 등록인원 100명 체크
	if( NF_FRIEND_MAX_CNT <= kContNFFriend.size() )
	{
		MsgCHSCli_AnsNFFriendAccept ans;
		ans.m_lErrorCode = EC_FE_FRIEND_OVERFLOW;

		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAccept_Tag, ans);
		SendToUser(lCSN, pld);

		return;
	}

	// 친구 상태로 업데이트
	if( FALSE == theNFDBMgr.UpdateNFFriendStatusToFriend(user.GetUserNick(), pMsg->m_strCharName) )
	{
		MsgCHSCli_AnsNFFriendAccept ans;
		ans.m_lErrorCode = EC_FE_DB_ERROR;

		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAccept_Tag, ans);	
		SendToUser(lCSN, pld);

		return;
	}

	TKey key;
	if( theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strCharName, key ) )
	{
		// 수락자의 친구목록에 신청자의 정보를 더한다.
		// 위치, 레벨은 NLS에서정보를 가져온 이후에 업데이트 한다.
		CNFFriend nfFriend;
		nfFriend.m_strCharName = pMsg->m_strCharName;
		nfFriend.m_bIsOnline = FALSE; // 아직 온라인인지 모르니까.
		user.AddNFFriend(key, nfFriend);

		ArcVectorT<TKey> kContKey;
		kContKey.push_back(key);

		// 신청자의 정보를 NSL에 요청
		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = lUSN;
		pld.un.m_msgReqLocation->m_lCSN = lCSN;
		pld.un.m_msgReqLocation->m_kContKey = kContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_ACCEPT_FRIEND;
		theNLSManager.GetUserLocation(pld);
	}
}