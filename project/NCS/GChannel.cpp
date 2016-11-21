// 
// GChannel.cpp
// 

#include "stdafx.h"
#include "GCHannel.h"
#include "Category.h"
#include "ChannelSvrTable.h"

#define LIMIT_USER_FACTOR		30
#define LIMIT_ROOM_FACTOR		30
#define LIMIT_WAITING_FACTOR	30
#define LIMIT_GLS_FACTOR		10

/////////////////////////////////////////////////////////////////////////////
// Game Channel
GameChannel::GameChannel()
{
	Clear();
	m_lChannelType = CHANTYPE_STATIC;
	m_lLimitCount = 0L;
	m_lMaxCount = 0L;
	m_lCACode = 0L;
	m_bIsChannelOpen = TRUE;
	m_bCreate = FALSE;
	m_lTraffic = 0L;
}

GameChannel::~GameChannel()
{
}
void GameChannel::CloseChannel()
{
	m_bIsChannelOpen = FALSE;
}
void GameChannel::OpenChannel()
{
	m_bIsChannelOpen = TRUE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL GameChannel::UpdateGameChannel(const ChannelUpdateInfo& channelInfo)
{
	m_lUserCount		= channelInfo.m_lUserCount;
	m_lTraffic			= m_lUserCount;
	m_lWaitingUserCount = channelInfo.m_lWaitingUserCount;
	m_lRoomCount		= channelInfo.m_lRoomCount;
	return TRUE;
}

BOOL GameChannel::UpdateGameChannel(const ChannelInfo& channelInfo)
{
	m_channelID			= channelInfo.m_channelID;
	m_nsapCHS			= channelInfo.m_nsapCHS;
	m_lMaxCount			= channelInfo.m_lMaxCount;
	m_lLimitCount		= channelInfo.m_lLimitCount;
	m_lCACode			= channelInfo.m_lCACode;
	m_sTitle			= channelInfo.m_sTitle.c_str();
	m_sCategoryName		= channelInfo.m_sCategoryName.c_str();
	m_lUserCount		= channelInfo.m_lUserCount;
	m_lTraffic			= m_lUserCount;
	m_lWaitingUserCount = channelInfo.m_lWaitingUserCount;
	m_lRoomCount		= channelInfo.m_lRoomCount;

	return TRUE;
}

ChannelInfo GameChannel::GetChannel()
{
	ChannelInfo		cInfo;

	cInfo.m_channelID = m_channelID;
	cInfo.m_nsapCHS = m_nsapCHS;
	cInfo.m_lMaxCount = m_lMaxCount;
	cInfo.m_lLimitCount = m_lLimitCount;
	cInfo.m_lCACode = m_lCACode;
	cInfo.m_sTitle = m_sTitle;
	cInfo.m_sCategoryName = m_sCategoryName;

	cInfo.m_lUserCount = m_lUserCount;
	cInfo.m_lWaitingUserCount = m_lWaitingUserCount;
	cInfo.m_lRoomCount = m_lRoomCount;

	return cInfo;
}

DWORD GameChannel::GetGameChannelID() const
{
	return m_channelID.m_dwGCIID;
}


void GameChannel::Clear()
{
	TBase::Clear();
}

void GameChannel::SetNSAP(NSAP& nsap)
{
	m_nsapCHS.m_dwPort = nsap.m_dwPort;
	m_nsapCHS.m_dwIP = nsap.m_dwIP;
}

NSAP& GameChannel::GetNSAP()
{
	return m_nsapCHS;
}

void GameChannel::OnCreate(long lPort)
{
	m_nsapCHS.m_dwPort = lPort;

	m_lUserCount = 0;
	m_lTraffic = 0;

	SetCreate(TRUE);
}

void GameChannel::OnDestroy()
{
	m_lUserCount = 0;
	m_lTraffic = 0;

	SetCreate(FALSE);
}

BOOL GameChannel::CanService(LONG& lFactor)
{
	if(!m_bCreate)
		return FALSE;
	if(!m_bIsChannelOpen)
		return FALSE;
	if( (m_lUserCount + 1) >= m_lLimitCount)
		return FALSE;
	if(m_lTraffic >= (LONG) (m_lLimitCount * 1.2))
		return FALSE;
	else
	{
		lFactor = m_lTraffic;
		return TRUE;
	}
}

BOOL GameChannel::CanGRJoin()
{
	if(!m_bCreate)
		return FALSE;
	if(!m_bIsChannelOpen)
		return FALSE;
	if( (m_lUserCount + 1) >= m_lMaxCount)
		return FALSE;
	if( (m_lUserCount) >= m_lMaxCount)
		return FALSE;
	return TRUE;
}

ChannelBaseInfo GameChannel::GetChannelBaseInfo()
{
	ChannelBaseInfo	baseInfo;
	baseInfo.m_channelID = m_channelID;
	baseInfo.m_nsapCHS = m_nsapCHS;
	baseInfo.m_lMaxCount = m_lMaxCount;
	baseInfo.m_lLimitCount = m_lLimitCount;
	baseInfo.m_lCACode = m_lCACode;
	baseInfo.m_sTitle = m_sTitle;
	baseInfo.m_sCategoryName = m_sCategoryName;
	return baseInfo;
}

