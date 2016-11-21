// 
// Listener.h
// 
/**********************************************************
Listener for Client
-bback99
***********************************************************/

#ifndef	_LISTENER_H__
#define _LISTENER_H__


#include <NF/ADL/MsgNCSCli.h>
#include "Link.h"


#define HSIGNAL_ONACCEPT HSIGNAL(0xFFFFFFFF)
#define LISTENERMSG_LINKCUT HSIGNAL(0xFFFFFFF1)

class CListener : public XLinkAdlManagerT<CLink, PayloadCliNCS,1024*1024>
{
	IMPLEMENT_TISAFEREFCNT(CListener)
public:
	typedef XLinkAdlManagerT<CLink, PayloadCliNCS, 1024*1024> TBase;
	typedef PayloadCliNCS TMsg;
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


public:
	virtual BOOL OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR szAddr, LONG lPort);		// 2002.10.15 Client IP 얻어오기 위함.
	// Message Handler

	BOOL OnReqJoinNCS(CLink* pLink, MsgCliNCS_ReqJoinNCS* pMsg);
	virtual void DestroyLink(CLink* pLink);


protected:
	virtual BOOL OnError(CLink* pSocket, long lEvent, int nErrorCode);
	virtual BOOL OnRcvMsg(CLink* pSocket, PayloadCliNCS& pld);
	virtual void SendToAll(const PayloadNCSCli& pld);
	void SendMsg(CLink* pSocket, const PayloadNCSCli& pld);
	

protected:
	void TLOGAlive();
#ifdef _DEBUG
	GXSigTimer m_timerAlive;
#endif
protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hObj, WPARAM wParam, LPARAM lParam);

private:
	GXSigTimer m_timerJackpot;	// for Jackpot
	DWORD m_dwListenerHandle;
};


extern CListener theListener;

#endif //_LISTENER_H__