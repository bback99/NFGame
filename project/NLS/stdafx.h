// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

/*
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define WIN32_LEAN_AND_MEAN

#pragma warning(disable:4267)
#pragma warning(disable:4530)
#pragma warning(disable:4512)
*/
#include <LXSvr.h>
#include <iostream>
#include <tchar.h>

#include <ADL/MsgNSAP.h> 
#include <ADL/MsgBLRBCommon.h> 
#include <NF/ADL/MsgNLSCli.h>
//#include <ADL/AdlTLSCLI_b.h>

#ifdef _DEBUG
#pragma comment(lib,"BLRBHandlerD.lib")
#else
#pragma comment(lib,"BLRBHandler.lib")
#endif

#ifdef _DEBUG
#define _DEBUG_VERBOSE
#endif

#define CONFIG_FILENAME "NLS.INI"

#define		LOG			theLog.Put

extern HTHREADPOOL GetThreadPool();

