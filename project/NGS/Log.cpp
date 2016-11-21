// Log.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <GService.h>

void __cdecl glog(LPCSTR fmt,...)
{
	char buf[256];
	va_list vl;
	va_start(vl, fmt);
	_vsnprintf(buf, 255, fmt, vl);
	buf[255] = 0;
	va_end(vl);

#ifdef _DEBUG
	OutputDebugStringA(buf);
	fprintf(stderr, buf);
#endif
}

void __cdecl tlog(LPCSTR fmt,...)
{
	char buf[256];
	va_list vl;
	va_start(vl, fmt);
	_vsnprintf(buf, 255, fmt, vl);
	buf[255] = 0;
	va_end(vl);

	SYSTEMTIME sys;
	::GetLocalTime(&sys);
	string sMsg = ::format("[%02d:%02d:%02d:%03d %05ld] %s", sys.wHour, 
		sys.wMinute, sys.wSecond, sys.wMilliseconds, ::GetCurrentThreadId(), buf);

#ifdef _DEBUG
	OutputDebugStringA(sMsg.c_str());
#endif
#ifdef _DEBUG_VERBOSE
	fprintf(stderr, sMsg.c_str());
#endif
}

void __cdecl svclog(LPCTSTR fmt,...)
{
	TCHAR buf[256];
	va_list vl;
	va_start(vl, fmt);
	_vsntprintf(buf, 255, fmt, vl);
	buf[255] = 0;
	va_end(vl);

#ifdef _DEBUG
	OutputDebugString(buf);
#endif
	//GetCurrentService()->LogEvent(buf);
}

