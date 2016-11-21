
#include "Control.h"
#include <GService.h>
#include "ErrorLog.h"


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

