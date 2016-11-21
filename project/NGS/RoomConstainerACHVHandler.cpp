
#include "stdafx.h"
#include <NFVariant/NFDBManager.h>
#include "Room.h"
#include "RoomTable.h"
#include "RoomInternalLogic.h"

void CRoom::AchvReportCallback(LONG GSN, LONG CSN, int achv_ID, const achv::EventItem_T *pEvtItem)
{
	theLog.Put(ERR_UK, "NGS_Logic, AchvReportCallback(), CSN :", CSN, ", RoomID :", pEvtItem->roomID.m_dwGRIID, ", Achv_ID :", achv_ID);

	// ���� ���� Ŭ���̾�Ʈ�� �˸�
	MsgNGSCli_NotifyCompleteAchvInfo		ntf;
	ntf.m_lCSN = CSN;

	CRoomPtr spRoom;
	BOOL bRet = theRoomTable.FindRoom(pEvtItem->roomID, &spRoom);
	if (!bRet)
	{
		theLog.Put(ERR_UK, "NGS_Logic, AchvReportCallback(), Not Found RoomID : ", pEvtItem->roomID.m_dwGRIID, ", CSN :", CSN);
		return;
	}
	
	LONG lGSN = spRoom->GetAchvUserInfo(CSN, ntf.m_strNickName);
	if (lGSN <= 0)
	{
		theLog.Put(ERR_UK, "NGS_Logic, AchvReportCallback(), Not Found User CSN :", CSN);
		return;
	}

	int nErrorCode = NF::EC_ACHV_SUCCESS;

	Achievement	completeACHV;
	completeACHV.Clear();
	completeACHV.m_lAchvID = achv_ID;
	completeACHV.m_dGauge = pEvtItem->val;

	if (achv::PR_COMPLETED == pEvtItem->result) {
        ntf.m_lErrorCode = nErrorCode;

		SYSTEMTIME systime;
		::GetLocalTime(&systime);
		completeACHV.m_strCompleteDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	}
	else
		ntf.m_lErrorCode = NF::EC_ACHV_NOTI_STATUS;

	ntf.m_listGetLately.push_back(completeACHV);

	// ������ ���� ����.. ���⼭�� �˸��⸸ �ϸ� ��	(�˸��°͵� ������ ���� �˸���...- ���߿� �߰�)	
	PayloadNGSCli pld(PayloadNGSCli::msgNotifyCompleteAchvInfo_Tag, ntf);
	spRoom->SendToUser(CSN, pld);
}

void CRoomInternalLogic::achv_ProcessLanding(CUser* pUser)
{
	CNFChar& nfChar = pUser->GetNFUser();
	CFish& biteFish = nfChar.GetBiteFish();

	TMapAchvFactor	mapFactorVal;
	mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_FISH_TYPE], biteFish.GetFish().m_lFishType));		// fishType -> 0: �Ϲ�, 1:�빰, 2:���, 3:����
	mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_FISH_GROUP_ID], biteFish.GetFish().m_lLockedNoteFishID));
	mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_SIGN], this->GetSignType()));
	mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_FISH_LENGTH], (LONG)biteFish.GetFishResultLength()));

	g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_LANDING, GetRoomID(), mapFactorVal);

	//achv_ProcessLanding_fishattack();
}
