#pragma once

// BroadCasing System Manager
#ifndef _NFGAME_
#include "ADL/MsgCommonStruct.h"
#else
#include "ADL/MsgNFCommonStruct.h"
#endif

#include "ADL/MsgCHSCli_NF.h"
#include "ADL/MsgBSCommon.h"
#include "ADL/MsgBSCHS.h"

class CBSManager : public IXObject
{
public:
	IMPLEMENT_TISAFE(CBSManager);

	CBSManager() ;
	virtual ~CBSManager();

////////////////////////////////////////////////////////////////////////////
public:
	void SendRegisterBMSInfoReqToBMS();

	void OnRcvRegisterAnsFromBMS(const LRBAddress &addr, MsgBMSCHS_RegisterBMSAns *pAns);
	void OnRcvRegisterNtfFromBMS(const LRBAddress &addr, MsgBMSCHS_RegisterBMSNtf *pNtf);

protected:
	LRBAddress  *m_aryAddrBMS;
	int			m_numBMS;

////////////////////////////////////////////////////////////////////////////
public:
	//BOOL OnRcvStartPeepReqFromCli(RoomID &roomID, LONG lUSN, ChannelID& CID, LONG lMSN  = -1);
	BOOL OnRcvStartPeepReqFromCli(RoomID &roomID, LONG lUSN, ChannelID& CID, LONG lSSN, BOOL bIsVIPRoom);
	void OnRcvStartPeepAnsFromBCS(MsgBCSCHS_StartPeepAns *pAns, ChannelID &cid, LONG &lUSN, MsgCHSCli_StartPeepAns* pToCli);
		 
protected:
	GCRITICAL_SECTION		m_csUsnCid;
	map<LONG, ChannelID>	m_mapUsnCid;

////////////////////////////////////////////////////////////////////////////
public:
	void AddRoomLRBAddr(const RoomID & roomID, const LRBAddress &addr);
	void RemoveRoomLRBAddr(const RoomID & roomID);
	BOOL GetRoomLRBAddr(const RoomID & roomID, LRBAddress &addr);

	void DeleteBMSAddr(const LRBAddress& addrBMS);

protected:	

	GCRITICAL_SECTION  m_csRoomAddr;
	map<RoomID, LRBAddress> m_mapRoomAddr;

	typedef ObjLockT<GCRITICAL_SECTION> AutoLock;
	typedef map<RoomID, LRBAddress>		TRoomIDAddrMap;
////////////////////////////////////////////////////////////////////////////


protected:
	DWORD m_dwRefCnt;
	STDMETHOD_(ULONG, AddRef)() { return (DWORD)::InterlockedIncrement((LPLONG)&m_dwRefCnt);};
	STDMETHOD_(ULONG, Release)(){ return (DWORD)::InterlockedDecrement((LPLONG)&m_dwRefCnt);};
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

};

extern CBSManager theBSManager;