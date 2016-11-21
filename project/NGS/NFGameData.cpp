#include "stdafx.h"
#include <ADL/MsgNFCommonStruct.h>
//#include <ADL/MsgNGSCli.h>
#include "NFGameData.h"
#include "NFDBManager.h"


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
	if (m_dFishResultSize <= 0)
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

void CFish::SetFishSkill(FishSkillInfo& fishSkillInfo, LONG lCurrentPatternType)//(LONG lFishSkillIndex, LONG lFishSkillType, LONG lFishSkillcount)
{
	m_fishSkillInfo = fishSkillInfo;
	m_lCurrentPatternType = lCurrentPatternType;
	m_mapFishSkillReuseCount[m_fishSkillInfo.m_lFishSkillIndex] = fishSkillInfo.m_lRunTime;
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
	landingFish.m_lFishIndex = m_FishInfo.m_lIndex;
	landingFish.m_dResultSize = m_dFishResultSize;
	landingFish.m_dResultWeight = m_dFishResultWeight;
	landingFish.m_dScore = m_dFishScore;
	landingFish.m_lExp = m_lFishResultExp;

	// test
	landingFish.m_lBonusExp = 10;
	landingFish.m_lMoney = 100;
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

BOOL CNFDataItemMgr::GetAllResourceData()
{
	BOOL bRet = TRUE;

	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "##### DB로 부터 데이터 로딩 시작 #####");

#ifdef _NCSNLS
	LONG lStoreType = 0;
#else
	LONG lStoreType = 1;

	theNFDBMgr.SelectNFLevel(theNFDataItemMgr.GetNFLevel());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Level Cnt :", theNFDataItemMgr.GetNFLevel().size());

	theNFDBMgr.SelectNFBasicChar(theNFDataItemMgr.GetNFBasicChar());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Basic Char Cnt :", theNFDataItemMgr.GetNFBasicChar().size());
#endif

	theNFDBMgr.SelectNFProduct(theNFDataItemMgr.GetProduct(), theNFDataItemMgr.GetProductSkill(), lStoreType);
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Product Cnt :", theNFDataItemMgr.GetProduct().size()+theNFDataItemMgr.GetProductSkill().size());

	theNFDBMgr.SelectNFProductPackage(theNFDataItemMgr.GetPackage());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Package Cnt :", theNFDataItemMgr.GetPackage().size());

#ifndef _NCSNLS
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
#endif

#ifdef _NGSNLS
	theNFDBMgr.SelectNFFishMap(TRUE, theNFDataItemMgr.GetFishMap());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FishMap Cnt :", theNFDataItemMgr.GetFishMap().size());

	theNFDBMgr.SelectNFFishingPoint(TRUE, theNFDataItemMgr.GetFishMap());

	theNFDBMgr.SelectNFFishInfo(TRUE, theNFDataItemMgr.GetFishInfo());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FishInfo :", theNFDataItemMgr.GetFishInfo().size());

	theNFDBMgr.SelectNFFishSkill(TRUE, theNFDataItemMgr.GetFishSkill());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FishSkill :", theNFDataItemMgr.GetFishSkill().size());

	theNFDBMgr.SelectNFFishSkillCode(TRUE, theNFDataItemMgr.GetFishSkillCode());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FishSkillCode :", theNFDataItemMgr.GetFishSkillCode().size());

	theNFDBMgr.SelectNFFishingCode(TRUE, theNFDataItemMgr.GetFPFICode());
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF FPFICode :", theNFDataItemMgr.GetFPFICode().size());

	LONG lCnt = 0;
	theNFDBMgr.SelectNFSignCode(TRUE, theNFDataItemMgr.GetFishMap(), lCnt);
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF Sign&SpecialFish :", lCnt);

	theNFDBMgr.SelectNFDropItem(TRUE, theNFDataItemMgr.GetDropItemRate(), lCnt);
	theLog.Put(INF_UK, "CNFDataItemMgr"_COMMA, "NF DropItemRate :", lCnt);
#endif

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

	m_mapEquipItem[pItem->m_lItemSRL] = pItem;

	return bRet;
}

BOOL CNFDataItemMgr::AddClothesItem(ClothesItem* pItem)
{
	BOOL bRet = TRUE;

	m_mapClothesItem[pItem->m_lItemSRL] = pItem;

	return bRet;
}

BOOL CNFDataItemMgr::AddUsableItem(UsableItem* pItem)
{
	BOOL bRet = TRUE;

	m_mapUsableItem[pItem->m_lItemSRL] = pItem;

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

void CNFDataItemMgr::AddAblitity(NFUser& NFUI, LONG lPartIndex, LONG lItemSRL)
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
			if (!CalcItemAbility(NFUI, lItemSRL, lPartIndex))
				theLog.Put(WAR_UK, "GLS_Null"_COMMA, "AddEquipItemAbility is Fail!!!, Char USN: ", NFUI.m_nfCharInfo.m_nfCharBaseInfo.m_lNFCSN, ", PartsIndex : ", lItemSRL);
		}
		break;
	default:
		break;
	}
}

long CNFDataItemMgr::AddApplyUseItem(NFCharAbilityExt& NFUI, Inven& useInven)
{
	const UsableItem* pUsableItem = NULL;
	const SkillItem* pSkillItem = NULL;

	// Inven에서 가져온 Item정보의 Type(Usable인지, Skill인지를 알아낸다.)
	if (useInven.m_ItemSRL >= eItemType_UsableItem * 10000 && useInven.m_ItemSRL < eItemType_Fish * 10000)			// Usable
	{
		pUsableItem = theNFDataItemMgr.GetUsableItemByIndex(useInven.m_ItemSRL);

		// 읽어온 아이템정보를 능력치에 더한다.
		theNFDataItemMgr.CalcUsableItemAbility(NFUI, pUsableItem);		
	}
	else if (useInven.m_ItemSRL >= eItemType_SkillItem * 10000 && useInven.m_ItemSRL < eItemType_EventItem * 10000)	// Skill
	{
		pSkillItem = theNFDataItemMgr.GetSkillItemByIndex(useInven.m_ItemSRL);

		// 읽어온 아이템정보를 능력치에 더한다.
		theNFDataItemMgr.CalcSkillItemAbility(NFUI, pSkillItem);
	}
	else
		return -2;					// 사용한 아이템이 Usable이나 Skill이 아니다.

	return 1;
}

// 1. bIsOnlyAdd = TRUE, Add만 bIsOnlyAdd = FALSE, Remove + Add
// 2. Debuff를 위한 환경속성의 값을 여기서 체크 한다.
BOOL CNFDataItemMgr::CalcItemAbility(NFUser& NFUI, long lItemSRL, long lPartIndex, BOOL bIsAdd)
{
	if (lItemSRL <= 0)
		return FALSE;

	NFCharInfo& NFCharInfo = NFUI.m_nfCharInfo;

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
			ClothesItem* pItem = theNFDataItemMgr.GetClothesItemByIndex(lItemSRL);
			if (pItem)
				CalcAblilty(NFCharInfo.m_nfCharAbility, &pItem->m_AbilityExt, bIsAdd);
			else 
				return FALSE;

			if (bIsAdd)	// 추가하는 아이템이고	
			{	
				// 환경속성에 영향을 받는 아이템이면, 해당아이템의 환경속성의 값을 저장
				if (lPartIndex == eItemType_Jack || lPartIndex == eItemType_Pant || lPartIndex == eItemType_Glov || lPartIndex == eItemType_Foot)
					NFCharInfo.m_nfCharCurInfo.m_mapCheckDebuff[lPartIndex] = pItem->m_lEnvAttribute;
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
			EquipItem* pItem = theNFDataItemMgr.GetEquipItemByIndex(lItemSRL);
			if (pItem)
				CalcAblilty(NFCharInfo.m_nfCharAbility, &pItem->m_AbilityExt, bIsAdd);
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
void CNFDataItemMgr::CalcAblilty(NFCharAbility& nfCharAbility, const NFCharAbilityExt* pItem, BOOL bIsAdd)
{
	int lSign = 1;
	if (!bIsAdd)
		lSign = lSign * -1;

 	nfCharAbility.m_dCast += (pItem->m_Ability.m_dCast*lSign);
 	if (nfCharAbility.m_dCast < 0)
 		nfCharAbility.m_dCast = 0;
 
 	nfCharAbility.m_dFlyDist += (pItem->m_Ability.m_dFlyDist*lSign);
 	if (nfCharAbility.m_dFlyDist < 0)
 		nfCharAbility.m_dFlyDist = 0;
 
 	nfCharAbility.m_dBacklash += (pItem->m_Ability.m_dBacklash*lSign);
 	if (nfCharAbility.m_dBacklash < 0)
 		nfCharAbility.m_dBacklash = 0;
 
 	nfCharAbility.m_dAction += (pItem->m_Ability.m_dAction*lSign);
 	if (nfCharAbility.m_dAction < 0)
 		nfCharAbility.m_dAction = 0;
 
 	nfCharAbility.m_dHooking += (pItem->m_Ability.m_dHooking*lSign);
 	if (nfCharAbility.m_dHooking < 0)
 		nfCharAbility.m_dHooking = 0;
 
 	nfCharAbility.m_dRodPower += (pItem->m_Ability.m_dRodPower*lSign);
 	if (nfCharAbility.m_dRodPower < 0)
 		nfCharAbility.m_dRodPower = 0;
 
 	nfCharAbility.m_dReelPower += (pItem->m_Ability.m_dReelPower*lSign);
 	if (nfCharAbility.m_dReelPower < 0)
 		nfCharAbility.m_dReelPower = 0;
 
 	nfCharAbility.m_dLanding += (pItem->m_Ability.m_dLanding*lSign);
 	if (nfCharAbility.m_dLanding < 0)
 		nfCharAbility.m_dLanding = 0;
 
 	nfCharAbility.m_dFishPoint += (pItem->m_Ability.m_dFishPoint*lSign);
 	if (nfCharAbility.m_dFishPoint < 0)
 		nfCharAbility.m_dFishPoint = 0;
 
 	nfCharAbility.m_dLuckyPoint += (pItem->m_Ability.m_dLuckyPoint*lSign);
 	if (nfCharAbility.m_dLuckyPoint < 0)
 		nfCharAbility.m_dLuckyPoint = 0;
 
 	nfCharAbility.m_dFatigue -= (pItem->m_Ability.m_dFatigue*lSign);
 	if (nfCharAbility.m_dFatigue < 0)
 		nfCharAbility.m_dFatigue = 0;
}

void CNFDataItemMgr::CalcUsableItemAbility(NFCharAbilityExt& NFUI, const UsableItem* pItem, BOOL bIsAdd)
{
	int lSign = 1;
	if (!bIsAdd)
		lSign = lSign * -1;

	CalcAblilty(NFUI.m_Ability, &pItem->m_AbilityExt, bIsAdd);
}

void CNFDataItemMgr::CalcSkillItemAbility(NFCharAbilityExt& NFUI, const SkillItem* pItem, BOOL bIsAdd)
{
	int lSign = 1;
	if (!bIsAdd)
		lSign = lSign * -1;

	CalcAblilty(NFUI.m_Ability, &pItem->m_AbilityExt, bIsAdd);

 	NFUI.m_dCastingScore += (pItem->m_AbilityExt.m_dCastingScore*lSign);
 	if (NFUI.m_dCastingScore < 0)
 		NFUI.m_dCastingScore = 0;
 
 	NFUI.m_dCastingBacklashRate += (pItem->m_AbilityExt.m_dCastingBacklashRate*lSign);
 	if (NFUI.m_dCastingBacklashRate < 0)
 		NFUI.m_dCastingBacklashRate = 0;
 
 	NFUI.m_lActionIncountScore += (pItem->m_AbilityExt.m_lActionIncountScore*lSign);
 	if (NFUI.m_lActionIncountScore < 0)
 		NFUI.m_lActionIncountScore = 0;
 
 	NFUI.m_dFishFlag += (pItem->m_AbilityExt.m_dFishFlag*lSign);
 	if (NFUI.m_dFishFlag < 0)
 		NFUI.m_dFishFlag = 0;
 
 	NFUI.m_dLuckyFlag += (pItem->m_AbilityExt.m_dLuckyFlag*lSign);
 	if (NFUI.m_dLuckyFlag < 0)
 		NFUI.m_dLuckyFlag = 0;
 
 	NFUI.m_lReduceRunTime += (pItem->m_AbilityExt.m_lReduceRunTime*lSign);
 	if (NFUI.m_lReduceRunTime < 0)
 		NFUI.m_lReduceRunTime = 0;
 
 	NFUI.m_dSpecialFishBite += (pItem->m_AbilityExt.m_dSpecialFishBite*lSign);
 	if (NFUI.m_dSpecialFishBite < 0)
 		NFUI.m_dSpecialFishBite = 0;
 
 	NFUI.m_dKeepLuckyFlag += (pItem->m_AbilityExt.m_dKeepLuckyFlag*lSign);
 	if (NFUI.m_dKeepLuckyFlag < 0)
 		NFUI.m_dKeepLuckyFlag = 0;
 
 	NFUI.m_dAddRateExp += (pItem->m_AbilityExt.m_dAddRateExp*lSign);
 	if (NFUI.m_dAddRateExp < 0)
 		NFUI.m_dAddRateExp = 0;
 
 	NFUI.m_dAddRateGameMoney += (pItem->m_AbilityExt.m_dAddRateGameMoney*lSign);
 	if (NFUI.m_dAddRateGameMoney < 0)
 		NFUI.m_dAddRateGameMoney = 0;
}

LONG CNFDataItemMgr::RemoveItemAbility(NFUser& NFUI, long lOldPartsTempSRL, long lNewPartsTempSRL, Inven& OldInven, Inven& NewInven)
{
	LONG lPartIndex = 0;
	NFCharInfo& NFCharInfo = NFUI.m_nfCharInfo;

	// Naked 때문에, Old든 New든 -1이 아닌경우에는 해당 PartIndex를 얻어와야 한다.
	if (lOldPartsTempSRL != -1 && lOldPartsTempSRL != 0) {
		TMapInven::iterator iterOld = NFCharInfo.m_nfCharInven.m_mapInven.find(lOldPartsTempSRL);
		if (iterOld != NFCharInfo.m_nfCharInven.m_mapInven.end())
			OldInven = (*iterOld).second;
		lPartIndex = OldInven.m_lPartsIndex;

		OldInven.m_bIsUsing = FALSE;
	}
	else
		OldInven.m_lTempSRL = lOldPartsTempSRL;

	if (lNewPartsTempSRL != -1 && lNewPartsTempSRL != 0) {
		TMapInven::iterator iterOld = NFCharInfo.m_nfCharInven.m_mapInven.find(lNewPartsTempSRL);
		if (iterOld != NFCharInfo.m_nfCharInven.m_mapInven.end())
			NewInven = (*iterOld).second;
		lPartIndex = NewInven.m_lPartsIndex;

		ItemCommon* itemCommon = NULL;
		// 새로 장착 하는 아이템에서 레벨 체크
		if (NewInven.m_lPartsIndex >= eItemType_Lure && NewInven.m_lPartsIndex < eItemType_UsableItem)
			itemCommon = theNFDataItemMgr.GetEquipItemByIndex(NewInven.m_ItemSRL);
		else if (NewInven.m_lPartsIndex >= eItemType_Acce && NewInven.m_lPartsIndex < eItemType_Lure)
			itemCommon = theNFDataItemMgr.GetClothesItemByIndex(NewInven.m_ItemSRL);
		else
			return -101;			// ItemCategory Type Error

		if (!itemCommon)
			return -102;			// item notfound

		if (itemCommon->m_lLv > NFUI.m_nfCharInfo.m_nfCharBaseInfo.m_lLevel)
			return -103;			// 아이템이 캐릭터 레벨보다 높다... 실패

		// 새로운 능력치 추가
		NewInven.m_bIsUsing = TRUE;
	}
	else
		NewInven.m_lTempSRL = lNewPartsTempSRL;

	return 1;
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