
#include "stdafx.h"

#ifdef _NGSNLS
	#include "../../project/NGS/Common.h"
#endif

#include <NF/NFCommonDefine.h>
#include <NFVariant/NFMenu.h>
#include <NFVariant/NFDBManager.h>
#include <NFVariant/NFItem.h>
using namespace NF;

//// ACHV BEGIN
#include <ACHV/AchvDef.h>
static achv::CAchvMgr& g_achv = achv::CAchvMgr::Instance();
//// ACHV END


CNFMenu	theNFMenu;

// protected functions




// protected functions


LONG CNFMenu::FindReduceInvenSlot(TMapInvenSlotList& mapCountableItem, NFInvenSlot& inven, LONG lItemCode)
{
	// �ڽ��� Inven���� �˻��ؿ´�. usable�� skill���� �����ؾ� ��
	TMapInvenSlotList::iterator iter = mapCountableItem.find(lItemCode);
	if (iter == mapCountableItem.end())
		return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;		// not found to my usable inven

	// �κ����� ������ ī��尡 ���� ���� �κ����� ������ �´�...
	ForEachElmt(TlstInvenSlot, (*iter).second, it, ij) 
	{
		if (inven.m_lRemainCount <= 0)
			inven = (*it);

		if (inven.m_lRemainCount > (*it).m_lRemainCount) {
			inven = (*it);
			break;
		}
	}

	if (inven.m_lInvenSRL == 0 || inven.m_lItemCode == 0)
		return NF::G_NF_ERR_INVALID_ITEM_SRL;
	return NF::G_NF_ERR_SUCCESS;
}

// �κ����� ī�� ������ ���� ������ ��� ���� �Լ�
// lChangeType�� CS_ADD�̸�,	char_inven���� card_slot���� �߰� �ϴ� ���....
// lChangeType�� CS_REMOVE�̸�, card_slot���� char_inven���� �߰� �ϴ� ���....
BOOL CNFMenu::ModifyExistCardItem(NFCharInven& mapInven, LONG lInvenSRL, LONG lChangeType, LONG lPartsIndex, NFInvenSlot& inven, LONG& lErrorCode)
{
	if (0 == lInvenSRL) {
		lErrorCode = NF::G_NF_ERR_INVALID_ITEM_SRL;
		return FALSE;
	}

	if (lPartsIndex >= eItemType_FishingCard && lPartsIndex <= eItemType_FishingCard7)
	{
		TMapInven::iterator iter = mapInven.m_mapCharInven.find(lInvenSRL);		
		if (iter == mapInven.m_mapCharInven.end()) {
			lErrorCode = NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN;
			return FALSE;
		}

		if (CS_ADD == lChangeType)				// ī�带 ���� �����Ƿ�... 
		{		
			(*iter).second.m_bIsUsing = TRUE;
			theNFItem.AddUsingInven(mapInven.m_mapUsingItem, (*iter).second);
		}
		else if (CS_REMOVE == lChangeType)		// ī�带 ī�� �κ����� ����
		{			
			(*iter).second.m_bIsUsing = FALSE;
			theNFItem.RemoveUsingInven(mapInven.m_mapUsingItem, (*iter).second.m_lPartsIndex);
		}
		else if (CS_NORMAL == lChangeType)		// ī���� �����ϰ� ���� ���� �κ��� �ڵ常 ���� �ǹǷ�...item_code�� ����
		{
			(*iter).second.m_bIsPackOpen = TRUE;
			(*iter).second.m_lItemCode = inven.m_lItemCode;
		}
		inven = (*iter).second;

		if (inven.m_lItemCode == 0)	{
			lErrorCode = NF::G_NF_ERR_INVALID_ITEM_SRL;
			return FALSE;
		}
	}
	else 
		return FALSE;

	return TRUE;
}

LONG CNFMenu::SetExistCountableItemToInven(NFCharInven& mapInven, NFInvenSlot& inven)
{
	TMapInvenSlotList::iterator	iter = mapInven.m_mapCountableItem.find(inven.m_lItemCode);
	if (iter == mapInven.m_mapCountableItem.end())
		return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;

	ForEachElmt(TlstInvenSlot, (*iter).second, itSlot, ijSlot) 
	{
		if ((*itSlot).m_lInvenSRL == inven.m_lInvenSRL) 
		{
			(*itSlot) = inven;
			break;
		}
	}
	return NF::G_NF_ERR_SUCCESS;
}

// errorcode range : -1 ~ -100
// -2 : not found CardInvenSRL;
// -3 : not found lItemCode;
// -4 : Fishing Card is not same level;
// -5 : card type is not L, G, R, E, F, A, C
// -6 : duplicate card type
LONG CNFMenu::CheckFishingCards(TMapInven& mapInven, TMapExchangeCard& mapExchangeCard, LONG lUpgradeType, BOOL bIsSpecialCard, LONG& lFishingCardLevel)
{
	LONG lErrorCode = NF::EC_ECR_SUCCESS;
	LONG lSameLevel = 0;
	TMapIndexCardItem mapCard = theNFDataItemMgr.GetMapCardItem();
	map<std::string, LONG>		mapUpgradeCheck;

	ForEachElmt(TMapExchangeCard, mapExchangeCard, it, ij)			//	<InvenSRL-ItemCode>
	{
		LONG lInvenSRL = (*it).first;

		TMapInven::iterator iterFind = mapInven.find(lInvenSRL);
		if (iterFind == mapInven.end())
			return NF::EC_ECR_NOT_FOUND_ITEMCODE_FOR_INVEN;			// not found InvenSRL for My Inven; because CardItem

		LONG lItemCode = (*iterFind).second.m_lItemCode;
		(*it).second = lItemCode;

		TMapIndexCardItem::iterator iterFind2 = mapCard.find(lItemCode);
		if (iterFind2 == mapCard.end())
			return NF::EC_ECR_NOT_FOUND_ITEMCODE_FOR_BT_CARD;			// not found itemCode for BT CARD;

		// ī�尡 ��� �������� Ȯ��
		CardItem* pCardItem = (*iterFind2).second;
		if (lSameLevel == 0)
			lSameLevel = pCardItem->m_lLv;
		else
		{
			if (lSameLevel != pCardItem->m_lLv)
				return NF::EC_ECR_NOT_SAME_LEVEL;		// Fishing Card is not same level;
			else
				lSameLevel = pCardItem->m_lLv;
		}
		lFishingCardLevel = lSameLevel;

		// Ȥ�� Pack�� �����ִ��� üũ �Ѵ�...
		if (pCardItem->m_strCardType == "P" || pCardItem->m_strCardType == "U")
			return NF::EC_ECR_IS_NOT_CARDTYPE;			// card type is not L, G, R, E, F, A, C

		// ī���� �ɷ�ġ�� ������ ��ȭ���� ��ȯ���� Ȯ��(��ȭ������ üũ...) - ��� ���� �ٸ� �ɷ�ġ�� �����̾�� �Ѵ�....
		if (2 == lUpgradeType)
		{
			map<std::string, LONG>::iterator iter = mapUpgradeCheck.find(pCardItem->m_strCardType);
			if (iter != mapUpgradeCheck.end())	// ��ȭ�Ϸ��µ� ī�� Ÿ���� ������ �ΰ� �̻� �ִ�.. failed
				return NF::EC_ECR_DUPLICATE_CARD_TYPE;		// duplicate card type
			else
				mapUpgradeCheck[pCardItem->m_strCardType] = 1;
		}
	}

	return lErrorCode;
}

// errorCode range : -100 ~ -200
LONG CNFMenu::ExchangeFishingCards(TMapInven& mapInven, NFInvenSlot& invenUpgradeCard, LONG lFishingCardLevel, NFInvenSlot& getCardPack, LONG lUpgradeType, BOOL bIsSpecialCard)
{
	LONG lErrorCode = NF::EC_ECR_SUCCESS;
	BOOL bIsUpgradeSuccess = FALSE;
	getCardPack.m_bIsPackOpen = TRUE;

	if (invenUpgradeCard.m_lInvenSRL != 0) {
		TMapInven::iterator iter = mapInven.find(invenUpgradeCard.m_lInvenSRL);
		if (iter == mapInven.end())
			return NF::EC_ECR_NOT_FOUND_UPGRADECARD_InvenSRL;			// not found lUpgradeCard InvenSRL;

		LONG lItemCode = (*iter).second.m_lItemCode;

		// ��ȭī������ üũ
		TMapIndexCardItem mapCard = theNFDataItemMgr.GetMapCardItem();
		TMapIndexCardItem::iterator iter2 = mapCard.find(lItemCode);
		if (iter2 == mapCard.end())
			return NF::EC_ECR_NOT_FOUND_UPGRADECARD_ITMSRL;				// not found lUpgradeCard lItemCode;

		if ((*iter2).second->m_strCardType != "U")
			return NF::EC_ECR_MISMATCH_UPGRADECARD_TYPE;				// mismatch upgradecard type
	}

	string strCardType = "P";
	// ���� ī��� ��ȯ, ���� ī���� �� ����...
	if (2 == lUpgradeType) {	// ����ī��� ������ ���, ��ȭ�����ÿ��� ī�� ���� �ø���...

		// ��ȭ Ȯ�� ����
		TVecUpgradeProb vecProb = theNFDataItemMgr.GetUpgradeProb();
		LONG lProb = vecProb[lFishingCardLevel];
		if (lProb <= 0)	
			return NF::EC_ECR_UPGRADE_PROB;		// upgrade prob error

		LONG lUpgradeRand = urandom(100);		// 100% Ȯ����...
		if (lProb > lUpgradeRand)		// ����
		{
			if (lFishingCardLevel < 10)
				lFishingCardLevel += 1;
			bIsUpgradeSuccess = TRUE;
		}
	}
	else if (3 == lUpgradeType) // ��ȭī��� ����, ������ ���� +3
	{
		strCardType = "U";

		// 1->5, 2->6, 3->7, 4->8, 5->9, 6->10
		if (lFishingCardLevel < 7)
			lFishingCardLevel += 3;
		else
			return NF::EC_ECR_UPGRADECARD_NOT_LEVEL;
	}

	// 
	TMapLevelCardItem& mapLevelCardItem = theNFDataItemMgr.GetMapCardPackItem();
	TMapCardTypeCardItem& mapCardTypeCardItem = mapLevelCardItem[lFishingCardLevel];

	// Pack or Upgrade Card�� ã�Ƽ� lItemCode�� �����Ѵ�.
	TMapCardTypeCardItem::iterator iterCard = mapCardTypeCardItem.find(strCardType);
	if (iterCard != mapCardTypeCardItem.end())
	{
		ForEachElmt(TlstCardItem, (*iterCard).second, it, ij)
		{
			getCardPack.m_bIsUsing = FALSE;
			if (strCardType == "P")
				getCardPack.m_bIsPackOpen = FALSE;
			getCardPack.m_strItemCategory = (*it)->m_strType;
			getCardPack.m_lPartsIndex = (*it)->m_lParts;
			getCardPack.m_lItemCode = (*it)->m_lItemCode;
			return 1;
		}
	}
	else
		return NF::EC_ECR_P_OR_U_NOT_FOUND;		// P or UŸ�� not found

	if (!bIsUpgradeSuccess)
		return NF::EC_ECR_FAIL_BECAUSE_PROB;		// ī�� ��ȭ�� ��û������, Ȯ���� ���� �����Ͽ� ��ȯ��

	return lErrorCode;
}

// 1. QuickSlot���� lReglItemCode�� Usable�� ��� ItemCode, SkillItem�� ��� InvenSRL
long CNFMenu::RegistQuickSlot(std::vector<LONG>& vecQuickSlot, NFCharInven& mapInven, LONG lRegSlotIndex, LONG lReglItemCode)
{
	if (lRegSlotIndex >= 0 && lRegSlotIndex < 11)
	{
		LONG lItemCode = vecQuickSlot[lRegSlotIndex];
		if (lItemCode > 0)	
			return NF::EC_QS_EXIST_ITEM_CODE;	// if exist, error
		else
		{
			TMapInvenSlotList::iterator iter = mapInven.m_mapCountableItem.find(lReglItemCode);		// lReglItemCode -> ItemCode
			if (iter != mapInven.m_mapCountableItem.end())
				vecQuickSlot[lRegSlotIndex] = lReglItemCode;		// okay...
			else
				return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;		// not found my inven
		}
	}
	else
		return NF::EC_QS_INVALID_INDEX;
	return NF::EC_QS_SUCCESS;
}

// 2. QuickSlot���� lRemlItemCode�� Usable�� ��� ItemCode, SkillItem�� ��� InvenSRL
// bOnlyRemove : �ý����� �ڵ����� ����� ��ɿ��� ����� �Ķ����, ��¥�� char_inven���� ���� �� ���̶� �翬�� countable_inven�� ����.. �׷��Ƿ� TRUE�� ��� üũ �� ��
long CNFMenu::RemoveQuickSlot(std::vector<LONG>& vecQuickSlot, NFCharInven& mapInven, LONG lRemSlotIndex, LONG lRemlItemCode, BOOL bOnlyRemove)
{
	if (lRemSlotIndex >= 0 && lRemSlotIndex < 11)
	{
		LONG lItemCode = vecQuickSlot[lRemSlotIndex];
		if (lItemCode <= 0)
			return NF::EC_QS_INVALID_ITEM_CODE;
		else
		{
			if (lItemCode == lRemlItemCode)
			{
				if (TRUE == bOnlyRemove)
					vecQuickSlot[lRemSlotIndex] = 0;
				else
				{
					TMapInvenSlotList::iterator iter = mapInven.m_mapCountableItem.find(lRemlItemCode);		// lRemlItemCode -> ItemCode
					if (iter != mapInven.m_mapCountableItem.end())
						vecQuickSlot[lRemSlotIndex] = 0;
					else
						return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;		// not found my inven
				}
			}
			else
				return NF::EC_QS_NOT_SAME_ITEM_CODE;
		}
	}
	else
		return NF::EC_QS_INVALID_INDEX;
	return NF::EC_QS_SUCCESS;
}

// 2. QuickSlot���� lReglItemCode, lRemlItemCode �� Usable�� ��� ItemCode, SkillItem�� ��� InvenSRL
long CNFMenu::SwitchingQuickSlot(std::vector<LONG>& vecQuickSlot, NFCharInven& mapInven, LONG lRegSlotIndex, LONG lReglItemCode, LONG lRemSlotIndex, LONG lRemlItemCode)
{
	if (lRegSlotIndex >= 0 && lRegSlotIndex < 11 && lRemSlotIndex >= 0 && lRemSlotIndex < 11)
	{
		if (lReglItemCode != 0)
		{
			TMapInvenSlotList::iterator iter = mapInven.m_mapCountableItem.find(lReglItemCode);		// ItemCode
			if (iter == mapInven.m_mapCountableItem.end())
				return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;		// not found my inven
		}

		if (lRemlItemCode != 0)
		{
			TMapInvenSlotList::iterator iter = mapInven.m_mapCountableItem.find(lRemlItemCode);		// ItemCode
			if (iter == mapInven.m_mapCountableItem.end())
				return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;		// not found my inven
		}

		vecQuickSlot[lRegSlotIndex] = lReglItemCode;		// swap
		vecQuickSlot[lRemSlotIndex] = lRemlItemCode;
	}
	else
		return NF::EC_QS_INVALID_INDEX;
	return NF::EC_QS_SUCCESS;
}

BOOL CNFMenu::CheckEnvDebuff(NFCharInfoExt* pNFCharInfo, LONG lEnvAttribute)
{
	if (pNFCharInfo->m_nfCheckDebuff[eItemType_Jack] == lEnvAttribute && 
		pNFCharInfo->m_nfCheckDebuff[eItemType_Pant] == lEnvAttribute && 
		pNFCharInfo->m_nfCheckDebuff[eItemType_Glov] == lEnvAttribute &&
		pNFCharInfo->m_nfCheckDebuff[eItemType_Foot] == lEnvAttribute) 
		return TRUE;

	return FALSE;
}

BOOL CNFMenu::AnsAchv(NFCharInfoExt* pNFCharInfo, const ArcListT<LONG>& lstAchvID, AnsAchvInfo& ansMsg)
{
	if (NULL != pNFCharInfo)
		return FALSE;

	ansMsg.m_nfAchievementPoint = pNFCharInfo->m_nfCharAchievement.m_nfCharAP;
	ansMsg.m_mapHistory = pNFCharInfo->m_nfCharAchievement.m_nfCharHistory;

	if (lstAchvID.size() <= 0)
		ansMsg.m_mapAchievement = pNFCharInfo->m_nfCharAchievement.m_nfCharAchieve;
	else
	{
		ForEachCElmt(ArcListT<LONG>, lstAchvID, it, ij)
		{
			TMapAchievement::iterator iter = pNFCharInfo->m_nfCharAchievement.m_nfCharAchieve.find((*it));
			if (iter != pNFCharInfo->m_nfCharAchievement.m_nfCharAchieve.end())
				ansMsg.m_mapAchievement.insert(make_pair((*iter).first, (*iter).second));
		}
	}
	return TRUE;
}

BOOL CNFMenu::CheckBuyMoney(const ReqBuyItem& reqMsg, std::map<LONG/*ItemID*/, Product>& mapProduct, LONGLONG& llTotPrice, LONG& lErrorCode)
{
	ForEachCElmt(TlstBuyItem, reqMsg.m_lstBuyItem, it, ij)
	{
		Product get_product;
		get_product.Clear();

		if (!theNFDataItemMgr.GetProductByItemCode((*it).m_lItemCode, (*it).m_lItemID, get_product)) {
			lErrorCode = NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMID;
			return FALSE;
		}
		else {
			mapProduct.insert(make_pair((*it).m_lItemID, get_product));
			llTotPrice += get_product.m_lPrice;
		}
	}
	return TRUE;
}

// 
BOOL CNFMenu::BuyItem(NFCharInfoExt* pNFCharInfo, LONG lGSN, const ReqBuyItem& reqMsg, const std::map<LONG/*ItemID*/, Product>& mapProduct, AnsBuyItem& ansMsg)
{
	if (NULL == pNFCharInfo)
		return FALSE;

	LONG lCSN = pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN;

	if (reqMsg.m_lBuyType == BIT_PRESENT)		// if present, 
		lCSN = reqMsg.m_lPresentToCSN;

	ForEachCElmt(TlstBuyItem, reqMsg.m_lstBuyItem, it, ij)
	{
		BuyItemResult	result;
		result.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

		LONG lCapacity = 0;
		NFInvenSlot oldInven; 
		oldInven.Clear();

		std::map<LONG/*ItemID*/, Product>::const_iterator iterFind = mapProduct.find((*it).m_lItemID);
		if (iterFind == mapProduct.end()) {
			ansMsg.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMID;
			return FALSE;
		}

		Product get_product = (*iterFind).second;

		result.m_lErrorCode = theNFMenu.GetInvenSRLFromExistInven(pNFCharInfo->m_nfCharInven, oldInven, get_product.m_lItemCode, lCapacity);
		if (NF::G_NF_ERR_SUCCESS != result.m_lErrorCode)
			continue;

		LONG lTempInvenSRL = oldInven.m_lInvenSRL;
		if (0 == lTempInvenSRL)
		{
			oldInven.m_lItemCode = (*it).m_lItemCode;
			oldInven.m_lPartsIndex = (*it).m_lItemCode/G_VALUE_CONVERT_PARTS;
		}

		// ����
		BOOL bCheckCard = FALSE;
		string strPackOpen = "Y";

		// �׽�Ʈ������ �Ѱ� ī�带 �����Ѵ�...
		CardItem* pItem = theNFDataItemMgr.GetCardItemByIndex((*it).m_lItemID);		// ������ ���Žô� ItemID
		if (pItem)
		{
			if (pItem->m_lParts >= eItemType_FishingCard || pItem->m_lParts <= eItemType_FishingCard7)
				bCheckCard = TRUE;

			if (pItem->m_strCardType == "P")
				strPackOpen = "N";
		}

		BOOL bQueryRet = FALSE;
		if (reqMsg.m_lBuyType == BIT_ADMIN || bCheckCard == TRUE)		// Admin Procedure ȣ��, ���������� �ǽ�ī�������� ��� ���
			bQueryRet = theNFDBMgr.BuyItemFromShop_Admin(BIT_ADMIN, pItem, lGSN, lCSN, (*it).m_lItemID, (*it).m_strBuyProductSRL, oldInven.m_lInvenSRL, strPackOpen, (int&)result.m_lErrorCode);
		else
			bQueryRet = theNFDBMgr.BuyItemFromShop(reqMsg.m_lBuyType, lGSN, lCSN, (*it).m_lItemID, (*it).m_strBuyProductSRL, oldInven.m_lInvenSRL, ansMsg.m_llGameMoney, (int&)result.m_lErrorCode);

		// Buy Item
		if (bQueryRet)
		{
			// Money ����
			pNFCharInfo->m_nfCharBaseInfo.m_llMoney = ansMsg.m_llGameMoney;

			// �׽�Ʈ������ �Ѱ� ī�带 �����Ѵ�...
			CardItem* pItem = theNFDataItemMgr.GetCardItemByIndex(oldInven.m_lItemCode);
			if (pItem)
			{
				if (pItem->m_strCardType != "P")
					oldInven.m_bIsPackOpen = TRUE;
			}

			NFInvenSlot newInven; newInven.Clear();
			result.m_lErrorCode = theNFItem.ProcessCapacityInven(pNFCharInfo->m_nfCharInven, lTempInvenSRL, oldInven, newInven, get_product.m_lItemCNT, lCapacity, result.m_mapOld, result.m_mapNew);
		}
		else {
			ansMsg.m_llGameMoney = pNFCharInfo->m_nfCharBaseInfo.m_llMoney;
			result.m_lErrorCode = NF::G_NF_ERR_DB_BUY_ITEM_SHOP;		// BuyItemFromShop Error
		}
		ansMsg.m_lstBuyItemResult.push_back(result);
	}

	return TRUE;
}

BOOL CNFMenu::RemoveItem(NFCharInfoExt* pNFCharInfo, LONG lGSN, const ReqRemoveItem& reqMsg, AnsRemoveItem& ansMsg)
{
	if (NULL == pNFCharInfo)
		return FALSE;

	LONG lCSN = pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN;

	ansMsg.m_lErrorCode = theNFItem.GetNFInvenSlot(pNFCharInfo->m_nfCharInven, reqMsg.m_lItemCode, reqMsg.m_lInvenSRL, FALSE, ansMsg.m_removeInvenSlot);
	if (NF::G_NF_ERR_SUCCESS == ansMsg.m_lErrorCode)
	{
		// User�� ������ ��û���� ��� DropLog�� 9�� �����...
		if (!theNFDBMgr.UpdateRemoveItem(lGSN, lCSN, reqMsg.m_lInvenSRL))
		{
			ansMsg.m_lErrorCode = NF::G_NF_ERR_DB_REMOVE_ITEM; // UpdateRemoveItem error
			return FALSE;
		}
	}

	ansMsg.m_removeInvenSlot.m_lRemainCount = 0;		// �ӽ÷�...

	return theNFItem.RemoveItem(pNFCharInfo->m_nfCharInven, ansMsg.m_removeInvenSlot);
}

BOOL CNFMenu::OpenCardPack(NFCharInfoExt* pNFCharInfo, LONG lGSN, LONG lReqInvenSRL, AnsOpenCardPack& ansMsg)
{
	BOOL bRet = TRUE;
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	int		nErrorCode = 0;

	// 1. Inven���� ������ �ִ� ���������� üũ
	TMapInven::iterator	iter = pNFCharInfo->m_nfCharInven.m_mapCharInven.find(lReqInvenSRL);
	if (iter != pNFCharInfo->m_nfCharInven.m_mapCharInven.end())
	{
		ansMsg.m_openCardInvenSlot = (*iter).second;

		if (ansMsg.m_openCardInvenSlot.m_lInvenSRL == 0 || ansMsg.m_openCardInvenSlot.m_lItemCode == 0) {
			ansMsg.m_lErrorCode = NF::G_NF_ERR_INVALID_ITEM_SRL;
			return FALSE;
		}

		// 2. ī�尡 �̹� �����Ѱ��� �ƴ��� üũ
		if (ansMsg.m_openCardInvenSlot.m_bIsPackOpen == FALSE)
		{
			//// 3. CardPack�� ������ �о�´�.
			TMapIndexCardItem& mapCardItem = theNFDataItemMgr.GetMapCardItem();
			CardItem* pItem = mapCardItem[ansMsg.m_openCardInvenSlot.m_lItemCode];
			if (pItem)
			{
				// 4. CardType�� Pack���� üũ
				if (pItem->m_strCardType == "P")
				{
					// Ȯ��ǥ���� �ش� Pack�� rate�� ������´�.
					TMapIndexCardPackRate& mapCardPackRate = theNFDataItemMgr.GetMapCardPackRate();
					TMapIndexCardPackRate::iterator iter = mapCardPackRate.find(pItem->m_lItemCode);
					if (iter != mapCardPackRate.end())
					{
						TlstCardRate& lstCardRate = (*iter).second;
						if (lstCardRate.size() > 0)
						{
							// Ȯ���� ���Ѵ�.. 100%
							LONG lChoice = urandom(100);
							LONG lRateTotal = 0;

							ForEachElmt(TlstCardRate, lstCardRate, it, ij)
							{
								lRateTotal += (*it).second;
								if (lChoice < lRateTotal)
								{
									ansMsg.m_openCardInvenSlot.m_lItemCode = (*it).first;
									break;
								}
							}

							// DB Procedure Call
							theNFDBMgr.UpdateOpenCardPack(lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, lReqInvenSRL, ansMsg.m_openCardInvenSlot.m_lItemCode, nErrorCode);
							ansMsg.m_lErrorCode = nErrorCode;
							if (ansMsg.m_lErrorCode == NF::G_NF_ERR_SUCCESS) 
							{
								if (!ModifyExistCardItem(pNFCharInfo->m_nfCharInven, lReqInvenSRL, CS_NORMAL, eItemType_FishingCard, ansMsg.m_openCardInvenSlot, ansMsg.m_lErrorCode))
									ansMsg.m_lErrorCode = NF::G_NF_ERR_MODIFY_EXIST_CARD;
							}
							else
								ansMsg.m_lErrorCode = NF::G_NF_ERR_DB_OPEN_CARDPACK;		// UpdateOpenCardPack Error
						}
						else
							ansMsg.m_lErrorCode = NF::EC_OC_INVALID_CARD_LIST_SIZE;		// CardItem Size invalid
					}
					else
						ansMsg.m_lErrorCode = NF::EC_OC_NOT_FOUND_CARDRATE;		// not found Card Information Data from CardPackRate
				}
				else
					ansMsg.m_lErrorCode = NF::EC_OC_MISSMATCH_CARDTYPE;		// not match CardType				
			}
			else
				ansMsg.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMCODE;		// not found Card Information Data from CardItem
		}
		else
			ansMsg.m_lErrorCode = NF::EC_OC_ALREADY_OPEN;		// already cardpack opened
	}
	else
		ansMsg.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN;		// not found error	

	return bRet;
}

BOOL CNFMenu::ChangeParts(NFCharInfoExt* pNFCharInfo, LONG lGSN, const RoomID& roomID, const ReqChangeParts& reqMsg, AnsChangeParts& ansMsg, LONG lEnvAttribute, BOOL& bDebuffRet)
{
	BOOL bRet = TRUE;
	if (NULL == pNFCharInfo)
		return FALSE;

	ansMsg.m_newInvenSlot.Clear();
	ansMsg.m_oldInvenSlot.Clear();

	ansMsg.m_oldInvenSlot.m_lItemCode = reqMsg.m_lOldItemCode;
	ansMsg.m_oldInvenSlot.m_lInvenSRL = reqMsg.m_lOldInvenSRL;
	ansMsg.m_newInvenSlot.m_lItemCode = reqMsg.m_lNewItemCode;
	ansMsg.m_newInvenSlot.m_lInvenSRL = reqMsg.m_lNewInvenSRL;

	// 1. �������� ������ �ִ� ������ �κ� �� ������ üũ
	ansMsg.m_lErrorCode = theNFItem.CheckValidChangingItems(pNFCharInfo, ansMsg.m_oldInvenSlot, ansMsg.m_newInvenSlot);
	if (NF::G_NF_ERR_SUCCESS != ansMsg.m_lErrorCode)
		return FALSE;

	// 2. DB�� ��û�Ѵ�...
	if (!theNFDBMgr.UpdatePartsByCSN(ansMsg.m_oldInvenSlot, ansMsg.m_newInvenSlot, lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, ansMsg.m_lErrorCode))
		return FALSE;

	// 3. DB�� �����ϸ�, ���� �޸𸮸� �����Ѵ�...
	ansMsg.m_lErrorCode = theNFItem.SetChangedItems(lGSN, roomID, pNFCharInfo, ansMsg.m_oldInvenSlot, ansMsg.m_newInvenSlot);
	if (NF::G_NF_ERR_SUCCESS != ansMsg.m_lErrorCode)
		return FALSE;
	
	if (lEnvAttribute != -1)
	{
		// �������� ���� ������, ȯ�� �Ӽ��� ���� üũ �Ѵ�.
		bDebuffRet = CheckEnvDebuff(pNFCharInfo, lEnvAttribute);
		if (FALSE == bDebuffRet)
			ansMsg.m_lErrorCode = NF::EC_CP_CARD_ENV_DEBUFF;			// ȯ��Ӽ� ������� �ް� �ִ�.
	}

	ansMsg.m_nfAbility = pNFCharInfo->m_nfAbility;

	return bRet;
}

BOOL CNFMenu::ChangeCardSlot(NFCharInfoExt* pNFCharInfo, NFAbilityExt& nfAbilityExt, LONG lGSN, const ReqChangeCardSlot& reqMsg, AnsChangeCardSlot& ansMsg)
{
	BOOL bRet = TRUE;
	if (NULL == pNFCharInfo)
		return FALSE;

	// Level�� ���� SlotIndex üũ
	// nfCharInfoExt.m_nfCharInven.m_mapUsingItem
	NFInvenSlot	inven;
	inven.Clear();

	ansMsg.m_lErrorCode = theNFItem.GetNFInvenSlot(pNFCharInfo->m_nfCharInven, 0, reqMsg.m_lInvenSRL, FALSE, inven);
	if (NF::G_NF_ERR_SUCCESS != ansMsg.m_lErrorCode)
		return FALSE;

	if (inven.m_lPartsIndex >= eItemType_FishingCard && inven.m_lPartsIndex <= eItemType_FishingCard7)
	{
		ItemCommon* pItemCommon = theNFDataItemMgr.GetCardItemByIndex(inven.m_lItemCode);
		if (pItemCommon)
		{
			// parts index check
			if (reqMsg.m_lParts >= eItemType_FishingCard1 && reqMsg.m_lParts <= eItemType_FishingCard7)
			{
				if (CS_ADD == reqMsg.m_lChangeType)		// ��� �ִ� ���� add
				{
					if (!ModifyExistCardItem(pNFCharInfo->m_nfCharInven, reqMsg.m_lInvenSRL, CS_ADD, reqMsg.m_lParts, ansMsg.m_removeFromInvenSlot, ansMsg.m_lErrorCode))
						return FALSE;
				}
				else if (CS_REMOVE == reqMsg.m_lChangeType)	// �ִ� ������ remove
				{
					if (!ModifyExistCardItem(pNFCharInfo->m_nfCharInven, reqMsg.m_lInvenSRL, CS_REMOVE, reqMsg.m_lParts, ansMsg.m_addToInvenSlot, ansMsg.m_lErrorCode))
						return FALSE;
				}
				else if (CS_SWITCHING == reqMsg.m_lChangeType)	// switching, ī�� ���� ���ο� ���� ���� �Ӵ� ����
				{
					// DB���� �Ʒ����� �����ϰ� �Ǹ� ���� �� �ʿ� ����...SelectNFCharInven���� �� �ϹǷ�....
					// ī���κ����� ����.. ĳ�����κ��� �߰�(bIsUsing = FALSE)
					//nfCharInfoExt.m_nfCharInven.m_mapUsingItem[reqMsg.m_lParts].m_bIsUsing = FALSE; Ȯ�� ����????? 2011/9/7
					if (!ModifyExistCardItem(pNFCharInfo->m_nfCharInven, reqMsg.m_lInvenSRL, CS_REMOVE, eItemType_FishingCard, ansMsg.m_addToInvenSlot, ansMsg.m_lErrorCode)) {
						ansMsg.m_lErrorCode = NF::EC_CP_CARD_SWITCHING;		// switching remove not found
						return FALSE;
					}

					// ī���κ��� �߰�.. ĳ���� �κ����� ����(bIsUsing = TRUE)
					if (!ModifyExistCardItem(pNFCharInfo->m_nfCharInven, reqMsg.m_lInvenSRL, CS_ADD, reqMsg.m_lParts, ansMsg.m_removeFromInvenSlot, ansMsg.m_lErrorCode)) {
						ansMsg.m_lErrorCode = NF::EC_CP_CARD_SWITCHING;		// switching remove not found
						return FALSE;
					}
				}
				else 
					ansMsg.m_lErrorCode = NF::EC_CP_CARD_CHANGE_TYPE;		// changeType Error

				// ��� ������ �����ϸ�, Call DB Procedure
				if (ansMsg.m_lErrorCode == NF::EC_CP_SUCCESS)
				{
					// DB Call - ChangeCardInven
					// ans.m_removeFromInven : �κ����κ��� ���ŵɳ�, ī�� ���Կ� �߰��ɳ�(�����ҳ�)
					// ans.m_addToInven : �κ����� �߰� �ɳ�, ī�� ���Կ��� ���ŵɳ�(���Եɳ�)
					if (theNFDBMgr.UpdateCardPartsByCSN(ansMsg.m_addToInvenSlot, ansMsg.m_removeFromInvenSlot, ansMsg.m_llReduceGameMoney, lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, ansMsg.m_lErrorCode))
					{			
						pNFCharInfo->m_nfCharInven.Clear();

						TlstInvenSlot lst;

						// @@ Waiting Remove...
						if (!theNFDBMgr.SelectNFCharInven(lst, pNFCharInfo->m_nfCharBaseInfo.m_strLastestLogOutDate, pNFCharInfo->m_nfCharInven, lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, ansMsg.m_lErrorCode))
							ansMsg.m_lErrorCode = NF::G_NF_ERR_DB_SELECT_CHAR_INVEN;			// SelectNFCharInvenByCSN Error

						if (NF::EC_CP_SUCCESS != theNFDBMgr.SelectNFAbilityExtByCSN(pNFCharInfo->m_nfAbility, nfAbilityExt, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN))
							ansMsg.m_lErrorCode = NF::G_NF_ERR_DB_SELECT_ABILITY;			// SelectNFAbilityExtByCSN Error

						NFAbility	nfBasicAbility;
						if (!theNFDataItemMgr.GetNFAbility(pNFCharInfo->m_nfCharExteriorInfo.m_lBasicCharSRL, nfBasicAbility))
							ansMsg.m_lErrorCode = NF::G_NF_ERR_ABILITY;			// 	GetNFAbility Error							
						pNFCharInfo->m_nfAbility += nfBasicAbility;

						ansMsg.m_nfAbility += pNFCharInfo->m_nfAbility;
						ansMsg.m_nfAbility += nfAbilityExt.m_nfAbility;
					}
					else
						ansMsg.m_lErrorCode = NF::G_NF_ERR_DB_UPDATE_PARTS;			// UpdateCardPartsByCSN Error
				}
			}
			else
				ansMsg.m_lErrorCode = NF::EC_CP_CARD_PARTS_INDEX;		// parts index error
		}
		else
			ansMsg.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMCODE;		// card item not found;
	}
	else
		ansMsg.m_lErrorCode = NF::EC_CP_CARD_NOT_CARDTYPE;		// Not match ItemCategory == "R"

	return bRet;
}

BOOL CNFMenu::ExchangeCard(NFCharInfoExt* pNFCharInfo, LONG lGSN, const ExchangeCards& reqMsg, AnsExchangeCards& ansMsg)
{
	BOOL bRet = TRUE;
	if (NULL == pNFCharInfo)
		return FALSE;

	LONG lFishingCardLevel = 0;
	int nErrorCode = NF::EC_ECR_SUCCESS;

	// Temp Inven
	NFInvenSlot	invenUpgradeCard;		// ��ȭ ī��
	invenUpgradeCard.Clear();
	
	TMapExchangeCard	mapExchangeCard;
	
	ForEachCElmt(vector<LONG>, reqMsg.m_vecExchangeCard, it, ij)
		mapExchangeCard.insert( make_pair((*it), 0) );

	invenUpgradeCard.m_lItemCode  = ansMsg.m_exchangeCards.m_lUpdateCardItemCode;
	invenUpgradeCard.m_lInvenSRL = ansMsg.m_exchangeCards.m_lUpdateCardInvenSRL;

	// Check FishingCard valid and Type
	// errorcode range : -2 ~ -100
	ansMsg.m_lErrorCode = CheckFishingCards(pNFCharInfo->m_nfCharInven.m_mapCharInven, mapExchangeCard, reqMsg.m_lUpgradeType, reqMsg.m_bIsSpecialCard, lFishingCardLevel);
	if (ansMsg.m_lErrorCode == NF::EC_ECR_SUCCESS)
	{
		// errorCode range : -100 ~ -200
		ansMsg.m_lErrorCode = ExchangeFishingCards(pNFCharInfo->m_nfCharInven.m_mapCharInven, invenUpgradeCard, lFishingCardLevel, ansMsg.m_newCardPack, reqMsg.m_lUpgradeType, reqMsg.m_bIsSpecialCard);
		if (ansMsg.m_lErrorCode == NF::EC_ECR_SUCCESS)
		{
			string strBuySRL = "BuySRL";
			string strProductSRL = "ProductSRL";
			theNFDBMgr.UpdateUpgrageCard(lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, mapExchangeCard, invenUpgradeCard, ansMsg.m_newCardPack, nErrorCode);

			ansMsg.m_lErrorCode = nErrorCode;
			// ����� �����̸�, �κ��� �߰�
			if (ansMsg.m_lErrorCode == NF::EC_ECR_SUCCESS) 
			{
				// ����� ī�带 ���� �Ѵ�...
				// <InvenSRL, ItemCode>
				ForEachElmt(TMapExchangeCard, mapExchangeCard, it, ij)
				{
					NFInvenSlot remove_inven;
					remove_inven.Clear();

					remove_inven.m_lItemCode = (*it).first;
					remove_inven.m_lInvenSRL = (*it).second;
					if (!theNFItem.RemoveItem(pNFCharInfo->m_nfCharInven, remove_inven)) {
						ansMsg.m_lErrorCode = NF::G_NF_ERR_REMOVE_INVEN_SLOT;
						return FALSE;
					}
				}

				if (invenUpgradeCard.m_lInvenSRL >= 0)
				{
					if (!theNFItem.RemoveItem(pNFCharInfo->m_nfCharInven, invenUpgradeCard)) {
						ansMsg.m_lErrorCode = NF::G_NF_ERR_REMOVE_INVEN_SLOT_UPGRADE_CARD;
						return FALSE;
					}
				}

				LONG lErr = theNFItem.AddInvenSlotItem(pNFCharInfo->m_nfQuickSlot, pNFCharInfo->m_nfCharInven, ansMsg.m_newCardPack);
				if (lErr != NF::G_NF_ERR_SUCCESS) {
					ansMsg.m_lErrorCode = lErr;
					return FALSE;
				}
			}
		}
	}
	return bRet;
}

// 2011-10-31, ItemCode �ߺ� üũ ����, 
// ������ ������ŭ quickslot�� ��ϵ� �� �ۿ� ���°� ������� Ȯ�� -bback99
BOOL CNFMenu::ChangeQuickSlot(NFCharInfoExt* pNFCharInfo, LONG lGSN, const ReqChangeQuickSlot& reqMsg, AnsChangeQuickSlot& ansMsg, BOOL bOnlyRemove)
{
	BOOL bRet = TRUE;
	if (NULL == pNFCharInfo)
		return FALSE;

	ArcVectorT< LONG > QuickSlot	= pNFCharInfo->m_nfQuickSlot;
	NFInvenSlot	changedInven;
	changedInven.Clear();

	if (CS_ADD == reqMsg.m_lChangeType)		// regist
		ansMsg.m_lErrorCode = RegistQuickSlot(QuickSlot, pNFCharInfo->m_nfCharInven, reqMsg.m_lRegistSlotIndex, reqMsg.m_lRegistItemCode);
	else if(CS_REMOVE == reqMsg.m_lChangeType)	// remove
		ansMsg.m_lErrorCode = RemoveQuickSlot(QuickSlot, pNFCharInfo->m_nfCharInven, reqMsg.m_lRemoveSlotIndex, reqMsg.m_lRemoveItemCode, bOnlyRemove);
	else if(CS_SWITCHING == reqMsg.m_lChangeType)	// switch
		ansMsg.m_lErrorCode = SwitchingQuickSlot(QuickSlot, pNFCharInfo->m_nfCharInven, reqMsg.m_lRegistSlotIndex, reqMsg.m_lRegistItemCode, reqMsg.m_lRemoveSlotIndex, reqMsg.m_lRemoveItemCode);
	else
	{
		ansMsg.m_lErrorCode = NF::G_NF_ERR_QUICK_SLOT_TYPE;
		return FALSE;
	}

	string strStartDate = G_INVALID_DATE, strEndDate = G_INVALID_DATE;

	if (theNFDBMgr.UpdateQuickSlotByCSN(lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, reqMsg.m_lChangeType, reqMsg.m_lRegistSlotIndex, reqMsg.m_lRegistItemCode, reqMsg.m_lRemoveSlotIndex, reqMsg.m_lRemoveItemCode, strStartDate, strEndDate, ansMsg.m_lErrorCode))
	{
		if (reqMsg.m_lChangeType != CS_SWITCHING) 
		{
			LONG lItemCode = reqMsg.m_lRegistItemCode;
			if (reqMsg.m_lChangeType == CS_REMOVE)
				lItemCode = reqMsg.m_lRemoveItemCode;

			if (lItemCode != -1)
				theNFItem.GetExistToInvenList(pNFCharInfo->m_nfCharInven, lItemCode, TRUE, strStartDate, strEndDate, ansMsg.m_lstChangedSlot, ansMsg.m_lErrorCode);
		}

		// success
		pNFCharInfo->m_nfQuickSlot = QuickSlot;
		ansMsg.m_vecQuickSlot = pNFCharInfo->m_nfQuickSlot;
	}
	else
		ansMsg.m_lErrorCode = NF::G_NF_ERR_DB_UPDATE_QUICK_SLOT;

	return bRet;
}

BOOL CNFMenu::GetProductList(AnsProductList& ans)
{
	ans.m_lErrorCode = NF::EC_ECR_SUCCESS;
	ans.m_mapProduct = theNFDataItemMgr.GetProduct();
	ans.m_mapProductSkill = theNFDataItemMgr.GetProductSkill();
	return TRUE;
}

// ItemCode�� ���ٴ� �Ҹ��� ȿ�»Ӹ� �ƴ϶� Gauge���� ���ٴ� �Ҹ���....
// ���, ItemCode�� ���� ����� ��� Capacity�� MAX������ ä������ ���� �Ѵٴ� �Ҹ�... 
LONG CNFMenu::GetInvenSRLForBuyItem(NFCharInven& charInven, LONG lItemCode)
{
	LONG lCapacity = theNFDataItemMgr.GetCapacityByItemCode(lItemCode);
	if (lCapacity == 0)
		return 0;		// Error

	LONG lParts = lItemCode/G_VALUE_CONVERT_PARTS;
	if (theNFItem.IsCountableItem(lParts))
	{
		TMapInvenSlotList::iterator iter = charInven.m_mapCountableItem.find(lItemCode);
		if (iter != charInven.m_mapCountableItem.end())
		{
			ForEachElmt(TlstInvenSlot, (*iter).second, it, ij)
			{
				if (lCapacity > (*it).m_lRemainCount)
					return (*it).m_lInvenSRL;
			}
		}
	}
	else
	{
		ForEachElmt(TMapInven, charInven.m_mapCharInven, it2, ij2)
		{
			if ((*it2).second.m_lItemCode == lItemCode)
			{
				if (lCapacity > (*it2).second.m_lRemainCount)
					return (*it2).second.m_lInvenSRL;
			}
		}
	}
	return 0;
}

LONG CNFMenu::GetInvenSRLFromExistInven(NFCharInven& charInven, NFInvenSlot& inven, const LONG lItemCode, LONG& lCapacity)
{
	lCapacity = theNFDataItemMgr.GetCapacityByItemCode(lItemCode);
	if (lCapacity == 0)
		return G_NF_ERR_NOT_FOUND_CAPACITY;		// Error

	LONG lParts = lItemCode/G_VALUE_CONVERT_PARTS;
	if (theNFItem.IsCountableItem(lParts))
	{
		TMapInvenSlotList::iterator iter = charInven.m_mapCountableItem.find(lItemCode);
		if (iter != charInven.m_mapCountableItem.end())
		{
			ForEachElmt(TlstInvenSlot, (*iter).second, it, ij)
			{
				if (lCapacity > (*it).m_lRemainCount) {
					inven = (*it);
					return NF::G_NF_ERR_SUCCESS;
				}
			}
		}
	}
	else
	{
		// Capacity�� 1�ʰ��� �����۵��� ������ �� �ɵ�.. 
		// ���� �������� �����۵��� �ƴϴ�...
		return NF::G_NF_ERR_SUCCESS;
	}
	return NF::G_NF_ERR_SUCCESS;
}

LONG CNFMenu::CalcLevelByAddExp(const LONG lGSN, NFCharInfoExt* pNFCharInfo, LONG lGetExp, const RoomID& roomID)
{
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErrorCode = NF::G_NF_ERR_SUCCESS;

	// ����ġ ����
	LONG lTotExp = pNFCharInfo->m_nfCharBaseInfo.m_lExp + lGetExp;
	while(true)
	{
		// ����ġ ���̺��� ��
		LONG lMaxExp = 0;

		lErrorCode = theNFDataItemMgr.GetNFExp(pNFCharInfo->m_nfCharBaseInfo.m_lLevel, lMaxExp);
		if (lErrorCode != NF::G_NF_ERR_SUCCESS)
			return lErrorCode;

		if (lTotExp >= lMaxExp)		// Level ��
		{
			pNFCharInfo->m_nfCharBaseInfo.m_lExp = lTotExp-lMaxExp;
			pNFCharInfo->m_nfCharBaseInfo.m_lLevel += 1;
			lTotExp = pNFCharInfo->m_nfCharBaseInfo.m_lExp;

			// �������ϰԵǸ�, GapTable���� Gap���̸�ŭ �ɷ�ġ�� ���Ѵ�...
			NFAbility abilityGap;
			abilityGap.Clear();

			lErrorCode = theNFDataItemMgr.GetNFAbilityGapByLevel(pNFCharInfo->m_nfCharBaseInfo.m_lLevel, abilityGap);
			if (lErrorCode != NF::G_NF_ERR_SUCCESS)
				return lErrorCode;

			pNFCharInfo->m_nfAbility += abilityGap;

			/// achv
			{
				TMapAchvFactor	mapFactorVal;
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_LEVEL], pNFCharInfo->m_nfCharBaseInfo.m_lLevel));
				g_achv.CheckAchv(lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, achv::AE_LEVEL, roomID, mapFactorVal);

				mapFactorVal.clear();
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_CHARM], (LONG)pNFCharInfo->m_nfAbility.m_dCharm));
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_STR], (LONG)pNFCharInfo->m_nfAbility.m_dStrength));
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_CONTROL], (LONG)pNFCharInfo->m_nfAbility.m_dControl));
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_FLY_DIST], (LONG)pNFCharInfo->m_nfAbility.m_dFlyDist));
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_AGILITY], (LONG)pNFCharInfo->m_nfAbility.m_dAgility));
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_LUCKY], (LONG)pNFCharInfo->m_nfAbility.m_dLuckyPoint));
				mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_HEALTH], (LONG)pNFCharInfo->m_nfAbility.m_dHealth));
				g_achv.CheckAchv(lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, achv::AE_ABILITY, roomID, mapFactorVal);
			}
		}
		else 
		{
			pNFCharInfo->m_nfCharBaseInfo.m_lExp = lTotExp;
			break;
		}
	}

	// Grade�� ���߿�...

	return lErrorCode;
}

// ������ ������ �ѹ濡 ����~~ (AP, EXP, Money, ITems...)
BOOL CNFMenu::GetRewardItem(LONG lGSN, NFCharInfoExt* pNFCharInfo, const RoomID& roomID, const ReqRewardItem& reqMsg, AnsRewardItem& ansMsg)
{
	if (NULL == pNFCharInfo)
		return FALSE;

	NFReward reward;
	ansMsg.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!g_achv.getedRewardItem(pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, reqMsg.m_lAchvID)) {
		ansMsg.m_lErrorCode = NF::EC_ACHV_REWARD_NOT_FOUND_ACHV_ID;
		return FALSE;
	}

	if (!theNFDataItemMgr.GetAchvReward(reqMsg.m_lAchvID, reward)) {
		ansMsg.m_lErrorCode = NF::EC_ACHV_REWARD_NOT_FOUND_REWARD;
		return FALSE;
	}

	// 1. User�� ������ Item ����
	std::vector<NFInvenSlot>	vecInven;
	std::vector<Product>	vecProduct;
	std::vector<int>		vecOutputInvenSRL;
	std::vector<LONG>		vecCapacity;

	//
	for(int i=0; i<achv::RewardItemIdCnt; i++) {
		NFInvenSlot	inven;
		inven.Clear();
		vecInven.push_back(inven);
		Product product;
		product.Clear();
		vecProduct.push_back(product);
		vecOutputInvenSRL.push_back(0);
		vecCapacity.push_back(0);
	}

	// invensrl�� ã�ƿ;� �Ѵ�...
	for(i=0; i<(int)reqMsg.m_vecRewardItem.size(); i++) 
	{
		if (reqMsg.m_vecRewardItem[i] != 0) 
		{
			theNFDataItemMgr.GetProductFromGiveItem(reqMsg.m_vecRewardItem[i], vecProduct[i]);
			GetInvenSRLFromExistInven(pNFCharInfo->m_nfCharInven, vecInven[i], vecProduct[i].m_lItemCode, vecCapacity[i]);

			vecOutputInvenSRL[i] = vecInven[i].m_lInvenSRL;
			if (0 == vecOutputInvenSRL[i])
			{
				vecInven[i].m_lItemCode = vecProduct[i].m_lItemCode;
				vecInven[i].m_lPartsIndex = vecProduct[i].m_lItemCode/G_VALUE_CONVERT_PARTS;
			}
		}
	}

	int nRewardItemID[achv::RewardItemIdCnt] = {achv::InvalidAchvId, };
	if (!theNFDBMgr.GiveAchvReward(vecOutputInvenSRL, lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, reqMsg.m_lAchvID, reqMsg.m_vecRewardItem, vecInven, ansMsg.m_lErrorCode)) {
		ansMsg.m_lErrorCode = NF::G_NF_ERR_DB_GIVE_REWARD;
		return FALSE;
	}

	if (ansMsg.m_lErrorCode != NF::G_NF_ERR_SUCCESS)
		return FALSE;

	// ��¥ ������Ʈ
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	ansMsg.m_strRewardDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	if (reqMsg.m_vecRewardItem.size() > 0)
	{
		for(int i=0; i<achv::RewardItemIdCnt; i++)
		{
			if (vecInven[i].m_lInvenSRL <= 0)
				continue;

			NFInvenSlot newInven; newInven.Clear();
			ansMsg.m_lErrorCode  = theNFItem.ProcessCapacityInven(pNFCharInfo->m_nfCharInven, vecOutputInvenSRL[i], vecInven[i], newInven, vecProduct[i].m_lItemCNT, vecCapacity[i], ansMsg.m_mapOldInven, ansMsg.m_mapNewInven);
			if (NF::G_NF_ERR_SUCCESS != ansMsg.m_lErrorCode)
				return ansMsg.m_lErrorCode;
			nRewardItemID[i] = reqMsg.m_vecRewardItem[i];
		}
	}

	// 1. Item�� ������ Money, exp, ap �� �����Ѵ�.
	ansMsg.m_lAchvID = reward.achv_ID;
	ansMsg.m_lRewardAP = reward.AP;
	ansMsg.m_lRewardExp = reward.exp;
	ansMsg.m_lRewardGameMoney = reward.money;

	// ����ġ ���
	ansMsg.m_lErrorCode = CalcLevelByAddExp(lGSN, pNFCharInfo, ansMsg.m_lRewardExp, roomID);
	if (NF::G_NF_ERR_SUCCESS == ansMsg.m_lErrorCode || NF::G_ERR_GET_NF_EXP_MAX == ansMsg.m_lErrorCode ||
		NF::G_ERR_GET_NF_EXP_LEVEL_NOT_FOUND == ansMsg.m_lErrorCode)
	{
		ansMsg.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
		ansMsg.m_lRewardExp = 0;

		if (!theNFDBMgr.UpdateCharExpAndLevel(pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, pNFCharInfo->m_nfCharBaseInfo.m_lExp, pNFCharInfo->m_nfCharBaseInfo.m_lLevel))
		{
			theLog.Put(ERR_UK, "Menu/GetRewardItem/CalcLevelByAddExp UpdateCharExpAndLevel Error!!! CSN :", pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, ", ErrorCode :", ansMsg.m_lErrorCode);
			return FALSE;
		}
	}
	else
	{
		theLog.Put(ERR_UK, "Menu/GetRewardItem/CalcLevelByAddExp Error!!! CSN :", pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, ", ErrorCode :", ansMsg.m_lErrorCode);
		return FALSE;			
	}

	ansMsg.m_lLevel	= pNFCharInfo->m_nfCharBaseInfo.m_lLevel;
	ansMsg.m_lTotalExp = pNFCharInfo->m_nfCharBaseInfo.m_lExp;
	ansMsg.m_nfAbility = pNFCharInfo->m_nfAbility;
	theNFDataItemMgr.GetNFExp(ansMsg.m_lLevel, ansMsg.m_lMaxExp);

	if (!g_achv.getRewardItem(pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, reqMsg.m_lAchvID, ansMsg.m_strRewardDate, nRewardItemID))
	{
		theLog.Put(ERR_UK, "Menu/GetRewardItem/getRewardItem Error!!! CSN :", pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, ", ACHV_ID :", reqMsg.m_lAchvID);
		return FALSE;
	}

	return TRUE;
}

BOOL CNFMenu::GetLockedNoteMain(LONG lGSN, LONG lCSN, AnsLockedNoteMain& ans)
{
	BOOL bRet = theNFDBMgr.SelectNFLockedNoteMain(lGSN, lCSN, ans.m_lockedNoteMain);
	if (!bRet)
		ans.m_lErrorCode = EC_LN_DB_FAIL_SELECT_MAIN;
	return bRet;
}

// �ӽ÷� �̷��� ���� - ���߿� Storage���� �����Ǹ� �̷��� ���� �ؾ� ��
BOOL CNFMenu::GetLockedNoteMap(LONG lGSN, LONG lCSN, LONG lMapID, TMapLockedNoteMap& mapLockedNoteMap, AnsLockedNoteMap& ans)
{
	BOOL bRet = TRUE;
	TMapLockedNoteMap::iterator iter = mapLockedNoteMap.find(lMapID);
	if (iter != mapLockedNoteMap.end())
	{
		ans.m_lMapID = lMapID;
		ans.m_lockedNoteMap = (*iter).second;
	}
	else
	{
		mapLockedNoteMap.clear();		// �ʿ� ���� ������ ���� ��� �ش� �ʿ� ���� ������ �о�� �ϳ�, ����� �ӽ÷� �̷��� ��� ��(���߿� �ٸ��� ����:2012-3-7)

		bRet = theNFDBMgr.SelectNFLockedNoteMap(lGSN, lCSN, mapLockedNoteMap);
		if (!bRet)
		{
			ans.m_lErrorCode = EC_LN_DB_FAIL_SELECT_MAP;
			ans.m_lockedNoteMap.Clear();
			return FALSE;
		}

		bRet = theNFDBMgr.SelectNFLockedNoteFish(lGSN, lCSN, mapLockedNoteMap);
		if (bRet)
		{
			TMapLockedNoteMap::iterator iter = mapLockedNoteMap.find(lMapID);
			if (iter != mapLockedNoteMap.end())
			{
				ans.m_lMapID = lMapID;
				ans.m_lockedNoteMap = (*iter).second;
			}
			else
				ans.m_lockedNoteMap.Clear();		// ������ ����
		}
		else {
			ans.m_lErrorCode = EC_LN_DB_FAIL_SELECT_MAP;
			ans.m_lockedNoteMap.Clear();
		}
	}
	return bRet;
}

LONGLONG CNFMenu::CheckRepairMoney(LONG lItemCode, LONG lGauge, LONG lReinforceLV, LONG& lEndurance, LONG& lErr)
{
	LONGLONG llRepairMoney = 0;

	EquipItem* pItem = theNFDataItemMgr.GetEquipItemByIndex(lItemCode);
	if (!pItem) {
		lErr = NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMCODE;
		return 0;
	}

	double fPrice = (double)pItem->m_llEndurancePrice;
	float fEndurance = (float)pItem->m_lEndurance;
	float fGauge = (float)lGauge;

	// �⺻���� �����Ǵ� ������ ��
	llRepairMoney = (LONGLONG)( (fPrice * 0.8)*((fEndurance - fGauge) / fEndurance) );
	return llRepairMoney;
}

BOOL CNFMenu::RepairEnduranceItem(LONG lGSN, LONG lCSN, NFCharInfoExt* pNFCharInfo, const ReqRepairEnduranceItem& req, AnsRepairEnduranceItem& ans)
{
	if (NULL == pNFCharInfo)
		return FALSE;
	ans.m_lRepairMoney = 0;

	if (req.m_llRepairMoney > pNFCharInfo->m_nfCharBaseInfo.m_llMoney) {
		ans.m_lErrorCode = EC_RI_CLI_BIGGER_SRV;
		return FALSE;
	}

	typedef std::map<LONG, LONGLONG> TMapRepairMoney;
	TMapRepairMoney repairMoney;

	if (1 == req.m_lRepairType)
	{
		TMapInven::iterator iter = pNFCharInfo->m_nfCharInven.m_mapCharInven.find(req.m_lInvenSRL);
		if (iter != pNFCharInfo->m_nfCharInven.m_mapCharInven.end())
		{
			NFInvenSlot& inven = (*iter).second;
			ans.m_lRepairMoney = CheckRepairMoney(inven.m_lItemCode, inven.m_lRemainCount, 1, inven.m_lRemainCount, ans.m_lErrorCode);
			if (req.m_llRepairMoney != ans.m_lRepairMoney)
				ans.m_lErrorCode = NF::EC_RI_NOT_MATCH_MONEY;
			else
			{
				if (ans.m_lRepairMoney > pNFCharInfo->m_nfCharBaseInfo.m_llMoney)
					ans.m_lErrorCode = NF::EC_RI_ENOUGH_MONEY;
			}
			ans.m_mapInven.insert(make_pair(inven.m_lInvenSRL, inven));
			repairMoney.insert(make_pair(inven.m_lInvenSRL, ans.m_lRepairMoney));
		}
		else
		{
			ans.m_lErrorCode = NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN;
			return FALSE;
		}
	}
	else if (2 == req.m_lRepairType)
	{
		ForEachElmt(TMapInven, pNFCharInfo->m_nfCharInven.m_mapCharInven, it, ij)
		{
			LONGLONG lMoney = 0;
			NFInvenSlot& inven = (*it).second;
			if (eItemType_Reel == inven.m_lPartsIndex || eItemType_Rod == inven.m_lPartsIndex)
			{
				if (inven.m_lRemainCount < 0)		// ������ ������
					continue;

				lMoney = CheckRepairMoney(inven.m_lItemCode, inven.m_lRemainCount, 1, inven.m_lRemainCount, ans.m_lErrorCode);
				if (lMoney != 0)
					ans.m_lRepairMoney += lMoney;
				else
					continue;
			}
			else
				continue;

			ans.m_mapInven.insert(make_pair(inven.m_lInvenSRL, inven));
			repairMoney.insert(make_pair(inven.m_lInvenSRL, lMoney));
		}

		if (req.m_llRepairMoney != ans.m_lRepairMoney) {
			ans.m_lErrorCode = NF::EC_RI_NOT_MATCH_MONEY;
			return FALSE;
		}
		else
		{
			if (ans.m_lRepairMoney > pNFCharInfo->m_nfCharBaseInfo.m_llMoney) {
				ans.m_lErrorCode = NF::EC_RI_ENOUGH_MONEY;
				return FALSE;
			}
		}
	}
	else {
		ans.m_lErrorCode = NF::EC_RI_REPAIR_TYPE;
		return FALSE;
	}

	if (NF::G_NF_ERR_SUCCESS == ans.m_lErrorCode)
	{
		ForEachElmt(TMapInven, ans.m_mapInven, it, ij)
		{
			TMapRepairMoney::iterator iter = repairMoney.find((*it).first);
			if (iter != repairMoney.end())
			{
				// DB�� ���� ��Ÿ������ �Ѱ��ֱ� ���ؼ�...
				if (!theNFDBMgr.RechargeGameFunc(lGSN, lCSN, (*it).second, (*it).second.m_lRemainCount, (*iter).second, ans.m_lErrorCode))
				{
					ans.m_lErrorCode = NF::EC_RI_DB_FAILED;
					return FALSE;
				}

				if (ans.m_lErrorCode != NF::G_NF_ERR_SUCCESS)
					return FALSE;

				pNFCharInfo->m_nfCharBaseInfo.m_llMoney = (*iter).second;

				ans.m_lErrorCode = theNFItem.ModifyExistItem(pNFCharInfo->m_nfCharInven, (*it).second);
				if (ans.m_lErrorCode != NF::G_NF_ERR_SUCCESS)
					return FALSE;

				ans.m_lRepairMoney = pNFCharInfo->m_nfCharBaseInfo.m_llMoney;
			}
			else
				return FALSE;
		}
	}
	else
		return FALSE;
	return TRUE;
}

BOOL CNFMenu::NextEnchantInfo(LONG lGSN, LONG lCSN, NFCharInfoExt* pNFCharInfo, const ReqNextEnchantInfo& req, AnsNextEnchantInfo& ans)
{
	if (NULL == pNFCharInfo)
		return FALSE;

	TMapInven::const_iterator c_itor = pNFCharInfo->m_nfCharInven.m_mapCharInven.find(req.m_lInvenSRL);
	if (c_itor != pNFCharInfo->m_nfCharInven.m_mapCharInven.end())
	{
		LONG lEnchantType = c_itor->second.m_lItemCode / G_VALUE_CONVERT_PARTS;
		LONG lEnchantLevel = c_itor->second.m_lEnchantLevel;

		if (9 <= lEnchantLevel)
		{
			ans.m_lErrorCode = EC_IE_MAX_ENCHANT;
			return FALSE;
		}

		if( c_itor->second.m_bIsUsing )
		{
			ans.m_lErrorCode = EC_IE_EQUIP;
			return FALSE;
		}

		NFItemEnchantInfo nfItemEnchantInfo;
		if (theNFDataItemMgr.GetNFItemEnchantInfo(lEnchantType, lEnchantLevel + 1, nfItemEnchantInfo))
		{	
			ans.m_nfAddAbility = nfItemEnchantInfo.nfAddAbility;
			return TRUE;
		}
		else
		{
			ans.m_lErrorCode = EC_IE_NOT_ENCHANT_TYPE;
			return FALSE;
		}
	}
	else
	{
		ans.m_lErrorCode = G_NF_ERR_FAIL;
		return FALSE;
	}
}

BOOL CNFMenu::ItemEnchant(LONG lGSN, LONG lCSN, NFCharInfoExt* pNFCharInfo, const ReqItemEnchant& req, AnsItemEnchant& ans)
{
	if (NULL == pNFCharInfo)
		return FALSE;

	TMapInven::iterator enchant_item_itor = pNFCharInfo->m_nfCharInven.m_mapCharInven.find(req.m_lEnchantItemInvenSRL);
	if (enchant_item_itor != pNFCharInfo->m_nfCharInven.m_mapCharInven.end())
	{
		ans.m_nfInvenSlot = enchant_item_itor->second;

		LONG lEnchantType = enchant_item_itor->second.m_lItemCode / G_VALUE_CONVERT_PARTS;
		LONG lNextEnchantLevel = enchant_item_itor->second.m_lEnchantLevel + 1;

		if (lEnchantType != eItemType_Rod && lEnchantType != eItemType_Reel)
		{
			ans.m_lErrorCode = EC_IE_NOT_ENCHANT_TYPE;
			return FALSE;
		}

		if (9 < lNextEnchantLevel)
		{
			ans.m_lErrorCode = EC_IE_MAX_ENCHANT;
			return FALSE;
		}

		if( enchant_item_itor->second.m_bIsUsing )
		{
			ans.m_lErrorCode = EC_IE_EQUIP;
			return FALSE;
		}
		
		EquipItem* pItem = theNFDataItemMgr.GetEquipItemByIndex(enchant_item_itor->second.m_lItemCode);
		if (!pItem)
		{
			ans.m_lErrorCode = G_NF_ERR_FAIL;
			return FALSE;
		}

		if( enchant_item_itor->second.m_lRemainCount < pItem->m_lEndurance )
		{
			ans.m_lErrorCode = EC_IE_GAUGE;
			return FALSE;
		}

		// ��ȭī�� üũ
		TMapInvenSlotList::iterator enchant_card_itor = pNFCharInfo->m_nfCharInven.m_mapCountableItem.find(req.m_lEnchantCardItemCode);
		if(enchant_card_itor == pNFCharInfo->m_nfCharInven.m_mapCountableItem.end())
		{
			ans.m_lErrorCode = EC_IE_NOT_EXIST_ENCHANT_CARD;
			return FALSE;
		}

		LONG lEnchantEnableLevelMin = 0;
		LONG lEnchantEnableLevelMax = 0;		

		if (req.m_lEnchantCardItemCode == 300081) // ��ȭī��1�� 1~3���� ����
		{
			lEnchantEnableLevelMin = 1;
			lEnchantEnableLevelMax = 3;
		}
		else if (req.m_lEnchantCardItemCode == 300082) // ��ȭī��2�� 4~6���� ����
		{
			lEnchantEnableLevelMin = 4;
			lEnchantEnableLevelMax = 6;
		}
		else if (req.m_lEnchantCardItemCode == 300083) // ��ȭī��3�� 7~9���� ����
		{
			lEnchantEnableLevelMin = 7;
			lEnchantEnableLevelMax = 9;
		}

		if (lNextEnchantLevel < lEnchantEnableLevelMin || lEnchantEnableLevelMax < lNextEnchantLevel)
		{
			ans.m_lErrorCode = EC_IE_NOT_ENCHANT_CARD_GRADE;
			return FALSE;
		}

		// ��� �� ��ȭī��
		NFInvenSlot card;
		card.Clear();
		card.m_lItemCode = req.m_lEnchantCardItemCode;

		LONG lErr = FindReduceInvenSlot(pNFCharInfo->m_nfCharInven.m_mapCountableItem, card, req.m_lEnchantCardItemCode);
		if (NF::G_NF_ERR_SUCCESS != lErr)
		{
			ans.m_lErrorCode = EC_IE_NOT_EXIST_ENCHANT_CARD;
			return FALSE;
		}

		// ��� �� ���� ������
		NFInvenSlot sub;
		sub.Clear();
		sub.m_lItemCode = req.m_lAssistItemCode;

		if (0 < req.m_lAssistItemCode)
		{
			lErr = FindReduceInvenSlot(pNFCharInfo->m_nfCharInven.m_mapCountableItem, sub, req.m_lAssistItemCode);
			if (NF::G_NF_ERR_SUCCESS != lErr)
			{
				ans.m_lErrorCode = EC_IE_NOT_FOUND_ASSIST_ITEM;
				return FALSE;
			}
		}

		// ��ȭ ����
		NFItemEnchantInfo nfItemEnchantInfo;
		if (!theNFDataItemMgr.GetNFItemEnchantInfo(lEnchantType, lNextEnchantLevel, nfItemEnchantInfo))
		{
			ans.m_lErrorCode = G_NF_ERR_FAIL;
			return FALSE;
		}

		LONG lEnchantResultType = 0;
		
		// ��ȭ Ȯ��
		long lSuccessVal = static_cast<long>(nfItemEnchantInfo.dSuccessRate * 100); // TODO: ����Ȯ������ ������ Ȯ�� PLUS
		long lRandomVal = urandom(10000);
		if (lRandomVal < lSuccessVal) // ��ȭ ����
		{
			lEnchantResultType = 1;
		}
		else // ��ȭ ����
		{
			// TODO: ����������, ������ �ı� ���� ������ ����

			long lDestroyVal = static_cast<long>(nfItemEnchantInfo.dDestroyRate *100);
			long lRandomVal = urandom(10000);
			if( lRandomVal < lDestroyVal ) // ������ �ı�
			{
				lEnchantResultType = 2;
			}
			else // ������ ����
			{
				lEnchantResultType = 3;
			}
		}
		
		LONG lCardGauge = 0;
		LONG lSubGauge = 0 ;
		
		if (!theNFDBMgr.EnchantItem(lGSN, lCSN, enchant_item_itor->second.m_lItemCode, enchant_item_itor->second.m_lInvenSRL, card.m_lItemCode, card.m_lInvenSRL, sub.m_lItemCode, sub.m_lInvenSRL, lEnchantResultType,
			ans.m_lErrorCode, lCardGauge, lSubGauge) || ans.m_lErrorCode != G_NF_ERR_SUCCESS)
		{
			ans.m_lErrorCode = NF::G_NF_ERR_DB;
			return FALSE;
		}

		card.m_lRemainCount = lCardGauge;
		sub.m_lRemainCount = lSubGauge;
		
		//////////////////////////////////////////////////////////////////////////
		// DB���� ���ν��� ȣ�� ������ �Ŀ� �޸� ����

		// ��ȭī�� �Ҹ�
		if (lCardGauge <= 0)
		{
			theNFItem.RemoveItem(pNFCharInfo->m_nfCharInven, card);
		}
		else
		{	
			SetExistCountableItemToInven(pNFCharInfo->m_nfCharInven, card);
		}

		ans.m_lstChangedInven.push_back(card);

		// ���������� �Ҹ�
		if (0 < req.m_lAssistItemCode)
		{
			if (lSubGauge <= 0)
			{
				theNFItem.RemoveItem(pNFCharInfo->m_nfCharInven, sub);
			}
			else
			{	
				SetExistCountableItemToInven(pNFCharInfo->m_nfCharInven, sub);
			}

			ans.m_lstChangedInven.push_back(sub);
		}

		if (1 == lEnchantResultType) // ��ȭ����
		{
			enchant_item_itor->second.m_lEnchantLevel += 1;
			pNFCharInfo->m_nfAbility += nfItemEnchantInfo.nfAddAbility;

			ans.m_bSuccess = TRUE;
			ans.m_nfAddAbility = nfItemEnchantInfo.nfAddAbility;
			ans.m_nfInvenSlot = enchant_item_itor->second;
		}
		else if(2 == lEnchantResultType) // ������ �ı�
		{
			pNFCharInfo->m_nfCharInven.m_mapCharInven.erase(enchant_item_itor);
			ans.m_bDestroy = TRUE;
		}
		else if(3 == lEnchantResultType) // ������ 0
		{
			enchant_item_itor->second.m_lRemainCount = 0;
			ans.m_nfInvenSlot = enchant_item_itor->second;
		}
	}
	else
	{
		ans.m_lErrorCode = EC_IE_NOT_FOUND_ITEM;
		return FALSE;
	}

	return TRUE;
}

BOOL CNFMenu::CalcNFAquaGauge(NFCharInfoExt* pNFCharInfo, LONG lElapsedClearHour, LONG lElapsedFeedHour)
{
	// ������, û�ᵵ ������ ���
	// ������ = DB�� ����� ������ - ( (����ð� - ���ؽð�) * 2.084 ) --> DB�� ����� �������� �� �ٶ��� ����, 48�ð��� ����ϸ� 0%�� �Ǿ��ϴϱ� �ð��� 2.084% ����
	// û�ᵵ = DB�� ����� û�ᵵ - ( (����ð� - û�ҽð�) * ��������޴� %) )
	// 1~2�ܰ� 0.84, 3~4�ܰ� 1.042, 5�ܰ� 1.39

	if (!pNFCharInfo)
		return FALSE;

	pNFCharInfo->m_nfAqua.m_dFeedGauge -= lElapsedFeedHour * 2.084;
	if (pNFCharInfo->m_nfAqua.m_dFeedGauge <= 0.0)
	{
		pNFCharInfo->m_nfAqua.m_dFeedGauge = 0.0;
	}

	double dClearDecreaseRate = 0.0;

	if (pNFCharInfo->m_nfAqua.m_lAquaLevel <= 2)
	{
		dClearDecreaseRate = 0.84;
	}
	else if (pNFCharInfo->m_nfAqua.m_lAquaLevel <= 4)
	{
		dClearDecreaseRate = 1.042;
	}
	else
	{
		dClearDecreaseRate = 1.39;
	}

	pNFCharInfo->m_nfAqua.m_dClearGauge -= lElapsedClearHour * dClearDecreaseRate;
	if (pNFCharInfo->m_nfAqua.m_dClearGauge <= 0.0)
	{
		pNFCharInfo->m_nfAqua.m_dClearGauge = 0.0;
	}

	return TRUE;
}

BOOL CNFMenu::InsertAquaFish(LONG lGSN, LONG lCSN, NFCharInfoExt* pNFCharInfo, LONG lFishID, LONG lLength, LONG lWeight, LONG lElapsedTime, LONG lScore, LONG& lErrorCode)
{
	lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!pNFCharInfo)
	{
		lErrorCode = NF::G_NF_ERR_NOT_FOUND_USER_NGS;
		return FALSE;
	}

	LONG lMaxFishCount = 0;
	LONG lAquaLevel = pNFCharInfo->m_nfAqua.m_lAquaLevel;

	switch (lAquaLevel)
	{
	case 1: lMaxFishCount = 10; break;
	case 2: lMaxFishCount = 15; break;
	case 3: lMaxFishCount = 20; break;
	case 4: lMaxFishCount = 30; break;
	case 5: lMaxFishCount = 40; break;
	default: break;
	}

	if (lMaxFishCount <= static_cast<LONG>(pNFCharInfo->m_nfAquaFish.size()))
	{
		lErrorCode = NF::EC_NA_MAX_AQUA_FISH;
		return FALSE;
	}

	if (!theNFDBMgr.InsertNFCharAquaFish(lCSN, lCSN, lFishID, lLength, lWeight, lElapsedTime, lScore))
	{
		lErrorCode = NF::G_NF_ERR_DB;
		return FALSE;
	}

	// NF_GT_CHAR_AQUA�� Score ������Ʈ.. TODO(acepm83@neowiz.com) ���� ������ �Բ� ���ν����� ����
	// �켱 �ӽ÷� �޸𸮸�
	pNFCharInfo->m_nfAqua.m_lAquaScore += lScore;

	return TRUE;
}

BOOL CNFMenu::GetAquaFish(LONG lCSN, TMapNFAquaFish& mapNFAquaFish)
{	
	if (!theNFDBMgr.SelectNFCharAquaFish(lCSN, mapNFAquaFish))
		return FALSE;

	return TRUE;
}