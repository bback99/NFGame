//
// Control.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Control.h"
#include "LRBHandler.h"

#include "Config.h"
//#include "PeerCtl.h"

///////////////////////////////////////////////////////////////////////////////////
// CControl

CControl theControl;

CControl::CControl()
{
	m_nReconnectCnt = 0;
}

CControl::~CControl()
{
}

STDMETHODIMP_(void) CControl::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if(hSignal == 0)
	{
		if(wParam == CONTROL_RECOVER_EX)
		{
			//BOOL bRet = RecoverEx();
			if(m_nReconnectCnt++ > 5)
				Stop();

			BOOL bRet = RunEx();
			if(!bRet)
			{
				::Sleep(20000);
				theControl.QueueSignal( GetThreadPool(), 0, CONTROL_RECOVER_EX, 0);
			}
		}
		else if(wParam == CONTROL_STOP)
		{
			Stop();
		}
	}
}

BOOL CControl::Stop()
{

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// for Master/Slave Mode

static int g_iRetry1 = 0;

BOOL CControl::RunEx()
{
	BOOL bRet = theLRBHandler.Run(::GetThreadPool());
	if(!bRet)
	{
		theLRBHandler.Stop();
		::Sleep(100);
		::XsigQueueSignal(GetThreadPool(), this, 0, CONTROL_RECOVER_EX, 0);
		LOG(INF_UK, "NAS_CONTROL, ### LRBHandler Run Failed !!! ###");
		return FALSE;
	}
	LOG(INF_UK, "NAS_CONTROL, ##### LRBHandler Run Success!!! #####");
	
	::Sleep(1000);
	return TRUE;
}

BOOL CControl::RecoverEx()
{	
	return TRUE;
}

BOOL CControl::GetComputerName(string & comp_name)
{
	char buff[100] = {0, };
	DWORD dwLen = sizeof(buff);
	if(!::GetComputerNameA(buff, &dwLen)) {
		LOG(ERR_UK, "NAS_CONTROL, +++++ fail GetCoumpterName +++++");
		return FALSE;
	}
	comp_name = buff;
	return TRUE;
}
CConfPath::CConfPath()
{
	if(!SetConfPath(CONFIG_FILENAME))
		memset(m_sConfigPath, 0, sizeof(m_sConfigPath));
}

BOOL CConfPath::SetConfPath(const char* strfilename)
{
#ifdef _VERSION_JAPAN_
	/////////// 설정파일 위치 지정 ////////////
	string strdir;
	char strDir[MAX_PATH];
	if(!::GetModuleFileNameA(NULL, strDir, MAX_PATH))
		return FALSE;
	strdir = strDir;
	int nIdx = strdir.rfind("\\");
	strdir = strdir.substr(0, nIdx+1);
	sprintf(m_sConfigPath, "%sConf\\%s", strdir.c_str(), strfilename);	
	///////////////////////////////////////////
#else
	strcpy(m_sConfigPath, strfilename);
#endif
	return TRUE;
}