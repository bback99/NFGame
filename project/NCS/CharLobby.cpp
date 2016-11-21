
#include "stdafx.h"
#include "User.h"
#include "CharLobby.h"
#include <NFVariant/NFItem.h>
#include <NFVariant/NFDBManager.h>

////// ACHV BEGIN
#include <ACHV/AchvDef.h>
static achv::CAchvMgr& g_achv = achv::CAchvMgr::Instance();
////// ACHV END

CCharLobby::CCharLobby(DWORD dwCharLobbyID)
{
	m_dwCharLobbyID	= dwCharLobbyID;
	m_pCharLobbyCxt	= new CCharLobbyContext(this);
	m_pUserManager	= new CCharLobbyUserManager(this);

	m_timerBanishUser.Activate(GetThreadPool(), this, 2000, 2000);
}

CCharLobby::~CCharLobby()
{

}

STDMETHODIMP_(ULONG) CCharLobby::AddRef() 
{
	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
	return dwRefCnt;
}

STDMETHODIMP_(ULONG) CCharLobby::Release() 
{
	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
	if(dwRefCnt == 0)
	{
	}
	return dwRefCnt;
}

// lock 필수
void CCharLobby::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if (m_timerBanishUser.IsHandle(hSignal))
	{
		OnKickOutBanishUser();
	}
	else
	{
		switch((LONG)hSignal)
		{
		case CCharLobby::CHARLOBBY_LOGIN:
			{
				CharLobby_LogIn((CUser*)wParam, (LONG)lParam);
				break;
			}
		case CCharLobby::CHARLOBBY_USERDISCONNECT:
		case CCharLobby::CHARLOBBY_LINKCUT_FROMNLS:
			{
				CharLobby_Disconnect((LONG)wParam);
				break;
			}
		default:
			{
				// Community 관련된 메세지처리는 아래 함수에서...
				OnSignal_Community(hSignal, wParam, lParam);
				break;
			}
		}
	}
}

void CCharLobby::OnKickOutBanishUser()
{
	TLock lo(this);	
	ForEachElmt(TMapBanishUser, m_mapBanishUser, it, ij)
	{
		DWORD dwCurTime = GetTickCount();
		if (dwCurTime - (DWORD)((*it).second >= 2000))
		{
			LOG(INF_UK, "CHS_CChannel_Kickout, CChannel::OnSignal() Delete GSN :", (*it).first, ", TimeGap :", dwCurTime - (DWORD)((*it).second));
			m_pUserManager->KickOutUser((*it).first);
			m_mapBanishUser.erase((*it).first);
		}
	}
}

void CCharLobby::CharLobby_LogIn(CUser* pUser, LONG lErrCode)
{
	TLock lo(this);
	if (NULL == pUser) {
		theLog.Put(WAR_UK, "CharLobby_LogIn"_COMMA, "pUser is NULL");
		return;
	}

	CUser* pPrevUser = m_pUserManager->FindUser(pUser->GetCSN());
	if (pPrevUser) {
		theLog.Put(ERR_UK, "CharLobby_LogIn"_COMMA, " Already CSN :", pUser->GetCSN());
		m_pUserManager->KickOutUser(pPrevUser->GetCSN());
		m_pUserManager->RemoveUser(pPrevUser, pPrevUser->GetCSN());		// Add해주기 위해서 remove 먼저 한다...
	}

	if (!m_pUserManager->AddUser(pUser, pUser->GetCSN())) {
		theLog.Put(WAR_UK, "CharLobby_LogIn"_COMMA, "AddUser CSN: ", pUser->GetCSN());
		return;
	}

	{// NLS에 CSN 업데이트
		RoomID roomID;
		pUser->NLSGetRoomID(roomID);
		TKey key(pUser->GetGSN(), pUser->GetCSN());
//		theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_NFCHARLOBBY, roomID, pUser->GetLevel());
	}

	if (!g_achv.login(pUser->GetGSN(), pUser->GetCSN()))
		theLog.Put(ERR_UK, "achv::CAchvMgr::login() failed @ CharLobby_LogIn: CSN :", pUser->GetCSN());

	MsgNCSCli_AnsLogIn	ans;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsLogIn_Tag, ans);
	m_pUserManager->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobby::CharLobby_Disconnect(LONG lCSN)
{
	TLock lo(this);
	m_pCharLobbyCxt->OnDisconnectUser(lCSN);
}

