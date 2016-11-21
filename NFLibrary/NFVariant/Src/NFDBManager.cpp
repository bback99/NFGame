#include "stdafx.h"
#include <NFVariant/NFDBManager.h>
#include <NFVariant/NFGameData.h>
#include <NFVariant/NFMenu.h>
#include <NFVariant/NFItem.h>


/*
W A R N I N G !!!
아래에서 LONGLONG에 대한 인자는 반드시 %I64d로 해야한다. %d로 하면 뒤에 따라오는 파라미터에 문제가 생긴다
*/


// NFChar
const string CNFDBManager::QRY_SELECT_NICK_CHECK = "GameDB|Q|SELECT COUNT(CSN) FROM NF_GT_CHAR WHERE LNICK=LOWER(?)|%s";
const string CNFDBManager::QRY_SELECT_ALL_BASIC_CHAR = "GameDB|Q|SELECT CHAR_TYPE, NAME, FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISH_POINT, LUCKY_POINT, DEFAULT_ROD, DEFAULT_LINE, DEFAULT_LURE, DEFAULT_REEL FROM NF_BT_CHAR";
const string CNFDBManager::QRY_SELECT_CHAR = "GameDB|Q|SELECT CHAR_TYPE, NICK, LNICK, REG_DATE, LAST_LOGON_DATE, LAST_LOGOUT_DATE, TUTORIAL_DATE, STUDY_SCENARIO_DATE, EXP, LEV, MONEY, FAMENAME, GRADE, HAIRCOLOR, HAIRSTYLE, FACE, JACKET, PANTS, SHORTCUT0, SHORTCUT1, SHORTCUT2, SHORTCUT3, SHORTCUT4, SHORTCUT5, SHORTCUT6, SHORTCUT7, SHORTCUT8, SHORTCUT9 FROM NF_GT_CHAR WHERE GSN=? AND CSN=?|%d|%d";
const string CNFDBManager::QRY_SELECT_NF_USER_GSN = "GameDB|Q|SELECT GSN FROM NF_GT_USER WHERE SITE_USER_ID=?|%s";
const string CNFDBManager::QRY_SELECT_NF_CHAR_CSN_ALL = "GameDB|Q|SELECT CSN FROM NF_GT_CHAR WHERE GSN=? ORDER BY REG_DATE|%d";
const string CNFDBManager::QRY_SELECT_NF_ABILITY_SUM = "GameDB|Q|SELECT nvl(SUM(FLY_DIST), 0), nvl(SUM(CHARM), 0), nvl(SUM(STRENGTH), 0), nvl(SUM(AGILITY), 0), nvl(SUM(CONTROL), 0), nvl(SUM(HEALTH), 0), nvl(SUM(FISH_POINT), 0), nvl(SUM(LUCKY_POINT), 0) FROM NF_BT_ITEM WHERE ITEM_CODE in (SELECT ITEM_CODE FROM NF_GT_CHAR_INVEN WHERE CSN=? AND EQUIP_YN='Y') AND ITEM_GRP <> 'S' AND ITEM_GRP <> 'U'|%d";
const string CNFDBManager::QRY_SELECT_NF_ABILITYEXT_SUM = "GameDB|Q|SELECT nvl(SUM(FLY_DIST), 0), nvl(SUM(CHARM), 0), nvl(SUM(STRENGTH), 0), nvl(SUM(AGILITY), 0), nvl(SUM(CONTROL), 0), nvl(SUM(HEALTH), 0), nvl(SUM(FISH_POINT), 0), nvl(SUM(LUCKY_POINT), 0), nvl(SUM(KEEP_LUCK_FLAG), 0), nvl(SUM(CASTING_SCORE), 0), nvl(SUM(BACKLASH_RATE), 0), nvl(SUM(INCOUNT_SCORE), 0), nvl(SUM(SPECIAL_FISH_BITE), 0), nvl(SUM(ADD_EXP_RATE), 0), nvl(SUM(ADD_GAME_MONEY_RATE), 0) FROM NF_BT_ITEM WHERE ITEM_CODE in (SELECT ITEM_CODE FROM NF_GT_CHAR_INVEN WHERE CSN=? AND EQUIP_YN='Y') AND ITEM_GRP <> 'S' AND ITEM_GRP <> 'U'|%d";
const string CNFDBManager::PKG_INSERT_NF_USER = "GameDB|S|PKG_NF_USER.REGIST_USER|%s|%s|%d";
const string CNFDBManager::PKG_INSERT_NF_CHAR = "GameDB|S|PKG_NF_CHAR.REGIST_CHAR|%s|%d|%d|%s|%d|%d|%d|%d|%Id";
const string CNFDBManager::PKG_DROP_NF_CHAR = "GameDB|S|PKG_NF_CHAR.DROP_CHAR|%d|%d";
const string CNFDBManager::PKG_NF_GAME_UPDATEDATA_CSN = "GameDB|S|PKG_NF_GAME.UPDATE_CHARDATA|%d|%d|%d|%d|%I64d|%s|%d|%d|%d|%d|%s";
const string CNFDBManager::PKG_LANDING_FISH = "GameDB|S|PKG_NF_GAME.LANDING_FISH|%d|%d|%s|%d|%d|%d|%I64d|%s|%s|%d|%d|%d|%f|%f|%d|%d|%d|%d|%s|%d|%s|%s|%s|%s|%s|%s|%s";
const string CNFDBManager::PKG_NF_GAME_QUICKSLOT_CSN = "GameDB|S|PKG_NF_GAME.UPDATE_SHORTCUT|%d|%d|%d|%d|%d|%d|%d";
const string CNFDBManager::QRY_UPDATE_LOGOUT_DATE = "GameDB|Q|UPDATE NF_GT_CHAR SET LAST_LOGOUT_DATE=? WHERE GSN=? AND CSN=?|%s|%d|%d";

// NFChar-Inven
const string CNFDBManager::QRY_SELECT_CHAR_SIMPLE_INVEN = "GameDB|Q|SELECT ITEM_CODE, INVEN_SRL, PARTS FROM NF_GT_CHAR_INVEN WHERE PARTS >= 9 AND PARTS < 20 AND EFF_END_DATE >= to_char(sysdate, 'YYYYMMDDHH24MISS') AND EQUIP_YN = 'Y' AND GSN=? AND CSN=?|%d|%d";
const string CNFDBManager::QRY_SELECT_NF_INVEN_CSN = "GameDB|Q|SELECT INVEN_SRL, ITEM_CODE, EFF_START_DATE, EFF_END_DATE, GAUGE, PARTS, EQUIP_YN, PACK_YN, PERIOD, ENCHANT_LEVEL FROM NF_GT_CHAR_INVEN WHERE EFF_END_DATE >= ? AND GSN=? AND CSN=?|%s|%d|%d";

// Level
const string CNFDBManager::QRY_SELECT_ALL_LEVEL = "GameDB|Q|SELECT LEV, NEED_EXP, GET_LIMIT_EXP, FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISH_POINT, LUCKY_POINT FROM NF_BT_LEVEL";

// Product
const string CNFDBManager::QRY_SELECT_PRODUCT = "GameDB|Q|SELECT ITEM_CODE, ITEM_ID, ITEM_GRP, SALEINFO_ID, REG_DATE, END_DATE, SALE_TYPE, PRICE, SALE_STATUS, ITEM_TYPE, USE_TYPE, GAUGE, PERIOD, PACK_YN, ACTIVATION_YN, NAME, HIT_YN, GAUGE_MONEY FROM NF_BT_ITEM WHERE SALE_STATUS = 'Y'";
const string CNFDBManager::PKG_SHOP_BUY_PRODUCT = "GameDB|S|PKG_NF_SHOP.BUY_PRODUCT|%d|%d|%d|%d|%d";
const string CNFDBManager::PKG_SHOP_BUY_PRODUCT_ADMIN = "GameDB|S|PKG_NF_ADMIN.GIVE_GAMEITEM|%d|%d|%d|%d|%s|%s";

// Item
const string CNFDBManager::PKG_GIVE_GAMEITEM = "GameDB|S|PKG_NF_GAMEITEM.GIVE_GAMEITEM|%d|%d|%d|%d|%d";
const string CNFDBManager::QRY_SELECT_GIVE_ITEM = "GameDB|Q|SELECT ITEM_CODE, ITEM_ID, ITEM_GRP, REG_DATE, END_DATE, USE_TYPE, GAUGE, PERIOD, PACK_YN, PARTS FROM NF_BT_ITEM WHERE SALE_STATUS = 'N' AND ITEM_TYPE = 'D' OR ITEM_TYPE = 'V'";
const string CNFDBManager::QRY_DELETE_GAMEITEM_INVEN = "GameDB|Q|DELETE NF_GT_CHAR_INVEN WHERE GSN=? AND CSN=? AND INVEN_SRL=?|%d|%d|%d";
const string CNFDBManager::PKG_NF_GAME_UPDATE_PARTS = "GameDB|S|PKG_NF_GAME.UPDATE_PARTS|%d|%d|%d|%d";
const string CNFDBManager::PKG_NF_GAME_USE_GAMEITEM = "GameDB|S|PKG_NF_GAMEITEM.USE_GAMEFUNC|%d|%d|%d|%d|%s|%d";
const string CNFDBManager::PKG_NF_GAME_RECHARGE_GAMEITEM = "GameDB|S|PKG_NF_GAMEITEM.RECHARGE_GAMEFUNC|%d|%d|%d|%d|%d|%I64d";
const string CNFDBManager::PKG_NF_GAMEITEM_ENCHANT_ITEM = "GameDB|S|PKG_NF_GAMEITEM.ENCHANT_ITEM|%d|%d|%d|%d|%d|%d|%d|%d|%d";


// Item-Card
const string CNFDBManager::PKG_OPEN_CARDPACK = "GameDB|S|PKG_NF_CARD.OPEN_CARDPACK|%d|%d|%d|%d";
const string CNFDBManager::PKG_UPGRADE_CARD = "GameDB|S|PKG_NF_CARD.UPGRADE_CARD|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%s";
const string CNFDBManager::PKG_NF_GAME_UPDATE_PARTS_CARD = "GameDB|S|PKG_NF_CARD.UPDATE_PARTS_CARD|%d|%d|%I64d|%d|%d|%d|%d";

// Tutorial and Scenario
const string CNFDBManager::QRY_UPDATE_TUTORIAL_DATE = "GameDB|Q|UPDATE NF_GT_CHAR SET TUTORIAL_DATE=? WHERE GSN=? AND CSN=?|%s|%d|%d";
const string CNFDBManager::QRY_UPDATE_STUDY_SCENARIO_DATE = "GameDB|Q|UPDATE NF_GT_CHAR SET STUDY_SCENARIO_DATE=? WHERE GSN=? AND CSN=?|%s|%d|%d";

// ACHV
//const string CNFDBManager::QRY_SELECT_NF_CHAR_ACHV = "GameDB|Q|SELECT ACHV_ID, PROGRESS, COMPLETE_DATE, GET_REWARD_DATE, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4, LAST_UPDATE_DATE, REMARK FROM NF_GT_CHAR_ACHV WHERE GSN=? AND CSN=?|%d|%d";
const string CNFDBManager::QRY_SELECT_CHAR_ACHV_POINT = "GameDB|Q|SELECT CATEGORY, AP FROM NF_GT_CHAR_AP WHERE GSN=? AND CSN=?|%d|%d";
const string CNFDBManager::QRY_SELECT_ALL_NF_ACHV_REWARD = "GameDB|Q|SELECT ACHV_ID, AP, MONEY, EXP, ITEM_CNT, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4 FROM NF_BT_ACHV_REWARD";
const string CNFDBManager::PKG_NF_ACHV_GIVE_REWARD = "GameDB|S|PKG_NF_ACHV.GIVE_REWARD|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d";

// New Achv...
const string CNFDBManager::QRY_SELECT_NF_CHAR_ACHV = "GameDB|Q|SELECT ACHV_ID, PROGRESS, COMPLETE_DATE, GET_REWARD_DATE, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4, LAST_UPDATE_DATE, REMARK FROM NF_GT_CHAR_ACHV WHERE GSN=? AND CSN=?|%d|%d";
const string CNFDBManager::PKG_NF_ACHV_UPDATE_ACHV = "GameDB|S|PKG_NF_ACHV.UPDATE_ACHV|%d|%d|%d|%.2f|%s";

// Community
const string CNFDBManager::QRY_SELECT_NF_LETTER_LIST = "GameDB|Q|SELECT MAIL_SRL, SEND_NICK, MAIL_TYPE, TITLE, READ_YN, (TRUNC(to_date(REG_DATE, 'YYYYMMDDHH24MISS')+60) - TRUNC(sysdate)) FROM (SELECT * FROM NF_GT_MAIL ORDER BY REG_DATE DESC) WHERE ROWNUM <= ? AND RECV_CSN = ?|%d|%d";
const string CNFDBManager::QRY_SELECT_NF_LETTER_CONTENTS = "GameDB|Q|SELECT CONTENTS, REG_DATE FROM NF_GT_MAIL WHERE MAIL_SRL=?|%I64d";
const string CNFDBManager::QRY_UPDATE_NF_LETTER_READ = "GameDB|Q|UPDATE NF_GT_MAIL SET READ_YN='Y', READ_DATE = to_char(sysdate, 'YYYYMMDDHH24MISS') WHERE MAIL_SRL=?|%I64d";
const string CNFDBManager::QRY_DELETE_NF_LETTER = "GameDB|Q|DELETE NF_GT_MAIL WHERE MAIL_SRL=?|%I64d";
const string CNFDBManager::QRY_INSERT_NF_LETTER = "GameDB|Q|INSERT INTO NF_GT_MAIL VALUES(SEQ_NF_GT_MAIL.nextval,( SELECT GSN FROM NF_GT_CHAR WHERE LNICK = lower(?)), (SELECT CSN FROM NF_GT_CHAR WHERE LNICK = lower(?)),?,(SELECT GSN FROM NF_GT_CHAR WHERE LNICK = lower(?)),(SELECT CSN FROM NF_GT_CHAR WHERE LNICK = lower(?)),?,?,?,?,?,'N',NULL,NULL,to_char(sysdate, 'YYYYMMDDHH24MISS'))|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s";

const string CNFDBManager::QRY_SELECT_NF_CHAR_KEY_BY_CHAR_NAME = "GameDB|Q|SELECT A.SITE_USER_ID USN, B.CSN FROM NF_GT_USER A, NF_GT_CHAR B WHERE A.GSN = B.GSN AND B.LNICK = lower(?)|%s";
const string CNFDBManager::QRY_SELECT_NF_FRIEND_LIST = "GameDB|Q|SELECT B.SITE_USER_ID, A.CSN, A.NICK, A.LEV FROM NF_GT_CHAR A, NF_GT_USER B, NF_GT_CHAR_FRIEND C WHERE C.CSN = ? AND C.FRIEND_CSN = A.CSN AND B.GSN = A.GSN AND C.STATUS = ?|%d|%s";
const string CNFDBManager::QRY_INSERT_NF_FRIEND = "GameDB|Q|INSERT INTO NF_GT_CHAR_FRIEND VALUES((SELECT GSN FROM NF_GT_CHAR WHERE CSN = ?),(SELECT CSN FROM NF_GT_CHAR WHERE CSN = ?),?,(SELECT GSN FROM NF_GT_CHAR WHERE CSN = ?),(SELECT CSN FROM NF_GT_CHAR WHERE CSN = ?),NULL)|%d|%d|%s|%d|%d";
const string CNFDBManager::QRY_DELETE_NF_FRIEND = "GameDB|Q|DELETE FROM NF_GT_CHAR_FRIEND WHERE CSN = ( SELECT CSN FROM NF_GT_CHAR WHERE LNICK = lower(?) ) AND FRIEND_CSN = ( SELECT CSN FROM NF_GT_CHAR WHERE LNICK = lower(?) ) AND STATUS=?|%s|%s|%s";
const string CNFDBManager::QRY_UPDATE_NF_FRIEND = "GameDB|Q|UPDATE NF_GT_CHAR_FRIEND SET STATUS=?, REG_DATE=? WHERE CSN = (SELECT CSN FROM NF_GT_CHAR WHERE LNICK = lower(?)) AND FRIEND_CSN = (SELECT CSN FROM NF_GT_CHAR WHERE LNICK=lower(?))|%s|%s|%s|%s";
const string CNFDBManager::QRY_SELECT_NF_FRIEND_RELATION = "GameDB|Q|SELECT 1 FROM NF_GT_CHAR_FRIEND WHERE CSN=? AND FRIEND_CSN=? AND STATUS=?|%d|%d|%s";
const string CNFDBManager::QRY_SELECT_NF_FRIEND_NICK_BY_STATUS = "GameDB|Q|SELECT A.NICK FROM NF_GT_CHAR A, NF_GT_CHAR_FRIEND B WHERE B.FRIEND_CSN = A.CSN AND B.STATUS = ? AND B.CSN = ?|%s|%d";

// Aquarium & LandNote
const string CNFDBManager::QRY_SELECT_GT_CHAR_LOCKEDNOTE_MAIN = "GameDB|Q|SELECT FISH_GROUP_CNT, LENGTH_FISH_GROUP_1, LENGTH_FISH_GROUP_2, LENGTH_BEST_1, LENGTH_BEST_2, LENGTH_REG_DATE_1, LENGTH_REG_DATE_2, CLASS_FISH_GROUP_1, CLASS_FISH_GROUP_2, CLASS_BEST_1, CLASS_BEST_2, CLASS_REG_DATE_1, CLASS_REG_DATE_2, CLASS_BEST, RECENT_FISH_GROUP, RECENT_LENGTH, RECENT_REG_DATE, RECENT_MAP_ID NF_GT_CHAR_MAP_MAIN WHERE GSN=? AND CSN=?|%d|%d";
const string CNFDBManager::QRY_LOCKED_NOTE_FISH_UPDATE = "GameDB|Q|UPDATE NF_GT_CHAR_MAP_FISH SET WEIGHT=?, LENGTH=?, SCORE=?, UPDATE_DATE=? WHERE MAP_ID=? AND GSN=? AND CSN = ? AND FISH_GROUP = ?|%d|%d|%d|%s|%d|%d|%d|%d";
const string CNFDBManager::QRY_LOCKED_NOTE_FISH_INSERT = "GameDB|Q|INSERT INTO NF_GT_CHAR_MAP_FISH(MAP_ID, GSN, CSN, FISH_GROUP, WEIGHT, LENGTH, SCORE, REG_DATE, UPDATE_DATE) values (?, ?, ?, ?, ?, ?, ?, ?, ?)|%d|%d|%d|%d|%d|%d|%d|%s|%s";
const string CNFDBManager::QRY_LOCKED_NOTE_MAP_UPDATE = "GameDB|Q|UPDATE NF_GT_CHAR_MAP SET FISH_GROUP_CNT=?, TOT_SCORE=?, UPDATE_DATE=? WHERE MAP_ID=? AND GSN=? AND CSN=?|%d|%d|%s|%d|%d|%d";
const string CNFDBManager::QRY_LOCKED_NOTE_MAP_INSERT = "GameDB|Q|INSERT INTO NF_GT_CHAR_MAP(MAP_ID, GSN, CSN, FISH_GROUP_CNT, TOT_SCORE, REG_DATE, UPDATE_DATE) values (?, ?, ?, ?, ?, ?, ?)|%d|%d|%d|%d|%d%|s|%s";
const string CNFDBManager::QRY_UPDATE_LOCKED_NOTE_MAIN = "GameDB|Q|UPDATE NF_GT_CHAR_MAP_MAIN SET FISH_ID_CNT=?,FISH_ID_1_1=?,WEIGHT_1_1=?,LENGTH_1_1=?,REG_DATE_1_1=?,FISH_ID_1_2=?,WEIGHT_1_2=?,LENGTH_1_2=?,REG_DATE_1_2=?,FISH_ID_1_3=?,WEIGHT_1_3=?,LENGTH_1_3=?,REG_DATE_1_3=?,FISH_ID_2_1=?,WEIGHT_2_1=?,LENGTH_2_1=?,REG_DATE_2_1=?,FISH_ID_2_2=?,WEIGHT_2_2=?,LENGTH_2_2=?,REG_DATE_2_2=?,FISH_ID_2_3=?,WEIGHT_2_3=?,LENGTH_2_3=?,REG_DATE_2_3=?,FISH_ID_3_1=?,WEIGHT_3_1=?,LENGTH_3_1=?,REG_DATE_3_1=?,FISH_ID_3_2=?,WEIGHT_3_2=?,LENGTH_3_2=?,REG_DATE_3_2=?,FISH_ID_3_3=?,WEIGHT_3_3=?,LENGTH_3_3=?,REG_DATE_3_3=? WHERE GSN=? AND CSN=?|%d|%d|%d|%d|%s|%d|%d|%d|%s|%d|%d|%d|%s|%d|%d|%d|%s|%d|%d|%d|%s|%d|%d|%d|%s|%d|%d|%d|%s|%d|%d|%d|%s|%d|%d|%d|%s|%d|%d";
const string CNFDBManager::QRY_SELECT_GT_CHAR_LOCKEDNOTE_MAP = "GameDB|Q|SELECT MAP_ID, FISH_GROUP_CNT, TOT_SCORE FROM NF_GT_CHAR_MAP WHERE GSN=? AND CSN=?|%d|%d";
const string CNFDBManager::QRY_SELECT_GT_CHAR_LOCKEDNOTE_FISH = "GameDB|Q|SELECT MAP_ID, FISH_GROUP, WEIGHT, LENGTH, SCORE, UPDATE_DATE FROM NF_GT_CHAR_MAP_FISH WHERE GSN=? AND CSN=?|%d|%d";

// WORKING(acepm83@neowiz.com) 수족관
const string CNFDBManager::QRY_INSERT_NF_GT_CHAR_AQUA_FISH = "GameDB|Q|INSERT INTO NF_GT_CHAR_AQUA_FISH VALUES(?,?,seq_nf_aqua.nextval,?,to_char(sysdate, 'YYYYMMDDHH24MISS'),?,?,?,?)|%d|%d|%d|%d|%d|%d|%d";
const string CNFDBManager::QRY_SELECT_NF_GT_CHAR_AQUA_FISH = "GameDB|Q|SELECT AQUA_SEQ, FISH_ID, REG_DATE, LENGTH, WEIGHT, CATCH_TIME, SCORE FROM NF_GT_CHAR_AQUA_FISH WHERE CSN=?|%d";
const string CNFDBManager::QRY_SELECT_NF_GT_CHAR_AQUA = "GameDB|Q|SELECT CLEAR_DATE, FEED_DATE, AQUA_LEVEL, AQUA_THEME_ITEM_CODE, FEED_GAUGE, CLEAR_GAUGE, AQUA_SCORE, TRUNC(MOD((sysdate - TO_DATE(CLEAR_DATE, 'YYYYMMDDHH24MISS')),1) * 24), TRUNC(MOD((sysdate - TO_DATE(FEED_DATE, 'YYYYMMDDHH24MISS')),1) * 24) FROM NF_GT_CHAR_AQUA WHERE GSN=? AND CSN=?|%d|%d";


// Cheat
const string CNFDBManager::QRY_UPDATE_CHAR_EXP_LEVEL = "GameDB|Q|UPDATE NF_GT_CHAR SET EXP=?, LEV=? WHERE CSN=?|%d|%d|%d";
const string CNFDBManager::QRY_UPDATE_CHAR_MONEY = "GameDB|Q|UPDATE NF_GT_CHAR SET MONEY=? WHERE CSN=?|%I64d|%d";

// MetaData
const string CNFDBManager::QRY_SELECT_ALL_NF_ITEM_EQUIP = "GameDB|Q|SELECT ITEM_CODE, ITEM_ID, ITEM_GRP, ITEM_LEV, CHAR_LEV, PARTS, CAPACITY, BUOYANCY, END_DEPTH, START_DEPTH, LOAD_MAX, LOAD_LIMIT, ENDURANCE, BOAT_TYPE, BOAT_LEV, BOAT_MAX, RETRIEVAL, ADD_FISH_LIST_ID, LURE, ENDURANCE_PRICE, EQUIP_ITEM_TYPE, USE_TYPE, FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISH_POINT, LUCKY_POINT, KEEP_LUCK_FLAG, CASTING_SCORE, BACKLASH_RATE, INCOUNT_SCORE, SPECIAL_FISH_BITE, ADD_EXP_RATE, ADD_GAME_MONEY_RATE FROM NF_BT_ITEM WHERE ITEM_GRP = 'E'";
const string CNFDBManager::QRY_SELECT_ALL_NF_ITEM_CLOTHES = "GameDB|Q|SELECT ITEM_CODE, ITEM_ID, ITEM_GRP, ITEM_LEV, CHAR_LEV, PARTS, CAPACITY, DEF_CHAR_SRL, ENVATTRI, USE_TYPE, FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISH_POINT, LUCKY_POINT, KEEP_LUCK_FLAG, CASTING_SCORE, BACKLASH_RATE, INCOUNT_SCORE, SPECIAL_FISH_BITE, ADD_EXP_RATE, ADD_GAME_MONEY_RATE FROM NF_BT_ITEM WHERE ITEM_GRP = 'C'";
const string CNFDBManager::QRY_SELECT_ALL_NF_ITEM_USABLE = "GameDB|Q|SELECT ITEM_CODE, ITEM_ID, ITEM_GRP, ITEM_LEV, CHAR_LEV, PARTS, CAPACITY, DELAY, HP, USE_TYPE, FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISH_POINT, LUCKY_POINT, KEEP_LUCK_FLAG, CASTING_SCORE, BACKLASH_RATE, INCOUNT_SCORE, SPECIAL_FISH_BITE, ADD_EXP_RATE, ADD_GAME_MONEY_RATE FROM NF_BT_ITEM WHERE ITEM_GRP = 'U'";
const string CNFDBManager::QRY_SELECT_ALL_NF_ITEM_SKILL = "GameDB|Q|SELECT ITEM_CODE, ITEM_ID, ITEM_GRP, ITEM_LEV, CHAR_LEV, PARTS, CAPACITY, USED, RE_USE_TIME, READY_TIME, CONDITION, USE_TYPE, FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISH_POINT, LUCKY_POINT, KEEP_LUCK_FLAG, CASTING_SCORE, BACKLASH_RATE, INCOUNT_SCORE, SPECIAL_FISH_BITE, ADD_EXP_RATE, ADD_GAME_MONEY_RATE FROM NF_BT_ITEM WHERE ITEM_GRP = 'S'";
const string CNFDBManager::QRY_SELECT_ALL_NF_ITEM_CARD = "GameDB|Q|SELECT ITEM_CODE, ITEM_ID, ITEM_GRP, ITEM_LEV, PARTS, CAPACITY, CARD_IDX, USE_TYPE, FLY_DIST, CHARM, STRENGTH, AGILITY, HEALTH, LUCKY_POINT FROM NF_BT_ITEM WHERE ITEM_GRP = 'R'";
const string CNFDBManager::QRY_SELECT_ALL_NF_CARDPACK_OPEN = "GameDB|Q|SELECT CARDPACK_ID, CARD_ID, RATE FROM NF_BT_CARDPACK_OPEN";
//const string CNFDBManager::QRY_SELECT_ALL_NF_MAP = "GameDB|Q|SELECT MAP_ID, LENGTH, WIDTH, POINT_SUM, POINT_BOAT, POINT_WALK, SIGN_GAUGE, MAP_MAX_SCORE FROM NF_BT_MAP";
const string CNFDBManager::QRY_SELECT_ALL_NF_MAP = "GameDB|Q|SELECT MAP_ID, LENGTH, WIDTH, POINT_SUM, POINT_BOAT, POINT_WALK, SIGN_GAUGE FROM NF_BT_MAP";
const string CNFDBManager::QRY_SELECT_ALL_NF_FISHINGPOINT = "GameDB|Q|SELECT FISHING_ID, MAP_ID, POINT_TYPE, LOCATIONX, LOCATIONY, MAX_USER, SALT_YN FROM NF_BT_FISHING_POINT";
const string CNFDBManager::QRY_SELECT_ALL_NF_FISH = "GameDB|Q|SELECT FISH_ID, MAP_ID, SALT, CLASS, TYPE, MIN_SIZE, MAX_SIZE, MIN_WEIGHT, MAX_WEIGHT, POINT, ATTACK, ATTACK_AMOUNT, ATTACK_RATE, RESTRATE, MOVE_PATTERN, MOVE_SPEED, SKILL_GROUP, LV, HITPOINT, MIN_DEPTH, MAX_DEPTH, LANDING_HEADSHAKE, FEEDTIME, HOOKTIME, DROPITEM_MAX, DROPITEM_IDX, EXP, MONEY, LANDNOTE_FISH_ID, FLY_DIST, CHARM, STRENGTH, AGILITY, LUCKY_POINT, LANDING_HEADSHAKE, MOVE_PATTERN FROM NF_BT_FISH";
const string CNFDBManager::QRY_SELECT_ALL_NF_FISHSKILL = "GameDB|Q|SELECT SKILL_ID, LV, TYPE, CONDITION, REUSETIME, HEADSHAKE, EFFECTDIRECTION, COSTHP, ATTACK_POWER, JUMP_HEIGHT, JUMP_DISTANCE, DIVINGDEPTH, ADDDISTANCE, ROD_REACITON_DIR, BULLET, CAST_TIME FROM NF_BT_FISH_SKILL";
const string CNFDBManager::QRY_SELECT_ALL_NF_FISHSKILLCODE = "GameDB|Q|SELECT SKILL_ID, SKILLSRL1, RATE1, SKILLSRL2, RATE2, SKILLSRL3, RATE3, SKILLSRL4, RATE4, SKILLSRL5, RATE5, SKILLSRL6, RATE6, SKILLSRL7, RATE7 FROM NF_BT_FISH_SKILL_CODE";
const string CNFDBManager::QRY_SELECT_ALL_NF_FISHINGCODE = "GameDB|Q|SELECT FISHING_ID, FISH_ID FROM NF_BT_FISHING_CODE";
const string CNFDBManager::QRY_SELECT_ALL_NF_SIGN = "GameDB|Q|SELECT MAP_ID, SIGN_ID, SIGN_PROBABILITY, FISH_ID, FISH_ID_PROBABILITY FROM NF_BT_SIGN";
const string CNFDBManager::QRY_SELECT_ALL_NF_DROP_ITEM = "GameDB|Q|SELECT DROPITEM_IDX, ITEM_ID, DROPITEM_RATE FROM NF_BT_FISH_DROPITEM";
const string CNFDBManager::QRY_SELECT_ALL_NF_ADD_FISH_LIST = "GameDB|Q|SELECT ADD_FISH_LIST_ID, FISH_ID_1, FISH_ID_1_RATE, FISH_ID_2, FISH_ID_2_RATE, FISH_ID_3, FISH_ID_3_RATE, FISH_ID_4, FISH_ID_4_RATE, FISH_ID_5, FISH_ID_5_RATE FROM NF_BT_ADD_FISH_LIST";
const string CNFDBManager::QRY_SELECT_ALL_NF_ITEM_ENCHANT_INFO = "GameDB|Q|SELECT ENCHANT_TYPE, ENCHANT_LEVEL, FLY_DIST, STRENGTH, AGILITY, CONTROL, FISHPOINT, SUCCESS_RATE, DESTROY_RATE, GUAGE_DECREASE, MAX_COUNT FROM NF_BT_ITEM_ENCHANT";

// 쓸지 않쓸지 모름...
const string CNFDBManager::QRY_SELECT_PRODUCT_NCS = "GameDB|Q|SELECT ITEM_ID, ITEM_GRP, SALEINFO_ID, REG_DATE, END_DATE, SALE_TYPE, PRICE, SALE_STATUS, ITEM_TYPE, GAUGE, PACK_YN, ACTIVATION_YN, NAME, HIT_YN, GAUGE_MONEY, USE_TYPE FROM NF_BT_ITEM WHERE SALE_STATUS = 'Y' AND ITEM_GRP = 'X'";

std::string GetParseData2(string& sTarget, const string& sToken)
{
	string sRet;
	int nIndex = sTarget.find_first_of(sToken.c_str());
	if ( nIndex != (int)NPOS )
	{
		sRet = sTarget.substr(0, nIndex);
		sTarget.erase( 0, nIndex + 1 ); // sToken도 지운다
	}
	else
	{
		sRet = sTarget;
		sTarget.erase();
	}
	return sRet;
}

std::vector<LONG> GetParse_RecentlyFish(std::string strTarget, const std::string& sToken)
{
	std::vector<LONG> vecRet;
	while(true)
	{
		LONG lVal = atoi(GetParseData2(strTarget, sToken).c_str());
		vecRet.push_back(lVal);
		
		if (strTarget.size() <= 0)
			break;
	}
	return vecRet;
}

std::vector<string> GetParse_RecentlyFish_string(std::string strTarget, const std::string& sToken)
{
	std::vector<string> vecRet;
	while(true)
	{
		std::string strVal = GetParseData2(strTarget, sToken);
		vecRet.push_back(strVal);

		if (strTarget.size() <= 0)
			break;
	}
	return vecRet;
}

#define MAX_tstringBUF_BIG (10240)

bool Converttime_t2NFDateString( const time_t timeval, std::string & NFDateString )
{
	if( achv::InvalidTime_t == timeval )
	{
		NFDateString = G_INVALID_DATE;
		return true;
	}
	char buffer[16];
	struct tm * timeinfo = localtime( &timeval );

	if( strftime( buffer, sizeof( buffer), "%Y%m%d%H%M%S" , timeinfo ) )
	{
		NFDateString = buffer;
		return true;
	}

	return false;
}

bool ConvertNFDateString2time_t( const std::string & NFDateString, time_t & timeVal )
{
	if( 0 == atoi( NFDateString.c_str() ) )
	{
		timeVal = achv::InvalidTime_t;
		return true;
	}

	if( NFDateString.length() < 14 )
	{
		timeVal = achv::InvalidTime_t;
		return false;
	}

	struct tm when;

	memset( &when, 0, sizeof(when) );

	std::string Token;

	when.tm_isdst = 0;

	Token = NFDateString.substr( 0, 4 );
	when.tm_year = atoi( Token.c_str() ) - 1900;

	Token = NFDateString.substr( 4, 2 );
	when.tm_mon = atoi( Token.c_str() ) - 1;

	Token = NFDateString.substr( 6, 2 );
	when.tm_mday = atoi( Token.c_str() );

	Token = NFDateString.substr( 8, 2 );
	when.tm_hour = atoi( Token.c_str() );

	Token = NFDateString.substr( 10, 2 );
	when.tm_min = atoi( Token.c_str() );

	Token = NFDateString.substr( 12, 2 );
	when.tm_sec = atoi( Token.c_str() );

	timeVal = mktime( &when );

	return true;
}

bool IsNULLField( const std::string& Value )
{
	return Value.length() == 0;
}

///////////////////////////////////////////////////////////////////////////////
// Format tstring

string vformat_big(LPCSTR fmt, va_list vl) 
{
	CHAR buf[MAX_tstringBUF_BIG + 1];
	int nBuf = _vsnprintf(buf, MAX_tstringBUF_BIG, fmt, vl);
	ASSERT(nBuf >= 0);
	if(nBuf < 0)
		buf[MAX_tstringBUF_BIG] = 0;
	return string(buf);
}
wstring vformat_big(LPCWSTR fmt, va_list vl) 
{
	WCHAR buf[MAX_tstringBUF_BIG + 1];
	int nBuf = _vsnwprintf(buf, MAX_tstringBUF_BIG, fmt, vl);
	ASSERT(nBuf >= 0);
	if(nBuf < 0)
		buf[MAX_tstringBUF_BIG] = 0;
	return wstring(buf);
}
string format_big(LPCSTR fmt, ...) 
{
	va_list vl;
	va_start(vl, fmt);
	string ret = vformat_big(fmt, vl);
	va_end(vl);
	return ret;
}
wstring format_big(LPCWSTR fmt, ...) 
{
	va_list vl;
	va_start(vl, fmt);
	wstring ret = vformat_big(fmt, vl);
	va_end(vl);
	return ret;
}

CNFDBManager	theNFDBMgr;

CNFDBManager::CNFDBManager()
{
}

CNFDBManager::~CNFDBManager()
{
}

BOOL CNFDBManager::DBInit()
{
	std::string		strDBGWMgrFileName = "DBGWMGR_NF.ini";
	int nErrorCode = 0;
	if ( !::DBGWMInit(strDBGWMgrFileName.c_str(), &nErrorCode ) )
		return FALSE;

	return TRUE;
}

BOOL CNFDBManager::HasRow(string& strResult)
{
	// 성공한 경우만 아래 코드가 실행되야 한다.
	strResult.erase(0, 4);
	return strResult != ""; 
}

std::string CNFDBManager::GetParseData(std::string& strTarget)
{
	std::string strRet;
	size_t nIndex = strTarget.find_first_of("|");
	if ( nIndex != -1 )
	{
		strRet = strTarget.substr(0, nIndex);
		strTarget.erase( 0, nIndex + 1 );
	}
	return strRet;
}

BOOL CNFDBManager::SucceedQuery(DBGW_String& sResult, int& spResult,BOOL isSP)
{
	if( isSP == TRUE )
	{
		string tempString = (LPCSTR)sResult.GetData();
		BOOL ret = ( GetParseData(tempString) == "S" );
		if( ret == TRUE )
		{
			spResult = atoi(GetParseData(tempString).c_str());
		}
		else 
		{
			GetParseData(tempString); // error code
			spResult = atoi(GetParseData(tempString).c_str()); // sp result
		}
		return ret;
	}
	else
	{
		return ( strncmp((LPCSTR)sResult.GetData(), "S|0|", strlen("S|0|")) == 0 );
	}
}

BOOL CNFDBManager::Exec(const string& qry, DBGW_String& sResult, int& spResult, BOOL isSP)
{
	int nErrCode = 0;

	BOOL bRet = ::ExecuteQuery(QT_NORMAL, qry.c_str(), &sResult, &nErrCode);
	if (!bRet || !SucceedQuery(sResult, spResult, isSP)) 
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CNFDBManager::ExecQry(const string& qry, string& sResult)
{
	DBGW_String dbResult;
	int tempInt = 0;
	BOOL ret = Exec(qry, dbResult, tempInt, FALSE);
	if( !ret )
	{
		return FALSE;
	}

	sResult = (LPCSTR)dbResult.GetData();
	return TRUE;
}

BOOL CNFDBManager::ExecSP(const string& qry, int& nResult, string& sResult)
{
	DBGW_String dbResult;
	BOOL ret = Exec(qry, dbResult, nResult, TRUE);
	if (!ret)
	{
		return FALSE;
	}

	sResult = (LPCSTR)dbResult.GetData();

//	GetParseData(sResult); // S 
//	GetParseData(sResult); // return code 
	
	return (SP_SUCCESS_RESULT == nResult);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// NFChar
BOOL CNFDBManager::SelectNFBasicChar(TMapNFBasicChar& mapNFBasicChar)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_ALL_BASIC_CHAR.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		NFBasicChar* pNFBasicChar = new NFBasicChar;
		if (pNFBasicChar)
		{
			pNFBasicChar->m_lIndex = atoi(GetParseData(strResult).c_str());
			pNFBasicChar->m_strCharName = GetParseData(strResult).c_str();
			pNFBasicChar->m_nfAbility.m_dFlyDist = atof(GetParseData(strResult).c_str());
			pNFBasicChar->m_nfAbility.m_dCharm = atof(GetParseData(strResult).c_str());
			pNFBasicChar->m_nfAbility.m_dStrength = atof(GetParseData(strResult).c_str());
			pNFBasicChar->m_nfAbility.m_dAgility = atof(GetParseData(strResult).c_str());
			pNFBasicChar->m_nfAbility.m_dControl = atof(GetParseData(strResult).c_str());
			pNFBasicChar->m_nfAbility.m_dHealth = atof(GetParseData(strResult).c_str());
			pNFBasicChar->m_nfAbility.m_dFishPoint = atof(GetParseData(strResult).c_str());
			pNFBasicChar->m_nfAbility.m_dLuckyPoint = atof(GetParseData(strResult).c_str());
			pNFBasicChar->m_lRodItemCode = atoi(GetParseData(strResult).c_str());
			pNFBasicChar->m_lReelItemCode = atoi(GetParseData(strResult).c_str());
			pNFBasicChar->m_lLineItemCode = atoi(GetParseData(strResult).c_str());
			pNFBasicChar->m_lLureItemCode = atoi(GetParseData(strResult).c_str());

			mapNFBasicChar[pNFBasicChar->m_lIndex] = pNFBasicChar;
		}

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFCharBaseNExteriorInfo(NFCharInfoExt& nfCharInfoExt, LONG lGSN, LONG lCSN)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_CHAR.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	// CHAR_TYPE, NICK, LNICK, REG_DATE, LAST_LOGON_DATE, LAST_LOGOUT_DATE, TUTORIAL_DATE, STUDY_SCENARIO_DATE, 
	// EXP, LEV, MONEY, FAMENAME, GRADE, HAIRCOLOR, HAIRSTYLE, FACE, JACKET, PANTS, 
	// SHORTCUT0, SHORTCUT1, SHORTCUT2, SHORTCUT3, SHORTCUT4, SHORTCUT5, SHORTCUT6, SHORTCUT7, SHORTCUT8, SHORTCUT9

	nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN = lCSN;

	nfCharInfoExt.m_nfCharExteriorInfo.m_lBasicCharSRL = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharBaseInfo.m_strCharName = GetParseData(strResult).c_str();
	GetParseData(strResult).c_str();		// LNICK
	GetParseData(strResult).c_str();		// REG_DATE
	nfCharInfoExt.m_nfCharBaseInfo.m_strLastestLoginDate = GetParseData(strResult);
	nfCharInfoExt.m_nfCharBaseInfo.m_strLastestLogOutDate = GetParseData(strResult);
	nfCharInfoExt.m_nfCharBaseInfo.m_strTutorialDate = GetParseData(strResult);
	nfCharInfoExt.m_nfCharBaseInfo.m_strStudyDate = GetParseData(strResult);

	nfCharInfoExt.m_nfCharBaseInfo.m_lExp = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharBaseInfo.m_lLevel = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharBaseInfo.m_llMoney = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharBaseInfo.m_strFameName = GetParseData(strResult).c_str();
	nfCharInfoExt.m_nfCharBaseInfo.m_lGrade = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharExteriorInfo.m_lHairColor = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharExteriorInfo.m_lHairStyle = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharExteriorInfo.m_lFace = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharExteriorInfo.m_lDefaultJacket = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_nfCharExteriorInfo.m_lDefaultPants = atoi(GetParseData(strResult).c_str());

	nfCharInfoExt.m_nfQuickSlot.clear();
	for(int i=0; i<G_MAX_QUICK_SLOT_SIZE; i++)		nfCharInfoExt.m_nfQuickSlot.push_back(atoi(GetParseData(strResult).c_str()));

	// 쓰레기값 방지
	nfCharInfoExt.m_nfAbility.Clear();
	nfCharInfoExt.m_nfCharAchievement.Clear();
	nfCharInfoExt.m_nfCharInven.Clear();
	nfCharInfoExt.m_nfCheckDebuff.clear();

	return TRUE;
}

BOOL CNFDBManager::SelectNFCharGSN(const string& strSiteUserID, long& lGSN)
{	
	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_USER_GSN.c_str(), strSiteUserID.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret) return FALSE;
	if (!HasRow(strResult)) return TRUE;

	lGSN = atoi(GetParseData(strResult).c_str());
	return TRUE;
}

// gsn -> all csn list
BOOL CNFDBManager::SelectAllNFCharCSNByGSN(std::list<LONG>& lstCSN, LONG lGSN, LONG& lErr)
{
	// get all csn
	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_CHAR_CSN_ALL.c_str(), lGSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret) return FALSE;
	
	if (!HasRow(strResult))
		return TRUE;

	while(true)
	{
		lstCSN.push_back( atoi(GetParseData(strResult).c_str()) );
		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFAbilityByCSN(NFCharInfoExt* pNFCharInfo, long lCSN)
{
	if (NULL == pNFCharInfo)
		return FALSE;

	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_ABILITY_SUM.c_str(), lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	pNFCharInfo->m_nfAbility.m_dFlyDist = atof(GetParseData(strResult).c_str());
	pNFCharInfo->m_nfAbility.m_dCharm = atof(GetParseData(strResult).c_str());
	pNFCharInfo->m_nfAbility.m_dStrength = atof(GetParseData(strResult).c_str());
	pNFCharInfo->m_nfAbility.m_dAgility = atof(GetParseData(strResult).c_str());
	pNFCharInfo->m_nfAbility.m_dControl = atof(GetParseData(strResult).c_str());
	pNFCharInfo->m_nfAbility.m_dHealth = atof(GetParseData(strResult).c_str());
	pNFCharInfo->m_nfAbility.m_dFishPoint = atof(GetParseData(strResult).c_str());
	pNFCharInfo->m_nfAbility.m_dLuckyPoint = atof(GetParseData(strResult).c_str());

	return TRUE;
}

// 현재 사용하고 있는 아이템의 능력치를 인벤토리_유즈에서 읽어온다.
// 기존 능력치로 업데이트 되기 때문에, 이 함수로 능력치를 읽어 왔을 경우 꼭, 캐릭터 기본 능력치를 다시 더해야 한다..
BOOL CNFDBManager::SelectNFAbilityExtByCSN(NFAbility& nfCharInfo, NFAbilityExt& nfCharInfoExt, long lCSN)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_ABILITYEXT_SUM.c_str(), lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	nfCharInfo.m_dFlyDist = atof(GetParseData(strResult).c_str());
	nfCharInfo.m_dCharm = atof(GetParseData(strResult).c_str());
	nfCharInfo.m_dStrength = atof(GetParseData(strResult).c_str());
	nfCharInfo.m_dAgility = atof(GetParseData(strResult).c_str());
	nfCharInfo.m_dControl = atof(GetParseData(strResult).c_str());
	nfCharInfo.m_dHealth = atof(GetParseData(strResult).c_str());
	nfCharInfo.m_dFishPoint = atof(GetParseData(strResult).c_str());
	nfCharInfo.m_dLuckyPoint = atof(GetParseData(strResult).c_str());

	// ext 추가
	nfCharInfoExt.m_dKeepLuckyFlag = atof(GetParseData(strResult).c_str());
	nfCharInfoExt.m_dCastingScore = atof(GetParseData(strResult).c_str());
	nfCharInfoExt.m_dCastingBacklashRate = atof(GetParseData(strResult).c_str());
	nfCharInfoExt.m_lActionIncountScore = atoi(GetParseData(strResult).c_str());
	nfCharInfoExt.m_dSpecialFishBite = atof(GetParseData(strResult).c_str());
	nfCharInfoExt.m_dAddRateExp = atof(GetParseData(strResult).c_str());
	nfCharInfoExt.m_dAddRateGameMoney = atof(GetParseData(strResult).c_str());

	return TRUE;
}

BOOL CNFDBManager::ExistNFCharNick(const string& strNick, int& nErr)
{
	std::string		strQuery;

	strQuery = format(QRY_SELECT_NICK_CHECK.c_str(), strNick.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
		return FALSE;

	LONG lCharCNT = atoi(GetParseData(strResult).c_str());
	if (lCharCNT > 0)
		nErr = NF::EC_CNC_NICKNAME_ALREADY;
	return TRUE;
}

BOOL CNFDBManager::InsertNFUser(const string& strSiteCode, const string& strSiteID, const string& strPWD, LONG lAdminLEV, LONG& lGSN)
{
	std::string		strQuery;

	strQuery = format(PKG_INSERT_NF_USER.c_str(), strSiteCode.c_str(), strSiteID.c_str(), lAdminLEV);

	std::string		strResult;
	int		nErrorCode = 0;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	lGSN = atoi(GetParseData(strResult).c_str());
	return TRUE;
}

BOOL CNFDBManager::InsertNFChar(LONG lGSN, NFCharInfo& nfCharInfoExt, int& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_INSERT_NF_CHAR.c_str(), 
		nfCharInfoExt.m_nfCharBaseInfo.m_strCharName.c_str(), lGSN, nfCharInfoExt.m_nfCharExteriorInfo.m_lBasicCharSRL, 
		nfCharInfoExt.m_nfCharBaseInfo.m_strCharName.c_str(), nfCharInfoExt.m_nfCharExteriorInfo.m_lHairColor, 
		nfCharInfoExt.m_nfCharExteriorInfo.m_lHairStyle, nfCharInfoExt.m_nfCharExteriorInfo.m_lFace, 
		nfCharInfoExt.m_nfCharExteriorInfo.m_lDefaultJacket, nfCharInfoExt.m_nfCharExteriorInfo.m_lDefaultPants);

	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN = atoi(GetParseData(strResult).c_str());
	return TRUE;
}

BOOL CNFDBManager::DeleteNFChar(LONG lGSN, LONG lCSN, int& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_DROP_NF_CHAR.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret)
		return FALSE;

	return TRUE;
}

// 물고기 한마리 잡았을 경우만 호출 된다..
BOOL CNFDBManager::UpdateLandingFish(GameResult& result, NFRoomOption& roomOption, LONG lLengthIndex, LONG lClassIndex, const string& strFishGroup, const string& strLength, const string& strRegDate, const string& strMapID, long& lErrorCode)
{
	LandingFish land_fish;
	land_fish.Clear();

	if (result.m_lstLandingFish.size() != 1) {
		lErrorCode = NF::G_NF_ERR_DB_LOCKED_NOTE_LAND_FISH;
		return FALSE;
	}

	ForEachElmt(TLstLandingFish, result.m_lstLandingFish, it, ij)
	{
		land_fish = (*it);
		break;
	}

	std::string		strQuery;
	strQuery = format(PKG_LANDING_FISH.c_str(), 
		result.m_lGSN, result.m_lCSN, result.m_strName.c_str(), result.m_lExp, result.m_lTotExp+result.m_lBonusExp, result.m_lLevel, result.m_llTotMoney, 
		result.m_strStartDate.c_str(), result.m_strLandingDate.c_str(), roomOption.m_lPlayType, land_fish.m_lFishID, land_fish.m_lGroupID, 
		land_fish.m_dResultSize, land_fish.m_dResultWeight, land_fish.m_dResultWeight, land_fish.m_lResultScore, land_fish.m_lClass, roomOption.m_lIdxFishMap,
		lLengthIndex, lClassIndex, "Y", "Y", "Y", strFishGroup.c_str(), strLength.c_str(), strRegDate.c_str(), strMapID.c_str());

	int nErrorCode = 0;
	std::string		strResult;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret)
		return FALSE;

	nErrorCode = lErrorCode;

	return TRUE;
}

// 앞으로 대전 모드에서만 사용하게 될 예정...(DB에서 수정해주면 작업 해야 함... 2012/3/15-bback99)
BOOL CNFDBManager::UpdateNFCharDataByCSN(GameResult& result, NFRoomOption& roomOption, long& lErrorCode)
{
	string strFishResult;

	//if (result.m_lstLandingFish.size() > 0)
	//{
	//	int nCnt = 1;
	//	// 물고기로그 생성
	//	TLstLandingFish::iterator iter = result.m_lstLandingFish.begin();
	//	for(iter; iter != result.m_lstLandingFish.end(); iter++)
	//	{
	//		LandingFish fish = (*iter);

	//		char szTemp[255] = {0};
	//		sprintf(szTemp, "%s,%d,%.2f,%.2f,%d", "fish", fish.m_lFishIndex, fish.m_dResultSize, fish.m_dResultWeight, fish.m_lExp);
	//		strFishResult += szTemp;

	//		if ((int)result.m_lstLandingFish.size() >= ++nCnt)
	//			strFishResult += '_';
	//	}	
	//}
	//else
	//	strFishResult += "''";

	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	string strDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	std::string		strQuery;

	if (strFishResult.size() <= 1000)
	{
		strQuery = format(PKG_NF_GAME_UPDATEDATA_CSN.c_str(), result.m_lGSN,  result.m_lCSN, result.m_lExp, result.m_lLevel, result.m_llTotMoney, 
			strDate.c_str(), roomOption.m_lLimitTimeType*60, roomOption.m_lPlayType, roomOption.m_lWinCondition, result.m_lRank, strFishResult.c_str());
	}
	else
	{
		strQuery = format_big(PKG_NF_GAME_UPDATEDATA_CSN.c_str(), result.m_lGSN,  result.m_lCSN, result.m_lExp, result.m_lLevel, result.m_llTotMoney, 
			strDate.c_str(), roomOption.m_lLimitTimeType*60, roomOption.m_lPlayType, roomOption.m_lWinCondition, result.m_lRank, strFishResult.c_str());
	}

	std::string		strResult;
	int nErrorCode = 0;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret)
		return FALSE;

	lErrorCode = nErrorCode;

	return TRUE;
}

BOOL CNFDBManager::UpdateQuickSlotByCSN(long lGSN, long lCSN, long lType, long lRegistSlotIndex, long lRegistlItemCode, long lRemoveSlotIndex, long lRemovelItemCode, string& strStartDate, string& strEndDate, long& lErrorCode)
{
	if (lType == CS_REMOVE) {	// remove DB에서는 스왑할때 빼고는 remove를 사용하지 않는다...
		lRegistSlotIndex = lRemoveSlotIndex;
		lRegistlItemCode = lRemovelItemCode;
		lRemoveSlotIndex = 0;
		lRemovelItemCode = 0;
	}

	std::string		strQuery;
	strQuery = format(PKG_NF_GAME_QUICKSLOT_CSN.c_str(), lGSN, lCSN, lType, lRegistlItemCode, lRegistSlotIndex, lRemovelItemCode, lRemoveSlotIndex);

	std::string		strResult;
	int nErrorCode = 0;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	strStartDate = GetParseData(strResult).c_str();
	strEndDate = GetParseData(strResult).c_str();
	lErrorCode = nErrorCode;

	return TRUE;
}

BOOL CNFDBManager::UpdateLastLogOutDate(LONG lGSN, LONG lCSN)
{
	// 날짜 업데이트
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	string	strLogOutDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	std::string		strQuery;
	strQuery = format(QRY_UPDATE_LOGOUT_DATE.c_str(), strLogOutDate.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
		return FALSE;
	return TRUE;
}


// NFChar-Inven
// NCS에서 사용, Cloth중에 bIsUsing이 TRUE인 경우만, 읽어와서 NFInvenSlot에 추가한다.
BOOL CNFDBManager::SelectNFSimpleInven(NFCharInven& nfCharInven, long lGSN, long lCSN, LONG& lErr)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_CHAR_SIMPLE_INVEN.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		NFInvenSlot inven;
		inven.Clear();

		// 아이템이 중복되거나, 나중에 필요할 경우에 TempIndex 추가		
		// ITEM_CODE, INVEN_SRL, PARTS FROM
		inven.m_lItemCode	= atoi(GetParseData(strResult).c_str());
		inven.m_lInvenSRL	= atoi(GetParseData(strResult).c_str());
		inven.m_lPartsIndex = atoi(GetParseData(strResult).c_str());
		inven.m_bIsUsing	= TRUE;				// IS_USED가 Y인놈만 가지고 왔기 때문에 bIsUSING을 항당 TRUE로 설정한다.

		lErr = theNFItem.AddInvenSlotItem(nfCharInven, inven);

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFCharInven(TlstInvenSlot& lstRemovedItem, const string& strLogoutDate, NFCharInven& nfCharInven, long lGSN, long lCSN, LONG& lErr)
{
	string strTempDate = strLogoutDate;
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	string	strCurrentDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	time_t time_current = 0;
	ConvertNFDateString2time_t(strCurrentDate, time_current);

	if (strTempDate.size() <= 0)
		strTempDate = strCurrentDate;

	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_INVEN_CSN.c_str(), strTempDate.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		NFInvenSlot inven;
		inven.Clear();

		// INVEN_SRL, ITEM_CODE, EFF_START_DATE, EFF_END_DATE, GAUGE, PARTS, EQUIP_YN, PACK_YN, PERIOD, ENCHANT_LEVEL
		inven.m_lInvenSRL		= atoi(GetParseData(strResult).c_str());
		inven.m_lItemCode		= atoi(GetParseData(strResult).c_str());
		inven.m_strBuyDate		= GetParseData(strResult).c_str();
		inven.m_strExpireDate	= GetParseData(strResult).c_str();
        inven.m_lRemainCount	= atoi(GetParseData(strResult).c_str());
		inven.m_lPartsIndex		= atoi(GetParseData(strResult).c_str());

		// Using
		string strNO("N");
		if (GetParseData(strResult).c_str() == strNO)
			inven.m_bIsUsing = FALSE;
		else
			inven.m_bIsUsing = TRUE;

		// PackOpen
		string strPackNO("N");
		if (GetParseData(strResult).c_str() == strPackNO)
			inven.m_bIsPackOpen = FALSE;
		else
			inven.m_bIsPackOpen = TRUE;

		// Period
		GetParseData(strResult);
		
		// Enchant Level
		inven.m_lEnchantLevel = atoi(GetParseData(strResult).c_str());

		// inven에 넣을지, removedItem에 넣을지...
		if (inven.m_strExpireDate == G_MAX_DATE && inven.m_strBuyDate != G_INVALID_DATE)		// 기간제가 아닌 아이템 조건
		{
			if (inven.m_lRemainCount <= 0)
			{
				if (!theNFDataItemMgr.CheckItemTypeByItemCode("E", inven.m_lItemCode, inven.m_lPartsIndex))
					lstRemovedItem.push_back(inven);
			}
			
			// 나중에 뺄꺼다..(유효성 체크를 해야 하므로 일단 넣어둔다...)
			lErr = theNFItem.AddInvenSlotItem(nfCharInven, inven);			
		}
		else		// 기간제 아이템
		{
			if (inven.m_strBuyDate == G_INVALID_DATE)		// 사용안한 기간제 아이템 체크
				lErr = theNFItem.AddInvenSlotItem(nfCharInven, inven);
			else
			{
				time_t time_inven = 0;
				ConvertNFDateString2time_t(inven.m_strExpireDate, time_inven);

				if ((time_inven - time_current) > 0)
					lErr = theNFItem.AddInvenSlotItem(nfCharInven, inven);
				else
					lstRemovedItem.push_back(inven);
			}
		}

		if (TRUE == inven.m_bIsUsing)
			// if item used, add using map
			theNFItem.AddUsingInven(nfCharInven.m_mapUsingItem, inven);

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

// Level
BOOL CNFDBManager::SelectNFLevel(TMapNFLevel& mapNFLevel)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_ALL_LEVEL.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		NFLevel* pNFLevel = new NFLevel;
		if (pNFLevel)
		{
			// LEV, NEEDEXP, GETLIMITEXP, FLYDIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISHPOINT, LUCKYPOINT, 
			pNFLevel->m_lLevel = atoi(GetParseData(strResult).c_str());
			pNFLevel->m_lNeedExp = atoi(GetParseData(strResult).c_str());
			pNFLevel->m_lGetLimitExp = atoi(GetParseData(strResult).c_str());

			pNFLevel->m_ability.m_dFlyDist = atof(GetParseData(strResult).c_str());
			pNFLevel->m_ability.m_dCharm = atof(GetParseData(strResult).c_str());
			pNFLevel->m_ability.m_dStrength = atof(GetParseData(strResult).c_str());
			pNFLevel->m_ability.m_dAgility = atof(GetParseData(strResult).c_str());
			pNFLevel->m_ability.m_dControl = atof(GetParseData(strResult).c_str());
			pNFLevel->m_ability.m_dHealth = atof(GetParseData(strResult).c_str());
			pNFLevel->m_ability.m_dFishPoint = atof(GetParseData(strResult).c_str());
			pNFLevel->m_ability.m_dLuckyPoint = atof(GetParseData(strResult).c_str());

			mapNFLevel[pNFLevel->m_lLevel] = pNFLevel;
		}

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

bool SortItemCNT(const Product& elem1, const Product& elem2)
{
	return elem1.m_lItemCNT < elem2.m_lItemCNT;		//오름차순 정렬 
}


// Product
BOOL CNFDBManager::SelectNFProduct(TMapItemCodeProduct& mapProduct, TMapItemCodeProduct& mapProductSkill)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_PRODUCT.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		Product product; product.Clear();
		// SELECT ITEM_CODE, ITEM_ID, ITEM_GRP, SALEINFO_ID, REG_DATE, END_DATE, SALE_TYPE, PRICE, SALE_STATUS, ITEM_TYPE, 
		// USE_TYPE, GAUGE, PERIOD, PACK_YN, ACTIVATION_YN, NAME, HIT_YN, GAUGE_MONEY FROM NF_BT_ITEM WHERE SALE_STATUS = 'Y'

		product.Clear();
		product.m_lItemCode = atoi(GetParseData(strResult).c_str());
		product.m_lItemID = atoi(GetParseData(strResult).c_str());
		product.m_strCategory = GetParseData(strResult).c_str();
		product.m_strProductSRL = GetParseData(strResult).c_str();
		product.m_strRegDate = GetParseData(strResult).c_str();
		GetParseData(strResult).c_str();		// END_DATE
		product.m_strPriceType = GetParseData(strResult).c_str();
		product.m_lPrice = _atoi64(GetParseData(strResult).c_str());
		GetParseData(strResult).c_str();		//SALE_STATUS
		product.m_strItemType = GetParseData(strResult).c_str();
		std::string strUseType = GetParseData(strResult).c_str();
		LONG lCNT = atoi(GetParseData(strResult).c_str());
		LONG lPeriod = atoi(GetParseData(strResult).c_str());
		if (strUseType == "P")
			product.m_lItemCNT = lPeriod;
		else	// 횟수제, 내구도제도??
			product.m_lItemCNT = lCNT;
		GetParseData(strResult).c_str();		// PACK_YN;
		GetParseData(strResult).c_str();		// ACTIVATION_YN
		product.m_strTitle = GetParseData(strResult).c_str();
		product.m_strIsHIT = GetParseData(strResult).c_str();
		product.m_lGetGameMoney = _atoi64(GetParseData(strResult).c_str());

		// 기존에 있는지 검색
		if (product.m_strCategory == "S") {
			TMapItemCodeProduct::iterator iter = mapProductSkill.find(product.m_lItemCode);
			if (iter != mapProductSkill.end()) {
				(*iter).second.push_back(product);
				(*iter).second.sort(SortItemCNT);
			}
			else {
				TlstProduct	lst;
				lst.push_back(product);
				mapProductSkill.insert(make_pair(product.m_lItemCode, lst));
			}
		}
		else {
			TMapItemCodeProduct::iterator iter = mapProduct.find(product.m_lItemCode);
			if (iter != mapProduct.end()) {
				(*iter).second.push_back(product);
				(*iter).second.sort(SortItemCNT);
			}
			else {
				TlstProduct	lst;
				lst.push_back(product);
				mapProduct.insert(make_pair(product.m_lItemCode, lst));
			}
		}

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFProductPackage(TMapIndexPackage& mapPackage)
{
	std::string		strQuery;
	//	strQuery = format(QRY_SELECT_PRODUCT_PACKAGE.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		LONG lPackageSRL = atoi(GetParseData(strResult).c_str());
		LONG lProductSRL = atoi(GetParseData(strResult).c_str());

		TMapIndexPackage::iterator iter = mapPackage.find(lPackageSRL);
		if (iter == mapPackage.end())
		{
			std::list<LONG>		lstProductSRL;
			lstProductSRL.push_back(lProductSRL);
			mapPackage[lPackageSRL] = lstProductSRL;
		}
		else
			(*iter).second.push_back(lProductSRL);

		if(strResult.size() <= 0)
			break;
	}

	return TRUE;
}

BOOL CNFDBManager::BuyItemFromShop(LONG lBuyItemType, LONG lGSN, LONG lCSN, LONG lItemID, const string& strProductSRL, LONG& lInvenSRL, LONGLONG& llMoney, int& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_SHOP_BUY_PRODUCT.c_str(), lBuyItemType, lGSN, lCSN, lItemID, lInvenSRL);

	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	lInvenSRL	= atoi(GetParseData(strResult).c_str());
	llMoney		= _atoi64(GetParseData(strResult).c_str());

	return TRUE;
}

BOOL CNFDBManager::BuyItemFromShop_Admin(LONG lBuyItemType, const CardItem* pItem, LONG lGSN, LONG lCSN, LONG lItemID, const string& strProductSRL, LONG& lInvenSRL, const string& strPackYN, int& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_SHOP_BUY_PRODUCT_ADMIN.c_str(), lBuyItemType, lItemID, lGSN, lCSN, strProductSRL.c_str(), strPackYN.c_str(), lInvenSRL);

	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	lInvenSRL = atoi(GetParseData(strResult).c_str());

	return TRUE;
}


// Item
BOOL CNFDBManager::InsertBuyItem(LONG lBuyItemType, LONG lItemID, LONG lGSN, LONG lCSN, NFInvenSlot& inven, int& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_GIVE_GAMEITEM.c_str(), lBuyItemType, lItemID, lGSN, lCSN, inven.m_lInvenSRL);

	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	inven.m_lInvenSRL = atoi(GetParseData(strResult).c_str());

	return TRUE;
}

BOOL CNFDBManager::SelectNFGiveItem(TMapItemIDProduct& mapGiveItem)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_GIVE_ITEM.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		Product* pProduct = new Product;
		if (pProduct)
		{
			//ITEM_CODE, ITEM_ID, ITEM_GRP, REG_DATE, END_DATE, USE_TYPE, GAUGE, PACK_YN, PARTS
			pProduct->Clear();
			pProduct->m_lItemCode = atoi(GetParseData(strResult).c_str());
			pProduct->m_lItemID = atoi(GetParseData(strResult).c_str());
			pProduct->m_strCategory = GetParseData(strResult).c_str();
			pProduct->m_strRegDate = GetParseData(strResult).c_str();
			GetParseData(strResult).c_str();		// option.m_strEndDate = 
			std::string strUseType = GetParseData(strResult).c_str();
			LONG lCNT = atoi(GetParseData(strResult).c_str());
			LONG lPeriod = atoi(GetParseData(strResult).c_str());
			if (strUseType == "P")
				pProduct->m_lItemCNT = lPeriod;
			else	// 횟수제, 내구도제도??
				pProduct->m_lItemCNT = lCNT;
			GetParseData(strResult).c_str();		// PACK_YN
			pProduct->m_strItemType = GetParseData(strResult).c_str();
			mapGiveItem.insert(make_pair(pProduct->m_lItemID, pProduct));
		}

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::UpdateRemoveItem(LONG lGSN, LONG lCSN, LONG lInvenSRL)
{
	std::string		strQuery;
	strQuery = format(QRY_DELETE_GAMEITEM_INVEN.c_str(), lGSN, lCSN, lInvenSRL);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
		return FALSE;
	return TRUE;
}

BOOL CNFDBManager::UpdatePartsByCSN(NFInvenSlot& oldInven, NFInvenSlot& newInven, long lGSN, long lCSN, long& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_NF_GAME_UPDATE_PARTS.c_str(), lGSN, lCSN, oldInven.m_lInvenSRL, newInven.m_lInvenSRL);

	std::string		strResult;
	int		nErrorCode = 0;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	newInven.m_strBuyDate = GetParseData(strResult).c_str();
	newInven.m_strExpireDate = GetParseData(strResult).c_str();
	lErrorCode = nErrorCode;

	return TRUE;
}

BOOL CNFDBManager::UseGameFunc(LONG lGSN, LONG lCSN, NFInvenSlot& updateItem, const std::string& strDelYN, LONG& lGauge, long& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_NF_GAME_USE_GAMEITEM.c_str(),
		lCSN, lGSN, updateItem.m_lItemCode, updateItem.m_lInvenSRL, strDelYN.c_str(), lGauge);

	std::string		strResult;
	int		nErrorCode = 0;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	lGauge = atoi(GetParseData(strResult).c_str());
	lErrorCode = nErrorCode;

	return TRUE;
}

BOOL CNFDBManager::RechargeGameFunc(LONG lGSN, LONG lCSN, const NFInvenSlot& inven, LONG& lAddGauge, LONGLONG& llMoney, LONG& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_NF_GAME_RECHARGE_GAMEITEM.c_str(),
		lCSN, lGSN, inven.m_lItemCode, inven.m_lInvenSRL, lAddGauge, llMoney);

	std::string		strResult;
	int		nErrorCode = 0;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	lAddGauge = atoi(GetParseData(strResult).c_str());
	llMoney = _atoi64(GetParseData(strResult).c_str());
	lErrorCode = nErrorCode;

	return TRUE;
}

BOOL CNFDBManager::EnchantItem(LONG lGSN, LONG lCSN, LONG lItemCode, LONG lInvenSRL, LONG lCardItemCode, LONG lCardInvenSRL, LONG lSubItemCode, LONG lSubInvenSRL, LONG lType, LONG& lErrorCode, LONG& lCardGauge, LONG& lSubGauge)
{
	std::string		strQuery;
	strQuery = format(PKG_NF_GAMEITEM_ENCHANT_ITEM.c_str(),
		lGSN, lCSN, lItemCode, lInvenSRL, lCardItemCode, lCardInvenSRL, lSubItemCode, lSubInvenSRL, lType);

	std::string strResult;
	int nErrorCode = 0;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	lErrorCode = nErrorCode;
	lCardGauge = atoi(GetParseData(strResult).c_str());
	lSubGauge = atoi(GetParseData(strResult).c_str());

	return TRUE;
}

// Item-Card
BOOL CNFDBManager::UpdateOpenCardPack(LONG lGSN, LONG lCSN, LONG lInvenSRL, LONG lChangedItemCode, int& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_OPEN_CARDPACK.c_str(), lGSN, lCSN, lInvenSRL, lChangedItemCode);

	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret)
		return FALSE;

	return TRUE;
}

class  stTemp {
public:
	LONG	m_lItemCode;
	LONG	m_lInvenSRL;
};

typedef std::vector<stTemp>	TvecTemp;

BOOL CNFDBManager::UpdateUpgrageCard(LONG lGSN, LONG lCSN, map<LONG, LONG>& mapUpgradeCardInven, NFInvenSlot& invenUpgradeCard, NFInvenSlot& getCardPack, int& lErrorCode)
{
	TvecTemp	vecTemp;
	for(map<LONG, LONG>::iterator it = mapUpgradeCardInven.begin(); it != mapUpgradeCardInven.end(); it++)
	{
		stTemp	temp;
		temp.m_lItemCode = (*it).first;
		temp.m_lInvenSRL = (*it).second;
		vecTemp.push_back(temp);
	}

	std::string strCardPackOpen = "N";
	if (getCardPack.m_bIsPackOpen == TRUE)
		strCardPackOpen = "Y";

	std::string		strQuery;
	strQuery = format(PKG_UPGRADE_CARD.c_str(), 
		lGSN, lCSN, 
		vecTemp[0].m_lItemCode, vecTemp[0].m_lInvenSRL,
		vecTemp[1].m_lItemCode, vecTemp[1].m_lInvenSRL,
		vecTemp[2].m_lItemCode, vecTemp[2].m_lInvenSRL,
		vecTemp[3].m_lItemCode, vecTemp[3].m_lInvenSRL,
		vecTemp[4].m_lItemCode, vecTemp[4].m_lInvenSRL,
		vecTemp[5].m_lItemCode, vecTemp[5].m_lInvenSRL,
		vecTemp[6].m_lItemCode, vecTemp[6].m_lInvenSRL,
		invenUpgradeCard.m_lItemCode, invenUpgradeCard.m_lInvenSRL, 
		strCardPackOpen.c_str());

	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	getCardPack.m_lInvenSRL	= atoi(GetParseData(strResult).c_str());

	return TRUE;
}

BOOL CNFDBManager::UpdateCardPartsByCSN(NFInvenSlot& oldInven, NFInvenSlot& newInven, LONGLONG& lMoney, long lGSN, long lCSN, long& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(PKG_NF_GAME_UPDATE_PARTS_CARD.c_str(), lGSN, lCSN, lMoney, oldInven.m_lInvenSRL, oldInven.m_lPartsIndex, newInven.m_lInvenSRL, newInven.m_lPartsIndex);

	std::string		strResult;
	int		nErrorCode = 0;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	lErrorCode	= nErrorCode;
	lMoney		= _atoi64(GetParseData(strResult).c_str());

	return TRUE;
}


// Tutorial and Scenario
BOOL CNFDBManager::UpdateTutorialDate(LONG lGSN, LONG lCSN, const string& strTutorialDate)
{
	std::string		strQuery;
	strQuery = format(QRY_UPDATE_TUTORIAL_DATE.c_str(), strTutorialDate.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
		return FALSE;
	return TRUE;
}

BOOL CNFDBManager::UpdateStudyScenarioDate(LONG lGSN, LONG lCSN, const string& strStudyScenarioDate)
{
	std::string		strQuery;
	strQuery = format(QRY_UPDATE_STUDY_SCENARIO_DATE.c_str(), strStudyScenarioDate.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
		return FALSE;
	return TRUE;
}


// ACHV
BOOL CNFDBManager::SelectNFAchvList(LONG lGSN, LONG lCSN, TMapAchievement& mapAchv, LONG& lErrorCode)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_CHAR_ACHV.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	// ACHV_ID, PROGRESS, COMPLETE_DATE, GET_REWARD_DATE, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4, LAST_UPDATE_DATE, REMARK FROM NF_GT_CHAR_ACHV WHERE GSN=? AND CSN=?
	while(true)
	{
		Achievement		achv;
		achv.Clear();

		achv.m_lAchvID			= atoi(GetParseData(strResult).c_str());
		achv.m_dGauge			= atof(GetParseData(strResult).c_str());
		achv.m_strCompleteDate	= GetParseData(strResult).c_str();
		achv.m_strRewardDate	= GetParseData(strResult).c_str();
		achv.m_lItemID1			= atoi(GetParseData(strResult).c_str());
		achv.m_lItemID2			= atoi(GetParseData(strResult).c_str());
		achv.m_lItemID3			= atoi(GetParseData(strResult).c_str());
		achv.m_lItemID4			= atoi(GetParseData(strResult).c_str());
		achv.m_strLastUpdateDate = GetParseData(strResult).c_str();
		GetParseData(strResult);			// remark

		mapAchv.insert( make_pair (achv.m_lAchvID, achv ) );

		if(strResult.size() <= 0)
			break;
	}

	return TRUE;
}

BOOL CNFDBManager::SelectNFAchvPoint(LONG lGSN, LONG lCSN, AchievementPoint& ap)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_CHAR_ACHV_POINT.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret) return FALSE;

	while(true)
	{
		string strCategory = GetParseData(strResult).c_str();
		if (strCategory == "G")
			ap.m_lCommunityAP = atoi(GetParseData(strResult).c_str());
		else if (strCategory == "F")
			ap.m_lFishingAP		= atoi(GetParseData(strResult).c_str());
		else if (strCategory == "I")
			ap.m_lItemAP		= atoi(GetParseData(strResult).c_str());
		else if (strCategory == "H")
			ap.m_lHistoryAP		= atoi(GetParseData(strResult).c_str());
		else if (strCategory == "P")
			ap.m_lPlayAP		= atoi(GetParseData(strResult).c_str());
		else if (strCategory == "E")
			ap.m_lEventAP		= atoi(GetParseData(strResult).c_str());
		else if (strCategory == "C")
			ap.m_lCommunityAP	= atoi(GetParseData(strResult).c_str());

		if(strResult.size() <= 0)
			break;
	}

	ap.m_lTotalAP = ap.m_lCommunityAP + ap.m_lEventAP + ap.m_lFishingAP + ap.m_lGrowthAP + ap.m_lHistoryAP + ap.m_lItemAP + ap.m_lPlayAP;
	return TRUE;
}

BOOL CNFDBManager::SelectAchvReward( TMapAchvReward & mapAchvReward )
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_ALL_NF_ACHV_REWARD.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	std::string Buf;
	while(true)
	{
		NFReward reward;

		// ACHV_ID, AP, MONEY, EXP, ITEM_CNT, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4

		reward.achv_ID = atoi( GetParseData(strResult).c_str() );

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.AP = atoi( Buf.c_str() );
		else
			reward.AP = 0;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.money = ::_atoi64( Buf.c_str() );
		else
			reward.money = 0LL;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.exp = atoi( Buf.c_str() );
		else
			reward.exp = 0;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.item_cnt = atoi( Buf.c_str() );
		else
			reward.item_cnt = 0;

		for( int i=0; i<4; i++)
		{
			if( !IsNULLField( Buf = GetParseData(strResult) ) )
				reward.vecItemID.push_back( atoi( Buf.c_str() ) );
			else
				reward.vecItemID.push_back( achv::InvalidRewardItemId ); 
		}

		mapAchvReward.insert( make_pair( (LONG)reward.achv_ID, reward) );

		if(strResult.size() <= 0)
			break;
	}

	return TRUE;
}

void MakeRewardItemIdArray( std::vector<int> & Result, const std::vector<int> & Reward_Item_Ids )
{
	for( int i = 0; i < achv::RewardItemIdCnt; ++i )
		Result.push_back( achv::InvalidRewardItemId );

	for( int i = 0; i < (int)Reward_Item_Ids.size(); ++i )
		Result[i] = Reward_Item_Ids[i];
}

BOOL CNFDBManager::GiveAchvReward(std::vector<int>& vecInvenSRL, LONG lGSN, LONG lCSN, int achv_id, const std::vector<int> & Reward_Item_Ids, std::vector<NFInvenSlot>& vecInven, LONG& lErrorCode)
{
	std::vector<int> Reward_item_Ids_For_Query;;
	MakeRewardItemIdArray( Reward_item_Ids_For_Query, Reward_Item_Ids );

	std::string		strQuery;
	strQuery = format(PKG_NF_ACHV_GIVE_REWARD.c_str(), lGSN, lCSN, achv_id, 
		Reward_item_Ids_For_Query[0], Reward_item_Ids_For_Query[1], Reward_item_Ids_For_Query[2], Reward_item_Ids_For_Query[3],
		vecInvenSRL[0], vecInvenSRL[1], vecInvenSRL[2], vecInvenSRL[3]);

	int nErrorCode;
	std::string		strResult;
	BOOL ret = ExecSP(strQuery, nErrorCode, strResult);
	lErrorCode = nErrorCode;

	if (!ret) return FALSE;
	if (!HasRow(strResult))	return TRUE;

	std::string Buf;

	int nSRL = 0;
	for( int i = 0; i < achv::RewardItemIdCnt; ++i ) 
	{
		nSRL = atoi(GetParseData(strResult).c_str());
		if (nSRL > 0)
			vecInven[i].m_lInvenSRL = nSRL;
	}

	return TRUE;
}

// Community
BOOL CNFDBManager::SelectNFLetterList( const LONG lCSN, CONT_NF_LETTER& rkContNFLetter, const BOOL bNewLetter )
{
	std::string strQuery;

	if( bNewLetter )
	{
		//strQuery = format(QRY_SELECT_NF_LETTER_LIST_ONLY_NOT_READ.c_str(), 200, lCSN, i64LastLetterIndex); // 최근 200개 편지 중, 안 읽은 편지만
		strQuery = format(QRY_SELECT_NF_LETTER_LIST.c_str(), 1, lCSN); // 가장 최근 편지 1개( 새편지 )
	}
	else
	{
		strQuery = format(QRY_SELECT_NF_LETTER_LIST.c_str(), 200, lCSN); // 최근 편지 200개만
	}

	std::string strResult;
	BOOL ret = ExecQry(strQuery, strResult);

	if (!ret )					return FALSE;
	if( !HasRow(strResult) )	return TRUE;

	while( !strResult.empty() )
	{
		CNFLetter nfLetter;
		nfLetter.Clear();

		nfLetter.m_i64LetterIndex = (LONGLONG)atof(GetParseData(strResult).c_str());
		nfLetter.m_strSender = GetParseData(strResult).c_str();
		nfLetter.m_strLetterType = GetParseData(strResult).c_str();
		nfLetter.m_strTitle = GetParseData(strResult).c_str();		
		std::string strTemp = GetParseData(strResult).c_str();
		nfLetter.m_bIsRead = ( strTemp == "Y" ) ? TRUE : FALSE;
		//nfLetter.m_strSendTime = GetParseData(strResult).c_str();
		nfLetter.m_lRemainDay = atoi(GetParseData(strResult).c_str());

		rkContNFLetter.insert( std::make_pair( nfLetter.m_i64LetterIndex, nfLetter ) );
	}

	return TRUE;
}

BOOL CNFDBManager::SelectNFLetterContent( const __int64 i64LetterIndex, string& rstrContent, string& rstrSendTime )
{
	std::string	strQuery = format(QRY_SELECT_NF_LETTER_CONTENTS.c_str(), i64LetterIndex);
	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	rstrContent = GetParseData(strResult).c_str();
	rstrSendTime = GetParseData(strResult).c_str();

	// 편지 읽음 상태로 업데이트.
	strQuery = format(QRY_UPDATE_NF_LETTER_READ.c_str(), i64LetterIndex);
	ret = ExecQry(strQuery, strResult);
	if(!ret)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CNFDBManager::InsertNFLetter( const string& rstrReceiver, const CNFLetter& rnfLetter )
{
	std::string	strQuery = format(QRY_INSERT_NF_LETTER.c_str(), rnfLetter.m_strSender.c_str(), rnfLetter.m_strSender.c_str(), rnfLetter.m_strSender.c_str(), rstrReceiver.c_str(), rstrReceiver.c_str(), rstrReceiver.c_str(), rnfLetter.m_strLetterType.c_str(), rnfLetter.m_strTitle.c_str(), rnfLetter.m_strContent.c_str(), NULL); // 마지막 NULL은 아이템 아이디(일단 NULL)
	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CNFDBManager::DeleteNFLetter( const vector<__int64>& kContLetterIndex )
{
	// 한페이지에서 보여지는 편지 개수 정해지면
	// IN ( ?, ?, ?, ?, ? .... ? ) 으로 쿼리 수정
	vector<__int64>::const_iterator iter = kContLetterIndex.begin();
	while ( kContLetterIndex.end() != iter )
	{
		std::string	strQuery = format(QRY_DELETE_NF_LETTER.c_str(), (*iter));
		std::string	strResult;
		BOOL ret = ExecQry(strQuery, strResult);
		if (!ret)
		{
			return FALSE;
		}

		++iter;
	}

	return TRUE;
}

BOOL CNFDBManager::SelectNFFriendInfo( const LONG lCSN, CONT_NF_FRIEND& rkContNFFriend, const LONG lStatus )
{
	char szTemp[255] = {0,};
	sprintf(szTemp, "%d", lStatus);
	std::string strStatus = szTemp;

	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_FRIEND_LIST.c_str(), lCSN, strStatus.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);

	if (!ret)				return FALSE;
	if (!HasRow(strResult)) return TRUE;	// 친구가 없어도 에러는 아님

	while( !strResult.empty() )
	{
		TKey		key;
		CNFFriend	nfFriend;
		nfFriend.Clear();

		key.m_lMainKey = atoi(GetParseData(strResult).c_str());			// USN
		key.m_lSubKey = atoi(GetParseData(strResult).c_str());			// CSN
		nfFriend.m_strCharName =  GetParseData(strResult).c_str();		// 캐릭터명
		nfFriend.m_lLevel = atoi(GetParseData(strResult).c_str());		// 레벨

		rkContNFFriend.insert( std::make_pair( key, nfFriend ) );
	}

	return TRUE;
}

// 캐릭터 이름으로 USN, CSN을 알아온다.
BOOL CNFDBManager::SelectNFCharKeyByCharName( const string& rstrCharName, TKey& rKey )
{
	std::string	strQuery = format(QRY_SELECT_NF_CHAR_KEY_BY_CHAR_NAME.c_str(), rstrCharName.c_str());
	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	rKey.m_lMainKey = atoi(GetParseData(strResult).c_str());
	rKey.m_lSubKey = atoi(GetParseData(strResult).c_str());

	return TRUE;
}

BOOL CNFDBManager::InsertNFFriend( const LONG lApplicantCSN, const LONG lAcceptorCSN, const LONG lStatus )
{
	char szTemp[255] = {0,};
	sprintf(szTemp, "%d", lStatus);
	std::string strStatus = szTemp;

	std::string strQuery = format( QRY_INSERT_NF_FRIEND.c_str(),
		lApplicantCSN,
		lApplicantCSN,
		strStatus.c_str(),
		lAcceptorCSN,
		lAcceptorCSN);

	std::string strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
	{
		return FALSE;
	}

	return TRUE;
}

// 친구상태로 업데이트.
BOOL CNFDBManager::UpdateNFFriendStatusToFriend( const string& rstrAcceptorCharName, const string& rstrApplicantCharName )
{
	char szTemp[255] = {0,};
	sprintf(szTemp, "%d", FR_FRIEND);
	std::string strStatus = szTemp;

	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	string strRegDate = ::format("%04d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	// 1. 수락자의 친구로 신청자를...
	std::string strQuery = format( QRY_UPDATE_NF_FRIEND.c_str(), strStatus.c_str(), strRegDate.c_str(), rstrAcceptorCharName.c_str(), rstrApplicantCharName.c_str() );
	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
	{
		return FALSE;
	}

	strQuery.clear();
	strResult.clear();

	// 2. 신청자의 친구로 수락자를...
	strQuery = format(QRY_UPDATE_NF_FRIEND.c_str(), strStatus.c_str(), strRegDate.c_str(), rstrApplicantCharName.c_str(), rstrAcceptorCharName.c_str() );
	ret = ExecQry(strQuery, strResult);
	if (!ret)
	{
		// 여기서 실패하면 롤백...
		strResult.clear();
		strQuery.clear();

		// 다시 친구 요청 받은 상태로
		sprintf(szTemp, "%d", FR_FRIEND_RECV);
		strStatus = szTemp;

		std::string strDeleteQuery = format( QRY_UPDATE_NF_FRIEND.c_str(), strStatus.c_str(), strRegDate.c_str(), rstrAcceptorCharName.c_str(), rstrApplicantCharName.c_str() );
		ExecQry(strDeleteQuery, strResult); // 이거 실패하면..? 롤백이 안되는거지..

		return FALSE;
	}

	return TRUE;
}

BOOL CNFDBManager::DeleteNFFriend(const string& rstrCharName, const string& rstrDeleteCharName, const LONG lStatus)
{
	char szTemp[255] = {0,};
	sprintf(szTemp, "%d", lStatus);
	std::string strStatus = szTemp;

	std::string strQuery = format(QRY_DELETE_NF_FRIEND.c_str(), rstrCharName.c_str(), rstrDeleteCharName.c_str(), strStatus.c_str() );
	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
	{
		return FALSE;
	}

	return TRUE;
}

// 상대방과 친구인지? 차단인지?
BOOL CNFDBManager::SelectNFFriendRelation( const LONG lCSN, const LONG lCheckCSN, const LONG lCheckStatus, LONG& rlResultStatus )
{
	char szTemp[255] = {0,};
	sprintf(szTemp, "%d", lCheckStatus);
	std::string strStatus = szTemp;

	std::string strQuery = format(QRY_SELECT_NF_FRIEND_RELATION.c_str(), lCSN, lCheckCSN, strStatus.c_str());
	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if(!ret)
	{
		return FALSE;
	}

	// SELECT 결과가 있으면 친구 or 차단( 체크하려고 했던 관계 임 )
	if( HasRow(strResult) )
	{
		rlResultStatus = lCheckStatus;
	}

	return TRUE;
}

// 블록, 나에게 친구신청한 캐릭터 이름 리스트
BOOL CNFDBManager::SelectNFFriendNickByStatus( const LONG lCSN, CONT_NF_FRIEND_NICK& rkContNFBlock, const LONG lStatus )
{
	char szTemp[255] = {0,};
	sprintf(szTemp, "%d", lStatus);
	std::string strStatus = szTemp;

	std::string strQuery = format(QRY_SELECT_NF_FRIEND_NICK_BY_STATUS.c_str(), strStatus.c_str(), lCSN);
	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);

	if (!ret)				return FALSE;
	if (!HasRow(strResult)) return TRUE;

	while( !strResult.empty() )
	{
		rkContNFBlock.push_back( GetParseData(strResult).c_str() );
	}

	return TRUE;
}

BOOL CNFDBManager::SelectNFLockedNote(LONG lGSN, LONG lCSN, NFLockedNote& lockedNote)
{
	// 최대한 Main->Map->Fish 순서 보장해야 함...
	if (!SelectNFLockedNoteMain(lGSN, lCSN, lockedNote.m_nfLockedNoteMain))
		return FALSE;
	if (!SelectNFLockedNoteMap(lGSN, lCSN, lockedNote.m_nfLockedNoteMap))
		return FALSE;
	if (!SelectNFLockedNoteFish(lGSN, lCSN, lockedNote.m_nfLockedNoteMap))
		return FALSE;
	return TRUE;
}

// LockedNoteMain
BOOL CNFDBManager::SelectNFLockedNoteMain(LONG lGSN, LONG lCSN, NFLockedNoteMain& lockedNoteMain)
{
	std::string		strQuery;
	std::string		strResult;

	// Main
	// FISH_GROUP_CNT, 
	// LENGTH_FISH_GROUP_1, LENGTH_FISH_GROUP_2, LENGTH_BEST_1, LENGTH_BEST_2, LENGTH_REG_DATE_1, LENGTH_REG_DATE_2
	// CLASS_FISH_GROUP_1, CLASS_FISH_GROUP_2, CLASS_BEST_1, CLASS_BEST_2, CLASS_REG_DATE_1, CLASS_REG_DATE_2, CLASS_BEST
	// RECENT_FISH_GROUP, RECENT_LENGTH, RECENT_REG_DATE, RECENT_MAP_ID
	Recently_Landing_Fish main1, main2;
	main1.Clear();
	lockedNoteMain.Clear();
	for(int i=0; i<G_LOCKED_NOTE_MAIN_TOP_CNT; i++)
		lockedNoteMain.m_vecTopLengthFish.push_back(main1);

	for(int i=0; i<G_LOCKED_NOTE_MAIN_TOP_CNT; i++)
		lockedNoteMain.m_vecTopClassFish.push_back(main1);

	for(int i=0; i<G_LOCKED_NOTE_RECENTLY_CNT; i++)
		lockedNoteMain.m_vecRecentlyLandingFish.push_back(main1);

	strQuery = format(QRY_SELECT_GT_CHAR_LOCKEDNOTE_MAIN.c_str(), lGSN, lCSN);
	BOOL ret = ExecQry(strQuery, strResult);

	if (!ret)				return FALSE;
	if (!HasRow(strResult)) return TRUE;

	lockedNoteMain.m_lTotCNTLandFish = atoi(GetParseData(strResult).c_str());

	// length
	main1.Clear(); main2.Clear();
	main1.m_lLockedNoteFishID = atoi(GetParseData(strResult).c_str());
	main2.m_lLockedNoteFishID = atoi(GetParseData(strResult).c_str());
	main1.m_lLength = atoi(GetParseData(strResult).c_str());
	main2.m_lLength = atoi(GetParseData(strResult).c_str());
	main1.m_strUpdateDate = GetParseData(strResult).c_str();
	main2.m_strUpdateDate = GetParseData(strResult).c_str();
	lockedNoteMain.m_vecTopLengthFish.push_back(main1);
	lockedNoteMain.m_vecTopLengthFish.push_back(main2);

	// best_length
	GetParseData(strResult).c_str();

	// class
	main1.Clear(); main2.Clear();
	main1.m_lLockedNoteFishID = atoi(GetParseData(strResult).c_str());
	main2.m_lLockedNoteFishID = atoi(GetParseData(strResult).c_str());
	main1.m_lClass = atoi(GetParseData(strResult).c_str());
	main2.m_lClass = atoi(GetParseData(strResult).c_str());
	main1.m_strUpdateDate = GetParseData(strResult).c_str();
	main2.m_strUpdateDate = GetParseData(strResult).c_str();
	lockedNoteMain.m_vecTopClassFish.push_back(main1);
	lockedNoteMain.m_vecTopClassFish.push_back(main2);

	// best_class
	GetParseData(strResult).c_str();

	// 
	std::vector<LONG> vecGroupID = GetParse_RecentlyFish(GetParseData(strResult), G_STRING_DELIMETER);		
	std::vector<LONG> vecLength = GetParse_RecentlyFish(GetParseData(strResult), G_STRING_DELIMETER);		
	std::vector<string> vecRegDate = GetParse_RecentlyFish_string(GetParseData(strResult), G_STRING_DELIMETER);		
	std::vector<LONG> vecMapID = GetParse_RecentlyFish(GetParseData(strResult), G_STRING_DELIMETER);

	for(int i=0; i<G_LOCKED_NOTE_RECENTLY_CNT; i++)
	{
		Recently_Landing_Fish		fish;
		fish.Clear();
		fish.m_lLockedNoteFishID = vecGroupID[i];
		fish.m_lLength = vecLength[i];
		fish.m_strUpdateDate = vecRegDate[i];
		fish.m_lMapID = vecMapID[i];
		lockedNoteMain.m_vecRecentlyLandingFish.push_back(fish);
	}

	return TRUE;
}

// LockedNoteMap
BOOL CNFDBManager::SelectNFLockedNoteMap(LONG lGSN, LONG lCSN, TMapLockedNoteMap& mapLockedNoteMap)
{
	std::string		strQuery;
	std::string		strResult;
	BOOL			ret = FALSE;

	// locked_note_map
	// MAP_ID, FISH_GROUP_CNT, TOT_SCORE
	strQuery = format(QRY_SELECT_GT_CHAR_LOCKEDNOTE_MAP.c_str(), lGSN, lCSN);

	strResult.clear();
	ret = ExecQry(strQuery, strResult);

	if (!ret)				return FALSE;
	if (!HasRow(strResult)) return TRUE;

	while(true)
	{
		NFLockedNoteMap		map;
		map.Clear();

		LONG lMapID			= atoi(GetParseData(strResult).c_str());
		map.m_lTotUnlockFishCNT = atoi(GetParseData(strResult).c_str());
		map.m_lTotLockedScore = atoi(GetParseData(strResult).c_str());

		TMapLockedNoteMap::iterator iter = mapLockedNoteMap.find(lMapID);
		if (iter != mapLockedNoteMap.end())
			return FALSE;		// 여기 들어오면 오면 안 된다...ROW가 MAPID로 중복 되어 있다는 의미
		else 
			mapLockedNoteMap.insert(make_pair(lMapID, map));

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::InsertLockedNoteMap(LONG lMapID, LONG lGSN, LONG lCSN, LONG ltotCNT, LONG lTotScore, const std::string& strInsertDate, LONG& lErr)
{
	std::string		strQuery;

	strQuery = format(QRY_LOCKED_NOTE_MAP_INSERT.c_str(), lMapID, lGSN, lCSN, ltotCNT, lTotScore, strInsertDate.c_str(), strInsertDate.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret) {
		lErr = EC_LN_DB_FAIL_INSERT_GT_MAP;
		return FALSE;
	}

	return TRUE;
}

BOOL CNFDBManager::UpdateLockedNoteMap(LONG lMapID, LONG lGSN, LONG lCSN, LONG ltotCNT, LONG lTotScore, const std::string& strUpdateDate, LONG& lErr)
{
	std::string		strQuery;

	strQuery = format(QRY_LOCKED_NOTE_MAP_UPDATE.c_str(), ltotCNT, lTotScore, strUpdateDate.c_str(), lMapID, lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret) {
		lErr = EC_LN_DB_FAIL_UPDATE_GT_MAP;
		return FALSE;
	}

	return TRUE;
}


// LockedNoteFish
BOOL CNFDBManager::SelectNFLockedNoteFish(LONG lGSN, LONG lCSN, TMapLockedNoteMap& mapLockedNoteMap)
{
	std::string		strQuery;
	std::string		strResult;
	BOOL			ret = FALSE;

	// locked_note_fish
	// MAP_ID, FISH_GROUP, WEIGHT, LENGTH, SCORE, UPDATE_DATE 
	strQuery = format(QRY_SELECT_GT_CHAR_LOCKEDNOTE_FISH.c_str(), lGSN, lCSN);

	strResult.clear();
	ret = ExecQry(strQuery, strResult);

	if (!ret)				return FALSE;
	if (!HasRow(strResult)) return TRUE;

	while(true)
	{
		LockedNote_Fish_Info		landFish;
		landFish.Clear();

		LONG lMapID			= atoi(GetParseData(strResult).c_str());
		LONG lFishGroupID	= atoi(GetParseData(strResult).c_str());
		landFish.m_lWeight	= atoi(GetParseData(strResult).c_str());
		landFish.m_lLength	= atoi(GetParseData(strResult).c_str());
		landFish.m_lScore	= atoi(GetParseData(strResult).c_str());
		landFish.m_strUpdateDate	= GetParseData(strResult).c_str();

		TMapLockedNoteMap::iterator iter = mapLockedNoteMap.find(lMapID);
		if (iter != mapLockedNoteMap.end())
			(*iter).second.m_TblLockedNote.insert(make_pair(lFishGroupID, landFish));
		else 
			return FALSE;			// 여기 들어오면 오면 안 된다...MAPID로 CNT, totScore는 없는데 MAP에서 잡은 물고기가 있다는 의미.. 에러!!!

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::InsertLockedNoteFish(LONG lMapID, LONG lGSN, LONG lCSN, LONG lLockedNoteFishID, LONG lWeight, LONG lLength, LONG lScore, const std::string& strUpdateDate, LONG& lErr)
{
	std::string		strQuery;

	strQuery = format(QRY_LOCKED_NOTE_FISH_INSERT.c_str(), lMapID, lGSN, lCSN, lLockedNoteFishID, lWeight, lLength, lScore, strUpdateDate.c_str(), strUpdateDate.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret) {
		lErr = EC_LN_DB_FAIL_INSERT_GT_MAP_FISH;
		return FALSE;
	}

	return TRUE;
}

BOOL CNFDBManager::UpdateLockedNoteFish(LONG lMapID, LONG lGSN, LONG lCSN, LONG lLockedNoteFishID, LONG lWeight, LONG lLength, LONG lScore, const std::string& strUpdateDate, LONG& lErr)
{
	std::string		strQuery;

	strQuery = format(QRY_LOCKED_NOTE_FISH_UPDATE.c_str(), lWeight, lLength, lScore, strUpdateDate.c_str(), lMapID, lGSN, lCSN, lLockedNoteFishID);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret) {
		lErr = EC_LN_DB_FAIL_UPDATE_GT_MAP_FISH;
		return FALSE;
	}

	return TRUE;
}

BOOL CNFDBManager::UpdateLockedNoteMain(LONG lTotLandCNT, LONG lGSN, LONG lCSN, const TVecRecentlyLandingFish& landMain)
{
	std::string		strQuery;

	strQuery = format(QRY_UPDATE_LOCKED_NOTE_MAIN.c_str(), lTotLandCNT, 
		landMain[0].m_lLockedNoteFishID, landMain[0].m_lWeight, landMain[0].m_lLength, landMain[0].m_strUpdateDate.c_str(),
		landMain[1].m_lLockedNoteFishID, landMain[1].m_lWeight, landMain[1].m_lLength, landMain[1].m_strUpdateDate.c_str(),
		landMain[2].m_lLockedNoteFishID, landMain[2].m_lWeight, landMain[2].m_lLength, landMain[2].m_strUpdateDate.c_str(),
		landMain[3].m_lLockedNoteFishID, landMain[3].m_lWeight, landMain[3].m_lLength, landMain[3].m_strUpdateDate.c_str(),
		landMain[4].m_lLockedNoteFishID, landMain[4].m_lWeight, landMain[4].m_lLength, landMain[4].m_strUpdateDate.c_str(),
		landMain[5].m_lLockedNoteFishID, landMain[5].m_lWeight, landMain[5].m_lLength, landMain[5].m_strUpdateDate.c_str(),
		landMain[6].m_lLockedNoteFishID, landMain[6].m_lWeight, landMain[6].m_lLength, landMain[6].m_strUpdateDate.c_str(),
		landMain[7].m_lLockedNoteFishID, landMain[7].m_lWeight, landMain[7].m_lLength, landMain[7].m_strUpdateDate.c_str(),
		landMain[8].m_lLockedNoteFishID, landMain[8].m_lWeight, landMain[8].m_lLength, landMain[8].m_strUpdateDate.c_str(),
		lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
		return FALSE;

	return TRUE;
}

// WORKING(acepm83@neowiz.com)
BOOL CNFDBManager::InsertNFCharAquaFish(LONG lGSN, LONG lCSN, LONG lFishID, LONG lLength, LONG lWeight, LONG lElapsedTime, LONG lScore)
{
	std::string strQuery;
	strQuery = format(QRY_INSERT_NF_GT_CHAR_AQUA_FISH.c_str(), lGSN, lCSN, lFishID, lLength, lWeight, lElapsedTime, lScore);

	std::string strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
		return FALSE;

	return TRUE;
}

// WORKING(acepm83@neowiz.com)
BOOL CNFDBManager::SelectNFCharAquaFish(LONG lCSN, TMapNFAquaFish& mapNFAquaFish)
{
	std::string strQuery;
	strQuery = format(QRY_SELECT_NF_GT_CHAR_AQUA_FISH.c_str(), lCSN);

	std::string strResult;
	BOOL ret = ExecQry(strQuery, strResult);

	if (!ret)					return FALSE;
	if (!HasRow(strResult))		return TRUE;

	while (!strResult.empty())
	{
		NFAquaFish aquaFishInfo;

		aquaFishInfo.m_lSeq			= atoi(GetParseData(strResult).c_str());
		aquaFishInfo.m_lFishID		= atoi(GetParseData(strResult).c_str());
		aquaFishInfo.m_strRegDate	= GetParseData(strResult).c_str();
		aquaFishInfo.m_lLength		= atoi(GetParseData(strResult).c_str());
		aquaFishInfo.m_lWeight		= atoi(GetParseData(strResult).c_str());
		aquaFishInfo.m_lElspedTime	= atoi(GetParseData(strResult).c_str());
		aquaFishInfo.m_lScore		= atoi(GetParseData(strResult).c_str());

		mapNFAquaFish.insert(std::make_pair(aquaFishInfo.m_lSeq,aquaFishInfo));
	}

	return TRUE;
}

// WORKING(acepm83@neowiz.com)
BOOL CNFDBManager::SelectNFCharAqua(LONG lGSN, LONG lCSN, NFAqua& nfAqua, LONG& lElapsedClearHour, LONG& lElapsedFeedHour)
{
	std::string strQuery;
	strQuery = format(QRY_SELECT_NF_GT_CHAR_AQUA.c_str(), lGSN, lCSN);

	std::string strResult;
	BOOL ret = ExecQry(strQuery, strResult);

	if (!ret)					return FALSE;
	if (!HasRow(strResult))		return TRUE; // TODO(acepm83@neowiz.com) 현재는 수족관 정보가 없어도 에러아님! 캐릭터 생성시 수족관테이블에 기본값으로 INSERT 하도록 해야 함.

	while (!strResult.empty())
	{
		nfAqua.m_strClearDate = GetParseData(strResult).c_str();
		nfAqua.m_strFeedDate = GetParseData(strResult).c_str();
		nfAqua.m_lAquaLevel = atoi(GetParseData(strResult).c_str());
		nfAqua.m_lAquaThemeItemCode = atoi(GetParseData(strResult).c_str());
		nfAqua.m_dFeedGauge = atof(GetParseData(strResult).c_str());
		nfAqua.m_dClearGauge = atof(GetParseData(strResult).c_str());
		nfAqua.m_lAquaScore = atoi(GetParseData(strResult).c_str());
		lElapsedClearHour = atoi(GetParseData(strResult).c_str());	// 청소 경과시간
		lElapsedFeedHour = atoi(GetParseData(strResult).c_str());	// 밥준 경과시간
	}

	return TRUE;
}

// NF Cheat
BOOL CNFDBManager::UpdateCharExpAndLevel(const LONG lCSN, const LONG lExp, const LONG lLevel)
{
	std::string	strQuery = format(QRY_UPDATE_CHAR_EXP_LEVEL.c_str(), lExp, lLevel, lCSN);

	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
	{
		return FALSE;
	}

	return TRUE;
}
BOOL CNFDBManager::UpdateCharMoney( const LONG lCSN, const __int64 i64Money )
{
	std::string	strQuery = format(QRY_UPDATE_CHAR_MONEY.c_str(), i64Money, lCSN);
	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CNFDBManager::UpdateCharAchv(const LONG lGSN, const LONG lCSN, const int nAchvID, const double dGauge)
{
	std::string	strQuery = format(QRY_UPDATE_CHAR_MONEY.c_str(), nAchvID, dGauge, lGSN, lCSN);
	std::string	strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret)
	{
		return FALSE;
	}

	return TRUE;
}


// MetaData
BOOL CNFDBManager::SelectItemCommon(string& strResult, ItemCommon& itemCommon)
{
	// USE_TYPE, FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, 
	// HEALTH, FISH_POINT, LUCKY_POINT, 
	itemCommon.m_strUseType = GetParseData(strResult).c_str();
 	itemCommon.m_nfAbilityExt.m_nfAbility.m_dFlyDist = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_nfAbility.m_dCharm = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_nfAbility.m_dStrength = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_nfAbility.m_dAgility = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_nfAbility.m_dControl = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_nfAbility.m_dHealth = atof(GetParseData(strResult).c_str());
	itemCommon.m_nfAbilityExt.m_nfAbility.m_dFishPoint = atof(GetParseData(strResult).c_str());
	itemCommon.m_nfAbilityExt.m_nfAbility.m_dLuckyPoint = atof(GetParseData(strResult).c_str());
	//atof(GetParseData(strResult).c_str());	// capacity

	// KEEP_LUCK_FLAG, CASTING_SCORE, BACKLASH_RATE, INCOUNT_SCORE, SPECIAL_FISH_BITE, ADD_EXP_RATE, ADD_GAME_MONEY_RATE
	// ext 추가
	itemCommon.m_nfAbilityExt.m_dKeepLuckyFlag = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_dCastingScore = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_dCastingBacklashRate = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_lActionIncountScore = atoi(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_dSpecialFishBite = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_dAddRateExp = atof(GetParseData(strResult).c_str());
 	itemCommon.m_nfAbilityExt.m_dAddRateGameMoney = atof(GetParseData(strResult).c_str());

	return TRUE;
}
 
BOOL CNFDBManager::SelectNFItemEquip(BOOL bIsAll, TMapIndexEquipItem& mapItem, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_ITEM_EQUIP.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		EquipItem* pItem = new EquipItem;
		if (!pItem)
			return FALSE;

		pItem->Clear();

		// ITEM_CODE, ITEM_ID, ITEM_GRP, ITEM_LEV, CHAR_LEV, PARTS, CAPACITY, BUOYANCY, END_DEPTH, START_DEPTH, LOAD_MAX, LOAD_LIMIT, ENDURANCE, BOAT_TYPE, BOAT_LEV, BOAT_MAX, 
		// RETRIEVAL, ADD_FISH_LIST_ID, LURE, ENDURANCE_PRICE, EQUIP_ITEM_TYPE
		pItem->m_lItemCode = atoi(GetParseData(strResult).c_str());
		pItem->m_lItemID = atoi(GetParseData(strResult).c_str());
		pItem->m_strType = GetParseData(strResult).c_str();
		pItem->m_lLv = atoi(GetParseData(strResult).c_str());
		pItem->m_lGrade = atoi(GetParseData(strResult).c_str());
		pItem->m_lParts= atoi(GetParseData(strResult).c_str());		// parts
		pItem->m_lCapacity = atoi(GetParseData(strResult).c_str());
		pItem->m_dBuoyancy = atof(GetParseData(strResult).c_str());
		pItem->m_dEndDepth = atof(GetParseData(strResult).c_str());
		pItem->m_dStartDepth = atof(GetParseData(strResult).c_str());
		pItem->m_dLineLoadMax = atof(GetParseData(strResult).c_str());
		pItem->m_dLineLoadLimit = atof(GetParseData(strResult).c_str());
		pItem->m_lEndurance = atoi(GetParseData(strResult).c_str());
		pItem->m_lWaterType = atoi(GetParseData(strResult).c_str());
		pItem->m_lBoatLev = atoi(GetParseData(strResult).c_str());
		pItem->m_lBoatMaxUsers = atoi(GetParseData(strResult).c_str());
		pItem->m_dRetrievalLine = atof(GetParseData(strResult).c_str());
		pItem->m_lAddFishListID = atoi(GetParseData(strResult).c_str());
		pItem->m_lLure = atoi(GetParseData(strResult).c_str());
		pItem->m_llEndurancePrice = _atoi64(GetParseData(strResult).c_str());
		pItem->m_lEquipItemType = atoi(GetParseData(strResult).c_str());

		// USER_TYPE, FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISH_POINT, LUCKY_POINT, 
		// KEEP_LUCK_FLAG, CASTING_SCORE, BACKLASH_RATE, INCOUNT_SCORE, SPECIAL_FISH_BITE, ADD_EXP_RATE, ADD_GAME_MONEY_RATE
		SelectItemCommon(strResult, *pItem);

		// Equip에만, AddFishList 추가
		theNFDataItemMgr.GetAddFishListByID(pItem->m_lAddFishListID, pItem->m_mapAddFishList);
		
		if (pItem->m_lItemCode == 190000)
			mapItem[pItem->m_lItemCode] = pItem;
		else
            mapItem[pItem->m_lItemCode] = pItem;
		
		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFItemClothes(BOOL bIsAll, TMapIndexClothesItem& mapItem, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_ITEM_CLOTHES.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		ClothesItem* pItem = new ClothesItem;
		if (!pItem)
			return FALSE;

		pItem->Clear();

		pItem->m_lItemCode = atoi(GetParseData(strResult).c_str());
		pItem->m_lItemID = atoi(GetParseData(strResult).c_str());
		pItem->m_strType = GetParseData(strResult).c_str();
		pItem->m_lLv = atoi(GetParseData(strResult).c_str());
		pItem->m_lGrade = atoi(GetParseData(strResult).c_str());
		pItem->m_lParts = atoi(GetParseData(strResult).c_str());
		pItem->m_lCapacity = atoi(GetParseData(strResult).c_str());
		pItem->m_lDefaultCharSRL = atoi(GetParseData(strResult).c_str());
		pItem->m_lEnvAttribute = atoi(GetParseData(strResult).c_str());

		SelectItemCommon(strResult, *pItem);

		mapItem[pItem->m_lItemCode] = pItem;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFItemUsable(BOOL bIsAll, TMapIndexUsableItem& mapItem, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_ITEM_USABLE.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		// ITEM_CODE, ITEM_ID, ITEM_GRP, ITEM_LEV, CHAR_LEV, PARTS, CAPACITY, DELAY, HP,  
		// FLY_DIST, CHARM, STRENGTH, AGILITY, CONTROL, HEALTH, FISH_POINT, LUCKY_POINT, KEEP_LUCK_FLAG, CASTING_SCORE, BACKLASH_RATE, INCOUNT_SCORE, SPECIAL_FISH_BITE, ADD_EXP_RATE, ADD_GAME_MONEY_RATE
		UsableItem* pItem = new UsableItem;
		if (!pItem)
			return FALSE;

		pItem->Clear();

		pItem->m_lItemCode = atoi(GetParseData(strResult).c_str());
		pItem->m_lItemID = atoi(GetParseData(strResult).c_str());
		pItem->m_strType = GetParseData(strResult).c_str();
		pItem->m_lLv = atoi(GetParseData(strResult).c_str());
		pItem->m_lGrade = atoi(GetParseData(strResult).c_str());
		pItem->m_lParts = atoi(GetParseData(strResult).c_str());
		pItem->m_lCapacity = atoi(GetParseData(strResult).c_str());
		pItem->m_lDelay = atoi(GetParseData(strResult).c_str());
		pItem->m_lHP = atoi(GetParseData(strResult).c_str());

		SelectItemCommon(strResult, *pItem);

		mapItem[pItem->m_lItemCode] = pItem;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFItemSkill(BOOL bIsAll, TMapIndexSkillItem& mapItem, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_ITEM_SKILL.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		SkillItem* pItem = new SkillItem;
		if (!pItem)
			return FALSE;

		pItem->Clear();

		pItem->m_lItemCode = atoi(GetParseData(strResult).c_str());
		pItem->m_lItemID = atoi(GetParseData(strResult).c_str());
		pItem->m_strType = GetParseData(strResult).c_str();
		pItem->m_lLv = atoi(GetParseData(strResult).c_str());
		pItem->m_lGrade = atoi(GetParseData(strResult).c_str());
		pItem->m_lParts = atoi(GetParseData(strResult).c_str());
		pItem->m_lCapacity = atoi(GetParseData(strResult).c_str());
		pItem->m_lUsed = atoi(GetParseData(strResult).c_str());
		pItem->m_lReuseTime = atoi(GetParseData(strResult).c_str());
		pItem->m_lReadyTime = atoi(GetParseData(strResult).c_str());
		pItem->m_lCondition = atoi(GetParseData(strResult).c_str());

		SelectItemCommon(strResult, *pItem);

		mapItem[pItem->m_lItemCode] = pItem;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFItemCard(BOOL bIsAll, TMapLevelCardItem& mapCardPackItem, TMapIndexCardItem& mapCardItem, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_ITEM_CARD.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		CardItem* pItem = new CardItem;
		if (!pItem)
			return FALSE;

		pItem->Clear();

		pItem->m_lItemCode = atoi(GetParseData(strResult).c_str());
		pItem->m_lItemID = atoi(GetParseData(strResult).c_str());
		pItem->m_strType = GetParseData(strResult).c_str();					// R
		pItem->m_lLv = atoi(GetParseData(strResult).c_str());
		pItem->m_lParts = atoi(GetParseData(strResult).c_str());
		pItem->m_lCapacity = atoi(GetParseData(strResult).c_str());
		pItem->m_strCardType = GetParseData(strResult).c_str();				// P or 능력치(7가지)
		pItem->m_strUseType = GetParseData(strResult).c_str();				// 
		pItem->m_nfAbilityExt.m_nfAbility.m_dFlyDist = atof(GetParseData(strResult).c_str());
		pItem->m_nfAbilityExt.m_nfAbility.m_dCharm = atof(GetParseData(strResult).c_str());
		pItem->m_nfAbilityExt.m_nfAbility.m_dStrength = atof(GetParseData(strResult).c_str());
		pItem->m_nfAbilityExt.m_nfAbility.m_dAgility = atof(GetParseData(strResult).c_str());
		pItem->m_nfAbilityExt.m_nfAbility.m_dHealth = atof(GetParseData(strResult).c_str());
		pItem->m_nfAbilityExt.m_nfAbility.m_dLuckyPoint = atof(GetParseData(strResult).c_str());
		
		mapCardItem[pItem->m_lItemCode] = pItem;

		// 카드 생성을 빠르게 하기 위해서 아래와 같이 만듬....
		//////////////////////////////////////////////////////////////////////////
		TMapLevelCardItem::iterator iter1 = mapCardPackItem.find(pItem->m_lLv);
		if (iter1 != mapCardPackItem.end())		// found
		{	
			TMapCardTypeCardItem mapCardType = (*iter1).second;
			TMapCardTypeCardItem::iterator iter2 = mapCardType.find(pItem->m_strCardType);
			if (iter2 != mapCardType.end())		// found
			{
				(*iter2).second.push_back(pItem);
			}
			else		// not found by card_type
			{
				TlstCardItem	lstCardItem;
				lstCardItem.push_back(pItem);

				(*iter1).second[pItem->m_strCardType] = lstCardItem;
			}
		}
		else		// not found by card_level
		{
			TlstCardItem	lstCardItem;
			lstCardItem.push_back(pItem);

			TMapCardTypeCardItem	mapCardType;
			mapCardType[pItem->m_strCardType] = lstCardItem;

			mapCardPackItem[pItem->m_lLv] = mapCardType;
		}

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFItemCardPackRate(BOOL bIsAll, TMapIndexCardPackRate& mapCardPackRate, LONG& lCnt)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_CARDPACK_OPEN.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		LONG lCardPackID = atoi(GetParseData(strResult).c_str());
		LONG lCardID = atoi(GetParseData(strResult).c_str());
		LONG lRate = atoi(GetParseData(strResult).c_str());

		TPairCardRate	pair;
		pair.first = lCardID;
		pair.second = lRate;

		TMapIndexCardPackRate::iterator iter = mapCardPackRate.find(lCardPackID);
		if (iter == mapCardPackRate.end())
		{
			TlstCardRate	lst;
			lst.push_back(pair);
			++lCnt;

			mapCardPackRate[lCardPackID] = lst;
		}
		else
		{
			TlstCardRate& lst = mapCardPackRate[lCardPackID];
			lst.push_back(pair);
			++lCnt;
		}

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFFishMap(BOOL bIsAll, TMapIndexFishMap& mapFishMap, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_MAP.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		FishMap* pFishMap = new FishMap;
		if (pFishMap)
		{
			pFishMap->m_lIndex = atoi(GetParseData(strResult).c_str());
			pFishMap->m_lMapLength = atoi(GetParseData(strResult).c_str());
			pFishMap->m_lMapWidth = atoi(GetParseData(strResult).c_str());
			pFishMap->m_lCntFP_Boat = atoi(GetParseData(strResult).c_str());			// Total Cnt를 위해서 다시 받는다.
			pFishMap->m_lCntFP_Boat = atoi(GetParseData(strResult).c_str());
			pFishMap->m_lCntFP_Walk = atoi(GetParseData(strResult).c_str());
			pFishMap->m_lSignMapPoint = atoi(GetParseData(strResult).c_str());
			//pFishMap->m_lMAXLockedSocre = atoi(GetParseData(strResult).c_str());
		}
		mapFishMap[pFishMap->m_lIndex] = pFishMap;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFFishingPoint(BOOL bIsAll, TMapIndexFishMap& mapFishMap, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_FISHINGPOINT.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		FishingPoint fishingPoint;
		fishingPoint.Clear();

		fishingPoint.m_lIndex = atoi(GetParseData(strResult).c_str());
		fishingPoint.m_lFishMapIndex = atoi(GetParseData(strResult).c_str());
		fishingPoint.m_lPointerType = atoi(GetParseData(strResult).c_str());
		fishingPoint.m_lLocation_X = atoi(GetParseData(strResult).c_str());			
		fishingPoint.m_lLocation_Y = atoi(GetParseData(strResult).c_str());
		fishingPoint.m_lMaxUsers = atoi(GetParseData(strResult).c_str());
		string strIsSalt = GetParseData(strResult).c_str();
		if (strIsSalt == "Y")
            fishingPoint.m_bIsSalt = TRUE;
		else
			fishingPoint.m_bIsSalt = FALSE;

		FishMap* pFishMap = mapFishMap[fishingPoint.m_lFishMapIndex];
		if (pFishMap)
			pFishMap->m_mapFishingPoint[fishingPoint.m_lIndex] = fishingPoint;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFFishInfo(BOOL bIsAll, TMapIndexFishInfo& mapFishInfo, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_FISH.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		// FISH_ID, MAP_ID, SALT, CLASS, TYPE, MIN_SIZE, MAX_SIZE, MIN_WEIGHT, MAX_WEIGHT, POINT, 
		// ATTACK, ATTACK_AMOUNT, ATTACK_RATE, RESTRATE, MOVE_PATTERN, MOVE_SPEED, SKILL_GROUP, LV, HITPOINT, MIN_DEPTH, 
		// MAX_DEPTH, LANDING_HEADSHAKE, FEEDTIME, HOOKTIME, DROPITEM_MAX, DROPITEM_IDX, EXP, SIGN_POINT, MONEY, LANDNOTE_FISH_ID, 
		// FLY_DIST, CHARM, STRENGTH, AGILITY, LUCKY_POINT, LANDING_HEADSHAKE, MOVE_PATTERN 

		FishInfo* pFishInfo = new FishInfo;
		if (pFishInfo)
		{
			pFishInfo->m_lIndex = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lMapID = atoi(GetParseData(strResult).c_str());
			long lType = atoi(GetParseData(strResult).c_str());
			if (lType)
				pFishInfo->m_bFishSalt = true;
			else
				pFishInfo->m_bFishSalt = false;
			pFishInfo->m_lFishClass = atoi(GetParseData(strResult).c_str());		
			pFishInfo->m_lFishType = atoi(GetParseData(strResult).c_str());			// 희귀물고기인지아닌지 구분하는 용도로 쓰임					
			pFishInfo->m_lFish_MinSize = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lFish_MaxSize = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lFish_MinWeight = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lFish_MaxWeight= atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lFishPoint = atoi(GetParseData(strResult).c_str());

			pFishInfo->m_dAttack = atof(GetParseData(strResult).c_str());
			pFishInfo->m_dAttackAmount = atof(GetParseData(strResult).c_str());
			pFishInfo->m_lAttackRate = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lRestRate = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lMovePattern = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lMoveSpeed = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lSkillGroup = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lFishLv = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lHitPoint = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lMinDepth = atoi(GetParseData(strResult).c_str());

			pFishInfo->m_lMaxDepth = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lLandingHeadShake = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lFeedTime = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lHookTime = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lDropItemMax = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lDropItemIdx = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lFishExp = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_llMoney = atoi(GetParseData(strResult).c_str());
			pFishInfo->m_lLockedNoteFishID = atoi(GetParseData(strResult).c_str());

			// 물고기 능력치
			pFishInfo->NFAbility::Clear();
			pFishInfo->m_dFlyDist = atof(GetParseData(strResult).c_str());
			pFishInfo->m_dCharm = atof(GetParseData(strResult).c_str());
			pFishInfo->m_dStrength = atof(GetParseData(strResult).c_str());
			pFishInfo->m_dAgility = atof(GetParseData(strResult).c_str());
			pFishInfo->m_dLuckyPoint = atof(GetParseData(strResult).c_str());
			pFishInfo->m_lLockedNoteScore = atoi(GetParseData(strResult).c_str());		// LANDING_HEADSHAKE 임시로
			pFishInfo->m_lBigFishID = atoi(GetParseData(strResult).c_str());			// MOVE_PATTERN 임시로
		}
		mapFishInfo.insert(make_pair(pFishInfo->m_lIndex, pFishInfo));

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFFishSkill(BOOL bIsAll, TMapIndexFishSkill& mapFishSkill, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_FISHSKILL.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		FishSkill* pFishSkill = new FishSkill;
		if (pFishSkill)
		{
			// SKILL_ID, LV, TYPE, CONDITION, REUSETIME, HEADSHAKE, EFFECTDIRECTION, COSTHP, ATTACK_POWER, 
			// JUMP_HEIGHT, JUMP_DISTANCE, DIVINGDEPTH, ADDDISTANCE, ROD_REACITON_DIR, BULLET, CAST_TIME
			pFishSkill->m_lIndex = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lLV = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lType = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lActivityCondition = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lReactivityTime = atoi(GetParseData(strResult).c_str());			
			pFishSkill->m_dHeadshakeProbability = atof(GetParseData(strResult).c_str());
			pFishSkill->m_bEffectCurrentDirection = (BOOL)atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lCostHP = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lAddAttackPower= atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lJumpHeight = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lJumpDistance = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lDivingDepth = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lAddDistance = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lRodReactionDir = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lIsBullet = atoi(GetParseData(strResult).c_str());
			pFishSkill->m_lCastTime = atoi(GetParseData(strResult).c_str());
		}
		mapFishSkill[pFishSkill->m_lIndex] = pFishSkill;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFFishSkillCode(BOOL bIsAll, TMapIndexFishSkillCode& mapFishSkillcode, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_FISHSKILLCODE.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		FishSkillCode* pFishSkillCode = new FishSkillCode;
		if (pFishSkillCode)
		{
			pFishSkillCode->m_lIndex = atoi(GetParseData(strResult).c_str());
			for(int i=0; i<7; i++)
			{
				FishSkillRate	fishSkillRate;
				fishSkillRate.m_lSkillIndex = atoi(GetParseData(strResult).c_str());
				fishSkillRate.m_lActiveRate = atoi(GetParseData(strResult).c_str());
				if (fishSkillRate.m_lSkillIndex != 0)
					pFishSkillCode->m_vecFishSkillRate.push_back(fishSkillRate);
			}
		}
		mapFishSkillcode[pFishSkillCode->m_lIndex] = pFishSkillCode;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFFishingCode(BOOL bIsAll, TMapIndexIndexList& mapFishingCode, long lSelectIndex)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_FISHINGCODE.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		LONG lFishingPointIndex = atoi(GetParseData(strResult).c_str());
		LONG lFishIndex = atoi(GetParseData(strResult).c_str());

		TMapIndexIndexList::iterator itFind = mapFishingCode.find(lFishingPointIndex);
		if (itFind != mapFishingCode.end())
		{
			BOOL bFind = FALSE;
			ForEachElmt(TListIndex, (*itFind).second, it, ij)
			{
				if ((*it) == lFishIndex)
					bFind = TRUE;
			}
			if (!bFind)
				(*itFind).second.push_back(lFishIndex);
		}
		else
		{
			TListIndex	newList;
			newList.push_back(lFishIndex);
			mapFishingCode[lFishingPointIndex] = newList;
		}

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFSignCode(BOOL bIsAll, TMapIndexFishMap& mapFishMap, LONG& lCnt)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_SIGN.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		LONG lFishMapIndex = atoi(GetParseData(strResult).c_str());
		LONG lSignIndex = atoi(GetParseData(strResult).c_str());
		double dSignProb = atof(GetParseData(strResult).c_str());
		LONG lSpecialFishIndex = atoi(GetParseData(strResult).c_str());
		double dSpecialFishProb = atof(GetParseData(strResult).c_str());

		// Map Index 찾기
		TMapIndexFishMap::iterator itFind = mapFishMap.find(lFishMapIndex);
		if (itFind != mapFishMap.end())
		{
			// Sign Index 찾기
			TMapSignProb	mapSignProb = (*itFind).second->m_mapSignProb;
			lCnt++;

			TMapSignProb::iterator iter = mapSignProb.find(lSignIndex);
			if (iter != mapSignProb.end())
			{
				// Special Fish Index 찾아서 넣기
				(*iter).second.m_mapSpecialFishProb[lSpecialFishIndex] = dSpecialFishProb;
			}
			else
			{
				SignProb	signProb;
				signProb.m_dSignProb = dSignProb;
				signProb.m_mapSpecialFishProb[lSpecialFishIndex] = dSpecialFishProb;
				
				(*itFind).second->m_mapSignProb[lSignIndex] = signProb;
			}
		}
		else
			return FALSE;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFDropItem(BOOL bIsAll, TMapDropItemRate& mapDropItemRate, LONG& lCnt)
{
	lCnt = 0;
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_DROP_ITEM.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		DropItemRate		dropRate;

		LONG lDropIndex= atoi(GetParseData(strResult).c_str());
		dropRate.m_lDropItemID = atoi(GetParseData(strResult).c_str());
		dropRate.m_lDropRate = atoi(GetParseData(strResult).c_str());

		TMapDropItemRate::iterator iter = mapDropItemRate.find(lDropIndex);
		if (iter != mapDropItemRate.end())
		{
			BOOL bDuplicate = FALSE;
			// 중복되면 겹친다...
			ForEachElmt(TlstDropItemRate, (*iter).second, it, ij)
			{
				if ((*it).m_lDropItemID == dropRate.m_lDropItemID) {
					(*it).m_lDropRate = dropRate.m_lDropRate;
					bDuplicate = TRUE;
				}
			}
				
			if (!bDuplicate) {
				(*iter).second.push_back(dropRate);
				++lCnt;
			}
		}
		else
		{
			TlstDropItemRate	lstDrop;
			lstDrop.push_back(dropRate);
			mapDropItemRate[lDropIndex] = lstDrop;
			++lCnt;
		}

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

BOOL CNFDBManager::SelectNFItemEnchantInfo(TmapNFItemEnchantInfo& mapNFItemEnchantInfo)
{
	std::string strQuery = format(QRY_SELECT_ALL_NF_ITEM_ENCHANT_INFO.c_str());
	
	std::string strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		NFItemEnchantInfo info;

		info.lEnchantType = atoi(GetParseData(strResult).c_str());
		info.lEnchantLevel = atoi(GetParseData(strResult).c_str());
		info.nfAddAbility.m_dFlyDist = atoi(GetParseData(strResult).c_str());
		info.nfAddAbility.m_dStrength = atoi(GetParseData(strResult).c_str());
		info.nfAddAbility.m_dAgility = atoi(GetParseData(strResult).c_str());
		info.nfAddAbility.m_dControl = atoi(GetParseData(strResult).c_str());
		info.nfAddAbility.m_dFishPoint = atoi(GetParseData(strResult).c_str());
		info.dSuccessRate = atof(GetParseData(strResult).c_str());
		info.dDestroyRate = atof(GetParseData(strResult).c_str()); 
		info.lGaugeDecrease = atoi(GetParseData(strResult).c_str());
		info.lMaxCount = atoi(GetParseData(strResult).c_str());

		std::pair<LONG, LONG> key(info.lEnchantType, info.lEnchantLevel);
		mapNFItemEnchantInfo.insert(std::make_pair(key, info));

		if (strResult.size() <= 0)
			break;
	}

	return TRUE;
}

BOOL CNFDBManager::SelectNFAddFishList(BOOL bIsAll, TMapAddFishList& mapAddFishList)
{
	std::string		strQuery;
	if (bIsAll)
		strQuery = format(QRY_SELECT_ALL_NF_ADD_FISH_LIST.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		TMapAddFishRate		mapAddFishRate;

		LONG	lFishIndex = 0;
		double	dFishRate = 0.0f;
		LONG	lAddFISH_ID = atoi(GetParseData(strResult).c_str());

		lFishIndex = atoi(GetParseData(strResult).c_str());
		dFishRate = atof(GetParseData(strResult).c_str());
		if (lFishIndex > 0) {
			mapAddFishRate[lFishIndex] = dFishRate;
			lFishIndex = 0; dFishRate = 0.0f;
		}

		lFishIndex = atoi(GetParseData(strResult).c_str());
		dFishRate = atof(GetParseData(strResult).c_str());
		if (lFishIndex > 0) {
			mapAddFishRate[lFishIndex] = dFishRate;
			lFishIndex = 0; dFishRate = 0.0f;
		}

		lFishIndex = atoi(GetParseData(strResult).c_str());
		dFishRate = atof(GetParseData(strResult).c_str());
		if (lFishIndex > 0) {
			mapAddFishRate[lFishIndex] = dFishRate;
			lFishIndex = 0; dFishRate = 0.0f;
		}

		lFishIndex = atoi(GetParseData(strResult).c_str());
		dFishRate = atof(GetParseData(strResult).c_str());
		if (lFishIndex > 0) {
			mapAddFishRate[lFishIndex] = dFishRate;
			lFishIndex = 0; dFishRate = 0.0f;
		}

		lFishIndex = atoi(GetParseData(strResult).c_str());
		dFishRate = atof(GetParseData(strResult).c_str());
		if (lFishIndex > 0) {
			mapAddFishRate[lFishIndex] = dFishRate;
			lFishIndex = 0; dFishRate = 0.0f;
		}

		mapAddFishList[lAddFISH_ID] = mapAddFishRate;

		if(strResult.size() <= 0)
			break;
	}
	return TRUE;
}

// New Achv 작업중...
BOOL CNFDBManager::SelectNFAchvByChar(const LONG lGSN, const LONG lCSN, MAP_ACHV_STATE& rkOutMapAchvState)
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_CHAR_ACHV.c_str(), lGSN, lCSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret) return FALSE;
	if (!HasRow(strResult)) return TRUE;

	// ACHV_ID, PROGRESS, COMPLETE_DATE, GET_REWARD_DATE, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4, LAST_UPDATE_DATE, REMARK FROM NF_GT_CHAR_ACHV WHERE GSN=? AND CSN=?
	while (!strResult.empty())
	{
		SAchvState achv;

		achv.lAchvID			= atoi(GetParseData(strResult).c_str());
		achv.dProgress			= atof(GetParseData(strResult).c_str());
		achv.strCompleteDate	= GetParseData(strResult).c_str();
		achv.strRewardDate		= GetParseData(strResult).c_str();
		achv.lItemID1			= atoi(GetParseData(strResult).c_str());
		achv.lItemID2			= atoi(GetParseData(strResult).c_str());
		achv.lItemID3			= atoi(GetParseData(strResult).c_str());
		achv.lItemID4			= atoi(GetParseData(strResult).c_str());
		achv.strLastUpdateDate	= GetParseData(strResult).c_str();
		GetParseData(strResult); // remark

		rkOutMapAchvState.insert(make_pair(achv.lAchvID, achv));
	}

	return TRUE;
}

BOOL CNFDBManager::UpdateAchvProgress(string& outLastUpdateTime, const LONG lGSN, const LONG lCSN, const LONG lAchvID, const double dProgress, const bool isCompleted)
{
	std::string		strQuery;
	strQuery = format(PKG_NF_ACHV_UPDATE_ACHV.c_str(), lGSN, lCSN, lAchvID, dProgress, isCompleted ? "Y" : "N" );

	int lErrorCode;
	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return lErrorCode;

	outLastUpdateTime =  GetParseData(strResult).c_str();

	return TRUE;
}