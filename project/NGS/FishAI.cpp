
#include "stdafx.h"
#include "FishAI.h"


CFishAI	theFishAI;



CFishAI::CFishAI()
{

}

CFishAI::~CFishAI()
{

}

BOOL CFishAI::CheckFishAbility(const NFAbility& ability, const FishInfo& fishInfo)
{
	if (fishInfo.m_lFishClass == FCT_NORMAL)					return TRUE;
	else if (fishInfo.m_lFishClass == FCT_CONCILIATION)			return ability.m_dFlyDist >= fishInfo.m_dFlyDist;
	else if (fishInfo.m_lFishClass == FCT_DOUBT)				return ability.m_dCharm >= fishInfo.m_dCharm;
	else if (fishInfo.m_lFishClass == FCT_ATTACK)				return ability.m_dStrength >= fishInfo.m_dStrength;
	else if (fishInfo.m_lFishClass == FCT_SPEED)				return ability.m_dAgility >= fishInfo.m_dAgility;
	else if (fishInfo.m_lFishClass == FCT_DEEPSEA)				return ability.m_dLuckyPoint >= fishInfo.m_dLuckyPoint;
	else if (fishInfo.m_lFishClass == FCT_CONCILIATION_DOUBT)	return (ability.m_dFlyDist >= fishInfo.m_dFlyDist) && (ability.m_dCharm >= fishInfo.m_dCharm);
	else if (fishInfo.m_lFishClass == FCT_CONCILIATION_ATTACK)	return (ability.m_dFlyDist >= fishInfo.m_dFlyDist) && (ability.m_dStrength >= fishInfo.m_dStrength);
	else if (fishInfo.m_lFishClass == FCT_CONCILIATION_SPEED)	return (ability.m_dFlyDist >= fishInfo.m_dFlyDist) && (ability.m_dAgility >= fishInfo.m_dAgility);
	else if (fishInfo.m_lFishClass == FCT_CONCILIATION_DEEPSEA)	return (ability.m_dFlyDist >= fishInfo.m_dFlyDist) && (ability.m_dLuckyPoint >= fishInfo.m_dLuckyPoint);
	else if (fishInfo.m_lFishClass == FCT_DOUBT_ATTACK)			return (ability.m_dCharm >= fishInfo.m_dCharm) && (ability.m_dStrength >= fishInfo.m_dStrength);
	else if (fishInfo.m_lFishClass == FCT_DOUBT_SPEED)			return (ability.m_dCharm >= fishInfo.m_dCharm) && (ability.m_dAgility >= fishInfo.m_dAgility);
	else if (fishInfo.m_lFishClass == FCT_DOUBT_DEEPSEA)		return (ability.m_dCharm >= fishInfo.m_dCharm) && (ability.m_dLuckyPoint >= fishInfo.m_dLuckyPoint);
	else if (fishInfo.m_lFishClass == FCT_ATTACK_SPEED)			return (ability.m_dStrength >= fishInfo.m_dStrength) && (ability.m_dAgility >= fishInfo.m_dAgility);
	else if (fishInfo.m_lFishClass == FCT_ATTACK_DEEPSEA)		return (ability.m_dStrength >= fishInfo.m_dStrength) && (ability.m_dLuckyPoint >= fishInfo.m_dLuckyPoint);
	else if (fishInfo.m_lFishClass == FCT_SPEED_DEEPSEA)		return (ability.m_dAgility >= fishInfo.m_dAgility) && (ability.m_dLuckyPoint >= fishInfo.m_dLuckyPoint);

	return FALSE;
}

// 2011-10-24
// ================== 물고기 종류 ================== //
//① 캐릭터가 낚시를 하고 있는 맵 ID와 포인트 ID의 정보
//② 포인트 ID에서 잡을 수 있는 물고기 LIST를 불러온다.
//③ 물고기의 데이터 중에 수심과 루어의 수심을 비교한다. 루어(웜)의 수심이 물고기의 수심과 맞지 않으면 물고기 LIST에서 제외한다.
//④ 캐릭터의 능력치로 잡을 수 없는 물고기들을 LIST에서 제외한다.( 잡을 수 있으면 LIST에 추가함)
//⑤ 잡을 수 있는 물고기 LIST에서 캐릭터 또는 루어가 가지는 특정 물고기의 입질 확률을 증가시키는 물고기가 있는지 판단한다.
//⑥ 크기 비교의 결과에 따라 다른 계산방법으로 확률이 결정된다.
//	i) char_Fishpoint >= Fish_Point 의 경우
//		Fish_Score = Fish_Point / char_Fishpoint 
//	ii) char_Fishpoint < Fish_Point 의 경우
//		Fish_Score = SQRT( char_Fishpoint ) / Fish_Point
//	iii) 물고기 LIST의 Fish_Score를 합으로 각 Fish_Score를 나누어서 각 물고기의 확률을 구한다. 
//우측의 입질 물고기 결정의 예를 참조하자.
//⑥ 물고기 LIST에 특정 물고기를 잡을 확률을 올려주는 '피쉬 플래그'가 해당 되는지 체크하고, 해당 물고기의 확률값에 확률을 더해준다.
BOOL CFishAI::GetFishingPointFishList(const NFLockedNoteMap& lockedNote, const NFAbility& ability, long lIdxFishMap, long lIdxFishPoint, CFish& biteSpecialFishInfo, EquipItem& equipLureItem, LONG& lError)
{ 
	if (ability.m_dFishPoint <= 0) {
		lError = -100;
		theLog.Put(WAR_UK, "NGS_LOGIC, GetFishingPointFishList(). dCharFishPoint : ", ability.m_dFishPoint);
		return FALSE;
	}

	typedef std::map<LONG, CFish> TMapChoiceFish;
	TMapChoiceFish	mapChoiceFish;
	typedef std::map<LONG, LONG> TMapBigFish;	// <BigFishID, LockedNoteFishID>
	TMapBigFish		mapBigFish;

	const TMapIndexIndexList mapFPFICode = theNFDataItemMgr.GetFPFICode();
	TMapIndexIndexList::const_iterator iterFind = mapFPFICode.find(lIdxFishPoint);
	if (iterFind == mapFPFICode.end()) {
		theLog.Put(WAR_UK, "NGS_LOGIC, GetFishingPointFishList(). MapIndexIndexList is NULL, IdxFishPoint :", lIdxFishPoint);
		lError = -101;
		return FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	// 1. FishingPoint에서 FishInfo 가져오기(일반 물고기 + LockedSocre + BigFish 물고기도 포함)
	ForEachCElmt(TListIndex, (*iterFind).second, it, ij)
	{
		FishInfo* pFishInfo = theNFDataItemMgr.GetFishInfoByIndex((*it));
		if (pFishInfo)
		{
			if (pFishInfo->m_lBigFishID > 0)
				mapBigFish.insert(make_pair(pFishInfo->m_lBigFishID, pFishInfo->m_lLockedNoteFishID));

			if (pFishInfo->m_lLockedNoteScore > lockedNote.m_lTotLockedScore)
				continue;
			else
			{
				CFish fish;
				fish.Clear();
				fish.SetFishInfo(*pFishInfo);

				// 기존 물고기 추가
				mapChoiceFish[pFishInfo->m_lIndex] = fish;
			}	
		}
		else
			theLog.Put(ERR_UK, "NGS_LOGIC, GetFishingPointFishList(), Not Found FishInfo Index:", (*it));
	}

	// 2. BigFish 제거
	// 
	ForEachElmt(TMapBigFish, mapBigFish, it2, ij2)
	{
		LONG lLockedNoteFishID = (*it2).second;
		TMapLockedNote::const_iterator iterFind = lockedNote.m_TblLockedNote.find(lLockedNoteFishID);
		if (iterFind == lockedNote.m_TblLockedNote.end())
		{
			LONG lBigFishID = (*it2).first;
			TMapChoiceFish::iterator iterChoice = mapChoiceFish.find(lBigFishID);
			if (iterChoice != mapChoiceFish.end())
				mapChoiceFish.erase(iterChoice);
		}
	}
	
	if (mapChoiceFish.size() <= 0)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, GetFishingPointFishList(). lstFindFish size is NULL, FishMapIndex: ", lIdxFishMap, ", FishingPointIndex: ", lIdxFishPoint);
		lError = -102;
		return FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	double dTotalFishScore = 0;
	double dTotalAddFishRate = 0;
	ForEachElmt(TMapChoiceFish, mapChoiceFish, it3, ij3)
	{
		CFish& pFish = (*it3).second;
		FishInfo& fishInfo = pFish.GetFish();

		// 소프트베이트 or 바이브레이션은 수심이없다(계속 가라 앉는다...)
		// 루어의 수심은 물고기 수심의 Min과 Max값의 중간에 있어야 한다!!!
		if (equipLureItem.m_dStartDepth == 0 || equipLureItem.m_dStartDepth >= fishInfo.m_lMinDepth)
		{
			// 물고기의 능력치 보다 캐릭터의 능력치가 낮으면 제외
			if (CheckFishAbility(ability, fishInfo))
			{
				// 1. Fish_Score를 구한다. (소수점 두자리까지)
				double dFishScore = 0;
				if (ability.m_dFishPoint >= fishInfo.m_lFishPoint)
					dFishScore = ROUND( (fishInfo.m_lFishPoint/ability.m_dFishPoint), 3);
				else
					dFishScore = ROUND( (sqrt((double)ability.m_dFishPoint) / fishInfo.m_lFishPoint), 3 );

				pFish.SetFishScore(dFishScore);
				dTotalFishScore += dFishScore;

				TMapAddFishRate::iterator iterFindFishRate = equipLureItem.m_mapAddFishList.find(fishInfo.m_lIndex);
				if (iterFindFishRate != equipLureItem.m_mapAddFishList.end())
					// 입질 확률을 보정했었는가?? 하나라도 걸리면, 입질 확률을 전체적으로 보정해야 한다....
					dTotalAddFishRate += (*iterFindFishRate).second;

				mapChoiceFish[fishInfo.m_lIndex] = pFish;
			}
		}
		else
			theLog.Put(WAR_UK, "NGS_LOGIC, GetFishingPointFishList(), Depth Check, FishIndex :", fishInfo.m_lIndex, ", Min/Max Depth :", fishInfo.m_lMinDepth, "/", fishInfo.m_lMaxDepth, ", LureDepth :", equipLureItem.m_dStartDepth);
	}

 	// 2. Fish_Score를 다 구했으면, 선택될(입질 물고기) 확률을 구함
 	// 3. Rand한 수를 선택하여, 범위에 들어오는 물고기를 선택한다.
 	long lBiteRate = urandom(1000);
 	long lTotBiteRate = 0;
 
 	ForEachElmt(TMapChoiceFish, mapChoiceFish, it4, ij4)
 	{
 		lTotBiteRate += (long)((ROUND((*it4).second.SetGetChoiceRate(dTotalFishScore, dTotalAddFishRate), 3)) * 1000);
 		if ( lTotBiteRate >= lBiteRate )
 		{
 			biteSpecialFishInfo = (*it4).second;
 			return TRUE;
 		}
 	}

	// 4. 아무것도 선택이 안 된경우, 맵에 선택되는 대표 물고기로 셋팅해줘야 한다. To Do....
	lError = -105;
	return FALSE;
}

// 2011/02/16 추가
// 1. 루어에 속한 물고기 입질확률을 희귀물고기 입질확률에 더해서 선택한다.
BOOL CFishAI::GetSpecialFishList(long lIdxFishMap, long lSignType, CFish& biteSpecialFishInfo, const EquipItem& equipItem)
{
	const TMapIndexFishMap mapFishMap = theNFDataItemMgr.GetFishMap();
	TMapIndexFishMap::const_iterator iterFind = mapFishMap.find(lIdxFishMap);
	if (iterFind == mapFishMap.end())
		return FALSE;
	
	const TMapSignProb mapSignProb = (*iterFind).second->m_mapSignProb;
	TMapSignProb::const_iterator iterFind2 = mapSignProb.find(lSignType);
	if (iterFind2 == mapSignProb.end())
		return FALSE;

	const TMapSpecialFishProb mapSpecialFishProb = (*iterFind2).second.m_mapSpecialFishProb;
	TMapSpecialFishProb mapTempSpecialFishProb = mapSpecialFishProb;		// 직접 수정하기 위해서....

	LONG lTotRand = urandom(100);
	double dSpecialFishRand = 0;
	double dTotalFishRand = 0;
	TMapSignProb		probAddEquipItem;
	BOOL bIsModify = FALSE;

	// 장비에 있는 확률을 희귀물고기 테이블에 찾아서 더한다...
	ForEachCElmt(TMapAddFishRate, equipItem.m_mapAddFishList, it, ij)
	{
		LONG lFishIndex = (*it).first;
		TMapSpecialFishProb::iterator iterFindRate = mapTempSpecialFishProb.find(lFishIndex);
		if (iterFindRate != mapTempSpecialFishProb.end()) 
		{
			mapTempSpecialFishProb[lFishIndex] = (*it).second + (*iterFindRate).second;
			dTotalFishRand += (*iterFindRate).second;

			// 입질 확률을 보정했었는가?? 하나라도 걸리면, 입질 확률을 전체적으로 보정해야 한다....
			bIsModify = TRUE;	
		}
	}

	// 입질 확률치 보정
	if (TRUE == bIsModify)
	{
		ForEachElmt(TMapSpecialFishProb, mapTempSpecialFishProb, it, ij)
		{
			double tempFishRate = (*it).second / (100 + dTotalFishRand);
			mapTempSpecialFishProb[(*it).first] = tempFishRate;
		}
	}

	// 
	ForEachElmt(TMapSpecialFishProb, mapTempSpecialFishProb, it, ij)
	{
		dSpecialFishRand += (*it).second;

		if (dSpecialFishRand >= (double)lTotRand)
		{
			FishInfo* pFishInfo = theNFDataItemMgr.GetFishInfoByIndex((*it).first);
			if (pFishInfo)
			{
				pFishInfo->Clear();
				biteSpecialFishInfo.SetFishInfo(*pFishInfo);
				return TRUE;
			}
			else
				return FALSE;
		}
	}
	return FALSE;
}

LONG CFishAI::GetBiteFishInfo(CUser* pUser, BiteFishInfo& biteFishInfo, const NFRoomOption& roomOption, LONG lSignType)
{
	CNFChar& nfChar = pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt = nfChar.GetNFChar().m_nfCharInfoExt;
	NFAbilityExt& abilityExt = nfChar.GetAbilityExt();
	const NFRoomOption& nfCharRoomOption = pUser->GetNFRoomOption();

	// NFUser(기본 능력치 + EquipItem + ClothItem) + NFUserItem(UsableItem + SkillItem)
	NFAbility	totAbility;
	totAbility.Clear();
	totAbility += nfCharInfoExt.m_nfAbility;
	totAbility += abilityExt.m_nfAbility;

	// 기존에 사용한 캐스팅 스킬이 있을 경우, 어떠한 어떠한 사이즈의 물고기를 잡는지에 영향을 미치기 위해서.. 여기서 더해준다.
	TLstSkill tempSkill = nfChar.GetUsedCastingSkill();
	ForEachElmt(TLstSkill, tempSkill, it, ij) 
	{ 
		SkillItem* pSkillItem = theNFDataItemMgr.GetSkillItemByIndex((*it).m_lItemCode);
		if (pSkillItem) {
			totAbility += (*pSkillItem).m_nfAbilityExt.m_nfAbility;
		}
		else
			theLog.Put(WAR_UK, "NGS_LOGIC, GetBiteFishInfo() Not Found CastingSkill!!!. CSN: ", pUser->GetCSN(), ", SkillIndex : ", (*it).m_lItemCode);
	}

	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);

	LONG lErrorCode = NF::G_NF_ERR_SUCCESS;
	BOOL bIsSpecialFish = FALSE;

	CFish& BiteFish = nfChar.GetBiteFish();
	BiteFish.Clear();
	FishInfo BiteFishInfo;
	BiteFishInfo.Clear();

	// 클라이언트가 요청한 특정한 물고기로 무조건 셋팅
	LONG lReqFishIndex = pUser->GetAbil(AT_REQFISH);
	pUser->SetAbil(AT_REQFISH, 0); // 정상 루틴 값으로 초기화

	if (lReqFishIndex > 0)
	{
		// 해당 물고기가 있는지 확인
		FishInfo* pReqFishInfo = theNFDataItemMgr.GetFishInfoByIndex(lReqFishIndex);
		if (pReqFishInfo)
		{
			BiteFish.SetFishInfo(*pReqFishInfo);

			// 스코어 따로 계산
			double dFishScore = 0;
			if (totAbility.m_dFishPoint >= pReqFishInfo->m_lFishPoint)
				dFishScore = ROUND( (pReqFishInfo->m_lFishPoint/totAbility.m_dFishPoint), 3 );
			else
				dFishScore = ROUND( (sqrt((double)totAbility.m_dFishPoint) / pReqFishInfo->m_lFishPoint), 3 );

			BiteFish.SetFishScore(dFishScore);

			theLog.Put(WAR_UK, "NGS_LOGIC, GetBiteFishInfo(). ReqFishIndex!!!! CSN : ", pUser->GetCSN(), ", ReqFishIndex: ", lReqFishIndex);
		}
		else
			return -100;
	}
	else	// 일반적으로...
	{
		// 착용한 루어 아이템 정보
		EquipItem* pItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
		if (!pItem)
		{
			theLog.Put(WAR_UK, "NGS_LOGIC, GetEquipItem(). Error CSN : ", pUser->GetCSN(), ", ItemIndex: ", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
			return -1;
		}

		if (lSignType > 0)		// 징조 일 경우....
		{
			// =============== 희귀 물고기 종류 ================ //
			// 0) 희귀 루어를 장착해야만, 희귀 물고기를 잡을 확률을 올릴 수 있다...(어떻게????)
			// 1) 발생한 징조별 희귀 물고기 리스트를 얻어온다.
			// 2) 확률에서 희귀 물고기를 얻어오지 못 한다면, 일반 물고기를 얻도록 한다.

			bIsSpecialFish = GetSpecialFishList(roomOption.m_lIdxFishMap, lSignType, BiteFish, *pItem);

			if (bIsSpecialFish)
			{
				FishInfo& fishInfo = BiteFish.GetFish();
				// Fish_Score를 구한다. (소수점 두자리까지)
				double dFishScore = 0;
				if (totAbility.m_dFishPoint >= fishInfo.m_lFishPoint)
					dFishScore = ROUND( (fishInfo.m_lFishPoint/totAbility.m_dFishPoint), 3 );
				else
					dFishScore = ROUND( (sqrt((double)totAbility.m_dFishPoint) / fishInfo.m_lFishPoint), 3 );

				BiteFish.SetFishScore(dFishScore);
			}
		}


		// 징조이지만, 징조 물고기가 선택 되지 않은 경우...
		if (FALSE == bIsSpecialFish || BiteFish.GetFish().m_lIndex <= 0)
		{
			TLstFish lstFindFish;
			long lError = 0;
			NFLockedNoteMap lockedNote;
			lockedNote.Clear();
			pUser->GetLockedNoteByMapID(roomOption.m_lIdxFishMap, lockedNote);
			
			BOOL bRet = GetFishingPointFishList(lockedNote, totAbility, roomOption.m_lIdxFishMap, nfCharRoomOption.m_lIdxFishingPoint, BiteFish, *pItem, lError);
			if (!bRet)
			{
				theLog.Put(WAR_UK, "NGS_LOGIC, GetFightingFish(). Error CSN : ", pUser->GetCSN(), ", GetFishingPointFishList ErrorCode : ", lError);
				return -2;
			}
			else
				theLog.Put(WAR_UK, "NGS_LOGIC, GetFightingFish(). !!!!! OK CSN : ", pUser->GetCSN(), ", FishMapIndex: ", nfCharRoomOption.m_lIdxFishMap, ", FishingPointIndex: ", nfCharRoomOption.m_lIdxFishingPoint);
		}
	}


	BiteFishInfo = BiteFish.GetFish();

	// ================== 물고기 길이 ================== //
	//물고기 크기를 결정하는 순서는 다음과 같다.
	//① 물고기 DB에서 물고기의 기본 정보를 불러 온다.
	//② 캐릭터의 액션 수치(char_Action), 피쉬 포인트 수치(char_Fishpoint), 럭키 포인트(char_Luckypoint)와 물고기의 피쉬 포인트 수치(Fish_Point)를 불러온다.
	//③ 캐릭터 피쉬 포인트 수치에서 물고기의 피쉬 포인트 수치를 뺀다. -값이 나오면 0으로 간주 하도록 한다.
	//result_Fishpoint = char_Fishpoint - Fish_Point ( result_Fishpoint < 0 이면 result_Fishpoint == 0 )
	//note) 피쉬 포인트는 새로운 물고기의 등장에 관련 되기도 한다. 차를 구해주지 않으면 물고기가 등장하자 마자 크기가 커지게 된다.
	//④ 물고기의 최소 크기 값(Min_Scale)을 구한다. ( 소수점 한자리 )
	//Min_Scale = ( SQRT ( char_Action )+ SQRT(  result_Fishpoint ) ) * 4 - 43 ( Min_Scale > 0 이면 Min_Scale == 0 )
	//note) 상수를 빼주는 이유는 낮은 특정 구간의 수치에서는 최소값의 적용을 받지 않게 하기 위해서이다.
	//⑤ 물고기의 최대 크기 값(Max_Scale)을 구한다. ( 소수점 한자리 )
	//Max_Scale = ( SQRT ( char_Luckypoint ) + SQRT ( result_Fishpoint ) ) * 8.2 - 75 ( Max_Scale < 0 이면 Max_Scale == 0 )
	//note) 역시 상수를 빼주는 이유는 낮은 특정 구간의 수치에서 최대값의 적용을 받지 않게 하기 위해서
	//⑥ 최소값, 최대값의 범위에서 랜덤한 수치( result_Scale )를 선택한다. (소수점 한자리, 단 Min_scale > Max_Scale 이면 result_Scale == Min_Scale )
	//note) 최소값이 최대값 보다 큰 경우가 발생한다. 이 경우 최소값의 수치를 물고기의 크기 비율로 선택하도록 한다.
	//⑦ 물고기 크기에 구해진 비율을 곱해준다. ( 단위 CM, 소수점 두자리 )
	//Final_Height = Fish_Height + ( Fish_Height * result_Scale )
	//⑧ 최종 크기( Final_Height )가 물고기 DB의 최대 크기를 넘는지 체크 한다. 만약 최종 크기가 DB의 최대 크기보다 크면 DB의 최대 크기를 물고기 크기에 반영하도록 한다.
	//note) 항상 최종 크기가 같이 나오면 어색할 수 있으므로 마지막 한자리 정도는 랜덤하게 출력할 수 있도록 하자.

	long lResultFishPoint = (long)(totAbility.m_dFishPoint - BiteFishInfo.m_lFishPoint);
	if (lResultFishPoint < 0)
		lResultFishPoint = 0;

	// 최소 크기 값(소수점 2자리, %)
	double dMinSize = ((sqrt( (double)totAbility.m_dCharm ) + sqrt( (double)lResultFishPoint )) * 4 - 43)/100;
	if (dMinSize < 0)
		dMinSize = 0;
	else
		dMinSize = ROUND(dMinSize, 1);

	// 최대 크기 값(소수점 2자리, %)
	double dMaxSize = ((sqrt( (double)totAbility.m_dLuckyPoint ) + sqrt( (double)lResultFishPoint )) * 8.2 - 75)/100;
	if (dMaxSize < 0)
		dMaxSize = 0;
	else
		dMaxSize = ROUND(dMaxSize, 1);

	double dResultSize = 0.0;
	if (dMinSize >= dMaxSize)
		// 최소값이 최대값보다 크거나 같으면 비율은 최소값
		dResultSize = dMinSize;
	else
	{
		// 최소값에서 최대값 사이의 랜덤 값을 구한다.
		double dSizeGap = dMaxSize - dMinSize;
		dSizeGap *= 10;

		long lResultSize = (long)dSizeGap;
		if (lResultSize <= 0)
			lResultSize = 1;
		dResultSize = (((double)(urandom(lResultSize)) * 1/10) + dMinSize)/100;
	}

	// 물고기 사이즈 계산, 사이즈 결과가 기본 max size보다 크면 max size로 설정한다.
	double dResultFishSize = BiteFishInfo.m_lFish_MinSize + (BiteFishInfo.m_lFish_MinSize * dResultSize);
	if (dResultFishSize > BiteFishInfo.m_lFish_MaxSize)
		dResultFishSize = BiteFishInfo.m_lFish_MaxSize;

	// 랜덤하게 사이즈 조절 하는 부분
	double dAddFishSize = 0;
	double dTemp = 0;
	dAddFishSize = urandom(10);
	dTemp = (double)urandom(10);
	dAddFishSize += dTemp * 0.01;
	dResultFishSize += dAddFishSize;

	BiteFish.SetFishResultSize(dResultFishSize);

	// ================== 물고기 무게 ================== //
	//① 물고기 DB의 기본 무게에 '크기 비율'(Final_Height )을 곱해준다.
	//Scale_Weight = Fish_Weight + ( Fish_Weight * result_Scale )
	//note) 크기의 비율로 무게의 비율을 증가시킬 경우 가볍거나 무거울 경우 오차가 발생할 수도 있다. 테스트 후에 검증, 수정하도록 한다.
	//② 캐릭터의 럭키 포인트 수치(char_Luckyoint)로 추가 무게 비율을 구한다. 
	//Add_Weight = ( SQRT( char_Luckyoint ) * 5.5 ) - 17.0 ( 단, Add_Weight < 0, Add_Weight == 0 )
	//③ Scale_Weight에 추가 무게 비율을 구한다.
	//Final_Weight = Scale_Weight + ( Scale_Weight * Add_Weight )
	//note) Scale_Weight가 클 경우 무게가 예상보다 무거워지는 현상이 발생하기도 한다. 테스트 후에 검증, 수정하도록 한다.
	//④ 최종 무게( Final_Weight )가 DB상의 최대 무게를 넘는지 체크 한다. 만약 최종 무게가 DB의 최대 무게보다 크면
	//DB의 최대 무게를 물고기 무게에 반영하도록 한다.
	//note) 항상 최종 무게가 같이 나오면 어색할 수 있으므로 마지막 한자리 정도는 랜덤하게 출력 할 수 있도록 하자.

	// 물고기 무게 계산
	double dResultFishWeight = BiteFishInfo.m_lFish_MinWeight + (BiteFishInfo.m_lFish_MinWeight * dResultSize);

	double dAddFishWeight = ((sqrt( (double)totAbility.m_dLuckyPoint ) * 5.5) - 17.0) / 100;
	if (dAddFishWeight < 0)
		dAddFishWeight = 0;
	dResultFishWeight = dResultFishWeight + (dResultFishWeight * dAddFishWeight);

	if (dResultFishWeight > BiteFishInfo.m_lFish_MaxWeight)
		dResultFishWeight = BiteFishInfo.m_lFish_MaxWeight;


	// 랜덤하게 크기 조절 하는 부분
	double dAddFishWeightRand = 0;
	dAddFishWeightRand = urandom(10);
	dTemp = (double)urandom(10);
	dAddFishWeightRand += dTemp * 0.1;
	dResultFishWeight += dAddFishWeightRand;

	BiteFish.SetFishResultWeight(dResultFishWeight);


	// ================== 물고기 경험치 ================== //
	//Final_Exp = Fish_Exp + ( Fish_Exp * Final_Height ) + ( Fish_Exp * Add_Weight ), 자연수 + Item사용시 추가되는 물고기 경험치비율
	double dResultExp = BiteFishInfo.m_lFishExp + ( BiteFishInfo.m_lFishExp * dResultSize ) + ( BiteFishInfo.m_lFishExp * dAddFishWeight ) + abilityExt.m_dAddRateExp;
	long lResultExp  = min((long)ROUND(dResultExp, 0), theNFDataItemMgr.GetCharExpByLevel(nfChar.GetLevel()));
	BiteFish.SetFishResultExp(lResultExp);


	// ================== 입질 확률 ================== //
	//입질 확률 = SQRT ( 액션 + ( 럭키포인트 / 2 ) ) * 4.5 + 15 + 계급 핸디캡( 초급 25%, 하수 15% )
	double dLevelHandicap = 0.0;

	if (nfCharInfoExt.m_nfCharBaseInfo.m_lLevel == EN_USERLEVEL_CHOBO)
		dLevelHandicap = 25;
	else if (nfCharInfoExt.m_nfCharBaseInfo.m_lLevel == EN_USERLEVEL_HASOO)
		dLevelHandicap = 15;
	else
		dLevelHandicap = 0.0;

	double dResultBiteRate = sqrt(totAbility.m_dCharm);
	double dResultBiteRateEx = ROUND(dResultBiteRate, 1);
	BiteFish.SetFishResultBiteRate(dResultBiteRateEx);


	BOOL bIsBite = FALSE;
	long lResultBiteRate = (long) dResultBiteRateEx * 10;
	long lRandBiteRate = urandom(1000);		// 소수점 한자리를 증가 했으므로, 100% 확률이 아니라 1000% 확률로 계산
	if (lResultBiteRate > lRandBiteRate)	// 확률 범위 안에 들어온 값이므로 확률 적용
	{
		bIsBite = TRUE;
	}
	BiteFish.SetFishIsBite(bIsBite);

	// 물고기 MAX HP 셋팅
	BiteFish.SetFishHP(BiteFishInfo.m_lHitPoint);

	// ================== Reward GameMoney ================== //
	LONGLONG llRewardGameMoney = BiteFishInfo.m_llMoney + (long)(BiteFishInfo.m_llMoney*dResultSize) + (long)(BiteFishInfo.m_llMoney*dAddFishWeight);
	BiteFish.SetRewardGameMoney(llRewardGameMoney);

	// ================== Reward FishScore ================== //
	// Final_Score = Fish_Exp * 0.5 + Fish+Money * 0.4 , 소수점 없음
	long lRewardFishScore = (long)(lResultExp * 0.5 + llRewardGameMoney * 0.4);
	BiteFish.SetResultFishScore(lRewardFishScore);

	// ================== Feeding Time ================== //
	// ① 피딩 타임( 소수점 두자리, 초 ) - millisecond
	// 물고기 DB의 피딩 타임 그대로 반영

	// FigthingFishInfo	
	biteFishInfo.m_lFishIndex = BiteFishInfo.m_lIndex;
	biteFishInfo.m_dFishLength = dResultFishSize;
	biteFishInfo.m_dFishWeight = dResultFishWeight;
	biteFishInfo.m_dFishFeedingTime = BiteFishInfo.m_lFeedTime;
	biteFishInfo.m_bIsFishBite = bIsBite;
	biteFishInfo.m_lFishScore = BiteFish.GetResultFishScore();
	biteFishInfo.m_llMoney = BiteFish.GetRewardGameMoney();


	// 로컬로 받아왔기 때문에, pUser에 다시 저장해야 함
	BiteFish.SetBiteFishInfo(biteFishInfo);
	nfChar.SetBiteFishInfo(BiteFish);

	return lErrorCode;
}


// Pattern Type Define
// 0: Attack
// 1: Rest
// 2: Skill
// 2011/4/13 : 클라이언트가 보내주는 Rate, Time 으로 설정한 후에 서버에서 검증하는 방식으로 변경
BOOL CFishAI::GetFishAIInfo(CUser* pUser, MsgCliNGS_ReqFighting& msg, const LineBreakInfo& LBI, LONG& lErrorCode)
{
	BOOL bRet = TRUE;
	lErrorCode = NF::EC_FR_SUCCESS;

	// Init
	CNFChar& nfChar = pUser->GetNFUser();
	CFish& biteFish = nfChar.GetBiteFish();
	FishInfo fishInfo = biteFish.GetFish();

	FishSkill		choiceFishSkill;
	choiceFishSkill.Clear();

	FishSkillInfo	fishSkillInfo;
	fishSkillInfo.Clear();

	TLstFishSkillInfo lstFishSkill;

	LONG lCurrentPatternType = 0, lCurrentMAXPatternTime = 0;

	// 2011.4.21 수정
	nfChar.SetCurrentFishDepth(LBI.m_lFishDepth);
	theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo(), ##### Client FishDepth : ", LBI.m_lFishDepth, "// Server Depth :", nfChar.GetCurrentFishDepth()); 
	nfChar.SetDistanceFromFish(LBI.m_dDistance);
	nfChar.SetLineLength(LBI.m_dLineLength);

	//////////////////////////////////////////////////////////////////////////
	if (nfChar.GetActionStatus() == STATUS_HOOKING)
	{
		// CurAttackRate과 CurRestRate의 값을 설정한다.
		nfChar.SetCurrentMAXFishPattern(EN_FISHHP_INIT, fishInfo.m_lAttackRate, fishInfo.m_lRestRate);

		// 제일 처음 셋팅 하는 값
		nfChar.SetCurrentMAXPattern(FS_ATTACK, fishInfo.m_lAttackRate);

		// 공격 방향
		biteFish.SetPrevFishDirection(GetAttackDirection(nfChar));

		nfChar.SetActionStatus(STATUS_FIGHTING);
	}

	// RestTime이 0일 경우는 스킬 재사용 시간을 줄이지 않는다... 
	if (msg.m_lPatternRate > 0)
		biteFish.DecrementFishSkillUseTime();

	nfChar.GetCurrentMAXPattern(lCurrentPatternType, lCurrentMAXPatternTime);

	if (nfChar.IsPatternComplete() &&  !msg.m_bIsFeverMode)		// 패턴이 끝났거나.. 끝났지만.. Fever가 아니면~~
	{
		biteFish.SetFishSkill(fishSkillInfo);	// 초기화

		if (FS_REST == lCurrentPatternType)		// RestRate or FeverMode
		{
			LONG lGetSkillErrorCode = NF::EC_FR_CONTINUE;
			// FishSkill 체크
			if (GetFishSkillInfo(biteFish, nfChar.GetCurrentFishDepth(), choiceFishSkill, lstFishSkill, nfChar, lGetSkillErrorCode))
			{
				nfChar.SetCurrentMAXPattern(FS_SKILL, choiceFishSkill.m_lCastTime);

				// 저장
				fishSkillInfo.m_lFishSkillIndex = choiceFishSkill.m_lIndex;

				fishSkillInfo.m_lReuseTime = choiceFishSkill.m_lReactivityTime;
				fishSkillInfo.m_lType = choiceFishSkill.m_lType;
				fishSkillInfo.m_lAddFishAttack = choiceFishSkill.m_lAddAttackPower;
				fishSkillInfo.m_lIsBullet = choiceFishSkill.m_lIsBullet;

				biteFish.SetFishSkill(fishSkillInfo);

				if (msg.m_lFishSkillID != fishSkillInfo.m_lFishSkillIndex)
					theLog.Put(DEV_UK, "NGS_LOGIC, MissMatch FishSkillID, Client : ", msg.m_lFishSkillID, " / Server : ", fishSkillInfo.m_lFishSkillIndex);
			}
			else 
			{
				// 디버깅 용으로 에러만 찍고 말 것...
				if (lGetSkillErrorCode != NF::EC_FR_CONTINUE)	theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo SkillFailed ErrorCode : ", lGetSkillErrorCode);

				//// FishSkill Data가 실패난 에러므로 Critial Error!!!  
				//if (lGetSkillErrorCode == NF::EC_FR_FAIL_CHOICEFISHSKILL) {
				//	theLog.Put(ERR_UK, "NGS_LOGIC, GetFishAIInfo SkillFailed EC_FR_FAIL_CHOICEFISHSKILL, CSN : ", pUser->GetCSN());
				//	lErrorCode = lGetSkillErrorCode;		// 외부로 에러를 리턴한다...
				//	bRet = FALSE;
				//}

				nfChar.SetCurrentMAXPattern(FS_ATTACK, fishInfo.m_lAttackRate);
				biteFish.SetPrevFishDirection(GetAttackDirection(nfChar));
			}
		}
		else 		// AttacRate, SkillRate
		{
			nfChar.SetCurrentMAXPattern(FS_REST, fishInfo.m_lRestRate);
			biteFish.SetPrevFishDirection(GetAttackDirection(nfChar));
		}

		// 변경 됐으면 다시 불러온다...
		nfChar.GetCurrentMAXPattern(lCurrentPatternType, lCurrentMAXPatternTime);
	}
	else
	{
		if (lCurrentPatternType == FS_SKILL)
			biteFish.GetFishSkill(fishSkillInfo);
	}

	// 서버가 DB로부터 읽어들여서 셋팅한 값과 클라이언트가 보낸 MAX_Time값이 다르다면, Client 값으로 셋팅...
	if (msg.m_lPatternType != nfChar.GetCurrentPatternType()) {
		theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo(), ##################... ServerType : ",  nfChar.GetCurrentPatternType(), " // Client Type : ", msg.m_lPatternType); 
		nfChar.SetCurrentPatternType(msg.m_lPatternType);
	}
	if (msg.m_lPatternRate != nfChar.GetCurrentMaxPatternTime()) {
		theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo(), ##################... ServerTime : ",  nfChar.GetCurrentMaxPatternTime(), " // ClientTime : ", msg.m_lPatternRate); 
		nfChar.SetCurrentPatternTime(msg.m_lPatternRate);
	}

	nfChar.DecrementCurrentPatternTime();
	theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo(), ******** ServerRemainTime : ",  nfChar.GetCurrentRemainTime()); 

	if (LBI.m_lPRNGValue != (ULONG)nfChar.GetPRNG_value()) {
		lErrorCode = NF::EC_FR_NOT_SAME_PRNG_VALUE;	// Server 와 Client의 PRNG값이 틀리다... 외부로 에러를 리턴...
		bRet = FALSE;
	}

	theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo(), ##### ",  nfChar.GetCurrentPatternType(), " RATE, CSN :", nfChar.GetCSN(), ", PRNG_Count : ", nfChar.GetPRNG_count()); 
	return bRet;
}

BOOL CFishAI::GetFishSkillInfo(CFish& fish, LONG lFishDepth, FishSkill& choiceFishSkill, TLstFishSkillInfo& lstFishSkill, CNFChar& NFChar, LONG& lErrorCode)
{
	FishInfo& biteFish = fish.GetFish();

	BOOL bRet = FALSE;

	// 어떤 스킬을 발동할 것인지, 확률에 따라서 선택한다.
	// 스킬이 발동되지 않을 확률 = 100% - 사용되는 스킬1 + 사용되는 스킬2 + .. 사용되는 스킬n
	TListIndex lstFishSkillCode;
	lstFishSkillCode.clear();
	if (!GetChoiceFishSkill(biteFish.m_lSkillGroup, choiceFishSkill, NFChar)) {
		lErrorCode = NF::EC_FR_FAIL_CHOICEFISHSKILL;
		return bRet;
	}

	// 발동한 스킬에 필요한 물고기 HP가 모질라는 경우도 스킬 취소
	if (fish.GetCurFishHP() < choiceFishSkill.m_lCostHP) {
		lErrorCode = NF::EC_FR_NOT_ENOUGH_FISHHP;
		return bRet;
	}
	else {
		// 2010-12-13 주석처리(임시)
        //fish.CalcFishHP(choiceFishSkill.m_lCostHP);
	}

	// 스킬 고유의 값을 셋팅한다.
	FishSkillInfo	fishSkillInfo;
	fishSkillInfo.Clear();
	SetFishSkillValue(fishSkillInfo, choiceFishSkill.m_lIndex, NFChar);
	lstFishSkill.push_back(fishSkillInfo);
	
	// 현재 물고기의 수심과 스킬 발동 수심을 비교한다. 
	// 현재 물고기의 수심 >= 스킬 발동 수심 이어야만 발동, 안 되면 FALSE
	// success condition 
	theLog.Put(WAR_UK, "NGS_LOGIC, GetFishSkillInfo(), SkillDepth :", choiceFishSkill.m_lActivityCondition, "/ FishDepth :", lFishDepth); 

	if (choiceFishSkill.m_lActivityCondition >= 0 && choiceFishSkill.m_lActivityCondition >= lFishDepth)
	{
		// 현재 발동시키려는 스킬이 기존에 발동 되었다면, 재사용대기시간에 걸리는지 확인..
		// 스킬단위의 쿨이기 때문에, 스킬마다 언제 사용 됐는지 체크 해야 함
		if (!fish.GetFishSkillReuseTime(choiceFishSkill.m_lIndex)) {
			lErrorCode = NF::EC_FR_REUSE_TIME;
			return bRet;
		}
		
		bRet = TRUE;
	}
	else {
		lErrorCode = NF::EC_FR_SKILL_CHOICE_FAIL_DEPTH;
	}

	return bRet;
}

// 물고기 스킬에 값을 넣어주는 함수
void CFishAI::SetFishSkillValue(FishSkillInfo& fishSkillInfo, LONG lFishSkillIndex, CNFChar& NFChar)
{
	fishSkillInfo.m_lFishSkillIndex = lFishSkillIndex;

	switch(fishSkillInfo.m_lFishSkillIndex)
	{
		// 방향이 있는 스킬류들...
		case 2:
		case 4:
		case 7:
			{
				fishSkillInfo.m_lEffectDirection = fishSkillInfo.m_lEffectDirection;
			}
			break;
		default:
			break;
	}
}

LONG CFishAI::GetAttackDirection(CNFChar& NFChar)
{
	return NFChar.GetPRNG() % 7;
}


void CFishAI::GetFishInfoByFishHP(FishInfo& fishInfo, LONG& lFishAttackRate, double& dFishAttackPower, double& dFishMoveSpeed)
{
	lFishAttackRate = fishInfo.m_lAttackRate;
	dFishAttackPower = fishInfo.m_dAttack;
	dFishMoveSpeed = fishInfo.m_lMoveSpeed;
}


BOOL CFishAI::GetChoiceFishSkill(LONG lFishSkillPattern, FishSkill& choiceFishSkill, CNFChar& nfChar)
{
	long lRandRate = 0, lTotalRate = 0;

	TMapIndexFishSkillCode mapFishSkillCode = theNFDataItemMgr.GetFishSkillCode();

	TMapIndexFishSkillCode::iterator iterFind = mapFishSkillCode.find(lFishSkillPattern);
	if (iterFind != mapFishSkillCode.end())
	{
		lRandRate = nfChar.GetPRNG()%100;

		FishSkillCode* pFishSkillCode = (FishSkillCode *)(*iterFind).second;
		if (pFishSkillCode)
		{
			ForEachElmt(TvecFishSkillRate, pFishSkillCode->m_vecFishSkillRate, it, ij)
			{
				lTotalRate += (*it).m_lActiveRate;
				if (lRandRate < lTotalRate)
				{
					theNFDataItemMgr.GetFishSkillByIndex((*it).m_lSkillIndex, choiceFishSkill);
					return TRUE;
				}
			}	
		}
	}
	return FALSE;
}