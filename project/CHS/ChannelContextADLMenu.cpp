
#include "stdafx.h"
#include <NFVariant/NFMenu.h>


void CChannelContext::OnReqChangeParts(MsgCliCHS_ReqChangeParts* pMsg, CUser & user)
{
	MsgCHSCli_AnsChangeParts	ans;
	ans.Clear();
	ans.m_ansChangeParts.m_lErrorCode = 0;				// 성공

	BOOL bBuffCheck = FALSE;

	RoomID roomID;
	roomID.Clear();

	if (!theNFMenu.ChangeParts(user.GetNFCharInfoExt(), user.GetGSN(), roomID, pMsg->m_reqChangeParts, ans.m_ansChangeParts, -1, bBuffCheck))
		theLog.Put(ERR_UK, "CHS_CChannelContext"_LK, "OnReqChangeParts Failed!!! CSN :", user.GetCSN(), " // ErrorCode :", ans.m_ansChangeParts.m_lErrorCode);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsChangeParts_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqChangeCardSlot(MsgCliCHS_ReqChangeCardSlot* pMsg, CUser & user)
{
	MsgCHSCli_AnsChangeCardSlot	ans;
	ans.m_ansChangeCardSlot.m_lErrorCode = 1;
	ans.m_ansChangeCardSlot.m_lChangeType = pMsg->m_reqChangeCardSlot.m_lChangeType;
	ans.m_ansChangeCardSlot.m_addToInvenSlot.Clear();
	ans.m_ansChangeCardSlot.m_removeFromInvenSlot.Clear();
	ans.m_ansChangeCardSlot.m_llReduceGameMoney = 0;
	ans.m_ansChangeCardSlot.m_nfAbility.Clear();
	ans.m_ansChangeCardSlot.m_bIsGameMoveUse = pMsg->m_reqChangeCardSlot.m_bIsUsingGameMoney;

	if (!theNFMenu.ChangeCardSlot(user.GetNFCharInfoExt(), user.GetAbilityExt(), user.GetGSN(), pMsg->m_reqChangeCardSlot, ans.m_ansChangeCardSlot))
		theLog.Put(ERR_UK, "CHS_CChannelContext"_LK, "OnReqChangeCardSlot Failed!!! CSN :", user.GetCSN(), " // ErrorCode :", ans.m_ansChangeCardSlot.m_lErrorCode);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsChangeCardSlot_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

//LONG  lIsUpgradeType;						// 1:동급카드로업그레이드, 2:상위카드로업그레이드, 3:강화카드로교환
//BOOL	bIsSpecialCard;						// 희귀카드(TRUE)인지 일반카드(FALSE)인지?
//BOOL	
//std::list<LONG> lstExchangeCardInvenSRL;	// 교환하려는 카드의 InvenSRL
//LONG	lUpdateCardInvenSRL;					// 희귀카드 일 경우 사용되는 강화카드의 InvenSRL;
void CChannelContext::OnReqExchangeCards(MsgCliCHS_ReqExchangeCards* pMsg, CUser & user)
{
	MsgCHSCli_AnsExchangeCards	ans;
	ans.m_ansExchangeCards.m_exchangeCards.m_lUpgradeType = pMsg->m_exchangeCards.m_lUpgradeType;
	ans.m_ansExchangeCards.m_exchangeCards.m_bIsSpecialCard = pMsg->m_exchangeCards.m_bIsSpecialCard;
	ans.m_ansExchangeCards.m_exchangeCards.m_vecExchangeCard = pMsg->m_exchangeCards.m_vecExchangeCard;
	ans.m_ansExchangeCards.m_exchangeCards.m_lUpdateCardInvenSRL = pMsg->m_exchangeCards.m_lUpdateCardInvenSRL;
	ans.m_ansExchangeCards.m_lErrorCode = NF::EC_ECR_SUCCESS;
	ans.m_ansExchangeCards.m_newCardPack.Clear();

	if (!theNFMenu.ExchangeCard(user.GetNFCharInfoExt(), user.GetGSN(), pMsg->m_exchangeCards, ans.m_ansExchangeCards))
		theLog.Put(ERR_UK, "CHS_CChannelContext"_LK, "OnReqExchangeCards Failed!!! CSN :", user.GetCSN(), " // ErrorCode :", ans.m_ansExchangeCards.m_lErrorCode);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsExchageCards_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqBuyItem(MsgCliCHS_ReqBuyItem* pMsg, CUser & user)
{
	MsgCHSCli_AnsBuyItem ans;
	ans.m_ansBuyItem.m_llGameMoney = 0;

	std::map<LONG/*ItemID*/, Product> mapProduct;

	LONGLONG llTotPrice = 0;
	if (theNFMenu.CheckBuyMoney(pMsg->m_reqBuyItem, mapProduct, llTotPrice, ans.m_ansBuyItem.m_lErrorCode))
	{
		if (!theNFMenu.BuyItem(user.GetNFCharInfoExt(), user.GetGSN(), pMsg->m_reqBuyItem, mapProduct, ans.m_ansBuyItem))
			LOG(ERR_UK, "CHS_CChannelContext"_LK, "ReqBuyItem Failed!!! CSN : ", user.GetCSN(), "// Money  :", ans.m_ansBuyItem.m_llGameMoney);
	}
	else
		LOG(ERR_UK, "CHS_CChannelContext"_LK, "CheckBuyMoney Failed!!! CSN : ", user.GetCSN(), "// Client_tot_Price :", pMsg->m_reqBuyItem.m_llReqBuyMoney, "// Server_tot_Price :", llTotPrice);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsBuyItem_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqRemoveItem(MsgCliCHS_ReqRemoveItem* pMsg, CUser & user)
{
	MsgCHSCli_AnsRemoveItem ans;
	ans.m_ansRemoveItem.m_lErrorCode = 1;
	ans.m_ansRemoveItem.m_removeInvenSlot.Clear();

	if (!theNFMenu.RemoveItem(user.GetNFCharInfoExt(), user.GetGSN(), pMsg->m_reqRemoveItem, ans.m_ansRemoveItem))
		LOG(INF_UK, "CHS_CChannelContext"_LK, "OnReqRemoveItem Failed!!! CSN :", user.GetCSN(), " // ErrorCode :", ans.m_ansRemoveItem.m_lErrorCode);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsRemoveItem_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqOpenCardPack(MsgCliCHS_ReqOpenCardPack* pMsg, CUser & user)
{
	MsgCHSCli_AnsOpenCardPack	ans;
	ans.m_ansOpenCardPack.m_lErrorCode = 1;

	if (!theNFMenu.OpenCardPack(user.GetNFCharInfoExt(), user.GetGSN(), pMsg->m_lInvenSRL, ans.m_ansOpenCardPack))
		LOG(INF_UK, "CHS_CChannelContext"_LK, "OnReqOpenCardPack Failed!!! CSN :", user.GetCSN(), " // ErrorCode :", ans.m_ansOpenCardPack.m_lErrorCode);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsOpenCardPack_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqChangeQuickSlot(MsgCliCHS_ReqChangeQuickSlot* pMsg, CUser & user)
{
	MsgCHSCli_AnsChangeQuickSlot ans;
	ans.m_ansChangeQuickSlot.m_lChangeType = pMsg->m_reqChangeQuickSlot.m_lChangeType;

	if (!theNFMenu.ChangeQuickSlot(user.GetNFCharInfoExt(), user.GetGSN(), pMsg->m_reqChangeQuickSlot, ans.m_ansChangeQuickSlot))
		theLog.Put(ERR_UK, "CHS_CChannelContext"_LK, "OnReqChangeQuickSlot Failed!!! CSN :", user.GetCSN(), " // ErrorCode :", ans.m_ansChangeQuickSlot.m_lErrorCode);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsChangeQuickSlot_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqNextEnchantInfo(MsgCliCHS_ReqNextEnchantInfo* pReq, CUser& user)
{
	MsgCHSCli_AnsNextEnchantInfo ans;
	ans.Clear();
	ans.m_ansNext.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!theNFMenu.NextEnchantInfo(user.GetGSN(), user.GetCSN(), user.GetNFCharInfoExt(), pReq->m_reqNext, ans.m_ansNext))
		theLog.Put(ERR_UK, "CHS_LOGIC, OnReqNextEnchantInfo() Failed. CSN: ", user.GetCSN(), " // ErrCode : ", ans.m_ansNext.m_lErrorCode);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsNextEnchantInfo_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqItemEnchant(MsgCliCHS_ReqItemEnchant* pReq, CUser& user)
{
	MsgCHSCli_AnsItemEnchant ans;
	ans.m_ansEnchant.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!theNFMenu.ItemEnchant(user.GetGSN(), user.GetCSN(), user.GetNFCharInfoExt(), pReq->m_reqEnchant, ans.m_ansEnchant))
		theLog.Put(ERR_UK, "CHS_LOGIC, OnReqItemEnchant() Failed. CSN: ", user.GetCSN(), " // ErrCode : ", ans.m_ansEnchant.m_lErrorCode);

	PayloadCHSCli pld(PayloadCHSCli::msgAnsItemEnchant_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}

void CChannelContext::OnReqAquaFish(MsgCliCHS_ReqAquaFish* pReq, CUser& user)
{
	MsgCHSCli_AnsAquaFish ans;
	ans.Clear();
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!theNFMenu.GetAquaFish(user.GetCSN(), ans.m_mapNFAquaFish))
	{
		ans.m_lErrorCode = NF::G_NF_ERR_FAIL;
		theLog.Put(ERR_UK, "CHS_LOGIC, OnReqAquaFish() Failed. CSN: ", user.GetCSN());
	}

	PayloadCHSCli pld(PayloadCHSCli::msgAnsAquaFish_Tag, ans);
	SendToUser(user.GetCSN(), pld);
}