//
// Config.h
//

#include "stdafx.h"

using namespace adl;

#ifndef CONFIG_H
#define CONFIG_H

#define MAXSIZE_QUEUE	500			// LRBHandler�� Mesage ��� queue size
#define MAX_DUMPSIZE  1024 * 1000	// data dump�� ���� buff�� �ִ� ũ��..

#define MAX_LCSVALUE	1			// Multi locatio ������ �Ѱ谪..


#define LCSPEER_CONNECTIONPORT	17178	// Peer ����� ���� port.. 

///////////////////////////////////////////////////////////////////////////////////
// CConfig


#define LCSPEEREVENT_SENDTOCLIENT ((HSIGNAL)0xfffffffd)
#define LCSPEEREVENT_SENDTOSERVER ((HSIGNAL)0xfffffffe)

#define PEERCTLEVENT_RECOVERCONNECT ((HSIGNAL) 0xfffffff5)
#define PEERCTLEVENT_TRANSMESSAGE	((HSIGNAL) 0xfffffff4)

class CConfig
{
	IMPLEMENT_TISAFE(CConfig)
public:
	typedef pair<DWORD, DWORD> IPSetT;
	// Pair�� �Ǵ� ����(Master/slave)�� IP set
	// vector�� ù��°���� �ڽ��� ���� IP set�� ��� �ִ�.
	typedef list<IPSetT> VecIPT;
	
public:
	CConfig();
	virtual ~CConfig();
public:
	BOOL Init();
//	SvcTypeID GetSvcTypeID() const;
//	LONG GetServiceType() const;
public:
	DWORD GetMyIP() { return m_nsapLCS.m_dwIP; }
	DWORD GetRemoteIP(); 
public:
	const NSAP& GetLCSNSAP() const {return m_nsapLCS;}
	LONG InitTimeStemp();
	void GetIPSet(vector<LRBAddress>& vec_IP);
public:
	void GetServerPair(IPSetT & setT, LONG lIndex = 0L);
	void SetAddress(string & sIP, DWORD dwPort = 9998);

//	SvcTypeID GetSvcTypeID() const;
	int GetLCSServerCount() { return m_IPSet.size(); }					/// LCS Server Count in LCS.ini
	int GetServerModulerIndex() { return m_lServerModulerIndex; }		/// LCS Server index in LCS.ini

	///*TCHAR*/ char m_szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	char m_szComputerName[1024];
	long m_lLCSID;
	LONG m_lTimeStemp;

	vector<string> m_IPStringSet;		///CPeerClient���� ���.
	
private:
	NSAP m_nsapLCS;
	VecIPT m_IPSet;		// server set�� ����. INI�� ��ϵ� �����߿� �ڽ��� ���� �׷��� ù��°�� �����Ѵ�. 
	long m_lServerModulerIndex;
};

extern CConfig theConfig;


#endif //!CONFIG_H
