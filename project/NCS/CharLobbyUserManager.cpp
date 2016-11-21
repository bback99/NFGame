
#include "stdafx.h"
#include "User.h"
#include "CharLobby.h"
#include "CharLobbyUserManager.h"

//////////////////////////////////////////////////////////////////////////
CCharLobbyUserManager::CCharLobbyUserManager(CCharLobby* pCharLobby) : m_pCharLobby(pCharLobby)
{

}

CCharLobbyUserManager::~CCharLobbyUserManager()
{

};


void CCharLobbyUserManager::OnUserMsg(CUser* pUser, PayloadCliNCS& pld)
{
	m_pCharLobby->GetCharLobbyCxt()->OnUserMsg(pUser, pld);
}

void CCharLobbyUserManager::PostUserDisconnect(CUser* pUser)
{
	m_pCharLobby->GetCharLobbyCxt()->PostUserDisconnect(pUser->GetCSN());		// CCharLobby´Â Key°ªÀÌ CSN
}