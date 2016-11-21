//
// Config.cpp
//

#include "stdafx.h"
#include "Config.h"
#include "LRBHandler.h"
#include "Control.h"

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

// INI�� ��ϵ� ������ IP ������ �ٲ㼭�� �ȵȴ�.
void CConfig::GetIPSet(vector<LRBAddress>& vec_IP)
{	
}

