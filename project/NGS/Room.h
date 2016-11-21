//
// Room.h
//

#ifndef ROOM_H
#define ROOM_H


#include <GXSigUtl.h>
#include "Common.h"

//// Using in accuse
//#pragma oMSG(" #define [_NGS_ACCUSE]")
#define _NGS_ACCUSE
#define ACCUSESIGNAL_AGENTANSWER		((HSIGNAL)0xff000001)

class CRoomInternalLogic;
typedef RCPtrT<CRoomInternalLogic> CRoomInternalLogicPtr;

interface IRoomContext;

#include "RoomEvent.h"
#include <ACHV/AchvEvent.h>



enum {
	ROOMMSG_CREATEREQ = 1,
	ROOMMSG_JOINREQ,
	ROOM_USERLINKCUT,
};


#define ROOM_USERLINKCUTTING						((HSIGNAL)0xffffffff)
#define ROOMSIGNAL_IBBANSWER						((HSIGNAL)0xfffffffa)
#define ROOMSIGNAL_NLSANSWER						((HSIGNAL)0xfffffffb)
#define ROOMSIGNAL_JACKPOT							((HSIGNAL)0xfffffffc)
#define ROOMSIGNAL_HOPEJACKPOT						((HSIGNAL)0xfffffffd)

#define ROOMSIGNAL_PROCESS_NF_FRIEND_LIST			((HSIGNAL)0xffffffaf)	// 模备格废
#define ROOMSIGNAL_PROCESS_NF_FRIEND_APPLICATION	((HSIGNAL)0xffffffbf)	// 模备夸没 贸府
#define ROOMSIGNAL_PROCESS_NF_FRIEND_ACCEPT			((HSIGNAL)0xffffffcf)	// 模备夸没 荐遏 贸府
#define ROOMSIGNAL_PROCESS_NF_FRIEND_AUTO_ACCEPT	((HSIGNAL)0xffffffdf)	// 模备夸没 荐遏 贸府(磊悼)
#define ROOMSIGNAL_PROCESS_NF_LETTER_NEW			((HSIGNAL)0xffffffef)	// 货 祈瘤 舅覆
#define ROOMSIGNAL_PROCESS_FOLLOW_USER				((HSIGNAL)0xfffffaff)



enum JackPotWPARAM
{
	JACKPOT_INVALID,
	JACKPOT_TOTMONEYNTF,
};

enum HopeJackPotWPARAM
{
	HOPEJACKPOT_INVALID,
	HOPEJACKPOT_TOTMONEYNTF
	
};


///////////////////////////////////////////////////////////////////////////////////
// CRoom

//class CRoom : public XSigSafeDataQueueHandlerT<RoomEvent, XLinkAdlManagerT<CLink, PayloadCliNGS,1024*1024> >

class CIRCManager
{
	IMPLEMENT_TIREFCNT0( CIRCManager )
private:

public:
	CIRCManager( CRoomInternalLogicPtr pLogic);
	virtual ~CIRCManager();
	IRoomContext * CreateRoomContext( HINSTANCE m_hGRC, DWORD dwRID );
	void OnDestroyRoomContext( HINSTANCE hGRC, DWORD dwRID, IRoomContext * pContext );

};



class CUser;

class CRoom : public IXObject, public IRoomEventHandler
{
	IMPLEMENT_TISAFE(CRoom)
public:
//	typedef XSigSafeDataQueueHandlerT<RoomEvent, XLinkAdlManagerT<CLink, PayloadCliNGS, 1024*1024> > TBase;
//	typedef IRoomEventHandler TBase;
	typedef map<LONG, LONG> TMapBanishUser;			// USN - RegTime

private:
	CRoom(const CRoom&);
protected:
	CRoom();
	virtual ~CRoom();
friend extern void AllocRoom(CRoom** ppRoom);
friend extern void FreeRoom(CRoom* pRoom);
friend extern void DeleteRoom(CRoom* pRoom);
public:
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	ULONG GetRefCnt() { return m_dwRefCnt; }

		
public: // external interface...		
	long GetState() const;

	LONG GetRunStopFlag(); 
	void SetRunStopFlag(LONG flag);

	LONG GetUserState(LONG lCSN, LONG lType);

	const RoomID& GetRoomID() const;
//	RoomID& GetRoomID();
	string GetRoomIDStr64();

	const LRBAddress& GetAddress() const;
	void ChangeCHSAddr(const LRBAddress& oldAddr, const LRBAddress& newAddr);
	void SetCHSAddr(const LRBAddress &chsAddr);

	BOOL GetRoomInfoInChannel(NFRoomInfoInChannel &roomInfo);

// NF
	virtual BOOL Run(const RoomID& roomID, const LRBAddress& addr, DWORD dwTypeID, const NFRoomOption& nfRoomOption, const string& sGameOption);
	void GetNFRoomOption(NFRoomOption& roomOption);
	void InitUserSlot();
	// chs->ngs
	void OnRcvAnsFreeRoomList(LONG lCSN, MsgCHSNGS_AnsFreeRoomList* pMsg);	
	LONG GetAchvUserInfo(LONG lCSN, string& strNickName);
	void SendToUser(LONG lCSN, const PayloadNGSCli& pld);

	static void AchvReportCallback(LONG GSN, LONG CSN, int achv_ID, const achv::EventItem_T *pEvtItem);

// NF
	virtual BOOL Stop();

	void OnBroadCastMessage( GBuf & gbuf );

	void SetChatForward();
	void ResetChatForward();

public:
	//Processing msgs from user
	void OnAnnounceMsg(const xstring& sAnnounce);
	void MulticastNotify(GBuf& gBuf);	
	void OnRKSAns(long lMsgType, DWORD dwRankType, GBuf &buf, DWORD dwUSN = 0);
	void OnIBBAns(long lCSN, string& sUserGameData, LONG lMsgType);


	void OnTerminateRoomReq(LONG lType);

private:
	BOOL ProcessCreateRoomReq( CUser * pUser );
	BOOL ProcessJoinRoomReq( CUser * pUser );

	void OnUserMsg(LONG lCSN);

	void ProccssCutoutUser( LONG lCSN, LinkCutReason reason );
	void OnKickOutBanashUser();

	void RoomCreateAndJoin(CUser * pUser, LONG lMsgType);
	LONG RemoveAllTimer();
	void UserLinkCut(DWORD dwIndex);

	BOOL ProcessDisconnectedUser( long lCSN );

	BOOL ProcessRoomEvent( const RoomEvent &evt );
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);			// TIMER..

	void ProcessAccuseSignal( const HSIGNAL &hObj, const WPARAM &wParam, const LPARAM &lParam );
	void ProcessTimerSignal( const HSIGNAL &hObj, const WPARAM &wParam, const LPARAM &lParam ,DWORD beforeLockTick );

	void ProcessUserMsg(LONG lCSN, PayloadCliNGS* pPld);

	void OnRcvChangeRoomOptionNtf(LONG lCSN, MsgCliNGS_ChangeRoomOptionNtf* pMsg);
	void OnRcvCreateRoomNtf(LONG lCSN , PayloadCliNGS* pPld);
	void OnRcvJoinRoomNtf(LONG lCSN , PayloadCliNGS* pPld);
	void OnRcvLeaveRoomReq(LONG lCSN , PayloadCliNGS* pPld);

	void ProcessNFFriendList(const MsgNLSCLI_AnsLocation& msg);
	void ProcessNFFriendApplication(const MsgNLSCLI_AnsLocation& msg);
	void ProcessNFFriendAccept(const MsgNLSCLI_AnsLocation& msg);
	void ProcessNFFriendAutoAccept(const MsgNLSCLI_AnsLocation& msg);
	void ProcessNFLetterReceive(const MsgNLSCLI_AnsLocation& msg);
	void ProcessFollowUser(const MsgNLSCLI_AnsLocation& msg);
private:
	HINSTANCE m_hGRC;
	IRoomContext * m_pIRC;

	CRoomInternalLogicPtr m_pInternalLogic;

	RCPtrT< CIRCManager > m_pIRCManager;

	DWORD m_dwRefCnt;
	DWORD m_dwRID;


	GXSigTimer m_timerAlive;

// NF
	GXSigTimer		m_timerGNFGame;
	GXSigTimer		m_timerBanishUser;
	TMapBanishUser	m_mapBanishUser;
// NF

protected:


	DWORD m_dwTypeID;

};


typedef RCPtrT<CRoom> CRoomPtr;

void AllocRoom(CRoom** ppRoom);
void FreeRoom(CRoom* pRoom);
void DeleteRoom(CRoom* pRoom);

#endif //!ROOM_H
