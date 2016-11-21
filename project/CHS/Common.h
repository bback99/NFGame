//
// Common.h
//

#ifndef COMMON_H
#define COMMON_H

//#include "ErrorLog.h"

#include <NF/NFStruct.h>
#include <NF/ADL/MsgNFCommonStruct.h>
#include <NF/ADL/MsgCHSNGS.h>
#include <NF/ADL/MsgCHSCli_NF.h>

#include "ADL/MsgBLRBCommon.h"
#include <NF/ADL/MsgCHSNCS.h>
#include "ADL/MsgAMSCHS.h"

#include "ADL/MsgCHSGLSCli.h"
#include "ADL/MSGChannelCommon.h"
#include "ADL/MsgInvitation.h"

//#pragma warning(disable:4706)
//#pragma warning(default:4706)
class ChannelPrefix;
class CUser;
class CLRBLink;


extern DWORD g_dwNewLinkCount;
extern DWORD g_dwDelLinkCount;
extern DWORD g_dwChatPart;


//#define MAX_FIELD_COUNT		8

///////////////////////////////////////////////////////////////////////  
// Bootup : Status Code
enum BOOT_STATUS
{
	CHS_STOP = 0,
	CHS_INIT, 
	CHS_INITIALIZING,
	CHS_CONNECT_LRB,
	CHS_CONNECTING_LRB,
	CHS_REG_INSTANCE,
	CHS_REGING_INSTANCE,
	CHS_REG_LB,
	CHS_REGING_LB,
	CHS_START_LISTENER, 
	CHS_STARTING_LISTENER,
	CHS_RUN
};

class HeartBeatFlag
{
	IMPLEMENT_TISAFE(HeartBeatFlag)
public:
	HeartBeatFlag() {m_lFlag = m_lPreFlag = 0L;}
	~HeartBeatFlag() {};

	void SetListenFlag();
	void SetLRBFlag();
	void SetChannelFlag();
	BOOL IsAlive();
	LONG GetFlagValue();
protected:
	LONG m_lFlag;
	LONG m_lPreFlag;
};

extern HeartBeatFlag theHeartBeat;

///////////////////////////////////////////////////////////////////////  
// 송/수신 메세지의 빈도를 측정. 실제 동작과는 무관한 정보.

// LRB Manger를 통해서 CHS가 처리하는 메세지.
class LRBMessageCnt
{
public:
	//from GLS
	DWORD m_dwAddRoom;
	DWORD m_dwRemRoom;
	DWORD m_dwJoinUser;
	DWORD m_dwLeaveUser;
	DWORD m_dwRoomState;
	DWORD m_dwAvatarChange;
	DWORD m_dwGameOption;
	DWORD m_dwRoomOption;

	//from LB
	DWORD m_dwRCList;
	//Channel Count
	DWORD m_dwCreateChannel;

	//from SMGW
	DWORD m_dwChatPart;

	//for delay(interval) 
	DWORD m_dwLRBDelay;

public:
	LRBMessageCnt();
	~LRBMessageCnt();
	void Clear();
	void StampLogNGS(LONG lLevel = 2L) ;
protected:

};

//Listener에서 처리되는 User의 접속요청 메세지
class ListenMessageCnt
{
public:
	DWORD m_dwJoin;	
	DWORD m_dwInvite;	// 초대와 바로가기를 포함.
	DWORD m_dwDirect;

	//for delay
	DWORD m_dwLTNDelay;
public:
	ListenMessageCnt();
	~ListenMessageCnt();
	void Clear();
	void StampLogListen(LONG lLevel = 2L);
};

typedef ArcVectorT<string> AvatarVector;

class IBBDataBus
{
public:
	LONG m_lUSN;	
	string m_sGameData;
	BOOL m_bUserGameDataChanged;
public: 
	IBBDataBus(long lUSN, const string & sGameData, BOOL bUserGameDataChanged = FALSE) : m_lUSN(lUSN)
	{
		m_bUserGameDataChanged = bUserGameDataChanged;
		m_sGameData.assign(sGameData.c_str(), sGameData.length());
	}
private:
	IBBDataBus(const IBBDataBus & bus);
	IBBDataBus& operator=(const IBBDataBus& bus);
};

class LoginStateBus
{
public:
	LONG m_lUSN;
	LONG m_lLoginState;
public: 
	LoginStateBus(long lUSN, LONG lLoginState) : m_lUSN(lUSN), m_lLoginState(lLoginState)
	{
	}
private:
	LoginStateBus(const LoginStateBus & bus);
	LoginStateBus& operator=(const LoginStateBus& bus);
};

class CLink;
class CChannelContext;
typedef AutoPtrT<CChannelContext> CChannelContextPtr;	//neo2

///////////////////////////////////////////////////////////////////////////////////
// CLink
class CLink : public GXLink
{
public:
	typedef GXLink TBase;
	CLink();
	virtual ~CLink();
	void SetUser(CUser* pUser);
	CUser* GetUser() { return m_pUser; }
	BOOL TestSndBufOverflow();
	_inline DWORD GetIndex() const { return m_dwIndex; }

	// For Disconnect 
	void SetDiconCheck(BOOL bCheck) { m_bDisconCheck = bCheck; }
	BOOL GetDiconCheck() { return m_bDisconCheck; }

public:
	template<class T>
	BOOL DoSendMsg(const T& obj)
	{
		GBuf ro;
		if(!::LStore(ro, obj)) return FALSE;
		return DoSend(ro);
	}

	void SetIP(LONG lIP)	{  m_lClientIP = lIP; }
	LONG GetIP()			{ return m_lClientIP; }
protected:
	CUser* m_pUser;
	DWORD m_dwIndex;

	LONG m_lClientIP;

	BOOL m_bDisconCheck;
};

//////////////////////////////////////////////////////////////////////
// CInviteLink

class CInviteLink : public GXLink
{
public:
	typedef GXLink TBase;
public:
	CInviteLink();
	virtual ~CInviteLink();
	BOOL TestSndBufOverflow();
public:
	void SetUserInfo(NFUserBaseInfo& uInfo) { ::BCopy(m_userInfo, uInfo); }
	NFUserBaseInfo& GetUserInfo() { return m_userInfo; }

	void SetNSAP(NSAP& nsapGLS) { ::BCopy(m_nsapNGS, nsapGLS); }
	NSAP& GetNSAP() { return m_nsapNGS; }
	
	void SetRoomID(RoomID& roomID) { ::BCopy(m_roomID, roomID); }
	RoomID& GetRoomID() { return m_roomID; }

	void SetChannelID(ChannelID& channelID) { ::BCopy(m_channelID, channelID); }
	ChannelID& GetChannelID() { return m_channelID; }

public:
	template<class T>
	BOOL DoSendMsg(const T& obj)
	{
		GBuf ro;
		if(!::LStore(ro, obj)) return FALSE;
		return DoSend(ro);
	}

private:
	NFUserBaseInfo m_userInfo;
	ChannelID m_channelID;
	NSAP m_nsapNGS;
	RoomID m_roomID;
};

//////////////////////////////////////////////////////////////////////
// CLRBLink

class CLRBLink : public GXLink
{
public:
	typedef GXLink TBase;
public:
	CLRBLink();
	virtual ~CLRBLink();
	NSAP GetnsapLRB() { return m_nsapLRB; }
	void SetnsapLRB(NSAP nsapLRB); 

	DWORD GetRecvBufSize();
	DWORD GetSendBufQSize();
protected:
	NSAP m_nsapLRB;
};
/////////////////////////////////////////////////////////////////////// 

RoomID Str64ToRoomID(string str);
string RoomIDToStr64(RoomID &rid);

void split(const char* text, const char separator, vector<string>& words);
#endif //!COMMON_H