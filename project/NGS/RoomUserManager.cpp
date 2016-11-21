#include "stdafx.h"
#include ".\roomusermanager.h"

#include "RoomUserLinkUtil.h"
#include "RoomEventQueue.h"

//// ACHV BEGIN
#include <ACHV/AchvDef.h>
static achv::CAchvMgr& g_achv = achv::CAchvMgr::Instance();
//// ACHV END


CRoomUserManager::CRoomUserManager( IRoomEventHandler * pHandler )
:
EventQueue(NULL),
UserLinkManager(NULL)
{
	EventQueue = new CRoomEventQueue( pHandler );
	if( EventQueue )
		EventQueue->Activate( GetThreadPool() );
	UserLinkManager = new CUserLinkManager( EventQueue );

	UserTable.clear();
}

CRoomUserManager::~CRoomUserManager(void)
{
	DestroyAllUser();
	if( UserLinkManager )
	{
		delete UserLinkManager;
		UserLinkManager = 0;
	}
	if( EventQueue )
	{
		EventQueue->DeactivateQueue();

		delete EventQueue;
		EventQueue= NULL;
	}
	UserTable.clear();
}

BOOL CRoomUserManager::AddUser( CUser * pUser )
{
	TLock lo(this);

	if( NULL == pUser )
		return false;

#ifdef _NGSNLS		// NF 일 경우, CSN 기반이므로 CSN을 Key값으로 넣는다...
	if( UserTable.end() != UserTable.find( pUser->GetCSN() ) )
#else
	if( UserTable.end() != UserTable.find( pUser->GetUSN() ) )
#endif
	{
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "UserManager::Tried to add user has usn aleady in UserManager");
		return false;
	}

	if( pUser->GetLink() )
	{
		if( UserLinkManager->AddUserLink( pUser->GetLink() ) )
#ifdef _NGSNLS		
			UserTable[pUser->GetCSN()] = pUser;
#else
			UserTable[pUser->GetUSN()] = pUser;
#endif

		return true;
	}
	else
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "UserManager::pUser->GetLink() is NULL in UserManager::AddUser");
		
	
	return false;
}

BOOL CRoomUserManager::DestroyUser( long lCSN )
{
	TLock lo(this);
	CUser * pUser = FindUser( lCSN );

	return DestroyUser( pUser );
}

BOOL CRoomUserManager::DestroyUser( CUser * pUser )
{
//// ACHV BEGIN
	if (pUser)
		g_achv.logout(pUser->GetCSN());
//// ACHV END

	TLock lo(this);
	if( pUser )
	{
		if( TRUE == RemoveUser( pUser ) )
		{
			RemoveUserLink( pUser );
			DeleteUserAndLink( pUser );
		}
	}

	return false;
}



BOOL CRoomUserManager::DestroyAllUser()
{
	TLock lo(this);
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		DestroyUser( (*i).second );
	}

	UserTable.clear();

	return true;
}


void CRoomUserManager::DeleteUserAndLink( CUser * pUser )
{

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

BOOL CRoomUserManager::RemoveUser( CUser * pUser )
{
	if( pUser )
	{
#ifdef _NGSNLS
		if( 0 < ( UserTable.erase( pUser->GetCSN() ) ) )
#else
		if( 0 < ( UserTable.erase( pUser->GetUSN() ) ) )
#endif
			return true;
	}
	return false;
}

void CRoomUserManager::RemoveUserLink( CUser * pUser )
{
	if( NULL != pUser )
	{
		CLink * pLink = pUser->GetLink();
		if( pLink)
		{
			UserLinkManager->RemoveUserLink( pLink );
		}
	}
}




BOOL CRoomUserManager::SendToUser( long lCSN, const PayloadNGSCli & pld )
{
	TLock lo(this);

	UserItr	it = UserTable.find(lCSN);

	if( UserTable.end() == it )
		return false;
	
	return SendToUser( (CUser *)(*it).second, pld );
	
}

void CRoomUserManager::SendToAllUsersExceptOne( long lExceptUSN, const PayloadNGSCli & pld )
{
	TLock lo(this);

	CUser * pUser = FindUser( lExceptUSN );
	if( NULL == pUser )
		UserLinkManager->SendToAllUser( pld );
	else
		UserLinkManager->SendToAllUserExceptOne( pUser->GetLink(), pld );

}

void CRoomUserManager::SendToAllUser( const PayloadNGSCli & pld )
{
	TLock lo(this);

	UserLinkManager->SendToAllUser( pld );
}


BOOL CRoomUserManager::SendToUser( CUser * pUser, const PayloadNGSCli & pld )
{
	if( NULL == pUser )
		return false;

	CLink * pLink = pUser->GetLink();

	if( pLink)
	{
		return 	UserLinkManager->SendToUser( pUser->GetLink(), pld );
	}

	return false;
}

void CRoomUserManager::AddRoomEvent( const RoomEvent & evt )
{
	TLock lo(this);
	EventQueue->PushQueue( evt );
}



void CRoomUserManager::GetAllUserBaseInfo( UserBaseInfoList &userbaseInfo )
{
	TLock lo(this);

	GBuf buf;
	UserBaseInfo info2;
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if( pUser )
		{
			buf.Clear();
			info2.Clear();
			pUser->GetUserData().BStore(buf);
			info2.BLoad(buf);
			userbaseInfo.push_back( info2 );
		}
	}
}

void CRoomUserManager::GetAllUserBaseInfo( TListNFCharInfo &userbaseInfo )
{
	TLock lo(this);

	GBuf buf;
	NFCharInfo info2;
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if( pUser )
		{
			buf.Clear();
			info2.Clear();
			pUser->GetNFCharInfoExt()->BStore(buf);
			info2.BLoad(buf);
			userbaseInfo.push_back( info2 );
		}
	}
}

CUser * CRoomUserManager::FindUser( long lCSN ) const
{
	TLock lo(this);
	UserConstItr it = UserTable.find( lCSN );
	if( UserTable.end() == it )
		return NULL;
	return (*it).second;
}

size_t CRoomUserManager::ProcessAliveCheckTimer( std::list<long> & AliveCheckTimeOutUSN, std::list<long> & needCheckUSN )
{
	TLock lo(this);
	size_t userCnt = 0;
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if (!pUser) continue;
		userCnt++;
		USER_STATE_INDEX state = pUser->GetState();
		if (state == US_CHECK) {
			//TLOG2("CRoomInternalLogic::OnCheckUserAliveTimer(USN:%d, UID:%s) - Disconnect\n", pUser->GetUSN(), pUser->GetUserID().c_str());
			
#ifdef _NGSNLS
			theLog.Put(DEV, "CRoomUserManager::OnCheckUserAliveTimer(USN: ", pUser->GetUSN(), ", UID: ", pUser->GetUserID().c_str(), " - Disconnect");
			AliveCheckTimeOutUSN.push_back(pUser->GetUSN());
#else
			theLog.Put(DEV, "CRoomUserManager::OnCheckUserAliveTimer(CSN: ", pUser->GetCSN(), ", UID: ", pUser->GetUserID().c_str(), " - Disconnect");
			AliveCheckTimeOutUSN.push_back(pUser->GetCSN());
#endif
		} 
		else if (state == US_NEEDCHECK) 
		{
#ifdef _NGSNLS
			needCheckUSN.push_back( pUser->GetCSN() );
#else
			needCheckUSN.push_back( pUser->GetUSN() );
#endif
			pUser->SetState(US_CHECK);
		} 
		else 
		{
			pUser->SetState(US_NEEDCHECK);
		}
	}
	return userCnt;
}

void CRoomUserManager::ResetMsgRcvCnt()
{
	TLock lo(this);
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		pUser->m_lRcvMsgCnt = 0;
	}
}


DWORD CRoomUserManager::GetUserLinkIndex( long lCSN )
{
	TLock lo(this);

	CUser * pUser = FindUser( lCSN );
	if( pUser )
	{
		CLink * pLink = pUser->GetLink();
		if( pLink )
			return pLink->GetIndex();
	}

	return CLink::INVALID_INDEX;
}

BOOL CRoomUserManager::CutUser( long lCSN )
{
	TLock lo(this);

	CUser * pUser = FindUser( lCSN );
	if( pUser )
	{
		UserLinkManager->RemoveUserLink( pUser->GetLink() );

		RoomEvent e( REV_USERCUT, lCSN );
		EventQueue->PushQueue( e );

		return TRUE;
	}
	else
		return FALSE;
}

void CRoomUserManager::clear()
{
	TLock lo(this);

	UserTable.clear();
	EventQueue->ClearEvent();
	UserLinkManager->clear();
}

BOOL CRoomUserManager::Start( HTHREADPOOL hPool )
{
	return UserLinkManager->Start( hPool );
}

BOOL CRoomUserManager::Stop()
{
	EventQueue->ClearEvent();
	UserLinkManager->Stop();
	DestroyAllUser();
	return true;
}


// NF
void CRoomUserManager::GetAllUserBaseInfo( NFUserBaseInfoList &nfUBI )
{
	TLock lo(this);

	GBuf buf;
	NFUserBaseInfo info2;
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if( pUser )
		{
			buf.Clear();
			info2.Clear();
			pUser->GetUserData().BStore(buf);
			info2.BLoad(buf);
			nfUBI.push_back( info2 );
		}
	}
}

void CRoomUserManager::GetAllUserBaseInfo(NFJoinUserBaseInfoList& joinUserBaseInfoList)
{
	TLock lo(this);

	NFJoinUserBaseInfo info2;
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if( pUser )
		{
			info2.Clear();

			info2.m_nfUserBaseInfo = pUser->GetNFUser().GetNFChar().m_nfUserBaseInfo;
			info2.m_nfCharBaseInfo = pUser->GetNFCharInfoExt()->m_nfCharBaseInfo;
			info2.m_lUserSlot = pUser->GetNFUser().GetUserSlot();
			info2.m_lUserStatus = pUser->GetNFUser().GetUserStatus();

			joinUserBaseInfoList.push_back( info2 );
		}
	}
}

BOOL CRoomUserManager::IsCheckMapLoading(BOOL bIsDis)
{
	TLock lo(this);

	BOOL bRet = FALSE;
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if( pUser )
		{
			CNFChar& nfChar = pUser->GetNFUser();
			MapLoadingProgress up = nfChar.GetMapLodingProgress();

			if (up.m_lProgress != 100)
			{
				if (bIsDis == TRUE) 
				{
					CutUser(pUser->GetCSN());
				}
				else
                	return bRet;
			}
		}
		else
			continue;
	}
	return TRUE;
}

BOOL CRoomUserManager::InitTeamPlayData()
{
	TLock lo(this);

	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if( pUser )
		{
			pUser->InitTeamPlayDataEachUser();
		}
		else
			continue;
	}
	return TRUE;
}

BOOL CRoomUserManager::GetSingleGameResult(TLstGameResult& lstPlayer, BOOL bIsSimple)
{
	TLock lo(this);

	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if ( pUser )
		{
			CNFChar& nfChar = pUser->GetNFUser();
			GameResult gameResult(pUser->GetSaveLandingFish(), 0, nfChar.GetLevel(), nfChar.GetGSN(), nfChar.GetCSN(), nfChar.GetExp());

			// 개인전 일 경우에만, 한마리도 못 잡으면 시상식대에 올리지 않아야 하기에...
			// 여기서 미리 잡아놓은 리스트를 보고, -1로 설정
			if (pUser->GetSaveLandingFish().m_lstLandingFish.size())
				gameResult.m_lRank = UserTable.size();
			else
				gameResult.m_lRank = -1;
			gameResult.m_lGrade = nfChar.GetGrade();
			gameResult.m_strName = nfChar.GetCharName();

			// 전체 보여주는 경우가 아니면 보여줄 필요가 없으므로 0으로 초기화!!! - 굳이 할 필요가 있을까?
			if (bIsSimple) {
				gameResult.m_lTotExp = 0;
				gameResult.m_lBonusExp = 0;
				gameResult.m_llTotMoney = 0;
			}
			lstPlayer.push_back(gameResult);
		}
		else
			return FALSE;
	}
	return TRUE;
}

BOOL CRoomUserManager::GetTeamGameResult(TLstGameResult& lstATeamPlayer, TLstGameResult& lstBTeamPlayer, BOOL bIsSimple)
{
	TLock lo(this);

	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if ( pUser )
		{
			CNFChar& nfChar = pUser->GetNFUser();
			GameResult gameResult(pUser->GetSaveLandingFish(), 0, nfChar.GetLevel(), nfChar.GetGSN(), nfChar.GetCSN(), nfChar.GetExp());
			gameResult.m_lGrade = nfChar.GetGrade();
			gameResult.m_strName = nfChar.GetCharName();
			LONG lMod = pUser->GetUserSlot() % 2;
			gameResult.m_lBattleType = lMod+1;

			// 전체 보여주는 경우가 아니면 보여줄 필요가 없으므로 0으로 초기화!!! - 굳이 할 필요가 있을까?
			if (bIsSimple) {
				gameResult.m_lTotExp = 0;
				gameResult.m_lBonusExp = 0;
				gameResult.m_llTotMoney = 0;
			}

			if (lMod == 0)
				lstATeamPlayer.push_back(gameResult);
			else
				lstBTeamPlayer.push_back(gameResult);
		}
		else
			return FALSE;
	}
	return TRUE;
}

BOOL CRoomUserManager::CheckUserGameStartStatus()
{
	TLock lo(this);

	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if ( pUser )
		{
			if (pUser->GetUserLocation() != ULS_ROOMLOBBY || pUser->GetUserStatus() != UIS_READY)
				return FALSE;
		}
		else
			continue;
	}
	return TRUE;
}

BOOL CRoomUserManager::SetUserGameStartStatus(USER_LOCATION_STATE lStatus, LONG lMapIndex)
{
	TLock lo(this);

	//	유저의 정보를 변경한다...	(유저 상태 체크 할때 같이 할 수 없는 이유는 -10 에러가 발생했을때, 롤백을 해줘야 하므로...)
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser* pUser = (*i).second;
		if ( pUser )
		{
			pUser->SetUserLocation(lStatus);

			NFRoomOption& roomOption = pUser->GetNFRoomOption();
			roomOption.m_lIdxFishMap = lMapIndex;
		}
		else
			continue;
	}
	return TRUE;
}

CUser* CRoomUserManager::GetNextCap(LONG lCSN)
{
	ForEachElmt(hashUserMap, UserTable, i, j) 
	{
		CUser * pUser = (*i).second;
		if (pUser)
		{
			if (lCSN == pUser->GetCSN())		// 혹시 선택된 놈이 방장 일 경우, pass
				continue;
			return pUser;
		}
		else
			continue;
	}
	return NULL;
}
// NF