
#include "stdafx.h"
#include "RoomInternalLogic.h"
#include <NF/ADL/MsgNFCommonStruct.h>
#include <NFVariant/NFDBManager.h>
#include <NFVariant/NFGameData.h>
#include <NFVariant/NFMenu.h>

// NFMenu Handler Start
// CHS와 공통, 로직이 변할 경우 CHS도 함께 수정해야 함
void CRoomInternalLogic::OnReqAchievement(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgNGSCli_AnsAchvInfo			ans;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.AnsAchv(pUser->GetNFCharInfoExt(), pMsg->un.m_msgReqAchvInfo->m_lstAchvID, ans.m_ansAchv))
		{
			theLog.Put(ERR_UK, "NGS_LOGIC, AnsAchv(). AnsAchv!!! Failed CSN : ", pUser->GetCSN(), ", ErrorCode :", ans.m_ansAchv.m_lErrorCode);
		}

		PayloadNGSCli pld(PayloadNGSCli::msgAnsAchvInfo_Tag, ans);
		UserManager->SendToUser(pUser->GetCSN(), pld);
	}
}

void CRoomInternalLogic::OnReqBuyItem(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqBuyItem* req = pMsg->un.m_msgReqBuyItem;
	MsgNGSCli_AnsBuyItem	ans;
	ans.m_ansBuyItem.m_llGameMoney = 0;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		std::map<LONG/*ItemID*/, Product> mapProduct;

		LONGLONG llTotPrice = 0;
		if (theNFMenu.CheckBuyMoney(req->m_reqBuyItem, mapProduct, llTotPrice, ans.m_ansBuyItem.m_lErrorCode))
		{
			if (!theNFMenu.BuyItem(pUser->GetNFCharInfoExt(), pUser->GetGSN(), req->m_reqBuyItem, mapProduct, ans.m_ansBuyItem))
				theLog.Put(ERR_UK, "NGS_LOGIC, ReqBuyItem Failed!!! CSN : ", pUser->GetCSN(), "// Money  :", ans.m_ansBuyItem.m_llGameMoney);
		}
		else
			theLog.Put(ERR_UK, "NGS_LOGIC, CheckBuyMoney Failed!!! CSN : ", pUser->GetCSN(), "// Client_tot_Price :", req->m_reqBuyItem.m_llReqBuyMoney, "// Server_tot_Price :", llTotPrice);
	}
	else
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqBuyItem(). Not Found User CSN : ", pUser->GetCSN());

	PayloadNGSCli pld(PayloadNGSCli::msgAnsBuyItem_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// CHS와 공통, 로직이 변할 경우 CHS도 함께 수정해야 함
void CRoomInternalLogic::OnReqRemoveItem(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqRemoveItem* req = pMsg->un.m_msgReqRemoveItem;
	MsgNGSCli_AnsRemoveItem ans;
	ans.m_ansRemoveItem.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	ans.m_ansRemoveItem.m_removeInvenSlot.Clear();

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.RemoveItem(pUser->GetNFCharInfoExt(), pUser->GetGSN(), req->m_reqRemoveItem, ans.m_ansRemoveItem))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqRemoveItem(). ReqRemoveItem!!! Failed CSN : ", pUser->GetCSN(), ", InvenSRL  :", req->m_reqRemoveItem.m_lInvenSRL, "ErrorCode :", ans.m_ansRemoveItem.m_lErrorCode);
	}
	else {
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqRemoveItem(). Not Found User CSN : ", pUser->GetCSN());
		ans.m_ansRemoveItem.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsRemoveItem_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// CHS와 공통, 로직이 변할 경우 CHS도 함께 수정해야 함
void CRoomInternalLogic::OnReqOpenCardPack(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqOpenCardPack* req = pMsg->un.m_msgReqOpenCardPack;
	MsgNGSCli_AnsOpenCardPack	ans;
	ans.m_ansOpenCardPack.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.OpenCardPack(pUser->GetNFCharInfoExt(), pUser->GetGSN(), req->m_lInvenSRL, ans.m_ansOpenCardPack))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqOpenCardPack(). ReqOpenCardPack!!! Failed CSN : ", pUser->GetCSN(), ", lItemID  :", req->m_lInvenSRL, "ErrorCode :", ans.m_ansOpenCardPack.m_lErrorCode);
	}
	else {
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqOpenCardPack(). Not Found User CSN : ", pUser->GetCSN());
		ans.m_ansOpenCardPack.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsOpenCardPack_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqProductList(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgNGSCli_AnsProductList	ans;

	theNFMenu.GetProductList(ans.m_ansProductList);

	PayloadNGSCli pld(PayloadNGSCli::msgAnsProductList_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// CHS와 공통, 로직이 변할 경우 CHS도 함께 수정해야 함
void CRoomInternalLogic::OnReqChangeParts(LONG lCSN, PayloadCliNGS* pMsg)
{
	// Level에 따라 착용 가능 여부 추가 - 2010/12/24
	MsgCliNGS_ReqChangeParts* req = pMsg->un.m_msgReqChangeParts;
	MsgNGSCli_AnsChangeParts	ans;
	ans.Clear();
	ans.m_ansChangeParts.m_lErrorCode = 0;				// 성공

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		BOOL bDebuffRet = pUser->GetEnvDebuff();		// ChangeParts 함수안에서 변경이 안 될 경우를 대비해서 미리 저장해놓는다...

		if (!theNFMenu.ChangeParts(pUser->GetNFCharInfoExt(), pUser->GetGSN(), GetRoomID(), req->m_reqChangeParts, ans.m_ansChangeParts, m_nfRoomOption.m_lEnvAttribute, bDebuffRet))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqChangeParts(). ReqChangeParts!!! Failed CSN : ", pUser->GetCSN(), "ErrorCode :", ans.m_ansChangeParts.m_lErrorCode);

		pUser->SetEnvDebuff(bDebuffRet);
	}
	else {
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqChangeParts(). Not Found User CSN : ", pUser->GetCSN());
		ans.m_ansChangeParts.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsChangeParts_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// CHS와 공통, 로직이 변할 경우 CHS도 함께 수정해야 함
void CRoomInternalLogic::OnReqChangeCardSlot(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqChangeCardSlot* req = pMsg->un.m_msgReqChangeCardSlot;
	MsgNGSCli_AnsChangeCardSlot	ans;
	ans.m_ansChangeCardSlot.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	ans.m_ansChangeCardSlot.m_lChangeType = req->m_reqChangeCardSlot.m_lChangeType;
	ans.m_ansChangeCardSlot.m_addToInvenSlot.Clear();
	ans.m_ansChangeCardSlot.m_removeFromInvenSlot.Clear();
	ans.m_ansChangeCardSlot.m_llReduceGameMoney = 0;
	ans.m_ansChangeCardSlot.m_nfAbility.Clear();
	ans.m_ansChangeCardSlot.m_bIsGameMoveUse = req->m_reqChangeCardSlot.m_bIsUsingGameMoney;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.ChangeCardSlot(pUser->GetNFCharInfoExt(), pUser->GetNFUser().GetAbilityExt(), pUser->GetGSN(), req->m_reqChangeCardSlot, ans.m_ansChangeCardSlot))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqChangeCardSlot(). ReqChangeCardSlot!!! Failed CSN : ", pUser->GetCSN(), "ErrorCode :", ans.m_ansChangeCardSlot.m_lErrorCode);
	}
	else {
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqChangeCardSlot(). Not Found User CSN : ", pUser->GetCSN());
		ans.m_ansChangeCardSlot.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsChangeCardSlot_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// CHS와 공통, 로직이 변할 경우 CHS도 함께 수정해야 함
//LONG  lIsUpgradeType;						// 1:동급카드로업그레이드, 2:상위카드로업그레이드, 3:강화카드로교환
//BOOL	bIsSpecialCard;						// 희귀카드(TRUE)인지 일반카드(FALSE)인지?
//BOOL	
//std::list<LONG> lstExchangeCardInvenSRL;	// 교환하려는 카드의 InvenSRL
//LONG	lUpdateCardInvenSRL;				// 희귀카드 일 경우 사용되는 강화카드의 InvenSRL;
void CRoomInternalLogic::OnReqExchangeCards(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqExchangeCards* req = pMsg->un.m_msgReqExchangeCards;
	MsgNGSCli_AnsExchangeCards	ans;
	ans.m_ansExchangeCards.m_exchangeCards.m_lUpgradeType = req->m_exchangeCards.m_lUpgradeType;
	ans.m_ansExchangeCards.m_exchangeCards.m_bIsSpecialCard = req->m_exchangeCards.m_bIsSpecialCard;
	ans.m_ansExchangeCards.m_exchangeCards.m_vecExchangeCard = req->m_exchangeCards.m_vecExchangeCard;
	ans.m_ansExchangeCards.m_exchangeCards.m_lUpdateCardInvenSRL = req->m_exchangeCards.m_lUpdateCardInvenSRL;
	ans.m_ansExchangeCards.m_lErrorCode = NF::EC_ECR_SUCCESS;
	ans.m_ansExchangeCards.m_newCardPack.Clear();

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.ExchangeCard(pUser->GetNFCharInfoExt(), pUser->GetGSN(), req->m_exchangeCards, ans.m_ansExchangeCards))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqExchangeCards(). ReqExchangeCards!!! Failed CSN : ", pUser->GetCSN(), "ErrorCode :", ans.m_ansExchangeCards.m_lErrorCode);
	}
	else {
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqExchangeCards(). Not Found User CSN : ", pUser->GetCSN());
		ans.m_ansExchangeCards.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsExchangeCards_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

// CHS와 공통, 로직이 변할 경우 CHS도 함께 수정해야 함
void CRoomInternalLogic::OnReqChangeQuickSlot(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqChangeQuickSlot* req = pMsg->un.m_msgReqChangeQuickSlot;
	MsgNGSCli_AnsChangeQuickSlot ans;
	ans.m_ansChangeQuickSlot.m_lChangeType = req->m_reqChangeQuickSlot.m_lChangeType;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.ChangeQuickSlot(pUser->GetNFCharInfoExt(), pUser->GetGSN(), req->m_reqChangeQuickSlot, ans.m_ansChangeQuickSlot))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqChangeQuickSlot(). ChangeQuickSlot!!! Failed CSN : ", pUser->GetCSN(), "ErrorCode :", ans.m_ansChangeQuickSlot.m_lErrorCode);
	}
	else {
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqChangeQuickSlot(). Not Found User CSN : ", pUser->GetCSN());
		ans.m_ansChangeQuickSlot.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsChangeQuickSlot_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

void CRoomInternalLogic::OnReqGetRewardItem(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqGetRewardItem* req = pMsg->un.m_msgReqGetRewardItem;
	MsgNGSCli_AnsGetRewardItem ans;
	ans.m_ansRewardItem.Clear();

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (!theNFMenu.GetRewardItem(pUser->GetGSN(), pUser->GetNFCharInfoExt(), GetRoomID(), req->m_reqRewardItem, ans.m_ansRewardItem))
			theLog.Put(ERR_UK, "NGS_LOGIC, OnReqGetRewardItem(). GetRewardItem Failed!!! CSN : ", pUser->GetCSN());
	}
	else {
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqGetRewardItem(). Not Found User CSN : ", pUser->GetCSN());
		ans.m_ansRewardItem.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsGetRewardItem_Tag, ans);
	UserManager->SendToUser(lCSN, pld);
}

//
void CRoomInternalLogic::OnReqLandNote(LONG lCSN, PayloadCliNGS* pMsg)
{
	MsgCliNGS_ReqLockedNote* req = pMsg->un.m_msgReqLockedNote;

	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
		if (1 == req->m_reqLockedNote.m_nReqLockedNoteType) 
		{
			MsgNGSCli_AnsLockedNoteMain ans;
			ans.m_ansLockedNoteMain.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

			if (theNFMenu.GetLockedNoteMain(pUser->GetGSN(), pUser->GetCSN(), ans.m_ansLockedNoteMain))
				pUser->GetNFCharInfoExt()->m_nfLockedNote.m_nfLockedNoteMain = ans.m_ansLockedNoteMain.m_lockedNoteMain;

			PayloadNGSCli pld(PayloadNGSCli::msgAnsLockedNoteMain_Tag, ans);
			UserManager->SendToUser(lCSN, pld);
		}
		else if (2 == req->m_reqLockedNote.m_nReqLockedNoteType)
		{
			MsgNGSCli_AnsLockedNoteMap ans;
			ans.m_ansLockedNoteMap.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

			if (theNFMenu.GetLockedNoteMap(pUser->GetGSN(), pUser->GetCSN(), req->m_reqLockedNote.m_lMapID, pUser->GetNFCharInfoExt()->m_nfLockedNote.m_nfLockedNoteMap, ans.m_ansLockedNoteMap))
				pUser->GetNFCharInfoExt()->m_nfLockedNote.m_nfLockedNoteMap[ans.m_ansLockedNoteMap.m_lMapID] = ans.m_ansLockedNoteMap.m_lockedNoteMap;

			PayloadNGSCli pld(PayloadNGSCli::msgAnsLockedNoteMap_Tag, ans);
			UserManager->SendToUser(lCSN, pld);
		}
	}
	else
		theLog.Put(ERR_UK, "NGS_LOGIC, OnReqLandNote(). Not Found User CSN : ", pUser->GetCSN());
}

// NFMenu Handler End