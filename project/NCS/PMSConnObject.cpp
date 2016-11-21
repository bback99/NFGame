#include "StdAfx.h"

#include <PMSConnObject.h>

#ifdef _USE_PMS
//#include "Common.h"
//#include "StatisticsTable.h"


CPMSConnector thePMSConnector;

////////////////////////////////////////////////////////////////////////////////////////////////
// 필수 구현 interface 
// PMSA가 내부적으로 응답 메세지를 생성, 전송함.
////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPMSConnector::OnRegionInfoReq(IPMSRegionInfoList* plstRegionInfo)
{
// 	StatInfoList lstStat;
// 	theStatTable.GetStatInfo(lstStat);
// 
// 	for(StatInfoList::iterator iter = lstStat.begin() ; iter != lstStat.end() ; iter++)
// 	{
// 		StatisticInfo& si = *iter;
// 
// 		VLONG vecInsert;
// 		vecInsert.insert(vecInsert.begin(), si.m_vecStatistics.begin(), si.m_vecStatistics.end());
// 		plstRegionInfo->AddRegionInfo(si.m_lSSN, 0, &vecInsert);
// 	}

	return TRUE;
}

BOOL CPMSConnector::OnStatInfoReq(IPMSStatInfoList* plstStatInfo)
{ 
// 	///////////////////////////////////////////////
// 	//  PMS 디버깅 코드
// #ifdef __PMS_DEBUG__
// 	static DWORD dwTime =::GetTickCount();
// 	BOOL  bIsLog = FALSE;
// 	if (::GetTickCount() - dwTime > 10*60*1000) // 10분
// 	{		
// 		bIsLog = TRUE;
// 		dwTime = ::GetTickCount();
// 	}
// 	else
// 		bIsLog = FALSE;	
// #endif
// 	///////////////////////////////////////////////
// 
// 	StatInfoList lstStat;
// 	theStatTable.GetStatInfo(lstStat);
// 
// 	RoomList lstRoom;
// 	theRoomTable.GetRoomList(lstRoom);
// 
// 	PayloadHA pld(PayloadHA::msgPMSStatInfoAns_Tag);
// 
// 	for (StatInfoList::iterator iter = lstStat.begin() ; iter != lstStat.end() ; iter++)
// 	{
// 		StatisticInfo& si = *iter;
// 
// 		if (si.m_lSSN != 0L)
// 		{
// 			PMSStatBase sinfo;
// 
// 			list<DWORD> lstCategory;
// 			lstCategory.clear();
// 
// 			sinfo.m_unitID.m_dwSSN = si.m_lSSN;
// 			sinfo.m_unitID.m_dwCategory = 0L;
// 			sinfo.m_dwCU = si.m_lTotCount;
// 			sinfo.m_dwSession = si.m_lTotCount;
// 			sinfo.m_dwChannelCnt = 0L;
// 
// 			LONG lRoomCount = 0L;
// 
// 			ForEachElmt(RoomList, lstRoom, i, j)
// 			{
// 				CRoom* pRoom = (*i);
// 				if (!pRoom) continue;
// 				if (si.m_lSSN == pRoom->GetRoomID().m_lSSN)
// 					lRoomCount++;
// 
// 				BOOL bIsIn = FALSE;
// 				for (list<DWORD>::iterator it = lstCategory.begin(); it != lstCategory.end(); it++)
// 				{
// 					if (*it == pRoom->GetRoomID().m_dwCategory)
// 						bIsIn = TRUE;
// 				}
// 				if (!bIsIn)
// 					lstCategory.push_back(pRoom->GetRoomID().m_dwCategory);
// 
// 				//pRoom->Release(); // lstStat의 size가 2 이상인 경우 두번 이상 Release()되어 유령방 발생했음.
// 			}
// 			sinfo.m_dwRoomCnt = lRoomCount;
// 			sinfo.m_sOptionInfo = "";
// 
// 			plstStatInfo->AddStatInfoList(sinfo.m_unitID.m_dwSSN, sinfo.m_unitID.m_dwCategory, sinfo.m_dwCU, sinfo.m_dwSession, sinfo.m_dwChannelCnt, sinfo.m_dwRoomCnt, sinfo.m_sOptionInfo.c_str());
// 
// 			///////////////////////////////////////////////
// 			//  PMS 디버깅 코드
// #ifdef __PMS_DEBUG__
// 			if (bIsLog == TRUE)
// 			{
// 				theLog.Put(INF_UK, "GLS_PMS_DEBUG, sinfo.m_dwRoomCnt:", sinfo.m_dwRoomCnt, 
// 					", sinfo.m_dwSession:", sinfo.m_dwSession,
// 					", sinfo.m_dwCU:",		sinfo.m_dwCU,
// 					", sinfo.m_dwSSN:",		sinfo.m_unitID.m_dwSSN,
// 					", sinfo.m_dwGSID:",	sinfo.m_unitID.m_dwGSID);
// 			}
// #endif
// 			///////////////////////////////////////////////
// 
// 
// 			for (list<DWORD>::iterator it2 = lstCategory.begin(); it2 != lstCategory.end(); it2++)
// 			{
// 				PMSStatBase sinfo2;
// 				sinfo2.m_unitID.m_dwSSN = si.m_lSSN;
// 				sinfo2.m_unitID.m_dwCategory = *it2;
// 				sinfo2.m_dwCU = 0L;
// 				sinfo2.m_dwSession = 0L;
// 				sinfo2.m_dwChannelCnt = 0L;
// 				sinfo2.m_dwRoomCnt = 0L;
// 				sinfo2.m_sOptionInfo = "";
// 
// 				plstStatInfo->AddStatInfoList(sinfo2.m_unitID.m_dwSSN, sinfo2.m_unitID.m_dwCategory, sinfo2.m_dwCU, sinfo2.m_dwSession, sinfo2.m_dwChannelCnt, sinfo2.m_dwRoomCnt, sinfo2.m_sOptionInfo.c_str());
// 			}
// 		}
// 	}
// 
// 
// 	ForEachElmt(RoomList, lstRoom, i, j)
// 	{
// 		CRoom* pRoom = (*i);
// 		if (pRoom)
// 			pRoom->Release();
// 	}

	return TRUE;
}

BOOL CPMSConnector::OnPerformInfoReq(IPMSPerformanceInfo *pPerformanceInfo)
{ 
	printf("OnRecv On Perform Info Req Message :  \n");

	return TRUE;
}

BOOL CPMSConnector::OnAnnounceReq(DWORD dwSSN, DWORD dwCategoryID, LPCSTR lpszMsg)
{
// 	LONG lLength = strlen(lpszMsg);
// 	if (lLength < 0)
// 	{
// 		theLog.Put(ERR_UK, "CPMSConnector"_COMMA, "OnAnnounceReq Msg size is Invalid. Size :", lLength);
// 		return FALSE;
// 	}
// 
// 	xstring sMsg;
// 	sMsg.assign((LPCXSTR)lpszMsg, lLength);
// 
// 	BOOL bRoomExist = FALSE;
// 
// 	RoomList lstRoom;
// 	lstRoom.clear();
// 
// 	theLog.Put(DEV, "CPMSConnector_OnAnnounceReq"_COMMA, "!!! Msg = [ ", sMsg, "], unitID.m_dwSSN/m_dwCategory=[", dwSSN, "/", dwCategoryID, "]");
// 
// 	if (dwSSN == 0)
// 		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_ALLSERVICE, lstRoom);
// 	else if (dwCategoryID <= 0)
// 		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_SSN, dwSSN, lstRoom);
// 	else
// 		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_CATEGORY, dwSSN, dwCategoryID, lstRoom);
// 
// 	if (!bRoomExist || !lstRoom.size())
// 		return S_OK;
// 
// 	ForEachElmt(RoomList, lstRoom, i, j)
// 	{
// 		CRoom* pRoom = (*i);
// 		if (!pRoom) continue;
// 		pRoom->OnAnnounceMsg(sMsg);
// 		pRoom->Release();
// 	}

	return S_OK; // return 0; 구현.. 
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

	return TRUE;
}

BOOL CPMSConnector::OnRegionInfoPCReq(IPMSRegionInfoListPC* plstRegionInfoPC)
{
// 	StatInfoList lstStatPC;
// 	theStatTable.GetStatInfoPC(lstStatPC);
// 
// 	for(StatInfoList::iterator iter = lstStatPC.begin() ; iter != lstStatPC.end() ; iter++)
// 	{
// 		StatisticInfo& si = *iter;
// 
// 		VLONG vecInsert;
// 		vecInsert.insert(vecInsert.begin(), si.m_vecStatistics.begin(), si.m_vecStatistics.end());
// 		plstRegionInfoPC->AddRegionInfoPC(si.m_lSSN, 0, &vecInsert);
// 	}

	return TRUE;
}

BOOL CPMSConnector::OnStatInfoPCReq(IPMSStatInfoListPC* plstStatInfoPC)
{
// 	StatInfoList lstStat;
// 	theStatTable.GetStatInfoPC(lstStat);
// 
// 	RoomList lstRoom;
// 	theRoomTable.GetRoomList(lstRoom);
// 
// 	PayloadHA pld(PayloadHA::msgPMSStatInfoAns_Tag);
// 
// 	//	theLog.Put(DEV_UK,"GLS_SockError, stat_size : ", lstStat.size(), " // Room_Size : ", lstRoom.size() );
// 
// 	for (StatInfoList::iterator iter = lstStat.begin() ; iter != lstStat.end() ; iter++)
// 	{
// 		StatisticInfo& si = *iter;
// 
// 		if (si.m_lSSN != 0L)
// 		{
// 			PMSStatBase sinfo;
// 
// 			list<DWORD> lstCategory;
// 			lstCategory.clear();
// 
// 			sinfo.m_unitID.m_dwSSN = si.m_lSSN;
// 			sinfo.m_unitID.m_dwCategory = 0L;
// 			sinfo.m_dwCU = si.m_lTotCount;
// 			sinfo.m_dwSession = si.m_lTotCount;
// 			sinfo.m_dwChannelCnt = 0L;
// 
// 			LONG lRoomCount = 0L;
// 
// 			ForEachElmt(RoomList, lstRoom, i, j)
// 			{
// 				CRoom* pRoom = (*i);
// 				if (!pRoom) continue;
// 				if (si.m_lSSN == pRoom->GetRoomID().m_lSSN)
// 				{
// 					lRoomCount++;
// 				}
// 
// 				BOOL bIsIn = FALSE;
// 				for (list<DWORD>::iterator it = lstCategory.begin(); it != lstCategory.end(); it++)
// 				{
// 					if (*it == pRoom->GetRoomID().m_dwCategory)
// 						bIsIn = TRUE;
// 				}
// 				if (!bIsIn)
// 					lstCategory.push_back(pRoom->GetRoomID().m_dwCategory);		
// 			}
// 
// 			sinfo.m_dwRoomCnt = lRoomCount;
// 			sinfo.m_sOptionInfo = "";
// 
// 			plstStatInfoPC->AddStatInfoListPC(sinfo.m_unitID.m_dwSSN, sinfo.m_unitID.m_dwCategory, sinfo.m_dwCU, sinfo.m_dwSession, sinfo.m_dwChannelCnt, sinfo.m_dwRoomCnt, sinfo.m_sOptionInfo.c_str());
// 
// 			//			theLog.Put(DEV_UK,"GLS_SockError,GameStat1 : ", sinfo.m_unitID.m_dwSSN, " ", sinfo.m_unitID.m_dwCategory, " ", sinfo.m_dwCU, " ", sinfo.m_dwSession, " ", sinfo.m_dwChannelCnt, " ", sinfo.m_dwRoomCnt, " ", sinfo.m_sOptionInfo);
// 
// 			for (list<DWORD>::iterator it2 = lstCategory.begin(); it2 != lstCategory.end(); it2++)
// 			{
// 				PMSStatBase sinfo2;
// 				sinfo2.m_unitID.m_dwSSN = si.m_lSSN;
// 				sinfo2.m_unitID.m_dwCategory = *it2;
// 				sinfo2.m_dwCU = 0L;
// 				sinfo2.m_dwSession = 0L;
// 				sinfo2.m_dwChannelCnt = 0L;
// 				sinfo2.m_dwRoomCnt = 0L;
// 				sinfo2.m_sOptionInfo = "";
// 
// 				plstStatInfoPC->AddStatInfoListPC(sinfo2.m_unitID.m_dwSSN, sinfo2.m_unitID.m_dwCategory, sinfo2.m_dwCU, sinfo2.m_dwSession, sinfo2.m_dwChannelCnt, sinfo2.m_dwRoomCnt, sinfo2.m_sOptionInfo.c_str());
// 
// 				//				theLog.Put(DEV_UK,"GLS_SockError,GameStat2 : ", sinfo2.m_unitID.m_dwSSN, " ", sinfo2.m_unitID.m_dwCategory, " ", sinfo2.m_dwCU, " ", sinfo2.m_dwSession, " ", sinfo2.m_dwChannelCnt, " ", sinfo2.m_dwRoomCnt, " ", sinfo2.m_sOptionInfo);
// 			}
// 		}
// 	}
// 
// 
// 	ForEachElmt(RoomList, lstRoom, i, j)
// 	{
// 		CRoom* pRoom = (*i);
// 		if (pRoom)
// 			pRoom->Release();
// 	}

	return TRUE;
}

#endif