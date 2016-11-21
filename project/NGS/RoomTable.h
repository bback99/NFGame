//
// RoomTable.h
//

#ifndef ROOMTABLE_H
#define ROOMTABLE_H

#include "Common.h"
#include <ADL/MsgAdminStruct.h>

///////////////////////////////////////////////////////////////////////////////////
class CRoom;

///////////////////////////////////////////////////////////////////////////////////
typedef list<RoomID> RoomIDList;
typedef plist<CRoom> RoomList;

///////////////////////////////////////////////////////////////////////////////////
// CRoomTable

enum 
{
	NGSME_UNI = 0,
	NGSME_MUL,
	NGSME_ANY,
	NGSME_BRO,
	MAX_MYADDR_CNT
};

class CRoomTable
{
	IMPLEMENT_TISAFE(CRoomTable)
public:
	typedef pmap<RoomID, CRoom> TIDRoomMap; // CCID -> Room
public:
	CRoomTable();
	virtual ~CRoomTable();
public:
	BOOL FindRoom(const RoomID& roomID, CRoom** ppRoom);
	BOOL Create(const RoomID& roomID, CRoom** ppRoom);

	BOOL RemoveRoom( const RoomID& rID );
	void GetRoomInfo(PUINT plSize, LPDWORD pdwCapacity, LPDWORD pdwAvailable, LPDWORD pdwMaxCapacity);
	void SendToAll(GBuf& buf);
public:
	BOOL IsNotiNeeded(LONG& lRoomCount);
	void OnNotify(LONG lRoomCount);
public:
	const NSAP& GetNSAP() const { return m_NSAP; }
	void SetNSAP(NSAP& nsap);
public:
	const LRBAddress& GetAddr() const { return m_addr; }
	LRBAddress& GetAddr() { return m_addr; }
	void SetAddr(const LRBAddress& addr) { TLock lo(this); m_addr = addr; }
	const LRBAddress& GetMulticastAddr() const { return m_MulticastAddr; }
	void SetMulticastAddr(const LRBAddress& addr) { TLock lo(this); m_MulticastAddr = addr; }
	const LRBAddress& GetChsAddr() const { return m_ChsMulticastAddr; }
	void SetChsAddr(const LRBAddress& addr) { TLock lo(this); m_ChsMulticastAddr = addr; }
	const LRBAddress& GetNLSAddr() const { return m_NLSMulticastAddr; }
	void SetNLSAddr(const LRBAddress& addr) { TLock lo(this); m_NLSMulticastAddr = addr; }
	const LRBAddress& GetNCSAddr() const { return m_NCSMulticastAddr; }
	void SetNCSAddr(const LRBAddress& addr) { TLock lo(this); m_NCSMulticastAddr = addr; }
	const LRBAddress& GetNASAddr() const { return m_NASMulticastAddr; }
	void SetNASAddr(const LRBAddress& addr) { TLock lo(this); m_NASMulticastAddr = addr; }

	void SetMyAddr(const LRBAddress& addr, int index) { TLock lo(this); m_MyAddr[index] = addr; }
	const LRBAddress& GetMyAddr(int index) { if(index >= 0 && index < MAX_MYADDR_CNT) return m_MyAddr[index]; else return m_MyAddr[0]; }

	DWORD GetTypeID() const { return m_dwTypeID; }
	void SetTypeID(DWORD dwType) { TLock lo(this); m_dwTypeID = dwType; }
public:
	BOOL IsRegistered() { TLock lo(this); return m_bRegistered; }
	void SetRegister(BOOL bRegister) { TLock lo(this); m_bRegistered = bRegister; }

	BOOL IsPLSRegistered() { TLock lo(this); return m_bPLSRegistered; }
	void SetPLSRegister(BOOL bPlsRegister) { TLock lo(this); m_bPLSRegistered = bPlsRegister; }

	BOOL IsIBBAlive() { TLock lo(this); return m_bIBBRegistered; }
	void SetIBBAlive() { TLock lo(this); m_bIBBRegistered = TRUE; }
	void SetIBBTerminate() { TLock lo(this); m_bIBBRegistered = FALSE; }

	LONG GetRoomCount() { TLock lo(this); return m_lRoomCount; }
public:

	BOOL GetRoomList(ANNOUNCE_TYPE type, RoomList& lstRoom);
	BOOL GetRoomList(ANNOUNCE_TYPE type, LONG lSSN, RoomList& lstRoom);
	BOOL GetRoomList(ANNOUNCE_TYPE type, LONG lSSN, DWORD dwCategory, RoomList& lstRoom);
	BOOL GetRoomList(ANNOUNCE_TYPE type, LONG lSSN, DWORD dwCategory, DWORD dwGCIID, RoomList& lstRoom);
	BOOL GetRoomList(ANNOUNCE_TYPE type, LONG lSSN, DWORD dwCategory, DWORD dwGCIID, DWORD dwGRIID, RoomList& lstRoom);
	BOOL GetRoomList(RoomList& lstRoom);

	void SetChatMonSSN(LONG lSSN) { TLock lo(this); m_lChatMonSSN = lSSN; }
	void SetChatMonCategory(LONG lCategory) { TLock lo(this); m_lChatMonCategory = lCategory; }

	LONG GetChatMonSSN() { return m_lChatMonSSN; }
	LONG GetChatMonCategory() { return m_lChatMonCategory; }

	void SetRoomTitleMonitoring(BOOL bRoomTitleMonitoring) { m_bRoomTitleMonitoring = bRoomTitleMonitoring; }
	BOOL IsRoomTitleMonitoringOn() { return m_bRoomTitleMonitoring; }

public:
	string& GetComputerName();
	void SetComputerName(LPCSTR szComputerName, DWORD dwLength);
	vector<DWORD> GetTypeIDVector() { TLock lo(this); return m_vecTypeID;};
protected:
	TIDRoomMap	mIDRoomMap;

	BOOL m_bRegistered;
	NSAP m_NSAP;

	BOOL m_bPLSRegistered;

	LONG m_lChatMonSSN;
	LONG m_lChatMonCategory;
	
	BOOL m_bIBBRegistered;
	BOOL m_bMDSRegistered;

	BOOL m_bRoomTitleMonitoring;

	LRBAddress m_addr;
	LRBAddress m_MulticastAddr;
	LRBAddress m_ChsMulticastAddr;
	LRBAddress m_NCSMulticastAddr;
	LRBAddress m_NLSMulticastAddr;
	LRBAddress m_NASMulticastAddr;
	LRBAddress m_MyAddr[4];

	DWORD m_dwTypeID;
	LONG m_lRoomCount;		// 현재 룸 수
	LONG m_lLastRoomCount;	// 직전에 CHS에게 알린 룸 수
	string m_sComputerName;

	vector<DWORD> m_vecTypeID;

	map<LONG, RoomID> m_mapUSNRoomID;
};

extern CRoomTable theRoomTable;

//extern void PostUserJoin(CUser* pUser);
extern void ProcessUserJoin(CUser* pUser);
extern void PostSendToAll(GBuf* buf);
extern void SendNoticeToRoom(string sChannel, GBuf* pBuf);

#endif //!ROOMTABLE_H
