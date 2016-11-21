// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__F4EDEA1C_5DCE_4120_BDA0_52DE84BCCB56__INCLUDED_)
#define AFX_STDAFX_H__F4EDEA1C_5DCE_4120_BDA0_52DE84BCCB56__INCLUDED_

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
#include <NF/NFServerDefine.h>
#include <NF/NFStruct.h>

#include <ADL/MsgBLRBCommon.h>

#include <XLRBHandlerBase.h>
#include <NFVariant/NFUtil.h>

#ifdef _DEBUG
	#pragma comment(lib,"BLRBHandlerD.lib")
#else
	#pragma comment(lib,"BLRBHandler.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib,"NFVariantD.lib")
#endif

//#define _USING_GDBGW_

#if defined(_USING_GDBGW_)
#ifdef _DEBUG
#pragma comment (lib, "DBGWManagerD.lib")
#else
#pragma comment (lib, "DBGWManager.lib")
#endif
#endif

#include <NWCode.h>

#include <iostream>

//#include "ErrorLog.h"

#define _USE_PMS

#define TLOG(sz)	theLog.Put(INF, sz);
#define TLOG0(sz)	theLog.Put(DEV, sz);
//#define _COMMA		","

#ifdef _DEBUG
#define _DEBUG_VERBOSE
#endif

#ifdef _DEBUG
//#pragma comment(lib, "MCBase2KD.lib")
#pragma comment(lib, "NDLUtilD.lib")
#else
//#pragma comment(lib, "MCBase2K.lib")
#pragma comment(lib, "NDLUtil.lib")
#endif

#define CONFIG_FILENAME	"GRCConfig.INI"

//extern IThreadPool* GetThreadPool();
extern HTHREADPOOL GetThreadPool();
extern HTHREADPOOL GetListenerThreadPool();
extern HTHREADPOOL GetDBThreadPool();

// TODO: reference additional headers your program requires here
//#define USED_IPOOL
#ifdef USED_IPOOL
#pragma oMSG("USED_IPOOL")
#else
#pragma oMSG("USED_IPOOL - NONE")
#endif

#if defined(_USING_GDBGW_)
#pragma warning(disable:4706)

#define QUERY_DELIMETER		 '|'
#define QUERY_DELIMETERW	L'|'
#define ABSOLUTE_DELETE_VALUE	0xffffffff		//// RoomID::m_dwGRIID 를 ABSOLUTE_DELETE_VALUE로 하면 무조건 지운다.

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


//#ifdef _DEBUG
#define __DUMP_TEST_
//#endif 

#pragma oMSG(" #define [_NGS_ACCUSE]")
#define _NGS_ACCUSE

#define	LOG	theLog.Put       
#define DEV	DEV_UK,"NGS_DEV, ", __FILE__,"(", __LINE__,"):",__FUNCTION__, ", "
#define WRN	WAR_UK,"NGS_WAR, ", __FUNCTION__, ", "
#define INF	INF_UK,"NGS_INF, ", __FUNCTION__, ", "
#define ERR	ERR_UK,"NGS_ERR, ", __FILE__,"(", __LINE__,"):",__FUNCTION__, ", "
#define EVT	EVE_UK,"NGS_EVT, ", __FUNCTION__, ", "
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__F4EDEA1C_5DCE_4120_BDA0_52DE84BCCB56__INCLUDED_)
