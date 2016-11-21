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
	LONG m_lUserCount;		// ���� ����� ��
	LONG m_lLastUserCount;	// ������ CHS���� �˸� ����� ��
};

extern CUserTable theUserTable;

#endif //!UserTable_H
