//
// FishAI.h
//

#ifndef FishAI_H
#define FishAI_H


#include "Common.h"
#include <NFVariant/NFGameData.h>


//////////////////////////////////////////////////////////////////////////
// FishAI에 서버만 사용되는 공식 수치 정의
const DWORD AI_CASTING_FLY_DIST_MIN = 10;
const DWORD AI_CASTING_FLY_DIST_MAX = 320;
const float AI_DRAG_CASTING_RATE = 2.3f;
const float AI_DRAG_CASTING_ADD_DIST = 10.0f;
const float AI_CLICK_CASTING_RATE = 0.8f;

const float AI_CASTING_ACC = 250.0f;
const float AI_DRAG_CASTING_ACC_RATE = 15.5f;
const float AI_DRAG_CASTING_ACC_ADD = 25.0f;
const float AI_CLICK_CASTING_ACC_RATE = 0.5f;

const float AI_CASTING_SCORE_RATE = 1.5f;
const double AI_CASTING_SCORE_MIN = 15.0f;
const double AI_CASTING_SCORE_MAX = 800.0f;






const int AI_LINE_MIN_REMAIN_COUNT = 40;		// 40m



///////////////////////////////////////////////////////////////////////////////////
// CFishAI

class CFishAI
{
	IMPLEMENT_TISAFE(CFishAI)
public:
	CFishAI();
	virtual ~CFishAI();

public:

	//
	LONG GetBiteFishInfo(CUser* pUser, BiteFishInfo& biteFishInfo, const NFRoomOption& roomOption, LONG lSignType);

	// 
	// 방향
	LONG FishAIDirection(LONG lPrevDirection);
	BOOL CheckFishAbility(const NFAbility& ability, const FishInfo& fishInfo);
	BOOL GetFishingPointFishList(const NFLockedNoteMap& lockedNote, const NFAbility& ability, long lIdxFishMap, long lIdxFishPoint, CFish& biteSpecialFishInfo, EquipItem& equipItem, LONG& lError);
	BOOL GetSpecialFishList(long lIdxFishMap, long lSignType, CFish& biteSpecialFishInfo, const EquipItem& equipItem);
	BOOL GetFishAIInfo(CUser* pUser, MsgCliNGS_ReqFighting& msg, const LineBreakInfo& LBI, LONG& lErrorCode);
	BOOL GetFishSkillInfo(CFish& biteFish, LONG lFishDepth, FishSkill& choiceFishSkill, TLstFishSkillInfo& lstFishSkill, CNFChar& NFChar, LONG& lErrorCode);
	void SetFishSkillValue(FishSkillInfo& fishSkillInfo, LONG lFishSkillIndex, CNFChar& NFChar);
	LONG GetAttackDirection(CNFChar& NFChar);
	void GetFishInfoByFishHP(FishInfo& fishInfo, LONG& lFishAttackRate, double& dFishAttackPower, double& dFishMoveSpeed);
	BOOL GetChoiceFishSkill(LONG lFishSkillPattern, FishSkill& choiceFishSkill, CNFChar& NFChar);

private:

	TMapIndexFishInfo		m_mapFishInfo;
	TMapIndexFishSkill		m_mapFishSkill;

	typedef map<long, long>	TMapUSN2;
	TMapUSN2				m_mapFishAI;

};


extern CFishAI theFishAI;


#endif //!FishAI_H
