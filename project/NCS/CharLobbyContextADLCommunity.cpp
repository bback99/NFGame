#include "stdafx.h"
#include "User.h"
#include "CharLobby.h"
#include "CharLobbyContext.h"
#include "CharLobbyManager.h"
#include <NFVariant/NFDBManager.h>
#include <NLSManager.h>

// Community
void CCharLobbyContext::OnReqNFLetterList(CUser* pUser, MsgCliNCS_ReqNFLetterList* pMsg)
{
	MsgNCSCli_AnsNFLetterList	ans;
	ans.Clear();

	if( FALSE == theNFDBMgr.SelectNFLetterList( pUser->GetCSN(), ans.m_kContLetter, pMsg->m_bNewLetter ) )
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
	else
        ans.m_lErrorCode = NF::EC_LE_SUCCESS;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterList_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqNFLetterContent(CUser* pUser, MsgCliNCS_ReqNFLetterContent* pMsg)
{
	MsgNCSCli_AnsNFLetterContent ans;
	ans.Clear();

	if( FALSE == theNFDBMgr.SelectNFLetterContent( pMsg->m_i64LetterIndex, ans.m_strContent, ans.m_strSendTime ) )
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
	else
		ans.m_lErrorCode = NF::EC_LE_SUCCESS;

	ans.m_i64LetterIndex = pMsg->m_i64LetterIndex;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterContent_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqNFLetterReceiverCheck(CUser* pUser, MsgCliNCS_ReqNFLetterReceiverCheck* pMsg)
{
	MsgNCSCli_AnsNFLetterReceiverCheck ans;
	ans.m_lErrorCode = NF::EC_LE_SUCCESS;

	TKey receiverKey;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strReceiver, receiverKey ) )
	{
		ans.m_lErrorCode = NF::EC_LE_NOT_EXIST_RECEIVER;
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterReceiverCheck_Tag, ans);	
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqNFLetterSend(CUser* pUser, MsgCliNCS_ReqNFLetterSend* pMsg)
{
	MsgNCSCli_AnsNFLetterSend ans;

	// 1. �޴� ĳ���Ͱ� �����ϴ°�?
	TKey receiverKey;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strReceiver, receiverKey ) )
	{	
		ans.m_lErrorCode = NF::EC_LE_NOT_EXIST_RECEIVER;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterSend_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	// 2. ���� ���ߴ°�?
	LONG lStatus = FR_NONE;
	if( TRUE == theNFDBMgr.SelectNFFriendRelation( receiverKey.m_lSubKey, pUser->GetCSN(), FR_BLOCK, lStatus ) )
	{
		if( FR_BLOCK == lStatus )
		{
			ans.m_lErrorCode = NF::EC_LE_BLOCK;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterSend_Tag, ans);	
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}
	}
	else
	{
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterSend_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	if( FALSE == theNFDBMgr.InsertNFLetter( pMsg->m_strReceiver, pMsg->m_nfLetter ) )
	{
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterSend_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	ans.m_lErrorCode = NF::EC_LE_SUCCESS;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterSend_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);

	// �� ������ ������ �˷��ش�.
	// ���� �������� NCS�� �ִ� ��� ã�Ƽ� ������.
	CCharLobby* pCharLobby = theCharLobbyManager.FindCharLobby( receiverKey.m_lSubKey );
	if (pCharLobby)
	{
		CUser* pReceiver = pCharLobby->GetUserManager()->FindUser( receiverKey.m_lSubKey );
		if(pReceiver) // NCS������ ���� ���
		{
			MsgNCSCli_NtfNFLetterReceive ntf;
			PayloadNCSCli pld(PayloadNCSCli::msgNtfNFLetterReceive_Tag, ans);
			pCharLobby->GetUserManager()->SendToUser(pReceiver->GetCSN(), pld);
			return;
		}
	}

	// ���� NCS�� ���� ���
	ArcVectorT<TKey> kContKey;
	kContKey.push_back( receiverKey );

	PayloadCLINLS pld_receiver(PayloadCLINLS::msgReqLocation_Tag);
	pld_receiver.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
	pld_receiver.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
	pld_receiver.un.m_msgReqLocation->m_kContKey = kContKey;
	pld_receiver.un.m_msgReqLocation->m_lCause = NLRC_NEW_LETTER;
	theNLSManager.GetUserLocation(pld_receiver);	
}

void CCharLobbyContext::OnReqNFLetterDelete(CUser* pUser, MsgCliNCS_ReqNFLetterDelete* pMsg)
{
	MsgNCSCli_AnsNFLetterDelete ans;
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if( FALSE == theNFDBMgr.DeleteNFLetter( pMsg->m_kContLetterIndex ) )
	{
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFLetterDelete_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqNFFriendInfo(CUser* pUser, MsgCliNCS_ReqNFFriendInfo* pMsg)
{
	const LONG lUSN = pUser->GetUSN();
	const LONG lCSN = pUser->GetCSN();

	CONT_NF_FRIEND kContNFFriend;
	if( FALSE == pUser->FindNFFriend( lCSN, kContNFFriend ) )
	{
		// ģ������� ������ DB���� �о�´�.
		if( FALSE == theNFDBMgr.SelectNFFriendInfo( lCSN, kContNFFriend, FR_FRIEND ) )
		{
			MsgNCSCli_AnsNFFriendInfo ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendInfo_Tag, ans);
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}

		pUser->SetNFFriend( lCSN, kContNFFriend );
	}

	// NLS�� ģ���� ��ġ ��û
	ArcVectorT<TKey> kContKey;
	CONT_NF_FRIEND::iterator iter = kContNFFriend.begin();
	while( iter != kContNFFriend.end() )
	{
		kContKey.push_back( iter->first );
		++iter;
	}

	PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
	pld.un.m_msgReqLocation->m_lUSN = lUSN;
	pld.un.m_msgReqLocation->m_lCSN = lCSN;
	pld.un.m_msgReqLocation->m_kContKey = kContKey;
	pld.un.m_msgReqLocation->m_lCause = NLRC_FRIEND_LIST;
	theNLSManager.GetUserLocation(pld);
}

void CCharLobbyContext::OnReqNFFriendAdd(CUser* pUser, MsgCliNCS_ReqNFFriendAdd* pMsg)
{
	const LONG lUSN = pUser->GetUSN();
	const LONG lCSN = pUser->GetCSN();

	// �ڽ����� ģ�� �߰� ����.
	if( pMsg->m_strCharName == pUser->GetCharName() )
	{
		MsgNCSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = NF::EC_FE_SELF;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	CONT_NF_FRIEND kContNFFriend;
	if( FALSE == pUser->FindNFFriend( lCSN, kContNFFriend ) )
	{
		// ģ������� ������ DB���� �о�´�.
		if( FALSE == theNFDBMgr.SelectNFFriendInfo( lCSN, kContNFFriend, FR_FRIEND ) )
		{
			MsgNCSCli_AnsNFFriendAdd ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}

		pUser->SetNFFriend( lCSN, kContNFFriend );
	}

	// �̹� ģ�� ���� �˻�
	CONT_NF_FRIEND::const_iterator friend_iter = kContNFFriend.begin();
	while( kContNFFriend.end() != friend_iter )
	{
		CONT_NF_FRIEND::mapped_type nfFriend = friend_iter->second;
		if( pMsg->m_strCharName == nfFriend.m_strCharName )
		{
			MsgNCSCli_AnsNFFriendAdd ans;
			ans.m_lErrorCode = NF::EC_FE_ALREADY_FRIEND;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}

		++friend_iter;
	}

	// ģ�� �ִ� ����ο� 100�� üũ
	if( NF_FRIEND_MAX_CNT <= kContNFFriend.size() )
	{
		MsgNCSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = NF::EC_FE_FRIEND_OVERFLOW;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	// ĳ���Ͱ� �����ϴ��� üũ( USN, CSN ��� )
	TKey key;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strCharName, key ) )
	{
		MsgNCSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = NF::EC_FE_NOT_EXIST_CHARACTER;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);	
		return;
	}

	LONG lStatus = FR_NONE;
	if( TRUE == theNFDBMgr.SelectNFFriendRelation( key.m_lSubKey, lCSN, FR_BLOCK, lStatus) )
	{
		// ���� ������ ���ܿ� ��ϵǾ� �ִ���?
		if( FR_BLOCK == lStatus )
		{
			MsgNCSCli_AnsNFFriendAdd ans;
			ans.m_lErrorCode = NF::EC_FE_BLOCK;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);	
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
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
					pUser->AddNFFriend (lCSN, key, nfFriend);

					{// ģ����û�� ������ �˸���, noti�� ���� �ش�.
						MsgNCSCli_AnsNFFriendAdd ans;
						ans.m_lErrorCode = NF::EC_FE_SUCCESS;
						PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);
						m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
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
		MsgNCSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	BOOL bIsSuccessBoth = FALSE; // ģ���߰� ���� �Ѵ� �����ߴ���.	
	if( theNFDBMgr.InsertNFFriend( pUser->GetCSN(), key.m_lSubKey, FR_FRIEND_SEND ) )
	{
		if( theNFDBMgr.InsertNFFriend( key.m_lSubKey, pUser->GetCSN(), FR_FRIEND_RECV ) )
		{
			bIsSuccessBoth = TRUE;
		}
		else
		{
			// �ѹ�.
			theNFDBMgr.DeleteNFFriend( pUser->GetCharName(), pMsg->m_strCharName );
		}
	}
	else
	{
		// �̹� ģ�� ��û��
		MsgNCSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = NF::EC_FE_ALREADY_APPLICATE;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	if( FALSE == bIsSuccessBoth)
	{
		MsgNCSCli_AnsNFFriendAdd ans;
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	// �¶����̸� �ٷ� �˷���� �ϹǷ�, NLS�� ���� ��ġ�� ��û
	ArcVectorT<TKey> kContKey;
	kContKey.push_back(key);

	PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
	pld.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
	pld.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
	pld.un.m_msgReqLocation->m_kContKey = kContKey;
	pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_ADD_FRIEND;
	pld.un.m_msgReqLocation->m_strCharName = pMsg->m_strCharName;
	theNLSManager.GetUserLocation(pld);
}

void CCharLobbyContext::OnReqNFFriendAccept(CUser* pUser, MsgCliNCS_ReqNFFriendAccept* pMsg)
{
	const LONG lUSN = pUser->GetUSN();
	const LONG lCSN = pUser->GetCSN();

	CONT_NF_FRIEND kContNFFriend;
	if( FALSE == pUser->FindNFFriend( lCSN, kContNFFriend ) )
	{
		// ģ������� ������ DB���� �о�´�.
		if( FALSE == theNFDBMgr.SelectNFFriendInfo( lCSN, kContNFFriend, FR_FRIEND ) )
		{
			MsgNCSCli_AnsNFFriendAccept ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAccept_Tag, ans);	
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}

		pUser->SetNFFriend( lCSN, kContNFFriend );
	}

	// �̹� ģ�� ���� �˻�
	CONT_NF_FRIEND::const_iterator friend_iter = kContNFFriend.begin();
	while( kContNFFriend.end() != friend_iter )
	{
		CONT_NF_FRIEND::mapped_type nfFriend = friend_iter->second;
		if( pMsg->m_strCharName == nfFriend.m_strCharName )
		{
			MsgNCSCli_AnsNFFriendAccept ans;
			ans.m_lErrorCode = NF::EC_FE_ALREADY_FRIEND;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAccept_Tag, ans);
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}

		++friend_iter;
	}

	// ģ�� �ִ� ����ο� 100�� üũ
	if( NF_FRIEND_MAX_CNT <= kContNFFriend.size() )
	{
		MsgNCSCli_AnsNFFriendAccept ans;
		ans.m_lErrorCode = NF::EC_FE_FRIEND_OVERFLOW;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAccept_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	// ģ�� ���·� ������Ʈ
	if( FALSE == theNFDBMgr.UpdateNFFriendStatusToFriend(pUser->GetCharName(), pMsg->m_strCharName) )	
	{
		MsgNCSCli_AnsNFFriendAccept ans;
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAccept_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	TKey key;
	if( theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strCharName, key ) ) // ��û���� USN, CSN�� �˾ƿ���.
	{
		// �������� ģ����Ͽ� ��û���� ������ ���Ѵ�.
		// ��ġ, ������ NLS���������� ������ ���Ŀ� ������Ʈ �Ѵ�.
		CNFFriend nfFriend;
		nfFriend.m_strCharName = pMsg->m_strCharName;
		nfFriend.m_bIsOnline = FALSE; // ���� �¶������� �𸣴ϱ�.
		pUser->AddNFFriend(lCSN, key, nfFriend);

		ArcVectorT<TKey> kContKey;
		kContKey.push_back(key);

		// ��û���� ������ NSL�� ��û
		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = lUSN;
		pld.un.m_msgReqLocation->m_lCSN = lCSN;
		pld.un.m_msgReqLocation->m_kContKey = kContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_ACCEPT_FRIEND;
		pld.un.m_msgReqLocation->m_strCharName = pMsg->m_strCharName;
		theNLSManager.GetUserLocation(pld);
	}
}

void CCharLobbyContext::OnReqNFFriendReject(CUser* pUser, MsgCliNCS_ReqNFFriendReject* pMsg)
{
	const LONG lCSN = pUser->GetCSN();

	MsgNCSCli_AnsNFFriendReject ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;

	if( theNFDBMgr.DeleteNFFriend( pUser->GetCharName(), pMsg->m_strCharName, FR_FRIEND_RECV )
		&& theNFDBMgr.DeleteNFFriend( pMsg->m_strCharName, pUser->GetCharName(), FR_FRIEND_SEND ) )
	{	
		pUser->DeleteNFFriendApplicant(lCSN, pMsg->m_strCharName);
	}
	else
	{
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendReject_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqNFFriendDelete(CUser* pUser, MsgCliNCS_ReqNFFriendDelete* pMsg)
{
	MsgNCSCli_AnsNFFriendDelete ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;

	if( FALSE == theNFDBMgr.DeleteNFFriend(pUser->GetCharName(), pMsg->m_strCharName) )
	{
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
	}
	else
	{
		pUser->DeleteNFFriend( pUser->GetCSN(), pMsg->m_strCharName );
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendDelete_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqNFBlockList(CUser* pUser, MsgCliNCS_ReqNFBlockList* pMsg)
{
	const LONG lCSN = pUser->GetCSN();

	CONT_NF_FRIEND_NICK kContNFBlock;
	if( FALSE == pUser->FindNFBlockList( lCSN, kContNFBlock ) )
	{
		if( FALSE == theNFDBMgr.SelectNFFriendNickByStatus( lCSN, kContNFBlock, FR_BLOCK ) )
		{
			MsgNCSCli_AnsNFBlockList ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFBlockList_Tag, ans);
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}

		pUser->SetNFBlock(lCSN, kContNFBlock);
	}

	MsgNCSCli_AnsNFBlockList ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	ans.m_kContBlockList = kContNFBlock;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFBlockList_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqNFFriendApplicantList(CUser* pUser, MsgCliNCS_ReqNFFriendApplicantList* pMsg)
{
	const LONG lCSN = pUser->GetCSN();

	CONT_NF_FRIEND_NICK kContNFFriendApplicant;
	if( FALSE == pUser->FindNFFriendApplicant( lCSN, kContNFFriendApplicant ) )
	{
		if( FALSE == theNFDBMgr.SelectNFFriendNickByStatus( lCSN, kContNFFriendApplicant, FR_FRIEND_RECV ) )
		{
			MsgNCSCli_AnsNFFriendApplicantList ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendApplicantList_Tag, ans);
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}

		pUser->SetNFFriendAplicant(lCSN, kContNFFriendApplicant);
	}

	MsgNCSCli_AnsNFFriendApplicantList ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	ans.m_kContNFFriendApplicantList = kContNFFriendApplicant;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendApplicantList_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqFollowUser(CUser* pUser, MsgCliNCS_ReqFollowUser* pMsg)
{
	TKey key;
	if( theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strCharName, key ) )
	{
		ArcVectorT<TKey> kContKey;
		kContKey.push_back( key );

		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
		pld.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
		pld.un.m_msgReqLocation->m_kContKey = kContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_FOLLOW_USER;
		pld.un.m_msgReqLocation->m_strCharName = pMsg->m_strCharName;
		theNLSManager.GetUserLocation(pld);
	}
}

void CCharLobbyContext::OnReqNFBlockOrUnBlock(CUser* pUser, MsgCliNCS_ReqNFBlockOrUnBlock* pMsg)
{
	const LONG lCSN = pUser->GetCSN();
	MsgNCSCli_AnsNFBlockOrUnBlock ans;
	pMsg->m_strCharName = tolower(pMsg->m_strCharName);

	// ĳ���Ͱ� �����ϴ��� üũ( USN, CSN ��� )
	TKey key;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( pMsg->m_strCharName, key ) )
	{
		ans.m_lErrorCode = NF::EC_FE_NOT_EXIST_CHARACTER;
		PayloadNCSCli pld(PayloadNCSCli::msgAnsNFBlockOrUnBlock_Tag, ans);	
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
		return;
	}

	CONT_NF_FRIEND_NICK kContNFBlock;
	if( FALSE == pUser->FindNFBlockList( lCSN, kContNFBlock ) )
	{
		if( FALSE == theNFDBMgr.SelectNFFriendNickByStatus( lCSN, kContNFBlock, FR_BLOCK ) )
		{
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}
	}

	pUser->SetNFBlock(lCSN, kContNFBlock);

	if( FOT_BLOCK == pMsg->m_lOrderType) // ����
	{
		// �̹� ��������?
		CONT_NF_FRIEND_NICK::const_iterator block_iter = std::find(kContNFBlock.begin(), kContNFBlock.end(), pMsg->m_strCharName);
		if( block_iter != kContNFBlock.end() )
		{
			ans.m_lErrorCode = NF::EC_FE_ALREADY_BLOCK;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFBlockOrUnBlock_Tag, ans);	
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}

		if( FALSE == theNFDBMgr.InsertNFFriend( lCSN, key.m_lSubKey, FR_BLOCK ) )
		{
			ans.m_lErrorCode = NF::EC_FE_ALREADY_BLOCK;
			PayloadNCSCli pld(PayloadNCSCli::msgAnsNFBlockOrUnBlock_Tag, ans);	
			m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
			return;
		}
		else
		{
			pUser->AddNFBlock(pUser->GetCSN(), pMsg->m_strCharName);		
		}
	}
	else if( FOT_UNBLOCK == pMsg->m_lOrderType ) //���� ����
	{
		if( FALSE == theNFDBMgr.DeleteNFFriend( pUser->GetCharName(), pMsg->m_strCharName, FR_BLOCK ) )
		{
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
		}
		else
		{
			pUser->DeleteNFBlock(pUser->GetCSN(), pMsg->m_strCharName);
		}
	}

	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	ans.m_lOrderType = pMsg->m_lOrderType;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

// ģ����� ��û ó��
void CCharLobbyContext::ProcessNFFriendList(MsgNLSCLI_AnsLocation& msg)
{
	CUser* pUser = m_pCharLobby->GetUserManager()->FindUser( msg.m_lCSN );
	if(!pUser)
		return;

	CONT_NF_FRIEND kContNFFriend;
	pUser->FindNFFriend(msg.m_lCSN, kContNFFriend);

	// NLS���� �޾ƿ� �������� ��ġ
	ArcVectorT< NLSBaseInfo >::iterator nls_iter = msg.m_kContNLSBaseInfo.begin();
	while( nls_iter != msg.m_kContNLSBaseInfo.end() )
	{
		// ģ���� ���� ������Ʈ
		CONT_NF_FRIEND::iterator find_iter = kContNFFriend.find( nls_iter->m_Key );
		if( kContNFFriend.end() != find_iter )
		{
			CONT_NF_FRIEND::mapped_type& element = find_iter->second;
			element.m_bIsOnline = TRUE;
			element.m_roomID = nls_iter->m_roomID;
			element.m_lLevel = nls_iter->m_lLevel;
			element.m_lStatus = nls_iter->m_lStatus;
		}

		++nls_iter;
	}

	// Ŭ���̾�Ʈ������ �ٸ� ������ Key���� ��� ������ �ȵǼ�
	// Key�� ���� vector�� �����ش�.
	MsgNCSCli_AnsNFFriendInfo ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	CONT_NF_FRIEND::iterator iter = kContNFFriend.begin();
	while( iter != kContNFFriend.end() )
	{
		ans.m_kContFriendInfo.push_back(iter->second);
		++iter;
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendInfo_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

// ģ����û ó��
void CCharLobbyContext::ProcessNFFriendApplication( const MsgNLSCLI_AnsLocation& msg )
{
	MsgNCSCli_AnsNFFriendAdd ans;	// ��û�ڿ���
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;

	// ��û��
	CUser* pApplicant = m_pCharLobby->GetUserManager()->FindUser( msg.m_lCSN );
	if( !pApplicant )
	{
		// ��û�ڰ� ���� ���� ���µ�..
		return;
	}

	std::string strApplicantName = pApplicant->GetCharName();

	ArcVectorT< NLSBaseInfo >::const_iterator acceptor_iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != acceptor_iter )
	{
		// �������� ��ġ
		switch( acceptor_iter->m_lStatus )
		{
		case NLSCLISTATUS_NFCHARLOBBY://NCS
			{
				// ������
				CUser* pAcceptor = m_pCharLobby->GetUserManager()->FindUser( acceptor_iter->m_Key.m_lSubKey );
				if( !pAcceptor )
				{
					ans.m_lErrorCode = NF::EC_FE_NOT_FOUND_CHARACTER;
					break;
				}

				// �����ڿ��� ģ����û�� ������ �˷��ش�.
				MsgNCSCli_NtfNFFriendAdd ntf;
				ntf.m_strCharName = strApplicantName;
				PayloadNCSCli pld(PayloadNCSCli::msgNtfNFFriendAdd_Tag, ntf);
				m_pCharLobby->GetUserManager()->SendToUser(pAcceptor->GetCSN(), pld);
			}break;
		case NLSCLISTATUS_NFCHANNELSERVER://CHS
			{
				// CHS������ ����
				MsgNCSCHS_NtfNFFriendAdd ntf;
				ntf.m_lReceiverCSN = acceptor_iter->m_Key.m_lSubKey;
				ntf.m_channelID = acceptor_iter->m_roomID;
				ntf.m_strSender = strApplicantName;
				PayloadNCSCHS pld(PayloadNCSCHS::msgNtfNFFriendAdd_Tag, ntf);
				theLRBHandler.SendToCHS(pld, acceptor_iter->m_serverLRBAddr);
			}break;
		case NLSCLISTATUS_NFGAMESERVER://NGS
			{
				// NGS������ ����
				MsgNCSNGS_NtfNFFriendAdd ntf;
				ntf.m_lReceiverCSN = acceptor_iter->m_Key.m_lSubKey;
				ntf.m_roomID = acceptor_iter->m_roomID;
				ntf.m_strSender = strApplicantName;
				PayloadNCSNGS pld(PayloadNCSNGS::msgNtfNFFriendAdd_Tag, ntf);
				theLRBHandler.SendToNGS(pld, acceptor_iter->m_serverLRBAddr);
			}break;
		}
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAdd_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pApplicant->GetCSN(), pld);
}

void CCharLobbyContext::ProcessNFFriendAutoAccept(const MsgNLSCLI_AnsLocation& msg)
{
	// !!��û��
	CUser* pApplicant = m_pCharLobby->GetUserManager()->FindUser( msg.m_lCSN );
	if( !pApplicant )
	{
		return;
	}

	// ��û���� ģ�� ����Ʈ
	CONT_NF_FRIEND kContApplicantFriend;
	pApplicant->FindNFFriend( pApplicant->GetCSN(), kContApplicantFriend );

	// ������ ����
	CNFFriend nfFriendAcceptor;
	nfFriendAcceptor.Clear();
	nfFriendAcceptor.m_strCharName = msg.m_strCharName;

	ArcVectorT< NLSBaseInfo >::const_iterator acceptor_iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != acceptor_iter )
	{
		// ��û���� ģ������ �߿��� �������� ����, ��ġ�� ������Ʈ
		CONT_NF_FRIEND::iterator applicant_friend_iter = kContApplicantFriend.find( acceptor_iter->m_Key );
		if( applicant_friend_iter != kContApplicantFriend.end() )
		{
			CONT_NF_FRIEND::mapped_type& element = applicant_friend_iter->second;
			element.m_lLevel = acceptor_iter->m_lLevel;
			element.m_roomID = acceptor_iter->m_roomID;
			element.m_bIsOnline = TRUE;
			element.m_lStatus = acceptor_iter->m_lStatus;
			nfFriendAcceptor = element;
		}
	}

	// �ڵ� �����Ȱ��� �˸���.
	MsgNCSCli_NtfNFFriendAccept ntf;
	ntf.m_nfFriend = nfFriendAcceptor;
	PayloadNCSCli pld(PayloadNCSCli::msgNtfNFFriendAccept_Tag, ntf);
	m_pCharLobby->GetUserManager()->SendToUser(pApplicant->GetCSN(), pld);
}

// ģ����û ���� �� ó��(�̹� DB�� ģ�����·� ������Ʈ ��)
void CCharLobbyContext::ProcessNFFriendAccept( const MsgNLSCLI_AnsLocation& msg )
{
	// ������
	CUser* pAcceptor = m_pCharLobby->GetUserManager()->FindUser( msg.m_lCSN );
	if( !pAcceptor )
	{
		return;
	}

	// ������ ����
	RoomID roomID;
	pAcceptor->NLSGetRoomID(roomID);
	CNFFriend nfFriendAcceptor;
	nfFriendAcceptor.m_bIsOnline = TRUE;
	nfFriendAcceptor.m_lLevel = pAcceptor->GetLevel();
	nfFriendAcceptor.m_strCharName = pAcceptor->GetCharName();
	nfFriendAcceptor.m_roomID = roomID;
	nfFriendAcceptor.m_lStatus = NLSCLISTATUS_NFCHARLOBBY;

	// �������� ģ�� ����Ʈ
	CONT_NF_FRIEND kContAcceptorFriend;
	pAcceptor->FindNFFriend( pAcceptor->GetCSN(), kContAcceptorFriend );

	// ��û�� ����
	CNFFriend nfFriendApplicant;
	nfFriendApplicant.Clear();
	nfFriendApplicant.m_strCharName = msg.m_strCharName;

	ArcVectorT< NLSBaseInfo >::const_iterator applicant_iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != applicant_iter )
	{
		// �������� ģ������ �߿��� ��û���� ����, ��ġ�� ������Ʈ
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

		// ��û���� ��ġ
		switch( applicant_iter->m_lStatus )
		{
		case NLSCLISTATUS_NFCHARLOBBY://NCS
			{
				// ��û���� ģ����Ͽ� �������� ������ ���ϰ�,
				// ��û�ڿ��� �������� ������ �����ش�.
				CUser* pApplicant = m_pCharLobby->GetUserManager()->FindUser( applicant_iter->m_Key.m_lSubKey );
				if( pApplicant )
				{
					pApplicant->AddNFFriend( applicant_iter->m_Key.m_lSubKey, TKey( msg.m_lUSN, msg.m_lCSN ), nfFriendAcceptor );

					MsgNCSCli_NtfNFFriendAccept ntf;
					ntf.m_nfFriend = nfFriendAcceptor;
					PayloadNCSCli pld(PayloadNCSCli::msgNtfNFFriendAccept_Tag, ntf);
					m_pCharLobby->GetUserManager()->SendToUser(pApplicant->GetCSN(), pld);
				}
			}break;
		case NLSCLISTATUS_NFCHANNELSERVER://CHS
			{
				// CHS�� ����
				MsgNCSCHS_NtfNFFriendAccept ntf;
				ntf.m_lReceiverCSN = applicant_iter->m_Key.m_lSubKey;
				ntf.m_channelID = applicant_iter->m_roomID;
				ntf.m_nfFriend = nfFriendAcceptor;

				PayloadNCSCHS pld(PayloadNCSCHS::msgNtfNFFriendAccept_Tag, ntf);
				theLRBHandler.SendToCHS(pld, applicant_iter->m_serverLRBAddr);
			}break;
		case NLSCLISTATUS_NFGAMESERVER://NGS
			{
				// NGS�� ����
				MsgNCSNGS_NtfNFFriendAccept ntf;
				ntf.m_lReceiverCSN = applicant_iter->m_Key.m_lSubKey;
				ntf.m_roomID = applicant_iter->m_roomID;
				ntf.m_nfFriend = nfFriendAcceptor;
				PayloadNCSNGS pld(PayloadNCSNGS::msgNtfNFFriendAccept_Tag, ntf);
				theLRBHandler.SendToNGS(pld, applicant_iter->m_serverLRBAddr);
			}break;
		default:
			{
			}break;
		}
	}

	// �����ڿ��� ��û���� ������ �����ش�.
	MsgNCSCli_AnsNFFriendAccept ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	ans.m_nfFriend = nfFriendApplicant;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFFriendAccept_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pAcceptor->GetCSN(), pld);
}

void CCharLobbyContext::ProcessNFLetterNew(const MsgNLSCLI_AnsLocation& msg)
{
	CUser* pUser = m_pCharLobby->GetUserManager()->FindUser( msg.m_lCSN );
	if( !pUser )
	{
		return;
	}

	ArcVectorT< NLSBaseInfo >::const_iterator iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != iter )
	{
		switch( iter->m_lStatus )
		{
		case NLSCLISTATUS_NFCHANNELSERVER: // CHS
			{
				MsgNCSCHS_NtfNFLetterReceive ntf;
				ntf.m_channelID = iter->m_roomID;
				ntf.m_lReceiverCSN = iter->m_Key.m_lSubKey;
				PayloadNCSCHS pld(PayloadNCSCHS::msgNtfNFLetterReceive_Tag, ntf);
				theLRBHandler.SendToCHS(pld, iter->m_serverLRBAddr);
			}break;
		case NLSCLISTATUS_NFGAMESERVER: // NGS
			{
				MsgNCSNGS_NtfNFLetterReceive ntf;
				ntf.m_roomID = iter->m_roomID;
				ntf.m_lReceiverCSN = iter->m_Key.m_lSubKey;
				PayloadNCSNGS pld(PayloadNCSNGS::msgNtfNFLetterReceive_Tag, ntf);
				theLRBHandler.SendToNGS(pld, iter->m_serverLRBAddr);
			}break;
		}
	}
}

void CCharLobbyContext::ProcessFollowUser(const MsgNLSCLI_AnsLocation& msg)
{
	CUser* pUser = m_pCharLobby->GetUserManager()->FindUser( msg.m_lCSN );
	if( !pUser )
	{
		return;
	}

	MsgNCSCli_AnsFollowUser ans;
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

	PayloadNCSCli pld(PayloadNCSCli::msgAnsFollowUser_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::ProcessAcceptFriendFromCHS(const MsgCHSNCS_NtfNFFriendAccept& msg)
{
	CUser* pApplicant = m_pCharLobby->GetUserManager()->FindUser(msg.m_lReceiverCSN);
	if(!pApplicant)
		return;

	MsgNCSCli_NtfNFFriendAccept ntf;
	ntf.m_nfFriend = msg.m_nfFriend;
	PayloadNCSCli pld(PayloadNCSCli::msgNtfNFFriendAccept_Tag, ntf);
	m_pCharLobby->GetUserManager()->SendToUser(pApplicant->GetCSN(), pld);
}

void CCharLobbyContext::ProcessNewLetterFromNGS(const MsgNGSNCS_NtfNFLetterReceive& msg)
{
	CUser* pReceiver = m_pCharLobby->GetUserManager()->FindUser(msg.m_lReceiverCSN);
	if( !pReceiver )
		return;

	MsgNCSCli_NtfNFLetterReceive ntf;
	PayloadNCSCli pld(PayloadNCSCli::msgNtfNFLetterReceive_Tag, ntf);
	m_pCharLobby->GetUserManager()->SendToUser(pReceiver->GetCSN(), pld);
}

void CCharLobbyContext::ProcessAcceptFriendFromNGS(const MsgNGSNCS_NtfNFFriendAccept& msg)
{
	CUser* pApplicant = m_pCharLobby->GetUserManager()->FindUser(msg.m_lReceiverCSN);
	if(!pApplicant)
		return;

	MsgNCSCli_NtfNFFriendAccept ntf;
	ntf.m_nfFriend = msg.m_nfFriend;
	PayloadNCSCli pld(PayloadNCSCli::msgNtfNFFriendAccept_Tag, ntf);
	m_pCharLobby->GetUserManager()->SendToUser(pApplicant->GetCSN(), pld);
}

void CCharLobbyContext::ProcessAddFriendFromNGS(const MsgNGSNCS_NtfNFFriendAdd& msg)
{
	CUser* pUser = m_pCharLobby->GetUserManager()->FindUser(msg.m_lReceiverCSN);
	if(!pUser)
		return;

	MsgNCSCli_NtfNFFriendAdd ntf2;
	ntf2.m_strCharName = msg.m_strSender;

	PayloadNCSCli pld(PayloadNCSCli::msgNtfNFFriendAdd_Tag, ntf2);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}