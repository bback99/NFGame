#include "stdafx.h"
#include <PMSConnObject.h>

CPMSConnector		thePMSConnector;

BOOL CPMSConnector::OnRegionInfoReq(IPMSRegionInfoList* plstRegionInfo)
{
	return TRUE;
}

BOOL CPMSConnector::OnStatInfoReq(IPMSStatInfoList* plstStatInfo)
{ 
	return TRUE;
}

BOOL CPMSConnector::OnPerformInfoReq(IPMSPerformanceInfo *pPerformanceInfo)
{ 
	printf("OnRecv On Perform Info Req Message :  \n");

	return TRUE;
}

BOOL CPMSConnector::OnAnnounceReq(DWORD dwSSN, DWORD dwCategoryID, LPCSTR lpszMsg)
{
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
	return TRUE;
}

BOOL CPMSConnector::OnRegionInfoPCReq(IPMSRegionInfoListPC* plstRegionInfoPC)
{
	return TRUE;
}

BOOL CPMSConnector::OnStatInfoPCReq(IPMSStatInfoListPC* plstStatInfoPC)
{
	return TRUE;
}