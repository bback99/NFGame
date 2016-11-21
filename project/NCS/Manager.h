//
//Manager.h
//
/**********************************************************
NCS Initalize Manager
-bback99
***********************************************************/

#ifndef _CMANAGER_
#define _CMANAGER_

class CManager
{
public:
	IMPLEMENT_TISAFE(CManager)
public:
	CManager();
	virtual~ CManager();
public:
	BOOL Init();
	BOOL GetInternalIP();
	BOOL Start();
	void Stop();
	string& GetIP();
private:
	unsigned long m_ulThreadCount;
	string m_sIP;

public:
	static HTHREADPOOL m_hThreadPool;
	static HTHREADPOOL GetThreadPool() 
	{
		ASSERT(m_hThreadPool);
		return m_hThreadPool;
	}

	GEvent	m_eRegInit;
	void SetRegInitEvent(void);
};

extern CManager theManager;

#endif //_CMANAGER_

