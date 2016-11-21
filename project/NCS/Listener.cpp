// 
// Listener.cpp
// 
#include "stdafx.h"
#include "User.h"
#include "Listener.h"
#include "CharLobbyManager.h"
#include <NLSManager.h>



#define ALIVE_INTERVAL		30000


CListener theListener;


void __stdcall _ListenerCallbackFunc(DWORD dwHandle, SOCKET hSocket, int nErrorCode, LPCSTR lpRemoteAddr, LONG lRemotePort, LPVOID lpContext)
{
	theListener.OnListenerAccept(hSocket, nErrorCode, lpRemoteAddr, lRemotePort);
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor
CListener::CListener() : m_dwListenerHandle(0)
{
}

CListener::~CListener()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
// 두번불릴수 있다.
BOOL CListener::Run(int nPort)
{
	TLock lo(this);

	BOOL bRet = TRUE;

	bRet = bRet && ::XlstnCreate(&m_dwListenerHandle, &_ListenerCallbackFunc, nPort, NULL, NULL);
	VALIDATE(bRet);

	if (!bRet)
		return FALSE;

#ifdef _DEBUG
	bRet = bRet && m_timerAlive.Activate(GetThreadPool(), this, ALIVE_INTERVAL, ALIVE_INTERVAL);
#endif
	bRet = bRet && m_timerJackpot.Activate(GetThreadPool(), this, 10000, 5000);
	if(!bRet)
	{
		Stop();
		return FALSE;
	}

	return TRUE;
}

BOOL CListener::Stop()
{
	TLock lo(this);

	if(m_dwListenerHandle)
	{
		::XlstnDestroy(m_dwListenerHandle);
		m_dwListenerHandle = 0;
	}

	ForEachElmt(TLinkMap, mLinkMap, i, j)
	{
		TLink* pLink = i->second;
		DestroyLink(pLink);
	}

	mLinkMap.clear();

#ifdef _DEBUG
	m_timerAlive.Deactivate();
#endif

	return TRUE;
}

STDMETHODIMP_(void) CListener::OnSignal(HSIGNAL hObj, WPARAM wParam, LPARAM lParam)
{
	if(hObj == 0)
	{
		TLock lo(this);
	}
 	else if (hObj == HSIGNAL_ONACCEPT)
 	{
 		TLock lo(this);
 
 		CLink* pLink = (CLink *) wParam;
 		SOCKET hSocket = (SOCKET) lParam;
 
 		if ((NULL == pLink) || (hSocket == INVALID_SOCKET))
 			return;
 
 		BOOL bRet = pLink->Register(hSocket, NULL, HSIGNAL_XLINKHANDLER);
 		if (!bRet)
 		{
 			::closesocket(hSocket);
 			delete(pLink);
 			return;
 		}
 
 		bRet = AddLink(pLink);
 		if (!bRet)
 		{
 			delete pLink;
 			return;
 		}
	}
	else
	{
		TLock lo(this);
#ifdef _DEBUG
		if(m_timerAlive.IsHandle(hObj)) {
			TLOGAlive();
		}
#endif
		TBase::OnSignal(hObj, wParam, lParam);
	}
}

void CListener::SendMsg(CLink* pSocket, const PayloadNCSCli& pld)
{
	pSocket->DoSendMsg(pld);
}

BOOL CListener::OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR szAddr, LONG lPort)
{
	if ((nErrorCode) || (hSocket == INVALID_SOCKET))
		return FALSE;

	CLink* pLink = new CLink;
	VALIDATE(pLink);
	pLink->SetIP(::inet_addr(szAddr));

	BOOL bRet = ::XsigQueueSignal(GetThreadPool(), this, HSIGNAL_ONACCEPT, (WPARAM)pLink, LPARAM(hSocket));
	VALIDATE(bRet);

	return TRUE;
}

void CListener::DestroyLink(CLink* pLink)
{
	CUser* pUser = pLink->GetUser();
	if(pUser)
	{
		RoomID roomID;
		pUser->NLSGetRoomID(roomID);
		TKey key(pUser->GetGSN(), pUser->GetCSN());
		// 주의!!!!! - 수정해야 할 지도 모름... CSN이 선택되어 있는 상태가 아니기에...
		theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, roomID, pUser->GetLevel());

		pUser->SetLink(NULL);
		pLink->SetUser(NULL);
		delete(pUser);
	}

	RemoveLink(pLink);
	pLink->Unregister();

	delete(pLink);
}

BOOL CListener::OnError(CLink* pSocket, long lEvent, int nErrorCode)
{
	if (pSocket != NULL)
	{
		theLog.Put(DEV, "CListener::OnError(", pSocket->GetHandle(), ")");
		DestroyLink(pSocket);
	}
	return FALSE;
}

BOOL CListener::OnRcvMsg(CLink* pLink, PayloadCliNCS& pld)
{
	switch(pld.mTagID)
	{
	case PayloadCliNCS::msgReqJoinNCS_Tag:
		return OnReqJoinNCS(pLink, pld.un.m_msgReqJoinNCS);
	default:
		{
			theLog.Put(DEV, "CListener::OnRcvMsg - Unknown message(Tag:", pld.mTagID, ")");
		}
		break;
	}
	return OnError(pLink, FD_READ, -999);
}

void CListener::SendToAll(const PayloadNCSCli& pld)
{
	GBuf buf;
	VALIDATE(::LStore(buf, pld));
	ForEachElmt(TLinkMap, mLinkMap, it, jt) 
	{
		TLink *pLink = it->second;
		ASSERT(pLink);
		pLink->DoSend(buf);
	}
}

void CListener::TLOGAlive()
{
#ifdef _DEBUG
	SYSTEMTIME sys;
	memset(&sys, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&sys); 

	//theLog.Put(DEV, "-----------------------------------------------------");
	//string sLogTemp = ::format("CListener is Alive! [%02d/%02d, %02d:%02d ]", sys.wMonth, sys.wDay, sys.wHour, sys.wMinute);
	//theLog.Put(DEV, sLogTemp.c_str());
	//theLog.Put(DEV, "-----------------------------------------------------");
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// Message Handler
BOOL CListener::OnReqJoinNCS(CLink* pLink, MsgCliNCS_ReqJoinNCS* pMsg)
{
	ASSERT(pMsg != NULL);

	CUser* pNewUser = new CUser(pMsg->m_lUSN, pMsg->m_lCSN, pMsg->m_strSiteCode, pMsg->m_strSiteUserID, pMsg->m_strPWD, pMsg->m_lAdminLEV);
	if (!pNewUser) return FALSE;

	pLink->SetUser(pNewUser);
	pNewUser->SetLink(pLink);

	RemoveLink(pLink);

	string strOption = "";
	// NLS로 User 등록 (Char는 선택되어 있지 않은 상태이므로 USN이 Key값이 된다...)
	// NLSCLISTATUS_NFCHARLOBBY => NCS에서는 CHARLOBBYMGR_NCSADDUSERANS 로 검색 하면 된다.
	return theNLSManager.AddUserToNLS(pNewUser, 0, strOption, 0, NLSCLISTATUS_NFCHARLOBBY, &theCharLobbyManager);	
}


