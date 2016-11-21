
#include "stdafx.h"
#include "common.h"
#include "Room.h"
#include "RoomTable.h"
#include "RoomInternalLogic.h"
#include "StatisticsTable.h"
#include "UserTable.h"
#include "Agent.h"
#include <NF/ADL/MsgNFCommonStruct.h>
#include "ErrorLog.h"
#include "FishAI.h"
#include <NFVariant/NFDBManager.h>
#include <NFVariant/NFGameData.h>
#include "NotifyToOthers.h"
#include <NFVariant/NFMenu.h>
#include <NFVariant/NFItem.h>
#include <NLSManager.h>


/////////////////////////////////////////////////////////////////////////////////////
void CRoomInternalLogic::OnReqTutorial(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqTutorial* req = pMsg->un.m_msgReqTutorial;
	MsgNGSCli_AnsTutorial		ans;
	ans.Clear();
	ans.m_lTutorialType = req->m_lTutorialType;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		// Ŭ�󿡼� Ʃ�丮�� �Ϸ� �Ǿ��ٴ� ��Ŷ ���� ��¥ Ȯ���ؼ� ���� ���� ���� �Ǵ�
		const std::string strDate = pUser->GetTutorialDate(req->m_lTutorialType);
		if (strDate == G_INVALID_DATE)
		{
			//��¥ ������Ʈ
			SYSTEMTIME systime;
			::GetLocalTime(&systime);
			string strDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

			BOOL bRet = FALSE;
			if (TUTORIAL_TYPE_1 == req->m_lTutorialType)
				bRet = theNFDBMgr.UpdateTutorialDate(pUser->GetGSN(), lCSN, strDate);
			else if (TUTORIAL_TYPE_2 == req->m_lTutorialType)
				bRet = theNFDBMgr.UpdateStudyScenarioDate(pUser->GetGSN(), lCSN, strDate);
			else
				ans.m_lErrorCode = EC_TT_TYPE;

			if (bRet)
			{
				pUser->SetTutorialDate(req->m_lTutorialType, strDate);
				ans.m_strTutorialDate = strDate;

				// ����
				TMapAchvFactor mapFactorVal;
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_TUTORIAL], req->m_lTutorialType));
				g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_TUTORIAL, GetRoomID(), mapFactorVal);

				mapFactorVal.clear();
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_LEVEL], pUser->GetLevel()));
				g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_LEVEL, GetRoomID(), mapFactorVal);
			}
			else
				ans.m_lErrorCode = EC_TT_DB_FAILED;
		}
		else
			ans.m_lErrorCode = EC_TT_ALREADY_COMPLETE;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsTutorial_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqNFCharInfo(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFCharInfo* req = pMsg->un.m_msgReqNFCharInfo;
	MsgNGSCli_AnsNFMyCharInfo		myAns;
	MsgNGSCli_AnsNFOtherCharInfo	myOhter;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (req->m_lType == 0)			// My
		{
			myAns.m_lMsgType = req->m_lMsgType;
			myAns.m_nfCharInfoExt.BCopy(*pUser->GetNFCharInfoExt());
			myAns.m_lUserSlot = pUser->GetUserSlot();
			myAns.m_lUserStatus = pUser->GetUserStatus();
			myAns.m_lIdxFishingPoint = pUser->GetPrevFishingPoint();
		}
		else if (req->m_lType == 1)		// other
		{
			// FindUser by CSN;
			CUser* pFindUser = UserManager->FindUser(req->m_lCSN);
			if (pFindUser)
			{
				const NFCharInfoExt nfCharInfoExt = pFindUser->GetNFUser().GetNFChar().m_nfCharInfoExt;
				myOhter.m_nfCharBaseInfo = nfCharInfoExt.m_nfCharBaseInfo;
				myOhter.m_nfCharExteriorInfo = nfCharInfoExt.m_nfCharExteriorInfo;
				myOhter.m_mapUsingItem = nfCharInfoExt.m_nfCharInven.m_mapUsingItem;
				myOhter.m_lMsgType = req->m_lMsgType;
			}
			//		else
			//			ans.m_ansExchangeCards.m_lErrorCode = ;
		}
		//		else
		//			ans.m_ansExchangeCards.m_lErrorCode = ;
	}
	//	else
	//		ans.m_ansExchangeCards.m_lErrorCode = ;

	if (req->m_lType == 0)
	{
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFMyCharInfo_Tag, myAns);
		UserManager->SendToUser(lCSN, pld);
	}
	else if (req->m_lType == 1)
	{
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFOtherCharInfo_Tag, myOhter);
		UserManager->SendToUser(lCSN, pld);
	}
}

void CRoomInternalLogic::OnRcvBanishReq(LONG lCSN, PayloadCliNGS* pPld)
{
	MsgNGSCli_BanishNtf	ntf;
	ntf.m_lBanishCSN = pPld->un.m_msgBanishReq->m_lBanishCSN;
	ntf.m_lErrorCode = 0;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		// ��ȿ� �ִ� �����鿡�� CSN�� �����ض�...
		if (m_nfRoomOption.m_lCapUSN != pUser->GetCSN())		// ������ �ƴѵ� Banish�� ��û�ߴ�. -_-;; ��ŷ�ѳ� ©�����
		{
			UserManager->CutUser(ntf.m_lBanishCSN);
			return;
		}

		CUser* pFindUser = UserManager->FindUser(ntf.m_lBanishCSN);
		if (!pFindUser)
			// Banish�ϱ����� ���� �������Ƿ�, ���忡�� �ش������� �����ٰ� �˷���� �Ѵ�.
			ntf.m_lErrorCode = -1;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgBanishNtf_Tag, ntf);
	UserManager->SendToAllUser(pld);
}

void CRoomInternalLogic::OnReqBiteFishInfo(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqBiteFishInfo* req = pMsg->un.m_msgReqBiteFishInfo;
	MsgNGSCli_AnsBiteFishInfo ans;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		// FindUser by CSN;
		CUser* pFindUser = UserManager->FindUser(req->m_lCSN);
		if (pFindUser)
		{
			CNFChar nfChar = pFindUser->GetNFUser();
			ans.m_lCSN = pFindUser->GetCSN();

			ans.m_biteFishInfo.Clear();			CFish& biteFish = nfChar.GetBiteFish();
			biteFish.GetBiteFishInfo(ans.m_biteFishInfo);
		}
//		else
//			ans.m_ansExchangeCards.m_lErrorCode = ;
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode = ;

	PayloadNGSCli pld(PayloadNGSCli::msgAnsBiteFishInfo_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqChangeUserInfo(LONG lCSN, PayloadCliNGS* pMsg)
{
	DWORD dwStart = GetTickCount();
	MsgCliNGS_ReqChangeUserInfo* req = pMsg->un.m_msgReqChangeUserInfo;

	MsgNGSCli_NotifyChangeUserInfo	ans;
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	ans.m_lInfoType = req->m_lInfoType;
	ans.m_lUserStatus = UIS_INVALID;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		ans.m_lCSN = pUser->GetCSN();

		if (1 == req->m_lInfoType)		// UserSlot
		{
			ans.m_lErrorCode = ChangeUserSlot(pUser, req->m_lMoveUserSlot);
			if (-1 == ans.m_lErrorCode)
			{
				theLog.Put(WAR_UK, "NGS_LOGIC, OnReqChangeUserInfo(). ChangeUserInfo UserSlot Failed!!! CSN : ", pUser->GetCSN());
			}
			else
			{
				ans.m_lMoveUserSlot = req->m_lMoveUserSlot;
				PayloadNGSCli pld(PayloadNGSCli::msgNotifyChangeUserInfo_Tag, ans);
				UserManager->SendToAllUser(pld);
				theLog.Put(WAR_UK, "NGS_LOGIC, OnReqChangeUserInfo(), --------------------------------------", GetTickCount() - dwStart);
				return;
			}
		}
		else if (2 == req->m_lInfoType)		// Status
		{
			if (pUser->GetUserStatus() == req->m_lUserStatus) {
				theLog.Put(WAR_UK, "NGS_LOGIC, OnReqChangeUserInfo(). ChangeUserInfo UserStatus SameStatus!!! CSN : ", pUser->GetCSN(), ", Status :", req->m_lUserStatus);
			}

			pUser->SetUserStatus(req->m_lUserStatus);
			ans.m_lUserStatus = pUser->GetUserStatus();
			PayloadNGSCli pld(PayloadNGSCli::msgNotifyChangeUserInfo_Tag, ans);
			UserManager->SendToAllUser(pld);
			theLog.Put(WAR_UK, "NGS_LOGIC, OnReqChangeUserInfo(), --------------------------------------", GetTickCount() - dwStart);
			return;
		}
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode = ;

	PayloadNGSCli pld(PayloadNGSCli::msgNotifyChangeUserInfo_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// lErrorCode : 1, ����
// lErrorCode : -1, ������ ���� FishingPoint
// lErrorCode : -2, FishingPoint�� MAX_USER �� ����!!
void CRoomInternalLogic::OnReqMoveFishingPoint(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqMoveFishingPoint* req = pMsg->un.m_msgReqMoveFishingPoint;
	MsgNGSCli_AnsMoveFishingPoint	ans;
	ans.Clear();

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		LONG lPrevFPIndex = pUser->GetPrevFishingPoint();

		if (G_TUTORIAL_I == req->m_lTutorialType || G_TUTORIAL_II == req->m_lTutorialType)
		{
			ans.m_lBoatOwnerCSN = 0;
			ans.m_lMovedMultiPortIndex = 0;
			ans.m_lErrorCode = ChangeFishingPoint_Tutorial(req->m_lFPIndex, pUser, ans.m_lMovedFPIndex);

			// ��û�� ������ roomOption ������ ����
			ans.m_lMovedFPIndex = ans.m_lMovedFPIndex;
			ans.m_lStatus = pUser->GetFPStatus();
			ans.m_lTutorialType = req->m_lTutorialType;
		}
		else
		{
			// ��û�� ����Ʈ�� Valid üũ
			FishingPoint* pFP = theNFDataItemMgr.GetFishingPointByIndex(m_nfRoomOption.m_lIdxFishMap, req->m_lFPIndex);
			if (pFP)
			{
				ans.m_lBoatOwnerCSN = 0;
				ans.m_lMovedMultiPortIndex = 0;
				ans.m_lErrorCode = ChangeFishingPoint(pFP, req->m_lFPIndex, pUser, ans.m_lBoatOwnerCSN, ans.m_lMovedMultiPortIndex);

				// ��û�� ������ roomOption ������ ����
				ans.m_lMovedFPIndex = req->m_lFPIndex;
				ans.m_lStatus = pUser->GetFPStatus();
				ans.m_lTutorialType = 0;

				// ���, ��Ʈ(�ؼ�, ���) üũ�Ͽ� 
				if (NF::G_NF_ERR_SUCCESS == ans.m_lErrorCode)
				{
					ans.m_lErrorCode = theNFItem.CalcEquipedItem_FP(pUser->GetNFCharInfoExt(), pFP->m_bIsSalt, ans.m_bAbilityChanged);
					if (NF::G_NF_ERR_SUCCESS == ans.m_lErrorCode && TRUE == req->m_bIsBoatOwnerLeave)
					{
						ans.m_lErrorCode = G_NF_ERR_MOVE_FP_FORCE; // ��Ʈ ������ ������ ������ �ǽ�����Ʈ �̵�
					}
				}

				ans.m_nfAbility = pUser->GetNFCharInfoExt()->m_nfAbility;
			}
			else
				 theLog.Put(ERR_UK, "NGS_LOGIC, OnReqMoveFishingPoint Not Found FishingPoint CSN:", pUser->GetCSN(), "/FP_IDX:", req->m_lFPIndex);
		}

		theLog.Put(DEV_UK, "NGS_LOGIC, OnReqMoveFishingPoint. CSN:", pUser->GetCSN(), "/FP_IDX:", req->m_lFPIndex, "/ErrorCode:", ans.m_lErrorCode);

		// �̵��� ���� ���� ��쿡�� �ٸ� �������� �����Ѵ�.
		if (NF::G_NF_ERR_SUCCESS == ans.m_lErrorCode 
		|| G_NF_ERR_MOVE_FP_FORCE == ans.m_lErrorCode) // ������ �̵��� ��쵵 ����
		{
			MsgNGSCli_NotifyMoveFishingPoint	ntf;
			ntf.m_lMovedCSN = pUser->GetCSN();
			ntf.m_lPrevFPIndex = lPrevFPIndex;
			ntf.m_lMovedFPIndex = req->m_lFPIndex;
			ntf.m_mapTotalMap = GetTotalMapInfo();
			PayloadNGSCli pld2(PayloadNGSCli::msgNotifyMoveFishingPoint_Tag, ntf);
			UserManager->SendToAllUser(pld2);
		}
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode = ;

	PayloadNGSCli pld(PayloadNGSCli::msgAnsMoveFishingPoint_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqTotalMapInfo(LONG lCSN, PayloadCliNGS* pMsg)
{
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	string strDate = ::format("%04d-%02d-%02d %02d:%02d:%02d ", 
		systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	theLog.Put(WAR_UK, "NGS_LOGIC, OnReqTotalMapInfo(). Time : ", strDate.c_str());

	MsgNGSCli_AnsTotalMapInfo	ans;
	ans.m_mapTotalMap = GetTotalMapInfo();

// 	// Debugging ��
// 	ForEachElmt(TMapTotalMapInfo, ans.m_mapTotalMap, it, ij)
// 	{
// 		FPInfo nfFPInfo = (*it).second;
// 		theLog.Put(WAR_UK, "NGS_LOGIC, OnReqTotalMapInfo(). FP_IDX:", (*it).first, "/FP_TYPE:", nfFPInfo.m_lFPType, "/BOAT_OWN:", nfFPInfo.m_lBoatOwnerCSN);
// 		
// 		ForEachElmt(TlstFPUserList, nfFPInfo.m_lstFPUserList, i, j)
// 			theLog.Put(WAR_UK, "NGS_LOGIC, OnReqTotalMapInfo(). CSN:", (*i).m_lCSN, "/STATUS:", (*i).m_lStatus);
// 	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsTotalMapInfo_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnNFGameData(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_NFGameData* req = pMsg->un.m_msgNFGameData;
	MsgNGSCli_NFGameData	ans;
	ans.m_strGameData = req->m_strGameData;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		ans.m_lSendCSN = pUser->GetCSN();

		PayloadNGSCli	pld(PayloadNGSCli::msgNFGameData_Tag, ans);

		if (req->m_lRecvCSN == -1) {		// SendToAll except pUser
			UserManager->SendToAllUsersExceptOne(lCSN, pld);
			return;
		}
		else
		{
			CUser* pSendUser = UserManager->FindUser(req->m_lRecvCSN);
			if (!pSendUser) return;
			UserManager->SendToUser(pSendUser->GetCSN(), pld);
			return;
		}
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode = ;

	PayloadNGSCli	pld(PayloadNGSCli::msgNFGameData_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// Game Start 
void CRoomInternalLogic::OnReqStartGame(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqStartGame* req = pMsg->un.m_msgReqStartGame;
	MsgNGSCli_NotifyStartGame		ans;
	ans.m_lStartGameType = req->m_lStartGameType;
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
	//	if (ans.m_lStartGameType == 3)
	//	{
			// ����°� WAIT�̾�߸� ���� ����!!!
			// �� ���� �޼��� ������ -1
			if (ROOMSTATE_START != GetRunStopFlag())
			{
				// �޼��� ���� ����� ������ �´��� üũ
				if (pUser->GetCSN() == m_nfRoomOption.m_lCapUSN)
				{
					// ������ ���, ���� ���� ������ ���ο��� ���������� üũ �Ѵ�....
					// �����ý���(090630)_������.doc page-5
					// ����(�������� ����) ������ ������ �ʿ� �ο��� ���� ������ �ִ� �ο��� 50%�̻��� �濡 �־�� �Ѵ�. 
					// ��, 12������ ������, ������ ������ ��쿡�� 6�� �̻��� �����ϸ� ������ ���� �� �� �ְ� �ȴ�.
					// ���� ���� ������ �˸��� ������ ������ ���ۡ� �������� Ȱ��ȭ, �� Ȱ��ȭ�� �˷��ֵ��� �ϸ� 
					// ��-��, Ʃ�丮�� ������� �����ϵ��� �Ѵ�. 
					// ������ �ƴ� �Ϲ� ������ ȥ�ڼ��� ������ ������ �� �ִ�
					//			LONG lCurSize = (LONG)mUsers.size();
					//			if ( m_nfRoomOption.m_lPlayType == 1 || m_nfRoomOption.m_lPlayType == 2 || 
					//				(m_nfRoomOption.m_lPlayType == 3 && lCurSize >= m_nfRoomOption.m_lMaxUserCnt/2) )
					
						if (FALSE == UserManager->CheckUserGameStartStatus())
							ans.m_lErrorCode = -10;

						if (ans.m_lErrorCode == 1)
						{
							UserManager->SetUserGameStartStatus(ULS_INROOM, m_nfRoomOption.m_lIdxFishMap);

							SetRunStopFlag(ROOMSTATE_READY);

							// ���� ���� Ÿ�̸� ����
							if (m_nfRoomOption.m_lLimitTimeType <= 0)	// �⺻ �ʷ� �����Ѵ�. (5��)
								m_nfRoomOption.m_lLimitTimeType = 1;

							m_nfRoomOption.m_lRemainTime = m_nfRoomOption.m_lLimitTimeType * 60;		// ���ѽð� * 60�ʷ� ����!!!
							ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

							// TotalMap �ʱ�ȭ
							InitTotalMap(m_nfRoomOption.m_lIdxFishMap);

							// ¡�� ����Ʈ �ʱ�ȭ
							if (!SetSignPoint(m_nfRoomOption.m_lIdxFishMap))
								ans.m_lErrorCode = -5;			// sign Point error

							NotifyUtil->NotifyChangeRoomStateToChs(ROOMSTATE_READY);
							SendNtfMapLoding();
							return;
						}
					//			else 
					//			{
					//				theLog.Put(WAR_UK, "NGS_LOGIC, OnReqStartGame(). Not Match pUser->CSN() ", pUser->GetCSN(), " != CapCSN ", m_nfRoomOption.m_lCapUSN);
					//				ans.m_ansExchangeCards.m_lErrorCode = -6;		// Current User Error
					//			}
				}
				else {
					theLog.Put(WAR_UK, "NGS_LOGIC, OnReqStartGame(). Not Match pUser->CSN() ", pUser->GetCSN(), " != CapCSN ", m_nfRoomOption.m_lCapUSN);
					ans.m_lErrorCode = -2;
				}
			}
			else {
				theLog.Put(WAR_UK, "NGS_LOGIC, OnReqStartGame(). Already StartGame!!! RoomID : ", RoomID2Str(GetRoomID()));
				ans.m_lErrorCode = -3;		
			}
	//	}
	//  else
	//  {
	//		// m_lStartGameType�� 1, 2�� ��쿡�� ��ȿ� �ִ� ��� ���������� �˷���� �Ѵ�...
	//		PayloadNGSCli pld(PayloadNGSCli::msgNotifyStartGame_Tag, ans);
	//		UserManager->SendToAllUser(pUser, pld);
	//		return;
	//  }
	}
	//	else
	//		ans.m_ansExchangeCards.m_lErrorCode = ;

	PayloadNGSCli pld(PayloadNGSCli::msgNotifyStartGame_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// ��ȸ���� �濡 ������ ������ In-Game���� ���� �Ϸ��� �� �� ������ ��Ŷ
// Ŭ���̾�Ʈ�� Progress��Ŷ�� ������ �ʰ�, �ʷε��� �� ������ ReqJoinInGame ��Ŷ�� ������.
void CRoomInternalLogic::OnReqJoinInGame(LONG lCSN, PayloadCliNGS* pMsg)
{
// 	MsgNGSCli_NotifyJoinInGame		ntf;
// 	ntf.m_lCSN = pUser->GetCSN();
// 
// 	if (ROOMSTATE_START == GetRunStopFlag())
// 	{
// 		// ���� ������ ���¸� ���� (Playing����)
// 		pUser->SetUserLocation(ULS_INROOM);
// 		ntf.m_lErrorCode = 1;
// 	}
// 	else
// 		ntf.m_lErrorCode = -1;
// 
// 	// �����ϸ� ��ȿ� �ִ� �����鿡�� ntf�� 
// 	PayloadNGSCli pld(PayloadNGSCli::msgNotifyJoinInGame_Tag, ntf);
// 	UserManager->SendToAllUser(pld);
}

void CRoomInternalLogic::OnNtfGameReadyProgress(LONG lCSN, PayloadCliNGS* pMsg)
{
	if (ROOMSTATE_START == GetRunStopFlag())
		return;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		// ���� ���� ����
		CNFChar& nfChar = pUser->GetNFUser();
		nfChar.SetMapLodingProgress(pMsg->un.m_msgNtfGameReadyProgress->m_MapLoadingProgress);

		if (!SendNtfGameStart())
			SendNtfGameReadyProgress(pUser);
	}
}

void CRoomInternalLogic::OnReqGameResult(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgNGSCli_AnsGameResult		ans;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!GetGameResult(pUser, ans.m_lstPlayer, ans.m_lstTeam))
			theLog.Put(WAR_UK, "NGS_LOGIC, OnReqGameResult(). OnReqGameResult Failed. RoomID : ", RoomID2Str(GetRoomID()), ", CSN :", pUser->GetCSN());
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode = ;

	PayloadNGSCli pld(PayloadNGSCli::msgAnsGameResult_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// for NFGame Message
// for Add NFSkill 
void CRoomInternalLogic::OnReqCasting(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqCasting* req = pMsg->un.m_msgReqCasting;
	MsgNGSCli_AnsCastingResult		ans;
	ans.m_lErrorCode = NF::G_NF_ERR_FAIL;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		CNFChar& nfChar					= pUser->GetNFUser();
		NFCharInfoExt& nfCharInfoExt	= nfChar.GetNFChar().m_nfCharInfoExt;
		NFAbilityExt& abilityExt		= nfChar.GetAbilityExt();
		ans.m_lstCastingSkill = req->m_lstCastingSkill;
		ans.m_lCSN = nfChar.GetCSN();

		BiteFishInfo		biteFishInfo;
		biteFishInfo.Clear();

		if (nfChar.GetDebuggingMode() == EN_DM_CASTING)
			nfChar.GetDebuggingCasting(pMsg);
		nfChar.InitNFCharInfo();

		if (!CheckValidItem(nfCharInfoExt, eItemType_Line))
		{
			ans.m_lErrorCode = G_NF_ERR_INVALID_LINE_ITEM;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsCastingResult_Tag, ans);
			UserManager->SendToUser(pUser->GetCSN(), pld);
		}

		if (!CheckValidItem(nfCharInfoExt, eItemType_Lure))
		{
			ans.m_lErrorCode = G_NF_ERR_INVALID_LURE_ITEM;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsCastingResult_Tag, ans);
			UserManager->SendToUser(pUser->GetCSN(), pld);
		}

		//////////////////////////////////////////////////////////////////////////
		// NFUser(�⺻ �ɷ�ġ + EquipItem + ClothItem) + NFUserItem(UsableItem + SkillItem)
		NFAbilityExt	totAbility;
		totAbility.Clear();
		totAbility += nfCharInfoExt.m_nfAbility;
		totAbility += abilityExt;

		// ����� ĳ���ý�ų������ �ɷ�ġ ++
		// 2010-05-06
		// ����� ĳ���� ��ų�� ���� �ϴ� ������ ���� ����...
		// 1. ĳ���� �Ÿ� �� incountergauge�� ���� ������ ��ģ��.
		// 2. ����� ������� ���� ������� Ÿ�Կ� ������ ��ģ��.
		//   - � ����Ⱑ ������ ������ ������ ���ľ� �Ѵ�.
		// 3. �ٽ� ĳ������ �ϸ�, ������ ����� ĳ���� �ɷ�ġ�� ������� ���ο� ������ ����Ǿ�� �Ѵ�.
		// 4. ���� ���ᰡ �Ǿ� �ٽ� �����ϸ�, ������ ĳ���� ����Ѱ� �������� �ʴ´�.

		// ����� ĳ���� ��ų ����
		nfChar.SetUsedCastingSkill(req->m_lstCastingSkill);
		TLstSkill tempSkill = nfChar.GetUsedCastingSkill();

		ForEachElmt(TLstSkill, tempSkill, it, ij) 
		{
			SkillItem* pSkillItem = theNFDataItemMgr.GetSkillItemByIndex((*it).m_lItemCode);
			if (pSkillItem) {
				totAbility += (*pSkillItem).m_nfAbilityExt.m_nfAbility;
				break;
			}
			else
				theLog.Put(WAR_UK, "NGS_LOGIC, OnReqCasting() Not Found CastingSkill!!!. CSN: ", pUser->GetCSN(), ", SkillIndex : ", (*it).m_lItemCode);
		}

		// 1. ĳ���� ��Ÿ�
		double dCastingDist = CalcCastingDist(req->m_lCastingType, totAbility.m_nfAbility.m_dFlyDist);

		// 2. ĳ���� ������ ����(m)
		double dCastAccuracy = CalcAccuracy(req->m_lCastingType, totAbility.m_nfAbility.m_dFlyDist);

		// 4. ���� ���� ��ǥ���� ���� �������� �̿��Ͽ� ��ä���� ���� ��, ��ǥ���� ���� ������ ���� ���ο� �����ϴ� ��ǥ�� �����ϰ� ����Ѵ�.
		// Ŭ���̾�Ʈ���� ���� Ŭ���̾�Ʈ ��ġ, ����, Ŭ���� ��ġ�� ������
		Coordinates RealCastingLocation = CalcRealCastingLocation(req->m_lCastingType, dCastingDist, req->m_UserLocation, req->m_CastingLocation, req->m_lDragIntensity, dCastAccuracy);

		// 5. ������ �Ÿ��� �����κ����� �Ÿ� ���
		// ������ ���� �Ÿ� ��� (������ġ��ǥ, Ŭ��or�巡���� ��ġ��ǥ)
		double dHypote = CalcHypote(req->m_UserLocation, RealCastingLocation);

		// 6. ĳ���� ���ھ�(�Ҽ��� ����) - �巡��, Ŭ�� ���� ���� ���
		// ����������(���� ���� ĳ������ġ�� ������ ĳ������ ��ġ�� ����)
		double dCastingScore = CalcCastingScore(req->m_CastingLocation, RealCastingLocation, totAbility.m_nfAbility, dHypote, dCastingDist, dCastAccuracy);
		double dResultCastingScore = abilityExt.m_dCastingScore + dCastingScore;

		// 7. ĳ���� ���ھ�� ��ī��Ʈ ������ ���ϱ�
		int nAchv_ScoreType = CS_NOVALUE;
		long lAddGauge = GetInCounterScoreByCastingScore(dResultCastingScore, nAchv_ScoreType);

		// 8. Lucky Point
		CalcLuckyPoint(totAbility.m_nfAbility.m_dLuckyPoint);

		// 9. ���� ������ �̸� �����Ͽ� ĳ���� ����� ������. (2010.10.27 ��ȹ ���濡 ���� ���� - bback99)
		LONG lErrorCode = theFishAI.GetBiteFishInfo(pUser, biteFishInfo, m_nfRoomOption, GetSignType());

		// Distance, LineLength Init
		nfChar.SetDistanceFromFish(dHypote);
		nfChar.SetLineLength(dHypote);
		nfChar.SetUserLocation(req->m_UserLocation);

		// Casting ��û�� ���� ����
		ans.m_lErrorCode = lErrorCode;
		ans.m_biteFishInfo = biteFishInfo;
		ans.m_CastingLocation.m_dX = RealCastingLocation.m_dX;
		ans.m_CastingLocation.m_dY = RealCastingLocation.m_dY;
		ans.m_lRealRange = (long)dHypote;
		ans.m_lCastingScore = (long)ROUND(dResultCastingScore, 0);
		ans.m_lFishHP = nfChar.GetBiteFish().GetMAXFishHP();
		ans.m_lPRNGSeed = nfChar.GetCSN() + GetTickCount();

		nfChar.SetPRNGSeed(static_cast<uint32_t>(ans.m_lPRNGSeed));

		// ���� ���
		ApplyInCounterSystem(pUser, ICS_CASTING_SUCCESS, lAddGauge);

		// ����
		TMapAchvFactor mapFactorVal;
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_CASTING_SCORE], nAchv_ScoreType));
		g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_CASTING, GetRoomID(), mapFactorVal);

		theLog.Put(DEV_UK, "NGS_LOGIC, OnReqCasting(). RecvMsg CSN: ", pUser->GetCSN(), ", CastingType: ", req->m_lCastingType, ", RealCastingDist: ", req->m_lRealCastingDist, ", DragIntensity: ", req->m_lDragIntensity, ", UserLocation X: ", req->m_UserLocation.m_dX, ", Y: ", req->m_UserLocation.m_dY, ", CastingLocation X: ", req->m_CastingLocation.m_dX, ", Y:", req->m_CastingLocation.m_dY);
		theLog.Put(DEV_UK, "NGS_LOGIC, OnReqCasting(). AnsMsg Errcode :", ans.m_lErrorCode, ", CastingLocation X: ", ans.m_CastingLocation.m_dX, ", Y:", ans.m_CastingLocation.m_dY, ", RealRange: ", ans.m_lRealRange, ", CastingScore: ", ans.m_lCastingScore);
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode = ;

	PayloadNGSCli pld(PayloadNGSCli::msgAnsCastingResult_Tag, ans);
	UserManager->SendToAllUser(pld);
}

void CRoomInternalLogic::OnReqCastingResult(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqCastingResult* req = pMsg->un.m_msgReqCastingResult;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		CNFChar& nfChar = pUser->GetNFUser();

		if (nfChar.GetDebuggingMode() == EN_DM_CASTING_RESULT)
			nfChar.GetDebuggingCasting(pMsg);

		// ������ ������ ��ǥ�� ���� ĳ���� ����
		if (req->m_lErrorCode)
		{
			ApplyInCounterSystem(pUser, ICS_CASTING_FAIL, 0);
			theLog.Put(WAR_UK, "NGS_LOGIC, OnReqCastingResult(). CSN: ", pUser->GetCSN());
		}

		nfChar.InitNFCharInfo();
	}
}

// Cli->NGS Action Request
// Skill Action�� ���� InCounterGauge�� ����Ѵ�.
// for NFSkill
void CRoomInternalLogic::OnReqAction(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqAction* req = pMsg->un.m_msgReqAction;
	MsgNGSCli_AnsActionResult		ansAction;
	ansAction.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		CNFChar& nfChar					= pUser->GetNFUser();
		NFCharInfoExt& nfCharInfoExt	= nfChar.GetNFChar().m_nfCharInfoExt;
		NFAbilityExt abilityExt			= nfChar.GetAbilityExt();

		// IncountScore ����, Action ��ġ���� �����ؼ� ActionSkill ���� ���Ѵ�...
		abilityExt.m_lActionIncountScore += (LONG)sqrt(nfCharInfoExt.m_nfAbility.m_dCharm);

		if (nfChar.GetActionStatus() != STATUS_INIT)
			return;

		// ����� ������ ��������, Landing�� ���������� �ٽ� ������ �ʴ´�. 
		// Ŭ���̾�Ʈ�� ����� ������ ���� ���� ���� ��� Action�� �����Ƿ�...
		// Ȥ�� �߰��� ���� �Ǵ� ���� üũ �ؼ� FALSE�� �������ƾ� ��!!!!!
		if (nfChar.GetDebuggingMode() == EN_DM_ACTION)
			nfChar.GetDebuggingCasting(pMsg);

		// NFUser(�⺻ �ɷ�ġ + EquipItem + ClothItem) + NFUserItem(UsableItem + SkillItem)
		NFAbility	totAbility;
		totAbility.Clear();
		totAbility += nfCharInfoExt.m_nfAbility;
		totAbility += abilityExt.m_nfAbility;

		// �׼� ��ų ����
		if (req->m_lManualActionType != 0) {	// ���� �׼�
			SkillItem* pSkillItem = theNFDataItemMgr.GetSkillItemByIndex(req->m_lManualActionType);
			if (pSkillItem) {
				totAbility += (*pSkillItem).m_nfAbilityExt.m_nfAbility;
				abilityExt += (*pSkillItem).m_nfAbilityExt;
			}
			else
				theLog.Put(WAR_UK, "NGS_LOGIC, OnReqAction() Not Found Manual CastingSkill!!!. CSN: ", pUser->GetCSN(), ", SkillIndex : ", req->m_lManualActionType);
		}
		else 
		{		// ����� ĳ���ý�ų������ �ɷ�ġ ++
			ForEachElmt(TLstSkill, req->m_lstActionSkill, it, ij)
			{
				SkillItem* pSkillItem = theNFDataItemMgr.GetSkillItemByIndex((*it).m_lItemCode);
				if (pSkillItem) {
					totAbility += (*pSkillItem).m_nfAbilityExt.m_nfAbility;
					abilityExt += (*pSkillItem).m_nfAbilityExt;
					break;
				}
				else
					theLog.Put(WAR_UK, "NGS_LOGIC, OnReqAction() Not Found Skill CastingSkill!!!. CSN: ", pUser->GetCSN(), ", SkillIndex : ", (*it).m_lItemCode);
			}
		}

		long lActionSkillIncounterGauge = abilityExt.m_lActionIncountScore;

		// 1. ��ų�� ���� ��ī��Ʈ ���� 
		BOOL bIsFullInCounterGauge = ApplyInCounterSystem(pUser, ICS_CHARM, lActionSkillIncounterGauge);

		// InCounterGauge�� Full�̸�, ����⸦ �����Ͽ� Ŭ���̾�Ʈ�� �����ش�.
		if (bIsFullInCounterGauge)
		{
			BiteFishInfo		biteFishInfo;
			biteFishInfo.Clear();

			// Casting ��û�� ���� ����
			MsgNGSCli_SendBiteFishInfo		ansBiteFish;
			ansBiteFish.m_lErrorCode = 0;
			ansBiteFish.m_lCSN = nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN;

			//
			nfChar.GetBiteFish().GetBiteFishInfo(biteFishInfo);

			ansBiteFish.m_biteFishInfo = biteFishInfo;

				// ������ ������...
			ansBiteFish.m_biteFishInfo.m_bIsFishBite = 1;

			// ����⸦ ���� ������ ����
			if (ansBiteFish.m_biteFishInfo.m_bIsFishBite) {
				ApplyInCounterSystem(pUser, ICS_BITE_SUCCESS, lActionSkillIncounterGauge);
				nfChar.SetActionStatus(STATUS_CHARM);
			}
			else
				ApplyInCounterSystem(pUser, ICS_BITE_FAIL, lActionSkillIncounterGauge);
				

			PayloadNGSCli pld(PayloadNGSCli::msgSendBiteFishInfo_Tag, ansBiteFish);
			UserManager->SendToUser(lCSN, pld);

			theLog.Put(DEV_UK, "NGS_LOGIC, OnReqAction(). BiteFishInfo CSN: ", pUser->GetCSN(), ", FishIndex: ", biteFishInfo.m_lFishIndex, ", Size: ", biteFishInfo.m_dFishLength, ", Weight: ", biteFishInfo.m_dFishWeight, ", FeedingTime: ", biteFishInfo.m_dFishFeedingTime, ", BiteResult: ", biteFishInfo.m_bIsFishBite);
			return;
		}
		else
		{
			// 2. �׼� ���� ���, Lineüũ
			nfChar.CalcWindLine(req->m_dReelClickTime);

			if (nfChar.GetLineLength() < 4)
				ansAction.m_lErrorCode = NF::G_NF_ERR_SUCCESS;		// ���� ������ �Ǿ����Ƿ�, Ŭ���̾�Ʈ�� �׼� ���и� ������.
			else
				ansAction.m_lErrorCode = NF::G_NF_ERR_FAIL;

			// Casting ��û�� ���� ����
			ansAction.m_lCSN = nfChar.GetCSN();
			ansAction.m_lActionType = req->m_lActionType;
			ansAction.m_lManualActionType = req->m_lManualActionType;
			ansAction.m_lstActionSkill = req->m_lstActionSkill;
			ansAction.m_lIncounterGauge = abilityExt.m_lActionIncountScore;
	
			theLog.Put(DEV_UK, "NGS_LOGIC, OnReqAction(). AnsActionResult CSN: ", pUser->GetCSN(), ", ActionType: ", ansAction.m_lActionType, ", ManualActionType: ", ansAction.m_lManualActionType, ", InCounterGauge: ", ansAction.m_lIncounterGauge);
		}
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode =;

	PayloadNGSCli pld(PayloadNGSCli::msgAnsActionResult_Tag, ansAction);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqHookingResult(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqHookingResult* req = pMsg->un.m_msgReqHookingResult;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		CNFChar& nfChar = pUser->GetNFUser();

		if (nfChar.GetActionStatus() != STATUS_CHARM)
			return;

		// Hooking ������, 1���� Fighting �޼����� ������.
		if ( HRT_SUCCESS <= req->m_lHookingResultType )
		{
			// ������Ʈ�� ����� ü�� ���
			if( HRT_GREAT == req->m_lHookingResultType )
			{
				CNFChar& nfChar = pUser->GetNFUser();
				NFCharInfoExt& nfCharInfoExt= nfChar.GetNFChar().m_nfCharInfoExt;

				// ĳ���� ��
				LONG lCharPower = 
					static_cast<LONG>( ROUND( ( sqrt( nfCharInfoExt.m_nfAbility.m_dStrength ) * CHAR_POWER_ROD )  + ( sqrt( nfCharInfoExt.m_nfAbility.m_dAgility ) * CHAR_POWER_REEL ), 1 ) );

				LONG lBulletFishHP = lCharPower * 3;
				nfChar.CalcFishHP(lBulletFishHP);
			}

			nfChar.SetActionStatus(STATUS_HOOKING);

			theLog.Put(WAR_UK, "NGS_LOGIC, OnReqHookingResult(). FishAI TimerStart CSN: ", pUser->GetCSN());

			// UserLocation�� LureLocation�� ��ǥ�� �̿��Ͽ� Distance�� LineLength�� ���Ѵ�.
			double dDistance = CalcHypote(req->m_UserLocation, req->m_LureLocation);
			nfChar.SetUserLocation(req->m_UserLocation);

			nfChar.SetDistanceFromFish(dDistance);
			nfChar.SetLineLength(dDistance);
		}

		if (req->m_lHookingResultType >= HRT_FAIL || req->m_lHookingResultType <= HRT_GREAT)
		{
			TMapAchvFactor mapFactorVal;
			mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_HOOKING_RESULT], req->m_lHookingResultType));
			g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_HOOKING, GetRoomID(), mapFactorVal);
		}
	}
	// else  pUser is NULL
}

void CRoomInternalLogic::ResultLanding(CUser* pUser, MsgNGSCli_AnsFigtingResult& ans, LONG lLandingBonusType)
{
	theLog.Put(WAR_UK, "NGS_LOGIC, OnSendFighting(), ErrorCode : ", ans.m_lErrorCode);

	achv_ProcessLanding(pUser);

	// �������ʽ� ��� ����
	if( lLandingBonusType != static_cast<LONG>( eLandingBonusType_None ) )
		pUser->SetAbil( AT_LANDING_BONUS, static_cast<int>( lLandingBonusType ) );

	// ���� ��� ���� �� ��� �޼��� ����
	SaveGameResult(pUser);
	InitFishing(pUser, EC_FR_LANDING);
}

void CRoomInternalLogic::ResultLineBreak_FeverMode(CUser* pUser, MsgNGSCli_AnsFigtingResult& ans, MsgCliNGS_ReqFighting& reqMsg, LineBreakInfo& info)
{
	if (info.m_lErrorCode == NF::EC_FR_CONTINUE)
	{
		if (!theFishAI.GetFishAIInfo(pUser, reqMsg, info, ans.m_lErrorCode)) {
			theLog.Put(ERR_UK, "NGS_LOGIC, ResultLineBreak_FeverMode(), CSN : ", pUser->GetCSN(), "// ErrorCode : ", ans.m_lErrorCode);
			InitFishing(pUser);
			return;
		}
	}
	else
		ans.m_lErrorCode = NF::EC_FR_SUCCESS;

	if (reqMsg.m_lPatternRate != 0)		// RateTime ���� 0�� �ƴ� ��쿡��, LineBreakChck �Ѵ�...
		ans.m_lErrorCode = LineBreakCheck_FeverMode(pUser, ans, reqMsg, info);
	else
		ans.m_lErrorCode = info.m_lErrorCode;

	if (ans.m_lErrorCode == info.m_lErrorCode)
	{
		switch(ans.m_lErrorCode)
		{
		case NF::EC_FR_CONTINUE:
			break;
		case NF::EC_FR_LANDING:
			{
				ResultLanding(pUser, ans, reqMsg.m_lLandingBonusType);
				break;
			}
		default:
			{
				InitFishing(pUser);
				break;
			}
		}
	}
	else
		theLog.Put(ERR_UK, "NGS_LOGIC, ResultLineBreak_FeverMode(), NotMatch CSN : ", pUser->GetCSN(), "Server Err : ", ans.m_lErrorCode, " // Client Err : ", info.m_lErrorCode);
}

void CRoomInternalLogic::ResultLineBreak(CUser* pUser, MsgNGSCli_AnsFigtingResult& ans, MsgCliNGS_ReqFighting& reqMsg, LineBreakInfo& info)
{
	if (info.m_lErrorCode == NF::EC_FR_CONTINUE)
	{
		if (!theFishAI.GetFishAIInfo(pUser, reqMsg, info, ans.m_lErrorCode)) {
			theLog.Put(ERR_UK, "NGS_LOGIC, ResultLineBreak(), CSN : ", pUser->GetCSN(), "// ErrorCode : ", ans.m_lErrorCode);
			InitFishing(pUser);
			return;
		}
	}
	else
		ans.m_lErrorCode = NF::EC_FR_SUCCESS;

	if (reqMsg.m_lPatternRate != 0)		// RateTime ���� 0�� �ƴ� ��쿡��, LineBreakChck �Ѵ�...
		ans.m_lErrorCode = LineBreakCheck(pUser, ans, reqMsg, info);
	else
		ans.m_lErrorCode = info.m_lErrorCode;

	// 4. LineBreakCheck�� �������� Ŭ�� ���� ���� Ʋ����, �������� ������ �Ǵ��Ѱ��� �̱� ������ �ٷ� ������.
	if (ans.m_lErrorCode == info.m_lErrorCode)
	{
		switch(ans.m_lErrorCode)
		{
		case NF::EC_FR_CONTINUE:
				break;
		case NF::EC_FR_LANDING:
			{
				ResultLanding(pUser, ans, reqMsg.m_lLandingBonusType);
				break;
			}
		case NF::EC_FR_LINEBREAK:
		case NF::EC_FR_HEADSHAKE_LOAD:
		case NF::EC_FR_MAXTIRELESS:
		case NF::EC_FR_LURELOSS:
		case NF::EC_FR_HEADSHAKE_LANDING:
		default:
			{
				InitFishing(pUser, ans.m_lErrorCode);
				break;
			}
		}

		ErrorReportingToClient(pUser, ans.m_lErrorCode);
	}
	else
		theLog.Put(ERR_UK, "NGS_LOGIC, ResultLineBreak(), NotMatch CSN : ", pUser->GetCSN(), "Server Err : ", ans.m_lErrorCode, " // Client Err : ", info.m_lErrorCode);
}

void CRoomInternalLogic::ErrorReportingToClient(CUser* pUser, LONG err_code)
{
	MsgNGSCli_NtfErrorReporting	ntf;

	CNFChar& nfChar = pUser->GetNFUser();
	LONG lItemCode = nfChar.GetNFChar().m_nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode;
	EquipItem* pLine = theNFDataItemMgr.GetEquipItemByIndex(lItemCode);
	if (pLine)
		ntf.m_line = (*pLine);

	lItemCode = nfChar.GetNFChar().m_nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode;
	EquipItem* pLure = theNFDataItemMgr.GetEquipItemByIndex(lItemCode);
	if (pLure)
		ntf.m_lure = (*pLure);

	lItemCode = nfChar.GetNFChar().m_nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Rod].m_lItemCode;
	EquipItem* pRod = theNFDataItemMgr.GetEquipItemByIndex(lItemCode);
	if (pRod)
		ntf.m_rod = (*pRod);

	lItemCode = nfChar.GetNFChar().m_nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Reel].m_lItemCode;
	EquipItem* pReel = theNFDataItemMgr.GetEquipItemByIndex(lItemCode);
	if (pReel)
		ntf.m_reel = (*pReel);

	ntf.m_fishInfo = nfChar.GetBiteFish().GetFish();
	nfChar.GetBiteFish().GetFishSkill(ntf.m_fishSkillInfo);

	PayloadNGSCli pld(PayloadNGSCli::msgNtfErrorReporting_Tag, ntf);
	UserManager->SendToUser(pUser->GetCSN(), pld);
}

// 2011.1.6 FishAI & LineBreak Check�� ����
void CRoomInternalLogic::OnReqFighting(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqFighting* pRcvMsg = pMsg->un.m_msgReqFighting;
	MsgNGSCli_AnsFigtingResult		ans;
	ans.m_dDistance = 0;
	ans.m_dDropAcceleration = 0;
	ans.m_dLineLength = 0;
	ans.m_dLineLoose = 0;
	ans.m_fLineLoadPresent = 0;
	ans.m_fMaxLineTension = 0;
	ans.m_dCharacterTireless = 0;
	ans.m_lFishHP = 0;
	ans.m_lFishSkillInfo.Clear();
	ans.m_dMaxCharTireless = 0;
	ans.m_lPRNGCount = 0;
	ans.m_lPRNGValue = 0;
	ans.m_lCSN = lCSN;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		CNFChar& nfChar = pUser->GetNFUser();

		if (nfChar.GetActionStatus() < STATUS_HOOKING)
			return;

		if (nfChar.GetDebuggingMode() == EN_DM_FIGHTING)
			nfChar.GetDebuggingCasting(pMsg);

		LONG lFishAICnt = pRcvMsg->m_lstLineBreakInfo.size();

		// 1. FishAICnt Range üũ
		if (lFishAICnt > 0 && lFishAICnt <= 50) 
		{
			// 2. FishAI üũ
			ForEachElmt(NFLineBreakInfoList, pRcvMsg->m_lstLineBreakInfo, it, ij)
			{
				if (pRcvMsg->m_bIsFeverMode)		// Fever ��� �� ���,
					ResultLineBreak_FeverMode(pUser, ans, *pRcvMsg, (*it));
				else
					ResultLineBreak(pUser, ans, *pRcvMsg, (*it));

				theLog.Put(WAR_UK, "NGS_LOGIC, OnSendFighting(), -@@@@@- ErrorCode : ", (*it).m_lErrorCode, ", Cli-PRNG :", (*it).m_lPRNGValue, ", Ser-PRNG : ", nfChar.GetPRNG_value(), "#### size:", pRcvMsg->m_lstLineBreakInfo.size());

				if (ans.m_lErrorCode != EC_FR_CONTINUE)
					break;
			}
		}
		else {
			ans.m_lErrorCode = NF::EC_FR_INVALID_FISH_CNT_RANGE;		// FishAICnt�� ���� 0���� 50 ���̰� �ƴϴ�...
			InitFishing(pUser);
		}	
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode = ;

	theLog.Put(WAR_UK, "NGS_LOGIC, OnReqFighting(), ---------------- CSN : ", ans.m_lCSN);

	if (EC_FR_LANDING != ans.m_lErrorCode)
	{
		PayloadNGSCli pld(PayloadNGSCli::msgSendFigthingResult_Tag, ans);
		UserManager->SendToUser(lCSN, pld);
	}
}

void CRoomInternalLogic::OnReqLanding(LONG lCSN, PayloadCliNGS* pMsg)
{
	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		achv_ProcessLanding(pUser);
		SaveGameResult(pUser);
		InitFishing(pUser);
		theLog.Put(WAR_UK, "NGS_LOGIC, OnReqLanding(). Send AnsReqLanding! CSN: ", pUser->GetCSN());
	}
}

void CRoomInternalLogic::OnReqUseItem(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqUseItem* req = pMsg->un.m_msgReqUseItem;
	MsgNGSCli_AnsUseItem	ans;
	ans.m_ansUsedItem.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	ans.m_ansUsedItem.m_bIsRemove = FALSE;
	ans.m_ansUsedItem.m_usedInven.Clear();

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		//////////////////////////////////////////////////////////////////////////
		long lErrorCode = ApplyUseItem(pUser, req->m_lItemCode, req->m_lQuickSlotIndex, ans.m_ansUsedItem);
		if (lErrorCode != NF::G_NF_ERR_SUCCESS)
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqLanding(). Error OnReqUseItem! USN : ", pUser->GetCSN(), ", ErrorCode : ", ans.m_ansUsedItem.m_lErrorCode);
	}
//	else
//		ans.m_ansExchangeCards.m_lErrorCode = ;

	PayloadNGSCli pld(PayloadNGSCli::msgAnsUseItem_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// Debugging : ��������϶��� ����ǵ��� ����
void CRoomInternalLogic::OnReqGameOver(LONG lCSN, PayloadCliNGS* pMsg)
{
	// ans
	MsgNGSCli_AnsGameOver	ans;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (m_nfRoomOption.m_lPlayType != 1)
		{
			// TimerKill
			m_nfRoomOption.m_lRemainTime = 0;

			PayloadNGSCli pld(PayloadNGSCli::msgAnsGameOver_Tag, ans);
			UserManager->SendToAllUser(pld);

			theLog.Put(WAR_UK, "NGS_LOGIC, OnReqGameOver(). GameOver!!!: ", pUser->GetCSN());

			// GameOver;
			SetRunStopFlag(ROOMSTATE_RUN);

			// ���� ���õ� ������ �ʱ�ȭ�Ѵ�.
			InitTeamPlayData();
		}
	}
}

// ����� ���� �����ϴ� ��Ŷ 
// ����Ⱑ �̵��� Ŭ���̾�Ʈ���� �浹�� �Ͼ ������ �����ؾ� �ϴ� ��� ������ ��
void CRoomInternalLogic::OnReqCorrectDirection(LONG lCSN, PayloadCliNGS* pMsg)
{
	
}


// �����̵� �����丮��, HIST_ID�� ������ �Ǵ��Ͽ� �޸𸮿� �����ϰ� �ִٰ�, 
// ���� �޼� ������ �Ǹ�, DB succeed_archive ���ν��� ȣ���� �Ŀ�, 
// ���� Notify ������ ���� �˸�
void CRoomInternalLogic::OnReqUpdateAchieveInfo(LONG lCSN, PayloadCliNGS* pMsg)
{
	if (!pMsg)	return;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (NULL == pUser)
		return;

	MsgCliNGS_ReqUpdateAchvInfo*	req = pMsg->un.m_msgReqUpdateAchvInfo;
	MsgNGSCli_AnsUpdateAchvInfo	ans;
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (req->m_lFactorType >= achv::AF_CAUSE || req->m_lFactorType <= achv::AF_FACTOR_MAX)
	{
		if (req->m_lEventID >= achv::AE_NONE || req->m_lEventID <= achv::AE_ACHV_MAX)
		{
			TMapAchvFactor mapFactorVal;
			mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[req->m_lFactorType], req->m_lGauge));
			g_achv.CheckAchv(pUser->GetGSN(), lCSN, req->m_lEventID, GetRoomID(), mapFactorVal);
		}
		else
			ans.m_lErrorCode = NF::EC_ACHV_NOT_FOUND_EVENT_TO_ACHV_ID;
	}
	else
		ans.m_lErrorCode = NF::EC_ACHV_NOT_FOUND_FACTOR_TO_ACHV_ID;

	PayloadNGSCli pld(PayloadNGSCli::msgAnsUpdateAchvInfo_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// FreeMode �� ���, �ش� Category(Map)�� �ش��ϴ� RoomList ��û
void CRoomInternalLogic::OnReqFreeRoomList(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgNGSCHS_ReqFreeRoomInfo	ans;
	ans.m_lCSN						= lCSN;
	ans.m_roomID					= GetRoomID();
	ans.m_addrNGS					= GetRoomAddr();
	PayloadNGSCHS pld(PayloadNGSCHS::msgReqFreeRoomInfo_Tag, ans);

	NotifyUtil->SendToCHS(GetCHSAddr(), pld);
}

void CRoomInternalLogic::OnAnsFreeRoomList(LONG lCSN, MsgCHSNGS_AnsFreeRoomList* pMsg)
{
	MsgNGSCli_AnsFreeRoomList	ans;
	ans.m_lstFreeRoomInfo = pMsg->m_lstRoomInfo;
	PayloadNGSCli	pld(PayloadNGSCli::msgAnsFreeRoomList_Tag, ans);

	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnCheatReq(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_CheatReq* req = pMsg->un.m_msgCheatReq;
 	GBuf buf((LPVOID)req->m_msg.c_str(), req->m_msg.size());

	CUser* pUser = UserManager->FindUser(lCSN);
	if( pUser )
	{
		// TODO: ġƮ ��밡���� �����ΰ�?

 		switch( req->m_nCheatID )
 		{
 		case NFCHEAT_REQFISH:
 			{
 				NFCheat_ReqFish msg;
 				msg.Clear();
 				msg.BLoad(buf);
 
 				pUser->SetAbil(AT_REQFISH, msg.m_lReqFishIndex);
 			}break;
 		case NFCHEAT_REQSIGN:
 			{
				NFCheat_ReqSign msg;
				msg.Clear();
				msg.BLoad(buf);
				
				SetAbil(AT_REQSIGN, msg.m_lReqSignIndex);
				
				FishInfo dummyFishInfo;
				LONG dummySignIndex;
				CheckSignPoint(dummyFishInfo, dummySignIndex);
 			}break;
		case NFCHEAT_GETMONEY:
			{
				NFCheat_GetMoney req;
				req.Clear();
				req.BLoad(buf);
				const __int64 i64AddMoney = req.m_i64Money;

				MsgNGSCli_CheatAns ans;
				ans.Clear();

				if( 0 < i64AddMoney )
				{
					__int64 i64CurMoney = pUser->GetNFUser().GetMoney();
					if( theNFDBMgr.UpdateCharMoney( pUser->GetCSN(), i64CurMoney + i64AddMoney ) )
					{
						pUser->GetNFUser().SetMoney( i64CurMoney + i64AddMoney );

						NFCheat_GetMoney result;
						result.m_i64Money = i64AddMoney; // ȹ���� ���ӸӴ�
						GBuf sendBuf;
						result.BStore(sendBuf);

						ans.m_bSuccess = TRUE;
						ans.m_nCheatID = NFCHEAT_GETMONEY;
						ans.m_msg.assign((LPCSTR)sendBuf->GetData(), sendBuf->GetLength());
					}
				}

				PayloadNGSCli pld(PayloadNGSCli::msgCheatAns_Tag, ans);
				UserManager->SendToUser(lCSN, pld);
			}break;
		case NFCHEAT_SETLEVEL:
			{
				NFCheat_SetLevel cheat;
				cheat.Clear();
				cheat.BLoad(buf);
				const LONG lSetLevel = cheat.m_lLevel;

				MsgNGSCli_CheatAns ans;
				ans.Clear();

				TMapNFLevel& mapNFLevel = theNFDataItemMgr.GetNFLevel();
				TMapNFLevel::const_iterator level_iter = mapNFLevel.find(lSetLevel);
				if( level_iter != mapNFLevel.end() )
				{
					TMapNFLevel::mapped_type nfLevel = level_iter->second;
					LONG lSetExp = nfLevel->m_lNeedExp;

					CNFChar& nfChar = pUser->GetNFUser();
					NFUser& nfUser = nfChar.GetNFChar();
					LONG lCurLevel = nfChar.GetLevel();

					if( theNFDBMgr.UpdateCharExpAndLevel(pUser->GetCSN(), lSetExp, lSetLevel) ) // ���� ����
					{
						if( lCurLevel < lSetLevel ) // ������
						{
							// �ɷ�ġ ����� ���� ������ �������� ������ �ɷ�ġ ���� ���̺��� �������Ƿ�, 
							// ���� ���鼭 ������ �����ִ� �� �ۿ� ����.
							for( LONG lLevel = lCurLevel + 1; lLevel <= lSetLevel; ++lLevel )
							{
								NFAbility abilityGap;
								abilityGap.Clear();

								if( NF::G_NF_ERR_SUCCESS == theNFDataItemMgr.GetNFAbilityGapByLevel( lLevel, abilityGap ) )
								{
									nfUser.m_nfCharInfoExt.m_nfAbility += abilityGap; // �ɷ�ġ ����
								}
							}
						}
						else if( lSetLevel < lCurLevel ) // ���� �ٿ�
						{
							for( LONG lLevel = lCurLevel; lSetLevel < lLevel; --lLevel )
							{
								NFAbility abilityGap;
								abilityGap.Clear();

								if( NF::G_NF_ERR_SUCCESS == theNFDataItemMgr.GetNFAbilityGapByLevel( lLevel, abilityGap ) )
								{
									nfUser.m_nfCharInfoExt.m_nfAbility -= abilityGap; // �ɷ�ġ ��
								}
							}
						}

						pUser->GetNFUser().SetExp(lSetExp);
						pUser->GetNFUser().SetLevel(lSetLevel);

						NFCheat_SetLevel result;
						result.m_lLevel = lSetLevel; // ����� ����
						result.m_nfAbility = nfUser.m_nfCharInfoExt.m_nfAbility; // ����� �ɷ�ġ

						GBuf sendBuf;
						result.BStore(sendBuf);
						
						ans.m_bSuccess = TRUE;
						ans.m_nCheatID = NFCHEAT_SETLEVEL;
						ans.m_msg.assign((LPCSTR)sendBuf->GetData(), sendBuf->GetLength());

						ArcMapT< std::string,LONG > mapFactorVal;
						mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_LEVEL], lSetLevel));
						g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_LEVEL, GetRoomID(), mapFactorVal);
					}
				}
				PayloadNGSCli pld(PayloadNGSCli::msgCheatAns_Tag, ans);
				UserManager->SendToUser(lCSN, pld);
			}break;
		case NFCHEAT_SET_LANDING_SCORE:
			{
				NFCheat_SetLandingScore req;
				req.Clear();
				req.BLoad(buf);

				MsgNGSCli_CheatAns ans;
				ans.Clear();
				ans.m_nCheatID = NFCHEAT_SET_LANDING_SCORE;

				CNFChar& nfChar = pUser->GetNFUser();
				TMapLockedNoteMap& mapLandNote = nfChar.GetNFChar().m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMap;
				TMapLockedNoteMap::iterator iter = mapLandNote.find(req.m_lMapID);
				if (iter != mapLandNote.end())
				{
					SYSTEMTIME systime;
					::GetLocalTime(&systime);
					std::string strUpdateDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
					LONG lErr = NF::G_NF_ERR_SUCCESS;
					theNFDBMgr.UpdateLockedNoteMap(req.m_lMapID, pUser->GetGSN(), pUser->GetCSN(), (*iter).second.m_lTotUnlockFishCNT, req.m_lScore, strUpdateDate, lErr);
					iter->second.m_lTotLockedScore = req.m_lScore;

					NFCheat_SetLandingScore result;
					result.m_lMapID = req.m_lMapID;
					result.m_lScore = req.m_lScore;				

					GBuf sendBuf;
					result.BStore(sendBuf);

					ans.m_bSuccess = TRUE;
					ans.m_msg.assign((LPCSTR)sendBuf->GetData(), sendBuf->GetLength());
				}

				PayloadNGSCli pld(PayloadNGSCli::msgCheatAns_Tag, ans);
				UserManager->SendToUser(lCSN, pld);
			}break;
		case NFCHEAT_SET_ACHV:
			{
				//// ���Ӿ����� ó���� �޼� �ǰ� �߰������� ��û�ؾ� ��...
				//NFCheat_SetAchv req;
				//req.Clear();
				//req.BLoad(buf);

				//MsgNGSCli_CheatAns ans;
				//ans.Clear();
				//ans.m_nCheatID = NFCHEAT_SET_ACHV;

				//CNFChar& nfChar = pUser->GetNFUser();
				//achv::ClerkPtr cptr = pBureau->getClerk(nfChar.GetNFChar().m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
				//if (cptr) 
				//{
				//	const achv::EventAchvIDMap& mapEvent = pBureau->GetEvtAchvIDMap();
				//	achv::Event event = achv::EVT_MAX;

				//	ForEachCElmt(achv::EventAchvIDMap, mapEvent, it, ij)
				//	{
				//		if ((*it).second == req.m_nAchvID)
				//		{
				//			event = (*it).first;
				//			break;
				//		}
				//	}

				//	if (event != achv::EVT_MAX)
				//		cptr->report(event, req.m_dGauge, GetRoomID());

				//	NFCheat_SetAchv result;
				//	result.m_nAchvID = req.m_nAchvID;
				//	result.m_dGauge = req.m_dGauge;				

				//	GBuf sendBuf;
				//	result.BStore(sendBuf);

				//	ans.m_bSuccess = TRUE;
				//	ans.m_msg.assign((LPCSTR)sendBuf->GetData(), sendBuf->GetLength());
				//}
				//PayloadNGSCli pld(PayloadNGSCli::msgCheatAns_Tag, ans);
				//UserManager->SendToUser(lCSN, pld);
			}break;
 		default: break;
 		}
	}
}

// Community
void CRoomInternalLogic::OnReqNFLetterList(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFLetterList* req = pMsg->un.m_msgReqNFLetterList;	
	MsgNGSCli_AnsNFLetterList ans;

	if( FALSE == GetNFLetterList( lCSN, ans.m_kContLetter, req->m_bNewLetter ) )
	{
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
	}

	ans.m_lErrorCode = NF::EC_LE_SUCCESS;
	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFLetterList_Tag, ans);
	UserManager->SendToUser(lCSN, pld);	
}

void CRoomInternalLogic::OnReqNFLetterContent(LONG lCSN, PayloadCliNGS* pMsg)
{	
	MsgCliNGS_ReqNFLetterContent* req = pMsg->un.m_msgReqNFLetterContent;
	MsgNGSCli_AnsNFLetterContent ans;

	if( GetNFLetterContent( req->m_i64LetterIndex, ans.m_strContent, ans.m_strSendTime ) )
	{
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
	}

	ans.m_lErrorCode = NF::EC_LE_SUCCESS;
	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFLetterContent_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqNFLetterSend(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFLetterSend* req = pMsg->un.m_msgReqNFLetterSend;
	MsgNGSCli_AnsNFLetterSend ans;	

	if( SendNFLetter( req->m_strReceiver, req->m_nfLetter ) )
	{	
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
	}

	// 1. �޴� ĳ���Ͱ� �����ϴ°�?
	TKey receiverKey;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( req->m_strReceiver, receiverKey ) )
	{	
		ans.m_lErrorCode = NF::EC_LE_NOT_EXIST_RECEIVER;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFLetterSend_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	// 2. ���� ���ߴ°�?
	LONG lStatus = FR_NONE;
	if( TRUE == theNFDBMgr.SelectNFFriendRelation( receiverKey.m_lSubKey, lCSN, FR_BLOCK, lStatus) )
	{
		// ���� ������ ���ܿ� ��ϵǾ� �ִ���?
		if( FR_BLOCK == lStatus )
		{	
			ans.m_lErrorCode = NF::EC_LE_BLOCK;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFLetterSend_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}
	}
	else
	{
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFLetterSend_Tag, ans);	
		SendToUser(lCSN, pld);
		return;
	}

	ans.m_lErrorCode = NF::EC_LE_SUCCESS;
	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFLetterSend_Tag, ans);
	UserManager->SendToUser(lCSN, pld);

	// �� ������ ������ �˷��ش�.
	CUser* pReceiver = UserManager->FindUser(receiverKey.m_lSubKey);
	if( pReceiver )
	{
		MsgNGSCli_NtfNFLetterReceive ntf;
		PayloadNGSCli pld(PayloadNGSCli::msgNtfNFLetterReceive_Tag, ntf);
		UserManager->SendToUser( receiverKey.m_lMainKey, pld );
	}
	else
	{
		CUser* pUser = UserManager->FindUser( lCSN );
		if( !pUser )
		{
			return;
		}

		ArcVectorT<TKey> kContKey;
		kContKey.push_back( receiverKey );

		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
		pld.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
		pld.un.m_msgReqLocation->m_kContKey = kContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_NEW_LETTER;
		theNLSManager.GetUserLocation(pld);
	}
}

void CRoomInternalLogic::OnReqNFLetterReceiverCheck(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFLetterReceiverCheck* req = pMsg->un.m_msgReqNFLetterReceiverCheck;

	MsgNGSCli_AnsNFLetterReceiverCheck ans;
	ans.m_lErrorCode = NF::EC_LE_SUCCESS;

	TKey receiverKey;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( req->m_strReceiver, receiverKey ) )
	{
		ans.m_lErrorCode = NF::EC_LE_NOT_EXIST_RECEIVER;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFLetterReceiverCheck_Tag, ans);
	SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqNFLetterDelete(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFLetterDelete* req = pMsg->un.m_msgReqNFLetterDelete;
	MsgNGSCli_AnsNFLetterDelete ans;
	ans.m_lErrorCode = NF::EC_LE_SUCCESS;

	if( !DeleteNFLetter( req->m_kContLetterIndex ) )
	{
		ans.m_lErrorCode = NF::EC_LE_DB_ERROR;
	}
	
	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFLetterDelete_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqNFFriendInfo(LONG lCSN, PayloadCliNGS* pMsg)
{
	CUser* pUser = UserManager->FindUser(lCSN);
	if( !pUser )
	{
		return;
	}

	CONT_NF_FRIEND kContNFFriend;
	kContNFFriend = pUser->GetNFFriendInfo();

	// ģ�������� ���� ����
	if( kContNFFriend.empty() )
	{
		// DB���� ģ�� ����Ʈ ����
		if( FALSE == GetNFFriendInfo( lCSN, kContNFFriend ) )
		{
			MsgNGSCli_AnsNFFriendInfo ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;

			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendInfo_Tag, ans);
			SendToUser(lCSN, pld);
		}

		pUser->SetNFFriendInfo( kContNFFriend );
	}

	// NLS�� ģ���� ��ġ ��û
	ArcVectorT<TKey> kContKey;
	CONT_NF_FRIEND::iterator iter = kContNFFriend.begin();
	while( iter != kContNFFriend.end() )
	{
		kContKey.push_back( iter->first );
		++iter;
	}

	PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
	pld.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
	pld.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
	pld.un.m_msgReqLocation->m_kContKey = kContKey;
	pld.un.m_msgReqLocation->m_lCause = NLRC_FRIEND_LIST;
	theNLSManager.GetUserLocation(pld);	
}

// ģ����û�� �� ��
void CRoomInternalLogic::OnReqNFFriendAdd(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFFriendAdd* req = pMsg->un.m_msgReqNFFriendAdd;

	CUser* pUser = UserManager->FindUser(lCSN);
	if( !pUser )
	{
		return;
	}

	// �ڽ����� ģ�� �߰� ����.
	if( req->m_strCharName == pUser->GetCharName() )
	{
		MsgNGSCli_AnsNFFriendInfo ans;
		ans.m_lErrorCode = NF::EC_FE_SELF;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	CONT_NF_FRIEND kContNFFriend = pUser->GetNFFriendInfo();
	if( kContNFFriend.empty() )
	{
		// ģ������� ������ DB���� �о�´�.
		if( FALSE == theNFDBMgr.SelectNFFriendInfo( lCSN, kContNFFriend, FR_FRIEND ) )	
		{
			MsgNGSCli_AnsNFFriendInfo ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		pUser->SetNFFriendInfo( kContNFFriend );
	}

	// �̹� ģ�� ���� �˻�
	CONT_NF_FRIEND::const_iterator friend_iter = kContNFFriend.begin();
	while( kContNFFriend.end() != friend_iter )
	{
		CONT_NF_FRIEND::mapped_type nfFriend = friend_iter->second;
		if( req->m_strCharName == nfFriend.m_strCharName )
		{
			MsgNGSCli_AnsNFFriendInfo ans;
			ans.m_lErrorCode = NF::EC_FE_ALREADY_FRIEND;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		++friend_iter;
	}

	// ģ�� �ִ� ����ο� 100�� üũ
	if( NF_FRIEND_MAX_CNT <= kContNFFriend.size() )
	{
		MsgNGSCli_AnsNFFriendInfo ans;
		ans.m_lErrorCode = NF::EC_FE_FRIEND_OVERFLOW;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	// ĳ���Ͱ� �����ϴ��� üũ( USN, CSN ��� )
	TKey key;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( req->m_strCharName, key ) )
	{
		MsgNGSCli_AnsNFFriendInfo ans;
		ans.m_lErrorCode = NF::EC_FE_NOT_EXIST_CHARACTER;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);	
		SendToUser(lCSN, pld);
		return;
	}

	LONG lStatus = FR_NONE;
	if( TRUE == theNFDBMgr.SelectNFFriendRelation( key.m_lSubKey, lCSN, FR_BLOCK, lStatus) )
	{
		// ���� ������ ���ܿ� ��ϵǾ� �ִ���?
		if( FR_BLOCK == lStatus )
		{
			MsgNGSCli_AnsNFFriendInfo ans;
			ans.m_lErrorCode = NF::EC_FE_BLOCK;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);	
			SendToUser(lCSN, pld);
			return;
		}

		if( TRUE == theNFDBMgr.SelectNFFriendRelation( key.m_lSubKey, lCSN, FR_FRIEND, lStatus) )
		{
			// ������ �̹� ���� ģ�� �����ΰ�?( ���� ������ ģ�� ���� �߾���, ���� �ٽ� ��û�ϴ� ��� )
			if( FR_FRIEND == lStatus )
			{
				// ��û�ڸ� ������(���������� ������)�� ģ���� ����Ѵ�. �������� �˸� �ʿ䵵 ����.				
				if( theNFDBMgr.InsertNFFriend( lCSN, key.m_lSubKey, FR_FRIEND ) )
				{
					CNFFriend nfFriend;
					nfFriend.m_strCharName = req->m_strCharName;
					nfFriend.m_bIsOnline = FALSE; // ���� �¶������� �𸣴ϱ�.
					pUser->AddNFFriend( key, nfFriend );

					{// ģ����û�� ������ �˸���, noti�� ���� �ش�.
						MsgNGSCli_AnsNFFriendInfo ans;
						ans.m_lErrorCode = NF::EC_FE_SUCCESS;
						PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);
						SendToUser(lCSN, pld);
					}

					// �ڵ� �������� ������ NSL�� ��û
					ArcVectorT<TKey> kContKey;
					kContKey.push_back(key);
					PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
					pld.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
					pld.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
					pld.un.m_msgReqLocation->m_kContKey = kContKey;
					pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_AUTO_ACCEPT_FRIEND;
					theNLSManager.GetUserLocation(pld);

					return;
				}
			}
		}
	}
	else
	{
		MsgNGSCli_AnsNFFriendInfo ans;
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);	
		SendToUser(lCSN, pld);
		return;
	}

	BOOL bIsSuccessBoth = FALSE; // ģ���߰� ���� �Ѵ� �����ߴ���.	
	if( theNFDBMgr.InsertNFFriend( pUser->GetCSN(), key.m_lSubKey, FR_FRIEND_SEND ) )
	{
		if( theNFDBMgr.InsertNFFriend( key.m_lSubKey, pUser->GetCSN(), FR_FRIEND_RECV ) )
		{
			bIsSuccessBoth = TRUE;
		}
		else
		{
			// �ѹ�.
			theNFDBMgr.DeleteNFFriend( pUser->GetCharName(), req->m_strCharName );
		}
	}

	if( FALSE == bIsSuccessBoth)
	{
		MsgNGSCli_AnsNFFriendInfo ans;
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	// �¶����̸� �ٷ� �˷���� �ϹǷ�, NLS�� ���� ��ġ�� ��û
	ArcVectorT<TKey> kContKey;
	kContKey.push_back(key);

	PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
	pld.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
	pld.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
	pld.un.m_msgReqLocation->m_kContKey = kContKey;
	pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_ADD_FRIEND;
	theNLSManager.GetUserLocation(pld);
}

void CRoomInternalLogic::OnReqNFFriendReject(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFFriendReject* pReq = pMsg->un.m_msgReqNFFriendReject;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (!pUser)
	{
		return;
	}

	MsgNGSCli_AnsNFFriendReject ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;

	if( theNFDBMgr.DeleteNFFriend( pUser->GetCharName(), pReq->m_strCharName, FR_FRIEND_RECV )
		&& theNFDBMgr.DeleteNFFriend( pReq->m_strCharName, pUser->GetCharName(), FR_FRIEND_SEND ) )
	{
		pUser->DeleteNFFriendApplicant(pReq->m_strCharName);
	}
	else
	{
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendReject_Tag, ans);
	SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqNFFriendAccept(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFFriendAccept* pReq = pMsg->un.m_msgReqNFFriendAccept;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (!pUser)
	{
		return;
	}

	CONT_NF_FRIEND kContNFFriend = pUser->GetNFFriendInfo();
	if( kContNFFriend.empty() )
	{
		// ģ������� ������ DB���� �о�´�.
		if( FALSE == theNFDBMgr.SelectNFFriendInfo( lCSN, kContNFFriend, FR_FRIEND ) )	
		{
			MsgNGSCli_AnsNFFriendAccept ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAccept_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		pUser->SetNFFriendInfo( kContNFFriend );
	}

	// �̹� ģ�� ���� �˻�
	CONT_NF_FRIEND::const_iterator friend_iter = kContNFFriend.begin();
	while( kContNFFriend.end() != friend_iter )
	{
		CONT_NF_FRIEND::mapped_type nfFriend = friend_iter->second;
		if( pReq->m_strCharName == nfFriend.m_strCharName )
		{
			MsgNGSCli_AnsNFFriendAccept ans;
			ans.m_lErrorCode = NF::EC_FE_ALREADY_FRIEND;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAccept_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		++friend_iter;
	}

	// ģ�� �ִ� ����ο� 100�� üũ
	if( NF_FRIEND_MAX_CNT <= kContNFFriend.size() )
	{
		MsgNGSCli_AnsNFFriendInfo ans;
		ans.m_lErrorCode = NF::EC_FE_FRIEND_OVERFLOW;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	// ģ�� ���·� ������Ʈ
	if( FALSE == NFFriendAccept( pUser->GetCharName(), pReq->m_strCharName ) )
	{
		MsgNGSCli_AnsNFFriendInfo ans;
		ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAccept_Tag, ans);
		SendToUser(lCSN, pld);
		return;
	}

	TKey key;
	if( GetNFCharKeyByCharName( pReq->m_strCharName, key ) )
	{
		// �������� ģ����Ͽ� ��û���� ������ ���Ѵ�.
		// ��ġ, ������ NLS���������� ������ ���Ŀ� ������Ʈ �Ѵ�.
		CNFFriend nfFriend;
		nfFriend.m_strCharName = pReq->m_strCharName;
		nfFriend.m_bIsOnline = FALSE; // ���� �¶������� �𸣴ϱ�.
		pUser->AddNFFriend(key, nfFriend);

		ArcVectorT<TKey> KContKey;
		KContKey.push_back(key);

		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
		pld.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
		pld.un.m_msgReqLocation->m_kContKey = KContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_NTF_ACCEPT_FRIEND;
		pld.un.m_msgReqLocation->m_strCharName = pReq->m_strCharName;
		theNLSManager.GetUserLocation(pld);
	}
}

void CRoomInternalLogic::OnReqNFBlockList(LONG lCSN, PayloadCliNGS* pMsg)
{
	CUser* pUser = UserManager->FindUser(lCSN);
	if( !pUser )
	{
		return;
	}

	CONT_NF_FRIEND_NICK kContNFBlock = pUser->GetNFBlockList();
	if( kContNFBlock.empty() )
	{
		if( FALSE == theNFDBMgr.SelectNFFriendNickByStatus( lCSN, kContNFBlock, FR_BLOCK ) )	
		{
			MsgNGSCli_AnsNFBlockList ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFBlockList_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		pUser->SetNFBlock( kContNFBlock );
	}

	MsgNGSCli_AnsNFBlockList ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	ans.m_kContBlockList = kContNFBlock;
	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFBlockList_Tag, ans);
	SendToUser(lCSN, pld);
	return;
}

void CRoomInternalLogic::OnReqNFFriendApplicantList(LONG lCSN, PayloadCliNGS* pMsg)
{
	CUser* pUser = UserManager->FindUser(lCSN);
	if( !pUser )
	{
		return;
	}

	CONT_NF_FRIEND_NICK kContNFFriendApplicant = pUser->GetNFBlockList();
	if( kContNFFriendApplicant.empty() )
	{
		if( FALSE == theNFDBMgr.SelectNFFriendNickByStatus( lCSN, kContNFFriendApplicant, FR_FRIEND_RECV ) )	
		{
			MsgNGSCli_AnsNFFriendApplicantList ans;
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendApplicantList_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		pUser->SetNFBlock( kContNFFriendApplicant );
	}

	MsgNGSCli_AnsNFFriendApplicantList ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	ans.m_kContNFFriendApplicantList = kContNFFriendApplicant;
	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendApplicantList_Tag, ans);
	SendToUser(lCSN, pld);
	return;
}

void CRoomInternalLogic::OnReqFollowUser(LONG lCSN, PayloadCliNGS* pMsg)
{
	CUser* pUser = UserManager->FindUser(lCSN);
	if( !pUser )
	{
		return;
	}

	MsgCliNGS_ReqFollowUser* req = pMsg->un.m_msgReqFollowUser;

	TKey key;
	if( theNFDBMgr.SelectNFCharKeyByCharName( req->m_strCharName, key ) )
	{
		ArcVectorT<TKey> kContKey;
		kContKey.push_back( key );

		PayloadCLINLS pld(PayloadCLINLS::msgReqLocation_Tag);
		pld.un.m_msgReqLocation->m_lUSN = pUser->GetUSN();
		pld.un.m_msgReqLocation->m_lCSN = pUser->GetCSN();
		pld.un.m_msgReqLocation->m_kContKey = kContKey;
		pld.un.m_msgReqLocation->m_lCause = NLRC_FOLLOW_USER;
		pld.un.m_msgReqLocation->m_strCharName = req->m_strCharName;
		theNLSManager.GetUserLocation(pld);
	}
}

void CRoomInternalLogic::OnReqNFBlockOrUnBlock(LONG lCSN, PayloadCliNGS* pMsg)
{
	CUser* pUser = UserManager->FindUser(lCSN);
	if( !pUser )
	{
		return;
	}

	MsgCliNGS_ReqNFBlockOrUnBlock* req = pMsg->un.m_msgReqNFBlockOrUnBlock;
	MsgNGSCli_AnsNFBlockOrUnBlock ans;

	string strCharName = tolower(req->m_strCharName);

	// ĳ���Ͱ� �����ϴ��� üũ( USN, CSN ��� )
	TKey key;
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName( strCharName, key ) )
	{	
		ans.m_lErrorCode = NF::EC_FE_NOT_EXIST_CHARACTER;
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFBlockOrUnBlock_Tag, ans);	
		SendToUser(lCSN, pld);
		return;
	}

	CONT_NF_FRIEND_NICK kContNFBlock = pUser->GetNFBlockList();
	if( kContNFBlock.empty() )
	{
		if( FALSE == theNFDBMgr.SelectNFFriendNickByStatus( lCSN, kContNFBlock, FR_BLOCK ) )
		{
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}

		pUser->SetNFBlock(kContNFBlock);
	}

	if( FOT_BLOCK == req->m_lOrderType) // ����
	{
		// �̹� ��������?
		CONT_NF_FRIEND_NICK::const_iterator block_iter = std::find(kContNFBlock.begin(), kContNFBlock.end(), strCharName);
		if( block_iter != kContNFBlock.end() )
		{
			ans.m_lErrorCode = NF::EC_FE_ALREADY_BLOCK;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFBlockOrUnBlock_Tag, ans);	
			SendToUser(lCSN, pld);
			return;
		}

		if( FALSE == theNFDBMgr.InsertNFFriend( lCSN, key.m_lSubKey, FR_BLOCK ) )
		{
			ans.m_lErrorCode = NF::EC_FE_ALREADY_BLOCK;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}
		else
		{
			pUser->AddNFBlock(strCharName);
		}
	}
	else if( FOT_UNBLOCK == req->m_lOrderType ) //���� ����
	{
		if( FALSE == theNFDBMgr.DeleteNFFriend( pUser->GetCharName(), strCharName, FR_BLOCK ) )
		{
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
			PayloadNGSCli pld(PayloadNGSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
			SendToUser(lCSN, pld);
			return;
		}
		else
		{
			pUser->DeleteNFBlock(strCharName);
		}
	}

	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	ans.m_lOrderType = req->m_lOrderType;
	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFBlockOrUnBlock_Tag, ans);
	SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqNFFriendDelete(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNFFriendDelete* req = pMsg->un.m_msgReqNFFriendDelete;

	CUser* pUser = UserManager->FindUser(lCSN);
	if( pUser )
	{
		MsgNGSCli_AnsNFFriendDelete ans;
		ans.m_lErrorCode = NF::EC_FE_SUCCESS;

		if( !DeleteNFFriend(pUser->GetCharName(), req->m_strCharName ) )
		{
			ans.m_lErrorCode = NF::EC_FE_DB_ERROR;
		}
		else
		{
			pUser->DeleteNFFriend( req->m_strCharName );
		}

		PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendDelete_Tag, ans);
		UserManager->SendToUser(lCSN, pld);
	}
}

void CRoomInternalLogic::OnReqUpdateStudyScenario(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgNGSCli_AnsUpdateStudyScenario	ans;
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	ans.m_strSutdyScenarioDate = G_INVALID_DATE;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (pUser->GetNFCharInfoExt()->m_nfCharBaseInfo.m_strStudyDate == G_INVALID_DATE)
		{
			// ��¥ ������Ʈ
			SYSTEMTIME systime;
			::GetLocalTime(&systime);
			string strDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

			if (theNFDBMgr.UpdateStudyScenarioDate(pUser->GetGSN(), pUser->GetCSN(), strDate))
			{
				pUser->GetNFCharInfoExt()->m_nfCharBaseInfo.m_strStudyDate = strDate;
				ans.m_strSutdyScenarioDate = strDate;
			}
			else 
				ans.m_lErrorCode = -2;		// StudyScenario DB Failed
		}
		else
			ans.m_lErrorCode = -1;

		PayloadNGSCli pld(PayloadNGSCli::msgAnsUpdateStudyScenario_Tag, ans);
		UserManager->SendToUser(lCSN, pld);
	}
}

// ����-����
void CRoomInternalLogic::OnReqRepairEnduranceItem(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqRepairEnduranceItem* req = pMsg->un.m_msgReqRepairEnduranceItem;

	MsgNGSCli_AnsRepairEnduranceItem	ans;
	ans.m_ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.RepairEnduranceItem(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt(), req->m_req, ans.m_ans))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqRepairEnduranceItem() Failed. CSN: ", pUser->GetCSN(), " // ErrCode : ", ans.m_ans.m_lErrorCode);

		PayloadNGSCli pld(PayloadNGSCli::msgAnsRepairEnduranceItem_Tag, ans);
		UserManager->SendToUser(lCSN, pld);
	}
}

void CRoomInternalLogic::OnReqNextEnchantInfo(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqNextEnchantInfo* req = pMsg->un.m_msgReqNextEnchantInfo;

	MsgNGSCli_AnsNextEnchantInfo ans;
	ans.Clear();
	ans.m_ansNext.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.NextEnchantInfo(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt(), req->m_reqNext, ans.m_ansNext))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqNextEnchantInfo() Failed. CSN: ", pUser->GetCSN(), " // ErrCode : ", ans.m_ansNext.m_lErrorCode);
		
		PayloadNGSCli pld(PayloadNGSCli::msgAnsNextEnchantInfo_Tag, ans);
		UserManager->SendToUser(lCSN, pld);
	}
}

void CRoomInternalLogic::OnReqItemEnchant(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqItemEnchant* req = pMsg->un.m_msgReqItemEnchant;

	MsgNGSCli_AnsItemEnchant ans;
	ans.m_ansEnchant.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.ItemEnchant(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt(), req->m_reqEnchant, ans.m_ansEnchant))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqItemEnchant() Failed. CSN: ", pUser->GetCSN(), " // ErrCode : ", ans.m_ansEnchant.m_lErrorCode);

		PayloadNGSCli pld(PayloadNGSCli::msgAnsItemEnchant_Tag, ans);
		UserManager->SendToUser(lCSN, pld);
	}
}

void CRoomInternalLogic::OnReqAquaFish(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgNGSCli_AnsAquaFish ans;
	ans.Clear();
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.GetAquaFish(lCSN, ans.m_mapNFAquaFish))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqAquaFish() Failed. CSN: ", lCSN);
	}
	else
	{
		ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsAquaFish_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqAquaInsert(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgNGSCli_AnsAquaInsert ans;
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		CNFChar nfChar = pUser->GetNFUser();
		CFish fish = nfChar.GetBiteFish();
		FishInfo fishInfo = fish.GetFish();
		BiteFishInfo biteFishInfo;
		fish.GetBiteFishInfo(biteFishInfo);
		
		LONG lErrorCode = NF::G_NF_ERR_SUCCESS;
		if (!theNFMenu.InsertAquaFish(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt(), fishInfo.m_lIndex, biteFishInfo.m_dFishLength, biteFishInfo.m_dFishWeight, 0, biteFishInfo.m_lFishScore, lErrorCode))
		{
			ans.m_lErrorCode = lErrorCode;
		}
	}
	else
	{
		ans.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
	}

	ans.m_lTotalScore = pUser->GetNFCharInfoExt()->m_nfAqua.m_lAquaScore;

	PayloadNGSCli pld(PayloadNGSCli::msgAnsAquaInsert_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

