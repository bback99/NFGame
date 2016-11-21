
#ifndef _NF_ITEM_H_
#define _NF_ITEM_H_

class CNFItem
{
public:
	CNFItem();
	virtual ~CNFItem();

protected:
	LONG CheckValid_Level(ItemCommon* pItemCommon, LONG lCharLevel);
	LONG CheckInvenIsFull(NFCharInven& mapInven);
	size_t GetInvenSize(NFCharInven& mapInven);
	LONG GetUsingItemCode(NFCharInfo* pInfo, int nUsingItemType, LONG& lItemCode);
	LONG GetUsingItemEnchantLevel(NFCharInfo* pInfo, int nUsingItemType);

	ItemCommon* GetItemCommon(NFInvenSlot& inven);

	LONG GetChar_BasicAbility(LONG lBasicCharIndex, NFAbility& nfAbility);
	LONG GetChar_LevelAbility(LONG lLevel, NFAbility& nfAbility);

	LONG GetChar_EquipedEquipItemAbility(NFCharInfo* pInfo, LONG nEquipedEquipItemType, LONG lItemCode, LONG lEnchantLevel, NFAbility& nfAbility, BOOL bIsAdd=TRUE);
	LONG GetChar_EquipedClothItemAbility(NFCharInfo* pInfo, LONG lItemCode, NFAbility& nfAbility, BOOL bIsAdd=TRUE);
	LONG CalcChangedItemAbility(NFCharInfo* pInfo, LONG lPartsIndex, const NFInvenSlot& invenOld, const NFInvenSlot& invenNew, NFAbility& nfAbility);
	LONG EquipedItemTotalAbility(TMapInven& inven, NFAbility& nfAbility);

	LONG GetAquaBuff(LONG lAquaScore, double dFeedGauge, double dClearGauge, NFAbility& nfAbility);

//
public:
	// access_inven
	LONG ProcessCapacityInven(NFCharInven& mapInven, LONG lTempInvenSRL, NFInvenSlot& oldInven, NFInvenSlot& newInven, LONG lItemCNT, LONG lCapacity, TMapInven& mapOld, TMapInven& mapNew);
	void AddUsingInven(TMapInven& usingInven, const NFInvenSlot& add_inven);
	void RemoveUsingInven(TMapInven& usingInven, LONG lPartsIndex);
	LONG AddInvenSlotItem(ArcVectorT< LONG >& slot, NFCharInven& mapInven, NFInvenSlot& inven);
	LONG AddInvenSlotItem(NFCharInven& mapInven, NFInvenSlot& inven);
	LONG GetNFInvenSlot(NFCharInven& nfCharInven, LONG lItemCode, LONG lInvenSRL, BOOL bIsUsingToggle, NFInvenSlot& inven);
	BOOL GetExistToInvenList(NFCharInven& mapInven, LONG lItemCode, BOOL bIsUsingToggle, string& strStartDate, string& strEndDate, TlstInvenSlot& lst, LONG& lErrorCode);
	LONG ModifyExistItem(NFCharInven& mapInven, const NFInvenSlot& inven);

	// 
	LONG AddEquipedItemTotalAbility(NFCharInfo* pInfo, NFAbility& nfAbility);

	//
	LONG GetCharAbility(NFCharInfoExt* pNFCharInfo);
	LONG CheckValidChangingItems(NFCharInfoExt* pNFCharInfo, NFInvenSlot& invenOld, NFInvenSlot& invenNew);
	LONG SetChangedItems(const LONG lGSN, const RoomID& roomID, NFCharInfoExt* pNFCharInfo, const NFInvenSlot& invenOld, const NFInvenSlot& invenNew);

	LONG CalcEquipedItem_FP(NFCharInfoExt* pNFCharInfo, int nItemType, BOOL bIsSalt, BOOL& bChanged, float fPenaltyRate);
	LONG CalcEquipedItem_FP(NFCharInfoExt* pNFCharInfo, BOOL bIsSalt, BOOL& bChanged);

	BOOL Check_PeriodItemValid(const string& strInven_EndDate);
	BOOL Check_RemainCountValid(LONG lRemainCount, LONG lItemCode, LONG lParts);
	LONG Check_TotalItemValid(LONG lGSN, NFCharInven& inven, TlstInvenSlot& lstRemovedInven);
	LONG AutoChange_DefaultItem(LONG lGSN, const RoomID& roomID, NFCharInfoExt* nfCharInfoExt, const TlstInvenSlot& lstRemovedInven, TlstInvenSlot& lstDefaultChangedInven);

	BOOL RemoveItem(NFCharInven& inven, const NFInvenSlot& remove_inven);
	BOOL IsCountableItem(const LONG lParts);
};

extern	CNFItem		theNFItem;

#endif //_NF_ITEM_H