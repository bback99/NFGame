//
// UserTable.h
//

#ifndef UserTable_H
#define UserTable_H

///////////////////////////////////////////////////////////////////////////////////
// CUserTable

class CUserTable
{
	IMPLEMENT_TISAFE(CUserTable)
public:
	CUserTable();
	virtual ~CUserTable();
public:
	void Add();
	void Remove();
public:
	BOOL IsNotiNeeded(LONG& lUserCount);
	void OnNotify(LONG lUserCount);
public:
	LONG GetUserCount() { TLock lo(this); return m_lUserCount; }
protected:
	LONG m_lUserCount;		// 현재 사용자 수
	LONG m_lLastUserCount;	// 직전에 CHS에게 알린 사용자 수
};

extern CUserTable theUserTable;

#endif //!UserTable_H
