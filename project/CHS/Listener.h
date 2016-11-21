//
// Listener.h
//

#ifndef LISTENER_H
#define LISTENER_H

#include "Common.h"
#include "stdafx.h"

class CListener;
class CLinkCounter;

#define HSIGNAL_ONACCEPT HSIGNAL(0xFFFFFFFF)

//// 채널/룸 중복접속 방지 위한 게임 설정 /// 고스톱, 맞고, 맞고 플러스, 정정당당 맞고, 호구 로봇, new 맞고. 포커류
extern map<long, long> gVecChannelRejoinGame;
#define FLAG_CHANNEL_REJOIN		100
#define IS_SINGLE_JOIN_SSN(a) (gVecChannelRejoinGame.find(a) == gVecChannelRejoinGame.end() || (gVecChannelRejoinGame.find(a))->second != FLAG_CHANNEL_REJOIN) ? FALSE : TRUE

///////////////////////////////////////////////////////////////////////////////////
// CSLinkAdlMgrT
#include <time.h>
class CSLinkAdlMgrT
{
#define LINK_TIME_GAME		20/*180*/		///Sec
	typedef map<CLink*, FILETIME> MAP_LINK_TIME;
	IMPLEMENT_TISAFEREFCNT(CSLinkAdlMgrT)
public:
	CSLinkAdlMgrT(CListener* pListener) { m_pListener = pListener; /*m_mapAddrLink.clear();*/ m_mapLinkLastRcvTime.clear(); }
	CSLinkAdlMgrT() { m_pListener = NULL; /*m_mapAddrLink.clear();*/ m_mapLinkLastRcvTime.clear(); }
	~CSLinkAdlMgrT() { /*m_mapAddrLink.clear();*/ m_mapLinkLastRcvTime.clear(); }

	void SetListener(CListener* pListener) { m_pListener = pListener; }

	BOOL AddSLink(CLink* pLink);
	BOOL RemoveSLink(CLink* pLink);
	void CheckSLink();
	void UpdateTime(CLink *pLink);
	int DiffFileTime(FILETIME aFT, FILETIME bFT, FILETIME& retFT);

private:
	//map<long, CLink*>			m_mapAddrLink;			///// Key = Link Address,	Data = CLink*
	MAP_LINK_TIME				m_mapLinkLastRcvTime;	///// Key = TLink*,			Data = Last RcvTime;
	CListener* m_pListener;

	friend class CListener;
};


///////////////////////////////////////////////////////////////////////////////////
// CListener
class CListener : public XLinkAdlManagerT<CLink, PayloadCliCHS>, public CSLinkAdlMgrT
{
	IMPLEMENT_TISAFEREFCNT(CListener)
public:
	typedef XLinkAdlManagerT<CLink, PayloadCliCHS> TBase;
	typedef PayloadCliCHS TMsg;
private:
	CListener(const CListener&);
public:
	CListener();
	virtual ~CListener();
public:
	virtual BOOL RunListen(int nPort);
	virtual BOOL StopListen();
protected:
	void SendMsg(CLink* pSocket, const PayloadCHSCli& pld);
//protected:
public:
	virtual void DestroyLink(CLink* pLink);
	
public:
	virtual BOOL OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR lpAddr, LONG lPort);
protected:
	virtual BOOL OnError(CLink* pSocket, long lEvent, int nErrorCode);
protected:
	virtual BOOL OnRcvMsg(CLink* pLink, PayloadCliCHS& pld);
	virtual void SendToAll(const PayloadCHSCli& pld);

	// PayloadCS
	BOOL OnGRInviteReq(CLink* pLink, MsgCliCHS_GRInviteReq* pMsg);
	BOOL OnRcvJoinChannelReq(CLink* pLink, MsgCliCHS_JoinChannelReq* pMsg);
	BOOL OnGRDirectCreateReq(CLink* pLink, MsgCliCHS_GRDirectCreateReq* pMsg);
	void SendListenerMsg(CLink & link, LONG lErrCode);	
protected:
	void OnChannelJoinAns(TLink* pLink, LONG lErrCode);
	void OnChannelInviteAns(CLink * pLink, LONG lErrCode);
	void OnChannelDirectJoinAns(CLink * pLink, LONG lErrCode);
protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);//BOOL bFired);			// TIMER..

	DWORD m_dwListener;
	GXSigTimer m_timerAlive;
	GXSigTimer m_timerResetUserCnt;

	ListenMessageCnt m_LMsgCount;
	CSLinkAdlMgrT m_SLinkAdlMgrT;

public:
	BOOL m_bStating;
};

extern CListener theListener;
////////////////////////////////////////////////////////////////////////////////////////////////////////
class CInviteListener : public XLinkAdlManagerT<CInviteLink, PayloadInvitation>
{
	IMPLEMENT_TISAFEREFCNT(CInviteListener)
public:
	typedef XLinkAdlManagerT<CInviteLink, PayloadInvitation> TBase;
	typedef PayloadCliCHS TMsg;
private:
	CInviteListener(const CInviteListener&);
public:
	CInviteListener();
	virtual ~CInviteListener();
public:
	virtual BOOL RunListen(int nPort);
	virtual BOOL StopListen();
protected:
	void SendMsg(CInviteLink* pSocket, const PayloadInvitation& pld);
protected:
	virtual void DestroyLink(CInviteLink* pLink);
	
public:
	virtual BOOL OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR lpAddr, LONG lPort);
protected:
	virtual BOOL OnError(TLink* pSocket, long lEvent, int nErrorCode);
protected:
	virtual BOOL OnRcvMsg(CInviteLink* pLink, PayloadInvitation& pld);
	BOOL OnRcvInvitationInfoReq(CInviteLink* pLink, MsgInvitationInfoReq* pMsg);
	void SendMsg(CInviteLink* pLink, PayloadInvitation& pld);
	virtual void SendToAll(const PayloadInvitation& pld);
protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);//BOOL bFired);			// TIMER..

	DWORD m_dwListener;
	ListenMessageCnt m_LMsgCount;
public:
	BOOL m_bStating;
};

extern CInviteListener theInviteListener;

#endif //!LISTENER_H
