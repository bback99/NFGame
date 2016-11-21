
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
		switch(nUsingItemType)		// 필수적으로 이놈들은 ItemCode가 유효해야 한다...
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

// 아이템 지급시 capacity 와 비교하여 새로운 인벤 or 기존 인벤에 겹치기 or 새로운 인벤 & 기존 인벤에 넣기를 판단하는 함수
// 1. BuyItem 호출시 사용
// 2. FishDrop 시 사용
// 3. RewardItem 지급시 사용
LONG CNFItem::ProcessCapacityInven(NFCharInven& mapInven, LONG lTempInvenSRL, NFInvenSlot& oldInven, NFInvenSlot& newInven, LONG lItemCNT, LONG lCapacity, TMapInven& mapOld, TMapInven& mapNew)
{
	LONG lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (lTempInvenSRL == oldInven.m_lInvenSRL)		// 서버 메모리와 DB에서 계산한 값이 같으므로, 업데이트 
	{
		oldInven.m_lRemainCount += lItemCNT;
		lErrorCode = theNFItem.ModifyExistItem(mapInven, oldInven);
		mapOld.insert(make_pair(oldInven.m_lInvenSRL, oldInven));
	}
	else		// 기존꺼 MAX로 만들고
	{	
		newInven = oldInven;			

		if (lTempInvenSRL == 0)		// 무조건 새로 만들어서 추가
		{	
			newInven.m_lInvenSRL = oldInven.m_lInvenSRL;
			newInven.m_lRemainCount = lItemCNT;
			lErrorCode = theNFItem.AddInvenSlotItem(mapInven, newInven);
			mapNew.insert(make_pair(newInven.m_lInvenSRL, newInven));
		}
		else	// 서버와 DB가 같지 않고, 서버에서 선택한 놈의 값이 있다면.... 추가 & 기존꺼 수정
		{
			// 기존꺼는 capacity만큼
			LONG lRemainCount = lCapacity - oldInven.m_lRemainCount;
			oldInven.m_lInvenSRL = lTempInvenSRL;
			oldInven.m_lRemainCount = lCapacity;
			lErrorCode = theNFItem.ModifyExistItem(mapInven, oldInven);
			if (NF::G_NF_ERR_SUCCESS != lErrorCode)
				return lErrorCode;
			mapOld.insert(make_pair(oldInven.m_lInvenSRL, oldInven));

			// 새로운건 capacity에서 기존꺼를 빼고 남은 나머지를 product 카운트에서 뺀값
			newInven.m_lRemainCount = lItemCNT - lRemainCount;
			lErrorCode = theNFItem.AddInvenSlotItem(mapInven, newInven);
			if (NF::G_NF_ERR_SUCCESS != lErrorCode)
				return lErrorCode;
			mapNew.insert(make_pair(newInven.m_lInvenSRL, newInven));
		}
	}
	return lErrorCode;
}

// bIsUsingToggle가 TRUE이면, Using을 Toggle로 반대로 셋팅한다.
LONG CNFItem::GetNFInvenSlot(NFCharInven& nfCharInven, LONG lItemCode, LONG lInvenSRL, BOOL bIsUsingToggle, NFInvenSlot& inven)
{
	LONG lErrCode = NF::G_NF_ERR_SUCCESS;

	LONG lParts = lItemCode/G_VALUE_CONVERT_PARTS;
	if (IsCountableItem(lParts))
	{
		// Inven에서 가지고 있는 아이템인지 체크
		TMapInvenSlotList::iterator	iter = nfCharInven.m_mapCountableItem.find(lItemCode);		
		if (iter != nfCharInven.m_mapCountableItem.end())
		{
			// Using으로 다 변경하려면, list에 있는 모든 것들을 다 바꿔야 한다.
			ForEachElmt(TlstInvenSlot, (*iter).second, itSlot, ijSlot) 
			{
				if ((*itSlot).m_lInvenSRL == lInvenSRL) 
				{
					if (bIsUsingToggle)
						(*itSlot).m_bIsUsing = !((*itSlot).m_bIsUsing);

					inven = (*itSlot); // 수정 후 가져와야 한다.
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

				if ((*it).m_lPartsIndex == eItemType_SkillItem)		// 스킬 아이템이면 날짜 셋팅 해야 한다...
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

		nMaxCount = pItem->m_lCount;		// 백팩을 장착 하고 있으니 최대 갯수(Count)에서 가지고 온다...2011/11/23 - 기획팀과 협의 함
	}

	if ((int)GetInvenSize(mapInven) >= nMaxCount)
		return NF::G_NF_ERR_ADD_INVEN_SIZE_FULL;		// MAX 

	return NF::G_NF_ERR_SUCCESS;	// possible
}

ItemCommon* CNFItem::GetItemCommon(NFInvenSlot& inven)
{
	ItemCommon* pItemCommon = NULL;

	// 새로 장착 하는 아이템에서 레벨 체크
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

// WORKING(acepm83@neowiz.com) 수족관 점수에 따른 능력치 추가(버프)
// 하위등급의 버프를 중복해서 가져야 한다.
LONG CNFItem::GetAquaBuff(LONG lAquaScore, double dFeedGauge, double dClearGauge, NFAbility& nfAbility)
{
	// 청결도, 포만도가 모두 0%이면 버프 없음
	if (dClearGauge <= 0.0 && dFeedGauge <= 0.0)
	{
		return NF::G_NF_ERR_SUCCESS;
	}

	NFAbility nfBuff;
	nfBuff.Clear();

	if (NAB_GUARD <= lAquaScore)
	{
		nfBuff.m_dControl = 5.0; // 제어5
	}

	if (NAB_LIFE <= lAquaScore)
	{
		nfBuff.m_dHealth = 10.0; // 힘10
	}

	if (NAB_LIGHT <= lAquaScore)
	{
		nfBuff.m_dAgility = 15.0; // 민첩15
	}

	if (NAB_STR <= lAquaScore)
	{
		nfBuff.m_dStrength = 25.0; // 힘25
	}

	if (NAB_PROTECTION <= lAquaScore)
	{
		nfBuff.m_dLuckyPoint = 30.0; // 운30
	}

	// 청결도, 포만도중 하나가 0%이면 버프의 50%만 받음
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
	// "6-1-3. 채비 아이템 타입에 따른 능력치 적용"라는 기획 내용에 의해서 아이템 타입이 다를 경우 패널티를 부여해야 한다.
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
		lErr = GetChar_EquipedClothItemAbility(pInfo, invenOld.m_lItemCode, nfAbility, FALSE);					// 능력치 차감
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;

		lErr = GetChar_EquipedClothItemAbility(pInfo, invenNew.m_lItemCode, nfAbility);							// 능력치 증가
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;
	}

	if (lPartsIndex >= eItemType_Lure && lPartsIndex <= eItemType_FishDetector)
	{
		lErr = GetChar_EquipedEquipItemAbility(pInfo, lPartsIndex, invenOld.m_lItemCode, invenOld.m_lEnchantLevel, nfAbility, FALSE);		// 능력치 차감
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;

		lErr = GetChar_EquipedEquipItemAbility(pInfo, lPartsIndex, invenNew.m_lItemCode, invenNew.m_lEnchantLevel, nfAbility);				// 능력치 증가
		if (lErr != NF::G_NF_ERR_SUCCESS)
			return lErr;
	}

	return lErr;
}

LONG CNFItem::GetChar_EquipedEquipItemAbility(NFCharInfo* pInfo, LONG nEquipedEquipItemType, LONG lItemCode, LONG lEnchantLevel, NFAbility& nfAbility, BOOL bIsAdd)
{
	if (lItemCode <= 0)
	{
		switch(nEquipedEquipItemType)		// 필수적으로 이놈들은 ItemCode가 유효해야 한다...
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
			nCheckOtherItemType = eItemType_Line;		// Line과 ItemType 체크
			break;
		}
	case eItemType_Rod:
		{
			nCheckOtherItemType = eItemType_Reel;		// Reel과 ItemType 체크
			break;
		}
	default: // 루어, 보트, 어종탐지기는 해당 아이템의 능력치만 더한다.
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

	// 아이템강화 단계에 따라 더해진 능력치를 구한다.
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
	penaltyAbilityRate += nfEnchantAddAbility; // 아이템강화 단계에 따라 더해진 능력치
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

	return NF::G_NF_ERR_SUCCESS;	// 의상아이템은 Naked로 착용할 경우, MyInven에 없다고 나오므로 에러가 아님...
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

// 유저 inven에 추가 하는 함수
// 인벤 최대 갯수는 유저의 힙색(eItemType_Bags)이 가질 수 있는 최대 갯수를 체크 한다.
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

				if (eItemType_UsableItem == (*it2).m_lPartsIndex)		// 임시코드(usable만 할것인지...???)
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
	return elem1.m_lRemainCount < elem2.m_lRemainCount;		//오름차순 정렬 
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

				if (eItemType_UsableItem == (*it2).m_lPartsIndex)		// 임시코드(usable만 할것인지...???)
					inven.m_bIsUsing = (*it2).m_bIsUsing;
			}

			(*iter).second.push_back(inven);

			// sorting remain_count로...
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

// 캐릭터가 서버(NCS,CHS,NGS) 접속시에 가져오는 능력치
LONG CNFItem::GetCharAbility(NFCharInfoExt* pNFCharInfo)
{
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErr = NF::G_NF_ERR_SUCCESS;

	// 1. 캐릭터 기본 능력치
	lErr = GetChar_BasicAbility(pNFCharInfo->m_nfCharExteriorInfo.m_lBasicCharSRL, pNFCharInfo->m_nfAbility);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;

	// 2. 캐릭터 레벨에 맞는 능력치
	lErr = GetChar_LevelAbility(pNFCharInfo->m_nfCharBaseInfo.m_lLevel, pNFCharInfo->m_nfAbility);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;

	// WORKING(acepm83@neowiz.com) 수족관 점수에 따른 능력치 추가(버프)
	lErr = GetAquaBuff(pNFCharInfo->m_nfAqua.m_lAquaScore, pNFCharInfo->m_nfAqua.m_dFeedGauge, pNFCharInfo->m_nfAqua.m_dClearGauge, pNFCharInfo->m_nfAbility);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;

	// 3. 캐릭터가 착용중인 채비_아이템 능력치 - 채비아이템의 패널티까지 체크!!!
	lErr = AddEquipedItemTotalAbility(pNFCharInfo, pNFCharInfo->m_nfAbility);
	if (lErr != NF::G_NF_ERR_SUCCESS)
		return lErr;

	return lErr;
}

// 아이템 변경시에 호출
// Naked 복장은 한번 설정되고 나면,절대 변경 될 일 없음...
// 클라이언트에서 변경하고자 하는 InvenSRL을 -1로 보내면, Naked 복장이기 때문에, 능력치만 제거하고 해당하는 파츠의 InvenSRL은 -1로 설정한다.
LONG CNFItem::CheckValidChangingItems(NFCharInfoExt* pNFCharInfo, NFInvenSlot& invenOld, NFInvenSlot& invenNew)
{
	if (NULL == pNFCharInfo)
		return NF::G_NF_ERR_NOT_FOUND_NF_CHAR_INFO_EXT;

	LONG lErr = NF::EC_CP_SUCCESS;

	// 클라이언트에서 입고 있는걸 또 입으려고 하거나, 벗고 있는걸 또 벗기려고 할때 문제가 생기므로.... (해킹이든, 버그이든) 막아야 한다...

	// Naked 때문에, Old든 New든 -1이 아닌경우에는 해당 PartIndex를 얻어와야 한다.
	// 벗기는 건 : 0, Naked는 -1
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
			return lErr;								// 아이템이 캐릭터 레벨보다 높다... 실패
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

	// 능력치 증/감
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
		
		// 새로 추가 or 기존에 있으면 그냥 업데이트 : 같은 PartsIndex에 업어치는것이므로...
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
	if (0 == pItem->m_lWaterType)	// 공통 : 패널티 무조건 없음
		return lErrCode;
	else if (1 == pItem->m_lWaterType)	// 담수
		bItemType = FALSE;
	else
		bItemType = TRUE;	// 해수

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
		case eItemType_Jack:		// 기간제 - Naked(InvenSRL = -1)
		case eItemType_Pant:		// Naked(InvenSRL = -1)
		case eItemType_Glov:		// Naked(InvenSRL = -1)
		case eItemType_Foot:		// Naked(InvenSRL = -1)
		case eItemType_Acce:		// 기간제 - 벗기는거(InvenSRL = 0)
		case eItemType_FishDetector:// 벗기는거(InvenSRL = 0)
		case eItemType_Boat:		// 벗기는거(InvenSRL = 0)
			{
				if (Check_PeriodItemValid((*it).second.m_strExpireDate))
					lstRemovedInven.push_back((*it).second);
				break;
			}
		case eItemType_Rod:			// 내구제(RemainCount가 0이하가 되도 아이템을 바꾸지 않는다...)
		case eItemType_Reel:		// 내구제(RemainCount가 0이하가 되도 아이템을 바꾸지 않는다...)
		case eItemType_Bags:		// 영구제
		default: break;
		}
	}

	ForEachElmt(TMapInvenSlotList, inven.m_mapCountableItem, it2, ij2)
	{
		ForEachElmt(TlstInvenSlot, (*it2).second, it3, ij3)
		{
			switch((*it3).m_lPartsIndex)
			{
			case eItemType_SkillItem:	// 기간제, QuickSlot에서 빠진다.
				{
					if (Check_PeriodItemValid((*it3).m_strExpireDate))
						lstRemovedInven.push_back((*it3));
					break;
				}
			case eItemType_UsableItem:	// QuickSlot에서 빠진다.
			case eItemType_Lure:		// 횟수제 - Naked(InvenSRL = -1)
			case eItemType_Line:		// 횟수제 - Naked(InvenSRL = -1)
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

// 제거 해야 할 아이템이 착용 중인 아이템 중에 기간이 만료 되었거나, RemainCount가 0이하 인 경우에 Default 아이템으로 바꾼다.
// RemainCount 가 0이하 체크는 Default 아이템이 0이하이므로 보류...
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
			case eItemType_Jack:		// 기간제 - Naked(InvenSRL = -1)
			case eItemType_Pant:		// Naked(InvenSRL = -1)
			case eItemType_Glov:		// Naked(InvenSRL = -1)
			case eItemType_Foot:		// Naked(InvenSRL = -1)
				{
					req_partsMsg.m_lNewInvenSRL = -1;
					break;
				}
			case eItemType_Lure:		// 횟수제 - Inven에서 찾아서
				{
					if (pBasicChar->m_lLureItemCode == (*it).m_lItemCode)	// 베이직인데 뺄려고 한다!!!
						continue;

					req_partsMsg.m_lNewItemCode = pBasicChar->m_lLureItemCode;
					break;
				}
			case eItemType_Line:		// 횟수제 - Inven에서 찾아서
				{
					if (pBasicChar->m_lLineItemCode == (*it).m_lItemCode)	// 베이직인데 뺄려고 한다!!!
						continue;

					req_partsMsg.m_lNewItemCode = pBasicChar->m_lLineItemCode;
					break;
				}
			case eItemType_Acce:		// 기간제 - 벗기는거(InvenSRL = 0)
			case eItemType_FishDetector:// 벗기는거(InvenSRL = 0)
			case eItemType_Boat:		// 벗기는거(InvenSRL = 0)
				{
					req_partsMsg.m_lNewInvenSRL = 0;
					break;
				}
			default: break;			// 유효하지 않아도 자동으로 교체하지 않는 아이템(내구도-Rod, Reel, 영구제-Bags, FishingCard)들은 무시!!!
			}

			//////////////////////////////////////////////////////////////////////////
			switch((*it).m_lPartsIndex)
			{
			case eItemType_UsableItem:
			case eItemType_SkillItem:
				{
					// Usable, Skill은 QuickSlot에서 뺀다...
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
					break;		// Usable, Skill은 사용이 끝나면, QuickSlot에서만 빼준다.(ChangeParts는 실행 안 함)
				}
			case eItemType_Lure:
			case eItemType_Line:
				{
					// 기본 아이템은 ITemCode-ItemID가 하나라는 가정하에, CountableInven에서 ITemCode로 검색하여, 맨 처음의 InvenSRL을 얻어온다.
					string strTemp = "";
					TlstInvenSlot lst;
					theNFItem.GetExistToInvenList(pNFCharInfo->m_nfCharInven, req_partsMsg.m_lNewItemCode, FALSE, strTemp, strTemp, lst, lErrCode);
					if (NF::G_NF_ERR_SUCCESS != lErrCode)
						return lErrCode;

					ForEachElmt(TlstInvenSlot, lst, itChanged, ijChanged) {
						req_partsMsg.m_lNewInvenSRL = (*itChanged).m_lInvenSRL;
						break;
					}
					// break안함 - Lure, Line은 기본 장비로 바꿔줘야 하므로...아래 ChangeParts를 실행해야 한다.
				}
			default:
				{
					// 미리 인벤에서 제거 하고, part를 변경한다.(안 그럼, using_inven에서 꼬이는 수가 생김)
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