#include "stdafx.h"
#include "Link.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CLink

CLink::CLink()
{
	static DWORD s_dwIndex = 0;
	m_dwIndex = ::InterlockedIncrement((LPLONG)&s_dwIndex);
	m_pUser = NULL;
}

CLink::~CLink()
{
	ASSERT(m_pUser == 0);
}

void CLink::SetUser(CUser* pUser)
{
	m_pUser = pUser;
}