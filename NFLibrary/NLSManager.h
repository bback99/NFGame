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
// NLS���� ��� �� Transactioó���� �ʿ��� data storage.
// �ٸ� �κа��� ������ ���� �����ϵ��� ����. -> but ���ο��� �ܺη��� call�� �Ҽ� ������ �Ѵ�.
//

#define NLSMAXCNT_USERLIST			3000	//for test // �ϴ� ��� ���� Ŀ���� ���ڸ� ���
#define NLSVALIDATE_TIMERINTERVAL	3000	// waiting user check (msec)
#define NLSLIMIT_WAITINGUSER		100		// NLSVALIDATE_TIMERINTERVAL �ð� ����, �� ���� ��ŭ�� ������ ������..NLS�� ���� ������ �Ǵ�. 

//
//	Cuser�� INLSObject�� ��ӹ޾ƾ� �Ѵ�..
//
#define INVALID_KEY			0L
#define ISVALID_KEY(key)	(key > INVALID_KEY)
#define MAX_ITEM_COUNT 100

#define ABSOLUTE_DELETE_VALUE	0xffffffff		//// RoomID::m_dwGRIID �� ABSOLUTE_DELETE_VALUE�� �ϸ� ������ �����.

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
// �ܺο��� ������ �����Ѵ�.
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
	// �Ʒ��� �ݵ�� �����ؾ� ��.. 
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