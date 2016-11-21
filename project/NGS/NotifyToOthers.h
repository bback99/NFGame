#pragma once
#include <LXSvr.h>
#include "common.h"

#include <NF/ADL/MsgCHSNGS.h>
#include <NF/ADL/MsgNCSNGS.h>
#include <ADL/MsgAMSGLS.h>
#include <ADL/MsgMDS.h>
#include <NF/ADL/MsgNLSCli.h>
#include <NF/ADL/MsgNASCli.h>
#include "ADL/msgLCSMSG2.h"

class CNotifyToOthers
{
private:
	LRBAddress chsAddr;
	DWORD dwRID;
	RoomID curRoomID;
public:
	CNotifyToOthers( void );
	void SetRoomData(  const DWORD dwRIndex, const RoomID & rID );
	void SetCHSAddr( const LRBAddress & AddrChs )
	{
		chsAddr = AddrChs;
	}

	void NotifyCreateRoomToChs(CUser* pUser, const NFRoomOption & nfRoomOption, const xstring & GameOption);
	void NotifyAddUserToChs(CUser* pUser, LONG lCSN);
	void NotifyRemoveUserToChs(long lCSN);
	void NotifyRemoveRoomToChs();
	void NotifyChangeRoomOptionToChs(const NFRoomOption & nfRoomOption);
	void NotifyChangeGameOptionToChs(const string & rOption);
	void NotifyChangeRoomStateToChs(LONG lState);
	void NotifyChangeUserInfoToChs(ChangeUserGameDataList& lstUserGameData);
	void NotifyChangeLoginStateToChs(LONG lCSN, LONG lLoginState);
	void NotifyNGSInfoToNCS(BOOL bAlways = FALSE );

	void SendToCHS(const LRBAddress& dest, PayloadNGSCHS& pld);
	void SendToNCS(const LRBAddress& dest, PayloadNGSNCS& pld);
	void SendToAMS(const LRBAddress& dest, PayloadGLSAMS& pld);
	void SendToNLS(const LRBAddress& dest, PayloadCLINLS& pld);
	void SendToPLS(const LRBAddress& dest, GBuf& buf);
	void SendToIBB(const LRBAddress& dest, GBuf& buf);
	void SendToAllCHS(const LRBAddress& dest, GBuf& buf);
	void SendToNAS(const LRBAddress& dest, PayloadCLINAS& pld);
};
