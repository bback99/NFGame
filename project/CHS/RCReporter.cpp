//
// RCReporter.cpp
//

#include "stdafx.h"
#include "RCReporter.h"
#include "Control.h"
#include "LRBManager.h"

///////////////////////////////////////////////////////////////////////////////////
// CRCReporter

CRCReporter::CRCReporter()
{
	m_bWaitRCList = FALSE;
	m_lTimerCount = 0L;
}
 
CRCReporter::~CRCReporter()
{
}

BOOL CRCReporter::RunCRC()
{
	TLock lo(this);

	BOOL bRet = TRUE;
	bRet = bRet && m_timerCRC.Activate(GetChannelThreadPool(), this, 1000, 1000);
	VALIDATE(bRet);

	return TRUE;
}

BOOL CRCReporter::StopCRC()
{
	TLock lo(this);
	m_timerCRC.Deactivate();
	return TRUE;
}

STDMETHODIMP_(void) CRCReporter::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	TLock lo(this);
	if(m_timerCRC.IsHandle(hSignal))
	{
		if(m_bWaitRCList)
			m_lTimerCount++;

		if(m_lTimerCount > 2) 
		{
			m_lstUSN.clear();	// 이런 식으로 지우면.. 재수없는 놈은 최근..
			LOG(INF_UK, "CHS_CRCReporter"_LK, "재수 없는 놈이군..Waiting USN List를 지우다.. size : ", m_lstUSN.size());
			m_lTimerCount = 0L;
			m_bWaitRCList = FALSE;
		}
	}
}

void CRCReporter::GetSendUSNList(vector<LONG>& vecUSN)
{
	TLock lo(this);
	ForEachElmt(ListUSNT, m_lstUSN, it, jt)
	{
		vecUSN.push_back(*it);
	}
	m_lstUSN.clear();
	m_lTimerCount = 0L;
	m_bWaitRCList = FALSE;
}

BOOL CRCReporter::OnCRCInfoReq(LONG lUSN)
{
	TLock lo(this);

	return ISVALID_USN(lUSN);

	if(m_bWaitRCList) //채널 리스트를 이미 요구 했으며, 기다리고 있음.
	{
		m_lstUSN.push_back(lUSN);
		m_lTimerCount = 0L;
		return TRUE;
	}
	else //이경우 호출측에서 LB로 채널 리스트 요구 메세지를 보낸다.
	{
		m_bWaitRCList = TRUE;
		return FALSE;
	}
	return FALSE;
}

//
// 채널 리스를 요구한 User가 응답을 기다리지 않고 접속을 종료할때.
//
void CRCReporter::RemCRCUSN(LONG lUSN)
{
	TLock lo(this);

	ForEachElmt(ListUSNT, m_lstUSN, it, jt)
	{
		if((DWORD)lUSN == (*it))
			break;
	}
	if(it == m_lstUSN.end())
		return;

	m_lstUSN.erase(it);
}