// NGS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Control.h"
#include <GService.h>
#include "ErrorLog.h"
#include <NFVariant/NFDBManager.h>
#include "Room.h"

#ifdef _USE_PMS
#include <PMSConnObject.h>
#endif // _USE_PMS


//// ACHV BEGIN
#include <ACHV/AchvDef.h>
static achv::CAchvMgr& g_achv = achv::CAchvMgr::Instance();
//// ACHV END


///////////////////////////////////////////////////////////////////////////////////
// CService

class CService : public GService
{
	IMPLEMENT_TISAFE(CService)
public:
	typedef GService TBase;
	CService(LPCTSTR szServiceName) : GService(szServiceName)
	{
//		SetDefaultLogEventID(MSG_INFO);
	}

#ifdef _USE_PMS
	virtual void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv) 
	{
		thePMSConnector.Init(dwArgc, lpszArgv);
		GService::ServiceMain(dwArgc, lpszArgv);
	}
#endif

protected:
	virtual BOOL InitService();
	BOOL InitServiceImple();

	virtual void ExitService();
private:
	HANDLE m_hFile;
};

CService _service(_T("NGS"));
static HTHREADPOOL _hThreadPool = 0;
// static HTHREADPOOL _hListenerThreadPool = 0;
static HTHREADPOOL _hDBThreadPool = 0;

HTHREADPOOL GetThreadPool()
{
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}

HTHREADPOOL GetListenerThreadPool()
{
//	ASSERT(_hListenerThreadPool != NULL);
//	return _hListenerThreadPool;
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}

HTHREADPOOL GetDBThreadPool()
{
//	ASSERT(_hDBThreadPool != NULL);
//	return _hDBThreadPool;
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}


//#define __DUMP_TEST_

BOOL CService::InitService()
{
#ifndef __DUMP_TEST_

	return InitServiceImple();

#else // __DUMP_TEST_
	::XtpSetLibFlag(XTP_THREADPROC_WRAP_SEH, TRUE);


	int nRetVal;
	__try
	{			
		nRetVal =  InitServiceImple();
	}
	__except(::xsehTopLevelExceptionFilter(GetExceptionInformation()))
	{
		::ExitProcess(0xFFFFFFFF); // -1
	}

	return nRetVal;
#endif // __DUMP_TEST_
}

BOOL CService::InitServiceImple()
{
	TLock lo(this);


	theLog.Init(this->m_bService, _T("NGS"), _T("NGS.ini"));	

	::xsehSetStructuredExceptionHandlerA("NGS", "C:\\LOG\\", true, MINIDUMP_FULL, true);


	theLog.Put(INF_UK, "NGS_General,*** Trying to startup system ***");

	// 쓰레드 풀 크기를 ini 에서 읽어오도록 한다. 최소 40, 최대 200 으로 한다.
	UINT pool_size_min = 40, pool_size_max = 200;	
	UINT pool_size = ::GetPrivateProfileIntA("CONTROL","THREADPOOLSIZE", 40, theControl.m_confPath.GetConfPath()/*"GRCConfig.INI"*/);
	if (pool_size < pool_size_min)
		pool_size = pool_size_min;
	else if (pool_size_max < pool_size)
		pool_size = pool_size_max;	
	theLog.Put(INF_UK, "NGS_General, call XtpCreatePool(). pool_size:", pool_size);

	_hThreadPool = ::XtpCreatePool(pool_size);

	if(!_hThreadPool)
	{
		//SVCLOG0("*** Failed to create default thread pool ***\n");
		theLog.Put(ERR_UK, "NGS_General_Error"_COMMA, "*** Failed to create default thread pool ***");
		return FALSE;
	}

//	_hListenerThreadPool = ::XtpCreatePool(10);
//	if(!_hListenerThreadPool)
//	{
//		SVCLOG0("*** Failed to create listener thread pool ***\n");
//		return FALSE;
//	}
//	_hDBThreadPool = ::XtpCreatePool(50);
//	if(!_hDBThreadPool)
//	{
//		SVCLOG0("*** Failed to create DB thread pool ***\n");
//		return FALSE;
//	}

	::XlinkInit(_hThreadPool);

	HRESULT hrInit = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hrInit))
	{
		//SVCLOG0("=== Failed Initialize Component ===\n");
		theLog.Put(ERR_UK, "NGS_General_Error"_COMMA, "Failed Initialize Component");
		return FALSE;
	}

	BOOL bRet = FALSE;

	theLog.Put(INF_UK, "NGS_General"_COMMA, "Using gDBGW");
	bRet = theNFDBMgr.DBInit();
	if (!bRet)
	{
		theLog.Put(ERR_UK, "NGS_theControl_Error"_COMMA, "DBGW Mananger Initialization is Failed");
		return FALSE;
	}


	theLog.Put(INF_UK, "NGS_General"_COMMA,		"########################################################");
	theLog.Put(INF_UK, "NGS_General"_COMMA, "########## NGS Server is [Korean Ranking Test] Version ##########");	
	theLog.Put(INF_UK, "NGS_General"_COMMA,		"########################################################");

	bRet = theControl.Run();

	if (!bRet)
	{
		//SVCLOG0("*** Failed to run theControl ***\n");
		theLog.Put(ERR_UK, "NGS_Service_Error"_COMMA, "Running theControl is Failed");
		theControl.Stop();
		return FALSE;
	}

#ifdef _USE_PMS
	theLog.Put(INF_UK, "NGS_General"_COMMA, "Using PMS");
#else
	theLog.Put(INF_UK, "NGS_General"_COMMA, "Not Using PMS, Just using AMS");
#endif

//// ACHV BEGIN
	if (!g_achv.LoadAchvXML())
	{
		theLog.Put(ERR_UK, "NGS_Service_Error"_COMMA, "achv::LoadAchvXML() failed.");
		theControl.Stop();
		return FALSE;
	}
	if (!g_achv.addReportCallback(CRoom::AchvReportCallback))
	{
		theLog.Put(ERR_UK, "NGS_Service_Error"_COMMA, "achv::addReportCallback() failed.");
		theControl.Stop();
		return FALSE;
	}
//// AHCV END
	theLog.Put(INF_UK, "NGS_General"_COMMA, "*** Success to startup system ***");

	return TRUE;
}

void CService::ExitService()
{
	TLock lo(this);

	BOOL bRet = theControl.Stop();
	VERIFY(bRet);

	//SVCLOG0("*** Success to shutdown system ***\n");
	theLog.Put(INF_UK, "NGS_General"_COMMA, "*** Success to shutdown system ***");

	theLog.Final();

	::CoUninitialize();
}

///////////////////////////////////////////////////////////////////////////////////
// main()

int _tmain(int argc, TCHAR* argv[])
{
	return _service.Main(argc, argv);
}

	