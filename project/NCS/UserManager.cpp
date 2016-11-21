#include "stdafx.h"

#include "User.h"
#include "UserLinkManager.h"
#include "CharLobbyManager.h"

//////////////////////////////////////////////////////////////////////////
CUserManager::CUserManager()
{
	m_pUserLinkManager = new CUserLinkManager(this);
}

CUserManager::~CUserManager(void)
{
	DestroyAllUser();
	if( m_pUserLinkManager )
	{
		delete m_pUserLinkManager;
		m_pUserLinkManager = 0;
	}
	UserTable.clear();
}

BOOL CUserManager::AddUser( CUser * pUser, long lKey )
{
	TLock lo(this);

	if( NULL == pUser )
		return false;

	if( UserTable.end() != UserTable.find( lKey ) )
	{
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "UserManager::Tried to add user has key already in UserManager");
		return false;
	}

	if( pUser->GetLink() )
	{
		if( m_pUserLinkManager->AddUserLink( pUser->GetLink() ) )
			UserTable[lKey] = pUser;

		return true;
	}
	else
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "UserManager::pUser->GetLink() is NULL in UserManager::AddUser");


	return false;
}

BOOL CUserManager::DestroyUser( long lKey )
{
	TLock lo(this);
	CUser * pUser = FindUser( lKey );

	return DestroyUser( pUser, lKey );
}

BOOL CUserManager::DestroyUser(CUser * pUser, long lKey)
{
	TLock lo(this);
	if( pUser )
	{
		if( TRUE == RemoveUser( pUser, lKey ) )
		{
			RemoveUserLink( pUser );			
			DeleteUserAndLink( pUser );
		}
	}

	return false;
}

BOOL CUserManager::DestroyAllUser()
{
	TLock lo(this);
	ForEachElmt(TMapUsermap, UserTable, i, j) 
	{
		//DestroyUser( (*i).second,  );
	}

	UserTable.clear();

	return true;
}

void CUserManager::DeleteUserAndLink( CUser * pUser )
{
	TLock lo(this);
	if( NULL != pUser )
	{
		CLink * pLink = pUser->GetLink();
		pUser->SetLink( NULL );
		if( NULL != pLink )
		{
			pLink->SetUser( NULL );
			delete pLink;
		}
		delete pUser;
	}
}

BOOL CUserManager::RemoveUser( CUser * pUser, long lKey )
{
	TLock lo(this);
	if( pUser )
	{
		if( 0 < ( UserTable.erase( lKey ) ) )
			return true;
	}
	return false;
}

BOOL CUserManager::RemoveLink(CUser* pUser)
{
	TLock lo(this);
	if( pUser )
	{
		m_pUserLinkManager->RemoveUserLink(pUser->GetLink(), TRUE);
	}
	return false;
}

void CUserManager::RemoveUserLink( CUser * pUser  )
{
	TLock lo(this);
	if( NULL != pUser )
	{
		CLink * pLink = pUser->GetLink();
		if( pLink)
		{
			m_pUserLinkManager->RemoveUserLink( pLink );
		}
	}
}

BOOL CUserManager::SendToUser( long lKey, const PayloadNCSCli & pld )
{
	TLock lo(this);

	UserItr	it = UserTable.find(lKey);

	if( UserTable.end() == it )
		return false;

	return SendToUser( (CUser *)(*it).second, pld );

}

void CUserManager::SendToAllUser(const PayloadNCSCli& pld)
{
	TLock lo(this);
	m_pUserLinkManager->SendToAllUser( pld );
}

void CUserManager::SendToAllUsersExceptOne( long lExceptKey, const PayloadNCSCli & pld )
{
	TLock lo(this);

	CUser * pUser = FindUser( lExceptKey );
	if( NULL == pUser )
		m_pUserLinkManager->SendToAllUser( pld );
	else
		m_pUserLinkManager->SendToAllUserExceptOne( pUser->GetLink(), pld );
}

BOOL CUserManager::SendToUser( CUser * pUser, const PayloadNCSCli & pld )
{
	if( NULL == pUser )
		return false;

	CLink * pLink = pUser->GetLink();
	if( pLink)
		return 	m_pUserLinkManager->SendToUser( pUser->GetLink(), pld );

	return false;
}

CUser * CUserManager::FindUser( long lKey ) const
{
	TLock lo(this);
	UserConstItr it = UserTable.find( lKey );
	if( UserTable.end() == it )
		return NULL;
	return (*it).second;
}

size_t CUserManager::ProcessAliveCheckTimer( std::list<long> & AliveCheckTimeOutKey, std::list<long> & needCheckKey )
{
	TLock lo(this);
	size_t userCnt = 0;
// 	ForEachElmt(hashUserMap, UserTable, i, j) 
// 	{
// 		CUser* pUser = (*i).second;
// 		if (NULL == pUser) continue;
// 		userCnt++;
// 		USER_STATE_INDEX state = pUser->GetState();
// 		if (state == US_CHECK) {
// 			//TLOG2("CRoomInternalLogic::OnCheckUserAliveTimer(Key:%d, UID:%s) - Disconnect\n", pUser->GetGSN(), pUser->GetUserID().c_str());
// 
// #ifdef _NGSNLS
// 			theLog.Put(DEV, "CUserManager::OnCheckUserAliveTimer(Key: ", pUser->GetCSN(), ", UID: ", pUser->GetUserID().c_str(), " - Disconnect");
// 			AliveCheckTimeOutKey.push_back(pUser->GetCSN());
// #else
// 			theLog.Put(DEV, "CUserManager::OnCheckUserAliveTimer(CSN: ", pUser->GetGSN(), ", UID: ", pUser->GetUserID().c_str(), " - Disconnect");
// 			AliveCheckTimeOutKey.push_back(pUser->GetGSN());
// #endif
// 		} 
// 		else if (state == US_NEEDCHECK) 
// 		{
// #ifdef _NGSNLS
// 			needCheckKey.push_back( pUser->GetCSN() );
// #else
// 			needCheckKey.push_back( pUser->GetGSN() );
// #endif
// 			pUser->SetState(US_CHECK);
// 		} 
// 		else 
// 		{
// 			pUser->SetState(US_NEEDCHECK);
// 		}
// 	}
	return userCnt;
}

void CUserManager::ResetMsgRcvCnt()
{
	TLock lo(this);
	ForEachElmt(TMapUsermap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		pUser->m_lRcvMsgCnt = 0;
	}
}

DWORD CUserManager::GetUserLinkIndex( long lKey )
{
	TLock lo(this);

	CUser * pUser = FindUser( lKey );
	if( pUser )
	{
		CLink * pLink = pUser->GetLink();
		if( pLink )
			return pLink->GetIndex();
	}

	return CLink::INVALID_INDEX;
}

// 1. client가 이상한 행동을 하면서 OnError가 호출 되는 경우
// - Post로 던져서 타이밍을 늦춰야 Library단의 Usermap이 문제가 안 생긴다...
// - 서버가 지우면서, 클라의 OnError에 의해 체크 되어 Assert 뜨는 현상 
// - DestroyUser(pUser, lKey) 로 해결 안 됨
// 2. 중복접복으로 판단되어 기존 유저 밀어내는 경우
BOOL CUserManager::KickOutUser( long lKey )
{
	TLock lo(this);

	CUser * pUser = FindUser( lKey );
	if( NULL != pUser )
	{
		PostUserDisconnect(pUser);
		return TRUE;
	}
	else
		return FALSE;
}

void CUserManager::Clear()
{
	TLock lo(this);
	UserTable.clear();
}

BOOL CUserManager::Start( HTHREADPOOL hPool )
{
	return m_pUserLinkManager->Start( hPool );
}

BOOL CUserManager::Stop()
{
	m_pUserLinkManager->Stop();
	DestroyAllUser();
	return true;
}