//
// RoomRoom.cpp
//

#include "stdafx.h"
#include "Room.h"

///////////////////////////////////////////////////////////////////////////////////
// CFreeRoomTable

class CFreeRoomTable 
{
	IMPLEMENT_TISAFE(CFreeRoomTable)
public:
	CFreeRoomTable() {}
	virtual ~CFreeRoomTable();

	CRoom* Pop();
	void Push(CRoom*);
	void Flush();
protected:
	typedef pqueue<CRoom> Table;
	Table mTable;
};

static CFreeRoomTable theFreeRoomTable;

CFreeRoomTable::~CFreeRoomTable()
{
	Flush();
}
CRoom* CFreeRoomTable::Pop()
{
	TLock lo(this);
	if(mTable.empty())
		return NULL;
	CRoom* pRoom = mTable.front();
	mTable.pop();
	return pRoom;
}
void CFreeRoomTable::Push(CRoom* pRoom)
{
//	ASSERT(pRoom);
	if (!pRoom) return;
	TLock lo(this);
	mTable.push(pRoom);
}
void CFreeRoomTable::Flush()
{
	TLock lo(this);
	CRoom* pRoom;
	while((pRoom = Pop()) != NULL)
		DeleteRoom(pRoom);
}

void AllocRoom(CRoom** ppRoom)
{
	ASSERT(ppRoom);
	*ppRoom = 0;

	CRoom* pRoom = theFreeRoomTable.Pop();
	if(!pRoom)
	{
		pRoom = new CRoom;
		if (!pRoom) return;
	}
	ASSERT(pRoom->GetState() == ROOMSTATE_DEAD);
	ASSERT(pRoom->GetRefCnt() == 0);
	if (pRoom->GetRefCnt() != 0)
		theLog.Put(INF_UK, "NGS_DebugInfo,AllocRoom(). RoomID:",RoomID2Str( pRoom->GetRoomID() ),", RoomIndex:", pRoom->m_dwRID,", State:",pRoom->GetState(),", RefCount:", pRoom->GetRefCnt());	
	pRoom->AddRef();	
	*ppRoom = pRoom;	
}

void FreeRoom(CRoom* pRoom)
{
	//theLog.Put(DEV, "FreeRoom(", pRoom->m_dwRID, ")");	

	ASSERT(pRoom->GetState() == ROOMSTATE_DEAD);
	ASSERT(pRoom->GetRefCnt() == 0);
	theFreeRoomTable.Push(pRoom);
	if (pRoom->GetRefCnt() != 0)
		theLog.Put(INF_UK, "NGS_DebugInfo,FreeRoom(). RoomID:",RoomID2Str(pRoom->GetRoomID() ),", RoomIndex:", pRoom->m_dwRID,", State:",pRoom->GetState(),", RefCount:", pRoom->GetRefCnt());
}

void DeleteRoom(CRoom* pRoom)
{
	theLog.Put(DEV, "DeleteRoom(", pRoom->m_dwRID, ")");

	delete(pRoom);
}


