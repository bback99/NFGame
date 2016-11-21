#pragma once

#ifdef _USE_STATMSG

#define LOG_COUNT		1000		//// log for messages
#define LOG_TIME_SEC	2000		//// log second

#define CHECK_COUNT		(LOG_COUNT*2)		//// check for missing msg
#define CHECK_TIME_SEC	(LOG_TIME_SEC*2)	//// check second
#define CHECK_TIME_GAP	300000		//// delete msg with time gap

class CUSNType
{
public:
	long m_lUSN;
	long m_lType;

	CUSNType(long lUSN, long lType):m_lUSN(lUSN), m_lType(lType) {}

	bool operator< (const CUSNType& rdata) const {
		if(this->m_lUSN == rdata.m_lUSN) 
			return(this->m_lType < rdata.m_lType);
		return (this->m_lUSN < rdata.m_lUSN);
	}
};

class CMsgTimeData
{
public:
	DWORD m_dwTotalTime;
	DWORD m_dwMaxTime;
	DWORD m_dwMinTime;
	long  m_lCount;

	CMsgTimeData() : m_dwTotalTime(0), m_dwMaxTime(0), m_dwMinTime(0), m_lCount(0)	{}
	CMsgTimeData(DWORD dwAveTime, DWORD dwMaxTime, DWORD dwMinTime, long lCount) : \
		m_dwTotalTime(dwAveTime), m_dwMaxTime(dwMaxTime), m_dwMinTime(dwMinTime), m_lCount(lCount) {}
};

class CStatMsgMgr
{	
	IMPLEMENT_TISAFE(CStatMsgMgr)

typedef map<CUSNType, list<DWORD> > MAP_USERMSGTIME;
typedef map<long, CMsgTimeData> MAP_MSGTIME;

public:
	CStatMsgMgr(void);
	~CStatMsgMgr(void);

	void InitData(int nLogCnt = LOG_COUNT, DWORD dwLogTime = LOG_TIME_SEC, \
		int nCheckCnt = CHECK_COUNT, DWORD dwCheckTime = CHECK_TIME_SEC, DWORD dwCheckTimeGap = CHECK_TIME_GAP);
	BOOL SetModulerValue(int nData);
	int GetModulerValue() { return m_nModuler; }

	BOOL EnterUserTime(long lUSN, long lType);
	BOOL LeaveUserTime(long lUSN, long lType);
	BOOL DeleteUserTime(long lUSN, long lType);	

private:
	BOOL UpdateMsgTime(long lType, DWORD dwTime);
	void LOG(LPCTSTR fmt,...);
	BOOL LogFile(string strLog);

private:
	MAP_USERMSGTIME m_mapUserMsgTime;
	MAP_MSGTIME m_mapMsgTime;
	map<long, long> m_mapLossMsg;

	char m_path[MAX_PATH];

	__time64_t m_dwCheckTime;
	int m_nCheckCnt;

	__time64_t m_dwLogTime;
	int m_nLogCnt;

	int		m_nCheckCnt_Data;
	DWORD	m_dwCheckTime_Data; 
	DWORD	m_dwCheckTimeGap_Data;

	int		m_nLogCnt_Data;
	DWORD	m_dwLogTime_Data;

	int		m_nModuler;
};

#define ENTER_STAT_MSG(ADATA, BDATA) \
	if(ADATA%theStatMsgMgr.GetModulerValue() == 0)	theStatMsgMgr.EnterUserTime(ADATA, BDATA);
#define LEAVE_STAT_MSG(ADATA, BDATA) \
	if(ADATA%theStatMsgMgr.GetModulerValue() == 0)	theStatMsgMgr.LeaveUserTime(ADATA, BDATA);
#define DELETE_STAT_MSG(ADATA, BDATA) \
	if(ADATA%theStatMsgMgr.GetModulerValue() == 0)	theStatMsgMgr.DeleteUserTime(ADATA, BDATA);

extern CStatMsgMgr theStatMsgMgr;

#endif