
#pragma once 

class CUser;
class CCharLobby;
class CUserLinkManager;
class CCharLobbyManager;
class PayloadCliNCS;
class PayloadNCSCli;

interface IUserManager
{
	virtual void OnUserMsg(CUser* pUser, PayloadCliNCS& pld) = 0;
	virtual BOOL DestroyUser(CUser * pUser, long lKey) = 0;
	virtual void RemoveUserLink(CUser * pUser) = 0;
	virtual void PostUserDisconnect(CUser* pUser) = 0;
};

class CUserManager : public IUserManager
{
	IMPLEMENT_TSAFE(CUserManager)

public:
	typedef PayloadCliNCS TMsg;
	typedef std::list<TMsg> TMsgQ;

	CUserManager();
	virtual ~CUserManager(void);

	CUserLinkManager * m_pUserLinkManager;

public:
	virtual void OnUserMsg(CUser* pUser, PayloadCliNCS& pld) = 0;
	virtual BOOL DestroyUser( CUser * pUser, long lKey );
	virtual void RemoveUserLink( CUser * pUser );
	virtual void PostUserDisconnect(CUser* pUser) = 0;

public:
	void Clear();
	BOOL Start(HTHREADPOOL hPool);
	BOOL Stop();
	size_t GetUserCnt();
	CUser * FindUser( long lKey ) const;
	BOOL AddUser( CUser* pUser, long lKey );
	BOOL KickOutUser( long lKey );
	BOOL DestroyUser( long lKey );
	BOOL DestroyAllUser();
	BOOL SendToUser( long lKey , const PayloadNCSCli & pld );
	BOOL SendToUser( CUser * pUser, const PayloadNCSCli & pld );
	void SendToAllUsersExceptOne( long lExceptKey, const PayloadNCSCli & pld );
	void ResetMsgRcvCnt();
	DWORD GetUserLinkIndex( long lKey ); 
	size_t ProcessAliveCheckTimer( std::list<long> & AliveCheckTimeOutKey, std::list<long> & needCheckKey );
	void SendToAllUser(const PayloadNCSCli& pld);
	BOOL RemoveLink(CUser* pUser);
	BOOL RemoveUser( CUser * pUser, long lKey );

	CUserLinkManager* GetUserLinkManager() { return m_pUserLinkManager; }

private:
	void DeleteUserAndLink( CUser * pUser );
	
	typedef std::map< /*Key*/long, CUser *> TMapUsermap;

	TMapUsermap UserTable;

	typedef TMapUsermap::iterator UserItr;
	typedef TMapUsermap::const_iterator UserConstItr;
};


