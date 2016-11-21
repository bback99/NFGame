
#pragma once 

#include <GXLinkUtl.h>
#include <GXSigUtl.h>
#include "Link.h"
#include "UserManager.h"

#include <NF/ADL/MsgNCSCli.h>

class CUserLinkManager : public XLinkAdlManagerT<CLink, PayloadCliNCS, 1024*1024>
{
	IMPLEMENT_TSAFE( CUserLinkManager )

public:
	CUserLinkManager(CUserManager* pUserManager);
	virtual ~CUserLinkManager();

public:
	typedef XLinkAdlManagerT<CLink, PayloadCliNCS, 1024*1024> TBase;
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

public:
	void clear();
	BOOL SendToUser( CLink * pLink , const PayloadNCSCli & pld );
	BOOL SendToAllUser( const PayloadNCSCli & pld );
	BOOL SendToAllUserExceptOne( CLink * pExcept, const PayloadNCSCli & pld );
	BOOL AddUserLink( CLink * pLink );
	BOOL RemoveUserLink( CLink * pLink, BOOL bIsLinkOnly=FALSE );
	BOOL Start( HTHREADPOOL hPool );
	void Stop();

public:
	STDMETHOD_(void, OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnError(CLink* pSocket, long lEvent, int nErrorCode);
	virtual BOOL OnRcvMsg(CLink* pLink, PayloadCliNCS& pld);

	DWORD m_dwRefCnt;

	CUserManager* m_pUserManager;
};
