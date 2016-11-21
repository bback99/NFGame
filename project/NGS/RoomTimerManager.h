#pragma once
#include <LxSvr.h>
#include "TimerStruct.h"
#include <ADL/MsgCommonStruct.h>


class CRoomTimerManager
{
	IMPLEMENT_TIREFCNT0( CRoomTimerManager )
private:

	IXObject * m_pTimerHandler;

	TimerList m_lstTimer;

	xstring roomIdStr;

	long	  m_lock_timer_debug;

public:
	CRoomTimerManager( );
	virtual ~CRoomTimerManager(void);

	void InitializeVar( const xstring & rIDstr, IXObject * pTimerHandler )
	{
		roomIdStr = rIDstr;
		m_pTimerHandler = pTimerHandler;
	}

	LONG RemoveAllTimer();
	LONG RemoveTimer(LONG lIndex);
	LONG AddTimer(DWORD dwDue, DWORD dwPeriod, LONG lTimerIndex );

	BOOL ProcessTimerSignal( LONG & UserTimerIndex, const HSIGNAL &hObj, const WPARAM &wParam, const LPARAM &lParam, DWORD queueTime );

};
