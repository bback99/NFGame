#include "stdafx.h"
#include ".\roomtimermanager.h"
#include "ChatHistory.h"

CRoomTimerManager::CRoomTimerManager():
m_lock_timer_debug( 0 )
{
}

CRoomTimerManager::~CRoomTimerManager(void)
{
}



LONG g_TimerCountDueAdd[101] = {0x00,};
LONG g_TimerCountDueRem[101] = {0x00,};
LONG g_TimerCountPerAdd[101] = {0x00,};
LONG g_TimerCountPerRem[101] = {0x00,};
LONG g_TimerNumLoggingTime = 0;

void RemTimerLogging(DWORD dwDue, DWORD dwPeriod)
{
	LONG dueIndex = (dwDue+99)/100; // 100ms ������ �ø�.
	if (dueIndex > 100)
		dueIndex = 100;
	::InterlockedIncrement(g_TimerCountDueRem+dueIndex);

	if (dwPeriod)
	{			
		LONG dueIndex = (dwPeriod+99)/100; // 100ms ������ �ø�.
		if (dueIndex > 100)
			dueIndex = 100;
		::InterlockedIncrement(g_TimerCountPerRem+dueIndex);
	}
}

void AddTimerLogging(DWORD dwDue, DWORD dwPeriod)
{
	LONG dueIndex = (dwDue+99)/100; // 100ms ������ �ø�.
	if (dueIndex > 100)
		dueIndex = 100;
	::InterlockedIncrement(g_TimerCountDueAdd+dueIndex);

	if (dwPeriod)
	{			
		LONG dueIndex = (dwPeriod+99)/100; // 100ms ������ �ø�.
		if (dueIndex > 100)
			dueIndex = 100;
		::InterlockedIncrement(g_TimerCountPerAdd+dueIndex);
	}

	long now = (long) time(NULL);	
	if (now < g_TimerNumLoggingTime + 600) // �α����� 10���� �̳��� ����
		return;

	long prev = ::InterlockedExchange(&g_TimerNumLoggingTime, now); // ���ο� ������ ����
	if (now >= prev + 600)  
	{
		//�α����� 10���� ���� ��� �α�

		char szLog[101*15];

		char *pos = szLog; *pos = 0x00;
		for(int i = 0; i < 101 ; i++)
		{
			if (g_TimerCountDueAdd[i])
			{
				sprintf(pos,"%d:%d,",i,g_TimerCountDueAdd[i]);
				pos += strlen(pos);
			}					
		}
		theLog.Put(INF_UK, "NGS_RoomTimer_Inf, g_TimerCountDueAdd: ", szLog);

		pos = szLog; *pos = 0x00;
		for(int i = 0; i < 101 ; i++)
		{
			if (g_TimerCountDueRem[i])
			{
				sprintf(pos,"%d:%d,",i,g_TimerCountDueRem[i]);
				pos += strlen(pos);
			}

		}						
		theLog.Put(INF_UK, "NGS_RoomTimer_Inf, g_TimerCountDueRem: ", szLog);

		pos = szLog; *pos = 0x00;
		for(int i = 0; i < 101 ; i++)
		{
			if (g_TimerCountDueAdd[i])
			{
				sprintf(pos,"%d:%d,",i,g_TimerCountDueAdd[i] - g_TimerCountDueRem[i]);
				pos += strlen(pos);
			}

		}						
		theLog.Put(INF_UK, "NGS_RoomTimer_Inf, g_TimerCountDueAdd - g_TimerCountDueRem: ", szLog);

		pos = szLog; *pos = 0x00;
		for(int i = 0; i < 101 ; i++)
		{
			if (g_TimerCountPerAdd[i])
			{
				sprintf(pos,"%d:%d,",i,g_TimerCountPerAdd[i]);
				pos += strlen(pos);
			}					
		}
		theLog.Put(INF_UK, "NGS_RoomTimer_Inf, g_TimerCountPerAdd: ", szLog);

		pos = szLog; *pos = 0x00;
		for(int i = 0; i < 101 ; i++)
		{
			if (g_TimerCountPerRem[i])
			{
				sprintf(pos,"%d:%d,",i,g_TimerCountPerRem[i]);
				pos += strlen(pos);
			}

		}						
		theLog.Put(INF_UK, "NGS_RoomTimer_Inf, g_TimerCountPerRem: ", szLog);

		pos = szLog; *pos = 0x00;
		for(int i = 0; i < 101 ; i++)
		{
			if (g_TimerCountPerAdd[i])
			{
				sprintf(pos,"%d:%d,",i,g_TimerCountPerAdd[i] - g_TimerCountPerRem[i]);
				pos += strlen(pos);
			}

		}						
		theLog.Put(INF_UK, "NGS_RoomTimer_Inf, g_TimerCountPerAdd - g_TimerCountPerRem: ", szLog);
	}
}




LONG CRoomTimerManager::AddTimer(DWORD dwDue, DWORD dwPeriod, LONG lTimerIndex )
{
	if( NULL == m_pTimerHandler )
		return -999;	//Invalid Index?

	//0.1 �� �������� �÷� ����.
	dwDue	 = (dwDue+99)/100*100;
	dwPeriod = (dwPeriod+99)/100*100;	



#pragma oMSG("TODO : ThreadSafe check!")
	//TLock lo(this);
	//CAutoLockCheck lc("CRoom::AddTimer", &m_lock_timer_debug);


	//static LONG lIndex = 0;
	LONG lIndex = 0;
	if (lTimerIndex >= 0) {
		ForEachElmt(TimerList, m_lstTimer, i, j) {
			if ((*i).m_lIndex == lTimerIndex) {
				DWORD lifeTime   = ::GetTickCount() - (*i).m_addTimerTime;
				theLog.Put(WAR_UK, "NGS_RoomTimer_Error,Already exist index: ", lTimerIndex, " in AddTimer. RoomID:", roomIdStr, ", m_lstTimer.size():", m_lstTimer.size(), ", the Timer's lifetime=", lifeTime);
				return -1;
			}
		}
		lIndex = lTimerIndex;
	} else {		
		ForEachElmt(TimerList, m_lstTimer, i, j) {
			if ((*i).m_lIndex > lIndex) lIndex = (*i).m_lIndex;
		}
	}
	TimerStruct ts;
	memset(&ts, 0x00, sizeof(ts));
	ts.m_pTimer = new GXSigTimer();	

	if (!ts.m_pTimer) {
		theLog.Put(WAR_UK, "NGS_RoomTimer_Error,Can't Make Timer. index: ", lTimerIndex, " in AddTimer. RoomID:", roomIdStr, ", m_lstTimer.size():", m_lstTimer.size());
		return -1;
	}
	if (lTimerIndex < 0) {
		ts.m_lIndex = ++lIndex;
	} else {
		ts.m_lIndex = lTimerIndex;
	}

	if (FALSE == ts.m_pTimer->Activate(GetThreadPool(), m_pTimerHandler, dwDue, dwPeriod))
	{
		theLog.Put(WAR_UK, "NGS_RoomTimer_Error,Can't Activate Timer.  index: ", lTimerIndex, " in AddTimer. RoomID:", roomIdStr, ", m_lstTimer.size():", m_lstTimer.size());
		return -1;
	}


	{	// Ÿ�̸� �����
		ts.m_prevTime = ::GetTickCount();
		ts.m_dueTime   = dwDue;
		ts.m_periodTime = dwPeriod;
		ts.m_addTimerTime = ts.m_prevTime;

		// Ÿ�̸� ��� �����			
		AddTimerLogging(dwDue, dwPeriod);
	}

	m_lstTimer.push_back(ts);
	return ts.m_lIndex;
}

LONG CRoomTimerManager::RemoveTimer(LONG lIndex)
{
	//TLock lo(this);
	//CAutoLockCheck lc("CRoom::RemoveTimer", &m_lock_timer_debug);

	ForEachElmt(TimerList, m_lstTimer, i, j) {
		if ((*i).m_lIndex == lIndex) {
			break;
		}
	}
	if (i == m_lstTimer.end())
	{
		//theLog.Put(WAR_UK, "NGS_RoomTimer_Error,RemoveTimer(). invalid_index:", lIndex, ", RoomID:", roomIdStr ); ������ ��� RemoveTimer�� ����(?) �ϱ� ������ ������ �αװ� ����.
		return -1;
	}

	HANDLE hObj = (*i).m_pTimer->GetHandle();	
	if (FALSE == (*i).m_pTimer->Deactivate())
		theLog.Put(WAR_UK, "NGS_RoomTimer_Error,RemoveTm()\t Deactivate Fail!. index:", lIndex, ", RoomID:", roomIdStr );
	else // Ÿ�̸� ��� �����		
		RemTimerLogging((*i).m_dueTime, (*i).m_periodTime);

	delete (*i).m_pTimer;	
	{	// Ÿ�̸� �����.
		DWORD now = ::GetTickCount();
		if ((*i).m_debug_count == 0)
		{  // Ÿ�̸Ӱ� ������ �ʰ� Remove ��.
			if( now  > (*i).m_prevTime + (*i).m_dueTime + 10000) // 10���� ������ ��
			{
				DWORD during	 = now -(*i).m_prevTime;
				DWORD lifeTime   = now - (*i).m_addTimerTime; // Ÿ�̸Ӱ� Active �� �� Remove �� �������� �ð�
				theLog.Put(WAR_UK, "NGS_RoomTimer_Error,RemoveTm()\t during :", during, ",\t dueTime:", (*i).m_dueTime, ",\t lifeTime:", lifeTime , ",\t RoomID:", roomIdStr,", index:", (*i).m_lIndex, ", hObj:", hObj);
			}


		}
		else if (((*i).m_debug_count == 1) && ((*i).m_periodTime ==0))
		{
			// one-shot Ÿ�̸ӿ��� Ÿ�̸Ӱ� �ѹ� �������Ƿ� ������ äŷ���� �ʴ´�.
		}
		else
		{  // �ֱ������� ������ Ÿ�̸� �̹Ƿ� �Ʒ��� ���� üŷ.
			if( now >  (*i).m_prevTime + (*i).m_periodTime + 10000) // 10���� ������ ��
			{
				DWORD during	 = now -(*i).m_prevTime;
				DWORD lifeTime   = now - (*i).m_addTimerTime; // Ÿ�̸Ӱ� Active �� �� Remove �� �������� �ð�
				theLog.Put(WAR_UK, "NGS_RoomTimer_Error,RemoveTm()\t during :", during, ",\t m_period:", (*i).m_periodTime, ",\t lifeTime:", lifeTime, ",\t count:", (*i).m_debug_count,", RoomID:", roomIdStr, ", index:", (*i).m_lIndex, ", hObj:", hObj);				
			}
		}
	}
	m_lstTimer.erase(i);
	return 0;
}

LONG CRoomTimerManager::RemoveAllTimer()
{
	//TLock lo(this);
	//CAutoLockCheck lc("CRoom::RemoveAllTimer", &m_lock_timer_debug);

	ForEachElmt(TimerList, m_lstTimer, i, j) {
		HANDLE hObj = (*i).m_pTimer->GetHandle();				     
		if (FALSE == (*i).m_pTimer->Deactivate())
			theLog.Put(WAR_UK, "NGS_RoomTimer_Error,RemoveAl()\t Deactivate Fail!. index:", (*i).m_lIndex, ", RoomID:", roomIdStr );
		else
		{ 
			// Ÿ�̸� ��� �����		
			RemTimerLogging((*i).m_dueTime, (*i).m_periodTime);
		}
		delete (*i).m_pTimer;
		{	// Ÿ�̸� �����.
			DWORD now = ::GetTickCount();
			if ((*i).m_debug_count == 0) 
			{ // Ÿ�̸Ӱ� ������ �ʰ� Remove ��.
				if( now > (*i).m_prevTime + (*i).m_dueTime + 10000) // 10���� ������ ��
				{
					DWORD during	 = now -(*i).m_prevTime;
					DWORD lifeTime   = now - (*i).m_addTimerTime; // Ÿ�̸Ӱ� Active �� �� Remove �� �������� �ð�					
					theLog.Put(WAR_UK, "NGS_RoomTimer_Error,RemoveAll()\t during :", during, ",\t dueTime:", (*i).m_dueTime, ",\t lifeTime:", lifeTime , ",\t RoomID:", roomIdStr,", index:", (*i).m_lIndex, ", hObj:", hObj);
				}
			}
			else if (((*i).m_debug_count == 1) && ((*i).m_periodTime == 0))
			{
				// one-shot Ÿ�̸ӿ��� Ÿ�̸Ӱ� �ѹ� �������Ƿ� ������ äŷ���� �ʴ´�.
			}
			else
			{  // �ֱ������� ������ Ÿ�̸� �̹Ƿ� �Ʒ��� ���� üŷ.
				if( now > (*i).m_prevTime + (*i).m_periodTime  + 10000) // 10���� ������ ��
				{					
					DWORD during	 = now -(*i).m_prevTime;
					DWORD lifeTime   = now - (*i).m_addTimerTime; // Ÿ�̸Ӱ� Active �� �� Remove �� �������� �ð�
					theLog.Put(WAR_UK, "NGS_RoomTimer_Error,RemoveAll()\t during :", during, ",\t m_period:", (*i).m_periodTime, ",\t lifeTime:", lifeTime, ",\t count:", (*i).m_debug_count,", RoomID:", roomIdStr, ", index:", (*i).m_lIndex, ", hObj:", hObj);				

				}
			}
		}

	}
	m_lstTimer.clear();
	return 0;
}


BOOL CRoomTimerManager::ProcessTimerSignal( LONG & UserTimerIndex, const HSIGNAL &hObj, const WPARAM &wParam, const LPARAM &lParam, DWORD queueTime )
{

	CAutoLockCheck lc("CRoom::OnSignal", &m_lock_timer_debug);

	//		BOOL bTimer = FALSE;
	ForEachElmt(TimerList, m_lstTimer, i, j) 
	{
		if ((*i).m_pTimer->IsHandle(hObj)) 
		{

			{  // Ÿ�̸� �����.
				//  GRC�� OnTimer���� RemoveTimer()�� ȣ���� �� �־ ���� �ø�. �׷� ��� (*i) ���� invalid ����.
				DWORD now = ::GetTickCount();
				if ((*i).m_debug_count == 0)
				{
					if( now > (*i).m_prevTime + (*i).m_dueTime + 10000) // 10���� ������ ��
					{
						DWORD during = now -(*i).m_prevTime;
						theLog.Put(WAR_UK, "NGS_RoomTimer_Error, OnSignal()\t during :", during, ",\t dueTime:", (*i).m_dueTime, ",\t queueTime:", queueTime, ", RoomID:", roomIdStr, ", index:", (*i).m_lIndex, ", hObj:", hObj);
					}
				}					
				else
				{
					if( now > (*i).m_prevTime + (*i).m_periodTime + 10000) // 10���� ������ ��
					{
						DWORD during = now -(*i).m_prevTime;
						theLog.Put(WAR_UK, "NGS_RoomTimer_Error, OnSignal()\t during :", during, ",\t m_period:", (*i).m_periodTime, ",\t queueTime:", queueTime, ", count:", (*i).m_debug_count,", RoomID:", roomIdStr, ", index:", (*i).m_lIndex, ", hObj:", hObj);
					}
				}
				(*i).m_debug_count++;
				(*i).m_prevTime = now;
			}

			UserTimerIndex = (*i).m_lIndex;
			return true;
		}
	}

	return false;
}
