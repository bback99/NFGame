//
// Common.cpp
//

#include "stdafx.h"
#include "Common.h"
#include "User.h"

//ConstSetFlag theConstSetFlag;
//LONG g_lBootState = 0L;

DWORD g_dwNewLinkCount;
DWORD g_dwDelLinkCount;

///////////////////////////////////////////////////////////////////////////////////
// Heart Beat Flag control
///////////////////////////////////////////////////////////////////////////////////
HeartBeatFlag theHeartBeat;

void HeartBeatFlag::SetListenFlag()
{
	TLock lo(this);
	m_lFlag |= 0x00000001;
}
void HeartBeatFlag::SetLRBFlag()
{
	TLock lo(this);
	m_lFlag |= 0x00000010;
}
BOOL HeartBeatFlag::IsAlive()
{
	TLock lo(this);
	BOOL ret = (m_lFlag == 0x00000011);
	m_lPreFlag = m_lFlag;
	m_lFlag = 0L;
	return ret;
}
LONG HeartBeatFlag::GetFlagValue() 
{
	return m_lPreFlag;
}


///////////////////////////////////////////////////////////////////////////////////
// LRBMessageCnt
///////////////////////////////////////////////////////////////////////////////////
LRBMessageCnt::LRBMessageCnt() 
{ 
	m_dwAddRoom = m_dwRemRoom = m_dwJoinUser = m_dwLeaveUser = m_dwRoomState = m_dwAvatarChange = 
	m_dwGameOption = m_dwRoomOption = m_dwRCList = m_dwCreateChannel = m_dwChatPart = m_dwLRBDelay = 0UL; 
}
LRBMessageCnt::~LRBMessageCnt() 
{

}
void LRBMessageCnt::Clear() 
{ 
	m_dwAddRoom = m_dwRemRoom = m_dwJoinUser = m_dwLeaveUser = m_dwRoomState = m_dwAvatarChange = 
	m_dwGameOption = m_dwRoomOption = m_dwRCList = m_dwCreateChannel = m_dwChatPart = 0UL; 
}

void LRBMessageCnt::StampLogNGS(LONG lLevel) 
{
	if(m_dwLRBDelay < 6)	// Alive check time가 10초 간격으로 동작하기 때문에 매번 LOG를 남길 필요가 없음.
	{
		m_dwLRBDelay++;
		return;
	}
	//	m_dwAddRoom, m_dwRemRoom, m_dwJoinUser, m_dwLeaveUser, m_dwRoomState, \
	//	m_dwAvatarChange, m_dwGameOption, m_dwRoomOption, m_dwRCList, m_dwCreateChannel, m_dwChatPart);
	string strlog = format("LRBCNT : [%d]AR[%d]RR[%d]JU[%d]LU[%d]RS[%d]CA[%d]GO[%d]RO-[%d]RL-[%d]CC-[%d]CP \n", \
		m_dwAddRoom, m_dwRemRoom, m_dwJoinUser, m_dwLeaveUser, m_dwRoomState, \
		m_dwAvatarChange, m_dwGameOption, m_dwRoomOption, m_dwRCList, m_dwCreateChannel, m_dwChatPart);
	LOG(INF_UK, "CHS_LRBMessageCnt", strlog);
	Clear();
	m_dwLRBDelay = 0UL;
}

///////////////////////////////////////////////////////////////////////////////////
// ListenMessageCnt
///////////////////////////////////////////////////////////////////////////////////
ListenMessageCnt::ListenMessageCnt() 
{ 
	m_dwJoin = m_dwInvite = m_dwDirect = m_dwLTNDelay = 0UL;
}
ListenMessageCnt::~ListenMessageCnt() 
{

}
void ListenMessageCnt::Clear() 
{ 
	m_dwJoin = m_dwInvite = m_dwDirect = 0UL;
}

void ListenMessageCnt::StampLogListen(LONG lLevel)
{
	if(m_dwLTNDelay < 6)	// Alive check time가 10초 간격으로 동작하기 때문에 매번 LOG를 남길 필요가 없음.
	{
		m_dwLTNDelay++;
		return;
	}
	string logstr = format("LTNCNT : [%d]J [%d]I [%d]D :: LINKCNT : [%d]New [%d]Del \n", 
		m_dwJoin, m_dwInvite, m_dwDirect, g_dwNewLinkCount, g_dwDelLinkCount);
	LOG(INF_UK, "CHS_ListenMessageCnt"_LK, logstr );
	Clear();
	m_dwLTNDelay = 0UL;
}

//////////////////////////////////////////////////////////////////////////////////
// CLink
///////////////////////////////////////////////////////////////////////////////////
CLink::CLink()
{
	static DWORD s_dwIndex = 0;
	m_dwIndex = ::InterlockedIncrement((LPLONG)&s_dwIndex);
	m_pUser = NULL;
	m_bDisconCheck = FALSE;

	// for check HANDLE leak
	g_dwNewLinkCount = ::InterlockedIncrement((LPLONG)&g_dwNewLinkCount);
}

CLink::~CLink()
{
	g_dwDelLinkCount = ::InterlockedIncrement((LPLONG)&g_dwDelLinkCount);
	m_bDisconCheck = FALSE;
}

void CLink::SetUser(CUser* pUser)
{
	m_pUser = pUser;
}

///////////////////////////////////////////////////////////////////////////////////
CInviteLink::CInviteLink()
{
}

CInviteLink::~CInviteLink()
{
}

///////////////////////////////////////////////////////////////////////////////////
// CLRBLink
///////////////////////////////////////////////////////////////////////////////////
CLRBLink::CLRBLink()
{
}

CLRBLink::~CLRBLink()
{
}

RoomID Str64ToRoomID(string str) // DB 에 64비트만 들어간다고 가정하고 일단 이렇게.
{	
	INT64 rid64;
	rid64 = _atoi64(str.c_str());

	RoomID rid;

	rid.m_dwGRIID = (DWORD)(rid64 & 0x00000000FFFFFFFF);
	rid64 >>= 32;
	rid.m_dwGCIID = (DWORD)(rid64 & 0x00000000FFFFFFFF);

	rid.m_lSSN = rid.m_dwGCIID/1000000;
	rid.m_dwCategory = (rid.m_dwGCIID - rid.m_lSSN*1000000)/1000;

	return rid;
}

string RoomIDToStr64(RoomID &rid)
{
	INT64 rid64;
	rid64 = rid.m_dwGCIID;
	rid64 <<=32;
	rid64 += rid.m_dwGRIID;

	char szRid[30] = "";
	_i64toa(rid64, szRid, 10);

	string str(szRid);
	return str;
}

void split(const char* text, const char separator, vector<string>& words) 
{
	char szBuf[1024] = {0,};
	int nLen = strlen(text);
	for (int i = 0, j = 0; i < nLen ; i++)
	{
		if (text[i] == '\\')
		{
			szBuf[j++] = text[++i];					
		}
		else if (text[i] == separator)
		{
			szBuf[j] = 0x00;
			words.push_back(szBuf);
			j = 0;
		}
		else
			szBuf[j++] = text[i];
	}
	szBuf[j] = 0x00;
	if (strlen(szBuf) >= 0)
		words.push_back(szBuf);
}
