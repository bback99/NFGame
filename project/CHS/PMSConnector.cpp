#include "StdAfx.h"
/**
#ifndef _PMSCONN_

#include <pmsconnector.h>

#include "Common.h"
#include "CHSInfoDir.h"
#include "StatisticsTable.h"
#include "ChannelDir.h"
#include "LRBHandler.h"
                                      
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
	printf("OnRecv OnOrderReq Message : CmdName. %s \n", lpCmdName);
	sprintf(lpResult, "SUCCESS %s ", lpCmdName);
	*lpResultLen = strlen(lpResult);
	return TRUE;
}


bool CPMSConnector::OnHeartbeatReq(LONG lSequenceNo)
{
	printf("OnRecv HeartBeatReq Message : SeqNo. %d \n", lSequenceNo);
	// 이 application이 정상 동작인지.. 판단 후 return.

	///////// 할꺼....
	// 요기 heart beat check.

	if(theLRBHandler.CheckStartUp())
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// optional interface 
// Answer가 필요할 경우, 직접 메세지를 생성하여 전달해야 함.
////////////////////////////////////////////////////////////////////////////////////////////////
long CPMSConnector::OnAnnounceReq(PMSUnitID unitID, LONG lLength, LPSTR lpMsg)
{
	//string sMsg = lpMsg;
	xstring sMsg;
	sMsg.assign((LPCXSTR)lpMsg, lLength);

	if(unitID.m_dwSSN == 0)
		theChannelDir.PMSAnnounceReqInSystem(sMsg);
	else if(unitID.m_dwCategory <= 0)
		theChannelDir.PMSAnnounceReqInSSN(unitID.m_dwSSN, sMsg);
	else
		theChannelDir.PMSAnnounceReqInCategory(unitID.m_dwSSN, unitID.m_dwCategory, sMsg);
	return S_OK; // return 0; 구현.. 
}

bool CPMSConnector::OnPerformInfoReq()
{
	printf("OnRecv On Perform Info Req Message :  \n");
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

	return TRUE;
}

bool CPMSConnector::OnStatInfoReq()
{
	//printf("OnRecv On Stat Info Req Message :  \n");
	// 정의된 형태의 게임별 사용자 접속 data를 채워서, 메세지 생성후 send 요청.
	
	typedef pair< long, long > PAIR_LONG;
	PAIR_LONG pairLong;
	PayloadHA pld(PayloadHA::msgPMSStatInfoAns_Tag);

	ChannelUpdateInfoList lst;
 	DWORD dwCount = theCHSInfoDir.GetCHSInfoListForPMS(lst, TRUE);
	DWORD dwGSID = GetGSID();
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
				PMSStatBase psb(PMSUnitID(cui.m_channelID.m_lSSN, dwGSID, cui.m_channelID.m_dwCategory), \
					cui.m_lWaitingUserCount, cui.m_lUserCount, 1, cui.m_lRoomCount, string(""));
				mapPMSStat[pairLong] = psb;
			}
			else
			{
				it->second.m_dwChannelCnt++;
				it->second.m_dwCU += cui.m_lWaitingUserCount;
				it->second.m_dwRoomCnt += cui.m_lRoomCount;
				it->second.m_dwSession += cui.m_lUserCount; //(cui.m_lUserCount-cui.m_lWaitingUserCount);				
			}
		}
		
		for(map<PAIR_LONG, PMSStatBase>::iterator iter1 = mapPMSStat.begin() ; iter1 != mapPMSStat.end() ; iter1++)
			pld.un.m_msgPMSStatInfoAns->m_vecStatBase.push_back(iter1->second);        
	}	
	///// 위에 안에다 넣어도 됨.
	SendMsg(pld);
	return TRUE;
}

#endif
*/