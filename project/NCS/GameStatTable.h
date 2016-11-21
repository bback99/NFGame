#pragma once

#include <ADL/MsgChannelCommon.h>

class CHSStatInfo
{
public :
	CHSStatInfo(const ChannelID& channelID, const LONG& lUserCount) : m_channelID(channelID), m_lUserCount(lUserCount) { }
	~CHSStatInfo() { }

	ChannelID& GetChannelD() { return m_channelID; }

	void SetUserCount(const LONG& lUserCount) { m_lUserCount = lUserCount; }
	const LONG GetUserCount() const { return m_lUserCount; }

	const DWORD GetCategoryNumber() const { return m_channelID.m_dwCategory; }
private :
	ChannelID m_channelID;
	LONG	  m_lUserCount;
};

class GLSStatInfo
{
public :
	GLSStatInfo(const LRBAddress& address, const LONG& lUserCount) : m_address(address), m_lUserCount(lUserCount) { }
	~GLSStatInfo() { }

	const LRBAddress& GetAddress() const  { return m_address; }

	void SetUserCount(const LONG& lUserCount) { m_lUserCount = lUserCount; }
	const LONG GetUserCount() const { return m_lUserCount; }

private :
	LRBAddress	m_address;
	LONG		m_lUserCount;
};

typedef list<CHSStatInfo*> CHSStatList;
typedef map<LONG, CHSStatList* > CHSStatInfoRepository;
typedef CHSStatInfoRepository::iterator ITER_CHSSTATREPO;
typedef CHSStatList::iterator ITER_CHSSTAT;

typedef list<GLSStatInfo*> GLSStatList;
typedef map<LONG, GLSStatList* > GLSStatInfoRepository;
typedef GLSStatInfoRepository::iterator ITER_GLSSTATREPO;
typedef GLSStatList::iterator ITER_GLSSTAT;

class GameStatTable
{
public:
	GameStatTable(void);
	~GameStatTable(void);

	void OnUpdateCHSStatInfo(const ChannelUpdateInfoList& list);
	void OnUpdateGLSStatInfo(const LONG& lSSN, const LRBAddress& address, const LONG& lUserCount);
	void OnDestroyCHS(const ChannelID& channelID);
	void OnDestroyGLS(const LRBAddress& address);

	const LONG GetTotalUserCount(const LONG& lSSN);

	void OnShowChannelUserCount(const LONG& lSSN, const DWORD& dwCategoryNum);

private :
	void SetCHSStatInfo(const LONG& lSSN, const ChannelID& channelID, const LONG& lUserCount);
	void SetGLSStatInfo(const LONG& lSSN, const LRBAddress& address, const LONG& lUserCount);
	void SetGLSStatInfo(const LRBAddress& address, const LONG& lUserCount = 0);


	CHSStatInfoRepository m_chsStatRepo;
	GLSStatInfoRepository m_glsStatRepo;

	GCRITICAL_SECTION	m_csCHSLock;
	GCRITICAL_SECTION	m_csGLSLock;
};

extern GameStatTable theGameStatTable;
