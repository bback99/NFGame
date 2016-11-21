//
// CHSInfoDir.cpp
//

#include "stdafx.h"
#include "CHSInfoDir.h"

CHSInfoDir theCHSInfoDir;

///////////////////////////////////////////////////////////////////////////////////
// CHSInfoDir

CHSInfoDir::CHSInfoDir() : _csCHSInfoDir(4000)
{
}
CHSInfoDir::~CHSInfoDir()
{
}

#ifdef _PMS_CODE_SEPERATION_
void CHSInfoDir::InsertCHSInfo(const ChannelID& channelID, LONG lUserCount, LONG lWaitingUserCount, LONG lRoomCount, LONG lPCRoomUserCount)
#else
void CHSInfoDir::AddCHSInfo(const ChannelID& channelID, long lUserCount, long lWaitingUserCount, long lRoomCount, long lPCRoomUserCount)
#endif
{
#ifdef _PMS_CODE_SEPERATION_
	if(0 > lUserCount)
	{
		lUserCount = 0;
	}
	if(0 > lWaitingUserCount)
	{
		lWaitingUserCount = 0;
	}
	if(0 > lRoomCount)
	{
		lRoomCount = 0;
	}
	if(0 > lPCRoomUserCount)
	{
		lPCRoomUserCount = 0;
	}
#endif
	TLock lo(this);

	CHSInfo& chsinfo = m_mapCHSInfo[channelID];
	chsinfo.second = true;

	ChannelUpdateInfoPCRoom & uinfo = chsinfo.first;
	uinfo.m_channelID = channelID;
	uinfo.m_lUserCount = lUserCount;
	uinfo.m_lWaitingUserCount = lWaitingUserCount;
	uinfo.m_lRoomCount = lRoomCount;
	uinfo.m_lPCRoomUserCount = lPCRoomUserCount;
}


#ifdef _PMS_CODE_SEPERATION_
BOOL CHSInfoDir::DeleteCHSInfo(const ChannelID & channelID)
#else
BOOL CHSInfoDir::RemoveCHSInfo(const ChannelID & channelID)
#endif
{
	BOOL bRet = TRUE;
	{
		TLock lo(this);
		bRet = (m_mapCHSInfo.erase(channelID) > 0);
	}
	return bRet;
}

BOOL CHSInfoDir::UpdateCHSInfo(const ChannelID& channelID, LONG lUserCount, LONG lWaitingUserCount, LONG lRoomCount, LONG lPCRoomUserCount)
{
	BOOL bRet = TRUE;
	{
		TLock lo(this);
		TCHSInfoMap::iterator it = m_mapCHSInfo.find(channelID);
		bRet = (it != m_mapCHSInfo.end());
		if(bRet)
		{
			// PCRoomUserCount add code
			CHSInfo& chsinfo = it->second;
			ChannelUpdateInfoPCRoom & info = chsinfo.first;
			info.m_lUserCount = lUserCount;
			info.m_lWaitingUserCount = lWaitingUserCount;
			info.m_lRoomCount = lRoomCount;
			info.m_lPCRoomUserCount = lPCRoomUserCount;
			chsinfo.second = true;
		}
	}
	return bRet;	
}

DWORD CHSInfoDir::GetCHSInfoList(ChannelUpdateInfoList & lst, BOOL bAll, BOOL bClearFlags)
{
	DWORD dwCount = 0;
	{
		TLock lo(this);
		ForEachElmt(TCHSInfoMap, m_mapCHSInfo, it, jt)
		{
			CHSInfo& info = it->second;
			if(info.second)
			{
				lst.push_back( ChannelUpdateInfo() );
				lst.back().BCopy( info.first );

				dwCount++;
				if(bClearFlags)
					info.second = false;
			}
			else if(bAll)
			{
				lst.push_back( ChannelUpdateInfo() );
				lst.back().BCopy( info.first );
			}
		}
	}

	return dwCount;
}

/////////// bAll : TRUE -> get all chsinfo list. FALSE -> get existing user chsinfo list 
DWORD CHSInfoDir::GetCHSInfoListForPMS(ChannelUpdateInfoList & lst, BOOL bAll)
{
	DWORD dwCount = 0;
	{
		TLock lo(this);
		ForEachElmt(TCHSInfoMap, m_mapCHSInfo, it, jt)
		{
			CHSInfo& info = it->second;
			if(info.first.m_lUserCount > 0 || info.first.m_lWaitingUserCount > 0)
			{
				lst.push_back( info.first );
				dwCount++;
			}
			else if(bAll)
			{
				lst.push_back( info.first );
				dwCount++;
			}
		}
	}

	return dwCount;
}

/////////// bAll : TRUE -> get all chsinfo list. FALSE -> get existing user chsinfo list 
DWORD CHSInfoDir::GetCHSInfoListForPMS(ChannelUpdateInfoPCRoomList & lst, BOOL bAll)
{
	DWORD dwCount = 0;
	{
		TLock lo(this);
		ForEachElmt(TCHSInfoMap, m_mapCHSInfo, it, jt)
		{
			CHSInfo& info = it->second;
			if(info.first.m_lUserCount > 0 || info.first.m_lWaitingUserCount > 0)
			{
				lst.push_back( info.first );
				dwCount++;
			}
			else if(bAll)
			{
				lst.push_back( info.first );
				dwCount++;
			}
		}
	}

	return dwCount;
}

DWORD CHSInfoDir::ClearFlags()
{
	DWORD dwCount = 0;
	{
		TLock lo(this);
		ForEachElmt(TCHSInfoMap, m_mapCHSInfo, it, jt)
		{
			CHSInfo& info = it->second;
			if(info.second)
			{
				info.second = false;
				dwCount++;
			}
		}
	}
	return dwCount;
}

DWORD CHSInfoDir::SetAllFlags()
{
	DWORD dwCount = 0;
	{
		TLock lo(this);
		ForEachElmt(TCHSInfoMap, m_mapCHSInfo, it, jt)
		{
			CHSInfo& info = it->second;
			info.second = true;
			dwCount++;
		}
	}
	return dwCount;
}

