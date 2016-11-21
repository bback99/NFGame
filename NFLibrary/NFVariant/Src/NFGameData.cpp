#include "stdafx.h"
#include <NF/ADL/MsgNFCommonStruct.h>
#include <NFVariant/NFGameData.h>
#include <NFVariant/NFDBManager.h>


CNFDataItemMgr	theNFDataItemMgr;


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CNFCharInfo::CNFCharInfo()
// {
// }
// 
// CNFCharInfo::~CNFCharInfo()
// {
// }



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CFish::CFish() : m_dFishScore(0)
{

}

CFish::~CFish()
{

}

BOOL CFish::ValidCheck()
{
	BOOL bRet = FALSE;
	if (m_FishInfo.m_lIndex < 0)
		return bRet;
	if (m_dFishScore <= 0)
		return bRet;
	if (m_dFishResultLength <= 0)
		return bRet;
	if (m_dFishResultWeight <= 0)
		return bRet;
// 	if (m_lFishResultExp <= 0)
// 		return bRet;

	bRet = TRUE;
	return bRet;
}

BOOL CFish::GetFishSkillReuseTime(LONG lFishSkillIndex)
{
	BOOL bRet = TRUE;

	TMapFishSkillReuseCount::iterator iterFind = m_mapFishSkillReuseCount.find(lFishSkillIndex);
	if (iterFind != m_mapFishSkillReuseCount.end())
	{
		// 재사용 남은 시간이 있으면... 발동한 스킬은 GG
		if ( (*iterFind).second > 0)
			bRet = FALSE;
	}
	return bRet;
}

void CFish::SetFishSkill(FishSkillInfo& fishSkillInfo)//(LONG lFishSkillIndex, LONG lFishSkillType, LONG lFishSkillcount)
{
	m_fishSkillInfo = fishSkillInfo;
	m_mapFishSkillReuseCount[m_fishSkillInfo.m_lFishSkillIndex] = fishSkillInfo.m_lReuseTime;
}

void CFish::DecrementFishSkillUseTime()
{
	if (!m_mapFishSkillReuseCount.size())
		return;


	ForEachElmt(TMapFishSkillReuseCount, m_mapFishSkillReuseCount, it, ij)
	{
		LONG lIndex = (*it).first;
		LONG lCount = (*it).second;

		if (lCount < 0)
			lCount = 0;
		else
			--lCount;

		m_mapFishSkillReuseCount[lIndex] = lCount;
	}
}

void CFish::GetLandingFish(LandingFish& landingFish)
{
	landingFish.m_lFishID = m_FishInfo.m_lIndex;
	landingFish.m_lGroupID = m_FishInfo.m_lLockedNoteFishID;
	landingFish.m_lClass = m_FishInfo.m_lFishClass;
	landingFish.m_dResultSize = m_dFishResultLength;
	landingFish.m_dResultWeight = m_dFishResultWeight;
	landingFish.m_lResultScore = m_lFishResultScore;
	landingFish.m_lExp = m_lFishResultExp;

	// test
	landingFish.m_lBonusExp = 10;
	landingFish.m_llMoney = m_llMoney;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CNFDataItemMgr::CNFDataItemMgr()
{
	m_vecUpgradeProb.reserve(11);
	m_vecUpgradeProb.push_back(0);
	m_vecUpgradeProb.push_back(100);
	m_vecUpgradeProb.push_back(100);
	m_vecUpgradeProb.push_back(100);
	m_vecUpgradeProb.push_back(80);
	m_vecUpgradeProb.push_back(60);
	m_vecUpgradeProb.push_back(40);
	m_vecUpgradeProb.push_back(30);
	m_vecUpgradeProb.push_back(20);
	m_vecUpgradeProb.push_back(10);
	m_vecUpgradeProb.push_back(5);
}

CNFDataItemMgr::~CNFDataItemMgr()
{

}

void CNFDataItemMgr::SetItemVariableInit()
{
	//CString strChargeType[] = {_T("무료"), _T("기간제"), _T("영구적"), _T("etc")};
	//CString strEquipItemType[] = {_T("ROD"), _T("스피닝릴"), _T("베이트릴"), _T("라인"), _T("루어"), _T("웜채비"), _T("보트"), _T("어군탐지기"), _T("소모성 아이템"), _T("기타 아이템"), _T("이벤트 아이템"), _T("없음")}; 
	//CString strFishingPointType[] = {_T("Walk"), _T("Boat")};
	//CString strFishInfoSalt[] = {_T("담수어"), _T("해수어")};
	//CString strFishinfoClass[] = {_T("0")};
	//CString strFishinfoType[] = {_T("0")};
	//CString strCharType[] = {_T(""), _T("남자"), _T("여자"), _T("흐긴")};
	//CString strHairType[] = {_T(""), _T("짧은머리"), _T("긴머리"), _T("파마머리"), _T("단발머리"), _T("스포츠머리")};
	//CString strFaceType[] = {_T(""), _T("달갈형"), _T("네모형"), _T("세모형")};
}

BOOL CNFDataItemMgr::GetAllResourceData(const bool bIsNGS)		// 데이터 로딩 우선 순위에 주의 할것!!!!!!!!!!!
{
	BOOL bRet = TRUE;

//	char szRoomIndex[1024] = {0,};
//	sprintf(szRoomIndex, "%.3d", 11);
//	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, szRoomIndex);


	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "##### DB로 부터 데이터 로딩 시작 #####");

	theNFDBMgr.SelectNFLevel(theNFDataItemMgr.GetNFLevel());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Level Cnt :", theNFDataItemMgr.GetNFLevel().size());

	theNFDBMgr.SelectNFBasicChar(theNFDataItemMgr.GetNFBasicChar());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Basic Char Cnt :", theNFDataItemMgr.GetNFBasicChar().size());

	theNFDBMgr.SelectNFProduct(theNFDataItemMgr.GetProduct(), theNFDataItemMgr.GetProductSkill());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Product Cnt :", theNFDataItemMgr.GetProduct().size()+theNFDataItemMgr.GetProductSkill().size());

	theNFDBMgr.SelectNFProductPackage(theNFDataItemMgr.GetPackage());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Package Cnt :", theNFDataItemMgr.GetPackage().size());

	theNFDBMgr.SelectNFGiveItem(theNFDataItemMgr.GetGiveItem());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF GiveItem Cnt :", theNFDataItemMgr.GetGiveItem().size());

	theNFDBMgr.SelectAchvReward(theNFDataItemMgr.GetMapAchvReward());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF AchvReward Cnt :", theNFDataItemMgr.GetMapAchvReward().size());

	theNFDBMgr.SelectNFItemEnchantInfo(theNFDataItemMgr.GetMapNFItemEnchantInfo());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF ItemEnchantInfo Cnt :", theNFDataItemMgr.GetMapNFItemEnchantInfo().size());

	if (bIsNGS)
	{
		// 레벨에 따른 능력치의 차이를 NGS만 읽어온다.
		theNFDataItemMgr.SetAbilityByNFLevel();

		// Item을 읽어오기전에 장비에 따른 물고기 추가 정보를 읽어온다.
		theNFDBMgr.SelectNFAddFishList(TRUE, theNFDataItemMgr.GetAddFishList());
		theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF AddFishList Cnt :", theNFDataItemMgr.GetAddFishList().size());
	}

	// Item들을 읽어오기전에 Func부터 읽어와서 능력치를 셋팅한다.
	theNFDBMgr.SelectNFItemEquip(TRUE, theNFDataItemMgr.GetMapEquipItem());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF EquipItem Cnt :", theNFDataItemMgr.GetMapEquipItem().size());

	theNFDBMgr.SelectNFItemClothes(TRUE, theNFDataItemMgr.GetMapClothesItem());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF ClothesItem Cnt :", theNFDataItemMgr.GetMapClothesItem().size());

	theNFDBMgr.SelectNFItemUsable(TRUE, theNFDataItemMgr.GetMapUsableItem());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF UsableItem Cnt :", theNFDataItemMgr.GetMapUsableItem().size());

	theNFDBMgr.SelectNFItemSkill(TRUE, theNFDataItemMgr.GetMapSkillItem());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF SkillItem Cnt :", theNFDataItemMgr.GetMapSkillItem().size());

	theNFDBMgr.SelectNFItemCard(TRUE, theNFDataItemMgr.GetMapCardPackItem(), theNFDataItemMgr.GetMapCardItem());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF CardItem Cnt :", theNFDataItemMgr.GetMapCardItem().size());

	LONG lCnt = 0;
	theNFDBMgr.SelectNFItemCardPackRate(TRUE, theNFDataItemMgr.GetMapCardPackRate(), lCnt);
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF CardPackRate Cnt :", lCnt);

	if (bIsNGS)
	{
		theNFDBMgr.SelectNFFishMap(TRUE, theNFDataItemMgr.GetFishMap());
		theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FishMap Cnt :", theNFDataItemMgr.GetFishMap().size());

		theNFDBMgr.SelectNFFishingPoint(TRUE, theNFDataItemMgr.GetFishMap());

		theNFDBMgr.SelectNFFishInfo(TRUE, theNFDataItemMgr.GetFishInfo());
		if (theNFDataItemMgr.GetFishInfo().size() <= 0)
			return FALSE;
		theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FishInfo :", theNFDataItemMgr.GetFishInfo().size());

		theNFDBMgr.SelectNFFishSkill(TRUE, theNFDataItemMgr.GetFishSkill());
		theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FishSkill :", theNFDataItemMgr.GetFishSkill().size());

		theNFDBMgr.SelectNFFishSkillCode(TRUE, theNFDataItemMgr.GetFishSkillCode());
		theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FishSkillCode :", theNFDataItemMgr.GetFishSkillCode().size());

		theNFDBMgr.SelectNFFishingCode(TRUE, theNFDataItemMgr.GetFPFICode());
		theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FPFICode :", theNFDataItemMgr.GetFPFICode().size());

		lCnt = 0;
		theNFDBMgr.SelectNFSignCode(TRUE, theNFDataItemMgr.GetFishMap(), lCnt);
		theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Sign&SpecialFish :", lCnt);

		theNFDBMgr.SelectNFDropItem(TRUE, theNFDataItemMgr.GetDropItemRate(), lCnt);
		theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF DropItemRate :", lCnt);
	}
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "##### DB로 부터 데이터 로딩 끝 #####");

	return bRet;
}

BOOL CNFDataItemMgr::DeleteAllResourceData()
{
	BOOL bRet = TRUE;

	//theApp.Log(_T("##### 메모리에 있는 데이터 삭제 시작 !!! #####"));

	// Char

	// EquipItem
	long lCnt = m_mapEquipItem.size();
	for(TMapIndexEquipItem::iterator it = m_mapEquipItem.begin(); it != m_mapEquipItem.end(); it++)
	{
		EquipItem* pDelData = (EquipItem*)(*it).second;
		if (pDelData)
			delete pDelData;
	}
	m_mapEquipItem.clear();
	//theApp.Log(format(_T("Delete EquipItem Cnt : %d"), lCnt));

	// ClothesItem
	lCnt = m_mapClothesItem.size();
	for(TMapIndexClothesItem::iterator it = m_mapClothesItem.begin(); it != m_mapClothesItem.end(); it++)
	{
		ClothesItem* pDelData = (ClothesItem*)(*it).second;
		if (pDelData)
			delete pDelData;
	}
	m_mapClothesItem.clear();
	//theApp.Log(format(_T("Delete ClothesItem Cnt : %d"), lCnt));

	// UsableItem
	lCnt = m_mapUsableItem.size();
	for(TMapIndexUsableItem::iterator it = m_mapUsableItem.begin(); it != m_mapUsableItem.end(); it++)
	{
		UsableItem* pDelData = (UsableItem*)(*it).second;
		if (pDelData)
			delete pDelData;
	}
	m_mapUsableItem.clear();
	//theApp.Log(format(_T("Delete UsableItem Cnt : %d"), lCnt));

	// Map
	lCnt = m_mapFishMap.size();
	for(TMapIndexFishMap::iterator it = m_mapFishMap.begin(); it != m_mapFishMap.end(); it++)
	{
		FishMap* pDelData = (FishMap*)(*it).second;
		if (pDelData)
			delete pDelData;
	}
	m_mapFishMap.clear();
	//theApp.Log(format(_T("Delete FishMap Cnt : %d"), lCnt));

	// Fish
	lCnt = m_mapFishInfo.size();
	for(TMapIndexFishInfo::iterator it = m_mapFishInfo.begin(); it != m_mapFishInfo.end(); it++)
	{
		FishInfo* pDelData = (FishInfo*)(*it).second;
		if (pDelData)
			delete pDelData;
	}
	m_mapFishInfo.clear();
	//theApp.Log(format(_T("Delete FishInfo Cnt : %d"), lCnt));

	// FishingPoint - FishInfo Code
	lCnt = m_mapFPFICode.size();
	m_mapFPFICode.clear();
	//theApp.Log(format(_T("Delete FPFICode Cnt : %d"), lCnt));

	//theApp.Log(_T("##### 메모리에 있는 데이터 삭제 끝 !!! #####"));

	return bRet;
}

BOOL CNFDataItemMgr::SetAbilityByNFLevel()
{
	BOOL bRet = TRUE;

	NFLevel		level;
	level.Clear();

	ForEachElmt(TMapNFLevel, m_mapNFLevel, it, ij)
	{
		const NFLevel* pNextLevel = (NFLevel*)(*it).second;
		if (!pNextLevel)
			return FALSE;

		NFLevel* pLevelGap = new NFLevel;
		if (!pLevelGap)
			return FALSE;

		if (level.m_lLevel != 0)
		{
			pLevelGap->SetAbilityGap(*pNextLevel, level);
		}

		level = *pNextLevel;
		m_mapNFLevelGap[pNextLevel->m_lLevel] = pLevelGap;
	}

	return bRet;
}


// @@@ 수정해야 한다. -2010.02.24
BOOL CNFDataItemMgr::CalcDistFP2FP()
{
	BOOL bRet = FALSE;

	ForEachElmt(TMapIndexFishMap, m_mapFishMap, it, ij)
	{
		FishMap* fishMap = (FishMap *)(*it).second;
		if (!fishMap)
			return FALSE;

		ForEachElmt(TMapFishingPoint, fishMap->m_mapFishingPoint, it2, ij2)
		{
			FishingPoint fpFirst = (*it2).second;

			ForEachElmt(TMapFishingPoint, fishMap->m_mapFishingPoint, it3, ij3)
			{
				FishingPoint fpSecond = (*it3).second;

				if (fpFirst.m_lIndex == fpSecond.m_lIndex)
					continue;

				long lXGap = fpSecond.m_lLocation_X - fpFirst.m_lLocation_X;
				long lYGap = fpSecond.m_lLocation_Y - fpFirst.m_lLocation_Y;

				double dDist = sqrt(double(abs(lXGap) * abs(lXGap) + abs(lYGap) * abs(lYGap)));

				if (dDist <= 60)//NF_FISHINGPOINT_VALID_DIST)
				{
					fpFirst.m_lstRelatoinFPIndex.push_back(fpSecond.m_lIndex);
				}
			}
		}
	}

	return bRet;
}

BOOL CNFDataItemMgr::AddNFLevel(NFLevel* pLevel)
{
	BOOL bRet = TRUE;

	m_mapNFLevel[pLevel->m_lLevel] = pLevel;

	return bRet;
}

BOOL CNFDataItemMgr::AddNFBasicChar(NFBasicChar* pBasicChar)
{
	BOOL bRet = TRUE;

	m_mapNFBasicChar[pBasicChar->m_lIndex] = pBasicChar;

	return bRet;
}

BOOL CNFDataItemMgr::AddEquipItem(EquipItem* pItem)
{
	BOOL bRet = TRUE;

	m_mapEquipItem[pItem->m_lItemCode] = pItem;

	return bRet;
}

BOOL CNFDataItemMgr::AddClothesItem(ClothesItem* pItem)
{
	BOOL bRet = TRUE;

	m_mapClothesItem[pItem->m_lItemCode] = pItem;

	return bRet;
}

BOOL CNFDataItemMgr::AddUsableItem(UsableItem* pItem)
{
	BOOL bRet = TRUE;

	m_mapUsableItem[pItem->m_lItemCode] = pItem;

	return bRet;
}

BOOL CNFDataItemMgr::AddFishMap(FishMap* pItem)
{
	BOOL bRet = TRUE;

	m_mapFishMap[pItem->m_lIndex] = pItem;

	return bRet;
}

BOOL CNFDataItemMgr::AddFishInfo(FishInfo* pItem)
{
	BOOL bRet = TRUE;

	m_mapFishInfo[pItem->m_lIndex] = pItem;

	return bRet;
}

void CNFDataItemMgr::AddAblitity(NFUser& NFUI, LONG lPartIndex, LONG llItemCode)
{
	// Invalid PartsIndex
	switch(lPartIndex)
	{
	case eItemType_Hair :
	case eItemType_Acce :
	case eItemType_Jack :
	case eItemType_Pant :
	case eItemType_Glov :
	case eItemType_Foot :
	case eItemType_Bags :
	case eItemType_Lure :
	case eItemType_Rod :
	case eItemType_Reel :
	case eItemType_Line :
	case eItemType_Boat :
	case eItemType_FishDetector :
		{
			if (!CalcItemAbility(NFUI, llItemCode, lPartIndex))
				theLog.Put(ERR_UK, "CNFDataItemMgr"_COMMA, "AddEquipItemAbility is Fail!!!, Char USN: ", NFUI.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN, ", PartsIndex : ", llItemCode);
		}
		break;
	default:
		break;
	}
}

long CNFDataItemMgr::AddApplyUseItem(NFAbilityExt& NFUI, NFInvenSlot& useInven)
{
	const UsableItem* pUsableItem = NULL;
	const SkillItem* pSkillItem = NULL;

	// NFInvenSlot에서 가져온 Item정보의 Type(Usable인지, Skill인지를 알아낸다.)
	if (useInven.m_lItemCode >= eItemType_UsableItem * 10000 && useInven.m_lItemCode < eItemType_Fish * 10000)			// Usable
	{
		pUsableItem = theNFDataItemMgr.GetUsableItemByIndex(useInven.m_lItemCode);

		// 읽어온 아이템정보를 능력치에 더한다.
		theNFDataItemMgr.CalcUsableItemAbility(NFUI, pUsableItem);		
	}
	else if (useInven.m_lItemCode >= eItemType_SkillItem * 10000 && useInven.m_lItemCode < eItemType_EventItem * 10000)	// Skill
	{
		pSkillItem = theNFDataItemMgr.GetSkillItemByIndex(useInven.m_lItemCode);

		// 읽어온 아이템정보를 능력치에 더한다.
		theNFDataItemMgr.CalcSkillItemAbility(NFUI, pSkillItem);
	}
	else
		return -2;					// 사용한 아이템이 Usable이나 Skill이 아니다.

	return 1;
}

// 1. bIsOnlyAdd = TRUE, Add만 bIsOnlyAdd = FALSE, Remove + Add
// 2. Debuff를 위한 환경속성의 값을 여기서 체크 한다.
BOOL CNFDataItemMgr::CalcItemAbility(NFUser& NFUI, long llItemCode, long lPartIndex, BOOL bIsAdd)
{
	if (llItemCode <= 0)
		return FALSE;

	NFCharInfoExt& nfCharInfoExt = NFUI.m_nfCharInfoExt;

	switch(lPartIndex)
	{
	case eItemType_Hair:
	case eItemType_Acce:
	case eItemType_Jack:
	case eItemType_Pant:
	case eItemType_Glov:
	case eItemType_Foot:
	case eItemType_Bags:
		{
			// Add
			ClothesItem* pItem = theNFDataItemMgr.GetClothesItemByIndex(llItemCode);
			if (pItem)
				CalcAblilty(nfCharInfoExt.m_nfAbility, &pItem->m_nfAbilityExt, bIsAdd);
			else 
				return FALSE;

			if (bIsAdd)	// 추가하는 아이템이고	
			{	
				// 환경속성에 영향을 받는 아이템이면, 해당아이템의 환경속성의 값을 저장
				if (lPartIndex == eItemType_Jack || lPartIndex == eItemType_Pant || lPartIndex == eItemType_Glov || lPartIndex == eItemType_Foot)
					nfCharInfoExt.m_nfCheckDebuff[lPartIndex] = pItem->m_lEnvAttribute;
			}

			break;
		}
	case eItemType_Lure:
	case eItemType_Rod:
	case eItemType_Reel:
	case eItemType_Line:
	case eItemType_Boat:
	case eItemType_FishDetector:
		{
			// Add
			EquipItem* pItem = theNFDataItemMgr.GetEquipItemByIndex(llItemCode);
			if (pItem)
				CalcAblilty(nfCharInfoExt.m_nfAbility, &pItem->m_nfAbilityExt, bIsAdd);
			else 
				return FALSE;

			break;
		}
	default:
		break;
	}

	return TRUE;
}


// bIsAdd = TRUE, +
// bIsAdd = FALSE, -
void CNFDataItemMgr::CalcAblilty(NFAbility& nfAbility, const NFAbilityExt* pItem, BOOL bIsAdd)
{
	int lSign = 1;
	if (!bIsAdd)
		lSign = lSign * -1;

 	nfAbility.m_dFlyDist += (pItem->m_nfAbility.m_dFlyDist*lSign);
 	if (nfAbility.m_dFlyDist < 0)
 		nfAbility.m_dFlyDist = 0;
 
 	nfAbility.m_dCharm += (pItem->m_nfAbility.m_dCharm*lSign);
 	if (nfAbility.m_dCharm < 0)
 		nfAbility.m_dCharm = 0;
 
 	nfAbility.m_dStrength += (pItem->m_nfAbility.m_dStrength*lSign);
 	if (nfAbility.m_dStrength < 0)
 		nfAbility.m_dStrength = 0;
 
 	nfAbility.m_dAgility += (pItem->m_nfAbility.m_dAgility*lSign);
 	if (nfAbility.m_dAgility < 0)
 		nfAbility.m_dAgility = 0;

	nfAbility.m_dControl += (pItem->m_nfAbility.m_dControl*lSign);
	if (nfAbility.m_dControl < 0)
		nfAbility.m_dControl = 0;
 
 	nfAbility.m_dHealth -= (pItem->m_nfAbility.m_dHealth*lSign);
 	if (nfAbility.m_dHealth < 0)
 		nfAbility.m_dHealth = 0;

	nfAbility.m_dFishPoint += (pItem->m_nfAbility.m_dFishPoint*lSign);
	if (nfAbility.m_dFishPoint < 0)
		nfAbility.m_dFishPoint = 0;

	nfAbility.m_dLuckyPoint += (pItem->m_nfAbility.m_dLuckyPoint*lSign);
	if (nfAbility.m_dLuckyPoint < 0)
		nfAbility.m_dLuckyPoint = 0;
}

void CNFDataItemMgr::CalcUsableItemAbility(NFAbilityExt& nfAbilityExt, const UsableItem* pItem, BOOL bIsAdd)
{
	int lSign = 1;
	if (!bIsAdd)
		lSign = lSign * -1;

	CalcAblilty(nfAbilityExt.m_nfAbility, &pItem->m_nfAbilityExt, bIsAdd);
}

void CNFDataItemMgr::CalcSkillItemAbility(NFAbilityExt& nfAbilityExt, const SkillItem* pItem, BOOL bIsAdd)
{
	int lSign = 1;
	if (!bIsAdd)
		lSign = lSign * -1;

	CalcAblilty(nfAbilityExt.m_nfAbility, &pItem->m_nfAbilityExt, bIsAdd);

 	nfAbilityExt.m_dCastingScore += (pItem->m_nfAbilityExt.m_dCastingScore*lSign);
 	if (nfAbilityExt.m_dCastingScore < 0)
 		nfAbilityExt.m_dCastingScore = 0;
 
 	nfAbilityExt.m_dCastingBacklashRate += (pItem->m_nfAbilityExt.m_dCastingBacklashRate*lSign);
 	if (nfAbilityExt.m_dCastingBacklashRate < 0)
 		nfAbilityExt.m_dCastingBacklashRate = 0;
 
 	nfAbilityExt.m_lActionIncountScore += (pItem->m_nfAbilityExt.m_lActionIncountScore*lSign);
 	if (nfAbilityExt.m_lActionIncountScore < 0)
 		nfAbilityExt.m_lActionIncountScore = 0;
 
 	nfAbilityExt.m_lReduceRunTime += (pItem->m_nfAbilityExt.m_lReduceRunTime*lSign);
 	if (nfAbilityExt.m_lReduceRunTime < 0)
 		nfAbilityExt.m_lReduceRunTime = 0;
 
 	nfAbilityExt.m_dSpecialFishBite += (pItem->m_nfAbilityExt.m_dSpecialFishBite*lSign);
 	if (nfAbilityExt.m_dSpecialFishBite < 0)
 		nfAbilityExt.m_dSpecialFishBite = 0;
 
 	nfAbilityExt.m_dKeepLuckyFlag += (pItem->m_nfAbilityExt.m_dKeepLuckyFlag*lSign);
 	if (nfAbilityExt.m_dKeepLuckyFlag < 0)
 		nfAbilityExt.m_dKeepLuckyFlag = 0;
 
 	nfAbilityExt.m_dAddRateExp += (pItem->m_nfAbilityExt.m_dAddRateExp*lSign);
 	if (nfAbilityExt.m_dAddRateExp < 0)
 		nfAbilityExt.m_dAddRateExp = 0;
 
 	nfAbilityExt.m_dAddRateGameMoney += (pItem->m_nfAbilityExt.m_dAddRateGameMoney*lSign);
 	if (nfAbilityExt.m_dAddRateGameMoney < 0)
 		nfAbilityExt.m_dAddRateGameMoney = 0;
}

LONG CNFDataItemMgr::GetCapacityByItemCode(LONG lItemCode)
{
	LONG	lPartsIndex	= lItemCode/G_VALUE_CONVERT_PARTS;

	if (lPartsIndex >= eItemType_Bags && lPartsIndex <= eItemType_FishDetector)
	{
		EquipItem* pItem = GetEquipItemByIndex(lItemCode);
		if (pItem)
			return pItem->m_lCapacity;
	}
	else if (lPartsIndex == eItemType_UsableItem)
	{
		UsableItem* pItem = GetUsableItemByIndex(lItemCode);
		if (pItem)
			return pItem->m_lCapacity;
	}
	else if (lPartsIndex >= eItemType_Hair && lPartsIndex <= eItemType_Foot)
		return 1;
	else if (lPartsIndex == eItemType_SkillItem) 
		return 1;
	else if (lPartsIndex >= eItemType_FishingCard && eItemType_FishingCard7)
	{
		CardItem* pItem = GetCardItemByIndex(lItemCode);
		if (pItem)
			return pItem->m_lCapacity;
	}
	else
		return 0;
	return 0;
}

// BOOL CNFDataItemMgr::AddFPFICode(long lFishingPointIndex, std::string& strListFishInfo)
// {
// 	BOOL bRet = TRUE;
// 
// 	// FishInfo List를 "|"로 파싱해서 FishingPoint FishInfo 코드에 넣기
// 	std::string strRet;
// 	while(true)
// 	{
// 		size_t nIndex = strListFishInfo.find_first_of("|");
// 		if (nIndex != -1)
// 		{
// 			strRet = strListFishInfo.substr(0, nIndex);
// 			strListFishInfo.erase( 0, nIndex + 1 );
// 
// 			// FishList를 저장한다.
// 			TPairIndexNIndex	pairKey;
// 			pairKey.first = lFishingPointIndex;
// 			pairKey.second = atoi(strRet.c_str());
// 
// 			m_mapFPFICode[pairKey] = pairKey.second;
// 		}
// 		else
// 			break;
// 	}
// 
// 	return bRet;
// }
// 
// BOOL CNFDataItemMgr::AddFPFICode(TPairIndexNIndex& pairFindKey)
// {
// 	BOOL bRet = FALSE;
// 
// 	if (!FindFPFICode(pairFindKey))			// 검색해서 없어야 추가 할 수 있다.
// 	{
// 		m_mapFPFICode[pairFindKey] = pairFindKey.second;
// 		bRet = TRUE;
// 	}
// 
// 	return bRet;
// }
//
//BOOL CNFDataItemMgr::FindFPFICode(TPairIndexNIndex& pairFindKey)
//{
//	BOOL bRet = FALSE;
//
//	TMapIndexIndex::iterator it = m_mapFPFICode.find(pairFindKey);
//	if (it != m_mapFPFICode.end())
//		bRet = TRUE;
//
//	return bRet;
//}
//
//BOOL CNFDataItemMgr::DeleteFPFICode(TPairIndexNIndex& pairDelKey)
//{
//	BOOL bRet = FALSE;
//
//	TMapIndexIndex::iterator it = m_mapFPFICode.find(pairDelKey);
//	if (it != m_mapFPFICode.end())
//	{
//		m_mapFPFICode.erase(it);
//		bRet = TRUE;
//	}
//
//	return bRet;
//}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////