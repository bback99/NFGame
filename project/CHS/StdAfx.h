// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__7CCA7C25_E738_4F28_B178_C5D44CB72311__INCLUDED_)
#define AFX_STDAFX_H__7CCA7C25_E738_4F28_B178_C5D44CB72311__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
// #define _ATL_FREE_THREADED

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning (disable : 4702)
#pragma warning(disable: 4512)

 
#define _DB_SR_ERROR_LOG //DB에 게임 에러 로그를 남긴다.
#define CONFIG_FILENAME "CHS.INI"


#include <atlbase.h>
#include <XTypes.h>
#include <LXSvr.h>
#include <ADL/MsgBLRBCommon.h>
#include <XLRBHandlerBase.h>


// 
#ifdef _DEBUG
	#pragma comment(lib, "NDLUtilD.lib")
	#pragma comment(lib, "BLRBHandlerD.lib")
	#pragma comment(lib, "DBGWManagerD.lib")
	#pragma comment(lib, "NFVariantD.lib")
#else
	#pragma comment(lib, "NDLUtil.lib")
	#pragma comment(lib, "BLRBHandler.lib")
	#pragma comment(lib, "DBGWManager.lib")
#endif


#include <Define.h>
#include <NWCode.h>

#pragma warning(disable:4706)
#include <DBGWManager.h>
#pragma warning(default:4706)

#define QUERY_DELIMETER		 '|'
#define QUERY_DELIMETERW	L'|'

#define		LOG			theLog.Put
#define		_LK			,","
//#define		_COMMA		,","
    
#include <iostream>

extern void __cdecl tlog(LPCTSTR szFmt, ...);
#define TLOG ::tlog
#define TLOG0(sz)              ::tlog(_T("%s"), sz)
#define TLOG1(sz, p1)          ::tlog(sz, p1)
#define TLOG2(sz, p1, p2)      ::tlog(sz, p1, p2)
#define TLOG3(sz, p1, p2, p3)  ::tlog(sz, p1, p2, p3)

extern void __cdecl svclog(LPCTSTR szFmt, ...);
#define SVCLOG ::svclog
#define SVCLOG0(sz)              ::svclog(_T("%s"), sz)
#define SVCLOG1(sz, p1)          ::svclog(sz, p1)
#define SVCLOG2(sz, p1, p2)      ::svclog(sz, p1, p2)
#define SVCLOG3(sz, p1, p2, p3)  ::svclog(sz, p1, p2, p3)

#ifdef _DEBUG
#define _DEBUG_VERBOSE
#endif

extern HTHREADPOOL GetThreadPool();
extern HTHREADPOOL GetChannelThreadPool();
extern HTHREADPOOL GetLRBThreadPool();

#define USE_DIFFREPORT
#ifdef USE_DIFFREPORT
#pragma oMSG("USE_DIFFREPORT")
#else
#pragma oMSG("USE_DIFFREPORT - NONE")
#endif

//#define USE_CRCREPORTER
#ifdef USE_CRCREPORTER
#pragma oMSG("USE_CRCREPORTER")
#else
#pragma oMSG("USE_CRCREPORTER - NONE")
#endif

//#define _CHSNLS
#ifdef _CHSNLS
#pragma oMSG("_CHSNLS")
#else
#pragma oMSG("_CHSNLS - NONE")
#endif

#define _CHS_ACCUSE
//#define _USE_MSGCOUNT
//LPCTSTR const	MY1LOGKEY1 = _T("MY1_Logkey1,");
//#define			MY1LOGKEY2 = _T("MY1_Logkey2,")


// TODO: reference additional headers your program requires here
#define MD5_TIME_GAP_DOWN		(60*60*8)			///	8  hour
#define MD5_TIME_GAP_UP			(60*5)				/// 10 sec
#define MD5_KEY					"gOGo !!pmANg"

#define DEFAULT_LAST_CHANGENICK_DATE "NONE"


//inline int urand() {
//	int i1, i2, i3;
//	i1 = rand() & 0x0fff;
//	i2 = (rand() & 0x0fff) << 12;
//	i3 = (rand() & 0x007f) << 24;
//	return (i1 | i2 | i3);
//}
//
//inline int urandom(int nummax) {
//	return (urand() * GetTickCount() % nummax);
//}

#include <NFVariant/NFUtil.h>
#include <NLSManager.h>


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__7CCA7C25_E738_4F28_B178_C5D44CB72311__INCLUDED_)
