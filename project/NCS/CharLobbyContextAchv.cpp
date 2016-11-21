#include "stdafx.h"
#include "User.h"
#include "CharLobbyManager.h"
#include "CharLobby.h"
#include "CharLobbyContext.h"
#include "Category.h"
#include <ACHV/AchvEvent.h>


void CCharLobbyContext::AchvReportCallback(LONG GSN, LONG CSN, int achv_ID, const achv::EventItem_T *pEvtItem)
{
	theLog.Put(ERR_UK, "NGS_Logic, AchvReportCallback(), CSN :", CSN, ", RoomID :", pEvtItem->roomID.m_dwGRIID, ", Achv_ID :", achv_ID);

	// ���� ���� Ŭ���̾�Ʈ�� �˸�
	MsgNCSCli_NotifyCompleteAchvInfo		ntf;
	ntf.m_lCSN = CSN;

	CCharLobby* pLobby = theCharLobbyManager.FindCharLobby(CSN);
	if (NULL == pLobby)
	{
		theLog.Put(ERR, "AchvReportCallback(), Not Found CharLobby GSN : ", GSN, ", CSN :", CSN);
		return;
	}

	int nErrorCode = 0;

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
	PayloadNCSCli pld(PayloadNCSCli::msgNotifyCompleteAchvInfo_Tag, ntf);
	pLobby->GetUserManager()->SendToUser(CSN, pld);
}	