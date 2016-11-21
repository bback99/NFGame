
#include "stdafx.h"
#include <NFVariant/NFGameData.h>
#include <NFVariant/NFDBManager.h>

const LONG g_lRoomCntByPage = 10;

// 1. m_lRoomType : RoomType(PlayType) =  2 or 3
// 2. m_lRoomStatus : Wait or All Room
// 3. m_lPage : Page 
// 4. lRoomCnt : Default(10��)

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

	if (pMsg->m_lTestType == 1 && m_roomlist_Test.size() <= 0)			// �׽�Ʈ ������ ���� �����...
	{
		// Mix �׽�Ʈ
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
		// MAXRoomCnt ����
		if (pMsg->m_lRoomCnt > 0)
			lMaxRoomCNT = pMsg->m_lRoomCnt;	

		////////// 
		// 2011/7/19 RoomList �׽�Ʈ ������ ������ �ϴ� �ڵ�.. 
		if (pMsg->m_lTestType == 1)		
		{
			AUTO_LOCK(&m_gcs);
			ForEachCElmt(NFRoomInfoInChannelList, m_roomlist_Test, it, ij)
			{
				NFRoomInfoInChannel rinfo = *it;

				if(pMsg->m_lRoomType == rinfo.m_roomOption.m_lPlayType)		// PlayType�� 1����...
				{
					// Page * lRoomCntByPage ���� ������ üũ�Ѵ�.
					if (++lCNT > (pMsg->m_lPage-1) * g_lRoomCntByPage)		// Page 2����
					{
						if (1 == pMsg->m_lRoomStatus)						// RoomStatus : wait or all 3����
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

				if(pMsg->m_lRoomType == rinfo.m_roomOption.m_lPlayType)		// PlayType�� 1����...
				{
					// Page * lRoomCntByPage ���� ������ üũ�Ѵ�.
					if (++lCNT > (pMsg->m_lPage-1) * g_lRoomCntByPage)		// Page 2����
					{
						if (1 == pMsg->m_lRoomStatus)						// RoomStatus : wait or all 3����
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

// ���� ���� ��� ��û
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

	// ĳ���Ͱ� �����ϴ��� üũ( USN, CSN ��� )
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

	if( FOT_BLOCK == pMsg->m_lOrderType) // ����
	{
		// �̹� ��������?
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
	else if( FOT_UNBLOCK == pMsg->m_lOrderType ) //���� ����
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

	// �ڽ����� ģ�� �߰� ����.
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
		// ģ������� ������ DB���� �о�´�.
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

	// �̹� ģ�� ���� �˻�
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

	// ģ�� �ִ� ����ο� 100�� üũ
	if( NF_FRIEND_MAX_CNT <= kContNFFriend.size() )
	{
		MsgCHSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = EC_FE_FRIEND_OVERFLOW;
		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	// ĳ���Ͱ� �����ϴ��� üũ( USN, CSN ��� )
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
		// ���� ������ ���ܿ� ��ϵǾ� �ִ���?
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
			// ������ �̹� ���� ģ�� �����ΰ�?( ���� ������ ģ�� ���� �߾���, ���� �ٽ� ��û�ϴ� ��� )
			if( FR_FRIEND == lStatus )
			{
				// ��û�ڸ� ������(���������� ������)�� ģ���� ����Ѵ�. �������� �˸� �ʿ䵵 ����.				
				if( theNFDBMgr.InsertNFFriend( lCSN, key.m_lSubKey, FR_FRIEND ) )
				{
					CNFFriend nfFriend;
					nfFriend.m_strCharName = pMsg->m_strCharName;
					nfFriend.m_bIsOnline = FALSE; // ���� �¶������� �𸣴ϱ�.
					user.AddNFFriend(key, nfFriend);

					{// ģ����û�� ������ �˸���, noti�� ���� �ش�.
						MsgCHSCli_AnsNFFriendAdd ans;
						ans.m_lErrorCode = EC_FE_SUCCESS;
						PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAdd_Tag, ans);
						SendToUser(lCSN, pld);
					}

					// �ڵ� �������� ������ NSL�� ��û
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

	BOOL bIsSuccessBoth = FALSE; // ģ���߰� ���� �Ѵ� �����ߴ���.	
	if( theNFDBMgr.InsertNFFriend( lCSN, key.m_lSubKey, FR_FRIEND_SEND ) )
	{
		if( theNFDBMgr.InsertNFFriend( key.m_lSubKey, lCSN, FR_FRIEND_RECV ) )
		{
			bIsSuccessBoth = TRUE;
		}
		else
		{
			// �ѹ�.
			theNFDBMgr.DeleteNFFriend( user.GetUserNick(), pMsg->m_strCharName );
		}
	}
	else
	{
		// �̹� ģ�� ��û��
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

// ģ����û�� ���� ����� ������ ���� ��
void CChannelContext::OnReqNFFriendAccept(MsgCliCHS_ReqNFFriendAccept* pMsg, CUser& user)
{	
	const LONG lUSN = user.GetUSN();
	const LONG lCSN = user.GetCSN();

	// �������� ģ�����
	CONT_NF_FRIEND kContNFFriend;
	if( FALSE == user.FindNFFriend( kContNFFriend ) )
	{
		// ģ������� ������ DB���� �о�´�.
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

	// ģ�� �ִ� ����ο� 100�� üũ
	if( NF_FRIEND_MAX_CNT <= kContNFFriend.size() )
	{
		MsgCHSCli_AnsNFFriendAccept ans;
		ans.m_lErrorCode = EC_FE_FRIEND_OVERFLOW;

		PayloadCHSCli pld(PayloadCHSCli::msgAnsNFFriendAccept_Tag, ans);
		SendToUser(lCSN, pld);

		return;
	}

	// ģ�� ���·� ������Ʈ
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
		// �������� ģ����Ͽ� ��û���� ������ ���Ѵ�.
		// ��ġ, ������ NLS���������� ������ ���Ŀ� ������Ʈ �Ѵ�.
		CNFFriend nfFriend;
		nfFriend.m_strCharName = pMsg->m_strCharName;
		nfFriend.m_bIsOnline = FALSE; // ���� �¶������� �𸣴ϱ�.
		user.AddNFFriend(key, nfFriend);

		ArcVectorT<TKey> kContKey;
		kContKey.push_back(key);

		// ��û���� ������ NSL�� ��û
		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = lUSN;
		pld.un.m_msgReqLocation->m_lCSN = lCSN;
		pld.un.m_msgReqLocation->m_kContKey = kContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_ACCEPT_FRIEND;
		theNLSManager.GetUserLocation(pld);
	}
}