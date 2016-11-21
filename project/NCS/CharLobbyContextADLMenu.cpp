
#include "stdafx.h"
#include "CharLobby.h"
#include "CharLobbyContext.h"
#include "User.h"
#include <NFVariant/NFGameData.h>
#include <NFVariant/NFMenu.h>

// Menu
void CCharLobbyContext::OnReqProductList(CUser* pUser, MsgCliNCS_ReqProductList* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsProductList	ans;
	theNFMenu.GetProductList(ans.m_ansProductList);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsProductList_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqAchvInfo(CUser* pUser, MsgCliNCS_ReqAchvInfo* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsAchvInfo			ans;
	ans.m_ans.Clear();

	if (!theNFMenu.AnsAchv(pUser->GetNFCharInfoExt(), pMsg->m_lstAchvID, ans.m_ans))
		theLog.Put(ERR, "OnReqAchvInfo Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ans.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsAchvInfo_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqChangeParts(CUser* pUser, MsgCliNCS_ReqChangeParts* pMsg)
{
	if (NULL == pMsg) return;

	// Level에 따라 착용 가능 여부 추가 - 2010/12/24
	MsgNCSCli_AnsChangeParts	ans;
	ans.Clear();	
	ans.m_ansChangeParts.m_lErrorCode = NF::EC_ECR_SUCCESS;				// 성공

	BOOL bDebuffRet = FALSE;		// ChangeParts 함수안에서 변경이 안 될 경우를 대비해서 미리 저장해놓는다...

	RoomID roomID;
	roomID.Clear();

	if (!theNFMenu.ChangeParts(pUser->GetNFCharInfoExt(), pUser->GetGSN(), roomID, pMsg->m_reqChangeParts, ans.m_ansChangeParts, -1, bDebuffRet))
		theLog.Put(ERR, "OnReqChangeParts Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansChangeParts.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsChangeParts_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqBuyItem(CUser* pUser, MsgCliNCS_ReqBuyItem* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsBuyItem	ans;
	ans.m_ansBuyItem.m_llGameMoney = 0;
	ans.m_ansBuyItem.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	std::map<LONG/*ItemID*/, Product> mapProduct;

	LONGLONG llTotPrice = 0;
	if (theNFMenu.CheckBuyMoney(pMsg->m_reqBuyItem, mapProduct, llTotPrice, ans.m_ansBuyItem.m_lErrorCode))
	{
		if (!theNFMenu.BuyItem(pUser->GetNFCharInfoExt(), pUser->GetGSN(), pMsg->m_reqBuyItem, mapProduct, ans.m_ansBuyItem))
			theLog.Put(ERR, "ReqBuyItem Failed!!! CSN : ", pUser->GetCSN(), "// Money  :", ans.m_ansBuyItem.m_llGameMoney);
	}
	else
        theLog.Put(ERR, "CheckBuyMoney Failed!!! CSN : ", pUser->GetCSN(), "// Client_tot_Price :", pMsg->m_reqBuyItem.m_llReqBuyMoney, "// Server_tot_Price :", llTotPrice);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsBuyItem_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqRemoveItem(CUser* pUser, MsgCliNCS_ReqRemoveItem* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsRemoveItem ans;
	ans.m_ansRemoveItem.m_lErrorCode = NF::EC_ECR_SUCCESS;
	ans.m_ansRemoveItem.m_removeInvenSlot.Clear();

	if (!theNFMenu.RemoveItem(pUser->GetNFCharInfoExt(), pUser->GetGSN(), pMsg->m_reqRemoveItem, ans.m_ansRemoveItem))
		theLog.Put(ERR, "RemoveItem Failed!!! CSN : ", pUser->GetCSN(), "// InvenSRL  :", pMsg->m_reqRemoveItem.m_lInvenSRL, "// ErrorCode :", ans.m_ansRemoveItem.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsRemoveItem_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqOpenCardPack(CUser* pUser, MsgCliNCS_ReqOpenCardPack* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsOpenCardPack	ans;
	ans.m_ansOpenCardPack.m_lErrorCode = NF::EC_ECR_SUCCESS;

	if (!theNFMenu.OpenCardPack(pUser->GetNFCharInfoExt(), pUser->GetGSN(), pMsg->m_lInvenSRL, ans.m_ansOpenCardPack))
		theLog.Put(ERR, "OnReqOpenCardPack Failed!!! CSN : ", pUser->GetCSN(), "// InvenSRL  :", pMsg->m_lInvenSRL, "// ErrorCode :", ans.m_ansOpenCardPack.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsOpenCardPack_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqChangeCardSlot(CUser* pUser, MsgCliNCS_ReqChangeCardSlot* pMsg)
{	
	if (NULL == pMsg) return;

	MsgNCSCli_AnsChangeCardSlot	ans;
	ans.m_ansChangeCardSlot.m_lErrorCode = NF::EC_ECR_SUCCESS;
	ans.m_ansChangeCardSlot.m_lChangeType = pMsg->m_reqChangeCardSlot.m_lChangeType;
	ans.m_ansChangeCardSlot.m_addToInvenSlot.Clear();
	ans.m_ansChangeCardSlot.m_removeFromInvenSlot.Clear();
	ans.m_ansChangeCardSlot.m_llReduceGameMoney = 0;
	ans.m_ansChangeCardSlot.m_nfAbility.Clear();
	ans.m_ansChangeCardSlot.m_bIsGameMoveUse = pMsg->m_reqChangeCardSlot.m_bIsUsingGameMoney;

	NFAbilityExt	nfAbilityExt;
	nfAbilityExt.Clear();

	if (!theNFMenu.ChangeCardSlot(pUser->GetNFCharInfoExt(), nfAbilityExt, pUser->GetGSN(), pMsg->m_reqChangeCardSlot, ans.m_ansChangeCardSlot))
		theLog.Put(ERR, "OnReqChangeCardSlot Failed!!! CSN : ", pUser->GetCSN(), "// InvenSRL  :", pMsg->m_reqChangeCardSlot.m_lInvenSRL, "// ErrorCode :", ans.m_ansChangeCardSlot.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsChangeCardSlot_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqExchangeCards(CUser* pUser, MsgCliNCS_ReqExchangeCards* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsExchangeCards	ans;
	ans.m_ansExchangeCards.m_exchangeCards.m_lUpgradeType = pMsg->m_exchangeCards.m_lUpgradeType;
	ans.m_ansExchangeCards.m_exchangeCards.m_bIsSpecialCard = pMsg->m_exchangeCards.m_bIsSpecialCard;
	ans.m_ansExchangeCards.m_exchangeCards.m_vecExchangeCard = pMsg->m_exchangeCards.m_vecExchangeCard;
	ans.m_ansExchangeCards.m_exchangeCards.m_lUpdateCardInvenSRL = pMsg->m_exchangeCards.m_lUpdateCardInvenSRL;
	ans.m_ansExchangeCards.m_lErrorCode = NF::EC_ECR_SUCCESS;
	ans.m_ansExchangeCards.m_newCardPack.Clear();

	if (!theNFMenu.ExchangeCard(pUser->GetNFCharInfoExt(), pUser->GetGSN(), pMsg->m_exchangeCards, ans.m_ansExchangeCards))
		theLog.Put(ERR, "ReqExchangeCards Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansExchangeCards.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsExchangeCards_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqChangeQuickSlot(CUser* pUser, MsgCliNCS_ReqChangeQuickSlot* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsChangeQuickSlot ans;
	ans.m_ansChangeQuickSlot.m_lChangeType = pMsg->m_reqChangeQuickSlot.m_lChangeType;

	if (!theNFMenu.ChangeQuickSlot(pUser->GetNFCharInfoExt(), pUser->GetGSN(), pMsg->m_reqChangeQuickSlot, ans.m_ansChangeQuickSlot))
		theLog.Put(ERR, "ChangeQuickSlot Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansChangeQuickSlot.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsChangeQuickSlot_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqRepairEnduranceItem(CUser* pUser, MsgCliNCS_ReqRepairEnduranceItem* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsRepairEnduranceItem ans;
	ans.m_ansItem.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!theNFMenu.RepairEnduranceItem(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt(), pMsg->m_reqItem, ans.m_ansItem))
		theLog.Put(ERR, "RepairEnduranceItem Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansItem.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsRepairEnduranceItem_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

// 다음 인첸트 정보
void CCharLobbyContext::OnReqNextEnchantInfo(CUser* pUser, MsgCliNCS_ReqNextEnchantInfo* pMsg)
{
	if (NULL == pMsg) 
		return;

	MsgNCSCli_AnsNextEnchantInfo ans;
	ans.Clear();
	ans.m_ansNext.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!theNFMenu.NextEnchantInfo(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt(), pMsg->m_reqNext, ans.m_ansNext))
		theLog.Put(ERR, "NextEnchantInfo Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansNext.m_lErrorCode);	

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNextEnchantInfo_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

// 아이템 강화 요청
void CCharLobbyContext::OnReqItemEnchant(CUser* pUser, MsgCliNCS_ReqItemEnchant* pMsg)
{
	if (NULL == pMsg)
		return;
	
	MsgNCSCli_AnsItemEnchant ans;
	ans.m_ansEnchant.Clear();
	ans.m_ansEnchant.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!theNFMenu.ItemEnchant(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt(), pMsg->m_reqEnchant, ans.m_ansEnchant))
		theLog.Put(ERR, "ItemEnchant Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansEnchant.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsItemEnchant_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqAquaFish(CUser* pUser, MsgCliNCS_ReqAquaFish* pMsg)
{
	if (NULL == pMsg)
		return;

	MsgNCSCli_AnsAquaFish ans;
	ans.Clear();
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!theNFMenu.GetAquaFish(pUser->GetCSN(), ans.m_mapNFAquaFish))
	{
		ans.m_lErrorCode = NF::G_NF_ERR_FAIL;
		theLog.Put(ERR, "GetAquaFish Failed!!! CSN : ", pUser->GetCSN());
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsAquaFish_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqRewardItem(CUser* pUser, MsgCliNCS_ReqRewardItem* pMsg)
{
	if (NULL == pMsg)
		return;

	MsgNCSCli_AnsRewardItem ans;
	ans.m_ansRewardItem.Clear();
	ans.m_ansRewardItem.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	RoomID roomID;
	roomID.Clear();

	if (!theNFMenu.GetRewardItem(pUser->GetGSN(), pUser->GetNFCharInfoExt(), roomID, pMsg->m_reqRewardItem, ans.m_ansRewardItem))
		theLog.Put(ERR, "GetRewardItem Failed!!! CSN : ", pUser->GetCSN(), " / ErrorCode :", ans.m_ansRewardItem.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsRewardItem_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqLockedNote(CUser* pUser, MsgCliNCS_ReqLockedNote* pMsg)
{
	if (NULL == pMsg)
		return;

	if (1 == pMsg->m_reqLockedNote.m_nReqLockedNoteType) 
	{
		MsgNCSCli_AnsLockedNoteMain ans;
		ans.m_ansLockedNoteMain.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

		if (theNFMenu.GetLockedNoteMain(pUser->GetGSN(), pUser->GetCSN(), ans.m_ansLockedNoteMain))
			pUser->GetNFCharInfoExt()->m_nfLockedNote.m_nfLockedNoteMain = ans.m_ansLockedNoteMain.m_lockedNoteMain;

		PayloadNCSCli pld(PayloadNCSCli::msgAnsLockedNoteMain_Tag, ans);
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
	}
	else if (2 == pMsg->m_reqLockedNote.m_nReqLockedNoteType)
	{
		MsgNCSCli_AnsLockedNoteMap ans;
		ans.m_ansLockedNoteMap.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

		if (theNFMenu.GetLockedNoteMap(pUser->GetGSN(), pUser->GetCSN(), pMsg->m_reqLockedNote.m_lMapID, pUser->GetNFCharInfoExt()->m_nfLockedNote.m_nfLockedNoteMap, ans.m_ansLockedNoteMap))
			pUser->GetNFCharInfoExt()->m_nfLockedNote.m_nfLockedNoteMap[ans.m_ansLockedNoteMap.m_lMapID] = ans.m_ansLockedNoteMap.m_lockedNoteMap;

		PayloadNCSCli pld(PayloadNCSCli::msgAnsLockedNoteMap_Tag, ans);
		m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
	}
}