#pragma once

#include <GXLinkUtl.h>
#include <GXSigUtl.h>
#include "Common.h"

class CUserLinkManager;
class CRoomEventQueue;




class CUser;

class CRoomUserManager
{
	//RWLock���� ���� ����?
	IMPLEMENT_TSAFE(CRoomUserManager)
public:
	CRoomUserManager( IRoomEventHandler * pHandler );
	virtual ~CRoomUserManager(void);

public:
	//interfacce
	void clear();
	BOOL Start( HTHREADPOOL hPool );
	BOOL Stop();

	size_t GetUserCnt()
	{
		return UserTable.size();
	}

	CUser * FindUser( long lCSN ) const;

	BOOL AddUser( CUser * pUser );

	BOOL CutUser( long lCSN );

	BOOL DestroyUser( long lCSN );
	BOOL DestroyAllUser();

	void GetAllUserBaseInfo( UserBaseInfoList &userbaseInfo );
	void GetAllUserBaseInfo( TListNFCharInfo &userbaseInfo );

// NF
	void GetAllUserBaseInfo( NFJoinUserBaseInfoList& joinUserBaseInfoList );
	void GetAllUserBaseInfo( NFUserBaseInfoList &nfUBI );
// NF
	
	BOOL SendToUser( long lCSN , const PayloadNGSCli & pld );
	
	void SendToAllUser( const PayloadNGSCli & pld );
	void SendToAllUsersExceptOne( long lExceptUSN, const PayloadNGSCli & pld );
	

	void ResetMsgRcvCnt();

	DWORD GetUserLinkIndex( long lCSN ); 

	size_t ProcessAliveCheckTimer( std::list<long> & AliveCheckTimeOutUSN, std::list<long> & needCheckUSN );

	void AddRoomEvent( const RoomEvent & evt );

public:
	//CUser Interface

	typedef PayloadCliNGS TMsg;
	typedef list<TMsg> TMsgQ;
//	typedef list<TGameData> GameDataQ;

public:


	

private:
	

	BOOL RemoveUser( CUser * pUser );

	BOOL SendToUser( CUser * pUser, const PayloadNGSCli & pld );
	
	void RemoveUserLink( CUser * pUser );
	
	void DeleteUserAndLink( CUser * pUser );
	BOOL DestroyUser( CUser * pUser );
	


	CUserLinkManager * UserLinkManager;
	CRoomEventQueue * EventQueue;
	//�濡 �� ���� �� �˰� ������ bucket size ������ �ִ°��� ����
	typedef stdext::hash_map< /*USN*/long, CUser *> hashUserMap;


	hashUserMap UserTable;

	
	typedef hashUserMap::iterator UserItr;
	typedef hashUserMap::const_iterator UserConstItr;


// NF
public:
	BOOL IsCheckMapLoading(BOOL bIsDis=FALSE);
	BOOL InitTeamPlayData();
	BOOL GetSingleGameResult(TLstGameResult& lstPlayer, BOOL bIsSimple);
	BOOL GetTeamGameResult(TLstGameResult& lstATeamPlayer, TLstGameResult& lstBTeamPlayer, BOOL bIsSimple);
	BOOL CheckUserGameStartStatus();
	BOOL SetUserGameStartStatus(USER_LOCATION_STATE lStatus, LONG lMapIndex);
	CUser* GetNextCap(LONG lCSN);
};
