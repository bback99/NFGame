//
// Reporter.cpp
//

#include "stdafx.h"
#include "Reporter.h"
#include "Control.h"
#include "ChannelDir.h"
#include "CHSInfoDir.h"
#include "LRBHandler.h"

///////////////////////////////////////////////////////////////////////////////////
// CReporter

CReporter theReporter;

CReporter::CReporter()
{
}
 
CReporter::~CReporter()
{
}

BOOL CReporter::RunReport()
{
	TLock lo(this);

	BOOL bRet = TRUE;
	bRet = bRet && m_timerCHSInfo.Activate(GetChannelThreadPool(), this, 3000, 1000);
	VALIDATE(bRet);

	return TRUE;
}

BOOL CReporter::StopReport()
{
	TLock lo(this);
	m_timerCHSInfo.Deactivate();
	return TRUE;
}

STDMETHODIMP_(void) CReporter::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if(hSignal == 0)
	{
	}
	else
	{
		if(m_timerCHSInfo.IsHandle(hSignal))
		{
			if(theControl.GetBootState() != CHS_RUN) 
			{
				theControl.RunControl();
			}
			else  
			{
				OnSendCHSInfo();
			}
		}
		else
		{
//			TLock lo(this);
//			TBase::OnSignal(hSignal, wParam, lParam);
		}
	}
}

void CReporter::SetAllDiffFlag()
{
	TLock lo(this);
	theCHSInfoDir.SetAllFlags();
}

BOOL CReporter::ChangeInterval(DWORD dwDue, DWORD dwPeriod)
{
	// need no TLock lo(this)
	return m_timerCHSInfo.Change(dwDue, dwPeriod);
}

void CReporter::OnSendCHSInfo()
{
	NSAPList lstNSAPGLS;
	lstNSAPGLS.clear();

	ChannelUpdateInfoList lst;
#ifdef USE_DIFFREPORT
	DWORD dwCount = theCHSInfoDir.GetCHSInfoList(lst, FALSE, TRUE);
#else
 	DWORD dwCount = theCHSInfoDir.GetCHSInfoList(lst, TRUE, TRUE);
#endif
	{
#ifdef USE_DIFFREPORT
 		if(dwCount > 0)
#endif
		{
			LRBAddress & lrbAddr = theLRBHandler.GetMyAddress();
			PayloadCHSNCS pld(PayloadCHSNCS::msgCHSInfoNtf_Tag, MsgCHSNCS_CHSInfoNtf(lst, lrbAddr));
			theLRBHandler.SendToAllNCS(pld);
		}
	}
}
