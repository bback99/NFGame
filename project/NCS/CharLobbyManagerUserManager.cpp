
#include "stdafx.h"
#include "User.h"
#include "CharLobbyManagerUserManager.h"
#include "CharLobbyManager.h"

//////////////////////////////////////////////////////////////////////////
CCharLobbyManagerUserManager::CCharLobbyManagerUserManager(CCharLobbyManager* pCharLobbyManager) : m_pCharLobbyManager(pCharLobbyManager)
{
}

CCharLobbyManagerUserManager::~CCharLobbyManagerUserManager()
{

};

void CCharLobbyManagerUserManager::OnUserMsg(CUser* pUser, PayloadCliNCS& pld)
{
	m_pCharLobbyManager->OnUserMsg(pUser, pld);
}

void CCharLobbyManagerUserManager::PostUserDisconnect(CUser* pUser)
{
	m_pCharLobbyManager->PostUserDisconnect(pUser->GetGSN());		// LobbyManager´Â Key°ªÀÌ GSN
}