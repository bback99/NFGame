
#include "stdafx.h"
#include <NF/NFErrorCode.h>
#include <NFVariant/NFItem.h>
#include <NFVariant/NFGameData.h>
#include <NFVariant/NFMenu.h>
#include <NFVariant/NFDBManager.h>


//// ACHV BEGIN
#include <ACHV/AchvDef.h>
static achv::CAchvMgr& g_achv = achv::CAchvMgr::Instance();
//// ACHV END



CNFItem		theNFItem;

CNFItem::CNFItem()
{

}

CNFItem::~CNFItem()
{

}

size_t CNFItem::GetInvenSize(NFCharInven& mapInven)
{
	size_t size = mapInven.m_mapCharInven.size();

	ForEachElmt(TMapInvenSlotList, mapInven.m_mapCountableItem, it, ij)
		size += (*it).second.size();

	return size;
}

LONG CNFItem::CheckValid_Level(ItemCommon* pItemCommon, LONG lCharLevel)
{
	if (pItemCommon->m_lLv > lCharLevel)
		return EC_CP_LEVEL;
	return NF::G_NF_ERR_SUCCESS;
}

LONG CNFItem::GetUsingItemCode(NFCharInfo* pInfo, int nUsingItemType, LONG& lItemCode)
{
	if (NULL == pInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	TMapInven::const_iterator iter = pInfo->m_nfCharInven.m_mapUsingItem.find(nUsingItemType);
	if (iter != pInfo->m_nfCharInven.m_mapUsingItem.end())
		lItemCode = (*iter).second.m_lItemCode;
	else
	{
		switch(nUsingItemType)		// �ʼ������� �̳���� ItemCode�� ��ȿ�ؾ� �Ѵ�...
		{
		case eItemType_Lure:
		case eItemType_Rod:
		case eItemType_Reel:
		case eItemType_Line:
			return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_USING;
		}
	}

	return NF::G_NF_ERR_SUCCESS;
}

LONG CNFItem::GetUsingItemEnchantLevel(NFCharInfo* pInfo, int nUsingItemType)
{
	if (NULL == pInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	TMapInven::const_iterator iter = pInfo->m_nfCharInven.m_mapUsingItem.find(nUsingItemType);
	if (iter != pInfo->m_nfCharInven.m_mapUsingItem.end())
	{
		return iter->second.m_lEnchantLevel;
	}
	
	return 0;
}

// ������ ���޽� capacity �� ���Ͽ� ���ο� �κ� or ���� �κ��� ��ġ�� or ���ο� �κ� & ���� �κ��� �ֱ⸦ �Ǵ��ϴ� �Լ�
// 1. BuyItem ȣ��� ���
// 2. FishDrop �� ���
// 3. RewardItem ���޽� ���
LONG CNFItem::ProcessCapacityInven(NFCharInven& mapInven, LONG lTempInvenSRL, NFInvenSlot& oldInven, NFInvenSlot& newInven, LONG lItemCNT, LONG lCapacity, TMapInven& mapOld, TMapInven& mapNew)
{
	LONG lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (lTempInvenSRL == oldInven.m_lInvenSRL)		// ���� �޸𸮿� DB���� ����� ���� �����Ƿ�, ������Ʈ 
	{
		oldInven.m_lRemainCount += lItemCNT;
		lErrorCode = theNFItem.ModifyExistItem(mapInven, oldInven);
		mapOld.insert(make_pair(oldInven.m_lInvenSRL, oldInven));
	}
	else		// ������ MAX�� �����
	{	
		newInven = oldInven;			

		if (lTempInvenSRL == 0)		// ������ ���� ���� �߰�
		{	
			newInven.m_lInvenSRL = oldInven.m_lInvenSRL;
			newInven.m_lRemainCount = lItemCNT;
			lErrorCode = theNFItem.AddInvenSlotItem(mapInven, newInven);
			mapNew.insert(make_pair(newInven.m_lInvenSRL, newInven));
		}
		else	// ������ DB�� ���� �ʰ�, �������� ������ ���� ���� �ִٸ�.... �߰� & ������ ����
		{
			// �������� capacity��ŭ
			LONG lRemainCount = lCapacity - oldInven.m_lRemainCount;
			oldInven.m_lInvenSRL = lTempInvenSRL;
			oldInven.m_lRemainCount = lCapacity;
			lErrorCode = theNFItem.ModifyExistItem(mapInven, oldInven);
			if (NF::G_NF_ERR_SUCCESS != lErrorCode)
				return lErrorCode;
			mapOld.insert(make_pair(oldInven.m_lInvenSRL, oldInven));

			// ���ο�� capacity���� �������� ���� ���� �������� product ī��Ʈ���� ����
			newInven.m_lRemainCount = lItemCNT - lRemainCount;
			lErrorCode = theNFItem.AddInvenSlotItem(mapInven, newInven);
			if (NF::G_NF_ERR_SUCCESS != lErrorCode)
				return lErrorCode;
			mapNew.insert(make_pair(newInven.m_lInvenSRL, newInven));
		}
	}
	return lErrorCode;
}

// bIsUsingToggle�� TRUE�̸�, Using�� Toggle�� �ݴ�� �����Ѵ�.
LONG CNFItem::GetNFInvenSlot(NFCharInven& nfCharInven, LONG lItemCode, LONG lInvenSRL, BOOL bIsUsingToggle, NFInvenSlot& inven)
{
	LONG lErrCode = NF::G_NF_ERR_SUCCESS;

	LONG lParts = lItemCode/G_VALUE_CONVERT_PARTS;
	if (IsCountableItem(lParts))
	{
		// Inven���� ������ �ִ� ���������� üũ
		TMapInvenSlotList::iterator	iter = nfCharInven.m_mapCountableItem.find(lItemCode);		
		if (iter != nfCharInven.m_mapCountableItem.end())
		{
			// Using���� �� �����Ϸ���, list�� �ִ� ��� �͵��� �� �ٲ�� �Ѵ�.
			ForEachElmt(TlstInvenSlot, (*iter).second, itSlot, ijSlot) 
			{
				if ((*itSlot).m_lInvenSRL == lInvenSRL) 
				{
					if (bIsUsingToggle)
						(*itSlot).m_bIsUsing = !((*itSlot).m_bIsUsing);

					inven = (*itSlot); // ���� �� �����;� �Ѵ�.
					break;
				}	
			}
		}
		else
			return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;
	}
	else
	{
		TMapInven::iterator	iter = nfCharInven.m_mapCharInven.find(lInvenSRL);		
		if (iter != nfCharInven.m_mapCharInven.end())
		{
			if (bIsUsingToggle)
				(*iter).second.m_bIsUsing = !((*iter).second.m_bIsUsing);

			inven = (*iter).second;
		}
		else
			return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN;
	}

	if (inven.m_lInvenSRL == 0 || inven.m_lItemCode == 0)
		return NF::G_NF_ERR_INVALID_ITEM_SRL;

	return lErrCode;
}

BOOL CNFItem::GetExistToInvenList(NFCharInven& mapInven, LONG lItemCode, BOOL bIsUsingToggle, string& strStartDate, string& strEndDate, TlstInvenSlot& lst, LONG& lErrorCode)
{
	TMapInvenSlotList::iterator	iter = mapInven.m_mapCountableItem.find(lItemCode);		// lItemCode => ItemCode
	if (iter != mapInven.m_mapCountableItem.end())
	{
		if (bIsUsingToggle)
		{
			ForEachElmt(TlstInvenSlot, (*iter).second, it, ij)
			{
				(*it).m_bIsUsing = !((*it).m_bIsUsing);

				if ((*it).m_lPartsIndex == eItemType_SkillItem)		// ��ų �������̸� ��¥ ���� �ؾ� �Ѵ�...
				{
					(*it).m_strBuyDate = strStartDate;
					(*it).m_strExpireDate = strEndDate;
				}
			}
		}

		lst = (*iter).second;
	}
	else {
		lErrorCode = NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;
		return FALSE;
	}

	return TRUE;
}

LONG CNFItem::CheckInvenIsFull(NFCharInven& mapInven)
{
	int nMaxCount = G_MAX_BASIC_INVEN_CNT;

	TMapInven::iterator it = mapInven.m_mapUsingItem.find(eItemType_Bags);
	if (it != mapInven.m_mapUsingItem.end())
	{
		ClothesItem* pItem = theNFDataItemMgr.GetClothesItemByIndex((*it).second.m_lItemCode);
		if (pItem == NULL)
			return NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMCODE;

		nMaxCount = pItem->m_lCount;		// ������ ���� �ϰ� ������ �ִ� ����(Count)���� ������ �´�...2011/11/23 - ��ȹ���� ���� ��
	}

	if ((int)GetInvenSize(mapInven) >= nMaxCount)
		return NF::G_NF_ERR_ADD_INVEN_SIZE_FULL;		// MAX 

	return NF::G_NF_ERR_SUCCESS;	// possible
}

ItemCommon* CNFItem::GetItemCommon(NFInvenSlot& inven)
{
	ItemCommon* pItemCommon = NULL;

	// ���� ���� �ϴ� �����ۿ��� ���� üũ
	if (inven.m_lPartsIndex >= eItemType_Lure && inven.m_lPartsIndex < eItemType_UsableItem)
		pItemCommon = theNFDataItemMgr.GetEquipItemByIndex(inven.m_lItemCode);
	else if (inven.m_lPartsIndex >= eItemType_Acce && inven.m_lPartsIndex < eItemType_Lure)
		pItemCommon = theNFDataItemMgr.GetClothesItemByIndex(inven.m_lItemCode);
	
	return pItemCommon;
}

LONG CNFItem::GetChar_BasicAbility(LONG lBasicCharIndex, NFAbility& nfAbility)
{
	NFAbility	nfBasicAbility;
	if (!theNFDataItemMgr.GetNFAbility(lBasicCharIndex, nfAbility))
		return NF::G_NF_ITEM_NOT_FOUND_BASIC_CHAR_ABILITY;
	return NF::G_NF_ERR_SUCCESS;
}

LONG CNFItem::GetChar_LevelAbility(LONG lLevel, NFAbility& nfAbility)
{
	TMapNFLevel& mapNFLevel = theNFDataItemMgr.GetNFLevel();
	TMapNFLevel::const_iterator iter = mapNFLevel.find(lLevel);
	if( iter != mapNFLevel.end() )
		nfAbility += (*iter).second->m_ability;
	else
		return NF::G_NF_ITEM_NOT_FOUND_LEVEL_ABILITY;
	return NF::G_NF_ERR_SUCCESS;
}

// WORKING(acepm83@neowiz.com) ������ ������ ���� �ɷ�ġ �߰�(����)
// ��������� ������ �ߺ��ؼ� ������ �Ѵ�.
LONG CNFItem::GetAquaBuff(LONG lAquaScore, double dFeedGauge, double dClearGauge, NFAbility& nfAbility)
{
	// û�ᵵ, �������� ��� 0%�̸� ���� ����
	if (dClearGauge <= 0.0 && dFeedGauge <= 0.0)
	{
		return NF::G_NF_ERR_SUCCESS;
	}

	NFAbility nfBuff;
	nfBuff.Clear();

	if (NAB_GUARD <= lAquaScore)
	{
		nfBuff.m_dControl = 5.0; // ����5
	}

	if (NAB_LIFE <= lAquaScore)
	{
		nfBuff.m_dHealth = 10.0; // ��10
	}

	if (NAB_LIGHT <= lAquaScore)
	{
		nfBuff.m_dAgility = 15.0; // ��ø15
	}

	if (NAB_STR <= lAquaScore)
	{
		nfBuff.m_dStrength = 25.0; // ��25
	}

	if (NAB_PROTECTION <= lAquaScore)
	{
		nfBuff.m_dLuckyPoint = 30.0; // ��30
	}

	// û�ᵵ, �������� �ϳ��� 0%�̸� ������ 50%�� ����
	if (dClearGauge <= 0.0 || dFeedGauge <= 0.0)
	{
		nfBuff *= 0.5;
	}

	nfAbility += nfBuff;

	return NF::G_NF_ERR_SUCCESS;
}

LONG CNFItem::AddEquipedItemTotalAbility(NFCharInfo* pInfo, NFAbility& nfAbility)
{
	if (NULL == pInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErr = NF::G_NF_ERR_SUCCESS;

	// clothes
	for(int nIndex = eItemType_Acce; nIndex <= eItemType_Bags; nIndex++)
	{
		LONG lItemCode = 0;
		lErr = GetUsingItemCode(pInfo, nIndex, lItemCode);
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;

		lErr = GetChar_EquipedClothItemAbility(pInfo, lItemCode, nfAbility);
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;
	}

	// equip
	// "6-1-3. ä�� ������ Ÿ�Կ� ���� �ɷ�ġ ����"��� ��ȹ ���뿡 ���ؼ� ������ Ÿ���� �ٸ� ��� �г�Ƽ�� �ο��ؾ� �Ѵ�.
	for(int nIndex = eItemType_Lure; nIndex <= eItemType_FishDetector; nIndex++)
	{
		LONG lItemCode = 0;
		lErr = GetUsingItemCode(pInfo, nIndex, lItemCode);
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;

		LONG lEnchantLevel = GetUsingItemEnchantLevel(pInfo, nIndex);

		lErr = GetChar_EquipedEquipItemAbility(pInfo, nIndex, lItemCode, lEnchantLevel, nfAbility);
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;
	}
	return lErr;
}

LONG CNFItem::ModifyExistItem(NFCharInven& mapInven, const NFInvenSlot& inven)
{
	LONG lErr = NF::G_NF_ERR_SUCCESS;

	if (0 == inven.m_lInvenSRL)
		return NF::G_NF_ERR_INVALID_ITEM_SRL;

	if (IsCountableItem(inven.m_lPartsIndex))
	{
		TMapInvenSlotList::iterator iter = mapInven.m_mapCountableItem.find(inven.m_lItemCode);
		if (iter != mapInven.m_mapCountableItem.end())
		{
			ForEachElmt(TlstInvenSlot, (*iter).second, it, ij) 
			{
				if ((*it).m_lInvenSRL == inven.m_lInvenSRL) {
					(*it) = inven;
					return NF::G_NF_ERR_SUCCESS;
				}
			}
		}
		else
			lErr = NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN;
	}
	else
	{
		TMapInven::iterator iter = mapInven.m_mapCharInven.find(inven.m_lInvenSRL);
		if (iter != mapInven.m_mapCharInven.end())
			(*iter).second = inven;
		else
			lErr = NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN;
	}
	return lErr;
}

LONG CNFItem::CalcChangedItemAbility(NFCharInfo* pInfo, LONG lPartsIndex, const NFInvenSlot& invenOld, const NFInvenSlot& invenNew, NFAbility& nfAbility)
{
	if (NULL == pInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErr = NF::G_NF_ERR_SUCCESS;

	if (lPartsIndex >= eItemType_Acce && lPartsIndex <= eItemType_Bags)
	{
		lErr = GetChar_EquipedClothItemAbility(pInfo, invenOld.m_lItemCode, nfAbility, FALSE);					// �ɷ�ġ ����
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;

		lErr = GetChar_EquipedClothItemAbility(pInfo, invenNew.m_lItemCode, nfAbility);							// �ɷ�ġ ����
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;
	}

	if (lPartsIndex >= eItemType_Lure && lPartsIndex <= eItemType_FishDetector)
	{
		lErr = GetChar_EquipedEquipItemAbility(pInfo, lPartsIndex, invenOld.m_lItemCode, invenOld.m_lEnchantLevel, nfAbility, FALSE);		// �ɷ�ġ ����
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;

		lErr = GetChar_EquipedEquipItemAbility(pInfo, lPartsIndex, invenNew.m_lItemCode, invenNew.m_lEnchantLevel, nfAbility);				// �ɷ�ġ ����
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;
	}

	return lErr;
}

LONG CNFItem::GetChar_EquipedEquipItemAbility(NFCharInfo* pInfo, LONG nEquipedEquipItemType, LONG lItemCode, LONG lEnchantLevel, NFAbility& nfAbility, BOOL bIsAdd)
{
	if (lItemCode <= 0)
	{
		switch(nEquipedEquipItemType)		// �ʼ������� �̳���� ItemCode�� ��ȿ�ؾ� �Ѵ�...
		{
		case eItemType_Lure:
		case eItemType_Rod:
		case eItemType_Reel:
		case eItemType_Line:
			return NF::G_NF_ITEM_NOT_FOUND_EQUIP_ITEM;
		}
		return NF::G_NF_ERR_SUCCESS;
	}
	
	EquipItem* pMyItem = theNFDataItemMgr.GetEquipItemByIndex(lItemCode);
	if (NULL == pMyItem)
		return NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMCODE;

	// Valid Check(ex. Level, Grade)
	LONG lErr = CheckValid_Level(pMyItem, pInfo->m_nfCharBaseInfo.m_lLevel);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;	

	int nCheckOtherItemType = 0;
	switch(nEquipedEquipItemType)
	{
	case eItemType_Reel:
		{
			nCheckOtherItemType = eItemType_Line;		// Line�� ItemType üũ
			break;
		}
	case eItemType_Rod:
		{
			nCheckOtherItemType = eItemType_Reel;		// Reel�� ItemType üũ
			break;
		}
	default: // ���, ��Ʈ, ����Ž����� �ش� �������� �ɷ�ġ�� ���Ѵ�.
		{
			if (TRUE == bIsAdd)
				nfAbility += pMyItem->m_nfAbilityExt.m_nfAbility;
			else
				nfAbility -= pMyItem->m_nfAbilityExt.m_nfAbility;
			return NF::G_NF_ERR_SUCCESS; 
		}
	}

	TMapInven::const_iterator iterOther = pInfo->m_nfCharInven.m_mapUsingItem.find(nCheckOtherItemType);
	if (iterOther == pInfo->m_nfCharInven.m_mapUsingItem.end())
		return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN;

	LONG lOtherItemCode = (*iterOther).second.m_lItemCode;
	EquipItem* pOtherItem = theNFDataItemMgr.GetEquipItemByIndex(lOtherItemCode);
	if (NULL == pOtherItem)
		return NF::G_NF_ITEM_NOT_FOUND_EQUIP_OTHER_ITEM;

	// �����۰�ȭ �ܰ迡 ���� ������ �ɷ�ġ�� ���Ѵ�.
	NFAbility nfEnchantAddAbility;
	nfEnchantAddAbility.Clear();
	for (int i = 1; i <= lEnchantLevel; ++i)
	{
		NFItemEnchantInfo nfItemEnchantInfo;
		theNFDataItemMgr.GetNFItemEnchantInfo(nEquipedEquipItemType, i, nfItemEnchantInfo);
		nfEnchantAddAbility += nfItemEnchantInfo.nfAddAbility;
	}

	float fPenaltyRate = 1.0f;		// 100%
	int nGap = abs(pOtherItem->m_lEquipItemType - pMyItem->m_lEquipItemType);
	if (2 == nGap)			fPenaltyRate = 0.50f;		// 100%

	NFAbility penaltyAbilityRate = pMyItem->m_nfAbilityExt.m_nfAbility;
	penaltyAbilityRate += nfEnchantAddAbility; // �����۰�ȭ �ܰ迡 ���� ������ �ɷ�ġ
	penaltyAbilityRate *= fPenaltyRate;

	if (bIsAdd)
		nfAbility += penaltyAbilityRate;
	else
		nfAbility -= penaltyAbilityRate;

	return NF::G_NF_ERR_SUCCESS;
}

LONG CNFItem::GetChar_EquipedClothItemAbility(NFCharInfo* pInfo, LONG lItemCode, NFAbility& nfAbility, BOOL bIsAdd)
{
	if (lItemCode > 0)
	{
		ClothesItem* pItem = theNFDataItemMgr.GetClothesItemByIndex(lItemCode);
		if (NULL != pItem)
		{
			// Valid Check(ex. Level, Grade)
			LONG lErr = CheckValid_Level(pItem, pInfo->m_nfCharBaseInfo.m_lLevel);
			if (lErr != NF::G_NF_ERR_SUCCESS)
				return lErr;

			if (TRUE == bIsAdd)
				nfAbility += pItem->m_nfAbilityExt.m_nfAbility;
			else
				nfAbility -= pItem->m_nfAbilityExt.m_nfAbility;
		}
		else
			return NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMCODE;
	}

	return NF::G_NF_ERR_SUCCESS;	// �ǻ�������� Naked�� ������ ���, MyInven�� ���ٰ� �����Ƿ� ������ �ƴ�...
}

void CNFItem::AddUsingInven(TMapInven& usingInven, const NFInvenSlot& add_inven)
{
	usingInven[add_inven.m_lPartsIndex] = add_inven;
}

void CNFItem::RemoveUsingInven(TMapInven& usingInven, LONG lPartsIndex)
{
	TMapInven::iterator it = usingInven.find(lPartsIndex);
	if (it != usingInven.end())
        usingInven.erase(it);
}

// ���� inven�� �߰� �ϴ� �Լ�
// �κ� �ִ� ������ ������ ����(eItemType_Bags)�� ���� �� �ִ� �ִ� ������ üũ �Ѵ�.
LONG CNFItem::AddInvenSlotItem(ArcVectorT< LONG >& slot, NFCharInven& mapInven, NFInvenSlot& inven)
{
	LONG lErr = CheckInvenIsFull(mapInven);
	if (NF::G_NF_ERR_SUCCESS != lErr)	return lErr;

	// itemCode->InvenSRL list
	if(IsCountableItem(inven.m_lPartsIndex))
	{
		ForEachElmt(ArcVectorT< LONG >, slot, it, ij) {
			if (inven.m_lItemCode == (*it)) {
				inven.m_bIsUsing = TRUE;
				break;
			}
		}

		TMapInvenSlotList::iterator iter = mapInven.m_mapCountableItem.find(inven.m_lItemCode);	// ItemCode
		if (iter != mapInven.m_mapCountableItem.end()) 
		{
			ForEachElmt(TlstInvenSlot, (*iter).second, it2, ij2)
			{
				if ((*it2).m_lInvenSRL == inven.m_lInvenSRL)
					return NF::G_NF_ERR_ADD_INVEN_COUNTABLE;

				if (eItemType_UsableItem == (*it2).m_lPartsIndex)		// �ӽ��ڵ�(usable�� �Ұ�����...???)
					inven.m_bIsUsing = (*it2).m_bIsUsing;
			}
			(*iter).second.push_back(inven);
		}
		else
		{
			TlstInvenSlot	lst;
			lst.push_back(inven);
			mapInven.m_mapCountableItem.insert( make_pair(inven.m_lItemCode, lst) );
		}
	}
	else
	{	// itemCode->InvenSRL list
		TMapInven::iterator iter = mapInven.m_mapCharInven.find(inven.m_lInvenSRL);				// InvenSRL
		if (iter == mapInven.m_mapCharInven.end())
			mapInven.m_mapCharInven.insert( make_pair(inven.m_lInvenSRL, inven) );
		else
			return NF::G_NF_ERR_ADD_INVEN_INVEN;
	}
	return NF::G_NF_ERR_SUCCESS;
}

bool SortItemRemainCNT(const NFInvenSlot& elem1, const NFInvenSlot& elem2)
{
	return elem1.m_lRemainCount < elem2.m_lRemainCount;		//�������� ���� 
}

LONG CNFItem::AddInvenSlotItem(NFCharInven& mapInven, NFInvenSlot& inven)
{
	LONG lErr = CheckInvenIsFull(mapInven);
	if (NF::G_NF_ERR_SUCCESS != lErr)	return lErr;

	// itemCode->InvenSRL list
	if (IsCountableItem(inven.m_lPartsIndex))
	{
		TMapInvenSlotList::iterator iter = mapInven.m_mapCountableItem.find(inven.m_lItemCode);	// ItemCode
		if (iter != mapInven.m_mapCountableItem.end()) 
		{
			ForEachElmt(TlstInvenSlot, (*iter).second, it2, ij2)
			{
				if ((*it2).m_lInvenSRL == inven.m_lInvenSRL)
					return NF::G_NF_ERR_ADD_INVEN_COUNTABLE;

				if (eItemType_UsableItem == (*it2).m_lPartsIndex)		// �ӽ��ڵ�(usable�� �Ұ�����...???)
					inven.m_bIsUsing = (*it2).m_bIsUsing;
			}

			(*iter).second.push_back(inven);

			// sorting remain_count��...
			(*iter).second.sort(SortItemRemainCNT);
		}
		else
		{
			TlstInvenSlot	lst;
			lst.push_back(inven);
			mapInven.m_mapCountableItem.insert( make_pair(inven.m_lItemCode, lst) );
		}
	}
	else
	{	// itemCode->InvenSRL list
		TMapInven::iterator iter = mapInven.m_mapCharInven.find(inven.m_lInvenSRL);				// InvenSRL
		if (iter == mapInven.m_mapCharInven.end())
			mapInven.m_mapCharInven.insert( make_pair(inven.m_lInvenSRL, inven) );
		else
			return NF::G_NF_ERR_ADD_INVEN_INVEN;
	}
	return NF::G_NF_ERR_SUCCESS;
}

// ĳ���Ͱ� ����(NCS,CHS,NGS) ���ӽÿ� �������� �ɷ�ġ
LONG CNFItem::GetCharAbility(NFCharInfoExt* pNFCharInfo)
{
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErr = NF::G_NF_ERR_SUCCESS;

	// 1. ĳ���� �⺻ �ɷ�ġ
	lErr = GetChar_BasicAbility(pNFCharInfo->m_nfCharExteriorInfo.m_lBasicCharSRL, pNFCharInfo->m_nfAbility);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;

	// 2. ĳ���� ������ �´� �ɷ�ġ
	lErr = GetChar_LevelAbility(pNFCharInfo->m_nfCharBaseInfo.m_lLevel, pNFCharInfo->m_nfAbility);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;

	// WORKING(acepm83@neowiz.com) ������ ������ ���� �ɷ�ġ �߰�(����)
	lErr = GetAquaBuff(pNFCharInfo->m_nfAqua.m_lAquaScore, pNFCharInfo->m_nfAqua.m_dFeedGauge, pNFCharInfo->m_nfAqua.m_dClearGauge, pNFCharInfo->m_nfAbility);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;

	// 3. ĳ���Ͱ� �������� ä��_������ �ɷ�ġ - ä��������� �г�Ƽ���� üũ!!!
	lErr = AddEquipedItemTotalAbility(pNFCharInfo, pNFCharInfo->m_nfAbility);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;

	return lErr;
}

// ������ ����ÿ� ȣ��
// Naked ������ �ѹ� �����ǰ� ����,���� ���� �� �� ����...
// Ŭ���̾�Ʈ���� �����ϰ��� �ϴ� InvenSRL�� -1�� ������, Naked �����̱� ������, �ɷ�ġ�� �����ϰ� �ش��ϴ� ������ InvenSRL�� -1�� �����Ѵ�.
LONG CNFItem::CheckValidChangingItems(NFCharInfoExt* pNFCharInfo, NFInvenSlot& invenOld, NFInvenSlot& invenNew)
{
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErr = NF::EC_CP_SUCCESS;

	// Ŭ���̾�Ʈ���� �԰� �ִ°� �� �������� �ϰų�, ���� �ִ°� �� ������� �Ҷ� ������ ����Ƿ�.... (��ŷ�̵�, �����̵�) ���ƾ� �Ѵ�...

	// Naked ������, Old�� New�� -1�� �ƴѰ�쿡�� �ش� PartIndex�� ���;� �Ѵ�.
	// ����� �� : 0, Naked�� -1
	if (invenOld.m_lInvenSRL != G_NAKED_PARTS && invenOld.m_lInvenSRL != 0)
	{
		lErr = GetNFInvenSlot(pNFCharInfo->m_nfCharInven, invenOld.m_lItemCode, invenOld.m_lInvenSRL, TRUE, invenOld);
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;
	}
	else {
		invenOld.m_lItemCode = 0; invenOld.m_lInvenSRL = 0;
	}

	if (invenNew.m_lInvenSRL != G_NAKED_PARTS && invenNew.m_lInvenSRL != 0) 
	{
		lErr = GetNFInvenSlot(pNFCharInfo->m_nfCharInven, invenNew.m_lItemCode, invenNew.m_lInvenSRL, TRUE, invenNew);
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;								// �������� ĳ���� �������� ����... ����
	}
	else {
		invenNew.m_lItemCode = 0; invenNew.m_lInvenSRL = 0;
	}

	return lErr;
}

LONG CNFItem::SetChangedItems(const LONG lGSN, const RoomID& roomID, NFCharInfoExt* pNFCharInfo, const NFInvenSlot& invenOld, const NFInvenSlot& invenNew)
{
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lPartsIndex = invenNew.m_lPartsIndex;
	if (lPartsIndex <= 0)
		lPartsIndex = invenOld.m_lPartsIndex;

	// �ɷ�ġ ��/��
	LONG lErr = CalcChangedItemAbility(pNFCharInfo, lPartsIndex, invenOld, invenNew, pNFCharInfo->m_nfAbility);

	if (invenOld.m_lInvenSRL != G_NAKED_PARTS && invenOld.m_lInvenSRL != 0)
	{
		lErr = ModifyExistItem(pNFCharInfo->m_nfCharInven, invenOld);
		if (NF::G_NF_ERR_SUCCESS != lErr)
			return lErr;
	}

	if (invenNew.m_lInvenSRL != G_NAKED_PARTS && invenNew.m_lInvenSRL != 0) 
	{
		lErr = ModifyExistItem(pNFCharInfo->m_nfCharInven, invenNew);
		if (NF::G_NF_ERR_SUCCESS != lErr)
			return lErr;
		
		// ���� �߰� or ������ ������ �׳� ������Ʈ : ���� PartsIndex�� ����ġ�°��̹Ƿ�...
		AddUsingInven(pNFCharInfo->m_nfCharInven.m_mapUsingItem, invenNew);
	}
	else
		RemoveUsingInven(pNFCharInfo->m_nfCharInven.m_mapUsingItem, lPartsIndex);

	/// achv
	{
		TMapAchvFactor mapFactorVal;
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_CHARM], (LONG)pNFCharInfo->m_nfAbility.m_dCharm));
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_STR], (LONG)pNFCharInfo->m_nfAbility.m_dStrength));
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_CONTROL], (LONG)pNFCharInfo->m_nfAbility.m_dControl));
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_FLY_DIST], (LONG)pNFCharInfo->m_nfAbility.m_dFlyDist));
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_AGILITY], (LONG)pNFCharInfo->m_nfAbility.m_dAgility));
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_LUCKY], (LONG)pNFCharInfo->m_nfAbility.m_dLuckyPoint));
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_HEALTH], (LONG)pNFCharInfo->m_nfAbility.m_dHealth));
		g_achv.CheckAchv(lGSN, pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN, achv::AE_ABILITY, roomID, mapFactorVal);
	}
	return lErr;
}

LONG CNFItem::CalcEquipedItem_FP(NFCharInfoExt* pNFCharInfo, int nItemType, BOOL bIsSalt, BOOL& bChanged, float fPenaltyRate)
{
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErrCode = NF::G_NF_ERR_SUCCESS;

	TMapInven::iterator iter = pNFCharInfo->m_nfCharInven.m_mapUsingItem.find(nItemType);
	if (iter == pNFCharInfo->m_nfCharInven.m_mapUsingItem.end())
	{
		if (eItemType_Boat == nItemType)
			return NF::G_NF_ERR_SUCCESS;
		return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_USING;
	}

	LONG lItemCode = (*iter).second.m_lItemCode;
	EquipItem* pItem = theNFDataItemMgr.GetEquipItemByIndex(lItemCode);
	if (NULL == pItem)
		return NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMCODE;

	BOOL bItemType = FALSE;
	if (0 == pItem->m_lWaterType)	// ���� : �г�Ƽ ������ ����
		return lErrCode;
	else if (1 == pItem->m_lWaterType)	// ���
		bItemType = FALSE;
	else
		bItemType = TRUE;	// �ؼ�

	if (bIsSalt != bItemType)
	{
		NFAbility pernaltyAblity = pItem->m_nfAbilityExt.m_nfAbility;
		pernaltyAblity *= fPenaltyRate;
		pNFCharInfo->m_nfAbility -= pernaltyAblity;
		bChanged = TRUE;
	}
	return lErrCode;
}

LONG CNFItem::CalcEquipedItem_FP(NFCharInfoExt* pNFCharInfo, BOOL bIsSalt, BOOL& bChanged)
{
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErrCode = NF::G_NF_ERR_SUCCESS;

	lErrCode = CalcEquipedItem_FP(pNFCharInfo, eItemType_Lure, bIsSalt, bChanged, PENALTY_RATE_LURE);
	if (NF::G_NF_ERR_SUCCESS != lErrCode)
		return lErrCode;

	lErrCode = CalcEquipedItem_FP(pNFCharInfo, eItemType_Boat, bIsSalt, bChanged, PENALTY_RATE_BOAT);
	if (NF::G_NF_ERR_SUCCESS != lErrCode)
		return lErrCode;

	return lErrCode;
}

BOOL CNFItem::Check_PeriodItemValid(const string& strInven_EndDate)
{
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	string	strCurrentDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	time_t time_current = 0;
	ConvertNFDateString2time_t(strCurrentDate, time_current);

	if (G_MAX_DATE == strInven_EndDate)
		return FALSE;

	time_t time_inven = 0;
	ConvertNFDateString2time_t(strInven_EndDate, time_inven);

	if ((time_inven - time_current) <= 0)
		return TRUE;
	return FALSE;
}

BOOL CNFItem::Check_RemainCountValid(LONG lRemainCount, LONG lItemCode, LONG lParts)
{
	if (lRemainCount == 0)
	{
		if (!theNFDataItemMgr.CheckItemTypeByItemCode("E", lItemCode, lParts))
			return TRUE;
	}
	return FALSE;
}

LONG CNFItem::Check_TotalItemValid(LONG lGSN, NFCharInven& inven, TlstInvenSlot& lstRemovedInven)
{
	LONG lErrCode = NF::G_NF_ERR_SUCCESS;

	ForEachElmt(TMapInven, inven.m_mapCharInven, it, ij)
	{
		switch((*it).second.m_lPartsIndex)
		{
		case eItemType_Jack:		// �Ⱓ�� - Naked(InvenSRL = -1)
		case eItemType_Pant:		// Naked(InvenSRL = -1)
		case eItemType_Glov:		// Naked(InvenSRL = -1)
		case eItemType_Foot:		// Naked(InvenSRL = -1)
		case eItemType_Acce:		// �Ⱓ�� - ����°�(InvenSRL = 0)
		case eItemType_FishDetector:// ����°�(InvenSRL = 0)
		case eItemType_Boat:		// ����°�(InvenSRL = 0)
			{
				if (Check_PeriodItemValid((*it).second.m_strExpireDate))
					lstRemovedInven.push_back((*it).second);
				break;
			}
		case eItemType_Rod:			// ������(RemainCount�� 0���ϰ� �ǵ� �������� �ٲ��� �ʴ´�...)
		case eItemType_Reel:		// ������(RemainCount�� 0���ϰ� �ǵ� �������� �ٲ��� �ʴ´�...)
		case eItemType_Bags:		// ������
		default: break;
		}
	}

	ForEachElmt(TMapInvenSlotList, inven.m_mapCountableItem, it2, ij2)
	{
		ForEachElmt(TlstInvenSlot, (*it2).second, it3, ij3)
		{
			switch((*it3).m_lPartsIndex)
			{
			case eItemType_SkillItem:	// �Ⱓ��, QuickSlot���� ������.
				{
					if (Check_PeriodItemValid((*it3).m_strExpireDate))
						lstRemovedInven.push_back((*it3));
					break;
				}
			case eItemType_UsableItem:	// QuickSlot���� ������.
			case eItemType_Lure:		// Ƚ���� - Naked(InvenSRL = -1)
			case eItemType_Line:		// Ƚ���� - Naked(InvenSRL = -1)
				{
					if (Check_RemainCountValid((*it3).m_lRemainCount, (*it3).m_lItemCode, (*it3).m_lPartsIndex))
						lstRemovedInven.push_back((*it3));
					break;
				}
			default: break;
			}
		}
	}
	return lErrCode;
}

BOOL CNFItem::IsCountableItem(const LONG lParts)
{
	if (eItemType_UsableItem == lParts || eItemType_SkillItem == lParts || eItemType_Line == lParts || eItemType_Lure == lParts || eItemType_FishingCard == lParts)
		return TRUE;

	return FALSE;
}

BOOL CNFItem::RemoveItem(NFCharInven& inven, const NFInvenSlot& remove_inven)
{
	LONG lParts = remove_inven.m_lItemCode/G_VALUE_CONVERT_PARTS;
	BOOL bIsRemove = FALSE;
	if (IsCountableItem(lParts))
	{
		TMapInvenSlotList::iterator iter = inven.m_mapCountableItem.find(remove_inven.m_lItemCode);			// ItemCode
		if (iter != inven.m_mapCountableItem.end()) 
		{
			ForEachElmt(TlstInvenSlot, (*iter).second, it2, ij2)
			{
				if ((*it2).m_lInvenSRL == remove_inven.m_lInvenSRL)	{
					(*iter).second.erase(it2);
					bIsRemove = TRUE;
				}
			}
		}
		else
            return FALSE;
	}
	else
	{
		TMapInven::iterator iter = inven.m_mapCharInven.find(remove_inven.m_lInvenSRL);						// InvenSRL
		if (iter != inven.m_mapCharInven.end())
			inven.m_mapCharInven.erase(iter);
		else
            return FALSE;
	}

	if (TRUE == remove_inven.m_bIsUsing)
		RemoveUsingInven(inven.m_mapUsingItem, remove_inven.m_lPartsIndex);
	return TRUE;
}

// ���� �ؾ� �� �������� ���� ���� ������ �߿� �Ⱓ�� ���� �Ǿ��ų�, RemainCount�� 0���� �� ��쿡 Default ���������� �ٲ۴�.
// RemainCount �� 0���� üũ�� Default �������� 0�����̹Ƿ� ����...
LONG CNFItem::AutoChange_DefaultItem(LONG lGSN, const RoomID& roomID, NFCharInfoExt* pNFCharInfo, const TlstInvenSlot& lstRemovedInven, TlstInvenSlot& lstDefaultChangedInven)
{
	LONG lErrCode = NF::G_NF_ERR_SUCCESS;

	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	NFBasicChar* pBasicChar = NULL;

	ForEachCElmt(TlstInvenSlot, lstRemovedInven, it, ij)
	{
		if (TRUE == (*it).m_bIsUsing)
		{
			ReqChangeParts req_partsMsg;
			req_partsMsg.Clear();
			req_partsMsg.m_lCSN = pNFCharInfo->m_nfCharBaseInfo.m_lNFCSN;

			if (NULL == pBasicChar)
				pBasicChar = theNFDataItemMgr.GetNFBasicChar(pNFCharInfo->m_nfCharExteriorInfo.m_lBasicCharSRL);

			req_partsMsg.m_lNewItemCode = 0;

			//////////////////////////////////////////////////////////////////////////
			switch((*it).m_lPartsIndex)
			{
			case eItemType_Jack:		// �Ⱓ�� - Naked(InvenSRL = -1)
			case eItemType_Pant:		// Naked(InvenSRL = -1)
			case eItemType_Glov:		// Naked(InvenSRL = -1)
			case eItemType_Foot:		// Naked(InvenSRL = -1)
				{
					req_partsMsg.m_lNewInvenSRL = -1;
					break;
				}
			case eItemType_Lure:		// Ƚ���� - Inven���� ã�Ƽ�
				{
					if (pBasicChar->m_lLureItemCode == (*it).m_lItemCode)	// �������ε� ������ �Ѵ�!!!
						continue;

					req_partsMsg.m_lNewItemCode = pBasicChar->m_lLureItemCode;
					break;
				}
			case eItemType_Line:		// Ƚ���� - Inven���� ã�Ƽ�
				{
					if (pBasicChar->m_lLineItemCode == (*it).m_lItemCode)	// �������ε� ������ �Ѵ�!!!
						continue;

					req_partsMsg.m_lNewItemCode = pBasicChar->m_lLineItemCode;
					break;
				}
			case eItemType_Acce:		// �Ⱓ�� - ����°�(InvenSRL = 0)
			case eItemType_FishDetector:// ����°�(InvenSRL = 0)
			case eItemType_Boat:		// ����°�(InvenSRL = 0)
				{
					req_partsMsg.m_lNewInvenSRL = 0;
					break;
				}
			default: break;			// ��ȿ���� �ʾƵ� �ڵ����� ��ü���� �ʴ� ������(������-Rod, Reel, ������-Bags, FishingCard)���� ����!!!
			}

			//////////////////////////////////////////////////////////////////////////
			switch((*it).m_lPartsIndex)
			{
			case eItemType_UsableItem:
			case eItemType_SkillItem:
				{
					// Usable, Skill�� QuickSlot���� ����...
					ReqChangeQuickSlot reqMsg;
					reqMsg.m_lChangeType = CS_REMOVE;
					reqMsg.m_lRemoveSlotIndex = 0;
					reqMsg.m_lRemoveItemCode = (*it).m_lItemCode;

					ForEachElmt(ArcVectorT< LONG >, pNFCharInfo->m_nfQuickSlot, itr, ijr)
					{
						if ((*itr) == reqMsg.m_lRemoveItemCode)
							break;
						else
							++reqMsg.m_lRemoveSlotIndex;
					}
					AnsChangeQuickSlot ansMsg;
					theNFMenu.ChangeQuickSlot(pNFCharInfo, lGSN, reqMsg, ansMsg, TRUE);
					if (NF::G_NF_ERR_SUCCESS != ansMsg.m_lErrorCode)
						return ansMsg.m_lErrorCode;
					break;		// Usable, Skill�� ����� ������, QuickSlot������ ���ش�.(ChangeParts�� ���� �� ��)
				}
			case eItemType_Lure:
			case eItemType_Line:
				{
					// �⺻ �������� ITemCode-ItemID�� �ϳ���� �����Ͽ�, CountableInven���� ITemCode�� �˻��Ͽ�, �� ó���� InvenSRL�� ���´�.
					string strTemp = "";
					TlstInvenSlot lst;
					theNFItem.GetExistToInvenList(pNFCharInfo->m_nfCharInven, req_partsMsg.m_lNewItemCode, FALSE, strTemp, strTemp, lst, lErrCode);
					if (NF::G_NF_ERR_SUCCESS != lErrCode)
						return lErrCode;

					ForEachElmt(TlstInvenSlot, lst, itChanged, ijChanged) {
						req_partsMsg.m_lNewInvenSRL = (*itChanged).m_lInvenSRL;
						break;
					}
					// break���� - Lure, Line�� �⺻ ���� �ٲ���� �ϹǷ�...�Ʒ� ChangeParts�� �����ؾ� �Ѵ�.
				}
			default:
				{
					// �̸� �κ����� ���� �ϰ�, part�� �����Ѵ�.(�� �׷�, using_inven���� ���̴� ���� ����)
					theNFItem.RemoveItem(pNFCharInfo->m_nfCharInven, (*it));

					BOOL bDebuffRet = FALSE;
					AnsChangeParts ansMsg;
					theNFMenu.ChangeParts(pNFCharInfo, lGSN, roomID, req_partsMsg, ansMsg, -1, bDebuffRet);
					if (NF::G_NF_ERR_SUCCESS != ansMsg.m_lErrorCode)
						return ansMsg.m_lErrorCode;
					lstDefaultChangedInven.push_back(ansMsg.m_newInvenSlot);
					break;
				}
			}
		}
	}

	return lErrCode;
}