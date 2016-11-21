
#ifndef NF_MENU_H
#define NF_MENU_H


typedef map<LONG, LONG>	TMapExchangeCard;

class CNFMenu
{
public:
	CNFMenu()	{}
	virtual ~CNFMenu()	{}


public:	// handler
	BOOL AnsAchv(NFCharInfoExt* nfCharInfoExt, const ArcListT<LONG>& lstAchvID, AnsAchvInfo& ansMsg);
	BOOL CheckBuyMoney(const ReqBuyItem& reqMsg, std::map<LONG/*ItemID*/, Product>& mapProduct, LONGLONG& lTotPrice, LONG& lErrorCode);
	BOOL BuyItem(NFCharInfoExt* nfCharInfoExt, LONG lGSN, const ReqBuyItem& reqMsg, const std::map<LONG/*ItemID*/, Product>& mapProduct, AnsBuyItem& ansMsg);
	BOOL RemoveItem(NFCharInfoExt* nfCharInfoExt, LONG lGSN, const ReqRemoveItem& reqMsg, AnsRemoveItem& ansMsg);
	BOOL OpenCardPack(NFCharInfoExt* nfCharInfoExt, LONG lGSN, LONG lReqInvenSRL, AnsOpenCardPack& ansMsg);
	BOOL ChangeParts(NFCharInfoExt* nfCharInfoExt, LONG lGSN, const RoomID& roomID, const ReqChangeParts& reqMsg, AnsChangeParts& ansMsg, LONG lEnvAttribute, BOOL& bRet);
	BOOL ChangeCardSlot(NFCharInfoExt* nfCharInfoExt, NFAbilityExt& nfAbilityExt, LONG lGSN, const ReqChangeCardSlot& reqMsg, AnsChangeCardSlot& ansMsg);
	BOOL ExchangeCard(NFCharInfoExt* nfCharInfoExt, LONG lGSN, const ExchangeCards& reqMsg, AnsExchangeCards& ansMsg);
	BOOL ChangeQuickSlot(NFCharInfoExt* nfCharInfoExt, LONG lGSN, const ReqChangeQuickSlot& reqMsg, AnsChangeQuickSlot& ansMsg, BOOL bOnlyRemove=FALSE);
	BOOL GetProductList(AnsProductList& ans);

// 외부에서 한번 접근한다...
	BOOL CheckEnvDebuff(NFCharInfoExt* pNFCharInfo, LONG lEnvAttribute);
	BOOL ModifyExistCardItem(NFCharInven& mapInven, LONG lInvenSRL, LONG lChangeType, LONG lPartsIndex, NFInvenSlot& inven, LONG& lErrorCode);
	LONG SetExistCountableItemToInven(NFCharInven& mapInven, NFInvenSlot& inven);
	LONG CalcLevelByAddExp(const LONG lGSN, NFCharInfoExt* pNFCharInfo, LONG lGetExp, const RoomID& roomID);
	
//
	LONG GetInvenSRLForBuyItem(NFCharInven& charInven, LONG lItemCode);
	LONG GetInvenSRLFromExistInven(NFCharInven& charInven, NFInvenSlot& inven, const LONG lItemCode, LONG& lCapacity);
	BOOL GetRewardItem(LONG lGSN, NFCharInfoExt* pNFCharInfo, const RoomID& roomID, const ReqRewardItem& reqMsg, AnsRewardItem& ansMsg);
	BOOL GetLockedNoteMain(LONG lGSN, LONG lCSN, AnsLockedNoteMain& ans);
	BOOL GetLockedNoteMap(LONG lGSN, LONG lCSN, LONG lMapID, TMapLockedNoteMap& mapLockedNoteMap, AnsLockedNoteMap& ans);
	LONG FindReduceInvenSlot(TMapInvenSlotList& mapCountableItem, NFInvenSlot& inven, LONG lItemCode);
	BOOL RepairEnduranceItem(LONG lGSN, LONG lCSN, NFCharInfoExt* pNFCharInfo, const ReqRepairEnduranceItem& req, AnsRepairEnduranceItem& ans);
	BOOL NextEnchantInfo(LONG lGSN, LONG lCSN, NFCharInfoExt* pNFCharInfo, const ReqNextEnchantInfo& req, AnsNextEnchantInfo& ans);
	BOOL ItemEnchant(LONG lGSN, LONG lCSN, NFCharInfoExt* pNFCharInfo, const ReqItemEnchant& req, AnsItemEnchant& ans);

	// WORKING(acepm83@neowiz.com)
	BOOL CalcNFAquaGauge(NFCharInfoExt* pNFCharInfo, LONG lElapsedClearHour, LONG lElapsedFeedHour);
	BOOL InsertAquaFish(LONG lGSN, LONG lCSN, NFCharInfoExt* pNFCharInfo, LONG lFishID, LONG lLength, LONG lWeight, LONG lElapsedTime, LONG lScore, LONG& lErrorCode);
	BOOL GetAquaFish(LONG lCSN, TMapNFAquaFish& mapNFAquaFish);

protected:
	// FishingCards
	LONG CheckFishingCards(TMapInven& mapInven, TMapExchangeCard& mapExchangeCard, LONG lUpgradeType, BOOL bIsSpecialCard, LONG& lFishingCardLevel);
	LONG ExchangeFishingCards(TMapInven& mapInven, NFInvenSlot& invenUpgradeCard, LONG lFishingCardLevel, NFInvenSlot& getCardPack, LONG lUpgradeType, BOOL bIsSpecialCard);

	// QuickSlot
	long RegistQuickSlot(std::vector<LONG>& vecQuickSlot, NFCharInven& mapInven, LONG lRegSlotIndex, LONG lRegItemCode);
	long RemoveQuickSlot(std::vector<LONG>& vecQuickSlot, NFCharInven& mapInven, LONG lRemSlotIndex, LONG lRemItemCode, BOOL bOnlyRemove);
	long SwitchingQuickSlot(std::vector<LONG>& vecQuickSlot, NFCharInven& mapInven, LONG lRegSlotIndex, LONG lReglItemCode, LONG lRemSlotIndex, LONG lRemItemCode);

	//
	LONGLONG CheckRepairMoney(LONG lItemCode, LONG lGauge, LONG lReinforceLV, LONG& lEndurance, LONG& lErr);
};


extern CNFMenu		theNFMenu;



#endif //NF_MENU_H