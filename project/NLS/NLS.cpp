// NLS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "GService.h"

#include "Config.h"
#include "Control.h"


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

	virtual void ExitService();

private:
	HANDLE m_hFile;
};

static CService _service(_T("NLS"));
static HTHREADPOOL _hThreadPool = 0;

HTHREADPOOL GetThreadPool()
{
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}


BOOL CService::InitService()
{
	TLock lo(this);

	// init configs
	BOOL bRet = FALSE;

	tstring tsErrMsg;
	tsErrMsg = theLog.Init(this->m_bService, this->m_szServiceName, _T("NLS.INI"));
	if (tsErrMsg.length() != 0) // Err처리
	{	
		if (m_bService)
			GetCurrentService()->LogEvent(_T("!!!! theLog::Init() Error : [%s]"), 	tsErrMsg.c_str());
		else
			_tprintf(_T("!!!! theLog::Init() Error : [%s]"), tsErrMsg.c_str());

		return FALSE;
	}

	bRet = theConfig.Init();

	LOG(INF_UK,"NLS_SERVICE_INF,*** Trying to startup system ***\n");

	VALIDATE(bRet);
	if(!bRet)
		return FALSE;

	_hThreadPool = ::XtpCreatePool(20);
	if(!_hThreadPool)
	{
		LOG(ERR_UK, "NLS_SERVICE_ERR, *** Failed to create default thread pool ***\n");		
		return FALSE;
	}

	::xsehSetStructuredExceptionHandler(_T("NLS"), _T("C:\\log\\"), true, MINIDUMP_FULL, true); 

	::XlinkInit(_hThreadPool);

	HRESULT hrInit = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hrInit)) {
		LOG(ERR_UK, "NLS_SERVICE_ERR, === Failed Initialize Component ===\n");
		return FALSE;
	}

	//NLS가 이미 떠 있을 경우, 새로운 Process의 생성을 막는다.
	HANDLE hMutex = ::CreateMutex(NULL, FALSE, _T("NLS_MUTEX"));
	if(!hMutex || ::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(ERR_UK,"TCS_SERVICE_ERR ,already run TCS \n");
		return FALSE;
	}

#ifdef _USE_PMS
#pragma oMSG("####### define _USE_PMS")
	LOG(INF_UK, "NLS_SERVICE_INF, *** Applied to [PMS] !! ***");
#else
	LOG(INF_UK, "NLS_SERVICE_INF, *** Not Applied to [PMS] !! ***");
#endif


#ifdef _NFGAME_
	LOG(INF_UK, "NLS_SERVICE_INF, ***** NFGAME NLS !! *****");
#else
	LOG(INF_UK, "NLS_SERVICE_INF, ***** Normal NLS !! *****");
#endif


#ifdef _USE_PMS
	DWORD dwPMSRet = thePMSConnector.Run();
	if (PMSC_ERR_OK != dwPMSRet)
	{
		LOG(ERR_UK, "NLS_SERVICE_INF, -----> PMS : Run() : FAILED  ///  ErrorCode : ", dwPMSRet);
		return FALSE;
	}
	else
		LOG(INF_UK, "NLS_SERVICE_INF,  -----> PMS : Run() : SUCCESS");

#endif
	theControl.RunEx();

	return TRUE;
}

void CService::ExitService()
{
	TLock lo(this);

	LOG(INF_UK, "NLS_SERVICE, *** Trying to shutdown system ***\n");
	LOG(INF_UK, "NLS_SERVICE, == Shutdown Service == \n");

	BOOL bRet = theControl.Stop();
	VERIFY(bRet);

	::CoUninitialize();

	return ;
}



int _tmain(int argc, TCHAR* argv[])
{
	return _service.Main(argc, argv);
}
