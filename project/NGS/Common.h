//
// Common.h
//

#ifndef COMMON_H
#define COMMON_H

#include <ADL/MsgCommonStruct.h>
#include <NF/ADL/MsgNGSCli.h>
#include <NF/ADL/MsgCHSNGS.h>
#include <EncryptPld.h>
#include "RoomEvent.h"
#include <NFVariant/NFGameData.h>
#include "PRNG.h"

class CAbil
{
	typedef std::map< int, int > TAbil; // AbilType, Value

public:
	CAbil(){}
	~CAbil(){}

// 복사 방지용
private:
	CAbil(const CAbil& rhs);
	CAbil& operator=(const CAbil& rhs);

private:
	TAbil m_kContAbil;

public:
	void SetAbil(const int nAbilType, const int nValue)
	{
		TAbil::iterator abil_iter = m_kContAbil.find( nAbilType );
		if( abil_iter != m_kContAbil.end() )
		{
			abil_iter->second = nValue;
		}
		else
		{
			m_kContAbil.insert( std::make_pair( nAbilType, nValue ) );
		}
	}

	int GetAbil(const int nAbilType)
	{
		TAbil::const_iterator c_abil_iter = m_kContAbil.find( nAbilType );
		if( c_abil_iter != m_kContAbil.end() )
		{
			return c_abil_iter->second;
		}

		return 0;
	}
};


///////////////////////////////////////////////////////////////////////
// NGS에서만 사용하는 Define
// extern
extern WORD g_wSvcType;

// struct
class ConstSetFlag
{
public:
	BOOL m_bLRBRun;
	BOOL m_bAMSRun;
	BOOL m_bSGWRUN;
	BOOL m_bLRBActive;
	BOOL m_bStatisticsFlag;
public:
	ConstSetFlag() : m_bLRBRun(FALSE), m_bLRBActive(FALSE) { };
	~ConstSetFlag() {};
	void SetStatisticsFlag(BOOL flag) { m_bStatisticsFlag = flag; }
	BOOL GetStatisticsFlag() { return m_bStatisticsFlag; }
};

extern ConstSetFlag theConstSetFlag;

struct AysncDBQueryItem
{
#define ADBQITEMMAGIC	0x87654321
	DWORD	magic;	
	DWORD	queryTime;
	DWORD	index;	
	int		errcode;
	DBGW_String *result;
	RoomID	roomID;
	LONG	queryType;
	string  query;

	AysncDBQueryItem() { 
		magic = ADBQITEMMAGIC;
		result = NULL;
	}
};

struct RoomEvent
{
	ROOMEVENT		m_nTag;
	union
	{
		LONG			m_lCSN;
		LONG			m_lEventParam;
	};
	
public:
	RoomEvent() { m_nTag = REV_ROOMINVALID; m_lCSN = INVALID_CSN; }
	RoomEvent(ROOMEVENT tag, LONG lCSN) : m_nTag(tag), m_lCSN(lCSN) {}
};

class CRankNWinType
{
public:
	LONG	lRank;
	LONG	lWinType;
};

class IRoomEventHandler
{
public:
	// Handling user events 
	virtual BOOL ProcessRoomEvent( const RoomEvent &evt ) = NULL;
};

// define
#define CHS_INDEX_ANY -1
#define ROOM_DIFF_COUNT 10
#define USER_DIFF_COUNT 50
#define USER_ALIVE_INTERVAL 60000 
#define ALIVE_INTERVAL		30000
#define SIGN_INTERVAL		60000
#define FISH_AI_INTERVAL	1000
#define TIMER_INDEX_SIGN	0x1234567
typedef struct __DBResultInfo
{
	LONG lRevVal;
	CComVariant pvtOutData;
	LONG lResult;
} DBResultInfo;
#define ROUND(x, dig) (floor((x)*pow(10,dig)+0.5)/pow(10,dig))
#define HYPOTE(x, y) sqrt(((x*x)+(y*y)))
typedef std::map<LONG, CRankNWinType>	TMapTeamData;


// enum
enum USER_STATE_INDEX {
	US_INVALID,
	US_ALIVE,
	US_NEEDCHECK,
	US_CHECK,
	US_DEAD,
	US_MAX
};

enum
{
	TERMINATE_ROOM_GRACEFUL = 1,
	TERMINATE_ROOM_IMMEDIATE
};

enum
{
	KICKOUT_USER_CHEAT = 1,
};

enum LinkCutReason
{
	LCS_LINKCUT_INVALID,
	LCS_LINKCUT_KICKOUT,
	LCS_LINKCUT_DISCONNECT
};

enum BCS_STOP_RES
{
	BCS_STOP_UNAVAILABLE,
	BCS_STOP_RES_HASMOREBCSINFO,
	BCS_STOP_RES_NOMOREBCSINFO
};

enum EN_FISHHP_PATTERN
{
	EN_FISHHP_INIT = 1,
	EN_FISHHP_50100,
	EN_FISHHP_2050,
	EN_FISHHP_0120,
	EN_FISHHP_0000,
};


// function
string RoomIDToStr64(const RoomID &rid);
string RoomID2Str( const RoomID &rid);
void RoomID2Str(string & str, const RoomID &rid);
void split(const char* text, const char separator, vector<string>& words);

//inline int urand() {
//	int i1, i2, i3;
//	i1 = rand() & 0x0fff;
//	i2 = (rand() & 0x0fff) << 12;
//	i3 = (rand() & 0x007f) << 24;
//	return (i1 | i2 | i3);
//}
//
//inline int urandom(int nummax) {
//	return (urand() * GetTickCount() % nummax);
//}

// variable
extern char* g_szDBAlias[];


///////////////////////////////////////////////////////////////////////////////////
// Class PreDefine
class CLink;
class CListener;
class CLrbLink;
class CUser;
class RoomID;
class TCUID;

class CNFChar
{
public:
	CNFChar();
	virtual ~CNFChar() { }

public:
	// Only NF
	NFUser& GetNFChar() { return m_nfUser; }
	const NFAbility& GetAbility() const { return m_nfUser.m_nfCharInfoExt.m_nfAbility; }
	const ArcVectorT< LONG >& GetQuickSlot() const { return m_nfUser.m_nfCharInfoExt.m_nfQuickSlot; }
	NFAbilityExt& GetAbilityExt() { return m_nfAbilityExt; }

	void SetNFUserBaseInfo(const NFUserBaseInfo& nfUBI) 
	{
		m_nfUser.m_nfUserBaseInfo = nfUBI; 
		m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN = nfUBI.m_lLastPlayCSN;
	}

	void SetCUser(CUser* pUser) { m_pUser = pUser; }
	void SetMapLodingProgress(MapLoadingProgress& mapLoadingProgress) { m_mapLoadingProgress = mapLoadingProgress; }
	MapLoadingProgress& GetMapLodingProgress() { return m_mapLoadingProgress; }

	long GetIncounterGauge() { return m_lIncounterGauge; }
	void SetIncounterGauge(long lInCounterGauge) { 
		m_lIncounterGauge = lInCounterGauge; 
	}
	void IncounterGaugeIncrement(long lGauge) { 
		m_lIncounterGauge += lGauge;
	}
	void IncounterGaugeDecrement(long lGauge) { 
		m_lIncounterGauge -= lGauge;
		if (m_lIncounterGauge < 0)
			m_lIncounterGauge = 0;
	}
	BOOL IsIncounterGaugeFull();


	void SetFPStatus(LONG lFPStatus) { m_lFPStatus = lFPStatus; }
	LONG GetFPStatus() { return m_lFPStatus; };

	void CalcAblilty(NFAbility& NFAbility, const ClothesItem* pItem, BOOL bIsAdd=TRUE);
	LONG ChangeQuickSlot(vector<LONG>& vecQuickSlot);

	EquipItem* GetBoatEquipItem();			// 현재 사용중인 Boat를 얻어온다.
	long GetUSN() const { return m_nfUser.m_nfUserBaseInfo.m_lUSN; }
	long GetCSN() const { return m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN; }
	long GetGSN() const { return m_nfUser.m_nfUserBaseInfo.m_lGSN; }
	LONG GetLevel() const { return m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel; }
	void SetLevel(LONG lLevel) { m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel = lLevel; }
	LONG GetExp() const { return m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_lExp; }
	void SetExp(LONG lExp) { m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_lExp = lExp; }
	LONG GetGrade() { return m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_lGrade; }
	string& GetCharName() { return m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_strCharName; }
	string& GetLastestLogOutDate() { return m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_strLastestLogOutDate; }

	__int64 GetMoney() const { return m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_llMoney; }
	void SetMoney( const __int64 lMoney ) { m_nfUser.m_nfCharInfoExt.m_nfCharBaseInfo.m_llMoney = lMoney; }

	void SetBiteFishInfo(CFish& biteFish) { m_BiteFish = biteFish; }
	CFish& GetBiteFish() { return m_BiteFish; }

	void SetPrevDragLevel(long lPrevDragLevel) { m_lPrevDragLevel = lPrevDragLevel; }
	long GetPrevDragLevel() { return m_lPrevDragLevel; }

	void SetCharacterMaxPower(double dCharMaxPower) { m_dCharacterMaxPower = dCharMaxPower; }
	double GetCharacterMaxPower() { return m_dCharacterMaxPower; }

	double GetDropAcceleration() { return m_dDropAcceleration; }
	void SetDropAcceleration(double dDropAcceleration) { m_dDropAcceleration = dDropAcceleration; }

	double GetCurrentCharacterPower() { return m_dCurrentCharacterPower; }
	void SetCurrentCharacterPower(double dAddCharPower);
	BOOL SetTireless(long lTirelessType, double dTirelessValue, long lFishHP=0);
	double GetCurrentTireless() { return m_dCurrentTireless; }
	double GetMaxTireless() { return m_dMaxTireless; }

	BOOL GetIsTempLoadPresent() { return m_bIsTempLoadPresent; }
	void SetIsTempLoadPresent(BOOL bIsTempLoadPresent) { m_bIsTempLoadPresent = bIsTempLoadPresent; }

	double GetTempLineLoadMax() { return m_dTempLineLoadMax; }
	void SetTempLineLoadMax(double lTempLineLoadMax) { m_dTempLineLoadMax = lTempLineLoadMax; }
	double GetTempLineLoadLimit() { return m_dTempLineLoadLimit; }
	void SetTempLineLoadLimit(double lTempLineLoadLimit) { m_dTempLineLoadLimit = lTempLineLoadLimit; }
	
	double GetRate() { return m_dRate; }
	void SetRate(double dRate) { m_dRate = dRate; }

	double CalcFishHP(long lCalcFishHP) 
	{ 
		return m_BiteFish.CalcFishHP(lCalcFishHP); 
	}
	LONG GetCurFishHP() { return m_BiteFish.GetCurFishHP(); }

	void SetDistanceFromFish(double dDistanceFromFish) { m_dDistanceFromFish = dDistanceFromFish; }
	double GetDistanceFromFish() { return m_dDistanceFromFish; }

	void SetLineLength(double dLineLength) { m_dLineLength = dLineLength; }
	double GetLineLength() { return m_dLineLength; }

	void SetCurrentFishDepth(LONG lCurrentFishDepth) { m_BiteFish.SetFishCurrentDepth(lCurrentFishDepth); }
	LONG GetCurrentFishDepth() { return m_BiteFish.GetFishCurrentDepth();}

	void SetPrevWindLine(double dPrevLine) { m_dPrevLine = dPrevLine; }
	double GetPrevWindLine() { return m_dPrevLine; }

	void SetPrevFishMoveSpeed(double dPrevFishMoveSpeed) { m_dPrevFishMoveSpeed = dPrevFishMoveSpeed; }
	double GetPrevFishMoveSpeed() { return m_dPrevFishMoveSpeed; }

	double CalcWindLine(double dReelClickTime, double dRetrievalLine=0, double dLimitLine=100000);
	double CalcUnwind(double dUnwindLine);

	void SetIsLineBigger(BOOL bCheck) { m_bIsLineBigger = bCheck; }
	BOOL GetIsLineBigger() { return m_bIsLineBigger; }

	void SetActionStatus(LONG lStatus) { m_lActionStatus = lStatus; }
	BOOL GetActionStatus() { return m_lActionStatus; }

	void SetLastSendTime(DWORD dTime) { m_dwLastSendTime = dTime; }
	DWORD GetLastSendTime() { return m_dwLastSendTime; }

	void SetUserLocation(Coordinates& UserLocation) { m_UserLocation = UserLocation; }
	Coordinates& GetUserLocation() { return m_UserLocation; }

	LONG SetHeadShakeCount(LONG lCount);
	LONG GetHeadShakeCount() { return m_lHeadShakeCount; }

	void SaveLandingFish(LONG lWinCondition);
	const TotalLandingFish& GetSaveLandingFish() const { return m_totLandingFish; }
	TotalLandingFish& GetSaveLandingFish() { return m_totLandingFish; }

	void GetAchievementPoint(AchievementPoint& achvPoint);
	void GetAchievement(TMapAchievement& mapAchv);

// FishAI
	BOOL IsPatternComplete()
	{
		if (m_lCurrentRemainTime <= 0)
			return TRUE;
		return FALSE;
	}
	void DecrementCurrentPatternTime()
	{
		InterlockedDecrement(&m_lCurrentRemainTime);
		if (m_lCurrentRemainTime < 0)
			m_lCurrentRemainTime = 0;
	}
	void GetCurrentMAXPattern(LONG& lCurrentPatternType, LONG& lCurrentPatternTime);
	void SetCurrentMAXFishPattern(LONG lFishHPType, LONG lAttackRate, LONG lRestRate)
	{ 
		m_lCurMAXAttackRate = lAttackRate; 
		m_lCurMAXRestRate = lRestRate; 
	}
	void GetCurrentMAXFishPattern(LONG& lAttackRate, LONG& lRestRate) { lAttackRate = m_lCurMAXAttackRate; lRestRate = m_lCurMAXRestRate; }
	void SetCurrentPatternType(LONG lCurrentPatternType) { m_lCurrentPatternType = lCurrentPatternType; }
	void SetCurrentPatternTime(LONG lCurrentPatternTime) 
	{ 
		m_lCurrentMaxPatternTime	= lCurrentPatternTime; 
		m_lCurrentRemainTime		= lCurrentPatternTime;
	}
	void SetCurrentMAXPattern(LONG lCurrentPatternType, LONG lCurrentPatternTime)
	{ 
		m_lCurrentPatternType = lCurrentPatternType; 

		if (FS_ATTACK == m_lCurrentPatternType || FS_SKILL == m_lCurrentPatternType)
			m_lCurMAXAttackRate = lCurrentPatternTime;
		else
			m_lCurMAXRestRate = lCurrentPatternTime; 

		m_lCurrentRemainTime		= lCurrentPatternTime;
		m_lCurrentMaxPatternTime	= lCurrentPatternTime;		// 클라이언트가 PatternTime을 변경하고자 요청 했을 때, 해당 패턴에 따라 비교 할려는 대상이 필요하므로.. 이 변수에 저장한다.
	}
	void ChangeFishPatternByFishHP(LONG lFishHPType, LONG lAttackRate, LONG lRestRate)
	{
		// 다르다면, 물고기에 있는 정보로 얻어와서 와서 더한다.
		if (m_lCurFishHPType != lFishHPType)
		{
			if (lAttackRate != 0)
                m_lCurMAXAttackRate += lAttackRate;
			
			if (lRestRate != 0)
				m_lCurMAXRestRate += lRestRate;
		}
		else
			m_lCurFishHPType = lFishHPType;
	}
	LONG GetCurrentRemainTime() { return m_lCurrentRemainTime; }
	LONG GetCurrentMaxPatternTime() { return m_lCurrentMaxPatternTime; }
	LONG GetCurrentMaxRemainTime() { return m_lCurMAXAttackRate; }
	LONG GetCurrentPatternType() { return m_lCurrentPatternType; }
	LONG GetCurrentFishSkillType() { return m_BiteFish.GetFishSkillType(); }
	BOOL IsSameDirection() { return m_BiteFish.GetPrevFishDirection() == m_BiteFish.GetFishSkillDirection(); }
	LONG ResultEnduranceRodItem() { return m_lRodEndurance; }
	LONG ResultEnduranceReelItem() { return m_lReelEndurance; }
	void AccountEndurance(LONG lReelEndurance, LONG lRodEndurance);
	void GetEquipItem(TMapInven& mapCountableItem);
	void GetLockedNoteMain(NFLockedNoteMain& lock_main);

// Debugging
	void SetDebuggingMode(LONG lDebuggingMode, PayloadCliNGS& pld);
	LONG GetDebuggingMode() { return m_lDebuggingMode; }
	void GetDebuggingCasting(PayloadCliNGS* pMsg);

	void SortingLandingFishResult(LONG lWinCondition, LONG lTopSize);
	void InitNFCharInfo();
	void InitTeamPlayDataEachUser();

	void SetUsedCastingSkill(TLstSkill& lstUsedCastingSkill) { m_lstUsedCastingSkill = lstUsedCastingSkill; }
	TLstSkill	GetUsedCastingSkill() { return m_lstUsedCastingSkill; }

	uint32_t GetPRNG() { return m_PRNG.Generate(); }
	uint32_t GetPRNG_count() { return m_PRNG.GetCount(); }
	uint32_t GetPRNG_value() { return m_PRNG.GetValue(); }
	void SetPRNGSeed(uint32_t nSeed) { m_PRNG.Reseed(nSeed); }

	void SetUserSlot(LONG lUserSlot) { m_lUserSlot = lUserSlot; }
	LONG GetUserSlot() { return m_lUserSlot; }
	void SetUserStatus(LONG lUserStatus) { m_lUserStatus = lUserStatus; }
	LONG GetUserStatus() { return m_lUserStatus; }

	// RodAngle
	void SetLowRodAngleMax(double dAngle) { m_dLowRodAngleMax = dAngle; }
	double GetLowRodAngleMax() { return m_dLowRodAngleMax; }
	void SetHighRodAngleMax(double dAngle) { m_dHighRodAngleMax = dAngle; }
	double GetHighRodAngleMax() { return m_dHighRodAngleMax; }
	
	double GetCurrentLowRodAngle() { return m_dCurrentLowRodAngle; }
	void SetCurrentLowRodAngle(double dAngle) { m_dCurrentLowRodAngle = dAngle; }
	double GetCurrentHighRodAngle() { return m_dCurrentHighRodAngle; }
	void SetCurrentHighRodAngle(double dAngle) { m_dCurrentHighRodAngle = dAngle; }

private:
	CUser*			m_pUser;
	NFUser			m_nfUser;
	CFish			m_BiteFish;
	TotalLandingFish m_totLandingFish;
	NFAbilityExt m_nfAbilityExt;

private:
	LONG 			m_lActionStatus;
	BOOL			m_bEnvDebuff;
	DWORD			m_dwLastSendTime;
	long			m_lPrevDragLevel;
	double			m_dCharacterMaxPower;
	double			m_dCurrentCharacterPower;
	double			m_dDropAcceleration;
	double			m_dMaxTireless;
	double			m_dCurrentTireless;
	BOOL			m_bIsTempLoadPresent;
	double			m_dTempLineLoadMax;
	double			m_dTempLineLoadLimit;
	double			m_dRate;
	double			m_dDistanceFromFish;
	double			m_dLineLength;
	Coordinates		m_UserLocation;
	double			m_dPrevLine;
	BOOL			m_bIsLineBigger;
	LONG			m_lHeadShakeCount;
	double			m_dPrevFishMoveSpeed;
	LONG			m_lCurFishHPType;

	LONG			m_lCurMAXAttackRate;
	LONG			m_lCurMAXRestRate;
	LONG			m_lCurrentPatternType;
	LONG			m_lCurrentRemainTime;
	LONG			m_lCurrentMaxPatternTime;

	LONG			m_lFPStatus;
	MapLoadingProgress	m_mapLoadingProgress; 
	LONG			m_lIncounterGauge;
	TLstSkill		m_lstUsedCastingSkill;
	PRNG			m_PRNG;
	LONG			m_lUserSlot;
	LONG			m_lUserStatus;

	LONG			m_lReelEndurance;
	LONG			m_lRodEndurance;

	// 로드 휘어짐에 따라서 텐션에 영향을 미치는 값
	double			m_dLowRodAngleMax;
	double			m_dCurrentLowRodAngle;
	double			m_dHighRodAngleMax;
	double			m_dCurrentHighRodAngle;


	// DebuggingCode
	LONG			m_lDebuggingMode;
	PayloadCliNGS	m_msgDebugging;
};


///////////////////////////////////////////////////////////////////////////////////
// CUser
class CUser : public INLSObject,
			  public CAbil
{
	IMPLEMENT_TISAFE(CUser)
		
public:
	typedef PayloadCliNGS TMsg;
	typedef list<TMsg> TMsgQ;
//	typedef list<TGameData> GameDataQ;
	
public:
	CUser(const NFUserBaseInfo& nfUBI);
	virtual ~CUser();
	CLink* GetLink() { return m_pLink; }
	void SetLink(CLink* pLink) { m_pLink = pLink; }

	const RoomID& GetRoomID() const { return m_roomID; }
	RoomID& GetRoomID() { return m_roomID; }
	void SetRoomID(const RoomID& roomID) { m_roomID = roomID; }

	const LRBAddress& GetCHSAddr() { return m_CHSAddr; }
	void SetCHSAddr(const LRBAddress& addr) { m_CHSAddr = addr; }	

	void SetUserState(LONG lState);

	void SetErrorCode(long lErrorCode) { m_lErrorCode = lErrorCode; }
	long GetErrorCode() const { return m_lErrorCode; }

	void SetState(USER_STATE_INDEX index) { m_state = index; }
	USER_STATE_INDEX GetState() { return m_state; }

	void SetValid(BOOL bValid) { m_bValid = bValid; }
	BOOL IsValid() const { return m_bValid; }
public:
	void UnpopMsg();
	void UnpopMsg(const TMsg& msg);
	void PushMsg(const TMsg& msg);
	BOOL PopMsg(TMsg& msg);

	BOOL IsPlaying() { return m_bIsPlaying; }
	void SetPlaying(BOOL bIsPlaying) { m_bIsPlaying = bIsPlaying; }

	LONG m_lRcvMsgCnt;

protected:
	TMsg mLastMsg;
	TMsgQ mRcvMsgQ;

	long m_lErrorCode;
	RoomID m_roomID;  //<- m_sChannel;
	LRBAddress m_CHSAddr;

	CLink* m_pLink;
	USER_STATE_INDEX m_state;
	
//	GameDataQ mGameDataQ; 

	BOOL m_bValid;
	BOOL m_bIsPlaying;

	/////////////////////////////////////
	/// NF
	// 기존꺼 수정
public:
	// for NLS
	STDMETHOD_(void, NLSGetKeyValue)(TKey& key);
	STDMETHOD_(BOOL, NLSGetRoomID) (RoomID & roomID);
	STDMETHOD_(void, NLSSetErrorCode)(LONG lECode);

	long GetUSN() { return m_nfUser.GetNFChar().m_nfUserBaseInfo.m_lUSN; }
	void SetUserID(xstring sID) { m_nfUser.GetNFChar().m_nfUserBaseInfo.m_sUID.assign(sID.c_str(), sID.length()); }
	xstring& GetUserID() { return m_nfUser.GetNFChar().m_nfUserBaseInfo.m_sUID; }

	void SetRoomOption(LONG lCSN, xstring & userID, NFRoomOption& nfRoomOption);

	// 추가
protected:
	NFRoomOption		m_nfRoomOption;
	CNFChar				m_nfUser;
	BOOL				m_bDebuff;
	CONT_NF_FRIEND		m_kContNFFriend;
	CONT_NF_FRIEND_NICK m_kContNFBlock;
	CONT_NF_FRIEND_NICK m_kContNFFriendApplicant;

public:		
	LONG GetCSN() { return m_nfUser.GetCSN(); }
	LONG GetGSN() { return m_nfUser.GetGSN(); }
	LONG GetLevel() { return m_nfUser.GetLevel(); }
	string& GetCharName() { return m_nfUser.GetCharName(); }
	string& GetLastestLogOutDate() { return m_nfUser.GetLastestLogOutDate(); }

	void SetUserSlot(LONG lUserSlot) { m_nfUser.SetUserSlot(lUserSlot); }
	LONG GetUserSlot() { return m_nfUser.GetUserSlot(); }
	void SetUserStatus(LONG lUserStatus) { m_nfUser.SetUserStatus(lUserStatus); }
	LONG GetUserStatus() { return m_nfUser.GetUserStatus(); }
	void SetMaxExp(LONG lMaxExp) { m_nfUser.GetNFChar().m_nfCharInfoExt.m_nfCharBaseInfo.m_lExpMax = lMaxExp; }

	LONG ChangeQuickSlot(vector<LONG>& vecQuickSlot) { return m_nfUser.ChangeQuickSlot(vecQuickSlot); }
	
	NFRoomOption& GetNFRoomOption() { return m_nfRoomOption; }
	const NFRoomOption& GetNFRoomOption() const { return m_nfRoomOption; }

	void SetCurrentFishingPoint(LONG lFishingPoint);
	LONG GetPrevFishingPoint() { return m_nfRoomOption.m_lIdxFishingPoint; }

	const xstring& GetPassword() const { return m_nfRoomOption.m_sPassword; }
	void SetPassword(xstring& pwd) { m_nfRoomOption.m_sPassword.assign(pwd.c_str(), pwd.length()); }

	void SetUserLocation(LONG index) { m_nfUser.GetNFChar().m_nfUserBaseInfo.m_lUserState = index; }
	LONG GetUserLocation() { return m_nfUser.GetNFChar().m_nfUserBaseInfo.m_lUserState; }
	LONG GetFPStatus() { return m_nfUser.GetFPStatus(); }

	void SaveLandingFish(LONG lWinCondition);
	TotalLandingFish& GetSaveLandingFish() { return m_nfUser.GetSaveLandingFish(); }

	void InitTeamPlayDataEachUser();
	void SetEnvDebuff(BOOL bDebuff) { m_bDebuff = bDebuff; }
	BOOL GetEnvDebuff() { return m_bDebuff; }

	NFUserBaseInfo& GetUserData() { return m_nfUser.GetNFChar().m_nfUserBaseInfo; }
	CNFChar& GetNFUser() { return m_nfUser; }
	NFCharInfoExt* GetNFCharInfoExt() { return &m_nfUser.GetNFChar().m_nfCharInfoExt; }
	NFLockedNote& GetLockedNote() { return m_nfUser.GetNFChar().m_nfCharInfoExt.m_nfLockedNote; }
	BOOL GetLockedNoteByMapID(LONG lMapID, NFLockedNoteMap& lockedNoteMap)
	{
		TMapLockedNoteMap::iterator it = m_nfUser.GetNFChar().m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMap.find(lMapID);
		if (it != m_nfUser.GetNFChar().m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMap.end()) {
			lockedNoteMap = (*it).second;
			return TRUE;
		}
		return FALSE;
	}

	void InitUserData();

	void SetNFFriendInfo( const CONT_NF_FRIEND& rkContNFFriend ) { m_kContNFFriend.clear(); m_kContNFFriend = rkContNFFriend; }
	CONT_NF_FRIEND& GetNFFriendInfo() { return m_kContNFFriend; }
	void AddNFFriend( const TKey& rAddKey, const CNFFriend& rAddFriend);
	void DeleteNFFriend( const string& rstrDeleteCharName );

	void SetNFBlock( const CONT_NF_FRIEND_NICK& rkContNFBlock ) { m_kContNFBlock.clear(); m_kContNFBlock = rkContNFBlock; }
	CONT_NF_FRIEND_NICK& GetNFBlockList() { return m_kContNFBlock; }
	void AddNFBlock( const string& rkCharName ) { m_kContNFBlock.push_back(rkCharName); }
	void DeleteNFBlock( const string& rkCharName );

	void SetNFFriendApplicant( const CONT_NF_FRIEND_NICK& rkContNFFriendApplicant ) { m_kContNFFriendApplicant.clear(); m_kContNFFriendApplicant = rkContNFFriendApplicant; }
	CONT_NF_FRIEND_NICK& GetNFFriendApplicant() { return m_kContNFFriendApplicant; }
	void AddNFFriendApplicant( const string& rkCharName ) { m_kContNFFriendApplicant.push_back(rkCharName); }
	void DeleteNFFriendApplicant( const string& rkCharName );

	const std::string GetTutorialDate(LONG lTutorialType)
	{
		if (TUTORIAL_TYPE_1 == lTutorialType)
			return m_nfUser.GetNFChar().m_nfCharInfoExt.m_nfCharBaseInfo.m_strTutorialDate;
		else if (TUTORIAL_TYPE_2 == lTutorialType)
			return m_nfUser.GetNFChar().m_nfCharInfoExt.m_nfCharBaseInfo.m_strStudyDate;
		return G_INVALID_DATE;
	}
	void SetTutorialDate(LONG lTutorialType, const string& strDate)
	{
		if (TUTORIAL_TYPE_1 == lTutorialType)
			m_nfUser.GetNFChar().m_nfCharInfoExt.m_nfCharBaseInfo.m_strTutorialDate = strDate;
		else if (TUTORIAL_TYPE_2 == lTutorialType)
			m_nfUser.GetNFChar().m_nfCharInfoExt.m_nfCharBaseInfo.m_strStudyDate = strDate;
		else return;
		return;
	}
};

typedef plist<CUser> CUserList;

// CLink
class CLink : public GXLink
{
public:
	typedef GXLink TBase;

	static const DWORD INVALID_INDEX = DWORD_MAX;

	CLink();
	virtual ~CLink();
	void SetUser(CUser* pUser);
	CUser* GetUser() { return m_pUser; }
	_inline void SetIP(LONG lIP) { m_lClientIP = lIP; }
	_inline LONG GetIP() const { return m_lClientIP; }
	_inline DWORD GetIndex() const { return m_dwIndex; }
	void SetGatewayIP(LONG lIP)	{	m_lGatewayIP = lIP;	}
	LONG GetGatewayIP()			{	return m_lGatewayIP;}
	void SetMacAddr(string sMacAddr)	{	m_sClientMacAddr = sMacAddr;}
	string GetMacAddr()					{	return m_sClientMacAddr;	}

public:
	template<class T>
	BOOL DoSendMsg(const T& obj)
	{
		GBuf ro;
		if(!::LStore(ro, obj)) return FALSE;
		return DoSend(ro);
	}
protected:
	CUser* m_pUser;
	DWORD m_dwIndex;
	LONG m_lClientIP;					// Client IP Information (2002.10.15)
	LONG m_lGatewayIP;					// Gateway IP Information (2004.8.27)
	string m_sClientMacAddr;
};

typedef plist<CLink> CLinkList;

// NF Room ReadySlot
class CUserSlot
{
public:
	CUserSlot();

public:
	void Init();
	LONG AddSlot(LONG lCSN, LONG lAddSlot=-1);
	LONG RemoveSlot(LONG lPrevSlot, LONG lCSN);
	LONG ChangeSlot(LONG lPrevSlot, LONG lMoveSlot, LONG lCSN);
	BOOL ChangeSlotSize(LONG lSize=32);

private:
	std::vector<LONG>	m_vecUserSlot;
};

// CLrbLink
class CLrbLink : public GBufSocket
{
public:
	typedef GBufSocket TBase;

	CLrbLink() {}
	virtual ~CLrbLink() {}
#if defined(LAPTA_NEW)
public:
	template<class T>
	BOOL DoSendMsg(const T& obj)
	{
		GBuf ro;
		if(!::LStore(ro, obj)) return FALSE;
		return DoSend(ro);
	}
#endif
};

class CEncryptMgr
{
	BOOL m_bMsgEncrypt;		// Flag for Using Message Encryption
	NC_NRC4_KEY m_key;

public:
	CEncryptMgr();
	~CEncryptMgr();

	void SetMsgEncrypt(BOOL bEnable);
	const NC_NRC4_KEY GetEncryptKey();

	BOOL CheckAndEncrypt( GBuf & toSend, const PayloadNGSCli & pld );
	BOOL CheckAndDecrypt( /*in and out*/PayloadCliNGS & pld );

	BOOL IsMsgEncryptNeeded(const PayloadCliNGS& rPld);
	BOOL IsMsgEncryptNeeded(const PayloadNGSCli& rPld);
};

extern CEncryptMgr theEncryptMgr;

#endif //!COMMON_H
