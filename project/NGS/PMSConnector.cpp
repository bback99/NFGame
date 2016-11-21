#include "StdAfx.h"

/**
#ifndef _PMSCONN_
#include <pmsconnector.h>

#ifdef _USE_PMS
#include "Common.h"
#include "StatisticsTable.h"
#include "Room.h"
#include "RoomTable.h"


CPMSConnector thePMSConnector;

CPMSConnector::CPMSConnector(void)
{
}

CPMSConnector::~CPMSConnector(void)
{
}

bool CPMSConnector::Init(int argc, LPTSTR argv[])
{
	return PAER_SUCCESS == InitPMSAgent(argc, argv) ;

}

bool CPMSConnector::Run()
{
	return RunPMSAgent();
}

void CPMSConnector::Stop()
{
	StopPMSAgent();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 필수 구현 interface 
// PMSA가 내부적으로 응답 메세지를 생성, 전송함.
////////////////////////////////////////////////////////////////////////////////////////////////
bool CPMSConnector::OnOrderReq(LPSTR lpCmdName, PMSUnitID unitID, LPSTR lpCtlVal, LONG lCtlValLen, LPSTR lpResult, LONG *lpResultLen)
{
	theLog.Put(DEV,"NGS_Pms,OnRecv OnOrderReq Message : CmdName.", lpCmdName);
	sprintf(lpResult, "SUCCESS %s ", lpCmdName);
	*lpResultLen = strlen(lpResult);
	return TRUE;
}

bool CPMSConnector::OnHeartbeatReq(LONG lSequenceNo)
{
	//printf("OnRecv HeartBeatReq Message : SeqNo. %d \n", lSequenceNo);
	// 이 application이 정상 동작인지.. 판단 후 return.

	///////// 할꺼....
	// 요기 heart beat check.

	return TRUE;	
}

////////////////////////////////////////////////////////////////////////////////////////////////
// optional interface 
// Answer가 필요할 경우, 직접 메세지를 생성하여 전달해야 함.
////////////////////////////////////////////////////////////////////////////////////////////////
long CPMSConnector::OnAnnounceReq(PMSUnitID unitID, LONG lLength, LPSTR lpMsg)
{
	//printf("OnRecv OnAnnounceReq Message : AnnounceMsg %s \n", lpMsg);
	//DWORD m_dwSSN; 
	//DWORD m_dwGSID;	
	//DWORD m_dwCategory;

	//string sMsg = lpMsg;
	xstring sMsg;
	sMsg.assign((LPCXSTR)lpMsg, lLength);

	BOOL bRoomExist = FALSE;

	RoomList lstRoom;
	lstRoom.clear();

	theLog.Put(DEV, "CPMSConnector_OnAnnounceReq"_COMMA, "!!! Msg = [ ", sMsg, "], unitID.m_dwSSN/m_dwCategory=[", unitID.m_dwSSN, "/", unitID.m_dwCategory, "]");

	if (unitID.m_dwSSN == 0)
		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_ALLSERVICE, lstRoom);
	else if (unitID.m_dwCategory <= 0)
		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_SSN, unitID.m_dwSSN, lstRoom);
	else
		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_CATEGORY, unitID.m_dwSSN, unitID.m_dwCategory, lstRoom);

	if (!bRoomExist || !lstRoom.size())
		return S_OK;

	ForEachElmt(RoomList, lstRoom, i, j)
	{
		CRoom* pRoom = (*i);
		if (!pRoom) continue;
		pRoom->OnAnnounceMsg(sMsg);
		pRoom->Release();
	}

	//return -1L;	// 구현 않을 경우와 동일
	return S_OK; // return 0; 구현.. 
}

bool CPMSConnector::OnPerformInfoReq()
{
	//printf("OnRecv On Perform Info Req Message :  \n");
	// 정의된 형태의 performance data를 채워서, 메세지 생성후 send 요청.


	// 아래는 sample..
	vecPerformT vec;
	//LONG DEF_LEN = 20;
	//for(int i = 0; i < DEF_LEN; i++)
	//{
	//	vec.push_back(i+1);
	//}


	PayloadHA pld(PayloadHA::msgPMSPerformAns_Tag);
	pld.un.m_msgPMSPerformAns->m_vecPerform = vec;

	SendMsg(pld);
	return TRUE;
}

bool CPMSConnector::OnRegionInfoReq()
{
#ifdef _USE_PMS
	//printf("OnRecv On Region Info Req Message :  \n");
	// 정의된 형태의 지역별 접속자 분포 data를 채워서, 메세지 생성후 send 요청.

	StatInfoList lstStat;
	theStatTable.GetStatInfo(lstStat);

	PayloadHA pld(PayloadHA::msgPMSRegionInfoAns_Tag);
	DWORD dwGSID = GetGSID();
	
	for(StatInfoList::iterator iter = lstStat.begin() ; iter != lstStat.end() ; iter++)
	{
		StatisticInfo& si = *iter;

		PMSRegionInfo rinfo;
		rinfo.m_unitID.m_dwCategory = 0;	/// 이거는 소용없을꺼 같음.
		rinfo.m_unitID.m_dwSSN = si.m_lSSN;
		rinfo.m_unitID.m_dwGSID = dwGSID;
		rinfo.m_vecRegion.insert(rinfo.m_vecRegion.begin(), si.m_vecStatistics.begin(), si.m_vecStatistics.end());
		pld.un.m_msgPMSRegionInfoAns->m_vecRegionInfo.push_back(rinfo);
	}
	SendMsg(pld);
#endif

	return TRUE;
}

//#define __PMS_DEBUG__
bool CPMSConnector::OnStatInfoReq()
{
#ifdef _USE_PMS
//	printf("OnRecv On Stat Info Req Message :  \n");
	// 정의된 형태의 게임별 사용자 접속 data를 채워서, 메세지 생성후 send 요청.

	///////////////////////////////////////////////
	//  PMS 디버깅 코드
#ifdef __PMS_DEBUG__
	static DWORD dwTime =::GetTickCount();
	BOOL  bIsLog = FALSE;
	if (::GetTickCount() - dwTime > 10*60*1000) // 10분
	{		
		bIsLog = TRUE;
		dwTime = ::GetTickCount();
	}
	else
		bIsLog = FALSE;	
#endif //__PMS_DEBUG__
	///////////////////////////////////////////////

	// 아래는 sample..
	StatInfoList lstStat;
	theStatTable.GetStatInfo(lstStat);

	RoomList lstRoom;
	theRoomTable.GetRoomList(lstRoom);

	PayloadHA pld(PayloadHA::msgPMSStatInfoAns_Tag);
	DWORD dwGSID = GetGSID();

	for (StatInfoList::iterator iter = lstStat.begin() ; iter != lstStat.end() ; iter++)
	{
		StatisticInfo& si = *iter;

		if (si.m_lSSN != 0L)
		{
			PMSStatBase sinfo;

			list<DWORD> lstCategory;
			lstCategory.clear();

			sinfo.m_unitID.m_dwGSID = dwGSID;
			sinfo.m_unitID.m_dwSSN = si.m_lSSN;
			sinfo.m_unitID.m_dwCategory = 0L;

			sinfo.m_dwCU = si.m_lTotCount;
			sinfo.m_dwSession = si.m_lTotCount;
			sinfo.m_dwChannelCnt = 0L;
			LONG lRoomCount = 0L;
			ForEachElmt(RoomList, lstRoom, i, j)
			{
				CRoom* pRoom = (*i);
				if (!pRoom) continue;
				if (si.m_lSSN == pRoom->GetRoomID().m_lSSN)
					lRoomCount++;

				BOOL bIsIn = FALSE;
				for (list<DWORD>::iterator it = lstCategory.begin(); it != lstCategory.end(); it++)
				{
					if (*it == pRoom->GetRoomID().m_dwCategory)
						bIsIn = TRUE;
				}
				if (!bIsIn)
					lstCategory.push_back(pRoom->GetRoomID().m_dwCategory);

				//pRoom->Release(); // lstStat의 size가 2 이상인 경우 두번 이상 Release()되어 유령방 발생했음.
			}
			sinfo.m_dwRoomCnt = lRoomCount;
			sinfo.m_sOptionInfo = "";

			pld.un.m_msgPMSStatInfoAns->m_vecStatBase.push_back(sinfo);
			
			///////////////////////////////////////////////
			//  PMS 디버깅 코드
#ifdef __PMS_DEBUG__
			if (bIsLog == TRUE)
			{
				theLog.Put(INF_UK, "NGS_PMS_DEBUG, sinfo.m_dwRoomCnt:", sinfo.m_dwRoomCnt, 
												", sinfo.m_dwSession:", sinfo.m_dwSession,
												", sinfo.m_dwCU:",		sinfo.m_dwCU,
												", sinfo.m_dwSSN:",		sinfo.m_unitID.m_dwSSN,
                                                ", sinfo.m_dwGSID:",	sinfo.m_unitID.m_dwGSID);
			}
#endif
			///////////////////////////////////////////////
			

			for (list<DWORD>::iterator it2 = lstCategory.begin(); it2 != lstCategory.end(); it2++)
			{
				PMSStatBase sinfo2;
				sinfo2.m_unitID.m_dwGSID = dwGSID;
				sinfo2.m_unitID.m_dwSSN = si.m_lSSN;
				sinfo2.m_unitID.m_dwCategory = *it2;
				sinfo2.m_dwCU = 0L;
				sinfo2.m_dwSession = 0L;
				sinfo2.m_dwChannelCnt = 0L;
				sinfo2.m_dwRoomCnt = 0L;
				sinfo2.m_sOptionInfo = "";

				pld.un.m_msgPMSStatInfoAns->m_vecStatBase.push_back(sinfo2);
			}
		}
	}

	
	ForEachElmt(RoomList, lstRoom, i, j)
	{
		CRoom* pRoom = (*i);
		if (pRoom)
			pRoom->Release();
	}

	SendMsg(pld);
#endif //_USE_PMS
	return TRUE;
}

#endif



#endif // _PMSCONN_
*/