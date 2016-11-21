
#ifndef _CHAR_LOBBY_
#define _CHAR_LOBBY_

#include "CharLobbyUserManager.h"
#include "CharLobbyContext.h"

class PayloadCliNCS;

class CCharLobby : public GXSigHandler
{
	IMPLEMENT_TISAFE(CCharLobby)

	enum E_CHARLOBBY {
		CHARLOBBY_LOGIN,
		CHARLOBBY_USERDISCONNECT,

		// From_NLS
		CHARLOBBY_LINKCUT_FROMNLS,
		CHARLOBBY_NLS_GET_FRIEND_LIST_INFO,			// NLS�� ���� ģ���� ���� ��û
		CHARLOBBY_PROCESS_NF_FRIEND_APPLICATION,	// ģ����û ó��
		CHARLOBBY_PROCESS_NF_FRIEND_ACCEPT,			// ģ����û ���� ó��
		CHARLOBBY_PROCESS_NF_FRIEND_AUTO_ACCEPT,	// ģ����û ���� ó��(�ڵ�)
		CHARLOBBY_PROCESS_FOLLOW_USER,
		CHARLOBBY_PROCESS_NF_LETTER_NEW,			// ������

		// From_CHS
		CHARLOBBY_NOTIFY_ACCEPT_FRIEND_FROM_CHS,

		// From_NGS
		CHARLOBBY_NOTIFY_ACCEPT_FRIEND_FROM_NGS,
		CHARLOBBY_NOTIFY_NEW_LETTER_FROM_NGS,
		CHARLOBBY_ADDFRIEND_NOTIFY,
	};

protected:
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

public:
	friend class CCharLobbyContext;
	typedef GXSigHandler TBase;

public:
	CCharLobby(DWORD dwCharLobbyID);
	virtual ~CCharLobby();
	void OnKickOutBanishUser();
	void CharLobby_LogIn(CUser* pUser, LONG lErrCode);
	void CharLobby_Disconnect(LONG lCSN);
	BOOL AddUser(CUser* pUser);


public:
	// community
	void OnSignal_Community(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
	void ProcessNFFriendList(LPXBUF buf);
	void ProcessNFFriendApplication(LPXBUF buf);
	void ProcessNFFriendAccept(LPXBUF buf);
	void ProcessNFFriendAutoAccept(LPXBUF buf);
	void ProcessFollowUser(LPXBUF buf);
	void ProcessNFLetterNew(LPXBUF buf);
	void ProcessAcceptFriendFromCHS(LPXBUF buf);
	void ProcessNewLetterFromNGS(LPXBUF buf);
	void ProcessAcceptFriendFromNGS(LPXBUF buf);
	void ProcessAddFriendFromNGS(LPXBUF buf);

public:
	CCharLobbyUserManager* GetUserManager() const { return m_pUserManager; }
	CCharLobbyContext* GetCharLobbyCxt() { return m_pCharLobbyCxt; }

private:
	DWORD					m_dwRefCnt;
	DWORD					m_dwCharLobbyID;
	CCharLobbyContext* 		m_pCharLobbyCxt;
	CCharLobbyUserManager*	m_pUserManager;

	GXSigTimer				m_timerBanishUser;
	typedef map<LONG, LONG> TMapBanishUser;			// USN - RegTime
	TMapBanishUser			m_mapBanishUser;
};


typedef vector< CCharLobby* > CCharLobbyTable;


#endif // _CHAR_LOBBY_