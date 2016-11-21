#include "stdafx.h"
#include ".\testchannelmgr.h"
#include "ChannelDir.h"

CTestChannelMgr::CTestChannelMgr(void)
{
}

CTestChannelMgr::~CTestChannelMgr(void)
{
}

BOOL CTestChannelMgr::GetTestChannel(ChannelBaseInfoList& lstChannelInfo)
{	
	FILE *stream;
	char strbuf[1025] = {0x00};

	char strDir[1024] = {0x00};
	DWORD dwRet = GetModuleFileNameA(NULL, strDir, 1024);
	if(dwRet == 0)
		return FALSE;
	string strDir1 = strDir;
	UINT nIDX = strDir1.find_last_of("\\");
	strDir1 = strDir1.substr(0, nIDX);

	char strFileName[2048] = {0x00};
	sprintf(strFileName, "%s\\channelcategorylist.txt", strDir1.c_str());	
	if( (stream = fopen( strFileName, "r" )) != NULL )
	{		
		while(GetFileLine(stream, strbuf, 1024) != -1)
		{
			ParsingString(strbuf, lstChannelInfo);
			memset(strbuf, 0, 1025);
		}		
	}
	else 
		return FALSE;
	fclose(stream);
	return TRUE;
}

int CTestChannelMgr::GetFileLine(FILE* fp, char* buf, int nCnt)
{
	int nIndex = 0;
	int nch;
	nch = fgetc(fp);
	while(!feof(fp))
	{		
		if(nch == '\n')
			return 1;
		else if(nIndex > nCnt)
			return 0;			
		buf[nIndex++] = (char)nch;
		nch = fgetc(fp);
	}
	return -1;
}

int CTestChannelMgr::ParsingString(char* strbuf, ChannelBaseInfoList& lstChannelInfo)
{
	string strtemp = "";

	string sSSN = "";
	string sCategoryNum = "";
	xstring sCategoryName;

	NSAP nsap = theChannelDir.GetCHSNsap();

	string strdata = strbuf;
	int nIndexFlag = 0;
	int nStringFlag = 0;
	for(UINT i = 0 ; i < strdata.size() ; i++)
	{
		char chdata = strdata.at(i);
		if(chdata != ' ' && chdata != '\t')
		{
			if(nStringFlag == 0)
				nStringFlag = 1;
			if(nIndexFlag == 0) {
				if(chdata > '9' || chdata < '0')
					return 0;
				sSSN.push_back(chdata);
			}
			else if(nIndexFlag == 1) {
				if(chdata > '9' || chdata < '0')
					return 0;
				sCategoryNum.push_back(chdata);
			}
			else if(nIndexFlag == 2) {
				sCategoryName.assign(strdata.substr(i));
				break;
			}
		}
		else if(nStringFlag == 1)
		{
			nStringFlag = 0;
			nIndexFlag++;
		}
		if(nIndexFlag >= 3)
			break;
	}
	if(nIndexFlag != 2)
		return 0;

	if(atol(sSSN.c_str()) < 1 || atol(sCategoryNum.c_str()) < 1)
		return 0;

	time(NULL);
	UINT nRnd = rand();
	nRnd /= 5000;
	ChannelBaseInfo chsInfo;
	ChannelRegistInfo channelForLB;
	for(UINT nIdx = 0 ; nIdx < 3 ; nIdx++)
	{
		channelForLB.m_channelID.m_lSSN = chsInfo.m_channelID.m_lSSN = atol(sSSN.c_str());
		channelForLB.m_channelID.m_dwCategory = chsInfo.m_channelID.m_dwCategory = atol(sCategoryNum.c_str());
		channelForLB.m_channelID.m_dwGCIID = chsInfo.m_channelID.m_dwGCIID = 100000*chsInfo.m_channelID.m_lSSN + \
			10000*chsInfo.m_channelID.m_dwCategory + nRnd + nIdx;
		chsInfo.m_lCACode = 0;
		channelForLB.m_lLimitCount = chsInfo.m_lLimitCount = 150;
		channelForLB.m_lMaxCount = chsInfo.m_lMaxCount = 120;
		channelForLB.m_sCategoryName = chsInfo.m_sCategoryName = sCategoryName;
		
		//char sTitle[1024] = {0x00};
		//sprintf(sTitle, "%s_Test_%d", sCategoryName.c_str(), nIdx);
		xstring sTitle;
		sTitle = ::format(_X("%s_Test_%d"), sCategoryName.c_str(), nIdx);
		channelForLB.m_sTitle = chsInfo.m_sTitle = sTitle;
		chsInfo.m_nsapCHS = nsap;

		channelForLB.m_lChannelType = 1;
		channelForLB.m_lChannelOpenState = 1;

		lstChannelInfo.push_back(chsInfo);
		theChannelDir.m_lstChannelInfoForLBReg.push_back(channelForLB);

//		LOG(INF_UK, "CHS_Channel_Create"_LK, "ChannelID = [", chsInfo.m_channelID.m_lSSN, "/", \
//			chsInfo.m_channelID.m_dwCategory, "/", chsInfo.m_channelID.m_dwGCIID, "]");
	}
	return 1;
}