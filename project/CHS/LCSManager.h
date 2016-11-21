// LCSManager.h: interface for the LCSManagerData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LCSMANAGER_H__E73D398F_5C09_4E57_816F_62E16F148C6A__INCLUDED_)
#define AFX_LCSMANAGER_H__E73D398F_5C09_4E57_816F_62E16F148C6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _GLSLCS
	#define _USELCS
#endif
#ifdef _CHSNLS
	#define _USELCS
#endif
#ifdef _USELCS

//
// LCS와의 통신 및 Transactio처리에 필요한 data storage.
// 다른 부분과의 완전한 독립 유지하도록 구성. -> but 내부에서 외부로의 call은 할수 없도록 한다.
//

#include "Common.h"
#include "ADL/MsgLCSCommon2.h"
#include "ADL/MsgLCSMSG2.h"

#define LCSMAXCNT_USERLIST			3000	//for test // 일단 모든 룸을 커버할 숫자를 사용
#define LCSVALIDATE_TIMERINTERVAL	3000	// waiting user check (msec)
#define LCSLIMIT_WAITINGUSER		100		// LCSVALIDATE_TIMERINTERVAL 시간 동안, 이 숫자 만큼의 응답이 없으면..LCS가 죽은 것으로 판단. 

//
//	Cuser는 INLSObject를 상속받아야 한다..
//
#define INVALID_KEY			0L
#define ISVALID_KEY(key)	(key > INVALID_KEY)
#define MAX_ITEM_COUNT 100

#define ABSOLUTE_DELETE_VALUE	0xffffffff		//// RoomID::m_dwGRIID 를 ABSOLUTE_DELETE_VALUE로 하면 무조건 지운다.

enum 
{
	LCSSTATUS_INVALID = 0, 
	LCSSTATUS_STOP = 1, 
	LCSSTATUS_RUN, 
	LCSSTATUS_RUN2
};

enum 
{
	LCSMODE_UNKNOWN = 0L, 
	LCSMODE_MASTER = 1L,
	LCSMODE_SLAVE = 2L
};

enum
{
	SUBINDEX_MASTER = 0, 
	SUBINDEX_SLAVE
};
// for Partitioning 
// Master/Slave server Ip를 pair로 관리.
// Vector<Pair<Pair<MasterIP, ONOFF>, Pair<Slave, ONOFF>>>의 형태로 관리한다.
struct IPUNIT
{
	LRBAddress m_dwIP;
	LRBAddress m_dwAddr;
	BOOL m_bOnOff;
public:
	IPUNIT() { m_dwIP.Clear(); m_dwAddr.Clear(); m_bOnOff = FALSE; }
	IPUNIT(LRBAddress dwIP, LRBAddress dwAddr, BOOL bOnOff) : m_dwIP(dwIP), m_dwAddr(dwAddr), m_bOnOff(bOnOff) {};
};

class LCSIPSet
{
	IMPLEMENT_TISAFE(LCSIPSet)
public:
	typedef pair<IPUNIT, IPUNIT> SET;
	typedef vector<SET> vecIPSet;

public:
	LCSIPSet() { m_lLCSCount = 0L; m_vecIPSet.clear(); }
	~LCSIPSet() {};

public:
	BOOL InitIPSet(const vector<LRBAddress> & vecIPSet, LRBAddress dwMasterIP, LRBAddress dwAddr);	// 처음 한번만 등록한다. 모든 LCS의 IPSet 정보는 동일해야 함.
	void UpdateLogacalAddr(LRBAddress dwIP, LRBAddress dwAddr);
	void StopLCS(LRBAddress dwAddr);
	BOOL StartLCS(LRBAddress dwAddr);

	LRBAddress SelectLCS(LONG lKey);
	LRBAddress GetMasterIP(LONG lIndex, BOOL bOnOff = TRUE);
	LRBAddress GetSlaveIP(LONG lIndex, BOOL bOnOff = TRUE);
	LRBAddress GetMasterAddr(LONG lIndex, BOOL bOnOff = TRUE);
	LRBAddress GetSlaveAddr(LONG lIndex, BOOL bOnOff = TRUE);
	LONG GetLCSCount() { return m_lLCSCount; }
	DWORD FindAddrIndex(LRBAddress dwLogicalAddr);

protected:
	LONG ExtractIndex(LONG lKey);
	LONG FindIP(LRBAddress dwIP, LONG &lSubIndex, BOOL bOnOff = TRUE);	// TRUE == ON, return Index
	LONG FindAddr(LRBAddress dwAddr, LONG &lSubIndex, BOOL bOnOff = TRUE);
	BOOL ChangeAddrSetOnOff(LRBAddress dwAddr, BOOL bOnOff);		// On or Off
	BOOL ChangeAddrSetMode(LRBAddress dwAddr, LONG lMode);		// Master or Slave
private:
	vecIPSet m_vecIPSet;
	LONG m_lLCSCount;

};

//  LCS manager가 내부적으로 유지할 데이터 
class LCSManagerData 
{
	IMPLEMENT_TISAFE(LCSManagerData)
public:
	typedef pair<KeyItem, LPNLSOBJECT> BaseT;
	typedef list<BaseT> LMT;
	typedef LMT::iterator iterator;

	typedef pair<LONG, LPNLSOBJECT> TempUserT;
	typedef list<TempUserT> UserPtrList;

public:
	LCSManagerData();
	virtual ~LCSManagerData();

	LONG PushItem(LPNLSOBJECT lpObj, LONG lType, DWORD & dwSerial);
	LONG PopItem(LONG lUSN, DWORD dwSerial, LONG &lType, LPNLSOBJECT * lpObj);
	void PopItem(LONG & lType, LPNLSOBJECT * lpObj);
	void RemItem(LONG lUSN);
	LONG CheckValidity(UserPtrList & lstUserPtr, LONG & ltotcnt);

	void SetTimeInterval(LONG lInterval)
	{
		m_lTimeInterval = lInterval;
		m_lMaxItemCnt = 100 * m_lTimeInterval / 1000 ;
	}

private:
	// 완전히 일치하는 Item을 찾는다. (USN and Serial)
	iterator Find(LONG lUSN, DWORD dwSerial)
	{
		ForEachElmt(LMT, m_LCSList, it, jt)
		{
			if((*it).first.m_lUSN == lUSN && (*it).first.m_dwSerial == dwSerial)
				return it;
		}
		return m_LCSList.end();
	}

	// USN만 일치하는 Item을 찾는다.
	iterator Find(LONG lUSN)
	{
		ForEachElmt(LMT, m_LCSList, it, jt)
		{
			if((*it).first.m_lUSN == lUSN)
				return it;
		}
		return m_LCSList.end();
	}

	inline DWORD GetSerial()
	{
		return ::InterlockedIncrement((LPLONG)&m_dwSerialNo);
	}

protected:
	LMT m_LCSList;
	DWORD m_dwSerialNo;

	LONG m_lMaxItemCnt;
	LONG m_lTimeInterval;

public:

};

//
// 외부와의 연결을 지원한다.
//
#ifdef _GLSLCS
#include "Room.h"
class CLrbManager;
#endif 

#ifdef _CHSNLS
#include "LRBHandler.h"
#include "channel.h"
#define CLrbManager CLRBHandler
class CLrbManager;
#define theLrbManager theLRBHandler
#endif 

class LCSManager : public IXObject
{
	IMPLEMENT_TISAFE(LCSManager)
public:
	typedef pair<LONG, LPNLSOBJECT> TempUserT;
	typedef list<TempUserT> UserPtrList;

	// for totoal connected user map
	typedef map<LONG, LCSBaseInfo> TotUserMapT;

	LCSManager();
	virtual ~LCSManager();
//////////////////////////////////////
// 아래는 반드시 셋팅해야 함.. 
	BOOL RunLCSManager(CLrbManager * pLRB, NSAP & nsap);
	void SetLogicAddr(DWORD dwAddr) { m_dwLogicAddr = dwAddr; }
	void SetTypeID(DWORD dwType) { m_dwTypeID = dwType; }
	void SetNSAP(NSAP & nsap) { m_nsap = nsap; }
//////////////////////////////////////
	void StopingLCSManager(LRBAddress dwLogicAddr);
	void LCSTerminateNtf(LRBAddress& _Addr);	

	void RecvLCSMessage(const PayloadLCSOUT * pld, const LRBAddress& src);
	void ReqRemUser(LONG lUSN, RoomID & rid);
	BOOL AddUserToNLS(LPNLSOBJECT lpObj, LONG client_ip, string & sOption, LONG lMsgType, IXObject * pObj);//CRoomPtr & spRoom);
	DWORD GetLogicAddr() { return m_dwLogicAddr; }
	DWORD GetTypeID() { return m_dwTypeID; }
	const NSAP & GetNSAP() { return m_nsap; }

#ifdef _GLSLCS
	BOOL SetAutoPlayNotify(LONG lUSN, RoomID& rid);
	BOOL ResetAutoPlayNotify(LONG lUSN, RoomID& rid);

	BOOL CheckDuplicatedAccessBySSN(LONG lSSN);
#endif

public: 
	void SendRegisterServiceAnsToLCS(const LRBAddress& dwLCSAddr);
protected: 
	void OnAddUserAnsFromLCS(MsgLCSOUT_AddUserAns & pld);
	void OnFindUserAnsFromLCS(MsgLCSOUT_FindUserAns & msg);
	void OnGetUserLCAnsFromLCS(MsgLCSOUT_GetUserLCAns & msg);
	void OnUserListReqFromLCS(MsgLCSOUT_UserListReq & pMsg, const LRBAddress& des);
	void OnDisconnectUserReqFromLCS(MsgLCSOUT_DisconnectUserReq & msg);
	void OnServerRegisterReqFromLCS(MsgLCSOUT_ServerRegisterReq & msg, const LRBAddress& des);
	void OnQueryUserStateReqFromLCS(MsgLCSOUT_QueryUserStateReq & msg, const LRBAddress& des);
	void OnKickOutUserReqFromLCS(MsgLCSOut_KickOutUserReq & msg, const LRBAddress& des);
	void OnKickOutUserNtfFromLCS(MsgLCSOut_KickOutUserNtf & msg, const LRBAddress& des);

protected:
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
	void OnValidateCheck();
	void SendLCSMessage(PayloadOUTLCS & pld, const LRBAddress& des);
	void PostDBQuery(LPXOBJECT pXObject, LONG lMsgType, LPNLSOBJECT lpObj);

	// for totoal connected user map
	void AddUser(LCSBaseInfo & baseInfo);
public:
	void RemUser(LONG lKey);
protected:
	LONG ExtractUser(LONG lPartKey, LONG lLCSCnt, LCSBaseInfoList & lstUser);

public:
	CLrbManager * m_pLRBConnector;
	NSAP m_nsap;

protected:
	LCSManagerData m_LCSData;
	GXSigTimer m_LCSTimer; // validity check

	DWORD m_dwRefCnt;

	LONG m_lMsgCount;
	LCSIPSet m_LCSIPSet;

	DWORD m_dwTypeID;
	DWORD m_dwLogicAddr;

	TotUserMapT m_mapConnectUser;

#ifdef _GLSLCS
	vector<LONG> m_vecCheckDupAccessSSN;
#endif
};

extern LCSManager theNLSManager;

class CRoomID_For_ChannelReJoin
{
public:
	long m_lMsgType;
	RoomID m_roomID;

	CRoomID_For_ChannelReJoin() { m_lMsgType = 0; }
	CRoomID_For_ChannelReJoin(long lMsgType, RoomID& roomID) : m_lMsgType(lMsgType), m_roomID(roomID) {};
};

#endif

#endif // !defined(AFX_LCSMANAGER_H__E73D398F_5C09_4E57_816F_62E16F148C6A__INCLUDED_)