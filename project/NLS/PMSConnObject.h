#ifndef __CPMSCONNWRAPPER_H_
#define __CPMSCONNWRAPPER_H_
//#pragma once
#include "PMSConn.h"
#include "ADL/PMS_GS.H"


#ifdef _UNICODE
#ifdef _DEBUG
	#pragma oMSG("########## PMSConnUD Lib 사용 ##########")
	#pragma comment(lib, "PMSConnUD.lib")
#else
	#pragma oMSG("########## PMSConnU Lib 사용 ##########")
	#pragma comment(lib, "PMSConnU.lib")
#endif
#else
#ifdef _DEBUG
	#pragma oMSG("########## PMSConnD Lib 사용 ##########")
	#pragma comment(lib, "PMSConnD.lib")
#else
	#pragma oMSG("########## PMSConn Lib 사용 ##########")
	#pragma comment(lib, "PMSConn.lib")
#endif
#endif



// IPMSObject를 Implementation 하는 Class
class CPMSConnector : public IPMSObject
{
public:
	CPMSConnector() { }
	virtual ~CPMSConnector() { Stop(); }

public:
		// PMSConn API를 호출 하기 위한 함수
	DWORD	Init(DWORD argc, LPTSTR argv[]){return ::PMSInitConn(argc, argv); }
	DWORD	Run(){ return ::PMSRunConn(this); }
	void	Stop(){ ::PMSStopConn(); }
	BOOL	SendWarningMsg(DWORD dwErrLvl, LPCSTR pszWarningMsg, LPCSTR pszTreatMsg, DWORD dwSSN, DWORD dwCategory)
	{
		return ::PMSSendWarningMsg(dwErrLvl, pszWarningMsg, pszTreatMsg, dwSSN, dwCategory);
	}
	LPCTSTR	GetConfigFileName();
	DWORD	GetStatus();

public:
	virtual BOOL OnHeartbeatReq(LONG lIndex); //{ return TRUE;}
	virtual BOOL OnAnnounceReq(DWORD dwSSN, DWORD dwCategoryID, LPCSTR lpszMsg);//{ return TRUE;}
	virtual BOOL OnRegionInfoReq(IPMSRegionInfoList* plstRegionInfo);//{ return TRUE;}
	virtual BOOL OnStatInfoReq(IPMSStatInfoList* plstStatInfo);//{ return TRUE;}
	virtual BOOL OnPerformInfoReq(IPMSPerformanceInfo *pPerformanceInfo);//{ return TRUE;}	
	virtual BOOL OnOrderReq(LPCSTR lpszCmdName, LPCSTR lpszCtlVal, LPSTR lpResult, LONG *lpResultLen, DWORD dwSSN, DWORD dwCategoryID);
	virtual BOOL OnRegionInfoPCReq(IPMSRegionInfoListPC* plstRegionInfoPC);
	virtual BOOL OnStatInfoPCReq(IPMSStatInfoListPC* plstStatInfoPC);
};

extern CPMSConnector	thePMSConnector;



#endif // if not def __CPMSCONNWRAPPER_H_