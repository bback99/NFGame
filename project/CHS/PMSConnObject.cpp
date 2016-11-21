#include "StdAfx.h"

#include <PMSConnObject.h>

#include "Common.h"
#include "CHSInfoDir.h"
#include "StatisticsTable.h"
#include "ChannelDir.h"
#include "LRBHandler.h"


CPMSConnector thePMSConnector;

////////////////////////////////////////////////////////////////////////////////////////////////
// 필수 구현 interface 
// PMSA가 내부적으로 응답 메세지를 생성, 전송함.
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPMSConnector::OnRegionInfoReq(IPMSRegionInfoList* plstRegionInfo)
{
	StatInfoList lstStat;
	theStatTable.GetStatInfo(lstStat);

	printf("OnRecv OnRegionInfoReq\n");
		
	for(StatInfoList::iterator iter = lstStat.begin() ; iter != lstStat.end() ; iter++)
	{
		StatisticInfo& si = *iter;

		VLONG vecInsert;
		vecInsert.insert(vecInsert.begin(), si.m_vecStatistics.begin(), si.m_vecStatistics.end());

		plstRegionInfo->AddRegionInfo( si.m_lSSN, 0, &vecInsert );
	}

	return TRUE;
}

BOOL CPMSConnector::OnStatInfoReq(IPMSStatInfoList* plstStatInfo)
{ 
	
	typedef pair< long, long > PAIR_LONG;
	PAIR_LONG pairLong;

	ChannelUpdateInfoList lst;
 	DWORD dwCount = theCHSInfoDir.GetCHSInfoListForPMS(lst, TRUE);
#ifdef _PMS_CODE_SEPERATION_                         
	PMSStatInfoPerChannelingList pmsStatInfoList;
	dwCount += theStatTable.GetSessionStatInfoList(pmsStatInfoList);

	for(PMSStatInfoPerChannelingList::iterator itr = pmsStatInfoList.begin(); itr != pmsStatInfoList.end(); itr++)
	{
		ChannelUpdateInfo channelUpdateInfo;
		channelUpdateInfo.m_channelID = itr->m_channelID;
		channelUpdateInfo.m_lUserCount = itr->m_lUserCount;
		channelUpdateInfo.m_lWaitingUserCount = itr->m_lWaitingUserCount;
		lst.push_back(channelUpdateInfo);
	}
#endif
	if(dwCount > 0)
	{		
		map<PAIR_LONG, PMSStatBase> mapPMSStat;
		for(ChannelUpdateInfoList::iterator iter = lst.begin(); iter != lst.end() ; iter++)
		{
			ChannelUpdateInfo& cui = *iter;
			pairLong.first = cui.m_channelID.m_lSSN;
			pairLong.second = cui.m_channelID.m_dwCategory;
			map< PAIR_LONG, PMSStatBase>::iterator it = mapPMSStat.find(pairLong);
        
			if(it == mapPMSStat.end())
			{
				// GSID = 0으로 Setting
				PMSUnitID	unitID(cui.m_channelID.m_lSSN, 0, cui.m_channelID.m_dwCategory);
				PMSStatBase psb(unitID, cui.m_lWaitingUserCount, cui.m_lUserCount, 1, cui.m_lRoomCount, string(""));
				mapPMSStat[pairLong] = psb;
			}
			else
			{
				it->second.m_dwChannelCnt++;
				it->second.m_dwCU += cui.m_lWaitingUserCount;
				it->second.m_dwRoomCnt += cui.m_lRoomCount;
				it->second.m_dwSession += cui.m_lUserCount; 
			}
		}
		
		for(map<PAIR_LONG, PMSStatBase>::iterator iter1 = mapPMSStat.begin() ; iter1 != mapPMSStat.end() ; iter1++)
		{
			PAIR_LONG		pairInsert = (*iter1).first;
			PMSStatBase		sbInsert = (*iter1).second;

			// SSN, Catergory, CU, Session, ChannelCNT, RoomCNT, Option
			plstStatInfo->AddStatInfoList(pairInsert.first, pairInsert.second, sbInsert.m_dwCU, sbInsert.m_dwSession, sbInsert.m_dwChannelCnt, sbInsert.m_dwRoomCnt, sbInsert.m_sOptionInfo.c_str());
		}
	}	

	return TRUE;
}

BOOL CPMSConnector::OnPerformInfoReq(IPMSPerformanceInfo *pPerformanceInfo)
{ 
	printf("OnRecv On Perform Info Req Message :  \n");

	return TRUE;
}

BOOL CPMSConnector::OnAnnounceReq(DWORD dwSSN, DWORD dwCategoryID, LPCSTR lpszMsg)
{
	LONG lLength = strlen(lpszMsg);
	if (lLength < 0)
	{
		LOG(ERR_UK, "CPMSConnector"_LK, "OnAnnounceReq Msg size is Invalid. Size :", lLength);
		return FALSE;
	}

	xstring sMsg;
	sMsg.assign((LPCXSTR)lpszMsg, lLength);

	if(dwSSN == 0)
		theChannelDir.PMSAnnounceReqInSystem(sMsg);
	else if(dwCategoryID <= 0)
		theChannelDir.PMSAnnounceReqInSSN(dwSSN, sMsg);
	else
		theChannelDir.PMSAnnounceReqInCategory(dwSSN, dwCategoryID, sMsg);

	return TRUE; 
}

BOOL CPMSConnector::OnOrderReq(LPCSTR lpszCmdName, LPCSTR lpszCtlVal, LPSTR lpResult, LONG *lpResultLen, DWORD dwSSN, DWORD dwCategoryID)
{
	printf("OnRecv OnOrderReq Message : CmdName. %s \n", lpszCmdName);
	sprintf(lpResult, "SUCCESS %s ", lpszCmdName);
	*lpResultLen = (LONG)strlen(lpResult);

	return TRUE;
}

BOOL CPMSConnector::OnHeartbeatReq(LONG lSequenceNo)
{
	printf("OnRecv HeartBeatReq Message : SeqNo. %d \n", lSequenceNo);

	///////// 할꺼....
	// 요기 heart beat check.
	if(theLRBHandler.CheckStartUp())
		return TRUE;
	
	return FALSE;
}

BOOL CPMSConnector::OnRegionInfoPCReq(IPMSRegionInfoListPC* plstRegionInfoPC)
{
	StatInfoList lstStatPC;
	theStatTable.GetStatInfoPC(lstStatPC);

	printf("OnRecv OnRegionInfoPCReq\n");
		
	for(StatInfoList::iterator iter = lstStatPC.begin() ; iter != lstStatPC.end() ; iter++)
	{
		StatisticInfo& si = *iter;

		VLONG vecInsert;
		vecInsert.insert(vecInsert.begin(), si.m_vecStatistics.begin(), si.m_vecStatistics.end());

		plstRegionInfoPC->AddRegionInfoPC( si.m_lSSN, 0, &vecInsert );
	}

	return TRUE;
}

BOOL CPMSConnector::OnStatInfoPCReq(IPMSStatInfoListPC* plstStatInfoPC)
{
	typedef pair< long, long > PAIR_LONG;
	PAIR_LONG pairLong;

	printf("OnRecv OnStatInfoPCReq\n");

	ChannelUpdateInfoPCRoomList lst;
 	DWORD dwCount = theCHSInfoDir.GetCHSInfoListForPMS(lst, TRUE);
#ifdef _PMS_CODE_SEPERATION_
	PMSStatInfoPerChannelingList pmsStatInfoList;
	dwCount += theStatTable.GetSessionStatInfoList(pmsStatInfoList);

	for(PMSStatInfoPerChannelingList::iterator itr = pmsStatInfoList.begin(); itr != pmsStatInfoList.end(); itr++)
	{
		ChannelUpdateInfoPCRoom channelUpdateInfo;
		channelUpdateInfo.m_channelID = itr->m_channelID;
		channelUpdateInfo.m_lUserCount = itr->m_lUserCount;
		lst.push_back(channelUpdateInfo);
	}
#endif

	if(dwCount > 0)
	{	
		map<PAIR_LONG, PMSStatBase> mapPMSStat;
		for(ChannelUpdateInfoPCRoomList::iterator iter = lst.begin(); iter != lst.end() ; iter++)
		{
			ChannelUpdateInfoPCRoom& cui = *iter;
			pairLong.first = cui.m_channelID.m_lSSN;
			pairLong.second = cui.m_channelID.m_dwCategory;
			map< PAIR_LONG, PMSStatBase>::iterator it = mapPMSStat.find(pairLong);
			if(it == mapPMSStat.end())
			{
				// GSID = 0으로 Setting
				PMSUnitID	unitID(cui.m_channelID.m_lSSN, 0, cui.m_channelID.m_dwCategory);
				PMSStatBase psb(unitID, cui.m_lPCRoomUserCount, cui.m_lUserCount, 1, cui.m_lRoomCount, string(""));
				mapPMSStat[pairLong] = psb;
			}
			else
			{
				it->second.m_dwChannelCnt++;
				it->second.m_dwCU += cui.m_lPCRoomUserCount;
				it->second.m_dwRoomCnt += cui.m_lRoomCount;
				it->second.m_dwSession += cui.m_lUserCount; 
			}
		}
		
		for(map<PAIR_LONG, PMSStatBase>::iterator iter1 = mapPMSStat.begin() ; iter1 != mapPMSStat.end() ; iter1++)
		{
			PAIR_LONG		pairInsert = (*iter1).first;
			PMSStatBase		sbInsert = (*iter1).second;

			// SSN, Catergory, PC_CU, Session, ChannelCNT, RoomCNT, Option
			plstStatInfoPC->AddStatInfoListPC(pairInsert.first, pairInsert.second, sbInsert.m_dwCU, sbInsert.m_dwSession, sbInsert.m_dwChannelCnt, sbInsert.m_dwRoomCnt, sbInsert.m_sOptionInfo.c_str());
		}
	}	

	return TRUE;
}

