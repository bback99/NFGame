//
// GRCContainerTimer.cpp
//

#include "stdafx.h"
#include "Room.h"
#include "RoomInternalLogic.h"
#include "UserTable.h"
#include "RoomTable.h"
#include "StatisticsTable.h"

typedef list<LONG> usnList;



void CRoomInternalLogic::OnCheckUserAliveTimer()
{
	CAutoLockCheck lc("CRoomInternalLogic::OnCheckUserAliveTimer()", &m_lock_this_debug);


	usnList  lstAliveCheckTimeOutCSN, lstNeedCheckCSN;
	
	lstAliveCheckTimeOutCSN.clear();
	lstNeedCheckCSN.clear();


	LONG lUserCount = UserManager->ProcessAliveCheckTimer( lstAliveCheckTimeOutCSN, lstNeedCheckCSN );

	PayloadNGSCli pld( PayloadNGSCli::msgIsAlive_Tag );

	ForEachElmt( usnList, lstNeedCheckCSN, i, j )
	{
		UserManager->SendToUser( *i, pld );
	}

	ForEachElmt(usnList, lstAliveCheckTimeOutCSN, i2, j2) 
	{
		LONG lCSN = *i2;
		CUser* pUser = FindUser(lCSN);
		if (!pUser) continue;
		if (pUser->GetState() != US_CHECK) continue;
		CLink* pLink = pUser->GetLink();
		if (!pLink) 
		{			
			// ��� �������� ���� ����
			if (pUser->IsValid()) 
			{
				pUser->SetValid(FALSE);
				CAutoLockCheck lc("CRoomInternalLogic::OnCheckUserAliveTimer()", &m_lock_cu_debug);
				UserStatInfo info;
				info.Copy(pUser->GetNFUser().GetNFChar().m_nfUserBaseInfo);
				theLog.Put(INF_UK, "NGS_STAT_DEBUG, SetStatInfoCaller: OnCheckUserAliveTimer, CSN: ", info.m_lUSN, ", RoomID:", GetRoomIDStr());
				theStatTable.SetStatInfo(m_RoomID, info, -1, 6);

				if (info.m_lIsPCRoomUser != 0)
				{
					theStatTable.SetStatInfoPC(m_RoomID, info, -1, 6);
				}
			}
			theUserTable.Remove();
// @@ 
//			UserManager->DestroyUser( lCSN );
			lUserCount--;
			continue;
		}

// @@ ���߿� �ּ� Ǯ��� ��!!!!!! - 2009/10/27   ������Ʈ �ÿ� ������ �ּ� ó��!!!!
// 		KickOutUser( lCSN );

	}
	if (lUserCount <= 0 ) {
		if (GetState() == ROOMSTATE_DEAD) {
			theRoomTable.RemoveRoom(m_RoomID);
		}
	}
}

void CRoomInternalLogic::ResetRcvMsgCnt()
{
	UserManager->ResetMsgRcvCnt();
}
