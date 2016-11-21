#ifndef __CPMSCONNWRAPPER_H_
#define __CPMSCONNWRAPPER_H_

#include "PMSConn.h"
#include "ADL/PMS_GS.H"

#ifdef _UNICODE
#ifdef _DEBUG
#pragma oMSG("########## PMSConnUD Lib ��� ##########")
#pragma comment(lib, "PMSConnUD.lib")
#else
#pragma oMSG("########## PMSConnU Lib ��� ##########")
#pragma comment(lib, "PMSConnU.lib")
#endif
#else
#ifdef _DEBUG
#pragma oMSG("########## PMSConnD Lib ��� ##########")
#pragma comment(lib, "PMSConnD.lib")
#else
#pragma oMSG("########## PMSConn Lib ��� ##########")
#pragma comment(lib, "PMSConn.lib")
#endif
#endif

// IPMSObject�� Implementation �ϴ� Class
class CPMSConnector : public IPMSObject
{
public:
	CPMSConnector() { }
	virtual ~CPMSConnector() { Stop(); }

public:
	// PMSConn API�� ȣ�� �ϱ� ���� �Լ�
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
	virtual BOOL OnHeartbeatReq(LONG lIndex);
	virtual BOOL OnAnnounceReq(DWORD dwSSN, DWORD dwCategoryID, LPCSTR lpszMsg);
	virtual BOOL OnRegionInfoReq(IPMSRegionInfoList* plstRegionInfo);
	virtual BOOL OnStatInfoReq(IPMSStatInfoList* plstStatInfo);
	virtual BOOL OnPerformInfoReq(IPMSPerformanceInfo *pPerformanceInfo);
	virtual BOOL OnOrderReq(LPCSTR lpszCmdName, LPCSTR lpszCtlVal, LPSTR lpResult, LONG *lpResultLen, DWORD dwSSN, DWORD dwCategoryID);
	virtual BOOL OnRegionInfoPCReq(IPMSRegionInfoListPC* plstRegionInfoPC);
	virtual BOOL OnStatInfoPCReq(IPMSStatInfoListPC* plstStatInfoPC);
};

extern CPMSConnector	thePMSConnector;

#endif // if not def __CPMSCONNWRAPPER_H_