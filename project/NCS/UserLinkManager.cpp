
#include "stdafx.h"
#include "UserLinkManager.h"
#include "CharLobbyManager.h"


//CUserLinkManager

CUserLinkManager::CUserLinkManager(CUserManager* pUserManager) : m_pUserManager(pUserManager), m_dwRefCnt(0)
{
}

CUserLinkManager::~CUserLinkManager()
{
}

ULONG CUserLinkManager::AddRef()
{
	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
	return dwRefCnt;
}

ULONG CUserLinkManager::Release()
{
	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
	if(dwRefCnt == 0)
	{
	}
	return dwRefCnt;
}

BOOL CUserLinkManager::Start( HTHREADPOOL hPool )
{
	return true;
}

void CUserLinkManager::clear()
{
	//CLink is destroyed only if explicitly required
	mLinkMap.clear();
}

BOOL CUserLinkManager::SendToUser( CLink * pLink , const PayloadNCSCli & pld )
{
	TLock lo(this);
	if( NULL == pLink)
		return false;

	return pLink->DoSendMsg( pld );
}

BOOL CUserLinkManager::SendToAllUser(  const PayloadNCSCli & pld )
{
	TLock lo(this);
	ForEachElmt( TLinkMap, mLinkMap, i, j )
	{
		i->second->DoSendMsg( pld );
	}

	return true;
}

BOOL CUserLinkManager::SendToAllUserExceptOne( CLink * pExcept, const PayloadNCSCli & pld )
{
	TLock lo(this);
	ForEachElmt( TLinkMap, mLinkMap, i, j )
	{
		if( pExcept == i->second )
			continue;
		i->second->DoSendMsg( pld );
	}

	return true;
}

BOOL CUserLinkManager::AddUserLink( CLink * pLink )
{
	TLock lo(this);
	return AddLink( pLink );
}

BOOL CUserLinkManager::RemoveUserLink( CLink * pLink, BOOL bIsLinkOnly )
{
	TLock lo(this);
	if( NULL != pLink )
	{
		if( NULL != pLink->GetHandle() )
		{
			RemoveLink( pLink );

			if (!bIsLinkOnly)
				pLink->Unregister();

			return true;
		}
	}
	return false;
}

BOOL CUserLinkManager::OnError(CLink* pLink, long lEvent, int nErrorCode)
{
	TLock lo(this);
	if( false == RemoveUserLink( pLink ) )
		return FALSE;

	m_pUserManager->PostUserDisconnect(pLink->GetUser());		// NLS·Î 

	return FALSE;
}

BOOL CUserLinkManager::OnRcvMsg(TLink* pLink, PayloadCliNCS& pld)
{
	TLock lo(this);
	CUser* pUser = pLink->GetUser();

	if(NULL == pUser)
	{
		// Remove  apLink and destroy it, because CUser object that matching pLink is not exist
		RemoveUserLink( pLink );
		delete pLink;
		return FALSE;
	}

	m_pUserManager->OnUserMsg(pUser, pld);
	return TRUE;
}

void CUserLinkManager::Stop()
{
}

void CUserLinkManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);
	TBase::OnSignal( hSignal, wParam, lParam );
}