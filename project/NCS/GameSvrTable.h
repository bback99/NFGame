// 
// GameSvrTable.h
// 
/**********************************************************
		Game(GLS) Server Infomation Table
									-ss
***********************************************************/

#ifndef _GAMESVRTABLE_H__
#define _GAMESVRTABLE_H__
#include <ADL/MsgAdminStruct.h>


class GLSInfo : public ServerRegistration
{
public:
	typedef ServerRegistration TBase;
public:
	GLSInfo();
	GLSInfo(ServerRegistration& serverInfo);
	GLSInfo(NSAP& nsap, LRBAddress lrbAddr);
	virtual ~GLSInfo();
public:
	LONG ComputeLoad(time_t lTime);
public:
	virtual void Clear();
public:
	void SetLoadInfo(LONG lRoomCount, LONG lUserCount, time_t lTime) { m_lRoomCount = lRoomCount; m_lUserCount = lUserCount; m_lLastUpdateTime = lTime; }
	void SetServiceTypeID(LONG lTypeID) { m_lServiceTypeID = lTypeID; }
	void SetLogicAddr(LRBAddress lrbAddr) { m_lrbLogicAddr = lrbAddr; }
	LRBAddress GetLogicAddr() { return m_lrbLogicAddr; }
	LONG GetServiceTypeID() { return m_lServiceTypeID; }
	LONG GetRoomCount() { return m_lRoomCount; }
	void SetRoomCount(LONG lRoomCount) { m_lRoomCount = lRoomCount; }
	LONG GetUserCount() { return m_lUserCount; }
	void SetUserCount(LONG lUserCount) { m_lUserCount = lUserCount; }

#ifdef _BELB_
	LONG ComputeLoad_RelayRoom(time_t lTime);
	LONG GetBetRoomCount() { return m_lBetRoomCount; }
	void SetBetRoomInc() { ++m_lBetRoomCount; }
	void SetBetRoomDec() { --m_lBetRoomCount; }
#endif

private:
	BOOL IsActive(time_t lTime);
private:
	LONG m_lRoomCount;
	LONG m_lUserCount;
	LONG m_lServiceTypeID;

#ifdef _BELB_
	LONG m_lBetRoomCount;
#endif

	//LONG m_lLogicAddr;
	LRBAddress	m_lrbLogicAddr;
	time_t m_lLastUpdateTime;
public:
	friend class GameSvrList;
};

class GameSvrList
{
	friend class GameSvrTable;
public:
	typedef ArcPtrListT<GLSInfo> TList;
public:
	GameSvrList(LONG lServerTypeID);
	virtual ~GameSvrList();
public:
	BOOL AddGameSvr(GLSInfo* pInfo);

public:
	BOOL DeleteGameSvr(LRBAddress lrbLogicAddr);	//Logic Address
	BOOL UpdateGameSvr(LRBAddress lrbLogicAddr, NSAP& nsap, LONG lRoomCount, LONG lUserCount, time_t lTime);

#ifdef _BELB_
	BOOL BetRoomIncGameSvr(LRBAddress LRBAddress, time_t lTime);
	BOOL BetRoomDecGameSvr(LRBAddress LRBAddress, time_t lTime);
	BOOL FindRelayRoomSvr(LRBAddress& lrbAddress);
#endif

	BOOL FindMinGameSvr(NSAP& nsap);
	BOOL FindMinGameSvr(LRBAddress& lrbAddress);
	//BOOL FindModGameSvr(NSAP& nsap, LONG lIndex, LONG& lCntOfGLS);
public:
	void SetServiceTypeID(LONG lTypeID) { m_lServiceTypeID = lTypeID; }
	LONG GetServiceTypeID() { return m_lServiceTypeID; }
	LONG GetRegistedGlsCount() { return m_lstGLS.size(); }
public:
	void Clear();
protected:
	TList m_lstGLS;
	LONG m_lServiceTypeID;
};

//GLS Server Table Accord to ServiceType
class GameSvrTable 
{
	IMPLEMENT_TISAFE(GameSvrTable)
public:
	typedef pmap<LONG, GameSvrList> TMap;
public:
	GameSvrTable();
	virtual ~GameSvrTable();
public:
	BOOL Init();
public:
	BOOL AddGameSvr(LONG lType, GLSInfo* glsInfo);
	BOOL UpdateGameSvr(LONG lType, LRBAddress lrbLogicAddr, NSAP& nsap, LONG lRoomCount, LONG lUserCount);
	BOOL DeleteGameSvr(LRBAddress lrbLogicAddr);
public:
	BOOL FindProperGameSvr(LONG lType, NSAP& nsap, DWORD dwRoomIndex, LONG& lCntOfGLS);

#ifdef _BELB_
	BOOL FindProperRoomSvr(LRBAddress& lrbAddress, LONG lIsRelayRoom=0);
	BOOL BetRoomIncrement(LONG lType, LRBAddress lrbLogicAddr);
	BOOL BetRoomDecrement(LONG lType, LRBAddress lrbLogicAddr);
#endif

public:
	void AllGameSvrList(AMS_GLSInfoList& rlstAMSGlsInfo);

	void Clear();
private:
	TMap m_glsTable;
};

extern GameSvrTable theGameSvrTable;

#endif