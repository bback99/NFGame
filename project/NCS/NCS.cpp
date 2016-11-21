// NCS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <GService.h>
#include "Manager.h"
#include "CharLobbyManager.h"

#ifdef _USE_PMS
#include <PMSConnObject.h>
#endif // _USE_PMS
///////////////////////////////////////////////////////////////////////////////////
// CService

class CService : public GService
{
	IMPLEMENT_TISAFE(CService)
public:
	typedef GService TBase;
	CService(LPCTSTR szServiceName) : GService(szServiceName)
	{
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

static CService _service(_T("NCS"));
static HTHREADPOOL _hThreadPool = 0;
static HTHREADPOOL _hDBThreadPool = 0;

HTHREADPOOL GetThreadPool()
{
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}

HTHREADPOOL GetListenerThreadPool()
{
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}

HTHREADPOOL GetDBThreadPool()
{
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

	theLog.Init(this->m_bService, _T("NCS"), _T("NCS.ini"));	

	::xsehSetStructuredExceptionHandlerA("NCS", "C:\\LOG\\", true, MINIDUMP_FULL, true);

	theLog.Put(INF_UK, "NCS_General,*** Trying to startup system ***");

	_hThreadPool = ::XtpCreatePool(30);
	if(!_hThreadPool)
	{
		theLog.Put(ERR_UK, "NCS_General_Error"_COMMA, "*** Failed to create default thread pool ***");
		return FALSE;
	}

	::XlinkInit(_hThreadPool);

	HRESULT hrInit = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hrInit))
	{
		theLog.Put(ERR_UK, "NCS_General_Error"_COMMA, "Failed Initialize Component");
		return FALSE;
	}

	BOOL bRet;
 	bRet = theManager.Init();
  	if (!bRet)
 	{
 		theLog.Put(ERR_UK, "NCS_Service_Error"_COMMA, "Manager Init is Failed");
 		return FALSE;
 	}

	bRet = theCharLobbyManager.CreateCharLobby();
	if (!bRet)
	{
		theLog.Put(ERR_UK, "NCS_Service_Error"_COMMA, "CreateCharLobby is Failed");
		return FALSE;
	}

	bRet = theManager.Start();
	if (!bRet)
	{
		theLog.Put(ERR_UK, "NCS_Service_Error"_COMMA, "Manager Start is Failed");
		return FALSE;
	}

#ifdef _USE_PMS
	theLog.Put(INF_UK, "NCS_General"_COMMA, "Using PMS");
#else
	theLog.Put(INF_UK, "NCS_General"_COMMA, "Not Using PMS, Just using AMS");
#endif

	theLog.Put(INF_UK, "NCS_General"_COMMA, "*** Success to startup system ***");

	return TRUE;
}

void CService::ExitService()
{
	TLock lo(this);

	theManager.Stop();

	theLog.Put(INF_UK, "NCS_General"_COMMA, "*** Success to shutdown system ***");

	theLog.Final();

	::CoUninitialize();
}

int _tmain(int argc, _TCHAR* argv[])
{
	return _service.Main(argc, argv);
}

