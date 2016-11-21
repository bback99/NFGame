//CRoomEventQueue
#include "stdafx.h"
#include "RoomEventQueue.h"

CRoomEventQueue::CRoomEventQueue( IRoomEventHandler * pHandler )
:
pHandler(pHandler),
m_dwRefCnt(0)
{

}

void CRoomEventQueue::OnQueueElmt( RoomEvent& t)
{
	pHandler->ProcessRoomEvent( t );
}

BOOL CRoomEventQueue::Activate( HTHREADPOOL hPool )
{
	return this->ActivateQueue( hPool );
}

ULONG CRoomEventQueue::AddRef()
{
	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
	return dwRefCnt;
}

ULONG CRoomEventQueue::Release()
{
	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
	if(dwRefCnt == 0)
	{
	}
	return dwRefCnt;
}