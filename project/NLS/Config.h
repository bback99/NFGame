//
// Config.h
//

#include "stdafx.h"

using namespace adl;

#ifndef CONFIG_H
#define CONFIG_H

#define MAXSIZE_QUEUE	500			// LRBHandler의 Mesage 대기 queue size
#define MAX_DUMPSIZE  1024 * 1000	// data dump를 위한 buff의 최대 크기..

#define MAX_LCSVALUE	1			// Multi locatio 정보의 한계값..


#define LCSPEER_CONNECTIONPORT	17178	// Peer 통신을 위한 port.. 

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
	// Pair가 되는 서버(Master/slave)의 IP set
	// vector의 첫번째에는 자신이 속한 IP set을 담고 있다.
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

	vector<string> m_IPStringSet;		///CPeerClient에서 사용.
	
private:
	NSAP m_nsapLCS;
	VecIPT m_IPSet;		// server set을 저장. INI에 등록된 순서중에 자신이 속한 그룹을 첫번째로 저장한다. 
	long m_lServerModulerIndex;
};

extern CConfig theConfig;


#endif //!CONFIG_H
