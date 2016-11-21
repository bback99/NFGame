

#pragma once

#include "UserManager.h"

class CCharLobby;

class CCharLobbyUserManager : public CUserManager
{
private:
	CCharLobby* m_pCharLobby;

public:
	CCharLobbyUserManager(CCharLobby* pCharLobby);
	virtual ~CCharLobbyUserManager();

	virtual void OnUserMsg(CUser* pUser, PayloadCliNCS& pld);
	virtual void PostUserDisconnect(CUser* pUser);
};


typedef AutoPtrT<CCharLobbyUserManager> CCharLobbyUserManagerPtr;