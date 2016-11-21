
#include "stdafx.h"
#include "CharLobbyManager.h"
#include <NF/NFMenu.h>


// Menu
BOOL CharLobbyManager::OnReqProductList(CLink* pLink, MsgCliNCS_ReqProductList* pMsg)
{
	BOOL bRet = TRUE;

	MsgNCSCli_AnsProductList ans;
	ans.m_lErrorCode = 0;

	ForEachElmt(TMapIndexProduct, theNFDataItemMgr.GetProduct(), it, ij)
		ans.m_lstProduct.push_back(*((*it).second));

	ForEachElmt(TMapIndexProduct, theNFDataItemMgr.GetProductSkill(), it, ij)
		ans.m_lstProductSkill.push_back(*((*it).second));

	PayloadNCSCli pld(PayloadNCSCli::msgAnsProductList_Tag, ans);
	SendMsg(pLink->GetUser(), pld);

	return bRet;
}

BOOL CharLobbyManager::OnReqChangeParts(CLink* pLink, MsgCliNCS_ReqChangeParts* pMsg)
{
	CUser* pUser = pLink->GetUser();
	if (!pUser)
		return FALSE;

	NFAbilityExt	nfAbilityExt;
	nfAbilityExt.Clear();

	// Level에 따라 착용 가능 여부 추가 - 2010/12/24
	MsgNCSCli_AnsChangeParts	ans;
	ans.Clear();	
	ans.m_ansChangeParts.m_lErrorCode = 0;				// 성공

	BOOL bDebuffRet = FALSE;		// ChangeParts 함수안에서 변경이 안 될 경우를 대비해서 미리 저장해놓는다...

	if (!theNFMenu.ChangeParts(pUser->GetNFCharInfoExt(pUser->GetCSN()), nfAbilityExt, pUser->GetGSN(), pMsg->m_reqChangeParts, ans.m_ansChangeParts, -1, bDebuffRet))
		theLog.Put(ERR, "OnReqChangeParts Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansChangeParts.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsChangeParts_Tag, ans);
	SendMsg(pLink->GetUser(), pld);

	return TRUE;
}

BOOL CharLobbyManager::OnReqBuyItem(CLink* pLink, MsgCliNCS_ReqBuyItem* pMsg)
{
	CUser* pUser = pLink->GetUser();
	if (!pUser)
		return FALSE;

	MsgNCSCli_AnsBuyItem	ans;
	ans.m_ansBuyItem.m_llGameMoney = 0;

	if (!theNFMenu.BuyItem(pUser->GetNFCharInfoExt(pUser->GetCSN()), pUser->GetGSN(), pMsg->m_reqBuyItem, ans.m_ansBuyItem))
		ForEachElmt(TlstBuyItemResult, ans.m_ansBuyItem.m_lstBuyItemResult, it, ij)
		{
			if ((*it).m_lErrorCode != NF::G_NF_ERR_SUCCESS)
				theLog.Put(ERR, "ReqBuyItem Failed!!! CSN : ", pUser->GetCSN(), "// ItemCode  :", (*it).m_buyInvenSlot.m_lItemCode, "// ErrorCode :", (*it).m_lErrorCode);
		}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsBuyItem_Tag, ans);
	SendMsg(pLink->GetUser(), pld);

	return TRUE;
}

BOOL CharLobbyManager::OnReqRemoveItem(CLink* pLink, MsgCliNCS_ReqRemoveItem* pMsg)
{
	CUser* pUser = pLink->GetUser();
	if (!pUser)
		return FALSE;
	
	MsgNCSCli_AnsRemoveItem ans;
	ans.m_ansRemoveItem.m_lErrorCode = 1;
	ans.m_ansRemoveItem.m_removeInvenSlot.Clear();

	if (!theNFMenu.RemoveItem(pUser->GetNFCharInfoExt(pUser->GetCSN()), pUser->GetGSN(), pMsg->m_reqRemoveItem, ans.m_ansRemoveItem))
		theLog.Put(ERR, "RemoveItem Failed!!! CSN : ", pUser->GetCSN(), "// InvenSRL  :", pMsg->m_reqRemoveItem.m_lInvenSRL, "// ErrorCode :", ans.m_ansRemoveItem.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsRemoveItem_Tag, ans);
	SendMsg(pLink->GetUser(), pld);

	return TRUE;
}

BOOL CharLobbyManager::OnReqOpenCardPack(CLink* pLink, MsgCliNCS_ReqOpenCardPack* pMsg)
{
	CUser* pUser = pLink->GetUser();
	if (!pUser)
		return FALSE;

	MsgNCSCli_AnsOpenCardPack	ans;
	ans.m_ansOpenCardPack.m_lErrorCode = 1;

	if (!theNFMenu.OpenCardPack(pUser->GetNFCharInfoExt(pUser->GetCSN()), pUser->GetGSN(), pMsg->m_lInvenSRL, ans.m_ansOpenCardPack))
		theLog.Put(ERR, "OnReqOpenCardPack Failed!!! CSN : ", pUser->GetCSN(), "// InvenSRL  :", pMsg->m_lInvenSRL, "// ErrorCode :", ans.m_ansOpenCardPack.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsOpenCardPack_Tag, ans);
	SendMsg(pLink->GetUser(), pld);

	return TRUE;
}

BOOL CharLobbyManager::OnReqChangeCardSlot(CLink* pLink, MsgCliNCS_ReqChangeCardSlot* pMsg)
{	
	CUser* pUser = pLink->GetUser();
	if (!pUser)
		return FALSE;

	MsgNCSCli_AnsChangeCardSlot	ans;
	ans.m_ansChangeCardSlot.m_lErrorCode = 1;
	ans.m_ansChangeCardSlot.m_lChangeType = pMsg->m_reqChangeCardSlot.m_lChangeType;
	ans.m_ansChangeCardSlot.m_addToInvenSlot.Clear();
	ans.m_ansChangeCardSlot.m_removeFromInvenSlot.Clear();
	ans.m_ansChangeCardSlot.m_llReduceGameMoney = 0;
	ans.m_ansChangeCardSlot.m_nfAbility.Clear();
	ans.m_ansChangeCardSlot.m_bIsGameMoveUse = pMsg->m_reqChangeCardSlot.m_bIsUsingGameMoney;

	NFAbilityExt	nfAbilityExt;
	nfAbilityExt.Clear();

	if (!theNFMenu.ChangeCardSlot(pUser->GetNFCharInfoExt(pUser->GetCSN()), nfAbilityExt, pUser->GetGSN(), pMsg->m_reqChangeCardSlot, ans.m_ansChangeCardSlot))
		theLog.Put(ERR, "OnReqChangeCardSlot Failed!!! CSN : ", pUser->GetCSN(), "// InvenSRL  :", pMsg->m_reqChangeCardSlot.m_lInvenSRL, "// ErrorCode :", ans.m_ansChangeCardSlot.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsChangeCardSlot_Tag, ans);
	SendMsg(pLink->GetUser(), pld);

	return TRUE;
}

BOOL CharLobbyManager::OnReqExchangeCards(CLink* pLink, MsgCliNCS_ReqExchangeCards* pMsg)
{
	CUser* pUser = pLink->GetUser();
	if (!pUser)
		return FALSE;

	MsgNCSCli_AnsExchangeCards	ans;
	ans.m_ansExchangeCards.m_exchangeCards.m_lUpgradeType = pMsg->m_exchangeCards.m_lUpgradeType;
	ans.m_ansExchangeCards.m_exchangeCards.m_bIsSpecialCard = pMsg->m_exchangeCards.m_bIsSpecialCard;
	ans.m_ansExchangeCards.m_exchangeCards.m_vecExchangeCard = pMsg->m_exchangeCards.m_vecExchangeCard;
	ans.m_ansExchangeCards.m_exchangeCards.m_lUpdateCardInvenSRL = pMsg->m_exchangeCards.m_lUpdateCardInvenSRL;
	ans.m_ansExchangeCards.m_lErrorCode = NF::EC_ECR_SUCCESS;
	ans.m_ansExchangeCards.m_newCardPack.Clear();

	if (!theNFMenu.ExchangeCard(pUser->GetNFCharInfoExt(pUser->GetCSN()), pUser->GetGSN(), pMsg->m_exchangeCards, ans.m_ansExchangeCards))
		theLog.Put(ERR, "ReqExchangeCards Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansExchangeCards.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsExchangeCards_Tag, ans);
	SendMsg(pLink->GetUser(), pld);

	return TRUE;
}

BOOL CharLobbyManager::OnReqChangeQuickSlot(CLink* pLink, MsgCliNCS_ReqChangeQuickSlot* pMsg)
{
	CUser* pUser = pLink->GetUser();
	if (!pUser)
		return FALSE;

	MsgNCSCli_AnsChangeQuickSlot ans;
	ans.m_ansChangeQuickSlot.m_lChangeType = pMsg->m_reqChangeQuickSlot.m_lChangeType;

	if (!theNFMenu.ChangeQuickSlot(pUser->GetNFCharInfoExt(pUser->GetCSN()), pUser->GetGSN(), pMsg->m_reqChangeQuickSlot, ans.m_ansChangeQuickSlot))
		theLog.Put(ERR, "ChangeQuickSlot Failed!!! CSN : ", pUser->GetCSN(), "// ErrorCode :", ans.m_ansChangeQuickSlot.m_lErrorCode);

	PayloadNCSCli pld(PayloadNCSCli::msgAnsChangeQuickSlot_Tag, ans);
	SendMsg(pLink->GetUser(), pld);

	return TRUE;
}