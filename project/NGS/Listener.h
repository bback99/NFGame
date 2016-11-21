//
// Listener.h
//

#ifndef LISTENER_H
#define LISTENER_H

#include "Common.h"

class CListener;
class CLinkCounter;



enum {
	LISTENERMSG_CREATEANS = 1,
	LISTENERMSG_JOINANS,
	LISTENER_ONACCEPT

//	LISTENERMSG_CREATEREQ,
//	LISTENERMSG_JOINREQ
};

#define HSIGNAL_ONACCEPT HSIGNAL(0xFFFFFFFF)
///////////////////////////////////////////////////////////////////////////////////
// CListener

class CListener : public XLinkAdlManagerT<CLink, PayloadCliNGS,1024*1024>
{
	IMPLEMENT_TISAFEREFCNT(CListener)
public:
	typedef XLinkAdlManagerT<CLink, PayloadCliNGS, 1024*1024> TBase;
	typedef PayloadCliNGS TMsg;
private:
	CListener(const CListener&);
public:
	CListener();
	virtual ~CListener();
public:
	virtual BOOL Run(int nPort);
	virtual BOOL Stop();
public:
	void OnLinkMove(CLink* pLink);
//#define	_TESTTOOL_DEBUG_
#ifdef _TESTTOOL_DEBUG_
	GXSigTimer m_timerPrintRoomCnt;
#endif

protected:
	void SendMsg(CLink* pSocket, const PayloadNGSCli& pld);
protected:
	virtual void DestroyLink(CLink* pLink);

public:
	virtual BOOL OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR szAddr, LONG lPort);		// 2002.10.15 Client IP 얻어오기 위함.
protected:
	virtual BOOL OnError(CLink* pSocket, long lEvent, int nErrorCode);
protected:
	virtual BOOL OnRcvMsg(CLink* pSocket, PayloadCliNGS& pld);
	BOOL OnRcvMsgOrg(CLink* pSocket, PayloadCliNGS& pld);											// 2009.07.06 searer - 암호화 작업을 위해 기존 OnRcvMsg 함수 이전
	virtual void SendToAll(const PayloadNGSCli& pld);

	// PayloadCliNGS
	BOOL OnRcvCreateRoomReq(CLink* pLink, MsgCliNGS_CreateRoomReq* pMsg);
	BOOL OnRcvJoinRoomReq(CLink* pLink, MsgCliNGS_JoinRoomReq* pMsg);

protected:
	void TLOGAlive();
#ifdef _DEBUG
	GXSigTimer m_timerAlive;
#endif
protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hObj, WPARAM wParam, LPARAM lParam);

private:
	GXSigTimer m_timerRoomTitleMonitor;	// for Room Title monitoring
	DWORD m_dwListenerHandle;
};


extern CListener theListener;

#endif //!LISTENER_H
