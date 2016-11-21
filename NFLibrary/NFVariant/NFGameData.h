//
// NFGameData_renewal.h
//

#ifndef NFGAMEDATA_H
#define NFGAMEDATA_H


#include <NF/ADL/MsgNFCommonStruct.h>


typedef std::map<LONG, LONG> TMapFishSkillReuseCount;


///////////////////////////////////////////////////////////////////////////////////
// CFish
class CFish
{
private:
	FishInfo		m_FishInfo;
	double 			m_dFishScore;			// 입질확률때 쓰이는 물고기score
	double			m_dFishResultLength;
	double			m_dFishResultWeight;
	long			m_lFishResultExp;	
	long			m_lFishResultScore;		// 물고기 랜딩 했을 경우 보상으로 주어지는 물고기 score
	LONGLONG		m_llMoney;
	std::string		m_strStartDate;
	std::string		m_strLandingDate;
	double			m_dResultBiteRate;
	BOOL			m_bIsBite;
	long			m_lPrevFishDirection;
	double			m_dChoiceRate;
	long			m_lMAXFishHP;
	long			m_lCurFishHP;
	long			m_lFishDepth;
	FishSkillInfo	m_fishSkillInfo;
	double			m_dCurrentFishAttack;
	double			m_dCurrentFishSkillAttack;
	long			m_lFishSkillMaxAttack;
	TMapFishSkillReuseCount	m_mapFishSkillReuseCount;


public:
	CFish();
	virtual ~CFish();

public:
	void Clear()
	{
		m_FishInfo.Clear();
		m_dFishScore = 0;
		m_dFishResultLength = 0;
		m_dFishResultWeight = 0;
		m_lFishResultExp = 0;
		m_lFishResultScore = 0;
		m_llMoney = 0;
		m_strStartDate.clear();
		m_strLandingDate.clear();
		m_dResultBiteRate = 0;
		m_bIsBite = 0;
		m_lPrevFishDirection = 0;
		m_dChoiceRate = 0;
		m_lMAXFishHP = 0;
		m_lCurFishHP = 0;
		m_lFishDepth = 0;
		m_fishSkillInfo.Clear();
		m_dCurrentFishAttack = 0;
		m_dCurrentFishSkillAttack = 0;
		m_lFishSkillMaxAttack = 0;
		m_mapFishSkillReuseCount.clear();
	}
	BOOL ValidCheck();
	BOOL GetFishSkillReuseTime(LONG lFishSkillIndex);
	FishInfo& GetFish() { return m_FishInfo; }
	LONG GetLandNoteFishID() { return m_FishInfo.m_lLockedNoteFishID; }
	LONG GetFishType() { return m_FishInfo.m_lFishType; }
	LONG GetFishMapID() { return m_FishInfo.m_lMapID; }
	void SetFishInfo(FishInfo& fishInfo) { m_FishInfo = fishInfo; }
	void SetBiteFishInfo(BiteFishInfo& biteFishInfo)
	{
		m_dResultBiteRate = biteFishInfo.m_bIsFishBite;
		m_dFishResultLength = biteFishInfo.m_dFishLength;
		m_dFishResultWeight = biteFishInfo.m_dFishWeight;
		m_bIsBite = biteFishInfo.m_bIsFishBite;
		m_lFishResultScore = biteFishInfo.m_lFishScore;
		m_llMoney = biteFishInfo.m_llMoney;

		SYSTEMTIME systime;
		::GetLocalTime(&systime);
		m_strStartDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
		m_strLandingDate.clear();
	}
	void GetBiteFishInfo(BiteFishInfo& biteFishInfo)
	{
		biteFishInfo.m_lFishIndex = m_FishInfo.m_lIndex;
		biteFishInfo.m_lLockedNoteFishID = m_FishInfo.m_lLockedNoteFishID;
		biteFishInfo.m_bIsFishBite = (BOOL)m_dResultBiteRate;
		biteFishInfo.m_dFishLength = m_dFishResultLength;
		biteFishInfo.m_dFishWeight = m_dFishResultWeight;
		biteFishInfo.m_bIsFishBite = m_bIsBite;
		biteFishInfo.m_lFishScore = m_lFishResultScore;
		biteFishInfo.m_llMoney = m_llMoney;
	}
	void SetLandingDate()
	{
		SYSTEMTIME systime;
		::GetLocalTime(&systime);
		m_strLandingDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	}
	void GetLockedNoteMainFishInfo(Recently_Landing_Fish& landFish)
	{
		landFish.m_lLockedNoteFishID = m_FishInfo.m_lLockedNoteFishID;
		landFish.m_lLength = (LONG)m_dFishResultLength;
		landFish.m_lWeight = (LONG)m_dFishResultWeight;
		landFish.m_lClass = m_FishInfo.m_lFishLv;
		landFish.m_strUpdateDate = m_strLandingDate;
	}
	const std::string& GetLandingDate() const { return m_strLandingDate; }
	double GetFishScore() { return m_dFishScore; }
	void SetFishScore(double dFishScore) { m_dFishScore = dFishScore; }
	void SetFishIsBite(BOOL bIsBite) { m_bIsBite = bIsBite; }
	double GetFishResultWeight() { return m_dFishResultWeight; }
	double GetFishResultLength() { return m_dFishResultLength; }
	void SetFishResultSize(double dFishResultSize) { m_dFishResultLength = dFishResultSize; }
	void SetFishResultWeight(double dFishResultWeight) { m_dFishResultWeight = dFishResultWeight; }
	void SetFishResultExp(long lFishResultExp) { m_lFishResultExp = lFishResultExp; }
	long GetFishResultExp() const { return m_lFishResultExp; }
	void SetFishResultBiteRate(double dResultBiteRate) { m_dResultBiteRate = dResultBiteRate; }
	void SetResultFishScore(long lRewardFishScore) { m_lFishResultScore = lRewardFishScore; }
	long GetResultFishScore() { return m_lFishResultScore; }
	LONGLONG GetRewardGameMoney() { return m_llMoney; }
	void SetRewardGameMoney(LONGLONG llResultGameMoney) { m_llMoney = llResultGameMoney; }
	void SetPrevFishDirection(long lPrevFishDirection) { m_lPrevFishDirection = lPrevFishDirection; }
	long GetPrevFishDirection() { return m_lPrevFishDirection; }
	double SetGetChoiceRate(double dTotalFishScore, double dTotalAddFishRate) 
	{ 
		m_dChoiceRate = m_dFishScore / dTotalFishScore;
		
		if (dTotalAddFishRate > 0)	// 입질 확률 보정
			m_dChoiceRate = (m_dChoiceRate / (100 + dTotalAddFishRate))*100;

		return m_dChoiceRate;
	}
	double GetChoiceRate() { return m_dChoiceRate; }
	void SetFishHP(long lMAXFishHP) { m_lCurFishHP = lMAXFishHP; m_lMAXFishHP = lMAXFishHP; }
	double CalcFishHP(long lCurFishHP)
	{ 
		m_lCurFishHP -= lCurFishHP;
		if (m_lCurFishHP <= 0) {
			m_lCurFishHP = 0;
			return 0;
		}

		return (double(m_lCurFishHP)/double(m_lMAXFishHP)) * 100;
	}
	LONG GetCurFishHP() { return m_lCurFishHP; }
	LONG GetMAXFishHP() { return m_lMAXFishHP; }
	void GetFishSkill(FishSkillInfo& fishSkillInfo) { fishSkillInfo = m_fishSkillInfo; }
	LONG GetFishSkillType() { return m_fishSkillInfo.m_lType; }
	void SetFishSkill(FishSkillInfo& fishSkillInfo);
	void DecrementFishSkillUseTime();
	void GetLandingFish(LandingFish& landingFish);
	LONG GetFishCurrentDepth() { return m_lFishDepth; }
	void SetFishCurrentDepth(LONG lFishDepth) { m_lFishDepth = lFishDepth; }
	LONG GetFishSkillDirection() { return m_fishSkillInfo.m_lEffectDirection; }

	double GetCurrentFishAttack() { return m_dCurrentFishAttack; }
	void SetCurrentFishAttack(double dCurrentFishAttack)
	{ 
		m_dCurrentFishAttack = dCurrentFishAttack; 
	}
	void AddCurrentFishAttack(double dAddFishPower) 
	{ 
		m_dCurrentFishAttack += dAddFishPower; 
		if (m_dCurrentFishAttack < 0)
			m_dCurrentFishAttack = 0;
	}
	double GetCurrentFishSkillAttack() { return m_dCurrentFishSkillAttack; }
	void SetCurrentFishSkillAttck(double dCurrentFishSkillAttack) 
	{ 
		m_dCurrentFishSkillAttack = dCurrentFishSkillAttack; 
	}
	void AddCurrentFishSkillAttack(double dAddFishSkillPower) 
	{ 
		m_dCurrentFishSkillAttack += dAddFishSkillPower; 
		if (m_dCurrentFishSkillAttack < 0)
			m_dCurrentFishSkillAttack = 0;
	}

	void SetFishSkillMaxAttack(long lFishSkillMaxAttack) { m_lFishSkillMaxAttack = lFishSkillMaxAttack; }
	long GetFishSkillMaxAttack() { return m_lFishSkillMaxAttack; }
};


typedef std::pair<LONG, LONG>		TPairCardRate;					// CardID, Rate
typedef std::list<TPairCardRate>	TlstCardRate;


typedef std::list<CFish>		TLstFish;

typedef std::map<long, NFLevel*> TMapNFLevel;
typedef std::map<long, NFBasicChar*> TMapNFBasicChar;
typedef std::map<long, NFUser*> TMapNFUser;
typedef std::map<long, EquipItem*> TMapIndexEquipItem;
typedef std::map<long, ClothesItem*> TMapIndexClothesItem;
typedef std::map<long, UsableItem*> TMapIndexUsableItem;
typedef std::map<long, SkillItem*> TMapIndexSkillItem;

typedef std::list<CardItem*> TlstCardItem;
typedef std::map<string, TlstCardItem>  TMapCardTypeCardItem;		// card_type, TlstCardItem
typedef std::map<long, TMapCardTypeCardItem>	TMapLevelCardItem;	// card_level, TMapCardTypeCardItem
typedef std::map<long, TlstCardRate>	TMapIndexCardPackRate;			// card_pack - rate

typedef std::map<long, CardItem*> TMapIndexCardItem;
typedef std::map<long, FishMap*> TMapIndexFishMap;
typedef std::map<long, FishInfo*> TMapIndexFishInfo;
typedef std::map<long, FishSkill*> TMapIndexFishSkill;
typedef std::map<long, FishSkillCode*> TMapIndexFishSkillCode;
typedef std::map<LONG, Product*> TMapItemIDProduct;				// <ItemID, Product>
typedef std::map<long, list<LONG> > TMapIndexPackage;
typedef std::map<LONG, TMapAddFishRate>	TMapAddFishList;

class DropItemRate
{
public:
	LONG	m_lDropItemID;
	LONG	m_lDropRate;
};
typedef std::list<DropItemRate> TlstDropItemRate;
typedef std::map<long, TlstDropItemRate> TMapDropItemRate;


typedef std::list<long> TListIndex;
typedef std::map<long, TListIndex> TMapIndexIndexList;

typedef std::vector<long> TVecUpgradeProb;


class CNFDataItemMgr
{
private:
	TMapNFLevel				m_mapNFLevel;
	TMapNFLevel				m_mapNFLevelGap;
	TMapNFBasicChar			m_mapNFBasicChar;
	TMapNFUser				m_mapNFUser;
	TMapIndexEquipItem		m_mapEquipItem;
	TMapIndexClothesItem	m_mapClothesItem;
	TMapIndexUsableItem		m_mapUsableItem;
	TMapIndexSkillItem		m_mapSkillItem;
	TMapLevelCardItem		m_mapCardPackItem;
	TMapIndexCardItem		m_mapCardItem;
	TMapIndexCardPackRate	m_mapCardPackRate;
	TMapIndexFishMap		m_mapFishMap;
	TMapIndexFishInfo		m_mapFishInfo;
	TMapIndexFishSkill		m_mapFishSkill;
	TMapIndexFishSkillCode	m_mapFishSkillCode;
	TMapIndexIndexList		m_mapFPFICode;					// FishPoint & FishInfo Code Map
	TMapItemCodeProduct		m_mapProduct;
	TMapItemCodeProduct		m_mapProductSkill;
	TMapItemIDProduct		m_mapGiveItem;
	TMapIndexPackage		m_mapPackage;
	TVecUpgradeProb			m_vecUpgradeProb;
	TMapDropItemRate		m_mapDropItemRate;
	TblAchieveInfo			m_tblAchieve;
	TMapHistoryInfo			m_mapHistoryInfo;
	TMapHistoryCode			m_mapHistoryCode;
	TMapAddFishList			m_mapAddFishList;
	TMapAchvReward			m_mapAchvReward;
	TmapNFItemEnchantInfo	m_mapNFItemEnchantInfo;

	

public:
	CNFDataItemMgr();
	virtual ~CNFDataItemMgr();

	// Get Row Data Function
	TMapNFLevel& GetNFLevel() { return m_mapNFLevel; }
	TMapNFLevel& GetNFLevelGap() { return m_mapNFLevelGap; }
	TMapNFBasicChar& GetNFBasicChar() { return m_mapNFBasicChar; }
	TMapNFUser& GetNFUser() { return m_mapNFUser; }
	TMapIndexEquipItem& GetMapEquipItem() { return m_mapEquipItem; }
	TMapIndexClothesItem& GetMapClothesItem() { return m_mapClothesItem; }
	TMapIndexUsableItem& GetMapUsableItem() { return m_mapUsableItem; }
	TMapIndexSkillItem& GetMapSkillItem() { return m_mapSkillItem; }
	TMapIndexCardItem& GetMapCardItem() { return m_mapCardItem; }				// lItemCode로 찾기 편하도록....(invald 체크)
	TMapIndexCardPackRate& GetMapCardPackRate() { return m_mapCardPackRate; }
	TMapLevelCardItem& GetMapCardPackItem() { return m_mapCardPackItem; }		// CardPack Open을 편하도록...
	TMapIndexFishMap& GetFishMap() { return m_mapFishMap; }
	TMapIndexFishInfo& GetFishInfo() { return m_mapFishInfo; }
	TMapIndexFishSkill& GetFishSkill() { return m_mapFishSkill; }
	TMapIndexFishSkillCode& GetFishSkillCode() { return m_mapFishSkillCode; }
	TMapIndexIndexList&	GetFPFICode() { return m_mapFPFICode; }
	TMapItemCodeProduct& GetProduct() { return m_mapProduct; }
	TMapItemCodeProduct& GetProductSkill() { return m_mapProductSkill; }
	TMapIndexPackage& GetPackage() { return m_mapPackage; }
	TMapItemIDProduct& GetGiveItem() { return m_mapGiveItem; }
	TVecUpgradeProb& GetUpgradeProb() { return m_vecUpgradeProb;}
	TMapDropItemRate& GetDropItemRate() { return m_mapDropItemRate; }
	TblAchieveInfo& GetHistory() { return m_tblAchieve; }
	TMapHistoryCode& GetHistoryCode() { return m_mapHistoryCode; }
	TMapAddFishList& GetAddFishList() { return m_mapAddFishList; }
	TMapAchvReward& GetMapAchvReward() { return m_mapAchvReward; }
	TmapNFItemEnchantInfo& GetMapNFItemEnchantInfo() { return m_mapNFItemEnchantInfo; }

public:
	BOOL GetFishSkillByIndex(LONG lFishSkill, FishSkill& fishSkill)
	{
		TMapIndexFishSkill::iterator iterFind = m_mapFishSkill.find(lFishSkill);
		if (iterFind != m_mapFishSkill.end())
		{
			fishSkill = *((*iterFind).second);
			return TRUE;
		}
		return FALSE;
	}

	LONG GetNFExp(LONG lLevel, LONG& lMaxExp)
	{
		NFLevel* pLevel = m_mapNFLevel[lLevel];
		if (!pLevel)
			return NF::G_ERR_GET_NF_EXP_NOT_FOUND;

		// MAX Check
		if (lLevel == (LONG)m_mapNFLevel.size()) {
			lMaxExp = pLevel->m_lNeedExp;
			return NF::G_ERR_GET_NF_EXP_MAX;
		}

		if (lLevel != pLevel->m_lLevel)
			return NF::G_ERR_GET_NF_EXP_MISSMATCH;

		lMaxExp = pLevel->m_lNeedExp;
		return NF::G_NF_ERR_SUCCESS;
	}
	LONG GetNFAbilityGapByLevel(LONG lLevel, NFAbility& ability)
	{
		NFLevel* pLevel = m_mapNFLevelGap[lLevel];
		if (!pLevel)
			return G_ERR_GET_NF_EXP_LEVEL_NOT_FOUND;

		ability = pLevel->m_ability;
		return NF::G_NF_ERR_SUCCESS;
	}
	LONG GetCharExpByLevel(LONG lLevel)
	{
		NFLevel* pLevel = m_mapNFLevel[lLevel];
		if (!pLevel)
			return NF::G_ERR_GET_NF_EXP_NOT_FOUND;

		return pLevel->m_lGetLimitExp;
	}

public:
	// Setting Function
	void SetItemVariableInit();

	BOOL GetAllResourceData(const bool bIsNGS = false);
	BOOL DeleteAllResourceData();
	BOOL SetAbilityByNFLevel();
	BOOL CalcDistFP2FP();

public:
	BOOL GetNFAbility(long lBasicCSN, NFAbility& NFAbility)
	{
		BOOL bRet = FALSE;
		TMapNFBasicChar::iterator it = m_mapNFBasicChar.find(lBasicCSN);
		if (it != m_mapNFBasicChar.end())
		{
			NFBasicChar* pNFBasicChar = (*it).second;
			if (pNFBasicChar)
			{
				NFAbility = pNFBasicChar->m_nfAbility;
				bRet = TRUE;
			}
		}
		return bRet;
	}
	const BOOL GetNFAbilityByLevel(const LONG lLEVEL, NFAbility& nfAbility) const
	{
		TMapNFLevel::const_iterator level_iter = m_mapNFLevel.find(lLEVEL);
		if( m_mapNFLevel.end() != level_iter )
		{
			const TMapNFLevel::mapped_type nfLevel = level_iter->second;
			nfAbility = nfLevel->m_ability;
		}
		else
		{
			return FALSE;
		}

		return TRUE;
	}
	NFBasicChar* GetNFBasicChar(long lCharIndex)
	{
		TMapNFBasicChar::iterator it = m_mapNFBasicChar.find(lCharIndex);
		if (it == m_mapNFBasicChar.end())
			return NULL;
		return m_mapNFBasicChar[lCharIndex];
	}
	BOOL CheckItemTypeByItemCode(const string& strType, long lItemCode, long lParts)
	{
		ItemCommon* pCommon = NULL;
		if (eItemType_Acce >= lParts || eItemType_Bags >= lParts)
			pCommon = GetClothesItemByIndex(lItemCode);
		else if (eItemType_Lure >= lParts || eItemType_FishDetector >= lParts)
			pCommon = GetEquipItemByIndex(lItemCode);
		else if (eItemType_UsableItem == lParts)
			pCommon = GetUsableItemByIndex(lItemCode);
		else if (eItemType_SkillItem == lParts)
			pCommon = GetUsableItemByIndex(lItemCode);
		else if (eItemType_FishingCard >= lParts || eItemType_FishingCard7 >= lParts)
			pCommon = GetCardItemByIndex(lItemCode);

		if (NULL == pCommon)
			return FALSE;
		
		if (pCommon->m_strUseType == strType)
			return TRUE;
		return FALSE;
	}
	EquipItem* GetEquipItemByIndex(long lItemIndex)
	{ 
		TMapIndexEquipItem::iterator it = m_mapEquipItem.find(lItemIndex);
		if (it == m_mapEquipItem.end())
			return NULL;
		return m_mapEquipItem[lItemIndex]; 
	}
	ClothesItem* GetClothesItemByIndex(long lItemIndex)
	{ 
		TMapIndexClothesItem::iterator it = m_mapClothesItem.find(lItemIndex);
		if (it == m_mapClothesItem.end())
			return NULL;
		return m_mapClothesItem[lItemIndex]; 
	}
	UsableItem* GetUsableItemByIndex(long lUsableItemIndex) 
	{ 
		TMapIndexUsableItem::iterator it = m_mapUsableItem.find(lUsableItemIndex);
		if (it == m_mapUsableItem.end())
			return NULL;
		return m_mapUsableItem[lUsableItemIndex]; 
	}
	SkillItem* GetSkillItemByIndex(long lSkillItemIndex) 
	{ 
		TMapIndexSkillItem::iterator it = m_mapSkillItem.find(lSkillItemIndex);
		if (it == m_mapSkillItem.end())
			return NULL;
		return m_mapSkillItem[lSkillItemIndex]; 
	}
	CardItem* GetCardItemByIndex(long lCardItemIndex)
	{
		TMapIndexCardItem::iterator it = m_mapCardItem.find(lCardItemIndex);
		if (it == m_mapCardItem.end())
			return NULL;
		return m_mapCardItem[lCardItemIndex]; 
	}
	FishMap* GetFishMapByIndex(long lFishMapIndex)
	{
		TMapIndexFishMap::iterator it = m_mapFishMap.find(lFishMapIndex);
		if (it == m_mapFishMap.end())
			return NULL;
		return m_mapFishMap[lFishMapIndex];
	}
	SkillItem* GetNFSkillByIndex(long lSkillIndex)
	{
		TMapIndexSkillItem::iterator it = m_mapSkillItem.find(lSkillIndex);
		if (it == m_mapSkillItem.end())
			return NULL;
		return m_mapSkillItem[lSkillIndex];
	}
	FishingPoint* GetFishingPointByIndex(long lMapIndex, long lFishingPointIndex)
	{
		TMapIndexFishMap::iterator it = m_mapFishMap.find(lMapIndex);
		if (it == m_mapFishMap.end())	return NULL;
		
		FishMap* pFishMap = (*it).second;
		if (!pFishMap)	return NULL;

		TMapFishingPoint::iterator it2 = pFishMap->m_mapFishingPoint.find(lFishingPointIndex);
		if (it2 == pFishMap->m_mapFishingPoint.end()) return NULL;

		return &(*it2).second;
	}
	FishInfo* GetFishInfoByIndex(long lFishInfoIndex)
	{
		TMapIndexFishInfo::iterator it = m_mapFishInfo.find(lFishInfoIndex);
		if (it == m_mapFishInfo.end())	return NULL;
		return m_mapFishInfo[lFishInfoIndex];
	}
	void GetAddFishListByID(long lAddFISH_ID, TMapAddFishRate& mapAddFishRate)
	{
		TMapAddFishList::iterator it = m_mapAddFishList.find(lAddFISH_ID);
		if (it == m_mapAddFishList.end()) return;
		mapAddFishRate = m_mapAddFishList[lAddFISH_ID];
	}
	BOOL GetHistoryByHistoryIndex(LONG lHistoryIndex, HistoryInfo& historyInfo)
	{
		TMapHistoryInfo::iterator it = m_mapHistoryInfo.find(lHistoryIndex);
		if (it == m_mapHistoryInfo.end())	return FALSE;
		historyInfo = m_mapHistoryInfo[lHistoryIndex];
		return TRUE;
	}
	BOOL GetNFAchievementByGroupIndex(LONG lAchievementGroupIndex, MapAchievementInfo& mapAchiment)
	{ 
		TblAchieveInfo::iterator it = m_tblAchieve.find(lAchievementGroupIndex);
		if (it == m_tblAchieve.end())	return FALSE;
		mapAchiment = m_tblAchieve[lAchievementGroupIndex];
		return TRUE;
	}
	BOOL GetAchvReward(LONG lAchvID, NFReward& reward)
	{
		TMapAchvReward::iterator it = m_mapAchvReward.find(lAchvID);
		if (it == m_mapAchvReward.end())	return FALSE;
		reward = m_mapAchvReward[lAchvID];
		return TRUE;
	}
	BOOL GetProductFromGiveItem(LONG lGiveItemID, Product& product)
	{
		TMapItemIDProduct::iterator it = m_mapGiveItem.find(lGiveItemID);
		if (it == m_mapGiveItem.end())	return FALSE;
		product = *m_mapGiveItem[lGiveItemID];
		return TRUE;
	}
	BOOL GetProductByItemCode(LONG lItemCode, LONG lItemID, Product& product)
	{
		TMapItemCodeProduct::iterator itFind;
		if (eItemType_SkillItem == lItemCode/G_VALUE_CONVERT_PARTS) {
			itFind = m_mapProductSkill.find(lItemCode);
			if (itFind == m_mapProductSkill.end()) return FALSE;
		}
		else {
			itFind = m_mapProduct.find(lItemCode);
			if (itFind == m_mapProduct.end()) return FALSE;
		}

		ForEachElmt(TlstProduct, (*itFind).second, it, ij)
		{
			if ((*it).m_lItemID == lItemID)
			{
				product = (*it);
				return TRUE;
			}
		}
		return FALSE;
	}
	BOOL GetNFItemEnchantInfo(LONG lEnchantType, LONG lEnchantLevel, NFItemEnchantInfo& info)
	{
		TPairEnchantKey key(lEnchantType, lEnchantLevel);
		TmapNFItemEnchantInfo::const_iterator c_itor = m_mapNFItemEnchantInfo.find(key);
		
		if( m_mapNFItemEnchantInfo.end() != c_itor )
		{
			info = c_itor->second;
			return TRUE;
		}

		return FALSE;
	}

public:
	BOOL AddNFLevel(NFLevel* pLevel);
	BOOL AddNFBasicChar(NFBasicChar* pBasicChar);
	BOOL AddEquipItem(EquipItem* pItem);
	BOOL AddClothesItem(ClothesItem* pItem);
	BOOL AddUsableItem(UsableItem* pItem);
	BOOL AddFishMap(FishMap* pItem);
	BOOL AddFishInfo(FishInfo* pItem);

	// Common Calc
	void AddAblitity(NFUser& nfUI, LONG lPartIndex, LONG llItemCode);
	long AddApplyUseItem(NFAbilityExt& nfAbilityExt, NFInvenSlot& useInven);
	void CalcAblilty(NFAbility& nfAbility, const NFAbilityExt* pItem, BOOL bIsAdd);
	BOOL CalcItemAbility(NFUser& nfUI, long llItemCode, long lPartIndex, BOOL bIsAdd=TRUE);
	void CalcUsableItemAbility(NFAbilityExt& nfAbilityExt, const UsableItem* usableItem, BOOL bIsAdd=TRUE);
	void CalcSkillItemAbility(NFAbilityExt& nfAbilityExt, const SkillItem* skillItem, BOOL bIsAdd=TRUE);
	LONG GetCapacityByItemCode(LONG lItemCode);
};


extern CNFDataItemMgr	theNFDataItemMgr;


#endif //!NFGAMEDATA_H
