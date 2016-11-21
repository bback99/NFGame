// 
// GChannel.h
// 
/**********************************************************
		Structure for Game Channel
									-ss
***********************************************************/

#ifndef _GCHANNEL_H__
#define _GCHANNEL_H__

#include <NF/ADL/MsgCHSNCS.h>

/////////////////////////////////////////////////////////////////////////
// Game Channel
class CHSInfo;
class Category;

class GameChannel : public ChannelInfo
{
public:
	typedef ChannelInfo TBase;

public:
	GameChannel();
	GameChannel(LONG lChannelType) : m_lChannelType(lChannelType) { }
	virtual ~GameChannel();
public:
	virtual void Clear();

	BOOL UpdateGameChannel(const ChannelUpdateInfo& channelUpdateInfo);
	BOOL UpdateGameChannel(const ChannelInfo& channelInfo);
	ChannelInfo GetChannel();
	DWORD GetGameChannelID() const;
	void SetNSAP(NSAP& nsap);
	NSAP& GetNSAP();
	void SetStatic(LONG lChannelType = CHANTYPE_STATIC) { m_lChannelType = lChannelType; }

	LONG WhatChannelType() { return m_lChannelType; }
	void CloseChannel(); 
	void OpenChannel();
	void SetChannelOpenState(BOOL bIsChannelOpen) { m_bIsChannelOpen = bIsChannelOpen;}
	void SetChannelOpenState2(BOOL bIsChannelOpen) { m_bIsChannelOpen = bIsChannelOpen; m_bCreate = bIsChannelOpen; }
	BOOL GetChannelOpenState() { return m_bIsChannelOpen; }
	LONG GetChannelType() { return m_lChannelType; }
	void SetChannelType(LONG lChannelType) { m_lChannelType = lChannelType ; }
	void SetCreate(BOOL bCreate) { m_bCreate = bCreate; }
	BOOL IsCreated() const { return m_bCreate; }
	void OnCreate(long lPort);
	void OnDestroy();

	BOOL CanService(LONG& lFactor);
	BOOL CanGRJoin();
	ChannelBaseInfo GetChannelBaseInfo();

private:
	LONG m_lChannelType;
	BOOL m_bIsChannelOpen;
	BOOL m_bCreate;
public:
	LONG m_lTraffic;
private:
	friend class GChannelTable;
	friend class Category;
};

#endif