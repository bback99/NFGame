// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
// #define _ATL_FREE_THREADED

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// TODO: reference additional headers your program requires here
#include <atlbase.h>

#define XASSERT_DEBUG
#define XCOMP_DEBUG

#include <LXSvr.h>
#include <NFVariant/NFUtil.h>
#include <NF/NFServerDefine.h>
#include <NF/NFStruct.h>
#include <ADL/MsgBLRBCommon.h>
#include <XLRBHandlerBase.h>

#ifdef _DEBUG
#pragma comment(lib,"BLRBHandlerD.lib")
#pragma comment(lib,"NFVariantD.lib")
#else
#pragma comment(lib,"BLRBHandler.lib")
#endif


#if defined(_USING_GDBGW_)
#ifdef _DEBUG
#pragma comment (lib, "DBGWManagerD.lib")
#else
#pragma comment (lib, "DBGWManager.lib")
#endif
#endif


#include <NWCode.h>
#include <iostream>


#define XLRB_RETRYCOUNT				5


#define _USE_PMS

#define TLOG(sz)	theLog.Put(INF, sz);
#define TLOG0(sz)	theLog.Put(DEV, sz);
//#define _COMMA		","

#ifdef _DEBUG
#define _DEBUG_VERBOSE
#endif

extern HTHREADPOOL GetThreadPool();
extern HTHREADPOOL GetListenerThreadPool();
extern HTHREADPOOL GetDBThreadPool();

#if defined(_USING_GDBGW_)
#pragma warning(disable:4706)

#define _X(a)	a
#include <DBGWManager.h>
#define ExecuteQueryX			ExecuteQuery
#define DBGW_XString			DBGW_String
#define LPCXSTR					LPCSTR
#define xchar					CHAR
#define xprintf					sprintf
#define xtol(x)					::atol(x)
#define NDL_ResultStrX			NDL_ResultStr
#define NDL_ResultListX			NDL_ResultList
#define	BSize_NDL_ResultStrX	BSize_NDL_ResultStr
#define BSize_NDL_ResultListX	BSize_NDL_ResultList
#define BEncode_NDL_ResultStrX	BEncode_NDL_ResultStr
#define BEncode_NDL_ResultListX	BEncode_NDL_ResultList
#define	BDecode_NDL_ResultListX	BDecode_NDL_ResultList
#define xstr2wstr(x)			::str2wstr(x)
#define xstr2str(x)				(x)
#define str2xstr(x)				(x)
#define xstrsize				strsize
#define xstrpos					strpos
#define xstrstr					strstr
#endif //_USING_GDBGW_


#define	LOG	theLog.Put 
#define DEV	DEV_UK,"NCS_DEV, ", __FILE__,"(", __LINE__,"):",__FUNCTION__, ", "
#define WRN	WAR_UK,"NCS_WAR, ", __FUNCTION__, ", "
#define INF	INF_UK,"NCS_INF, ", __FUNCTION__, ", "
#define ERR	ERR_UK,"NCS_ERR, ", __FILE__,"(", __LINE__,"):",__FUNCTION__, ", "
#define EVT	EVE_UK,"GLS_EVT, ", __FUNCTION__, ", "


//inline int urand() {
//	int i1, i2, i3;
//	i1 = rand() & 0x0fff;
//	i2 = (rand() & 0x0fff) << 12;
//	i3 = (rand() & 0x007f) << 24;
//	return (i1 | i2 | i3);
//}
//
//
//inline int urandom(int nummax) {
//	return (urand() * GetTickCount() % nummax);
//}
