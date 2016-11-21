//
//CManager.cpp
//

#include "StdAfx.h"
#include "Manager.h"
#include "Listener.h"
#include "LRBHandler.h"
#include <NFVariant/NFDBManager.h>
#include <PMSConnObject.h>
#include "Category.h"
#include "CharLobbyContext.h"

//// ACHV BEGIN
#include <ACHV/AchvDef.h>
static achv::CAchvMgr& g_achv = achv::CAchvMgr::Instance();
//// ACHV END


CManager		theCManager;

HTHREADPOOL CManager::m_hThreadPool = 0;

//////////////////////////////////////////////////////
CManager::CManager()
{
	m_ulThreadCount = 1;
	m_sIP.erase();
}
CManager::~CManager()
{
}
BOOL CManager::Init()
{
	SYSTEM_INFO sys;
	::GetSystemInfo(&sys);

	//m_ulThreadCount = (sys.dwNumberOfProcessors*2) + 1;	
	UINT m_ulThreadCount = 30;
	LOG (INF, "CManager : Thread Count in threadpool  [", m_ulThreadCount, "]");

	m_hThreadPool = ::XtpCreatePool(m_ulThreadCount);
	if(!m_hThreadPool) {		
		LOG (ERR, "CManager : *** Failed to create default thread pool ***");
		return FALSE;
	}

	BOOL bRet = GetInternalIP();
	if(!bRet) {
		LOG (ERR, "CManager : *** Failed to GetInternal IP ***");
		return FALSE;
	}

	bRet = theLRBHandler.Init();
	if (!bRet) {
		LOG (ERR, "CManager : *** Failed to LRBHanlder Init ***");
		return FALSE;
	}

	theLog.Put(INF_UK, "NGS_General"_COMMA, "Using gDBGW");
	bRet = theNFDBMgr.DBInit();
	if (!bRet) {
		theLog.Put(ERR_UK, "NCS_theControl_Error"_COMMA, "DBGW Mananger Initialization is Failed");
		return FALSE;
	}

	bRet = theNFDataItemMgr.GetAllResourceData();
	if (!bRet) {
		theLog.Put(ERR_UK, "NCS_theControl_Error"_COMMA, "theNFDataItemMgr GetAllResourceData is Failed");
		return FALSE;
	}

	bRet = theServiceTable.Init();
	if (!bRet) {
		theLog.Put(ERR_UK, "NCS_theControl_Error"_COMMA, "theServiceTable Init is Failed");
		return FALSE;
	}

	//// ACHV BEGIN
	if (!g_achv.LoadAchvXML())
	{
		theLog.Put(ERR_UK, "NCS_theControl_Error"_COMMA, "achv::LoadAchvXML() failed.");
		return FALSE;
	}
	if (!g_achv.addReportCallback(CCharLobbyContext::AchvReportCallback))
	{
		theLog.Put(ERR_UK, "NCS_theControl_Error"_COMMA, "achv::addReportCallback() failed.");
		return FALSE;
	}
	//// AHCV END

	m_eRegInit.Create();

	return TRUE;
}

BOOL CManager::Start()
{
	BOOL bRet = theLRBHandler.Run();
	if(!bRet) {
		LOG ( ERR, "CManager : *** Failed to run XLRBMgr instance ***");
		return FALSE;
	}

	int nRet = m_eRegInit.Wait(30000) ;

	if ( _wait_timeout(nRet)) {
		LOG ( ERR, "CManager : *** Failed Init TimeOut RegEvent ***");
		return FALSE;
	}
	else if ( _wait_failed(nRet) ) {
		LOG ( ERR, "CManager : *** Failed Init WaitFailed RegEvent ***");
		return FALSE;
	}

	bRet = theListener.Run(NCS_PORT);
	if(!bRet) {
		LOG ( ERR, "CManager : *** Failed to Init listener ***");
		return FALSE;
	}

	DWORD dwPMSRet = thePMSConnector.Run();
	if (PMSC_ERR_OK != dwPMSRet)
		LOG (ERR, "CManager : !!!!!!!!!!!!!!!! PMS : Run() FAILED /// ErrorCode : ", dwPMSRet);
	else
		LOG (INF, "CManager : -----> PMS : Run() SUCCESS");

	return TRUE;
}

void CManager::Stop()
{
	TLock lo(this);
	theListener.Stop();
	theLRBHandler.Stop();
	thePMSConnector.Stop() ;
}

BOOL CManager::GetInternalIP()
{
	BOOL bRet = FALSE;

	char buf[1024];
	if(::gethostname(buf, 1024) != 0) return FALSE;
	HOSTENT* phe = ::gethostbyname(buf);
	if(!phe) return bRet;

	// 먼저 10.X.Y.Z의 형태가 아닌 놈을 찾는다.
	if(!phe->h_addr_list) {
		return bRet;
	}
	for(int i = 0; ; i++) {
		char * pa = phe->h_addr_list[i];
		//if nothing or list end
		if(pa == NULL) 
		{
			//nothing
			if(m_sIP.length() == 0)
			{
				LOG (INF, "CManager : ----CManager::Nothing!!!----");
				break;
			}
			//have only 2xx.xxx.xxx.xxx
			LOG (INF, "CManager : CManager Using IP is ", m_sIP);
			bRet = TRUE;
			break;
		}
		char * p = ::inet_ntoa(*(in_addr*)(pa));
		if(p) {
			if(::strncmp(p, "10.", 3) == 0) {
				m_sIP = p;
				LOG(INF, "CManager :CManager Using INTERNAL IP is ", m_sIP);
				bRet = TRUE;
				break;
			}
			else
				m_sIP = p;
		}
	}

	return bRet;
}

string& CManager::GetIP()
{
	return m_sIP;
}
void CManager::SetRegInitEvent(void)
{
	m_eRegInit.SetEvent(); 
}
