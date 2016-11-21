//
// StatisticsTable.h
//

#ifndef STATISTICSTABLE_H
#define STATISTICSTABLE_H

#include "Common.h"
#ifndef _CHSLCS
	#include <ADL/MsgAMSGLS.h>
#endif

//#define __PMS_DEBUG__

#ifdef _PMS_CODE_SEPERATION_
class PMSStatInfoPerChanneling {
public :
	PMSStatInfoPerChanneling() : m_lWaitingUserCount(0), m_lUserCount(0) {
	}
	ChannelID	m_channelID;
	LONG	m_lWaitingUserCount;	// CHS 대기실에 붙어 있는 사용자 수
	LONG	m_lUserCount;		// CHS, GLS 전체 통틀어 채널에 있는 모든 사용자 수
};

typedef std::map<ChannelID, PMSStatInfoPerChanneling> PMSStatInfoPerChannelingMap; 
typedef std::list<PMSStatInfoPerChanneling> PMSStatInfoPerChannelingList;
#endif

class ChannelInfoPCRoom : public ChannelInfo {

public:
	LONG	m_lPCRoomUserCount;
public:
	typedef ChannelInfo TBase;

	ChannelInfoPCRoom()
	{
		Clear();
	}
	virtual void Clear()
	{
		m_lPCRoomUserCount = 0L;
	}
};

typedef list<ChannelInfoPCRoom> ChannelInfoPCRoomList;


class UserStatInfo {
public:
	void Copy(const UserBaseInfo& ubi) {
		m_lUSN = ubi.m_lUSN;
		m_lUSC = ubi.m_lUSC;			
		m_lAgeIndex = ubi.m_lAgeIndex;
		m_lJobIndex = ubi.m_lJobIndex;
		m_lRegionIndex = ubi.m_lRegionIndex;
		m_lGuildIndex = ubi.m_lGuildIndex;
		m_lMemberIndex = ubi.m_lMemberIndex;
		m_lIsPCRoomUser = ubi.m_lUserType;				// lUserType이 0이 아닐 경우 PC방 유저임
#ifdef _PMS_CODE_SEPERATION_
		m_lPMSCode = (GetPMSCode(ubi.m_sMemberInfo) / 100)*100;
#endif
		
	}
public:
	LONG m_lUSN;
	LONG m_lJobIndex;
	LONG m_lAgeIndex;
	LONG m_lUSC;
	LONG m_lGuildIndex;
	LONG m_lMemberIndex;
	LONG m_lRegionIndex;
	LONG m_lIsPCRoomUser;
#ifdef _PMS_CODE_SEPERATION_
	LONG m_lPMSCode;

private :
	LONG GetPMSCode(const std::string& sMemberInfo)
	{
		int len = sMemberInfo.length();

		int stop = 0;
		int start = sMemberInfo.find_first_not_of(",");
		
		while ((start >= 0) && (start < len)) {
			stop = sMemberInfo.find_first_of(",", start);
			if ((stop < 0) || (stop > len))
			{
				stop = len;			
			}
			
			std::string code = sMemberInfo.substr(start, stop - start);

			std::string::size_type delPos = code.find("=");
			if(std::string::npos == delPos)
			{
				return 0;
			}
			std::string key = code.substr(0, delPos );
			std::string value = code.substr(delPos+1);
			if("PMSCODE" == key)
			{
				return atoi(value.c_str());
			}
			start = sMemberInfo.find_first_not_of(",", stop+1);
		}
		return 0;
	}
#endif
};

///////////////////////////////////////////////////////////////////////////////////
// CStatisticsTable
#ifdef _USE_MSGCOUNT
class CStatisticsTable : public IXObject
#else
class CStatisticsTable
#endif
{
#ifdef _USE_MSGCOUNT
	IMPLEMENT_TISAFEREFCNT(CStatisticsTable)
#else
	IMPLEMENT_TISAFE(CStatisticsTable)
#endif

public:
	typedef map<LONG, StatisticInfo> StatMap; // SSN -> Statistics
public:
	CStatisticsTable();
	virtual ~CStatisticsTable();
public:
	BOOL GetStatInfo(StatInfoList& lstStat);
	BOOL GetStatInfoPC(StatInfoList& lstStat);

#ifdef _NGSNLS	
	BOOL SetStatInfo(RoomID &rid, UserStatInfo& info, LONG lDiff, int debug_type);
	BOOL SetStatInfoPC(RoomID &rid, UserStatInfo& info, LONG lDiff, int debug_type);
#else
	BOOL SetStatInfo(LONG lSSN, UserStatInfo& info, LONG lDiff);
	BOOL SetStatInfoPC(LONG lSSN, UserStatInfo& info, LONG lDiff);
#endif

	LONG GetTotalCU();
	LONG GetTotalCUPC();
protected:
	StatMap mStatMap;
	StatMap mStatMapPC;
	StatisticInfo m_TotalStatInfo;
	StatisticInfo m_TotalStatInfoPC;

#ifdef _NGSNLS	
	typedef map<LONG, LONG> UsnCount; // <USN, Count>
	typedef map<LONG, UsnCount*> UserMap; // <SSN, UsnList*>
	UserMap	m_mapSSNUser;		    
#endif

#ifdef _USE_MSGCOUNT
//////////////////////////////////////////////////////////////////////////
// 2007.10.08 추가
private:
	typedef map<LONG, LONG> MsgCountMap; // <SSN, MsgCount>
	MsgCountMap m_mapMsgCount;
	string m_sMyIP;
	GXSigTimer m_timerSaveMsg;	
public:
	void Init();
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
	void AddMsgCount(LONG lSSN);
	void ClearMsgCount();
	void SaveMsgCount();
#endif

#ifdef _PMS_CODE_SEPERATION_
public :
	void UpdateSessionStatInfoDelta(const ChannelID& cid, const UserStatInfo& usi, LONG userCountDelta, LONG waitingUserCount);
	DWORD GetSessionStatInfoList(PMSStatInfoPerChannelingList& channelingInfoList);
private :
		PMSStatInfoPerChannelingMap m_mapChannelingCUCount;	// 각 사 채널링별 CU 정보
#endif
};

extern CStatisticsTable theStatTable;

#endif //!STATISTICSTABLE_H
