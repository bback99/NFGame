//
// Config.cpp
//

#include "stdafx.h"
#include "Config.h"

#include "LRBHandler.h"
#include "Control.h"

//DWORD ALIVE_INTERVAL = 60000;
//DWORD ALIVE_COUNT = 6;

///////////////////////////////////////////////////////////////////////////////////
// CConfig

CConfig theConfig;

extern CLRBHandler theLRBHandler;

CConfig::CConfig()
{
	m_szComputerName[0] = 0;	
	m_lLCSID = 0xFFFF;
	m_IPSet.clear();
	m_IPStringSet.clear();

	m_lServerModulerIndex = 0;

//	//////////////// for test /////////////////////
//	IPSetT setIP(123, 234);		
//	m_IPSet.push_back(setIP);//insert(m_IPSet.begin(), setIP);
////	m_IPSet.clear();
//	///////////////////////////////////////////////
}

CConfig::~CConfig()
{
}

BOOL GetIpAddress(string &s_ip)
{
	char cHostName[255] = {0, };
	char cOuterAddress[100] = {0, };
	HOSTENT* hostinfo;
	char *pTempAddr = NULL;

	hostinfo = ::gethostbyname(cHostName);
	memset(cHostName, 0, 100);
	::gethostname(cHostName, 100);
	if(hostinfo)
	{
		for(int i=0 ; *(hostinfo->h_addr_list) != NULL; i++)
		{
			pTempAddr = inet_ntoa(*(struct in_addr*)*( (hostinfo->h_addr_list)++ ));
			if(pTempAddr != NULL) 
			{
				memcpy(cOuterAddress, pTempAddr, strlen(pTempAddr));
			}
		}
		if(strlen(cOuterAddress) < 7) {
			return FALSE;
		}
	}
	s_ip = cOuterAddress;

	return TRUE;
}

//#include "../pms/include/PMSDefinition.h"
BOOL CConfig::Init()
{	
	TLock lo(this);

	string sIP;
	::GetIpAddress(sIP);
	SetAddress(sIP);

	// get computer name
	{
		DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
		BOOL bRet = ::GetComputerNameA(m_szComputerName, &dwSize);
		VALIDATE(bRet);
		if (!bRet)
			return FALSE;
	}
	return TRUE;
}

void CConfig::SetAddress(string &sIP, DWORD dwPort)
{
	m_nsapLCS.m_dwPort = dwPort;
	m_nsapLCS.SetIP(sIP);
}



void CConfig::GetServerPair(IPSetT & setT, LONG lIndex)
{
	if(m_IPSet.size() > 0)
	{
		VecIPT::iterator iter = m_IPSet.begin();
		//setT = m_IPSet[0];
		setT = m_IPSet.front();
	}
}

DWORD CConfig::GetRemoteIP()
{
	pair<DWORD, DWORD> ip_set;
	GetServerPair(ip_set);

	DWORD r_ip = 0UL;
	(ip_set.first == GetMyIP()) ? r_ip = ip_set.second : r_ip = ip_set.first;

	return r_ip;
}

LONG CConfig::InitTimeStemp()
{
	SYSTEMTIME _t;
	::GetLocalTime(& _t);
	m_lTimeStemp = ((_t.wMinute * 60 + _t.wSecond) * 1000);
	return m_lTimeStemp;
}

// INI에 등록된 서버의 IP 순서를 바꿔서는 안된다.
void CConfig::GetIPSet(vector<LRBAddress>& vec_IP)
{
// 	LRBAddress dwIPFirst;
// 	LRBAddress dwIPSecond;
// 
// 	for(int i = 0; i < 10; i++)
// 	{
// 		char szBuf1[100] = {0, };
// 		char szBuf2[100] = {0, };
// 		char szKeyWord1[100] = {0, };
// 		sprintf(szKeyWord1, "SET%dA", i);
// 		char szKeyWord2[100] = {0, };
// 		sprintf(szKeyWord2, "SET%dB", i);
// 		BOOL dwRet = ::GetPrivateProfileStringA("LCS", szKeyWord1, "", szBuf1, 100, theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/);
// 		dwRet = dwRet && ::GetPrivateProfileStringA("LCS", szKeyWord2, "", szBuf2, 100, theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/);
// 		if(!dwRet)
// 			break;
// 
// 		//DWORD dwIPFirst = ::inet_addr(szBuf1);
// 		//DWORD dwIPSecond = ::inet_addr(szBuf2);
// 				
// 		theLRBHandler.MakeOtherAddress(dwIPFirst, string(szBuf1), DWORD('U'), SVCCAT_LCS); 
// 		theLRBHandler.MakeOtherAddress(dwIPSecond, string(szBuf2), DWORD('U'), SVCCAT_LCS); 
// 		
// 		vec_IP.push_back(dwIPFirst);
// 		vec_IP.push_back(dwIPSecond);
// 	}
}

