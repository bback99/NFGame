#pragma once 

#include <GXLinkUtl.h>
#include <GXSigUtl.h>
#include "Common.h"


class CRoomEventQueue;



class CUserLinkManager : public XLinkAdlManagerT<CLink, PayloadCliNGS,1024*1024 >
{
	IMPLEMENT_TSAFE( CUserLinkManager )
public:
	

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	CUserLinkManager( CRoomEventQueue * pQueue );
	virtual ~CUserLinkManager();

public:

	void clear();

	BOOL SendToUser( CLink * pLink , const PayloadNGSCli & pld );

	BOOL SendToAllUser( const PayloadNGSCli & pld );
	BOOL SendToAllUserExceptOne( CLink * pExcept, const PayloadNGSCli & pld );

	BOOL AddUserLink( CLink * pLink );
	BOOL RemoveUserLink( CLink * pLink );
	
	void OnUserDestroyed( long lCSN );

	BOOL Start( HTHREADPOOL hPool );
	void Stop();


	
	

protected:
	virtual BOOL OnRcvMsg(CLink* pLink, PayloadCliNGS& pld);

	typedef XLinkAdlManagerT<CLink, PayloadCliNGS,1024*1024 > TBase;
	STDMETHOD_(void, OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

private:
	typedef XLinkAdlManagerT<CLink, PayloadCliNGS,1024*1024 > TBase;


	BOOL PostLinkCutEvt( CLink * pLink );
	virtual BOOL OnError(CLink* pSocket, long lEvent, int nErrorCode);

	CRoomEventQueue * EventQueue;

	DWORD m_dwRefCnt;
};