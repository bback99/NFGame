//
// CHSInfoDir.h
//

#ifndef CHSInfoDir_h
#define CHSInfoDir_h

#include "Common.h"



// PCRoom UserCount Class
/////////////////////////////////////////////////////////////////////////////////// 
class ChannelUpdateInfoPCRoom : public ChannelUpdateInfo
{
public:
	LONG	m_lPCRoomUserCount;
public:
	ChannelUpdateInfoPCRoom()
	{
		m_lPCRoomUserCount = 0L;
	}

	void Clear()
	{
		m_lPCRoomUserCount = 0L;
	}
};


typedef ArcListT<ChannelUpdateInfoPCRoom> ChannelUpdateInfoPCRoomList;





/////////////////////////////////////////////////////////////////////////////////// 
// CHSInfoDir

class CHSInfoDir
{
	IMPLEMENT_TISAFE(CHSInfoDir)
protected:
	//typedef std::pair<ChannelUpdateInfo, bool> CHSInfo;
	typedef std::pair<ChannelUpdateInfoPCRoom, bool> CHSInfo;

	typedef map<ChannelID, CHSInfo> TCHSInfoMap; 
	TCHSInfoMap m_mapCHSInfo;
public:
	CHSInfoDir();
	virtual ~CHSInfoDir();
public:
#ifdef _PMS_CODE_SEPERATION_
	void InsertCHSInfo(const ChannelID& channelID, LONG lUserCount = 0, LONG lWaitingUserCount = 0, LONG lRoomCount = 0, LONG lPCRoomUserCount = 0);
	BOOL DeleteCHSInfo(const ChannelID& channelID);
#else
	void AddCHSInfo(const ChannelID& channelID, LONG lUserCount, LONG lWaitingUserCount, LONG lRoomCount, LONG lPCRoomUserCount);
	BOOL RemoveCHSInfo(const ChannelID& channelID);
#endif	
	BOOL UpdateCHSInfo(const ChannelID& channelID, LONG lUserCount, LONG lWaitingUserCount, LONG lRoomCount, LONG lPCRoomUserCount);
	DWORD GetCHSInfoList(ChannelUpdateInfoList& lst, BOOL bAll, BOOL bClearFlags);
	DWORD GetCHSInfoListForPMS(ChannelUpdateInfoList & lst, BOOL bAll = FALSE);
	DWORD GetCHSInfoListForPMS(ChannelUpdateInfoPCRoomList & lst, BOOL bAll = FALSE);
	DWORD ClearFlags();
	DWORD SetAllFlags();
};

extern CHSInfoDir theCHSInfoDir;

#endif //!CHSInfoDir_h
