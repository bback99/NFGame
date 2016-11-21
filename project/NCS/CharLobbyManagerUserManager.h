

#pragma once

#include "UserManager.h"


class CCharLobbyManagerUserManager : public CUserManager
{
private:
	CCharLobbyManager* m_pCharLobbyManager;

public:
	CCharLobbyManagerUserManager(CCharLobbyManager* pCharLobbyManager);
	virtual ~CCharLobbyManagerUserManager();

	virtual void OnUserMsg(CUser* pUser, PayloadCliNCS& pld);
	virtual void PostUserDisconnect(CUser* pUser);
};