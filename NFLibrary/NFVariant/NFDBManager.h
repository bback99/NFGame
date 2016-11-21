
#pragma once

#include <NFVariant/NFGameData.h>
#include <ACHV/AchvData.h>

class CNFDBManager
{
public:
	const static int SP_SUCCESS_RESULT = 1;

	// NFChar
	const static string QRY_SELECT_NICK_CHECK;
	const static string QRY_SELECT_ALL_BASIC_CHAR;
	const static string QRY_SELECT_CHAR;
	const static string QRY_SELECT_NF_USER_GSN;
	const static string QRY_SELECT_NF_CHAR_CSN_ALL;
	const static string QRY_SELECT_NF_ABILITY_SUM;
	const static string QRY_SELECT_NF_ABILITYEXT_SUM;
	const static string PKG_INSERT_NF_USER;
	const static string PKG_INSERT_NF_CHAR;
	const static string PKG_DROP_NF_CHAR;
	const static string PKG_NF_GAME_UPDATEDATA_CSN;
	const static string PKG_LANDING_FISH;
	const static string PKG_NF_GAME_QUICKSLOT_CSN;
	const static string QRY_UPDATE_LOGOUT_DATE;

	// NFChar-Inven
	const static string QRY_SELECT_CHAR_SIMPLE_INVEN;
	const static string QRY_SELECT_NF_INVEN_CSN;

	// Level
	const static string QRY_SELECT_ALL_LEVEL;

	// Product
	const static string QRY_SELECT_PRODUCT;
	const static string PKG_SHOP_BUY_PRODUCT;
	const static string PKG_SHOP_BUY_PRODUCT_ADMIN;

	// Item
	const static string PKG_GIVE_GAMEITEM;
	const static string QRY_SELECT_GIVE_ITEM;
	const static string QRY_DELETE_GAMEITEM_INVEN;
	const static string PKG_NF_GAME_UPDATE_PARTS;
	const static string PKG_NF_GAME_USE_GAMEITEM;
	const static string PKG_NF_GAME_RECHARGE_GAMEITEM;
	const static string PKG_NF_GAMEITEM_ENCHANT_ITEM;
	
	// Item-Card
	const static string PKG_OPEN_CARDPACK;
	const static string PKG_UPGRADE_CARD;
	const static string PKG_NF_GAME_UPDATE_PARTS_CARD;

	// Tutorial and Scenario
	const static string QRY_UPDATE_TUTORIAL_DATE;
	const static string QRY_UPDATE_STUDY_SCENARIO_DATE;

	// ACHV
	//const static string QRY_SELECT_NF_CHAR_ACHV;
	const static string QRY_SELECT_CHAR_ACHV_POINT;
	const static string QRY_SELECT_ALL_NF_ACHV_REWARD;
	const static string PKG_NF_ACHV_GIVE_REWARD;

	// New Achv...

	const static string QRY_SELECT_NF_CHAR_ACHV;
	const static string PKG_NF_ACHV_UPDATE_ACHV;

	// Community
	const static string QRY_SELECT_NF_LETTER_LIST;				// 편지 본문 제외하고 읽기	
	const static string QRY_SELECT_NF_LETTER_CONTENTS;			// 편지 본문 읽기
	const static string QRY_UPDATE_NF_LETTER_READ;				// 편지 읽음 상태로 업데이트
	const static string QRY_DELETE_NF_LETTER;						// 편지 삭제	
	const static string QRY_INSERT_NF_LETTER;
	const static string	QRY_SELECT_NF_CHAR_KEY_BY_CHAR_NAME;
	const static string QRY_SELECT_NF_FRIEND_LIST;	
	const static string QRY_INSERT_NF_FRIEND;
	const static string QRY_DELETE_NF_FRIEND;
	const static string QRY_UPDATE_NF_FRIEND;
	const static string QRY_SELECT_NF_FRIEND_RELATION;
	const static string QRY_SELECT_NF_FRIEND_NICK_BY_STATUS;

	// Aquarium & LandNote
	const static string QRY_SELECT_GT_CHAR_LOCKEDNOTE_MAIN;
	const static string QRY_LOCKED_NOTE_FISH_UPDATE;
	const static string QRY_LOCKED_NOTE_FISH_INSERT;
	const static string QRY_LOCKED_NOTE_MAP_UPDATE;
	const static string QRY_LOCKED_NOTE_MAP_INSERT;
	const static string QRY_UPDATE_LOCKED_NOTE_MAIN;
	const static string QRY_SELECT_GT_CHAR_LOCKEDNOTE_MAP;
	const static string QRY_SELECT_GT_CHAR_LOCKEDNOTE_FISH;
	
	// WORKING(acepm83@neowiz.com) 수족관
	const static string QRY_INSERT_NF_GT_CHAR_AQUA_FISH;
	const static string QRY_SELECT_NF_GT_CHAR_AQUA_FISH;
	
	const static string QRY_SELECT_NF_GT_CHAR_AQUA;

	// Cheat
	const static string QRY_UPDATE_CHAR_EXP_LEVEL;
	const static string QRY_UPDATE_CHAR_MONEY;

	// MetaData
	const static string QRY_SELECT_ALL_NF_ITEM_EQUIP;
	const static string QRY_SELECT_ALL_NF_ITEM_CLOTHES;
	const static string QRY_SELECT_ALL_NF_ITEM_USABLE;
	const static string QRY_SELECT_ALL_NF_ITEM_SKILL;
	const static string QRY_SELECT_ALL_NF_ITEM_CARD;
	const static string QRY_SELECT_ALL_NF_CARDPACK_OPEN;
	const static string QRY_SELECT_ALL_NF_MAP;
	const static string QRY_SELECT_ALL_NF_FISHINGPOINT;
	const static string QRY_SELECT_ALL_NF_FISH;
	const static string QRY_SELECT_ALL_NF_FISHSKILL;
	const static string QRY_SELECT_ALL_NF_FISHSKILLCODE;
	const static string QRY_SELECT_ALL_NF_FISHINGCODE;
	const static string QRY_SELECT_ALL_NF_SIGN;
	const static string QRY_SELECT_ALL_NF_DROP_ITEM;
	const static string QRY_SELECT_ALL_NF_ADD_FISH_LIST;
	const static string QRY_SELECT_ALL_NF_ITEM_ENCHANT_INFO;

	// 쓸지 않쓸지 모름...
	const static string QRY_SELECT_PRODUCT_NCS;
	

public:
	CNFDBManager();
	~CNFDBManager();


public:
	BOOL DBInit();
	BOOL HasRow(string& strResult);
	string GetParseData(std::string& sTarget);
	BOOL SucceedQuery(DBGW_String& sResult, int& spResult,BOOL isSP);
	BOOL Exec(const string& qry, DBGW_String& sResult,int& spResult, BOOL isSP); // function only for inner-use by ExecQry, ExecSP
	BOOL ExecQry(const string& qry, string& sResult);
	BOOL ExecSP(const string& qry, int& nResult, string& outSResult);

public:
	// NFChar
	BOOL SelectNFBasicChar(TMapNFBasicChar& mapBasicChar);
	BOOL SelectNFCharBaseNExteriorInfo(NFCharInfoExt& nfCharInfoExt, LONG lGSN, long lCSN);
	BOOL SelectNFCharGSN(const string& strSiteUserID, long& lGSN);
	BOOL SelectAllNFCharCSNByGSN(std::list<LONG>& lstCSN, LONG lGSN, LONG& lErr);
	BOOL SelectNFAbilityByCSN(NFCharInfoExt* pNFCharInfo, long lCSN);
	BOOL SelectNFAbilityExtByCSN(NFAbility& nfCharInfo, NFAbilityExt& nfCharInfoExt, long lCSN);
	BOOL ExistNFCharNick(const string& strNick, int& nErr);
	BOOL InsertNFUser(const string& strSiteCode, const string& strSiteID, const string& strPWD, LONG lAdminLEV, LONG& lGSN);
	BOOL InsertNFChar(LONG lGSN, NFCharInfo& nfCharInfoExt, int& lErrorCode);
	BOOL DeleteNFChar(LONG lGSN, LONG lCSN, int& lErrorCode);
	BOOL UpdateNFCharDataByCSN(GameResult& result, NFRoomOption& roomOption, long& lErrorCode);
	BOOL UpdateLandingFish(GameResult& result, NFRoomOption& roomOption, LONG lLengthIndex, LONG lClassIndex, const string& strFishGroup, const string& strLength, const string& strRegDate, const string& strMapID, long& lErrorCode);
	BOOL UpdateQuickSlotByCSN(long lGSN, long lCSN, long lType, long lRegistSlotIndex, long lRegistlItemCode, long lRemoveSlotIndex, long lRemovelItemCode, string& strStartDate, string& strEndDate, long& lErrorCode);
	BOOL UpdateLastLogOutDate(LONG lGSN, LONG lCSN);

	// NFChar-Inven
	BOOL SelectNFSimpleInven(NFCharInven& nfCharSkill, LONG lGSN, long lCSN, LONG& lErr);
	BOOL SelectNFCharInven(TlstInvenSlot& lstRemovedItem, const string& strLogoutDate, NFCharInven& nfCharInven, long lGSN, long lCSN, LONG& lErr);
	BOOL SelectNFCharInven(TlstInvenSlot& lstRemovedItem, const string& strLogoutDate, NFCharInven& nfCharInven, TMapInven& mapExpiredInven, long lGSN, long lCSN, LONG& lErr);

	// Level
	BOOL SelectNFLevel(TMapNFLevel& mapNFLevel);

	// Product
	BOOL SelectNFProduct(TMapItemCodeProduct& mapProduct, TMapItemCodeProduct& mapProductSkill);
	BOOL SelectNFProductPackage(TMapIndexPackage& mapPackage);
	BOOL BuyItemFromShop(LONG lBuyItemType, LONG lGSN, LONG lCSN, LONG lItemID, const string& strProductSRL, LONG& lInvenSRL, LONGLONG& llMoeny, int& lErrorCode);
	BOOL BuyItemFromShop_Admin(LONG lBuyItemType, const CardItem* pItem, LONG lGSN, LONG lCSN, LONG lItemID, const string& strProductSRL, LONG& lInvenSRL, const string& strPackYN, int& lErrorCode);

	// Item
	BOOL InsertBuyItem(LONG lBuyItemType, LONG lItemID, LONG lGSN, LONG lCSN, NFInvenSlot& inven, int& lErrorCode);
	BOOL SelectNFGiveItem(TMapItemIDProduct& mapGiveItem);
	BOOL UpdateRemoveItem(LONG lGSN, LONG lCSN, LONG lInvenSRL);
	BOOL UpdatePartsByCSN(NFInvenSlot& oldInven, NFInvenSlot& newInven, long lGSN, long lCSN, long& lErrorCode);
	BOOL UseGameFunc(LONG lGSN, LONG lCSN, NFInvenSlot& updateItem, const std::string& strDelYN, LONG& lGauge, long& lErrorCode);
	BOOL RechargeGameFunc(LONG lGSN, LONG lCSN, const NFInvenSlot& inven, LONG& lAddGauge, LONGLONG& llMoney, LONG& lErrorCode);
	BOOL EnchantItem(LONG lGSN, LONG lCSN, LONG lItemCode, LONG lInvenSRL, LONG lCardItemCode, LONG lCardInvenSRL, LONG lSubItemCode, LONG lSubInvenSRL, LONG lType, LONG& lErrorCode, LONG& lCardGauge, LONG& lSubGauge);

	// Item-Card
	BOOL UpdateOpenCardPack(LONG lGSN, LONG lCSN, LONG lInvenSRL, LONG lChangedItemCode, int& lErrorCode);
	BOOL UpdateUpgrageCard(LONG lGSN, LONG lCSN, map<LONG, LONG>& mapUpgradeCardInven, NFInvenSlot& upgradeCard, NFInvenSlot& getCardPack, int& lErrorCode);
	BOOL UpdateCardPartsByCSN(NFInvenSlot& oldInven, NFInvenSlot& newInven, LONGLONG& lMoney, long lGSN, long lCSN, long& lErrorCode);

	// Tutorial and Scenario
	BOOL UpdateTutorialDate(LONG lGSN, LONG lCSN, const string& strTutorialDate);
	BOOL UpdateStudyScenarioDate(LONG lGSN, LONG lCSN, const string& strStudyScenarioDate);

	// ACHV
	BOOL SelectNFAchvList(LONG lGSN, LONG lCSN, TMapAchievement& mapAchv, LONG& lErrorCode);
	BOOL SelectNFAchvPoint(LONG lGSN, LONG lCSN, AchievementPoint& ap);
	BOOL SelectAchvReward( TMapAchvReward & mapAchvReward );
	BOOL GiveAchvReward(std::vector<int>& vecInvenSRL, LONG lGSN, LONG lCSN, int achv_id, const std::vector<int>& Reward_Item_Ids, std::vector<NFInvenSlot>& vecInven, LONG& lErrorCode);

	// New Achv 작업중...
	BOOL SelectNFAchvByChar(const LONG lGSN, const LONG lCSN, MAP_ACHV_STATE& rkOutMapAchvState);
	BOOL UpdateAchvProgress(string& outLastUpdateTime, const LONG lGSN, const LONG lCSN, const LONG lAchvID, const double dProgress, const bool isCompleted);

	// Community
	BOOL SelectNFLetterList( const LONG lCSN, CONT_NF_LETTER& rkContNFLetter, const BOOL bNewLetter);
	BOOL SelectNFLetterContent( const __int64 i64LetterIndex, string& rstrContent, string& rstrSendTime );
	BOOL InsertNFLetter( const string& rstrReceiver, const CNFLetter& rnfLetter );
	BOOL DeleteNFLetter( const vector<__int64>& kContLetterIndex );
	BOOL SelectNFFriendInfo( const LONG lCSN, CONT_NF_FRIEND& rkContNFFriend, const LONG lStatus );
	BOOL SelectNFCharKeyByCharName( const string& rstrCharName, TKey& rKey );	
	BOOL InsertNFFriend( const LONG lApplicantCSN, const LONG lAcceptorCSN, const LONG lStatus );
	BOOL UpdateNFFriendStatusToFriend( const string& rstrAcceptorCharName, const string& rstrApplicantCharName );
	BOOL DeleteNFFriend(const string& rstrCharName, const string& rstrDeleteCharName, const LONG lStatus = FR_FRIEND);
	BOOL SelectNFFriendRelation( const LONG lCSN, const LONG lCheckCSN, const LONG lCheckStatus, LONG& rlResultStatus );	
	BOOL SelectNFFriendNickByStatus( const LONG lCSN, CONT_NF_FRIEND_NICK& rkContNFBlock, const LONG lStatus ); // 블록, 나에게 친구신청한 캐릭터 이름 리스트	

	// Aquarium & LockedNote
	BOOL SelectNFLockedNote(LONG lGSN, LONG lCSN, NFLockedNote& lockedNote);
	BOOL SelectNFLockedNoteMain(LONG lGSN, LONG lCSN, NFLockedNoteMain& lockedNoteMain);
	BOOL SelectNFLockedNoteMap(LONG lGSN, LONG lCSN, TMapLockedNoteMap& mapLockedNoteMap);
	BOOL UpdateLockedNoteMap(LONG lMapID, LONG lGSN, LONG lCSN, LONG ltotCNT, LONG lScore, const std::string& strUpdateDate, LONG& lErr);
	BOOL InsertLockedNoteMap(LONG lMapID, LONG lGSN, LONG lCSN, LONG ltotCNT, LONG lScore, const std::string& strInsertDate, LONG& lErr);
	BOOL SelectNFLockedNoteFish(LONG lGSN, LONG lCSN, TMapLockedNoteMap& mapLockedNoteMap);
	BOOL UpdateLockedNoteFish(LONG lMapID, LONG lGSN, LONG lCSN, LONG lLockedNoteFishID, LONG lWeight, LONG lLength, LONG lScore, const std::string& strUpdateDate, LONG& lErr);
	BOOL InsertLockedNoteFish(LONG lMapID, LONG lGSN, LONG lCSN, LONG lLockedNoteFishID, LONG lWeight, LONG lLength, LONG lScore, const std::string& strInsertDate, LONG& lErr);
	BOOL UpdateLockedNoteMain(LONG lTotLandCNT, LONG lGSN, LONG lCSN, const TVecRecentlyLandingFish& landMain);
	BOOL InsertNFCharAquaFish(LONG lGSN, LONG lCSN, LONG lFishID, LONG lLength, LONG lWeight, LONG lElapsedTime, LONG lScore);
	BOOL SelectNFCharAquaFish(LONG lCSN, TMapNFAquaFish& mapNFAquaFish);
	BOOL SelectNFCharAqua(LONG lGSN, LONG lCSN, NFAqua& nfAqua, LONG& lElapsedClearHour, LONG& lElapsedFeedHour);

	// Cheat
	BOOL UpdateCharExpAndLevel(const LONG lCSN, const LONG lExp, const LONG lLevel);
	BOOL UpdateCharMoney(const LONG lCSN, const __int64 i64Money);
	BOOL UpdateCharAchv(const LONG lGSN, const LONG lCSN, const int nAchvID, const double dGauge);

	// MetaData
	BOOL SelectItemCommon(string& strResult, ItemCommon& itemCommon);
	BOOL SelectNFItemEquip(BOOL bIsAll, TMapIndexEquipItem& mapItem, long lSelectIndex=-1);
	BOOL SelectNFItemClothes(BOOL bIsAll, TMapIndexClothesItem& mapItem, long lSelectIndex=-1);
	BOOL SelectNFItemUsable(BOOL bIsAll, TMapIndexUsableItem& mapUsableItem, long lSelectIndex=-1);
	BOOL SelectNFItemSkill(BOOL bIsAll, TMapIndexSkillItem& mapSkillItem, long lSelectIndex=-1);
	BOOL SelectNFItemCard(BOOL bIsAll, TMapLevelCardItem& mapCardPackItem, TMapIndexCardItem& mapCardItem, long lSelectIndex=-1);
	BOOL SelectNFItemCardPackRate(BOOL bIsAll, TMapIndexCardPackRate& mapCardPackRate, LONG& lCnt);
	BOOL SelectNFFishMap(BOOL bIsAll, TMapIndexFishMap& mapFishMap, long lSelectIndex=-1);
	BOOL SelectNFFishingPoint(BOOL bIsAll, TMapIndexFishMap& mapFishMap, long lSelectIndex=-1);
	BOOL SelectNFFishInfo(BOOL bIsAll, TMapIndexFishInfo& mapFishInfo, long lSelectIndex=-1);
	BOOL SelectNFFishSkill(BOOL bIsAll, TMapIndexFishSkill& mapFishSkill, long lSelectIndex=-1);
	BOOL SelectNFFishSkillCode(BOOL bIsAll, TMapIndexFishSkillCode& mapFishSkillcode, long lSelectIndex=-1);
	BOOL SelectNFFishingCode(BOOL bIsAll, TMapIndexIndexList& mapFishingCode, long lSelectIndex=-1);
	BOOL SelectNFSignCode(BOOL bIsAll, TMapIndexFishMap& mapFishMap, LONG& lCnt);
	BOOL SelectNFDropItem(BOOL bIsAll, TMapDropItemRate& mapDropItemRate, LONG& lCnt);
	BOOL SelectNFAddFishList(BOOL bIsAll, TMapAddFishList& mapAddFishList);
	BOOL SelectNFItemEnchantInfo(TmapNFItemEnchantInfo& mapNFItemEnchantInfo);
};

bool Converttime_t2NFDateString( const time_t timeval, std::string & NFDateString );
bool ConvertNFDateString2time_t( const std::string & NFDateString, time_t & timeVal );


extern CNFDBManager		theNFDBMgr;
