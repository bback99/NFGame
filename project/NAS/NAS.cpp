// NAS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GService.h"
#include "Config.h"
#include "Control.h"
#include <NFVariant/NFDBManager.h>

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

static CService _service(_T("NAS"));
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
	tsErrMsg = theLog.Init(this->m_bService, this->m_szServiceName, _T("NAS.INI"));
	if (tsErrMsg.length() != 0) // Err처리
	{	
		if (m_bService)
			GetCurrentService()->LogEvent(_T("!!!! theLog::Init() Error : [%s]"), 	tsErrMsg.c_str());
		else
			_tprintf(_T("!!!! theLog::Init() Error : [%s]"), tsErrMsg.c_str());

		return FALSE;
	}

	bRet = theConfig.Init();

	LOG(INF_UK,"NAS_SERVICE_INF,*** Trying to startup system ***\n");

	VALIDATE(bRet);
	if(!bRet)
		return FALSE;

	_hThreadPool = ::XtpCreatePool(20);
	if(!_hThreadPool)
	{
		LOG(ERR_UK, "NAS_SERVICE_ERR, *** Failed to create default thread pool ***\n");
		return FALSE;
	}

	::xsehSetStructuredExceptionHandler(_T("NAS"), _T("C:\\log\\"), true, MINIDUMP_FULL, true); 

	::XlinkInit(_hThreadPool);

	HRESULT hrInit = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hrInit)) {
		LOG(ERR_UK, "NAS_SERVICE_ERR, === Failed Initialize Component ===\n");
		return FALSE;
	}
	
	//// Achv.XML 파일을 읽는다.
	//bRet = g_kAchvDef.LoadAchvXML();
	//if (!bRet)
	//{
	//	LOG(ERR_UK, "NAS_SERVICE_ERR, === Achv.XML load failed!!! ===\n");
	//	return FALSE;
	//}

	LOG(ERR_UK, "NAS_SERVICE_INF, *** Using gDBGW ***\n");
	bRet = theNFDBMgr.DBInit();
	if (!bRet)
	{
		LOG(ERR_UK, "NAS_SERVICE_ERR, === DBGW Mananger Initialization is Failed ===\n");
		return FALSE;
	}

#ifdef _USE_PMS
#pragma oMSG("####### define _USE_PMS")
	LOG(INF_UK, "NAS_SERVICE_INF, *** Applied to [PMS] !! ***");
#else
	LOG(INF_UK, "NAS_SERVICE_INF, *** Not Applied to [PMS] !! ***");
#endif


#ifdef _NFGAME_
	LOG(INF_UK, "NAS_SERVICE_INF, ***** NFGAME NAS !! *****");
#else
	LOG(INF_UK, "NAS_SERVICE_INF, ***** Normal NAS !! *****");
#endif


#ifdef _USE_PMS
	DWORD dwPMSRet = thePMSConnector.Run();
	if (PMSC_ERR_OK != dwPMSRet)
	{
		LOG(ERR_UK, "NAS_SERVICE_INF, -----> PMS : Run() : FAILED  ///  ErrorCode : ", dwPMSRet);
		return FALSE;
	}
	else
		LOG(INF_UK, "NAS_SERVICE_INF,  -----> PMS : Run() : SUCCESS");

#endif
	theControl.RunEx();

	return TRUE;
}

void CService::ExitService()
{
	TLock lo(this);

	LOG(INF_UK, "NAS_SERVICE, *** Trying to shutdown system ***\n");
	LOG(INF_UK, "NAS_SERVICE, == Shutdown Service == \n");

	BOOL bRet = theControl.Stop();
	VERIFY(bRet);

	::CoUninitialize();

	return ;
}

int _tmain(int argc, TCHAR* argv[])
{
	return _service.Main(argc, argv);
}
