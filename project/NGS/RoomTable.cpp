//
// RoomTable.cpp
//

#include "stdafx.h"
#include "RoomTable.h"
#include "Room.h"

#include "Control.h"
#include <NLSManager.h>
///////////////////////////////////////////////////////////////////////////////////
// CRoomTable

CRoomTable theRoomTable;

// NF - EXA-VPN 때문에 존재 해야 한다....
DWORD GetIpAddress(string& strIP)
{
	char buf[1024];
	if(::gethostname(buf, 1024) != 0) return FALSE;
	HOSTENT* phe = ::gethostbyname(buf);
	if(!phe) return FALSE;
	DWORD dwIP = 0;

	// 먼저 10.X.Y.Z의 형태가 아닌 놈을 찾는다.
	if(!phe->h_addr_list) 
		return FALSE;

	for(int i = 0; *(phe->h_addr_list); i++) 
	{
		char * pa = phe->h_addr_list[i];
		if(pa == NULL) 
			break;
		strIP = ::inet_ntoa(*(in_addr*)(pa));
		dwIP = ((in_addr*)(pa))->S_un.S_addr;
	}
	return dwIP;
}
// NF

CRoomTable::CRoomTable() : m_bRegistered(FALSE), m_lRoomCount(0), m_lLastRoomCount(0)
{
	string sIP = GSocketUtil::GetHostIPString();
	//BOOL bRet =	GetIpAddress(sIP);
	if (sIP.size() <= 0)
	{
		theLog.Put(ERR_UK, "NGS_theRoomTable_Error"_COMMA, "*** Failed to get host address ***");
		return;
	}

	m_NSAP.SetIP(sIP);
	theNLSManager.SetNSAP(m_NSAP);

	CHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	szComputerName[0] = 0;
	DWORD dwSize = MAX_COMPUTERNAME_LENGTH;
	BOOL bRet = ::GetComputerNameA(szComputerName, &dwSize);
	if (!bRet)
	{
		theLog.Put(ERR_UK, "NGS_theRoomTable_Error"_COMMA, "*** Failed to get host name ***");
		return;
	}
	m_sComputerName.assign(szComputerName, dwSize);
}

CRoomTable::~CRoomTable()
{
}

BOOL CRoomTable::FindRoom(const RoomID& roomID, CRoom** ppRoom)
{
	ASSERT(ppRoom && !*ppRoom);
	*ppRoom = NULL;

	TLock lo(this);
	TIDRoomMap::iterator it = mIDRoomMap.find(roomID);
	if(it == mIDRoomMap.end()) 
		return FALSE;
	*ppRoom = it->second;
	(*ppRoom)->AddRef();
	return TRUE;
}

BOOL CRoomTable::Create(const RoomID& roomID, CRoom** ppRoom)
{
	ASSERT(ppRoom && !*ppRoom);
	*ppRoom = NULL;

	TLock lo(this);
	{
		TIDRoomMap::iterator it = mIDRoomMap.find(roomID);
		if(it != mIDRoomMap.end())
			return FALSE;

		AllocRoom(ppRoom); // rc == 1
		ASSERT(*ppRoom);
		ASSERT((*ppRoom)->GetRefCnt() == 1);
		ASSERT((*ppRoom)->GetState() == ROOMSTATE_DEAD);

		// add to table...
		(*ppRoom)->AddRef();
		mIDRoomMap[roomID] = *ppRoom;
		m_lRoomCount++;
	}
	return TRUE;
}

BOOL CRoomTable::RemoveRoom( const RoomID & rID )
{
	TLock lo(this);
	{
		//ASSERT(pRoom->GetState() == ROOMSTATE_DEAD);
		

		TIDRoomMap::iterator it = mIDRoomMap.find(rID);

//		VALIDATE(it != mIDRoomMap.end());
		if(it == mIDRoomMap.end()) {
			//TLOG0("CRoomTable::RemoveRoom() - Cannot find pRoom.\n");
			theLog.Put(WAR_UK, "NGS_theRoomTable_Error"_COMMA, "Can't find Room ID: ", RoomID2Str(rID), " in RemoveRoom");
			return FALSE;
		}

		CRoom * pRoom = it->second;
		if (!pRoom)
			return FALSE;

// NF
		pRoom->SetRunStopFlag(ROOMSTATE_STOP);
		pRoom->InitUserSlot();
// NF

//		VALIDATE(it->second == pRoom);
/*		if (it->second != pRoom)
		{
			//TLOG0("CRoomTable::RemoveRoom() - it->second != pRoom.\n");
			theLog.Put(WAR_UK, "NGS_theRoomTable_Error"_COMMA, "it->second != pRoom in RemoveRoom");
		}*/

		mIDRoomMap.erase(it);
		pRoom->Release();
		m_lRoomCount--;
		pRoom->Stop();
	}

	return TRUE;
}

void CRoomTable::SendToAll(GBuf& buf)
{
	TLock lo(this);
	CRoom* pRoom = NULL;
	ForEachCElmt(TIDRoomMap, mIDRoomMap, i1,j1)
	{
		pRoom = i1->second;
		//pRoom->SendToAll(buf);
		pRoom->OnBroadCastMessage( buf );
	}
}

BOOL CRoomTable::IsNotiNeeded(LONG& lRoomCount)
{
	TLock lo(this);
	lRoomCount = m_lRoomCount;
	return ((m_lRoomCount <= (m_lLastRoomCount - ROOM_DIFF_COUNT)) || (m_lRoomCount >= m_lLastRoomCount + ROOM_DIFF_COUNT));
}

void CRoomTable::OnNotify(LONG lRoomCount)
{
	TLock lo(this);
	m_lLastRoomCount = lRoomCount;
}

//void CRoomTable::SetNSAP(NSAP& nsap)
//{
//	TLock lo(this);
//	if (m_NSAP.m_lPort == 0) {
//		m_NSAP = nsap;
//	}
//}

BOOL CRoomTable::GetRoomList(ANNOUNCE_TYPE type, RoomList& lstRoom)
{
	TLock lo(this);
	ForEachCElmt(TIDRoomMap, mIDRoomMap, i1,j1)
	{
		CRoom* pRoom = i1->second;
		if (!pRoom) continue;
		pRoom->AddRef();
		lstRoom.push_back(pRoom);
	}

	if (!lstRoom.size()) return FALSE;

	return TRUE;
}

BOOL CRoomTable::GetRoomList(ANNOUNCE_TYPE type, LONG lSSN, RoomList& lstRoom)
{
	TLock lo(this);
	ForEachCElmt(TIDRoomMap, mIDRoomMap, i1,j1)
	{
		const RoomID& roomID = i1->first;
		if (roomID.m_lSSN != lSSN) continue;
		CRoom* pRoom = i1->second;
		if (!pRoom) continue;
		pRoom->AddRef();
		lstRoom.push_back(pRoom);
	}

	if (!lstRoom.size()) return FALSE;

	return TRUE;
}

BOOL CRoomTable::GetRoomList(ANNOUNCE_TYPE type, LONG lSSN, DWORD dwCategory, RoomList& lstRoom)
{
	TLock lo(this);
	ForEachCElmt(TIDRoomMap, mIDRoomMap, i1,j1)
	{
		const RoomID& roomID = i1->first;
		if (roomID.m_lSSN != lSSN) continue;
		if (roomID.m_dwCategory != dwCategory) continue;
		CRoom* pRoom = i1->second;
		if (!pRoom) continue;
		pRoom->AddRef();
		lstRoom.push_back(pRoom);
	}

	if (!lstRoom.size()) return FALSE;

	return TRUE;
}

BOOL CRoomTable::GetRoomList(ANNOUNCE_TYPE type, LONG lSSN, DWORD dwCategory, DWORD dwGCIID, RoomList& lstRoom)
{
	TLock lo(this);
	ForEachCElmt(TIDRoomMap, mIDRoomMap, i1,j1)
	{
		const RoomID& roomID = i1->first;
		if (roomID.m_lSSN != lSSN) continue;
		if (roomID.m_dwCategory != dwCategory) continue;
		if (roomID.m_dwGCIID != dwGCIID) continue;
		CRoom* pRoom = i1->second;
		if (!pRoom) continue;
		pRoom->AddRef();
		lstRoom.push_back(pRoom);
	}

	if (!lstRoom.size()) return FALSE;

	return TRUE;
}

BOOL CRoomTable::GetRoomList(ANNOUNCE_TYPE type, LONG lSSN, DWORD dwCategory, DWORD dwGCIID, DWORD dwGRIID, RoomList& lstRoom)
{
	TLock lo(this);
	RoomID roomID(lSSN, dwCategory, dwGCIID, dwGRIID);
	CRoom* pRoom = NULL;
	if (FindRoom(roomID, &pRoom)) {
		lstRoom.push_back(pRoom);
		return TRUE;
	}

	return FALSE;
}

BOOL CRoomTable::GetRoomList(RoomList& lstRoom)
{
	TLock lo(this);
	ForEachCElmt(TIDRoomMap, mIDRoomMap, i1,j1)
	{
		CRoom* pRoom = i1->second;
		if (!pRoom) continue;
		pRoom->AddRef();
		lstRoom.push_back(pRoom);
	}

	if (!lstRoom.size()) return FALSE;

	return TRUE;
}

string& CRoomTable::GetComputerName()
{
	TLock lo(this);

	return m_sComputerName;
}

void CRoomTable::SetComputerName(LPCSTR szName, DWORD dwLength)
{
	TLock lo(this);

	m_sComputerName.assign(szName, dwLength);
}

///////////////////////////////////////////////////////////////////////////////////
// Global Functions

DWORD WINAPI _SendToAllProc(LPVOID arg)
{
	AutoPtrT<GBuf> ap = (GBuf*)arg;
	theRoomTable.SendToAll(*ap);
	return 0;
}

void PostSendToAll(GBuf* pBuf)
{
	BOOL bRet = ::XtpQueueWorkItem(GetThreadPool(), _SendToAllProc, pBuf);
	if(!bRet)
		delete(pBuf);
}
