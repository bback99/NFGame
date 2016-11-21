#pragma once
#include "LxSvr.h"

typedef struct __TimerStruct {
	LONG m_lIndex;	
	GXSigTimer* m_pTimer;
	DWORD m_prevTime;
	DWORD m_dueTime;
	DWORD m_periodTime;
	LONG  m_debug_count;
	DWORD m_addTimerTime;
} TimerStruct;

typedef list<TimerStruct> TimerList;