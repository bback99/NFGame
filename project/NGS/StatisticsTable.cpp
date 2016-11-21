//
// StatisticsTable.cpp
//
#include "stdafx.h"
#include "StatisticsTable.h"
#ifdef _NGSNLS
#include "room.h"
#endif

#ifdef __PMS_DEBUG__
#pragma oMSG("__PMS_DEBUG__ defined")
#endif

///////////////////////////////////////////////////////////////////////////////////
// CStatisticsTable

CStatisticsTable theStatTable;

CStatisticsTable::CStatisticsTable()
{
}

CStatisticsTable::~CStatisticsTable()
{
}

#ifdef _NGSNLS	
BOOL CStatisticsTable::SetStatInfo(RoomID &rid, UserStatInfo& info, LONG lDiff, int debug_type)
#else
BOOL CStatisticsTable::SetStatInfo(LONG lSSN, UserStatInfo& info, LONG lDiff)
#endif
{
	TLock lo(this);

#ifdef _NGSNLS
	LONG lSSN = rid.m_lSSN;
	{ 
		// CU 가 음수가 나오는 경우가 있어서 디버깅 코드 추가.

		UserMap::iterator it = 	m_mapSSNUser.find(lSSN);

		//SSN이 없을 경우
		if(it == m_mapSSNUser.end())
		{
			if(lDiff==1)
			{
				UsnCount* usnCount = new UsnCount;
				(*usnCount)[info.m_lUSN] = 1;
				m_mapSSNUser[lSSN] = usnCount;
				theLog.Put(INF_UK, "NGS_CUInf, new ssn registerd in m_mapSSNUser. SSN=", lSSN);
			}
			if(lDiff == -1) //등록되지 않은 SSN에 대한 사용자 삭제이기 때문에 Error
			{
				theLog.Put(WAR_UK, "NGS_CUWar,SetStatInfo() not exist user. RoomID=", RoomID2Str(rid), ", usn=", info.m_lUSN, ", debug_type=", debug_type);
				return FALSE;
			}
		}
		else
		{
			UsnCount& usnCount = *it->second;
			UsnCount::iterator iter = usnCount.find(info.m_lUSN);
			if(iter != usnCount.end()) //list에 USN 있을 경우
			{
				switch (lDiff)
				{
				case 1: // 사용자 추가인데 이미 usn이 들어 있다 Error
					usnCount[info.m_lUSN]++;
					theLog.Put(WAR_UK, "NGS_CUWar,SetStatInfo() already exist user. Count++:", usnCount[info.m_lUSN],", RoomID=", RoomID2Str(rid), ", usn=", info.m_lUSN, ", usnCountSize= ", usnCount.size(), ", debug_type=", debug_type);
					if (usnCount[info.m_lUSN] == 0)
						usnCount.erase(info.m_lUSN);
					return FALSE;
				case -1: // 있는 사용자 제거이므로 
					usnCount[info.m_lUSN]--;
					if (usnCount[info.m_lUSN] == 0) // 정상 사항이므로 맵에서 지우고 통계 정보 업데이트.
						usnCount.erase(info.m_lUSN);
					else // 이전에 +1 이 아닌 경우(두번 이상 Add 되어 있거나, Add 없이 Remove 되어 있는 경우) 일단 지우지 않고 카운트만 조절
					{
						theLog.Put(WAR_UK, "NGS_CUWar,SetStatInfo(). RemoveReq But Count is not Zero. Count--:", usnCount[info.m_lUSN], ", RoomID=", RoomID2Str(rid), ", usn=", info.m_lUSN, ", usnCountSize= ", usnCount.size(), ", debug_type=", debug_type);
						return FALSE;
					}
					break;
				default: // 현재 lDiff는 1,-1 로만 호출한다.
					theLog.Put(WAR_UK, "NGS_CUErr,SetStatInfo(). lDiff=", lDiff, " debug_type=", debug_type);
					break;
				}			
			}
			else //list에 USN이 없는 경우
			{
				switch (lDiff)
				{
				case -1: // 없는 usn에 대해 삭제 요청. Error
					usnCount[info.m_lUSN] = -1;
					theLog.Put(ERR_UK, "NGS_CUWar,SetStatInfo() RemoveReq But not exist user. RoomID=", RoomID2Str(rid), ", usn=", info.m_lUSN, ", usnCountSize= ", usnCount.size(), ", debug_type=", debug_type);
					return FALSE;
				case 1: // 없는 사용자 추가는 정상 사항이으므로 맵에 넣고 통계정보 업데이트.
					usnCount[info.m_lUSN] = 1;
					break;
				default: // 현재 lDiff는 1,-1 로만 호출한다.
					theLog.Put(WAR_UK, "NGS_CUErr, SetStatInfo(). lDiff=", lDiff, " debug_type=", debug_type);
					break;
				}
			}
		}
	}
#endif

	m_TotalStatInfo.m_lTotCount += lDiff;
	if (m_TotalStatInfo.m_lTotCount < 0) {
		theLog.Put(ERR_UK, "NGS_CUErr,SetStatInfo() m_TotalStatInfo.m_lTotCount=[", m_TotalStatInfo.m_lTotCount,"]");
		m_TotalStatInfo.m_lTotCount = 0;
	}

	StatisticInfo* pStatInfo;
	StatisticInfo statTemp;
	StatMap::iterator it = mStatMap.find(lSSN);
	if(it == mStatMap.end())
	{
		pStatInfo = &statTemp;
	}
	else
		pStatInfo = &(it->second);

	pStatInfo->m_lSSN = lSSN;
	pStatInfo->m_lTotCount += lDiff;

	switch(info.m_lJobIndex)
	{
	case JOB_ELEMSTUDENT:
		pStatInfo->m_lElemStudent += lDiff;
		m_TotalStatInfo.m_lElemStudent += lDiff;
		break;
	case JOB_MIDLSTUDENT:
		pStatInfo->m_lMidlStudent += lDiff;
		m_TotalStatInfo.m_lMidlStudent += lDiff;
		break;
	case JOB_HIGHSTUDENT:
		pStatInfo->m_lHighStudent += lDiff;
		m_TotalStatInfo.m_lHighStudent += lDiff;
		break;
	case JOB_UNIVSTUDENT:
		pStatInfo->m_lUnivStudent += lDiff;
		m_TotalStatInfo.m_lUnivStudent += lDiff;
		break;
	case JOB_HOUSEWIFE:
		pStatInfo->m_lHouseWife += lDiff;
		m_TotalStatInfo.m_lHouseWife += lDiff;
		break;
	case JOB_OFFICEWORK:
		pStatInfo->m_lOfficeWorker += lDiff;
		m_TotalStatInfo.m_lOfficeWorker += lDiff;
		break;
	case JOB_SELFEMPLOY:
		pStatInfo->m_lSelfEmployed += lDiff;
		m_TotalStatInfo.m_lSelfEmployed += lDiff;
		break;
	case JOB_UNEMPLOY:
		pStatInfo->m_lUnEmpl += lDiff;
		m_TotalStatInfo.m_lUnEmpl += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lAgeIndex)
	{
	case AGE_0TO13:
		pStatInfo->m_lAge0to13 += lDiff;
		m_TotalStatInfo.m_lAge0to13 += lDiff;
		break;
	case AGE_14TO16:
		pStatInfo->m_lAge14to16 += lDiff;
		m_TotalStatInfo.m_lAge14to16 += lDiff;
		break;
	case AGE_17TO19:
		pStatInfo->m_lAge17to19 += lDiff;
		m_TotalStatInfo.m_lAge17to19 += lDiff;
		break;
	case AGE_20TO24:
		pStatInfo->m_lAge20to24 += lDiff;
		m_TotalStatInfo.m_lAge20to24 += lDiff;
		break;
	case AGE_25TO29:
		pStatInfo->m_lAge25to29 += lDiff;
		m_TotalStatInfo.m_lAge25to29 += lDiff;
		break;
	case AGE_30TO39:
		pStatInfo->m_lAge30to39 += lDiff;
		m_TotalStatInfo.m_lAge30to39 += lDiff;
		break;
	case AGE_40TO99:
		pStatInfo->m_lAge40to99 += lDiff;
		m_TotalStatInfo.m_lAge40to99 += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lUSC)
	{
	case USC_MALE:
		pStatInfo->m_lMale += lDiff;
		m_TotalStatInfo.m_lMale += lDiff;
		break;
	case USC_FEMALE:
		pStatInfo->m_lFemale += lDiff;
		m_TotalStatInfo.m_lFemale += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lGuildIndex)
	{
	case GUILD_JOIN:
		pStatInfo->m_lGuild += lDiff;
		m_TotalStatInfo.m_lGuild += lDiff;
		break;
	case GUILD_NOTJOIN:
		pStatInfo->m_lNonGuild += lDiff;
		m_TotalStatInfo.m_lNonGuild += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lMemberIndex)
	{
	case MEMBERSHIP_JOIN:
		pStatInfo->m_lMembership += lDiff;
		m_TotalStatInfo.m_lMembership += lDiff;
		break;
	case MEMBERSHIP_NOTJOIN:
		pStatInfo->m_lNonMember += lDiff;
		m_TotalStatInfo.m_lNonMember += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lRegionIndex)
	{
	case REGION_KYUNGKI:
		pStatInfo->m_lKyungki += lDiff;
		m_TotalStatInfo.m_lKyungki += lDiff;
		break;
	case REGION_SEOUL:
		pStatInfo->m_lSeoul += lDiff;
		m_TotalStatInfo.m_lSeoul += lDiff;
		break;
	case REGION_INCHEON:
		pStatInfo->m_lIncheon += lDiff;
		m_TotalStatInfo.m_lIncheon += lDiff;
		break;
	case REGION_KANGWON:
		pStatInfo->m_lKangwon += lDiff;
		m_TotalStatInfo.m_lKangwon += lDiff;
		break;
	case REGION_CHOONGNAM:
		pStatInfo->m_lChoongnam += lDiff;
		m_TotalStatInfo.m_lChoongnam += lDiff;
		break;
	case REGION_CHOONGBUK:
		pStatInfo->m_lChoongbuk += lDiff;
		m_TotalStatInfo.m_lChoongbuk += lDiff;
		break;
	case REGION_DAEJEON:
		pStatInfo->m_lDaejeon += lDiff;
		m_TotalStatInfo.m_lDaejeon += lDiff;
		break;
	case REGION_KYUNGBUK:
		pStatInfo->m_lKyungbuk += lDiff;
		m_TotalStatInfo.m_lKyungbuk += lDiff;
		break;
	case REGION_KYUNGNAM:
		pStatInfo->m_lKyungnam += lDiff;
		m_TotalStatInfo.m_lKyungnam += lDiff;
		break;
	case REGION_DAEGU:
		pStatInfo->m_lDaegu += lDiff;
		m_TotalStatInfo.m_lDaegu += lDiff;
		break;
	case REGION_BUSAN:
		pStatInfo->m_lBusan += lDiff;
		m_TotalStatInfo.m_lBusan += lDiff;
		break;
	case REGION_ULSAN:
		pStatInfo->m_lUlsan += lDiff;
		m_TotalStatInfo.m_lUlsan += lDiff;
		break;
	case REGION_JUNBUK:
		pStatInfo->m_lJunbuk += lDiff;
		m_TotalStatInfo.m_lJunbuk += lDiff;
		break;
	case REGION_JUNNAM:
		pStatInfo->m_lJunnam += lDiff;
		m_TotalStatInfo.m_lJunnam += lDiff;
		break;
	case REGION_KWANGJU:
		pStatInfo->m_lKwangju += lDiff;
		m_TotalStatInfo.m_lKwangju += lDiff;
		break;
	case REGION_JEJU:
		pStatInfo->m_lJeju += lDiff;
		m_TotalStatInfo.m_lJeju += lDiff;
		break;
	case REGION_FOREIGN:
		pStatInfo->m_lForeign += lDiff;
		m_TotalStatInfo.m_lForeign += lDiff;
		break;
	default:
		break;
	}


	int nIdx = info.m_lUSC * NUM_OF_AGE * NUM_OF_REGION + (info.m_lAgeIndex - 1) * NUM_OF_REGION + (info.m_lRegionIndex - 1);
	if (nIdx < NUM_OF_AGE * NUM_OF_REGION * NUM_OF_USC  && nIdx >= 0)
	{
		pStatInfo->m_vecStatistics[nIdx] += lDiff;
		m_TotalStatInfo.m_vecStatistics[nIdx] += lDiff;
	}
	else
	{
		string sLogTemp = ::format("Invalid Index - USN: %ld, USEX: %ld, UAGE: %ld, UREGION: %ld", info.m_lUSN, info.m_lUSC, info.m_lAgeIndex, info.m_lRegionIndex);
		theLog.Put(WAR_UK, "NGS_Stat_Error"_COMMA, sLogTemp.c_str());
	}

	if(it == mStatMap.end())
	{
		mStatMap[lSSN] = statTemp;
		theLog.Put(INF_UK, "NGS_CUInf, new ssn registerd in mStatMap. SSN=", lSSN);
	}

#ifdef _NGSNLS
	// CU 디버그
	LONG *debug = (LONG*)&statTemp.m_lSSN;
	for (int i = 0; i < 40 ; i++)
		if (debug[i] < 0 )
			theLog.Put(ERR_UK, "NGS_CUErr,SetStatInfo() SSN=",lSSN,", index=",i,", value=[", debug[i],"]");
#endif


	return TRUE;
}

#ifdef _NGSNLS	
BOOL CStatisticsTable::SetStatInfoPC(RoomID &rid, UserStatInfo& info, LONG lDiff, int debug_type)
#else
BOOL CStatisticsTable::SetStatInfoPC(LONG lSSN, UserStatInfo& info, LONG lDiff)
#endif
{
	TLock lo(this);

#ifdef _NGSNLS	
	LONG lSSN = rid.m_lSSN;
#endif

	m_TotalStatInfoPC.m_lTotCount += lDiff;
	if (m_TotalStatInfoPC.m_lTotCount < 0) {
		theLog.Put(ERR_UK, "NGS_CUErr,SetStatInfo() m_TotalStatInfoPC.m_lTotCount=[", m_TotalStatInfoPC.m_lTotCount,"]");
		m_TotalStatInfoPC.m_lTotCount = 0;
	}

	StatisticInfo* pStatInfo;
	StatisticInfo statTemp;
	StatMap::iterator it = mStatMapPC.find(lSSN);
	if(it == mStatMapPC.end())
	{
		pStatInfo = &statTemp;
	}
	else
		pStatInfo = &(it->second);

	pStatInfo->m_lSSN = lSSN;
	pStatInfo->m_lTotCount += lDiff;

	switch(info.m_lJobIndex)
	{
	case JOB_ELEMSTUDENT:
		pStatInfo->m_lElemStudent += lDiff;
		m_TotalStatInfoPC.m_lElemStudent += lDiff;
		break;
	case JOB_MIDLSTUDENT:
		pStatInfo->m_lMidlStudent += lDiff;
		m_TotalStatInfoPC.m_lMidlStudent += lDiff;
		break;
	case JOB_HIGHSTUDENT:
		pStatInfo->m_lHighStudent += lDiff;
		m_TotalStatInfoPC.m_lHighStudent += lDiff;
		break;
	case JOB_UNIVSTUDENT:
		pStatInfo->m_lUnivStudent += lDiff;
		m_TotalStatInfoPC.m_lUnivStudent += lDiff;
		break;
	case JOB_HOUSEWIFE:
		pStatInfo->m_lHouseWife += lDiff;
		m_TotalStatInfoPC.m_lHouseWife += lDiff;
		break;
	case JOB_OFFICEWORK:
		pStatInfo->m_lOfficeWorker += lDiff;
		m_TotalStatInfoPC.m_lOfficeWorker += lDiff;
		break;
	case JOB_SELFEMPLOY:
		pStatInfo->m_lSelfEmployed += lDiff;
		m_TotalStatInfoPC.m_lSelfEmployed += lDiff;
		break;
	case JOB_UNEMPLOY:
		pStatInfo->m_lUnEmpl += lDiff;
		m_TotalStatInfoPC.m_lUnEmpl += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lAgeIndex)
	{
	case AGE_0TO13:
		pStatInfo->m_lAge0to13 += lDiff;
		m_TotalStatInfoPC.m_lAge0to13 += lDiff;
		break;
	case AGE_14TO16:
		pStatInfo->m_lAge14to16 += lDiff;
		m_TotalStatInfoPC.m_lAge14to16 += lDiff;
		break;
	case AGE_17TO19:
		pStatInfo->m_lAge17to19 += lDiff;
		m_TotalStatInfoPC.m_lAge17to19 += lDiff;
		break;
	case AGE_20TO24:
		pStatInfo->m_lAge20to24 += lDiff;
		m_TotalStatInfoPC.m_lAge20to24 += lDiff;
		break;
	case AGE_25TO29:
		pStatInfo->m_lAge25to29 += lDiff;
		m_TotalStatInfoPC.m_lAge25to29 += lDiff;
		break;
	case AGE_30TO39:
		pStatInfo->m_lAge30to39 += lDiff;
		m_TotalStatInfoPC.m_lAge30to39 += lDiff;
		break;
	case AGE_40TO99:
		pStatInfo->m_lAge40to99 += lDiff;
		m_TotalStatInfoPC.m_lAge40to99 += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lUSC)
	{
	case USC_MALE:
		pStatInfo->m_lMale += lDiff;
		m_TotalStatInfoPC.m_lMale += lDiff;
		break;
	case USC_FEMALE:
		pStatInfo->m_lFemale += lDiff;
		m_TotalStatInfoPC.m_lFemale += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lGuildIndex)
	{
	case GUILD_JOIN:
		pStatInfo->m_lGuild += lDiff;
		m_TotalStatInfoPC.m_lGuild += lDiff;
		break;
	case GUILD_NOTJOIN:
		pStatInfo->m_lNonGuild += lDiff;
		m_TotalStatInfoPC.m_lNonGuild += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lMemberIndex)
	{
	case MEMBERSHIP_JOIN:
		pStatInfo->m_lMembership += lDiff;
		m_TotalStatInfoPC.m_lMembership += lDiff;
		break;
	case MEMBERSHIP_NOTJOIN:
		pStatInfo->m_lNonMember += lDiff;
		m_TotalStatInfoPC.m_lNonMember += lDiff;
		break;
	default:
		break;
	}
	switch(info.m_lRegionIndex)
	{
	case REGION_KYUNGKI:
		pStatInfo->m_lKyungki += lDiff;
		m_TotalStatInfoPC.m_lKyungki += lDiff;
		break;
	case REGION_SEOUL:
		pStatInfo->m_lSeoul += lDiff;
		m_TotalStatInfoPC.m_lSeoul += lDiff;
		break;
	case REGION_INCHEON:
		pStatInfo->m_lIncheon += lDiff;
		m_TotalStatInfoPC.m_lIncheon += lDiff;
		break;
	case REGION_KANGWON:
		pStatInfo->m_lKangwon += lDiff;
		m_TotalStatInfoPC.m_lKangwon += lDiff;
		break;
	case REGION_CHOONGNAM:
		pStatInfo->m_lChoongnam += lDiff;
		m_TotalStatInfoPC.m_lChoongnam += lDiff;
		break;
	case REGION_CHOONGBUK:
		pStatInfo->m_lChoongbuk += lDiff;
		m_TotalStatInfoPC.m_lChoongbuk += lDiff;
		break;
	case REGION_DAEJEON:
		pStatInfo->m_lDaejeon += lDiff;
		m_TotalStatInfoPC.m_lDaejeon += lDiff;
		break;
	case REGION_KYUNGBUK:
		pStatInfo->m_lKyungbuk += lDiff;
		m_TotalStatInfoPC.m_lKyungbuk += lDiff;
		break;
	case REGION_KYUNGNAM:
		pStatInfo->m_lKyungnam += lDiff;
		m_TotalStatInfoPC.m_lKyungnam += lDiff;
		break;
	case REGION_DAEGU:
		pStatInfo->m_lDaegu += lDiff;
		m_TotalStatInfoPC.m_lDaegu += lDiff;
		break;
	case REGION_BUSAN:
		pStatInfo->m_lBusan += lDiff;
		m_TotalStatInfoPC.m_lBusan += lDiff;
		break;
	case REGION_ULSAN:
		pStatInfo->m_lUlsan += lDiff;
		m_TotalStatInfoPC.m_lUlsan += lDiff;
		break;
	case REGION_JUNBUK:
		pStatInfo->m_lJunbuk += lDiff;
		m_TotalStatInfoPC.m_lJunbuk += lDiff;
		break;
	case REGION_JUNNAM:
		pStatInfo->m_lJunnam += lDiff;
		m_TotalStatInfoPC.m_lJunnam += lDiff;
		break;
	case REGION_KWANGJU:
		pStatInfo->m_lKwangju += lDiff;
		m_TotalStatInfoPC.m_lKwangju += lDiff;
		break;
	case REGION_JEJU:
		pStatInfo->m_lJeju += lDiff;
		m_TotalStatInfoPC.m_lJeju += lDiff;
		break;
	case REGION_FOREIGN:
		pStatInfo->m_lForeign += lDiff;
		m_TotalStatInfoPC.m_lForeign += lDiff;
		break;
	default:
		break;
	}

	int nIdx = info.m_lUSC * NUM_OF_AGE * NUM_OF_REGION + (info.m_lAgeIndex - 1) * NUM_OF_REGION + (info.m_lRegionIndex - 1);
	if (nIdx < NUM_OF_AGE * NUM_OF_REGION * NUM_OF_USC  && nIdx >= 0)
	{
		pStatInfo->m_vecStatistics[nIdx] += lDiff;
		m_TotalStatInfoPC.m_vecStatistics[nIdx] += lDiff;
	}
	else
	{
		string sLogTemp = ::format("Invalid Index - USN: %ld, USEX: %ld, UAGE: %ld, UREGION: %ld", info.m_lUSN, info.m_lUSC, info.m_lAgeIndex, info.m_lRegionIndex);
		theLog.Put(WAR_UK, "NGS_Stat_Error"_COMMA, sLogTemp.c_str());
	}


	if(it == mStatMapPC.end())
	{
		mStatMapPC[lSSN] = statTemp;
		theLog.Put(INF_UK, "NGS_CUInf, new ssn registerd in mStatMap. SSN=", lSSN);
	}

#ifdef _NGSNLS
	// CU 디버그
	LONG *debug = (LONG*)&statTemp.m_lSSN;
	for (int i = 0; i < 40 ; i++)
		if (debug[i] < 0 )
			theLog.Put(ERR_UK, "NGS_CUErr,SetStatInfo() SSN=",lSSN,", index=",i,", value=[", debug[i],"]");
#endif
	return TRUE;
}

BOOL CStatisticsTable::GetStatInfo(StatInfoList& lstStat)
{
	TLock lo(this);

	lstStat.clear();
	lstStat.push_back(m_TotalStatInfo);
	ForEachElmt(StatMap, mStatMap, i, j) {
		lstStat.push_back(i->second);
	}
	return TRUE;
}

BOOL CStatisticsTable::GetStatInfoPC(StatInfoList& lstStatPC)
{
	TLock lo(this);

	lstStatPC.clear();
	lstStatPC.push_back(m_TotalStatInfoPC);
	ForEachElmt(StatMap, mStatMapPC, i, j) {
		lstStatPC.push_back(i->second);
	}
	return TRUE;
}

LONG CStatisticsTable::GetTotalCU()
{
	return m_TotalStatInfo.m_lTotCount;
}

LONG CStatisticsTable::GetTotalCUPC()
{
	return m_TotalStatInfoPC.m_lTotCount;
}
#ifdef _USE_MSGCOUNT
void CStatisticsTable::Init()
{
	m_sMyIP = GSocketUtil::GetHostIPString();
	m_timerSaveMsg.Activate(GetThreadPool(), this, 43200000, 3600000); // 12시간 이후에 1시간 마다
	//m_timerSaveMsg.Activate(GetThreadPool(), this, 60000, 10000); // test

}
STDMETHODIMP_(void) CStatisticsTable::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)	
{
	if(m_timerSaveMsg.IsHandle(hSignal))
	{
		SYSTEMTIME sys_time;
		::GetSystemTime(&sys_time);
		if ( sys_time.wDay >= 12 ) // 12일 이후부터는 DB 저장하지 않는다. 
		{
			m_timerSaveMsg.Deactivate();
			return; 
		}
		SaveMsgCount();
	}
}
void CStatisticsTable::AddMsgCount(LONG lSSN)
{
	TLock lo(this);
	LONG lCount = m_mapMsgCount[lSSN];
	m_mapMsgCount[lSSN] = ++lCount;
	theLog.Put(DEV_UK, "Msg Count Add. SSN=", lSSN, " Count=", m_mapMsgCount[lSSN]);
};

void CStatisticsTable::ClearMsgCount()
{
	TLock lo(this);
	m_mapMsgCount.clear();
	theLog.Put(DEV_UK, "Msg Count Flush");
};

void CStatisticsTable::SaveMsgCount()
{
	MsgCountMap mapMsgCountTemp;
	{
		TLock lo(this);
		mapMsgCountTemp = m_mapMsgCount;
		ClearMsgCount();
	}
	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	string sTime = ::format("%04d%02d%02d%02d", sys_time.wYear, sys_time.wMonth, sys_time.wDay, sys_time.wHour);

	ForEachElmt(MsgCountMap, mapMsgCountTemp, i, j)
	{
		string sQuery = ::format("GameDB|Q|%s('%s',%d,'%s',%d)", "insert into  research_channelrefresh values", sTime.c_str() ,i->first, m_sMyIP.c_str(), i->second); 

		DBGW_String strResult;
		int nResult;

		BOOL bRet = ExecuteQuery(1, sQuery.c_str(), &strResult, &nResult);

		if (!bRet)
			LOG(INF_UK, "CHS_RetrvJackpotMoney_Error"_LK, "gDBGW Query Failure: ", nResult, ", Query :", sQuery);
		else
			LOG(DEV_UK, "CHS_RetrvJackpotMoney_Success"_LK, "gDBGW Query Success: ", sQuery);
	}
}

#endif	// _USE_MSGCOUNT

#ifdef _PMS_CODE_SEPERATION_
void CStatisticsTable::UpdateSessionStatInfoDelta(const ChannelID& cid, const UserStatInfo& usi, LONG userCountDelta, LONG waitingUserCount)
{
	TLock lo(this);
	if(0 >= usi.m_lPMSCode) // 방어 코드. 채널링 PMS 코드가 포함 되어 있지 않은 사용자는 채널링 카운트에 포함하지 않고 리턴한다.
	{
		return;
	}
	ChannelID key_cid = cid;
	key_cid.m_dwGCIID = 0;
	key_cid.m_dwCategory += usi.m_lPMSCode;

	PMSStatInfoPerChannelingMap::iterator it = m_mapChannelingCUCount.find(key_cid);
	if(it == m_mapChannelingCUCount.end())
	{
		// GSID = 0으로 Setting
		PMSStatInfoPerChanneling statInfo;		
		statInfo.m_channelID = key_cid;
		m_mapChannelingCUCount[key_cid] = statInfo;
	}
	m_mapChannelingCUCount[key_cid].m_lUserCount += userCountDelta;
	m_mapChannelingCUCount[key_cid].m_lWaitingUserCount += waitingUserCount;
}

DWORD CStatisticsTable::GetSessionStatInfoList(PMSStatInfoPerChannelingList& channelingInfoList)
{
	TLock lo(this);
	DWORD dwCount = 0;
	channelingInfoList.clear();
	ForEachElmt(PMSStatInfoPerChannelingMap, m_mapChannelingCUCount, i, j)
	{
		channelingInfoList.push_back(i->second);
		dwCount++;
	}
	return dwCount;
}
#endif

