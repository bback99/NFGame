// ErrorLog.h
/*
// create log file and write error message!! 

// Service�� ���۵ɶ� ��ü�� �����ǰ�.. ���۵� ��¥�� LOGyyyyMMddHHmm.mcu �� �̸��� ������ ����. 
// �����絥 RegLog(char * errMsg)�� ���ؼ� ����ð� milliseconds������ �޼����� ���.
// RegLog�� overloading..
// ���ϴ� ��츸 log�� ���� �� �ִ� flag��.. registery���.
#ifndef __ERRLOG_H
#define __ERRLOG_H

#include "stdafx.h"

class CErrorLog
{
public:
	CErrorLog(BOOL bPL = FALSE);
 	~CErrorLog();
	void _GLSLOG(int iLevel, char *buf);
	void __cdecl LOG(int iLevel, LPCSTR sLogKey, LPCSTR fmt,...);
	int SetIsLog();
	void NewLogFile();

//	void RegLog(int iLevel, char * errMsg);
//	void RegLog(int iLevel, const long errCode, char * errMsg = "");
//	void RegLog(int iLevel, const long errCode, const long errExtra, char * errMsg = "");
//	void RegLog(int iLevel, const char * str_data, const long errCode, char * errMsg);
//	void RegDouble(int iLevel, long value, double errCode, char * errMsg = "");
protected:
	char m_path[100];
	long m_lDate;

	int m_iLevel;
	string m_sComputerName;
	CComAutoCriticalSection m_CS;
};

extern CErrorLog theErr;

#if defined _PL_INFO
extern CErrorLog thePL;
#endif //_PL_INFO

#endif
*/
#ifndef __NLOG_H__
#define __NLOG_H__

#include <windows.h>
#include <GUtil.h>

#include <string>
#include <iostream>
#include <sstream>

class ServerLog;
extern ServerLog theLog;

using namespace std;


#define LEVELNUM	5
enum LOGLEVEL { 
	INF = 0,	ERR,	WAR,	DEV,	EVE,
	INF_UK,		ERR_UK,	WAR_UK,	DEV_UK,	EVE_UK
};


class logstream : public ostringstream
{
public:	
	logstream&  operator<< (const wstring& str)
	{
		*((ostringstream*)(this)) << (wstr2str(str));
		return *this;
	}
	logstream& operator<< (const  LPCWSTR sz)
	{
		wstring wstr(sz);
		*((ostringstream*)(this)) <<(wstr2str(wstr));
		return *this;
	}

	template <class T1>
		logstream& operator<< (T1 t1)
	{
		*((ostringstream*)(this)) << (t1);
		return *this;
	}
};


enum LOGFILETER { FILTER_EVENT = 0x04, FILTER_CONSOL = 0x02, FILTER_FILE = 0x01};

#define LEVEL_FILTER(level)	{ if (!m_fFilter[level]) return;}

class ServerLog
{
public:
	ServerLog();
	virtual ~ServerLog() { Final();}

	string  Init (BOOL fSvcMode, LPCSTR szSvcName,LPCSTR szINI);	
	wstring Init (BOOL fSvcMode, LPCWSTR szSvcName,LPCWSTR szINI);
	void	Final();
	void	SetLevelFilter(LOGLEVEL level, LPCSTR  szFilter);
	void	SetLevelFilter(LOGLEVEL level, LPCWSTR szFilter);

	template <class T1>
		inline void Put (LOGLEVEL level, T1 t1) 
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 ;		
		Log(level, stream.str().c_str());
	}


protected:
	// �α� ���͸� ����
	BOOL		m_fSvcMode;					// ���� ����� ��� TRUE. Init()�Լ��� �з����ͷ� ����
	DWORD		m_fFilter[LEVELNUM];		// �α� ������ ���͸� ��. INI ���Ͽ��� �о��


	// �α� ���� ���� ���� ����
	BOOL		m_fPerThread;				// Thread ���� �α� ����� ��� TRUE. INI ���Ͽ��� �о��	
	string		m_strLogFolder;				// �α����� ������ ���̽� ����. INI ���Ͽ��� �о��
	long		m_sizeMax;					// �α������� �ִ� ����Ʈ ũ��. INI ���Ͽ��� �ް�����Ʈ ������ ���
	BOOL		m_fUseBufferedLog;			// buffered log ��� ����
	vector<string> m_vLogStartMsg;			// �α����� ������ �α� ���� �޽����� �����ϰ� ���� ��Ʈ��

	// �α�����ȭ ���� ���� ���õ� ����
	char		m_szSvcName[50];			// ���� ����
	char		m_szCompName[50];			// Computer ����
	char		m_aszLogKey[LEVELNUM*2][60];	// �α� ������ ����Ʈ �α�Ű


	// �α� ���� ����
	struct LOGFILE {		// �ڵ鸵 �Ǵ� �α� ���� ���� ����
		FILE *	fp;			// ���� ������
		long	size;		// ���Ͽ� ������ �� ����Ʈ ũ��		
		time_t	tomorrow;	// ������ 0�� 0�� 0��
	};
	LOGFILE*			m_file;			// m_fPerThead == FALSE ��  ��� ���� ������ ���Ͽ� ���� ����
	DWORD				m_dwTLSSlot;	// Thread���� �α׸� ����� ��� ���� ������ ���Ͽ� ���� ���� ������ ���� TLS Index
	vector<LOGFILE *>	m_vTLSSlot;		// Thread���� �α׸� ����� ��� ���μ��� ����� LOGFILE ��ü�� �����ϱ� ���� ����Ʈ


	GCRITICAL_SECTION m_cs;	 // �Ӱ迵���� ���� ����ȭ ��ü

protected:
	void Log			(LOGLEVEL level, LPCSTR szMsg);	
	void LogAtConsol    (LPCSTR szMsg);	
	void LogAtEvent		(LPCSTR szMsg);
	void LogAtFile		(LPCSTR szMsg);
	void LogAtFileLock	(time_t now, LPCSTR szHeader, LPCSTR szMsg);
	void LogAtFileTLS	(time_t now, LPCSTR szHeader, LPCSTR szMsg);	
	LOGFILE* CreateFile	(time_t now, LPCSTR szHeader, LOGFILE* file = NULL, BOOL fSizeMax=FALSE);

	int	 ReadConfigure	(LPCSTR szINIFile);

public:
	///////////////////////////////////////////////////////////////////////////////
	//  ���ĺ��ʹ� Put ��� �Լ��� Template Overloading				
	///////////////////////////////////////////////////////////////////////////////

	template <class T1,  class T2>
		void Put (LOGLEVEL  level, T1 t1, T2 t2)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3>	
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4 t4)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4, class T5>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4 << t5;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4 << t5 << t6;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4 << t5 << t6 << t7;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  class T9>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5, class T6,  class T7,  class T8,  class T9,  class T10>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  
	class T6,  class T7,  class T8,  class T9,  class T10, class T11>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4  t4,  T5  t5, 
		T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4 << t5 
			<< t6 << t7 << t8 << t9 << t10 << t11;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6, 
	class T7,  class T8,  class T9,  class T10, class T11, class T12>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4  t4,  T5  t5,  T6  t6, 
		T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3 << t4 <<  t5  << t6 
			<< t7 << t8 << t9 << t10 << t11 << t12;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  
	class T7,  class T8,  class T9,  class T10, class T11, class T12, class T13>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3 t3, T4  t4,  T5  t5,  T6  t6,
		T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3  << t4 << t5  << t6 
			<< t7 << t8 << t9 << t10 << t11 << t12 << t13;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  
	class T8,  class T9,  class T10, class T11, class T12, class T13, class T14>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7, 
		T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13, T14 t14)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3  << t4  << t5  << t6  << t7 
			<< t8 << t9 << t10 << t11 << t12 << t13 << t14;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  
	class T8,  class T9,  class T10, class T11, class T12, class T13, class T14, class T15>
		void Put (LOGLEVEL level,  T1 t1, T2 t2, T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7, 
		T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2 << t3  << t4  << t5  << t6  << t7 
			<< t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  
	class T9,  class T10, class T11, class T12, class T13, class T14, class T15, class T16>
		void Put (LOGLEVEL level,  T1 t1, T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8, 
		T9 t9, T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2  << t3  << t4  << t5  << t6  << t7  << t8 
			<< t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  
	class T9,  class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17>
		void Put (LOGLEVEL level,  T1 t1, T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8, 
		T9 t9, T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1 << t2  << t3  << t4  << t5  << t6  << t7  << t8 
			<< t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8, class T9,
	class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18>
		void Put (LOGLEVEL level,  T1 t1,   T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  T9  t9, 
		T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, T18 t18)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  << t9 
			<< t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  class T9,
	class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  T9  t9, 
		T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, T18 t18, T19 t19)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  << t9 
			<< t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  class T9,  class T10, 
	class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  T9  t9,  T10 t10, 
		T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, T18 t18, T19 t19, T20 t20)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  << t9  << t10 
			<< t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << t20;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  
	class T8,  class T9,  class T10, class T11, class T12, class T13, class T14, 
	class T15, class T16, class T17, class T18, class T19, class T20, class T21>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  
		T8  t8,  T9  t9,  T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, 
		T15 t15, T16 t16, T17 t17, T18 t18, T19 t19, T20 t20, T21 t21)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  
			<< t8  << t9  << t10 << t11 << t12 << t13 << t14 
			<< t15 << t16 << t17 << t18 << t19 << t20 << t21;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  
	class T8,  class T9,  class T10, class T11, class T12, class T13, class T14, 
	class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  
		T8  t8,  T9  t9,  T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, 
		T15 t15, T16 t16, T17 t17, T18 t18, T19 t19, T20 t20, T21 t21, T22 t22)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  
			<< t8  << t9  << t10 << t11 << t12 << t13 << t14 
			<< t15 << t16 << t17 << t18 << t19 << t20 << t21 << t22;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  
	class T8,  class T9,  class T10, class T11, class T12, class T13, class T14, class T15, 
	class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  
		T8  t8,  T9  t9,  T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, 
		T16 t16, T17 t17, T18 t18, T19 t19, T20 t20, T21 t21, T22 t22, T23 t23)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  
			<< t8  << t9  << t10 << t11 << t12 << t13 << t14 << t15
			<< t16 << t17 << t18 << t19 << t20 << t21 << t22 << t23;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  
	class T9,  class T10, class T11, class T12, class T13, class T14, class T15, class T16, 
	class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  
		T9  t9,  T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, 
		T17 t17, T18 t18, T19 t19, T20 t20, T21 t21, T22 t22, T23 t23, T24 t24)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  
			<< t9  << t10 << t11 << t12 << t13 << t14 << t15 << t16 
			<< t17 << t18 << t19 << t20 << t21 << t22 << t23 << t24;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  
	class T9,  class T10, class T11, class T12, class T13, class T14, class T15, class T16, 
	class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  
		T9  t9,  T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, 
		T17 t17, T18 t18, T19 t19, T20 t20, T21 t21, T22 t22, T23 t23, T24 t24, T25 t25)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  
			<< t9  << t10 << t11 << t12 << t13 << t14 << t15 << t16 
			<< t17 << t18 << t19 << t20 << t21 << t22 << t23 << t24 << t25;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  
	class T9,  class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, 
	class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  
		T9  t9,  T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, 
		T18 t18, T19 t19, T20 t20, T21 t21, T22 t22, T23 t23, T24 t24, T25 t25, T26 t26)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  
			<< t9  << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 
			<< t18 << t19 << t20 << t21 << t22 << t23 << t24 << t25 << t26;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  class T9,  
	class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, 
	class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  T9  t9,  
		T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, T18 t18, 
		T19 t19, T20 t20, T21 t21, T22 t22, T23 t23, T24 t24, T25 t25, T26 t26, T27 t27)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  << t9  
			<< t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 
			<< t19 << t20 << t21 << t22 << t23 << t24 << t25 << t26 << t27;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  class T9,  
	class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, 
	class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  T9  t9,  
		T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, T18 t18, 
		T19 t19, T20 t20, T21 t21, T22 t22, T23 t23, T24 t24, T25 t25, T26 t26, T27 t27, T28 t28)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  << t9  
			<< t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 
			<< t19 << t20 << t21 << t22 << t23 << t24 << t25 << t26 << t27 << t28;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  class T9,  
	class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, 
	class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  T9  t9, 
		T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, T18 t18, T19 t19, 
		T20 t20, T21 t21, T22 t22, T23 t23, T24 t24, T25 t25, T26 t26, T27 t27, T28 t28, T29 t29)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  << t9  
			<< t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 
			<< t20 << t21 << t22 << t23 << t24 << t25 << t26 << t27 << t28 << t29;
		Log(level, stream.str().c_str());
	}

	template <class T1,  class T2,  class T3,  class T4,  class T5,  class T6,  class T7,  class T8,  class T9, class  T10, 
	class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, 
	class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30>
		void Put (LOGLEVEL level,  T1  t1,  T2 t2,   T3  t3,  T4  t4,  T5  t5,  T6  t6,  T7  t7,  T8  t8,  T9  t9,  T10 t10, 
		T11 t11, T12 t12, T13 t13, T14 t14, T15 t15, T16 t16, T17 t17, T18 t18, T19 t19, T20 t20, 
		T21 t21, T22 t22, T23 t23, T24 t24, T25 t25, T26 t26, T27 t27, T28 t28, T29 t29, T30 t30)
	{
		LEVEL_FILTER(level%5);
		logstream stream;
		stream << m_aszLogKey[level] << t1  << t2  << t3  << t4  << t5  << t6  << t7  << t8  << t9  << t10 
			<< t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << t20 
			<< t21 << t22 << t23 << t24 << t25 << t26 << t27 << t28 << t29 << t30 ;
		Log(level, stream.str().c_str());
	}
};
#endif // __NLOG_H__
