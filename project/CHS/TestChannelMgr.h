#pragma once

#ifndef _TESTCHANNELMGR_
#define _TESTCHANNELMGR_

#include "common.h"

class CTestChannelMgr
{
public:
	CTestChannelMgr(void);
	~CTestChannelMgr(void);

	BOOL GetTestChannel(ChannelBaseInfoList& lstChannelInfo);
	int GetFileLine(FILE* fp, char* buf, int nCnt);
	int ParsingString(char* strbuf, ChannelBaseInfoList& lstChannelInfo);
};


#endif // _TESTCHANNELMGR_