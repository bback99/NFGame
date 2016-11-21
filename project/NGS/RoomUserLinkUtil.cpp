#include "stdafx.h"
#include "RoomEventQueue.h"
#include "RoomUserLinkUtil.h"




//CUserLinkManager

CUserLinkManager::CUserLinkManager( CRoomEventQueue * pQueue )
:
EventQueue( pQueue )
,m_dwRefCnt(0)
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

BOOL CUserLinkManager::SendToUser( CLink * pLink , const PayloadNGSCli & pld )
{
	TLock lo(this);
	if( NULL == pLink)
		return false;

	GBuf tobeSent;
	if( false == theEncryptMgr.CheckAndEncrypt( tobeSent,  (pld) ) )
		return false;;

	return pLink->DoSend( tobeSent );
}
BOOL CUserLinkManager::SendToAllUser(  const PayloadNGSCli & pld )
{
	TLock lo(this);
	GBuf tobeSent;
	if( false == theEncryptMgr.CheckAndEncrypt( tobeSent, pld ) )
		return false;;

	ForEachElmt( TLinkMap, mLinkMap, i, j )
	{
		i->second->DoSend( tobeSent );
	}

	return true;
}

BOOL CUserLinkManager::SendToAllUserExceptOne( CLink * pExcept, const PayloadNGSCli & pld )
{
	TLock lo(this);
	GBuf toSent;
	if( false == theEncryptMgr.CheckAndEncrypt( toSent, pld ) )
		return false;;

	ForEachElmt( TLinkMap, mLinkMap, i, j )
	{
		if( pExcept == i->second )
			continue;
		i->second->DoSend( toSent );
	}

	return true;
}

BOOL CUserLinkManager::AddUserLink( CLink * pLink )
{
	TLock lo(this);
	return AddLink( pLink );
}

BOOL CUserLinkManager::RemoveUserLink( CLink * pLink )
{
	TLock lo(this);
	if( NULL != pLink )
	{
		if( NULL != pLink->GetHandle() )
		{
			RemoveLink( pLink );
			pLink->Unregister();
			PostLinkCutEvt( pLink );

			return true;
		}
	}

	return false;
}


BOOL CUserLinkManager::OnRcvMsg(TLink* pLink, PayloadCliNGS& pld)
{
	TLock lo(this);
	CUser* pUser = pLink->GetUser();

	//	ASSERT(pUser);
	if(!pUser)
	{
		// Remove  apLink and destroy it, because CUser object that matching pLink is not exist
		RemoveUserLink( pLink );
		delete pLink;
		return FALSE;
	}

	if( false == theEncryptMgr.CheckAndDecrypt(pld) )
		return FALSE;

	pUser->PushMsg(pld);

// NF
	RoomEvent e(REV_USERMSG, pUser->GetCSN());
// NF

	EventQueue->PushQueue( e );

	return TRUE;
}

BOOL CUserLinkManager::PostLinkCutEvt( CLink * pLink )
{
	if (!pLink) return FALSE;
	CUser* pUser = pLink->GetUser();

	if(pUser)
	{
// NF
		RoomEvent e(REV_USERLINKERROR, pUser->GetCSN() );
// NF
		EventQueue->PushQueue(e);
		return TRUE;
	}

	return FALSE;
}

BOOL CUserLinkManager::OnError(CLink* pLink, long lEvent, int nErrorCode)
{
	TLock lo(this);
//	if (!pLink) return FALSE;
//	CUser* pUser = pLink->GetUser();

	if( false == RemoveUserLink( pLink ) )
		return FALSE;

	PostLinkCutEvt( pLink );

//	if(pUser)
//	{
//		RoomEvent e(REV_USERLINKERROR, pUser->GetUSN() );
//		EventQueue->PushQueue(e);
//		theLog.Put(DEV_UK, "NGS_DevInfo"_COMMA, " CUserLinkManager::OnError() and pUser is Not NULL");
//		return TRUE;
//	}

	return FALSE;
}


void CUserLinkManager::Stop()
{
	
}

void CUserLinkManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);
	TBase::OnSignal( hSignal, wParam, lParam );
}