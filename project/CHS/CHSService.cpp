// CHSService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Control.h"
#include <GService.h>
#include "agent.h"
#include <NFVariant/NFDBManager.h>

#include "LRBHandler.h"
#define theLRBManager theLRBHandler


#include <PMSConnObject.h>

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

	virtual void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv) 
	{
		__try
		{
			thePMSConnector.Init(dwArgc, lpszArgv);
			GService::ServiceMain(dwArgc, lpszArgv);
		}
		__finally
		{
			fflush(NULL);
		}
	}

protected:
	virtual BOOL InitService();

	virtual void ExitService();
};

static CService _service(_T("CHS"));
static HTHREADPOOL _hThreadPool = 0;

static HTHREADPOOL _hChannelThreadPool = 0;
static HTHREADPOOL _hLRBThreadPool = 0;
static HTHREADPOOL _hIOThreadPool = 0;

HTHREADPOOL GetThreadPool()
{
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}
HTHREADPOOL GetChannelThreadPool()
{
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}
HTHREADPOOL GetLRBThreadPool()
{
	ASSERT(_hThreadPool != NULL);
	return _hThreadPool;
}

void StopCHSService()
{	
	SetEvent(_service.m_hEvent);
}

BOOL CService::InitService()
{
	TLock lo(this);
	tstring tsErrMsg;

	tstring tstrCnfPath;
#ifndef _UNICODE
	tstrCnfPath = theControl.m_confPath.GetConfPath();
#else
	tstrCnfPath = str2wstr(string(theControl.m_confPath.GetConfPath()));
#endif
	tsErrMsg = theLog.Init(this->m_bService, this->m_szServiceName, tstrCnfPath.c_str()/*_T("CHS.ini")*/);
	if (tsErrMsg.length() != 0) // Err처리
	{	
		if (m_bService)
			GetCurrentService()->LogEvent(_T("!!!! theLog::Init() Error : [%s]"), 	tsErrMsg.c_str());
		else
			_tprintf(_T("!!!! theLog::Init() Error : [%s]"), tsErrMsg.c_str());

		return FALSE;
	}

//	TLOG0("*** Trying to startup system ***\n");
	LOG(INF_UK, "CHS_SERVER_START"_LK, "####### Start CHS Service ########");

	#pragma oMSG("[Compile Info] : Using Win32 SEH")
	::xsehSetStructuredExceptionHandler(_T("CHS"), _T("C:\\LOG\\"), true, MINIDUMP_FULL, true); 

	_hThreadPool = ::XtpCreatePool(15);
	if(!_hThreadPool)
	{
		LOG(INF_UK, "CHS_CService_CreatePool_Err"_LK, "*** Failed to create thread pool ***");
		return FALSE;
	}
	::XlinkInit(_hThreadPool);

	////////////// channel info
	LOG(INF_UK, "CHS_CService"_LK, "================== CHS INFO =====================");
#ifdef _CHSNLS
	LOG(INF_UK, "CHS_CService"_LK, "*** Applied Channel Rejoin with LCS!! ***");
#else
	LOG(INF_UK, "CHS_CService"_LK, "*** Not Applied Channel Rejoin with LCS!! ***");
#endif
	LOG(INF_UK, "CHS_CService"_LK, "=================================================");
	////////////////////////////

	int nErrorCode = 0;
	if (!theNFDBMgr.DBInit())
	{
		LOG(INF_UK, "CHS_CService_DBGWMInit_Err"_LK, "*** Failed to connect gDBGW : DBGWMInit() *** : ErrorCode = ", nErrorCode);
		return FALSE;
	}

	DWORD dwPMSRet = thePMSConnector.Run();
	if (PMSC_ERR_OK != dwPMSRet)
	{
		LOG(ERR_UK, "CHS_SERVICE_INF, -----> PMS : Run() : FAILED  ///  ErrorCode : ", dwPMSRet);
		return FALSE;
	}
	else
		LOG(INF_UK, "CHS_SERVICE_INF,  -----> PMS : Run() : SUCCESS");

	if(!theControl.InitChannel())
	{
		LOG(INF_UK, "CHS_CService_InitChannel_Err"_LK, "!!!!!!!!!!!!! fail InitChannel() ");
		return FALSE;
	}	

	HRESULT hrInit = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hrInit)) {
		LOG(INF_UK, "CHS_CService"_LK, "=== Failed Initialize Component ===\n");
		return FALSE;
	}

	/////////// 설정파일 디렉토리 설정. ////////////
	////////////////////////////////////////////////

	BOOL bRet = theControl.RunControl();
	if(!bRet)
	{
		TLOG0("*** Failed to run theControl ***\n");
		return FALSE;
	}

	return TRUE;
}

void CService::ExitService()
{
	TLock lo(this);

	theControl.SetBootState(CHS_STOP);

	LOG(INF_UK, "CHS_CService"_LK, "*** Trying to shutdown system ***\n");	
	TLOG(_T("*** Trying to shutdown system ***\n"));	
	BOOL bRet = theControl.StopControl();
	thePMSConnector.Stop();
	VERIFY(bRet);
	::CoUninitialize();
	LOG(INF_UK, "CHS_CService"_LK, "== Shutdown Service ==");
}

int _tmain(int argc, TCHAR* argv[])
{
	return _service.Main(argc, argv);
}
