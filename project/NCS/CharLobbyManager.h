

#ifndef _CHAR_LOBBY_MANAGER_
#define _CHAR_LOBBY_MANAGER_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Link.h"

#include <NF/ADL/MsgNCSCli.h>
class CCharLobby;
class CCharLobbyManagerUserManager;

class CCharLobbyManager : public GXSigHandler
{
	IMPLEMENT_TISAFE(CCharLobbyManager)
	typedef GXSigHandler TBase;

public:
	enum {
		CHARLOBBYMGR_NCSADDUSERANS,
		CHARLOBBYMGR_LOGOUT,
		CHARLOBBYMGR_NFCHARINFOLIST,
		CHARLOBBYMGR_USERDISCONNECT,
	};

public:
	CCharLobbyManager();
	virtual ~CCharLobbyManager();

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD_(void, OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

public:
	CCharLobbyManagerUserManager* GetUserManager() const { return m_pUserManager; }

	void OnUserMsg(CUser* pUser, PayloadCliNCS& pld);
	BOOL CreateCharLobby();
	void JoinNCSUser(LPARAM lParam);
	void OnReqLogOut(LPARAM lParam);
	BOOL AddUser(CUser* pUser);
	void RemoveLink(CUser* pUser);
	BOOL GetNFCharInfoFromDB(CUser* pUser, LONG& lErr);
	CCharLobby* FindCharLobby(LONG lKey);
	void OnUserDisconnect(LONG lKey);	/*GSN*/
	void PostUserDisconnect(LONG lKey);
	void ProcessReqLogin(CUser* pUser, LONG lLoginCSN);

protected:
	BOOL InsertDefaultCharItem(const LONG lGSN, NFCharInfo& nfCharInfo);

	// adl handler
public:
	void OnReqCreateNFChar(CUser* pUser, MsgCliNCS_ReqCreateNFChar* pMsg);
	void OnReqDeleteNFChar(CUser* pUser, MsgCliNCS_ReqDeleteNFChar* pMsg);
	void OnReqCheckExistNick(CUser* pUser, MsgCliNCS_ReqExistNick* pMsg);
	void OnReqCharShopList(CUser* pUser, MsgCliNCS_ReqCharShopList* pMsg);
	void OnReqLogin(CUser* pUser, MsgCliNCS_ReqLogIn* pMsg);
	void OnReqTutorial(CUser* pUser, MsgCliNCS_ReqTutorial* pMsg);
	void OnReqNFCharInfoList(CUser* pUser, MsgCliNCS_ReqNFCharInfoList* pMsg);

private:
	DWORD m_dwRefCnt;

	typedef std::map<DWORD, CCharLobby*> TMapCharLobby;
	TMapCharLobby m_mapCharLobby;

	CCharLobbyManagerUserManager*		m_pUserManager;
};


extern CCharLobbyManager	theCharLobbyManager;


#endif  //_CHAR_LOBBY_MANAGER_
