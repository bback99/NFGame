#include "stdafx.h"

#ifdef _USE_STATMSG

#include ".\statmsgmgr.h"
#include <direct.h>
#include <time.h>

CStatMsgMgr theStatMsgMgr;

CStatMsgMgr::CStatMsgMgr(void)
{
	int nRet = _tmkdir(_T("C:\\LOG"));
	if(nRet != 0) printf("!!! _mkdir() Error! : nRet = %d\n", nRet);
	SYSTEMTIME sys;
	memset(&sys, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&sys);
	memset(m_path, 0, sizeof(m_path));
	sprintf(m_path, "c:\\LOG\\Stat_LOG%04d%02d%02d_%02d%02d.txt", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute);

	_time64( &m_dwCheckTime );
	m_nCheckCnt = 0;
	_time64( &m_dwLogTime );
	m_nLogCnt = 0;

	m_nLogCnt_Data = LOG_COUNT;
	m_dwLogTime_Data = LOG_TIME_SEC;

	m_nCheckCnt_Data = CHECK_COUNT;
	m_dwCheckTime_Data = CHECK_TIME_SEC;
	m_dwCheckTimeGap_Data = CHECK_TIME_GAP;

	m_nModuler = 7;

	LOG("###################### Statistic Msg Manager OPEN !!! ######################");
	LOG("############### [Default]  Log : Cnt/Time = [%d/%d], Check Cnt/Time/Gap = [%d/%d/%d]", \
		m_nLogCnt_Data, m_dwLogTime_Data, m_nCheckCnt_Data, m_dwCheckTime_Data, m_dwCheckTimeGap_Data);
	LOG("############### [Default] SetModulerValue = [%d]", m_nModuler);
}

CStatMsgMgr::~CStatMsgMgr(void)
{
}

void CStatMsgMgr::InitData(int nLogCnt, DWORD dwLogTime, int nCheckCnt, DWORD dwCheckTime, DWORD dwCheckTimeGap)
{
	m_nLogCnt_Data = nLogCnt;
	m_dwLogTime_Data = dwLogTime;

	m_nCheckCnt_Data = nCheckCnt;
	m_dwCheckTime_Data = dwCheckTime;
	m_dwCheckTimeGap_Data = dwCheckTimeGap;

	LOG("############### [Set] Log : Cnt/Time = [%d/%d], Check Cnt/Time/Gap = [%d/%d/%d]", \
		nLogCnt, dwLogTime, nCheckCnt, dwCheckTime, dwCheckTimeGap);
}

BOOL CStatMsgMgr::SetModulerValue(int nData) 
{ 
	m_nModuler = nData; 
	LOG("############### [Set] SetModulerValue = [%d]", m_nModuler);
	return TRUE; 
}

BOOL CStatMsgMgr::EnterUserTime(long lUSN, long lType)
{
	TLock lo(this);
	if(lUSN <= 0) return FALSE;

	DWORD dwTime = ::GetTickCount();
	MAP_USERMSGTIME::iterator iter = m_mapUserMsgTime.find(CUSNType(lUSN, lType));
	if(iter == m_mapUserMsgTime.end()) {
		list<DWORD> lstLong;
		lstLong.push_back(dwTime);
		m_mapUserMsgTime[CUSNType(lUSN, lType)] = lstLong;
	}
	else
		iter->second.push_back(dwTime);
		 
	return TRUE;
}

BOOL CStatMsgMgr::LeaveUserTime(long lUSN, long lType)
{
	TLock lo(this);
	if(lUSN <= 0) return FALSE;

	MAP_USERMSGTIME::iterator iter = m_mapUserMsgTime.find(CUSNType(lUSN, lType));
	if(iter != m_mapUserMsgTime.end()) {
		if(iter->second.empty()) {
			m_mapUserMsgTime.erase(CUSNType(lUSN, lType));
			if(lUSN%(m_nModuler*5) == 0) LOG("list is empty : [USN/Type] = [%d/%d]", lUSN, lType);
			return FALSE;
		}
		else {
			DWORD dwTime = ::GetTickCount();
			DWORD dwNewTime = dwTime - iter->second.front();
			if(dwNewTime >= 0)				// 0으로 다시 돌아올때 이때는 update 무시한다.
				UpdateMsgTime(iter->first.m_lType, dwNewTime);
							
			if(iter->second.size() <= 1)	m_mapUserMsgTime.erase(CUSNType(lUSN, lType));
			else							iter->second.pop_front();
		}
	}
	else
	{
		if(lUSN%(m_nModuler*5) == 0)
			LOG("map is empty : [%d]", lType);
	}

	//////////// check for missing msg ////////////
	if(m_nCheckCnt++ >= m_nCheckCnt_Data)
	{
		__time64_t ltime;
		_time64( &ltime );
		if(ltime - m_dwCheckTime_Data > m_dwCheckTime)
		{
			DWORD dwTick = GetTickCount();
			ForEachElmt(MAP_USERMSGTIME, m_mapUserMsgTime, iter1, iter2)
			{
				if(iter1->second.size() < 1) {
					if((iter1->first.m_lUSN)%(m_nModuler*5) == 0)
						LOG("MAP_USERMSGTIME is empty : MSG Type = %d, USN = %d", iter1->first.m_lType, iter1->first.m_lUSN);
                    m_mapUserMsgTime.erase(iter1);
					continue;
				}
				else {
					ForEachElmt(list<DWORD>, iter1->second, it1, it2) {
						if(dwTick - m_dwCheckTimeGap_Data > *it1)
						{
							if((iter1->first.m_lUSN)%(m_nModuler*5) == 0)
								LOG("MAP_USERMSGTIME is Timeover : MSG Type = %d, USN = %d", iter1->first.m_lType, iter1->first.m_lUSN);
							iter1->second.erase(it1);
						}
					}
					if(iter1->second.size() < 1) m_mapUserMsgTime.erase(iter1);
				}
			}
			m_dwCheckTime = ltime;
		}
		m_nCheckCnt = 0;
	}
	////////////////////////////////////////////////

	return TRUE;
}

BOOL CStatMsgMgr::DeleteUserTime(long lUSN, long lType)
{
	TLock lo(this);
	if(lUSN <= 0) return FALSE;

	MAP_USERMSGTIME::iterator iter = m_mapUserMsgTime.find(CUSNType(lUSN, lType));
	if(iter != m_mapUserMsgTime.end()) {
		if(iter->second.empty()) {
			m_mapUserMsgTime.erase(CUSNType(lUSN, lType));
			if(lUSN%(m_nModuler*5) == 0) LOG("list is empty : [%d]", lType);
			return FALSE;
		}
		else {
			map<long, long>::iterator itloss = m_mapLossMsg.find(lType);
			if(itloss == m_mapLossMsg.end())	m_mapLossMsg[lType] = 1;
			else								m_mapLossMsg[lType] += 1;

			if(iter->second.size() <= 1)	m_mapUserMsgTime.erase(CUSNType(lUSN, lType));
			else							iter->second.pop_front();
		}
	}
	else {
		if(lUSN%(m_nModuler*5) == 0) LOG("map is empty : [%d]", lType);
	}
	return TRUE;
}

BOOL CStatMsgMgr::UpdateMsgTime(long lType, DWORD dwTime)
{
	__time64_t ltime;
	_time64( &ltime );

	MAP_MSGTIME::iterator iter = m_mapMsgTime.find(lType);
	if(iter == m_mapMsgTime.end())
		m_mapMsgTime[lType] = CMsgTimeData(dwTime, dwTime, dwTime, 1);
	else {		
		CMsgTimeData& mtData = iter->second;		
		mtData.m_dwTotalTime += dwTime;
		mtData.m_lCount++;
		if(mtData.m_dwMaxTime < dwTime) mtData.m_dwMaxTime = dwTime;
		if(mtData.m_dwMinTime > dwTime) mtData.m_dwMinTime = dwTime;		
	}

	////////// log print ////////////
	if(++m_nLogCnt >= m_nLogCnt_Data || ltime - m_dwLogTime_Data > m_dwLogTime) {					
		for(MAP_MSGTIME::iterator itData = m_mapMsgTime.begin() ; itData != m_mapMsgTime.end() ; itData++) {
			CMsgTimeData& mtData = itData->second;
			if(m_nLogCnt >= m_nLogCnt_Data)
				LOG("COUNT > %d \t: MsgType/Ave/Max/Min/Cnt = [%7d]/[%7d]/[%7d]/[%7d]/[%7d]", LOG_COUNT, long(itData->first), \
					mtData.m_lCount>0 ? mtData.m_dwTotalTime/mtData.m_lCount : 0, mtData.m_dwMaxTime, \
					mtData.m_lCount>0 ? mtData.m_dwMinTime : 0, mtData.m_lCount);
			else
				LOG("Update Time \t: MsgType/Ave/Max/Min/Cnt = [%7d]/[%7d]/[%7d]/[%7d]/[%7d]", long(itData->first), \
					mtData.m_lCount>0 ? mtData.m_dwTotalTime/mtData.m_lCount : 0, mtData.m_dwMaxTime, \
					mtData.m_lCount>0 ? mtData.m_dwMinTime : 0, mtData.m_lCount);
			mtData = CMsgTimeData(0, 0, 10000, 0);
		}	
		for(map<long, long>::iterator itloss = m_mapLossMsg.begin() ; itloss != m_mapLossMsg.end() ; itloss++)
		{
			LOG("LOSS MSG = [lType/Count] : [%d/%d]", itloss->first, itloss->second);
			itloss->second = 0;
		}
		LOG("=============================================================================================");
		m_nLogCnt = 0;
		m_dwLogTime = ltime;
	}
	/////////////////////////////////

	return TRUE;
}

void __cdecl CStatMsgMgr::LOG(LPCTSTR fmt,...)
{
	char buf[1024] = {0, };
	va_list vl;
	va_start(vl, fmt);
	_vsnprintf(buf, 1024, fmt, vl);
	buf[1024-1] = 0;
	va_end(vl);

	LogFile(buf);
}

BOOL CStatMsgMgr::LogFile(string strLog)
{
	TLock lo(this);
	FILE * fh = fopen(m_path, "a" );
	if(fh == NULL) {
		return FALSE;
	}

	SYSTEMTIME sys;
	memset(&sys, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&sys);

	fprintf(fh, "STAT[%02d_%02d:%02d:%02d:%03d] = %s\n", sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, strLog.c_str());
	fclose( fh );
	return TRUE;
}

#endif