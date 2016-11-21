
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
// ================== ����� ���� ================== //
//�� ĳ���Ͱ� ���ø� �ϰ� �ִ� �� ID�� ����Ʈ ID�� ����
//�� ����Ʈ ID���� ���� �� �ִ� ����� LIST�� �ҷ��´�.
//�� ������� ������ �߿� ���ɰ� ����� ������ ���Ѵ�. ���(��)�� ������ ������� ���ɰ� ���� ������ ����� LIST���� �����Ѵ�.
//�� ĳ������ �ɷ�ġ�� ���� �� ���� �������� LIST���� �����Ѵ�.( ���� �� ������ LIST�� �߰���)
//�� ���� �� �ִ� ����� LIST���� ĳ���� �Ǵ� �� ������ Ư�� ������� ���� Ȯ���� ������Ű�� ����Ⱑ �ִ��� �Ǵ��Ѵ�.
//�� ũ�� ���� ����� ���� �ٸ� ��������� Ȯ���� �����ȴ�.
//	i) char_Fishpoint >= Fish_Point �� ���
//		Fish_Score = Fish_Point / char_Fishpoint 
//	ii) char_Fishpoint < Fish_Point �� ���
//		Fish_Score = SQRT( char_Fishpoint ) / Fish_Point
//	iii) ����� LIST�� Fish_Score�� ������ �� Fish_Score�� ����� �� ������� Ȯ���� ���Ѵ�. 
//������ ���� ����� ������ ���� ��������.
//�� ����� LIST�� Ư�� ����⸦ ���� Ȯ���� �÷��ִ� '�ǽ� �÷���'�� �ش� �Ǵ��� üũ�ϰ�, �ش� ������� Ȯ������ Ȯ���� �����ش�.
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
	// 1. FishingPoint���� FishInfo ��������(�Ϲ� ����� + LockedSocre + BigFish ����⵵ ����)
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

				// ���� ����� �߰�
				mapChoiceFish[pFishInfo->m_lIndex] = fish;
			}	
		}
		else
			theLog.Put(ERR_UK, "NGS_LOGIC, GetFishingPointFishList(), Not Found FishInfo Index:", (*it));
	}

	// 2. BigFish ����
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

		// ����Ʈ����Ʈ or ���̺극�̼��� �����̾���(��� ���� �ɴ´�...)
		// ����� ������ ����� ������ Min�� Max���� �߰��� �־�� �Ѵ�!!!
		if (equipLureItem.m_dStartDepth == 0 || equipLureItem.m_dStartDepth >= fishInfo.m_lMinDepth)
		{
			// ������� �ɷ�ġ ���� ĳ������ �ɷ�ġ�� ������ ����
			if (CheckFishAbility(ability, fishInfo))
			{
				// 1. Fish_Score�� ���Ѵ�. (�Ҽ��� ���ڸ�����)
				double dFishScore = 0;
				if (ability.m_dFishPoint >= fishInfo.m_lFishPoint)
					dFishScore = ROUND( (fishInfo.m_lFishPoint/ability.m_dFishPoint), 3);
				else
					dFishScore = ROUND( (sqrt((double)ability.m_dFishPoint) / fishInfo.m_lFishPoint), 3 );

				pFish.SetFishScore(dFishScore);
				dTotalFishScore += dFishScore;

				TMapAddFishRate::iterator iterFindFishRate = equipLureItem.m_mapAddFishList.find(fishInfo.m_lIndex);
				if (iterFindFishRate != equipLureItem.m_mapAddFishList.end())
					// ���� Ȯ���� �����߾��°�?? �ϳ��� �ɸ���, ���� Ȯ���� ��ü������ �����ؾ� �Ѵ�....
					dTotalAddFishRate += (*iterFindFishRate).second;

				mapChoiceFish[fishInfo.m_lIndex] = pFish;
			}
		}
		else
			theLog.Put(WAR_UK, "NGS_LOGIC, GetFishingPointFishList(), Depth Check, FishIndex :", fishInfo.m_lIndex, ", Min/Max Depth :", fishInfo.m_lMinDepth, "/", fishInfo.m_lMaxDepth, ", LureDepth :", equipLureItem.m_dStartDepth);
	}

 	// 2. Fish_Score�� �� ��������, ���õ�(���� �����) Ȯ���� ����
 	// 3. Rand�� ���� �����Ͽ�, ������ ������ ����⸦ �����Ѵ�.
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

	// 4. �ƹ��͵� ������ �� �Ȱ��, �ʿ� ���õǴ� ��ǥ ������ ��������� �Ѵ�. To Do....
	lError = -105;
	return FALSE;
}

// 2011/02/16 �߰�
// 1. �� ���� ����� ����Ȯ���� ��͹���� ����Ȯ���� ���ؼ� �����Ѵ�.
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
	TMapSpecialFishProb mapTempSpecialFishProb = mapSpecialFishProb;		// ���� �����ϱ� ���ؼ�....

	LONG lTotRand = urandom(100);
	double dSpecialFishRand = 0;
	double dTotalFishRand = 0;
	TMapSignProb		probAddEquipItem;
	BOOL bIsModify = FALSE;

	// ��� �ִ� Ȯ���� ��͹���� ���̺� ã�Ƽ� ���Ѵ�...
	ForEachCElmt(TMapAddFishRate, equipItem.m_mapAddFishList, it, ij)
	{
		LONG lFishIndex = (*it).first;
		TMapSpecialFishProb::iterator iterFindRate = mapTempSpecialFishProb.find(lFishIndex);
		if (iterFindRate != mapTempSpecialFishProb.end()) 
		{
			mapTempSpecialFishProb[lFishIndex] = (*it).second + (*iterFindRate).second;
			dTotalFishRand += (*iterFindRate).second;

			// ���� Ȯ���� �����߾��°�?? �ϳ��� �ɸ���, ���� Ȯ���� ��ü������ �����ؾ� �Ѵ�....
			bIsModify = TRUE;	
		}
	}

	// ���� Ȯ��ġ ����
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

	// NFUser(�⺻ �ɷ�ġ + EquipItem + ClothItem) + NFUserItem(UsableItem + SkillItem)
	NFAbility	totAbility;
	totAbility.Clear();
	totAbility += nfCharInfoExt.m_nfAbility;
	totAbility += abilityExt.m_nfAbility;

	// ������ ����� ĳ���� ��ų�� ���� ���, ��� ��� �������� ����⸦ ������� ������ ��ġ�� ���ؼ�.. ���⼭ �����ش�.
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

	// Ŭ���̾�Ʈ�� ��û�� Ư���� ������ ������ ����
	LONG lReqFishIndex = pUser->GetAbil(AT_REQFISH);
	pUser->SetAbil(AT_REQFISH, 0); // ���� ��ƾ ������ �ʱ�ȭ

	if (lReqFishIndex > 0)
	{
		// �ش� ����Ⱑ �ִ��� Ȯ��
		FishInfo* pReqFishInfo = theNFDataItemMgr.GetFishInfoByIndex(lReqFishIndex);
		if (pReqFishInfo)
		{
			BiteFish.SetFishInfo(*pReqFishInfo);

			// ���ھ� ���� ���
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
	else	// �Ϲ�������...
	{
		// ������ ��� ������ ����
		EquipItem* pItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
		if (!pItem)
		{
			theLog.Put(WAR_UK, "NGS_LOGIC, GetEquipItem(). Error CSN : ", pUser->GetCSN(), ", ItemIndex: ", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
			return -1;
		}

		if (lSignType > 0)		// ¡�� �� ���....
		{
			// =============== ��� ����� ���� ================ //
			// 0) ��� �� �����ؾ߸�, ��� ����⸦ ���� Ȯ���� �ø� �� �ִ�...(���????)
			// 1) �߻��� ¡���� ��� ����� ����Ʈ�� ���´�.
			// 2) Ȯ������ ��� ����⸦ ������ �� �Ѵٸ�, �Ϲ� ����⸦ �򵵷� �Ѵ�.

			bIsSpecialFish = GetSpecialFishList(roomOption.m_lIdxFishMap, lSignType, BiteFish, *pItem);

			if (bIsSpecialFish)
			{
				FishInfo& fishInfo = BiteFish.GetFish();
				// Fish_Score�� ���Ѵ�. (�Ҽ��� ���ڸ�����)
				double dFishScore = 0;
				if (totAbility.m_dFishPoint >= fishInfo.m_lFishPoint)
					dFishScore = ROUND( (fishInfo.m_lFishPoint/totAbility.m_dFishPoint), 3 );
				else
					dFishScore = ROUND( (sqrt((double)totAbility.m_dFishPoint) / fishInfo.m_lFishPoint), 3 );

				BiteFish.SetFishScore(dFishScore);
			}
		}


		// ¡��������, ¡�� ����Ⱑ ���� ���� ���� ���...
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

	// ================== ����� ���� ================== //
	//����� ũ�⸦ �����ϴ� ������ ������ ����.
	//�� ����� DB���� ������� �⺻ ������ �ҷ� �´�.
	//�� ĳ������ �׼� ��ġ(char_Action), �ǽ� ����Ʈ ��ġ(char_Fishpoint), ��Ű ����Ʈ(char_Luckypoint)�� ������� �ǽ� ����Ʈ ��ġ(Fish_Point)�� �ҷ��´�.
	//�� ĳ���� �ǽ� ����Ʈ ��ġ���� ������� �ǽ� ����Ʈ ��ġ�� ����. -���� ������ 0���� ���� �ϵ��� �Ѵ�.
	//result_Fishpoint = char_Fishpoint - Fish_Point ( result_Fishpoint < 0 �̸� result_Fishpoint == 0 )
	//note) �ǽ� ����Ʈ�� ���ο� ������� ���忡 ���� �Ǳ⵵ �Ѵ�. ���� �������� ������ ����Ⱑ �������� ���� ũ�Ⱑ Ŀ���� �ȴ�.
	//�� ������� �ּ� ũ�� ��(Min_Scale)�� ���Ѵ�. ( �Ҽ��� ���ڸ� )
	//Min_Scale = ( SQRT ( char_Action )+ SQRT(  result_Fishpoint ) ) * 4 - 43 ( Min_Scale > 0 �̸� Min_Scale == 0 )
	//note) ����� ���ִ� ������ ���� Ư�� ������ ��ġ������ �ּҰ��� ������ ���� �ʰ� �ϱ� ���ؼ��̴�.
	//�� ������� �ִ� ũ�� ��(Max_Scale)�� ���Ѵ�. ( �Ҽ��� ���ڸ� )
	//Max_Scale = ( SQRT ( char_Luckypoint ) + SQRT ( result_Fishpoint ) ) * 8.2 - 75 ( Max_Scale < 0 �̸� Max_Scale == 0 )
	//note) ���� ����� ���ִ� ������ ���� Ư�� ������ ��ġ���� �ִ밪�� ������ ���� �ʰ� �ϱ� ���ؼ�
	//�� �ּҰ�, �ִ밪�� �������� ������ ��ġ( result_Scale )�� �����Ѵ�. (�Ҽ��� ���ڸ�, �� Min_scale > Max_Scale �̸� result_Scale == Min_Scale )
	//note) �ּҰ��� �ִ밪 ���� ū ��찡 �߻��Ѵ�. �� ��� �ּҰ��� ��ġ�� ������� ũ�� ������ �����ϵ��� �Ѵ�.
	//�� ����� ũ�⿡ ������ ������ �����ش�. ( ���� CM, �Ҽ��� ���ڸ� )
	//Final_Height = Fish_Height + ( Fish_Height * result_Scale )
	//�� ���� ũ��( Final_Height )�� ����� DB�� �ִ� ũ�⸦ �Ѵ��� üũ �Ѵ�. ���� ���� ũ�Ⱑ DB�� �ִ� ũ�⺸�� ũ�� DB�� �ִ� ũ�⸦ ����� ũ�⿡ �ݿ��ϵ��� �Ѵ�.
	//note) �׻� ���� ũ�Ⱑ ���� ������ ����� �� �����Ƿ� ������ ���ڸ� ������ �����ϰ� ����� �� �ֵ��� ����.

	long lResultFishPoint = (long)(totAbility.m_dFishPoint - BiteFishInfo.m_lFishPoint);
	if (lResultFishPoint < 0)
		lResultFishPoint = 0;

	// �ּ� ũ�� ��(�Ҽ��� 2�ڸ�, %)
	double dMinSize = ((sqrt( (double)totAbility.m_dCharm ) + sqrt( (double)lResultFishPoint )) * 4 - 43)/100;
	if (dMinSize < 0)
		dMinSize = 0;
	else
		dMinSize = ROUND(dMinSize, 1);

	// �ִ� ũ�� ��(�Ҽ��� 2�ڸ�, %)
	double dMaxSize = ((sqrt( (double)totAbility.m_dLuckyPoint ) + sqrt( (double)lResultFishPoint )) * 8.2 - 75)/100;
	if (dMaxSize < 0)
		dMaxSize = 0;
	else
		dMaxSize = ROUND(dMaxSize, 1);

	double dResultSize = 0.0;
	if (dMinSize >= dMaxSize)
		// �ּҰ��� �ִ밪���� ũ�ų� ������ ������ �ּҰ�
		dResultSize = dMinSize;
	else
	{
		// �ּҰ����� �ִ밪 ������ ���� ���� ���Ѵ�.
		double dSizeGap = dMaxSize - dMinSize;
		dSizeGap *= 10;

		long lResultSize = (long)dSizeGap;
		if (lResultSize <= 0)
			lResultSize = 1;
		dResultSize = (((double)(urandom(lResultSize)) * 1/10) + dMinSize)/100;
	}

	// ����� ������ ���, ������ ����� �⺻ max size���� ũ�� max size�� �����Ѵ�.
	double dResultFishSize = BiteFishInfo.m_lFish_MinSize + (BiteFishInfo.m_lFish_MinSize * dResultSize);
	if (dResultFishSize > BiteFishInfo.m_lFish_MaxSize)
		dResultFishSize = BiteFishInfo.m_lFish_MaxSize;

	// �����ϰ� ������ ���� �ϴ� �κ�
	double dAddFishSize = 0;
	double dTemp = 0;
	dAddFishSize = urandom(10);
	dTemp = (double)urandom(10);
	dAddFishSize += dTemp * 0.01;
	dResultFishSize += dAddFishSize;

	BiteFish.SetFishResultSize(dResultFishSize);

	// ================== ����� ���� ================== //
	//�� ����� DB�� �⺻ ���Կ� 'ũ�� ����'(Final_Height )�� �����ش�.
	//Scale_Weight = Fish_Weight + ( Fish_Weight * result_Scale )
	//note) ũ���� ������ ������ ������ ������ų ��� �����ų� ���ſ� ��� ������ �߻��� ���� �ִ�. �׽�Ʈ �Ŀ� ����, �����ϵ��� �Ѵ�.
	//�� ĳ������ ��Ű ����Ʈ ��ġ(char_Luckyoint)�� �߰� ���� ������ ���Ѵ�. 
	//Add_Weight = ( SQRT( char_Luckyoint ) * 5.5 ) - 17.0 ( ��, Add_Weight < 0, Add_Weight == 0 )
	//�� Scale_Weight�� �߰� ���� ������ ���Ѵ�.
	//Final_Weight = Scale_Weight + ( Scale_Weight * Add_Weight )
	//note) Scale_Weight�� Ŭ ��� ���԰� ���󺸴� ���ſ����� ������ �߻��ϱ⵵ �Ѵ�. �׽�Ʈ �Ŀ� ����, �����ϵ��� �Ѵ�.
	//�� ���� ����( Final_Weight )�� DB���� �ִ� ���Ը� �Ѵ��� üũ �Ѵ�. ���� ���� ���԰� DB�� �ִ� ���Ժ��� ũ��
	//DB�� �ִ� ���Ը� ����� ���Կ� �ݿ��ϵ��� �Ѵ�.
	//note) �׻� ���� ���԰� ���� ������ ����� �� �����Ƿ� ������ ���ڸ� ������ �����ϰ� ��� �� �� �ֵ��� ����.

	// ����� ���� ���
	double dResultFishWeight = BiteFishInfo.m_lFish_MinWeight + (BiteFishInfo.m_lFish_MinWeight * dResultSize);

	double dAddFishWeight = ((sqrt( (double)totAbility.m_dLuckyPoint ) * 5.5) - 17.0) / 100;
	if (dAddFishWeight < 0)
		dAddFishWeight = 0;
	dResultFishWeight = dResultFishWeight + (dResultFishWeight * dAddFishWeight);

	if (dResultFishWeight > BiteFishInfo.m_lFish_MaxWeight)
		dResultFishWeight = BiteFishInfo.m_lFish_MaxWeight;


	// �����ϰ� ũ�� ���� �ϴ� �κ�
	double dAddFishWeightRand = 0;
	dAddFishWeightRand = urandom(10);
	dTemp = (double)urandom(10);
	dAddFishWeightRand += dTemp * 0.1;
	dResultFishWeight += dAddFishWeightRand;

	BiteFish.SetFishResultWeight(dResultFishWeight);


	// ================== ����� ����ġ ================== //
	//Final_Exp = Fish_Exp + ( Fish_Exp * Final_Height ) + ( Fish_Exp * Add_Weight ), �ڿ��� + Item���� �߰��Ǵ� ����� ����ġ����
	double dResultExp = BiteFishInfo.m_lFishExp + ( BiteFishInfo.m_lFishExp * dResultSize ) + ( BiteFishInfo.m_lFishExp * dAddFishWeight ) + abilityExt.m_dAddRateExp;
	long lResultExp  = min((long)ROUND(dResultExp, 0), theNFDataItemMgr.GetCharExpByLevel(nfChar.GetLevel()));
	BiteFish.SetFishResultExp(lResultExp);


	// ================== ���� Ȯ�� ================== //
	//���� Ȯ�� = SQRT ( �׼� + ( ��Ű����Ʈ / 2 ) ) * 4.5 + 15 + ��� �ڵ�ĸ( �ʱ� 25%, �ϼ� 15% )
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
	long lRandBiteRate = urandom(1000);		// �Ҽ��� ���ڸ��� ���� �����Ƿ�, 100% Ȯ���� �ƴ϶� 1000% Ȯ���� ���
	if (lResultBiteRate > lRandBiteRate)	// Ȯ�� ���� �ȿ� ���� ���̹Ƿ� Ȯ�� ����
	{
		bIsBite = TRUE;
	}
	BiteFish.SetFishIsBite(bIsBite);

	// ����� MAX HP ����
	BiteFish.SetFishHP(BiteFishInfo.m_lHitPoint);

	// ================== Reward GameMoney ================== //
	LONGLONG llRewardGameMoney = BiteFishInfo.m_llMoney + (long)(BiteFishInfo.m_llMoney*dResultSize) + (long)(BiteFishInfo.m_llMoney*dAddFishWeight);
	BiteFish.SetRewardGameMoney(llRewardGameMoney);

	// ================== Reward FishScore ================== //
	// Final_Score = Fish_Exp * 0.5 + Fish+Money * 0.4 , �Ҽ��� ����
	long lRewardFishScore = (long)(lResultExp * 0.5 + llRewardGameMoney * 0.4);
	BiteFish.SetResultFishScore(lRewardFishScore);

	// ================== Feeding Time ================== //
	// �� �ǵ� Ÿ��( �Ҽ��� ���ڸ�, �� ) - millisecond
	// ����� DB�� �ǵ� Ÿ�� �״�� �ݿ�

	// FigthingFishInfo	
	biteFishInfo.m_lFishIndex = BiteFishInfo.m_lIndex;
	biteFishInfo.m_dFishLength = dResultFishSize;
	biteFishInfo.m_dFishWeight = dResultFishWeight;
	biteFishInfo.m_dFishFeedingTime = BiteFishInfo.m_lFeedTime;
	biteFishInfo.m_bIsFishBite = bIsBite;
	biteFishInfo.m_lFishScore = BiteFish.GetResultFishScore();
	biteFishInfo.m_llMoney = BiteFish.GetRewardGameMoney();


	// ���÷� �޾ƿԱ� ������, pUser�� �ٽ� �����ؾ� ��
	BiteFish.SetBiteFishInfo(biteFishInfo);
	nfChar.SetBiteFishInfo(BiteFish);

	return lErrorCode;
}


// Pattern Type Define
// 0: Attack
// 1: Rest
// 2: Skill
// 2011/4/13 : Ŭ���̾�Ʈ�� �����ִ� Rate, Time ���� ������ �Ŀ� �������� �����ϴ� ������� ����
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

	// 2011.4.21 ����
	nfChar.SetCurrentFishDepth(LBI.m_lFishDepth);
	theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo(), ##### Client FishDepth : ", LBI.m_lFishDepth, "// Server Depth :", nfChar.GetCurrentFishDepth()); 
	nfChar.SetDistanceFromFish(LBI.m_dDistance);
	nfChar.SetLineLength(LBI.m_dLineLength);

	//////////////////////////////////////////////////////////////////////////
	if (nfChar.GetActionStatus() == STATUS_HOOKING)
	{
		// CurAttackRate�� CurRestRate�� ���� �����Ѵ�.
		nfChar.SetCurrentMAXFishPattern(EN_FISHHP_INIT, fishInfo.m_lAttackRate, fishInfo.m_lRestRate);

		// ���� ó�� ���� �ϴ� ��
		nfChar.SetCurrentMAXPattern(FS_ATTACK, fishInfo.m_lAttackRate);

		// ���� ����
		biteFish.SetPrevFishDirection(GetAttackDirection(nfChar));

		nfChar.SetActionStatus(STATUS_FIGHTING);
	}

	// RestTime�� 0�� ���� ��ų ���� �ð��� ������ �ʴ´�... 
	if (msg.m_lPatternRate > 0)
		biteFish.DecrementFishSkillUseTime();

	nfChar.GetCurrentMAXPattern(lCurrentPatternType, lCurrentMAXPatternTime);

	if (nfChar.IsPatternComplete() &&  !msg.m_bIsFeverMode)		// ������ �����ų�.. ��������.. Fever�� �ƴϸ�~~
	{
		biteFish.SetFishSkill(fishSkillInfo);	// �ʱ�ȭ

		if (FS_REST == lCurrentPatternType)		// RestRate or FeverMode
		{
			LONG lGetSkillErrorCode = NF::EC_FR_CONTINUE;
			// FishSkill üũ
			if (GetFishSkillInfo(biteFish, nfChar.GetCurrentFishDepth(), choiceFishSkill, lstFishSkill, nfChar, lGetSkillErrorCode))
			{
				nfChar.SetCurrentMAXPattern(FS_SKILL, choiceFishSkill.m_lCastTime);

				// ����
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
				// ����� ������ ������ ��� �� ��...
				if (lGetSkillErrorCode != NF::EC_FR_CONTINUE)	theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo SkillFailed ErrorCode : ", lGetSkillErrorCode);

				//// FishSkill Data�� ���г� �����Ƿ� Critial Error!!!  
				//if (lGetSkillErrorCode == NF::EC_FR_FAIL_CHOICEFISHSKILL) {
				//	theLog.Put(ERR_UK, "NGS_LOGIC, GetFishAIInfo SkillFailed EC_FR_FAIL_CHOICEFISHSKILL, CSN : ", pUser->GetCSN());
				//	lErrorCode = lGetSkillErrorCode;		// �ܺη� ������ �����Ѵ�...
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

		// ���� ������ �ٽ� �ҷ��´�...
		nfChar.GetCurrentMAXPattern(lCurrentPatternType, lCurrentMAXPatternTime);
	}
	else
	{
		if (lCurrentPatternType == FS_SKILL)
			biteFish.GetFishSkill(fishSkillInfo);
	}

	// ������ DB�κ��� �о�鿩�� ������ ���� Ŭ���̾�Ʈ�� ���� MAX_Time���� �ٸ��ٸ�, Client ������ ����...
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
		lErrorCode = NF::EC_FR_NOT_SAME_PRNG_VALUE;	// Server �� Client�� PRNG���� Ʋ����... �ܺη� ������ ����...
		bRet = FALSE;
	}

	theLog.Put(DEV_UK, "NGS_LOGIC, GetFishAIInfo(), ##### ",  nfChar.GetCurrentPatternType(), " RATE, CSN :", nfChar.GetCSN(), ", PRNG_Count : ", nfChar.GetPRNG_count()); 
	return bRet;
}

BOOL CFishAI::GetFishSkillInfo(CFish& fish, LONG lFishDepth, FishSkill& choiceFishSkill, TLstFishSkillInfo& lstFishSkill, CNFChar& NFChar, LONG& lErrorCode)
{
	FishInfo& biteFish = fish.GetFish();

	BOOL bRet = FALSE;

	// � ��ų�� �ߵ��� ������, Ȯ���� ���� �����Ѵ�.
	// ��ų�� �ߵ����� ���� Ȯ�� = 100% - ���Ǵ� ��ų1 + ���Ǵ� ��ų2 + .. ���Ǵ� ��ųn
	TListIndex lstFishSkillCode;
	lstFishSkillCode.clear();
	if (!GetChoiceFishSkill(biteFish.m_lSkillGroup, choiceFishSkill, NFChar)) {
		lErrorCode = NF::EC_FR_FAIL_CHOICEFISHSKILL;
		return bRet;
	}

	// �ߵ��� ��ų�� �ʿ��� ����� HP�� ������� ��쵵 ��ų ���
	if (fish.GetCurFishHP() < choiceFishSkill.m_lCostHP) {
		lErrorCode = NF::EC_FR_NOT_ENOUGH_FISHHP;
		return bRet;
	}
	else {
		// 2010-12-13 �ּ�ó��(�ӽ�)
        //fish.CalcFishHP(choiceFishSkill.m_lCostHP);
	}

	// ��ų ������ ���� �����Ѵ�.
	FishSkillInfo	fishSkillInfo;
	fishSkillInfo.Clear();
	SetFishSkillValue(fishSkillInfo, choiceFishSkill.m_lIndex, NFChar);
	lstFishSkill.push_back(fishSkillInfo);
	
	// ���� ������� ���ɰ� ��ų �ߵ� ������ ���Ѵ�. 
	// ���� ������� ���� >= ��ų �ߵ� ���� �̾�߸� �ߵ�, �� �Ǹ� FALSE
	// success condition 
	theLog.Put(WAR_UK, "NGS_LOGIC, GetFishSkillInfo(), SkillDepth :", choiceFishSkill.m_lActivityCondition, "/ FishDepth :", lFishDepth); 

	if (choiceFishSkill.m_lActivityCondition >= 0 && choiceFishSkill.m_lActivityCondition >= lFishDepth)
	{
		// ���� �ߵ���Ű���� ��ų�� ������ �ߵ� �Ǿ��ٸ�, ������ð��� �ɸ����� Ȯ��..
		// ��ų������ ���̱� ������, ��ų���� ���� ��� �ƴ��� üũ �ؾ� ��
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

// ����� ��ų�� ���� �־��ִ� �Լ�
void CFishAI::SetFishSkillValue(FishSkillInfo& fishSkillInfo, LONG lFishSkillIndex, CNFChar& NFChar)
{
	fishSkillInfo.m_lFishSkillIndex = lFishSkillIndex;

	switch(fishSkillInfo.m_lFishSkillIndex)
	{
		// ������ �ִ� ��ų����...
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