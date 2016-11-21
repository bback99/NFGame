#pragma once

#include <GXLinkUtl.h>
#include <GXSigUtl.h>
#include "Common.h"

class CRoomEventQueue : public XSigSafeDataQueueHandlerT< RoomEvent >
{
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
public:
	CRoomEventQueue( IRoomEventHandler * pHandler );

	virtual void OnQueueElmt( RoomEvent& t);

	BOOL Activate( HTHREADPOOL hPool );
	void ClearEvent()
	{
		FlushQueue();
	}


private:
	DWORD m_dwRefCnt;
	IRoomEventHandler * pHandler;
};
