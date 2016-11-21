// NLSManager.h: interface for the NLSManagerData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NLSMANAGER_H__E73D398F_5C09_4E57_816F_62E16F148C6A__INCLUDED_)
#define AFX_NLSMANAGER_H__E73D398F_5C09_4E57_816F_62E16F148C6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _NGSNLS
#define _USENLS
#endif
#ifdef _CHSNLS
#define _USENLS
#endif
#ifdef _NCSNLS
#define _USENLS
#endif

#ifdef _USENLS


#include <NF/ADL/MsgNLSCli.h>

///////////////////////////////////////////////////////////////////////////////////
// NLS와의 통신 및 Transactio처리에 필요한 data storage.
// 다른 부분과의 완전한 독립 유지하도록 구성. -> but 내부에서 외부로의 call은 할수 없도록 한다.
//

#define NLSMAXCNT_USERLIST			3000	//for test // 일단 모든 룸을 커버할 숫자를 사용
#define NLSVALIDATE_TIMERINTERVAL	3000	// waiting user check (msec)
#define NLSLIMIT_WAITINGUSER		100		// NLSVALIDATE_TIMERINTERVAL 시간 동안, 이 숫자 만큼의 응답이 없으면..NLS가 죽은 것으로 판단. 

//
//	Cuser는 INLSObject를 상속받아야 한다..
//
#define INVALID_KEY			0L
#define ISVALID_KEY(key)	(key > INVALID_KEY)
#define MAX_ITEM_COUNT 100

#define ABSOLUTE_DELETE_VALUE	0xffffffff		//// RoomID::m_dwGRIID 를 ABSOLUTE_DELETE_VALUE로 하면 무조건 지운다.

enum 
{
	NLSSTATUS_INVALID = 0, 
	NLSSTATUS_STOP = 1, 
	NLSSTATUS_RUN, 
	NLSSTATUS_RUN2
};

enum 
{
	NLSMODE_UNKNOWN = 0L, 
	NLSMODE_MASTER = 1L,
	NLSMODE_SLAVE = 2L
};

enum
{
	SUBINDEX_MASTER = 0, 
	SUBINDEX_SLAVE
};
//
// 외부와의 연결을 지원한다.
//
#ifdef _NGSNLS
#include "../project/ngs/Room.h"
class CLrbManager;
#endif 

#ifdef _CHSNLS
#include <NF/NFServerDefine.h>
#include "LRBHandler.h"
#include "channel.h"
#define CLrbManager CLRBHandler
class CLrbManager;
#define theLrbManager theLRBHandler
#endif 

#ifdef _NCSNLS
#include <NF/NFServerDefine.h>
#include "../project/ncs/LRBHandler.h"
#define CLrbManager	CLRBHandler;
class CLrbManager;
#define theLrbManager theLRBHandler
#endif

class NLSManager : public IXObject
{
	IMPLEMENT_TISAFE(NLSManager)
public:
	typedef pair<LONG, LPNLSOBJECT> TempUserT;
	typedef list<TempUserT> UserPtrList;

	// for totoal connected user map
	typedef map<LONG, NLSBaseInfo> TotUserMapT;

	NLSManager();
	virtual ~NLSManager();
	//////////////////////////////////////
	// 아래는 반드시 셋팅해야 함.. 
	BOOL RunNLSManager(NSAP & nsap);
	void SetLogicAddr(DWORD dwAddr) { m_dwLogicAddr = dwAddr; }
	void SetTypeID(DWORD dwType) { m_dwTypeID = dwType; }
	void SetNSAP(NSAP & nsap) { m_nsap = nsap; }
	//////////////////////////////////////
	void StopingNLSManager(LRBAddress dwLogicAddr);
	void NLSTerminateNtf(LRBAddress& _Addr);	
	void RecvNLSMessage(const PayloadNLSCLI * pld, const LRBAddress& src);

	BOOL AddUserToNLS(LPNLSOBJECT lpObj, LONG client_ip, string & sOption, LONG lMsgType, LONG lStatus, IXObject * pObj);//CRoomPtr & spRoom);
	void DelUserToNLS(LONG lGSN, RoomID & rid);
	void UpdateUserToNLS(TKey& key, LONG lStatus, RoomID & rid, LONG lLevel, LONG lGameMode = 0);

	DWORD GetLogicAddr() { return m_dwLogicAddr; }
	DWORD GetTypeID() { return m_dwTypeID; }
	const NSAP & GetNSAP() { return m_nsap; }

	void GetUserLocation( PayloadCLINLS& pld );

public: 
	void OnInsertCharacterAns(MsgNLSCLI_InsertCharacterAns & msg);
	void OnDeleteCharacterAns(MsgNLSCLI_DeleteCharacterAns & msg);
	void OnGetCharacterAns(MsgNLSCLI_GetCharacterAns & msg);
	void OnDisconnectUserReq(MsgNLSCLI_DisconnectUserReq & msg);

	void OnAnsLocation(MsgNLSCLI_AnsLocation& msg);

	//

	void SendRegisterServiceAnsToNLS(const LRBAddress& dwNLSAddr);
 	void OnFindUserAnsFromNLS(MsgNLSCLI_GetCharacterAns & msg);
// 	void OnGetUserLCAnsFromNLS(MsgNLSCLI_GetUserLCAns & msg);
// 	void OnUserListReqFromNLS(MsgNLSCLI_UserListReq & pMsg, const LRBAddress& des);
// 	void OnDisconnectUserReqFromNLS(MsgNLSCLI_DisconnectUserReq & msg);
// 	void OnServerRegisterReqFromNLS(MsgNLSCLI_ServerRegisterReq & msg, const LRBAddress& des);
// 	void OnQueryUserStateReqFromNLS(MsgNLSCLI_QueryUserStateReq & msg, const LRBAddress& des);
// 	void OnKickOutUserReqFromNLS(MsgNLSCLI_KickOutUserReq & msg, const LRBAddress& des);
// 	void OnKickOutUserNtfFromNLS(MsgNLSCLI_KickOutUserNtf & msg, const LRBAddress& des);

protected:
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
	void OnValidateCheck();
	void SendNLSMessage(PayloadCLINLS & pld, const LRBAddress& des);
	void PostDBQuery(LPXOBJECT pXObject, LONG lMsgType, LPNLSOBJECT lpObj);

public:
	NSAP m_nsap;

protected:
	GXSigTimer m_NLSTimer; // validity check

	DWORD m_dwRefCnt;

	LONG m_lMsgCount;

	DWORD m_dwTypeID;
	DWORD m_dwLogicAddr;

	TotUserMapT m_mapConnectUser;

#ifdef _NGSNLS
	vector<LONG> m_vecCheckDupAccessSSN;
#endif
};

extern NLSManager theNLSManager;

class CRoomID_For_ChannelReJoin
{
public:
	long m_lMsgType;
	RoomID m_roomID;

	CRoomID_For_ChannelReJoin() { m_lMsgType = 0; }
	CRoomID_For_ChannelReJoin(long lMsgType, RoomID& roomID) : m_lMsgType(lMsgType), m_roomID(roomID) {};
};

#endif	// #ifdef _USENLS

#endif // !defined(AFX_NLSMANAGER_H__E73D398F_5C09_4E57_816F_62E16F148C6A__INCLUDED_)