//
// Common.cpp
//

#include "stdafx.h"
#include "Common.h"
#include "ErrorLog.h"
#include <ADL/MsgCommonStruct.h>

WORD g_wSvcType = 0;

char* g_szDBAlias[] = {"NullDB|","GameDB|", "LogDB|", "PoliceDB|", "AdminDB|", "MainDB|"};
// 2007.04.26 현재 실서비스에는 NullDB가 들어가 있지 않고 (즉, 버그패치가 되어 있지 않고)
// "GameDB" 부터 시작한다.  다음 정기점검 2007.05.08 에 반영하고 반영되면 본 코드는 주석처리 하시오.

///////////////////////////////////////////////////////////////////////////////////
string RoomIDToStr64(const RoomID &rid)
{
	INT64 rid64;
	rid64 = rid.m_dwGCIID;
	rid64 <<=32;
	rid64 += rid.m_dwGRIID;

	char szRid[30] = "";
	_i64toa(rid64, szRid, 10);

	string str(szRid);
	return str;
}

string RoomID2Str( const RoomID &rid)
{
	return ::format("%d|%d|%d|%d",rid.m_lSSN, rid.m_dwCategory, rid.m_dwGCIID, rid.m_dwGRIID); 
}

void RoomID2Str(string & str, const RoomID &rid)
{
	 
	str = ::format("%d|%d|%d|%d",rid.m_lSSN, rid.m_dwCategory, rid.m_dwGCIID, rid.m_dwGRIID);
}

void split(const char* text, const char separator, vector<string>& words) 
{
	char szBuf[1024] = {0,};
	int nLen = strlen(text);
	for (int i = 0, j = 0; i < nLen ; i++)
	{
		if (text[i] == '\\')
		{
			szBuf[j++] = text[++i];					
		}
		else if (text[i] == separator)
		{
			szBuf[j] = 0x00;
			words.push_back(szBuf);
			j = 0;
		}
		else
			szBuf[j++] = text[i];
	}
	szBuf[j] = 0x00;
	if (strlen(szBuf) >= 0)
		words.push_back(szBuf);
}

///////////////////////////////////////////////////////////////////////////////////
// CNFChar
CNFChar::CNFChar()
{
	m_nfUser.Clear();
	m_nfAbilityExt.Clear();
	m_totLandingFish.Clear();
	m_lIncounterGauge = 0;		// 초기에 한번만 초기화 되고, 이후에 상황에 따라 초기화 되어야 할 변수
	m_lUserSlot = 0;

	InitNFCharInfo();
}

// 물고기 한마리 잡으면 초기화 되어야 하는 데이터
void CNFChar::InitNFCharInfo()
{
	SetActionStatus(STATUS_INIT);
	m_lUserStatus = UIS_INVALID;
	m_dwLastSendTime = 0;
	m_lPrevDragLevel = 0;
	m_dCharacterMaxPower = 0;
	m_dCurrentCharacterPower = 0;
	m_dCurrentTireless = 0;
	m_dTempLineLoadMax = 0;
	m_dTempLineLoadLimit = 0;
	m_dRate = 0;
	m_dDistanceFromFish = 0;
	m_dLineLength = 0;
	m_UserLocation.Clear();
	m_dPrevLine = 0;
	m_bIsLineBigger = 0;
	m_lHeadShakeCount = 0;
	m_dPrevFishMoveSpeed = 0;
	m_lCurFishHPType = 0;
	m_lCurMAXAttackRate = 0;
	m_lCurMAXRestRate = 0;
	m_lCurrentPatternType = 0;
	m_lCurrentRemainTime = 0;
	m_lCurrentMaxPatternTime = 0;
	//m_BiteFish.Clear(); // 수족관 보내기 전에 초기화 하면 안됨.
	m_bIsTempLoadPresent = FALSE;
	m_PRNG.Reseed(0);
	m_bEnvDebuff = FALSE;
	m_lDebuggingMode = 0;
	m_lFPStatus = 1;
	m_mapLoadingProgress.Clear();						// 대회 끝나면 초기화
	m_lstUsedCastingSkill.clear();
	m_dDropAcceleration = 0;
	m_dLowRodAngleMax = 0;
	m_dCurrentLowRodAngle = 0;
	m_dHighRodAngleMax = 0;
	m_dCurrentHighRodAngle = 0;
	m_lReelEndurance = 0;
	m_lRodEndurance = 0;
}

// 레벨에 따른 IncounterGauge 값이 Full인지 체크
BOOL CNFChar::IsIncounterGaugeFull()
{
	BOOL bRet = FALSE;
	long lFullGauge = 0;

	NFCharInfoExt& nfCharInfoExt = m_nfUser.m_nfCharInfoExt;

	switch(nfCharInfoExt.m_nfCharBaseInfo.m_lLevel)
	{
	case 1:	lFullGauge = 100;	break;			// 초보
	case 2:	lFullGauge = 120;	break;			// 하수
	case 3:	lFullGauge = 140;	break;			// 중수
	case 4:	lFullGauge = 160;	break;			// 고수
	case 5:	lFullGauge = 180;	break;			// 프로
	default: lFullGauge = 180;	break;			// MAX값
	}

	if (m_lIncounterGauge >= lFullGauge)
		bRet = TRUE;
	return bRet;
}

void CNFChar::SetCurrentCharacterPower(double dAddCharPower)
{
	m_dCurrentCharacterPower = dAddCharPower;
}


BOOL CNFChar::SetTireless(long lTirelessType, double dTirelessValue, long lFishHP)
{
	// 피로도는 초기에 0부터 시작하며, 피로도가 MAX값에 다다르면 실패...
	// 아이템을 이용해서 피로도를 감소 시켜야 함
	switch(lTirelessType)
	{
	case TIRELESS_INIT:	// MAX값 설정
		{
			m_dMaxTireless = dTirelessValue;
			m_dCurrentTireless = m_dMaxTireless;
			break;			
		}
	case TIRELESS_USEABLE_ITEM:	// 아이템 사용해서 피로도 감소
		{
			m_dCurrentTireless += dTirelessValue;
			if (m_dCurrentTireless > m_dMaxTireless)
				m_dCurrentTireless = m_dMaxTireless;
			break;
		}
	case TIRELESS_NORMAL:	// 힘을 쓰거나, 물고기 힘에 의해서 피로도 증가, FishHP가 0이하이면, 더이상 증가하지 않는다.
		{
			if (lFishHP > 0)
				m_dCurrentTireless -= dTirelessValue; 
			break;	
		}
	case TIRELESS_CONTROL_FAIL: // ROD 컨트롤에 의해 피로도 증가
		{
			CFish&		biteFish		= GetBiteFish();
			FishInfo&	biteFishInfo	= biteFish.GetFish();
			long m_lAddTireless = (long)(biteFishInfo.m_dAttack * 2); // 2011.09.26 변경
			
			m_dCurrentTireless -= m_lAddTireless;
			break;
		}
	case TIRELESS_BULLET_FAIL:
		{
			long m_lAddTireless = (long)(m_dMaxTireless * 0.10);
			m_dCurrentTireless -= m_lAddTireless;
			break;
		}
	default: break;
	}

	if (m_dMaxTireless < 0)
		return FALSE;

	if(m_dCurrentTireless < 0)
		m_dCurrentTireless = 0;

	if (m_dMaxTireless < m_dCurrentTireless)
		m_dCurrentTireless = m_dMaxTireless;		

	if(m_dCurrentTireless == 0)
		return TRUE;		// 피로도 만땅... Fighting 실패!!

	return FALSE;
}

// bRet = TRUE, 줄이 조건에 다 되었다... Landing으로~
// bRet = FALSE, 아직 남아서 랜딩 조건이 안 된다.
double CNFChar::CalcWindLine(double dReelClickTime, double dRetrievalLine, double dLimitLine)		// 줄을 감을때...
{ 
	double dRetWindLine = 0;
	double dRetrievalLinem = dRetrievalLine / 100;

	//		double dLineLimit = dRetrievalLine * 1; // 1초 단위로 움직이기 때문에 
	double dLine = dRetrievalLinem * dReelClickTime;
	if (dLine < 0)
		return dRetWindLine;

	if (m_dPrevLine > 0)
		dLine += m_dPrevLine;

	if (dLine > dLimitLine)
		dLine = dLimitLine;

	//theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, CalcWindLine : ", dLine, ", m_dLineLength :", m_dLineLength);

	m_dLineLength -= dLine;
	if (m_dLineLength <= 0)
		m_dLineLength = 0;
	return dLine;				
}

double CNFChar::CalcUnwind(double dUnwindLine)
{
	m_dLineLength += dUnwindLine;
	return m_dLineLength;
}

LONG CNFChar::SetHeadShakeCount(LONG lCount) 
{ 
	m_lHeadShakeCount = lCount; 
	if (m_lHeadShakeCount >= 3)
		return 0;
	return m_lHeadShakeCount;
}

// 낚시 중간에 장비(Reel, Rod)가 바뀌지 않는 다는 가정하에 아래와 같이 구현... 절대 바뀌면 안 됨 -ㅅ-;;;
void CNFChar::AccountEndurance(LONG lReelEndurance, LONG lRodEndurance)
{
	m_lReelEndurance += (LONG)(lReelEndurance * 0.05);
	m_lRodEndurance	+= (LONG)(lRodEndurance * 0.05);

	// test
	//m_lReelEndurance = 50;
	//m_lRodEndurance = 50;
}

void CNFChar::GetEquipItem(TMapInven& mapEquipItem)
{
	for(int i=eItemType_Lure; i<= eItemType_Line; i++)
	{
		TMapInven::iterator it = m_nfUser.m_nfCharInfoExt.m_nfCharInven.m_mapUsingItem.find(i);
		if (it != m_nfUser.m_nfCharInfoExt.m_nfCharInven.m_mapUsingItem.end())
			mapEquipItem.insert(make_pair((*it).first, (*it).second));
	}
}

void CNFChar::GetLockedNoteMain(NFLockedNoteMain& lock_main)
{
	lock_main = m_nfUser.m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMain;
}

void CNFChar::SetDebuggingMode(LONG lDebuggingMode, PayloadCliNGS& pld)
{ 
	m_lDebuggingMode = lDebuggingMode; 
	m_msgDebugging = pld;
}

void CNFChar::GetDebuggingCasting(PayloadCliNGS* pMsg)
{  
	switch(pMsg->mTagID)
	{
	case PayloadCliNGS::msgReqCasting_Tag:
		{
			//LONG			lCastingType;		// (1. DragCasting, 2. ClickCasting)
			//LONG			lRealCastingDist;	// 실제 던진 거리
			//LONG			lDragIntensity;		// 던진 강도
			pMsg->un.m_msgReqCasting->m_lCastingType = m_msgDebugging.un.m_msgReqCasting->m_lCastingType; 
			pMsg->un.m_msgReqCasting->m_lRealCastingDist = m_msgDebugging.un.m_msgReqCasting->m_lRealCastingDist;
			pMsg->un.m_msgReqCasting->m_lDragIntensity = m_msgDebugging.un.m_msgReqCasting->m_lDragIntensity;

			break;
		}
	case PayloadCliNGS::msgReqCastingResult_Tag:
		{
			//LONG	m_lErrorCode;
			pMsg->un.m_msgReqCastingResult->m_lErrorCode = m_msgDebugging.un.m_msgReqCastingResult->m_lErrorCode;
			break;
		}
	case PayloadCliNGS::msgReqAction_Tag:
		{
			//LONG	m_lActionType;
			//LONG	m_lManualActionType;
			//double	m_dReelClickTime;
			pMsg->un.m_msgReqAction->m_lActionType = m_msgDebugging.un.m_msgReqAction->m_lActionType;
			pMsg->un.m_msgReqAction->m_lManualActionType = m_msgDebugging.un.m_msgReqAction->m_lManualActionType;
			pMsg->un.m_msgReqAction->m_dReelClickTime = m_msgDebugging.un.m_msgReqAction->m_dReelClickTime;
			break;
		}
	case PayloadCliNGS::msgReqHookingResult_Tag:
		{
			pMsg->un.m_msgReqHookingResult->m_lHookingResultType = m_msgDebugging.un.m_msgReqHookingResult->m_lHookingResultType;
			break;
		}
	case PayloadCliNGS::msgReqFighting_Tag:
		{
			//LONG	m_lActionSeccuess;
			//float	m_fRateByDirInten;
			//LONG	m_lDragLevel;
			//double	m_dReelClickTime;
			//LONG	m_lLineLength;
			pMsg->un.m_msgReqFighting->m_fRateByDirInten = m_msgDebugging.un.m_msgReqFighting->m_fRateByDirInten;
//			pMsg->un.m_msgReqFighting->m_lDragLevel = m_msgDebugging.un.m_msgReqFighting->m_lDragLevel;
//			pMsg->un.m_msgReqFightingAction->m_dReelClickTime = m_msgDebugging.un.m_msgReqFightingAction->m_dReelClickTime;
//			pMsg->un.m_msgReqFightingAction->m_dLineLength = m_msgDebugging.un.m_msgReqFightingAction->m_dLineLength;
			break;
		}
	case PayloadCliNGS::msgReqLanding_Tag:
		{
			break;
		}
	default:
		break;
	}

	m_lDebuggingMode = 0;
}

void CNFChar::SaveLandingFish(LONG lWinCondition)
{
	LandingFish landingFish;
	landingFish.Clear();
	m_BiteFish.GetLandingFish(landingFish);
	m_totLandingFish.m_lstLandingFish.push_back(landingFish);
	m_BiteFish.SetLandingDate();

	// get_result_sorting(5)
	SortingLandingFishResult(lWinCondition, 5);
}

bool CompareSize_LandingFish(const LandingFish& elem1, const LandingFish& elem2)
{
	return elem1.m_dResultSize > elem2.m_dResultSize;
}

bool CompareWeight_LandingFish(const LandingFish& elem1, const LandingFish& elem2)
{
	return elem1.m_dResultWeight > elem2.m_dResultWeight;
}

void CNFChar::SortingLandingFishResult(LONG lWinCondition, LONG lTopSize)
{
	// 누적 때문에 초기화!!!
	m_totLandingFish.m_lTotScore = 0;
	m_totLandingFish.m_dTotSize = 0;
	m_totLandingFish.m_dTotWeight = 0;
	m_totLandingFish.m_llTotMoney = 0;

	// count
	m_totLandingFish.m_lCatchCount = m_totLandingFish.m_lstLandingFish.size();

	// 
	switch(lWinCondition)
	{
	case ERW_MAXSIZE:
	case ERW_TOT_SIZE_COUNT:
		{
			m_totLandingFish.m_lstLandingFish.sort(CompareSize_LandingFish);	
		}
		break;

	case ERW_MAXWEIGHT:
	case ERW_TOT_WEIGHT_COUNT:
		{
			m_totLandingFish.m_lstLandingFish.sort(CompareWeight_LandingFish);
		}
		break;

	default:
		break;
	}

	LONG lCnt = 0;
	ForEachElmt(TLstLandingFish, m_totLandingFish.m_lstLandingFish, it, ij)
	{
		m_totLandingFish.m_lTotScore += (*it).m_lResultScore;
		m_totLandingFish.m_llTotMoney += (*it).m_llMoney;
		
		if (lTopSize >= ++lCnt)
			m_totLandingFish.m_dTotSize += (*it).m_dResultSize;

		if (lTopSize >=lCnt)
			m_totLandingFish.m_dTotWeight += (*it).m_dResultWeight;

		if (lCnt == 1) {
			m_totLandingFish.m_dMaxSize = (*it).m_dResultSize;
			m_totLandingFish.m_dMaxWeight = (*it).m_dResultWeight;
		}
		else {
			if (m_totLandingFish.m_dMaxSize < (*it).m_dResultSize)
				m_totLandingFish.m_dMaxSize = (*it).m_dResultSize;
			
			if (m_totLandingFish.m_dMaxWeight < (*it).m_dResultWeight)
				m_totLandingFish.m_dMaxWeight = (*it).m_dResultWeight;
		}
	}
}

// 대회 한판이 끝나면 초기화 되어야 하는 데이터
void CNFChar::InitTeamPlayDataEachUser()
{
	m_totLandingFish.Clear();

	InitNFCharInfo();
}

LONG CNFChar::ChangeQuickSlot(vector<LONG>& vecQuickSlot)
{
	NFCharInfoExt& nfCharInfoExt = m_nfUser.m_nfCharInfoExt;
	nfCharInfoExt.m_nfQuickSlot.clear();

	ForEachElmt(vector<LONG>, vecQuickSlot, it, ij)
		nfCharInfoExt.m_nfQuickSlot.push_back( (*it) );

	return 1;
}

// NFInvenSlot에서 현재 사용중인 아이템중 보트가 있는지 체크해온다.
EquipItem* CNFChar::GetBoatEquipItem()
{
	NFCharInfoExt& nfCharInfoExt = m_nfUser.m_nfCharInfoExt;
	EquipItem* pBoat = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Boat].m_lItemCode);
	return pBoat;
}

// FishAI
void CNFChar::GetCurrentMAXPattern(LONG& lCurrentPatternType, LONG& lCurrentPatternTime)
{	
	lCurrentPatternType = m_lCurrentPatternType; 

	if (FS_ATTACK == m_lCurrentPatternType || FS_SKILL == m_lCurrentPatternType)
		lCurrentPatternTime = m_lCurMAXAttackRate;
	else
		lCurrentPatternTime = m_lCurMAXRestRate; 
}

void CNFChar::GetAchievementPoint(AchievementPoint& achvPoint)
{
	achvPoint = m_nfUser.m_nfCharInfoExt.m_nfCharAchievement.m_nfCharAP;
}

void CNFChar::GetAchievement(TMapAchievement& mapAchv)
{
	mapAchv = m_nfUser.m_nfCharInfoExt.m_nfCharAchievement.m_nfCharAchieve;
}

///////////////////////////////////////////////////////////////////////////////////
// CUser
CUser::CUser(const NFUserBaseInfo& nfUBI) : m_bValid(FALSE)
{
	m_pLink = NULL;
	m_state = US_INVALID;
	m_lRcvMsgCnt = 0;
	m_bDebuff = FALSE;

	m_nfUser.SetNFUserBaseInfo(nfUBI);
	m_nfUser.GetNFChar().m_nfUserBaseInfo.m_lUserState = ULS_ROOMLOBBY;
}

CUser::~CUser()
{
	if(m_pLink)
	{
		m_pLink->SetUser(NULL);
		delete(m_pLink);
		m_pLink = NULL;
	}
}

void CUser::UnpopMsg()
{
	TLock lo(this);
	mRcvMsgQ.push_front(mLastMsg); 
}

void CUser::UnpopMsg(const TMsg& msg) 
{
	TLock lo(this);
	mRcvMsgQ.push_front(msg); 
}

void CUser::PushMsg(const TMsg& msg) 
{
	TLock lo(this);
	mRcvMsgQ.push_back(TMsg());
	mRcvMsgQ.back().BCopy(msg);
}

BOOL CUser::PopMsg(CUser::TMsg& msg) 
{
	TLock lo(this);
	if(mRcvMsgQ.empty()) return FALSE;
	mLastMsg.BCopy(mRcvMsgQ.front());
	msg.BCopy(mLastMsg);
	mRcvMsgQ.pop_front();
	return TRUE;
}

// Only NF
STDMETHODIMP_(void) CUser::NLSGetKeyValue(TKey& key)
{
	key.m_lMainKey = GetUSN();
	key.m_lSubKey = GetCSN();
}

STDMETHODIMP_(BOOL) CUser::NLSGetRoomID(RoomID & roomID)
{
	roomID = GetRoomID();
	return TRUE;
}

STDMETHODIMP_(void) CUser::NLSSetErrorCode(LONG lECode)
{
	SetErrorCode(lECode);
}

void CUser::InitTeamPlayDataEachUser()
{
	SetUserLocation(ULS_ROOMLOBBY);
	InitUserData();
	m_nfUser.InitTeamPlayDataEachUser();
}

void CUser::SaveLandingFish(LONG lWinCondition)
{
	m_nfUser.SaveLandingFish(lWinCondition);
}

void CUser::SetUserState(LONG lState)
{
	m_nfUser.GetNFChar().m_nfUserBaseInfo.m_lUserState = lState;
}

void CUser::SetCurrentFishingPoint(LONG lFishingPoint)
{
	m_nfRoomOption.m_lIdxFishingPoint = lFishingPoint;
}

void CUser::SetRoomOption(LONG lCSN, xstring & userID, NFRoomOption& nfRoomOption)
{
	m_nfRoomOption.BCopy(nfRoomOption);

	// 방제 신고에 따른 추가 (2003/01/22)
	m_nfRoomOption.m_lCreatorUSN = lCSN;//roomOption.m_lCreatorUSN;
	m_nfRoomOption.m_sCreatorID = userID.c_str();//roomOption.m_sCreatorID.c_str();
}

void CUser::InitUserData()
{
	m_nfRoomOption.m_lIdxFishingPoint = 0;
}

void CUser::AddNFFriend( const TKey& rAddKey, const CNFFriend& rAddFriend)
{
	m_kContNFFriend.insert( std::make_pair( rAddKey, rAddFriend ) );
}

void CUser::DeleteNFFriend( const string& rstrDeleteCharName )
{
	CONT_NF_FRIEND::iterator iter = m_kContNFFriend.begin();
	while( m_kContNFFriend.end() != iter )
	{
		if( rstrDeleteCharName == iter->second.m_strCharName )
		{
			m_kContNFFriend.erase(iter);
			break;
		}

		++iter;
	}
}

void CUser::DeleteNFBlock( const string& rkCharName )
{
	CONT_NF_FRIEND_NICK::iterator iter = std::find(m_kContNFBlock.begin(), m_kContNFBlock.end(), rkCharName);
	if( iter != m_kContNFBlock.end() )
	{
		m_kContNFBlock.erase(iter);
	}
}

void CUser::DeleteNFFriendApplicant( const string& rkCharName )
{
	CONT_NF_FRIEND_NICK::iterator iter = std::find(m_kContNFFriendApplicant.begin(), m_kContNFFriendApplicant.end(), rkCharName);
	if( iter != m_kContNFFriendApplicant.end() )
	{
		m_kContNFFriendApplicant.erase(iter);
	}
}

///////////////////////////////////////////////////////////////////////////////
// CLink
CLink::CLink()
{
	static DWORD s_dwIndex = 0;
	m_dwIndex = ::InterlockedIncrement((LPLONG)&s_dwIndex);
	m_pUser = NULL;
}

CLink::~CLink()
{
	ASSERT(m_pUser == 0);
}

void CLink::SetUser(CUser* pUser)
{
	m_pUser = pUser;
}

//////////////////////////////////////////////////////////////////////////
// CUserSlot
CUserSlot::CUserSlot()
{
	m_vecUserSlot.reserve(32);

	for(int i=0; i<16; i++)
		m_vecUserSlot.push_back(-1);
}

void CUserSlot::Init()
{
	for(size_t i=0; i<m_vecUserSlot.size(); i++)
		m_vecUserSlot[i] = -1;
}

LONG CUserSlot::AddSlot(LONG lCSN, LONG lAddSlot)
{
	LONG lRet = -1;
	if (-1 != lAddSlot)		// 해당 위치에 추가, 기존에 있으면 에러 -1 리턴
	{
		if (m_vecUserSlot[lAddSlot] == -1)		// 기존에 있다..
		{
			m_vecUserSlot[lAddSlot] = lCSN;
			lRet = lAddSlot;
		}
	}
	else		// 자동으로 빈 곳 찾아주기
	{
		for(size_t i=0; i<m_vecUserSlot.size(); i++)
		{
			LONG lSlot = m_vecUserSlot[i];
			if (lSlot == -1)
			{
				m_vecUserSlot[i] = lCSN;
				lRet = i;
				break;
			}
		}
	}
	return lRet;
}

LONG CUserSlot::RemoveSlot(LONG lPrevSlot, LONG lCSN)
{
	if (lPrevSlot < 0 || lPrevSlot >= 500 )
		return -1;

	if (m_vecUserSlot[lPrevSlot] == lCSN)
	{
		m_vecUserSlot[lPrevSlot] = -1;
		return -1;
	}
	
	return 1;		// error 기존 사람과 지우려는 사람이 다르다!!!
}

LONG CUserSlot::ChangeSlot(LONG lPrevSlot, LONG lMoveSlot, LONG lCSN)
{
	if (lPrevSlot < 0 || lPrevSlot >= 500 )
		return -1;

	if (lMoveSlot < 0 || lMoveSlot >= 500 )
		return -1;

	if (m_vecUserSlot[lMoveSlot] == -1)
	{
		m_vecUserSlot[lMoveSlot] = lCSN;
		m_vecUserSlot[lPrevSlot] = -1;
		return lMoveSlot;
	}
	return -1;
}

BOOL CUserSlot::ChangeSlotSize(LONG lSize)
{
	if (lSize == (LONG)m_vecUserSlot.size())
		return FALSE;

	for(int i=0; i<lSize-16; i++)
		m_vecUserSlot.push_back(-1);

	return TRUE;
}




///////////////////////////////////////////////////////////////////////////////////
// EncryptManager

#define KEY_SEED_LEN 16

CEncryptMgr theEncryptMgr;

CEncryptMgr::CEncryptMgr()
{
	m_bMsgEncrypt = FALSE;
	unsigned char szKeySeed[KEY_SEED_LEN];
	::memcpy(szKeySeed, "$z#*H7U@$2Fwer#c", KEY_SEED_LEN);
	::InitNCryptKey(m_key, szKeySeed, KEY_SEED_LEN);
}

CEncryptMgr::~CEncryptMgr()
{
}

void CEncryptMgr::SetMsgEncrypt(BOOL bEnable)
{
	m_bMsgEncrypt = bEnable;
}

const NC_NRC4_KEY CEncryptMgr::GetEncryptKey()
{
	return m_key;
}

BOOL CEncryptMgr::IsMsgEncryptNeeded(const PayloadCliNGS& rPld)
{
	if (FALSE == m_bMsgEncrypt)
		return FALSE;

	switch (rPld.mTagID)
	{
	case PayloadCliNGS::msgCreateRoomReq_Tag:
	case PayloadCliNGS::msgJoinRoomReq_Tag:
	case PayloadCliNGS::msgCreateRoomNtf_Tag:
	case PayloadCliNGS::msgJoinRoomNtf_Tag:
		return TRUE;
	}

	return FALSE;
}

BOOL CEncryptMgr::IsMsgEncryptNeeded(const PayloadNGSCli& rPld)
{
	if (FALSE == m_bMsgEncrypt)
		return FALSE;

	switch (rPld.mTagID)
	{
	case PayloadNGSCli::msgCreateRoomNtf_Tag:
	case PayloadNGSCli::msgJoinRoomAns_Tag:
		return TRUE;
	}

	return FALSE;
}

BOOL CEncryptMgr::CheckAndEncrypt( GBuf & toSend, const PayloadNGSCli & pld )
{
	toSend.Clear();

// @@ Encrypt
//	if (TRUE == theEncryptMgr.IsMsgEncryptNeeded(pld))
//	{
//		PayloadNGSCli EncPld;
//		int ret = 0;
//		if( 0 == ( ret = ::EncryptPld<PayloadNGSCli, EncryptedMsg>(EncPld, pld, PayloadNGSCli::msgEncryptedMsg_Tag, theEncryptMgr.GetEncryptKey()) ) )
//		{
//			if( ::LStore( toSend, EncPld ) )
//				return true;
//		} 
//		else
//		{
//			theLog.Put(WAR_UK, "NGS_PacketEncrypt"_COMMA, "CRoomInternalLogic::SendToUser - Message Encryption is failed. (Tag:", pld.mTagID, ", Error:", GetEncryptPldErrorString(ret), ")");
//		}
//	}
//	else
//	{
		if( ::LStore( toSend, pld ) )
			return true;
//	}
	return false;	
}


BOOL CEncryptMgr::CheckAndDecrypt( /* IN and OUT*/PayloadCliNGS & pld )
{
	// @@ Encrypt
	//if (PayloadCliNGS::msgEncryptedMsg_Tag == pld.mTagID)
	//{
	//	int nRet = ::DecryptPld<PayloadCliNGS, EncryptedMsg>(pld, pld, theEncryptMgr.GetEncryptKey());
	//	if( 0 == nRet )
	//	{
	//		if (TRUE == theEncryptMgr.IsMsgEncryptNeeded(pld))
	//		{
	//			return true;
	//		}
	//		else
	//		{
	//			theLog.Put(WAR_UK, "NGS_PacketEncrypt"_COMMA, "CRoomInternalLogic::OnUserMsg - This Message should not be Encrypted. (Tag:", pld.mTagID, ")");
	//		}
	//	}
	//	else
	//	{
	//		theLog.Put(WAR_UK, "NGS_PacketEncrypt"_COMMA, "CRoomInternalLogic::OnUserMsg - Message Decryption is failed. (Error:", GetEncryptPldErrorString(nRet), ")");
	//	}
	//}
	//// if Plain Message...
	//else
	//{
	//	// Check using Encryption
	//	if (FALSE == theEncryptMgr.IsMsgEncryptNeeded(pld))
	//	{

	//		return true;
	//	}
	//	else
	//		theLog.Put(WAR_UK, "NGS_PacketEncrypt"_COMMA, "CRoomInternalLogic::OnUserMsg - This Message should be Encrypted. (Tag:", pld.mTagID, ")");
	//}
	return true;
}