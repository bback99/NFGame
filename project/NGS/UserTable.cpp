//
// UserTable.cpp
//

#include "stdafx.h"
#include "Common.h"
#include "UserTable.h"

///////////////////////////////////////////////////////////////////////////////////
// CUserTable

CUserTable theUserTable;

CUserTable::CUserTable() : m_lUserCount(0), m_lLastUserCount(0)
{
}

CUserTable::~CUserTable()
{
}

void CUserTable::Add()
{
	TLock lo(this);
	m_lUserCount++;
}

void CUserTable::Remove()
{
	TLock lo(this);
	m_lUserCount--;
	if (m_lUserCount < 0) m_lUserCount = 0;
}

BOOL CUserTable::IsNotiNeeded(LONG& lUserCount)
{
	TLock lo(this);
	lUserCount = m_lUserCount;
	return ((m_lUserCount <= (m_lLastUserCount - USER_DIFF_COUNT)) || (m_lUserCount >= m_lLastUserCount + USER_DIFF_COUNT));
}

void CUserTable::OnNotify(LONG lUserCount)
{
	TLock lo(this);
	m_lLastUserCount = lUserCount;
}

