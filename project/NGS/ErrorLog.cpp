// ErrorLog.cpp
/*
#include "stdafx.h"
#include "ErrorLog.h"
#include "RoomTable.h"

#ifdef _GLSLCS
#include <direct.h>
#endif

CErrorLog theErr;

#if defined _PL_INFO
CErrorLog thePL(TRUE);
#endif //_PL_INFO

CErrorLog::CErrorLog(BOOL bPL)
{
	SYSTEMTIME sys;
	memset(&sys, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&sys);
	memset(m_path, 0, sizeof(m_path));
	m_lDate = sys.wDay;
	_mkdir("C:\\LOG");
	_mkdir("C:\\LOG\\GLS");

	char szComputerName[1024] = {0x00};
	DWORD dwSize = sizeof(szComputerName);
	if (!::GetComputerNameA(szComputerName, &dwSize))
		m_sComputerName = "GetComputerNameFailed";
	else
		m_sComputerName = szComputerName;

	sprintf(m_path, "C:\\LOG\\GLS\\GLS_%s_%04d%02d%02d_%02d%02d%02d.txt", m_sComputerName.c_str(), sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

	if(bPL) sprintf(m_path, "C:\\LOG\\GLS_PLLOG%04d%02d%02d_%02d%02d.txt", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute);

	FILE * fh = fopen(m_path, "a" );
	if(fh == NULL)
	{
		return;
	}

	fprintf(fh, "#time,logkey,logdata\n");
	fclose( fh );

	m_iLevel = SetIsLog();
}

CErrorLog::~CErrorLog()
{
}

void __cdecl CErrorLog::LOG(int iLevel, LPCSTR sLogKey, LPCSTR fmt,...)
{
	if (m_iLevel < iLevel)
		return;

	char buf[1024] = {0, };
	va_list vl;
	va_start(vl, fmt);
	_vsnprintf(buf, 1024, fmt, vl);
	buf[1024-1] = 0;
	va_end(vl);

	SYSTEMTIME sys;
	memset(&sys, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&sys);
	m_CS.Lock();

	BOOL bIsNew = FALSE;

	if (m_lDate != sys.wDay)
	{
		sprintf(m_path, "C:\\LOG\\GLS\\GLS_%s_%04d%02d%02d_%02d%02d%02d.txt", m_sComputerName.c_str(), sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
		m_lDate = sys.wDay;
		bIsNew = TRUE;
	}

	FILE * fh = fopen(m_path, "a" );
	if (fh == NULL)
	{
		m_CS.Unlock();
		return;
	}

	if (bIsNew)
		fprintf(fh, "#time,logkey,logdata\n");

	fprintf(fh, "@@@%02d:%02d:%02d.%03d,%s,%s\n", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, sLogKey, buf);
	fclose( fh );
	m_CS.Unlock();

	buf[0] = 0;
}

void CErrorLog::_GLSLOG(int iLevel, char *buf)
{
}

int CErrorLog::SetIsLog()
{
	return 2;
}

void CErrorLog::NewLogFile()
{
	SYSTEMTIME sys;
	memset(&sys, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&sys);
	memset(m_path, 0, sizeof(m_path));

	_mkdir("C:\\LOG");
	_mkdir("C:\\LOG\\GLS");
	sprintf(m_path, "C:\\LOG\\GLS\\GLS_LOG%04d%02d%02d_%02d%02d.txt", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute);
}
*/

#include "stdafx.h"

#include <sys/timeb.h>
#include <time.h>
#include "GService.h"
#include "ErrorLog.h"


ServerLog theLog;

// 개발 기간 중 디버깅 모드로 돌리거나 콘솔에서 돌리는 경우의 로그 유실 방지 루틴
// INI에 버퍼관련 설정키가 없으면 기본설정이 매 로그마다  flush하는 것이므로 
// 버퍼사용 옵션을 켜지 않는 일반적인 상황에서는 필요없는 루틴임
static BOOL WINAPI HandlerRoutineFlush( DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT :
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:		
		fflush(NULL);
	}
	return FALSE;
}


ServerLog::ServerLog()
{
	m_dwTLSSlot			= TLS_OUT_OF_INDEXES;
	m_fPerThread		= FALSE;
	m_fUseBufferedLog	= FALSE;
	m_file				= NULL;
	m_vLogStartMsg.clear();
	memset (m_szSvcName,  0x00, sizeof(m_szSvcName));	
	memset (m_szCompName, 0x00, sizeof(m_szCompName));
	memset (m_aszLogKey,  0x00, sizeof(m_aszLogKey));
	SetConsoleCtrlHandler(HandlerRoutineFlush,TRUE);
}

string ServerLog::Init (BOOL fSvcMode, LPCSTR  szSvcName, LPCSTR  szINIFile)
{	
	m_fSvcMode = fSvcMode;
	strcpy(m_szSvcName, szSvcName);

	// INI 파일에서 설정값 가져오기
	ReadConfigure(szINIFile);

	// 로그 폴더 만들기
#ifdef UNICODE
	wstring tsLogFolder = str2wstr(m_strLogFolder + "\\");
#else
	string tsLogFolder = m_strLogFolder + "\\";
#endif
	if (!GUtil::MakeDirectory(tsLogFolder))
	{
		string strErrMsg = "MakeFolder Fail!! : ";
		strErrMsg += m_strLogFolder;
		return strErrMsg;
	}

	// TLS setting
	if (m_fPerThread)
	{
		if ((m_dwTLSSlot = TlsAlloc ())== TLS_OUT_OF_INDEXES)
			return "TlsAlloc() fail. TSL_OUT_OF_INDEX";
	}

	// LogKey String Setting	
	sprintf(m_aszLogKey[0], "%s_Inf,", m_szSvcName);
	sprintf(m_aszLogKey[1], "%s_Err,", m_szSvcName);
	sprintf(m_aszLogKey[2], "%s_War,", m_szSvcName);
	sprintf(m_aszLogKey[3], "%s_Dev,", m_szSvcName);
	sprintf(m_aszLogKey[4], "%s_Evt,", m_szSvcName);
	for (int i = LEVELNUM; i < LEVELNUM*2 ; i++)
		strcpy(m_aszLogKey[i], "");

	// Computer 이름 얻기
	DWORD dwLen = sizeof(m_szCompName);
	if(!::GetComputerNameA(m_szCompName, &dwLen)) {
		strcpy(m_szCompName, "unknown-computer");
	}

	// 서버 시작 로그 남기기. 게임시스템팀 요구사항
	char szStartLog[100];
	sprintf(szStartLog, "%s_Start, Server started", m_szSvcName);
	LogAtFile(szStartLog);

	// 서버 시작 조건(configure 정보) 남기기. 게임플랫폼팀 요구사항
	for (vector<string>::iterator i = m_vLogStartMsg.begin(); i != m_vLogStartMsg.end(); i++)
	{
		char szStartConf[200];
		sprintf(szStartConf,"%s_LogInfo,%s", m_szSvcName, (*i).c_str());
		LogAtFile(szStartConf);
		if (!m_fSvcMode)
			LogAtConsol((*i).c_str());
	}
	m_vLogStartMsg.clear();

	return string("");
}

wstring ServerLog::Init (BOOL fSvcMode, LPCWSTR szSvcName, LPCWSTR szINIFile)
{
	wstring wstrSvcName(szSvcName);
	wstring wstrINIFile(szINIFile);
	string  strSvcName = wstr2str(wstrSvcName);
	string  strINIFile = wstr2str(wstrINIFile);
	LPCSTR  szSvcNameA = strSvcName.c_str();
	LPCSTR  szINIFileA = strINIFile.c_str();

	string str = Init(fSvcMode, szSvcNameA, szINIFileA);

	return str2wstr(str);
}


//////// Write Log Message	////////////////////
void ServerLog::Log (LOGLEVEL level, LPCSTR szMsg)
{
	level	= static_cast<LOGLEVEL>(level%5);
	DWORD fFilter = m_fFilter[level];

	if (fFilter & FILTER_FILE)
		LogAtFile(szMsg);
	if (fFilter & FILTER_CONSOL)
		LogAtConsol(szMsg);
	if (fFilter & FILTER_EVENT)
		LogAtEvent(szMsg);
}

void ServerLog::LogAtConsol  (LPCSTR szMsg)
{	
	cout << szMsg << endl;
}

void ServerLog::LogAtEvent (LPCSTR szMsg)
{	
#ifdef UNICODE	
	wstring tsMsg   = str2wstr(string(szMsg));	
#endif
	GService *pService = GetCurrentService();


	if (pService && m_fSvcMode)
		pService->LogEvent(tsMsg.c_str());
	else
	{
		HANDLE hEventSource = RegisterEventSourceA(NULL, m_szSvcName);
		if (hEventSource != NULL)
		{	
			::ReportEventA(
				hEventSource,
				EVENTLOG_INFORMATION_TYPE,
				0,
				0, // EventID 
				NULL, 
				1,
				0,
				&szMsg,
				NULL
				);
			DeregisterEventSource(hEventSource);
		}
	}
}

void ServerLog::LogAtFile(LPCSTR szMsg)
{		
	struct tm*		now;
	struct _timeb	nowf;
	_ftime(&nowf);
	now = localtime(&nowf.time);


	char szheader[50];
	sprintf(szheader, "%s,%s,%d.%02d.%02d %02d:%02d:%02d.%03d", 
		m_szCompName,m_szSvcName,
		now->tm_year+1900, now->tm_mon+1, now->tm_mday,
		now->tm_hour, now->tm_min, now->tm_sec, nowf.millitm);

	if (!m_fPerThread)
		LogAtFileLock(nowf.time, szheader, szMsg);		
	else
		LogAtFileTLS (nowf.time, szheader, szMsg);
}

void ServerLog::LogAtFileLock(time_t now, LPCSTR szHeader, LPCSTR szMsg)
{
	GCSLOCK lock(&m_cs);

	if (m_file == NULL)
		m_file = CreateFile(now, szHeader);
	else if (m_file->tomorrow < now)
		m_file = CreateFile(now, szHeader, m_file);
	else if (m_sizeMax < m_file->size)
		m_file = CreateFile(now, szHeader, m_file, TRUE);

	if (m_file)
	{		
		m_file->size += (fprintf(m_file->fp, "%s,%s\n", szHeader, szMsg)+1);	
		if (!m_fUseBufferedLog)
			fflush(m_file->fp);
	}

}

void ServerLog::LogAtFileTLS(time_t now, LPCSTR szHeader, LPCSTR szMsg)
{
	LOGFILE *file = static_cast<LOGFILE*>(TlsGetValue(m_dwTLSSlot));

	if (file == NULL)
	{
		file = CreateFile(now, szHeader);
		m_vTLSSlot.push_back(file);
		TlsSetValue(m_dwTLSSlot, file);
	}
	else if (file->tomorrow < now)
		file = CreateFile(now, szHeader, file);
	else if ( m_sizeMax < file->size)
		file = CreateFile(now, szHeader, file, TRUE);

	if (file)
	{
		file->size += (fprintf(file->fp, "%s,%s\n", szHeader, szMsg)+1);
		if (!m_fUseBufferedLog)
			fflush(file->fp);
	}	
}


////////  Create File	///////////////////////
static long GetFileSize(LPCSTR szFilePath)
{
	DWORD dwSizeHigh = 0;
	DWORD dwSize = 0;	
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile  = ::CreateFileA(szFilePath, 0x00, 0x00, NULL, OPEN_EXISTING, 0x00, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	dwSize = ::GetFileSize(hFile, &dwSizeHigh);

	if (LONG_MAX < dwSize)
		return LONG_MAX;

	if (0 < dwSizeHigh)
		return LONG_MAX;

	return dwSize;
}
ServerLog::LOGFILE* ServerLog::CreateFile(time_t nowt, LPCSTR szHeader, LOGFILE *file, BOOL fSizeMax)
{
	FILE *	fp = NULL;

	struct tm*	now;
	now = localtime(&nowt);

	// 파일 생성	
	char szTime[50], szMilSec[50];

	if (fSizeMax)
	{
		struct _timeb	nowf;
		_ftime(&nowf);
		sprintf(szMilSec,"_%02d%02d%02d%03d", now->tm_hour, now->tm_min, now->tm_sec, nowf.millitm);
	}
	else
		szMilSec[0]=0x00;


	sprintf(szTime,"%d%02d%02d%s.txt",
		now->tm_year+1900, now->tm_mon+1, now->tm_mday, szMilSec);

	string strPath = m_strLogFolder + "\\";
	strPath += m_szSvcName;
	strPath += "_";
	strPath += m_szCompName;
	strPath += "_";
	if (m_fPerThread)
	{
		char szThreadID[20];
		sprintf(szThreadID, "0X%08X_", GetCurrentThreadId());
		strPath += szThreadID;
	}
	strPath += szTime;				

	long lFileSize = GetFileSize(strPath.c_str());
	if (lFileSize == LONG_MAX || lFileSize >= m_sizeMax)
		return CreateFile(nowt, szHeader, file, TRUE);

	if ((fp = fopen(strPath.c_str(), "a")) == NULL)
	{	
		if (m_fSvcMode)
			GetCurrentService()->LogEvent(_T("로그파일 열기에 실패 했습니다."));
		else
			printf("로그파일 열기에 실패했습니다.");
		return NULL;
	};	

	// set m_tomorrow start time
	struct tm date;
	const time_t dayseconds = 24*60*60;
	time_t tomorrow = time(NULL) + dayseconds;

	memcpy(&date, localtime(&tomorrow), sizeof(date));

	date.tm_hour = (date.tm_sec = (date.tm_min = 0x00));
	tomorrow = mktime(&date);	

	if (file == NULL)
		file = new LOGFILE;
	else if (file->fp)
		fclose(file->fp);

	file->fp		= fp;
	file->tomorrow	= tomorrow;
	file->size		= lFileSize;

	return file;
}


void ServerLog::SetLevelFilter(LOGLEVEL level, LPCSTR szFilter)
{	
	if (strstr(szFilter, "E"))
		m_fFilter[level] |= FILTER_EVENT;
	if (strstr(szFilter, "C"))
		m_fFilter[level] |= FILTER_CONSOL;
	if (strstr(szFilter, "F"))
		m_fFilter[level] |= FILTER_FILE;
}

void ServerLog::SetLevelFilter(LOGLEVEL level, LPCWSTR szFilter)
{	
	if (wcsstr(szFilter, _T("E"))) // ? UNICODE 빌드일때만 올바르게 동작?
		m_fFilter[level] |= FILTER_EVENT;
	if (wcsstr(szFilter, _T("C"))) // ? UNICODE 빌드일때만 올바르게 동작?
		m_fFilter[level] |= FILTER_CONSOL;
	if (wcsstr(szFilter, _T("F"))) // ? UNICODE 빌드일때만 올바르게 동작?
		m_fFilter[level] |= FILTER_FILE;
}


BOOL ServerLog::ReadConfigure (LPCSTR szINIFile)
{	
	LPCSTR const LOGROOT		= "c:\\Log";
	LPCSTR const LOGSECTION		= "LOG";
	LPCSTR const LOGFOLDER		= "BaseLogFolder";
	LPCSTR const LOGPERTHREAD	= "LogPerThread";
	LPCSTR const MAXFILESIZE	= "MaxFileSize";
	LPCSTR const USEBUFFER		= "UseBufferedLog";


	LPCSTR const KEY[LEVELNUM] = {
		"InformationLog", "ErrorLog", "WarningLog",
			"DevelopLog",	  "EventLog"
	};
	char  FILTER[LEVELNUM][4] = { "XCF",	"ECF",	"XCF", "XXX",	"XCF"};

	m_vLogStartMsg.clear();
	string strTmp;
	strTmp = string("Server started with ") + szINIFile;
	m_vLogStartMsg.push_back(strTmp);


	for (int i = 0; i < LEVELNUM; i++)
	{		
		m_fFilter[i] = 0x00;
		char szBuf[4] = {0x00,};
		//::GetPrivateProfileString(LOGSECTION, KEY[i], FILTER[i], FILTER[i], 4, szINIFile);
		::GetPrivateProfileStringA(LOGSECTION, KEY[i], "", szBuf, 4, szINIFile);
		if (strlen(szBuf) == 0)
		{	
			strTmp  = "Not Found Key \"";
			strTmp +=  KEY[i];
			strTmp += "\" In [LOG] Section. Using Default Value\"";
			strTmp +=  FILTER[i];
			strTmp += "\"";
		}
		else
		{
			szBuf[3] = 0x00;
			strcpy(FILTER[i], szBuf);
			strTmp  = KEY[i];
			strTmp += " = ";
			strTmp += FILTER[i];
		}
		m_vLogStartMsg.push_back(strTmp);		

		SetLevelFilter(static_cast<LOGLEVEL>(i), FILTER[i]);
		if (m_fSvcMode)						// 서비스 모드인 경우
			m_fFilter[i] &= ~FILTER_CONSOL;	// 콘솔에 출력하지 않게 재조정한다.
	}

	char szBuf[MAX_PATH] = { 0x00, };	
	szBuf[MAX_PATH-1] = 0x00;
	::GetPrivateProfileStringA(LOGSECTION, LOGFOLDER, "", szBuf, MAX_PATH -1, szINIFile);
	if (strlen(szBuf) == 0)
	{
		strTmp  = "Not Found Key \"";
		strTmp +=  LOGFOLDER;
		strTmp += "\" In [LOG] Section. Using Default Value\"";
		strTmp +=  LOGROOT;
		strTmp += "\"";
		m_strLogFolder = LOGROOT;
	}
	else
	{	
		while(strlen(szBuf) && (szBuf[strlen(szBuf)-1] == '\\'))
			szBuf[strlen(szBuf)-1] = 0x00;

		strTmp  = LOGFOLDER;
		strTmp += " = ";
		strTmp += szBuf;
		m_strLogFolder = szBuf;
	}
	m_vLogStartMsg.push_back(strTmp);
	m_strLogFolder += "\\";
	m_strLogFolder += m_szSvcName;


	int iPerThread	= (int)::GetPrivateProfileIntA(LOGSECTION, LOGPERTHREAD,-1, szINIFile);
	if (iPerThread == -1)
	{
		strTmp  = "Not Found Key \"";
		strTmp +=  LOGPERTHREAD;
		strTmp += "\" In [LOG] Section. Using Default Value \"0\"(false)";		
		m_fPerThread = FALSE;
	}
	else
	{
		char szINT[15]={0x00,};
		_itoa(iPerThread, szINT, 10);		
		strTmp = LOGPERTHREAD;
		strTmp += " = ";	
		strTmp += szINT;
		strTmp += iPerThread ? "(true)" : "(false)";
		m_fPerThread = iPerThread ? TRUE : FALSE;
	}
	m_vLogStartMsg.push_back(strTmp);

	int iUseBufferedLog   = (int)::GetPrivateProfileIntA(LOGSECTION, USEBUFFER,   -1, szINIFile);
	if (iUseBufferedLog == -1)
	{
		strTmp  = "Not Found Key \"";
		strTmp +=  USEBUFFER;
		strTmp += "\" In [LOG] Section. Using Default Value \"0\"(false)";	
		m_fUseBufferedLog = FALSE;
	}
	else
	{
		char szINT[15]={0x00,};
		_itoa(iUseBufferedLog, szINT, 10);

		strTmp  = USEBUFFER;
		strTmp += " = ";		
		strTmp += szINT;
		strTmp += iUseBufferedLog ? "(true)" : "(false)";
		m_fUseBufferedLog = iUseBufferedLog ? TRUE : FALSE;
	}
	m_vLogStartMsg.push_back(strTmp);

	int iMaxFileSize = (int)::GetPrivateProfileIntA(LOGSECTION, MAXFILESIZE, -1, szINIFile);
	if (iMaxFileSize == -1)
	{
		strTmp  = "Not Found Key \"";
		strTmp +=  MAXFILESIZE;
		strTmp += "\" In [LOG] Section. Using Default Value \"0\"(Infinite)";
		m_sizeMax = 0x0000007FF;
	}
	else
	{
		char szINT[15]={0x00,};
		_itoa(iMaxFileSize, szINT, 10);

		strTmp  = MAXFILESIZE;
		strTmp += " = ";
		strTmp += szINT;
		strTmp += iMaxFileSize > 0 ?  "M byte" : "(INFINITE)";		

		if ((iMaxFileSize > 0) && (iMaxFileSize <= 0x0000007FF))
			m_sizeMax = iMaxFileSize;
		else
			m_sizeMax = 0x0000007FF;
	}

	m_sizeMax <<= 20; // 메가바이트를 바이트로 변환. m_sizeMax = m_sizeMax*1024*1024

	m_vLogStartMsg.push_back(strTmp);

	return TRUE;
}


void ServerLog::Final()
{
	if (!m_fPerThread)
	{
		if (!m_file)
			return;

		fclose(m_file->fp);
		delete m_file;
		m_file = NULL;
	}
	else
	{
		if (m_dwTLSSlot == TLS_OUT_OF_INDEXES)
			return;

		for (vector<LOGFILE*>::iterator i = m_vTLSSlot.begin(); i != m_vTLSSlot.end(); i = i++)
		{
			LOGFILE* file = *i;

			if (file)
			{
				fclose(file->fp);
				delete file;
			}
		}
		::TlsFree(m_dwTLSSlot);
		m_dwTLSSlot = TLS_OUT_OF_INDEXES;
	}
}
