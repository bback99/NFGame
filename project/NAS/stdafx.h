// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <LXSvr.h>
#include <iostream>
#include <tchar.h>

#include <ADL/MsgNSAP.h> 
#include <ADL/MsgBLRBCommon.h> 
#include <NF/ADL/MsgNASCli.h>
#include <NF/NFServerDefine.h>
#include <NF/NFStruct.h>
#include <DBGWManager.h>

#include "AchvDef.h"

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

#ifdef _DEBUG
#define _DEBUG_VERBOSE
#endif

#define CONFIG_FILENAME "NAS.INI"

#define		LOG			theLog.Put
#define _COMMA		","

extern HTHREADPOOL GetThreadPool();



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