#pragma once


//
// RoomInternalLogic.h
//

#ifndef GRCCONTAINER_H
#define GRCCONTAINER_H

///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////

class CUser;
//class CRoom;
class CUserMap;
class CLrbMsg;
class PayloadNGSCli;
class PayloadCliNGS;
class PayloadNGSCHS;
class PayloadNGSNCS;
class PayloadGLSAMS;
class PayloadCHSNGS;
class MsgSMGWGLS_UserLeaveNtf;
class MsgAMSGLS_RoomDeleteReq;

class PayloadOUTLCS;
class PayloadCliMDS;
class CRoomTimerManager;
class CNotifyToOthers;

///////////////////////////////////////////////////////////////////////////////////
#include <GRCContainerBase.h>
#include "ChatHistory.h"
#include "Agent.h"
#include "common.h"
//#define DECLEXPMETHOD(r,f) virtual r __stdcall f
//#define IMPLEXPMETHOD(r,f) r __stdcall f
//
//struct RCUserMsg;
//class RCUserData;
//
//interface IGRCContainer
//{
//// 	DECLEXPMETHOD(LONG, GetUserMsg)(RCUserMsg& msg) = 0;
//
//	virtual LONG __stdcall GetUserMsg(RCUserMsg& msg) = 0;
//	virtual LONG __stdcall SendUserMsg(LONG lCSN, RCUserMsg& msg) = 0;
//	virtual LONG __stdcall NotifyChangeRoomState(LONG lRoomState) = 0;
//	virtual LONG __stdcall GetUserData(LONG lCSN, RCUserData& userInfo) = 0;
//	virtual LONG __stdcall SetUserData(LONG lCSN, const RCUserData& userInfo) = 0;
//	virtual LONG __stdcall SetTimer(LONG lDue, LONG lPeriod) = 0;
//	virtual LONG __stdcall NotifyAddUser(LONG lCSN) = 0;
//	virtual LONG __stdcall NotifyRemoveUser(LONG lCSN) = 0;
//};
//
////IMPLEEXPMETHOD(LONG, CRoomInternalLogic::GetUserMsg)(RCUserMsg& msg)
////{
////}


///////////////////////////////////////////////////////////////////////////////////
//// ACHV BEGIN
#include <ACHV/AchvDef.h>
static achv::CAchvMgr& g_achv = achv::CAchvMgr::Instance();
//// ACHV END


// CRoomInternalLogic
#include "RoomUserManager.h"


class IRoomEventHandler;
//class CRoomInternalLogic : public IGRCContainer
class CRoomInternalLogic : public CAbil
{
	friend class CGRCContainer;
	IMPLEMENT_TIREFCNT0( CRoomInternalLogic )
protected:


public:
	CRoomInternalLogic( const DWORD dwRID, IRoomEventHandler * pHandler );
	~CRoomInternalLogic();

protected:
	BOOL AddUser(CUser* pUser);
	void RemoveUser( LONG lCSN );

protected:
	CUser* FindUser(LONG lCSN);
	//	CUser* FindUser(xstring sLUserID);
protected:

	LONG GetUserState(LONG lCSN, LONG lType);
	//	string GetTitle() const;
	NFRoomOption& GetNFRoomOption() { return m_nfRoomOption; }
	//protected:
	//	BOOL ExistUser() { return(mUsers.size() != 0) ;  }

	void SetRoomID( const RoomID& rID );
protected:
	BOOL OnRun(const RoomID& roomID, const NFRoomOption& nfRoomOption, const string& sGameOption, IXObject * pTimerHandler );

	void OnStop();
	//	BOOL OnUserJoin(CUser* pUser);
	BOOL OnCreateRoomReq(CUser* pUser);
	//Remove user( and destroy ) and logging int

	bool ProcessExitedUser( LONG lCSN, BOOL bPrevious );
	BOOL KickOutUser(LONG lCSN, BOOL bPrevious = FALSE );
	//void OnUserDisconnect(LONG lCSN, BOOL bPrevious = FALSE);
	//Only Remove (and destroy )User 

	void OnUserMsg(LONG lCSN);
	void OnTimer(LONG lIndex);
protected:
	void OnRcvMsg(CUser* pUser, PayloadCliNGS* pPld);
	void OnRcvChangeRoomOptionNtf(LONG usn, MsgCliNGS_ChangeRoomOptionNtf* pMsg);
	void OnRcvAlive( LONG usn );
	void OnRcvChatMsgTo( LONG usn, MsgCliNGS_ChatMsgTo* pMsg);
	void OnRcvChatMsgAll( LONG usn, MsgCliNGS_ChatMsgAll* pMsg);
	// 신고
	void OnRcvAccuseReq( LONG usn, MsgCliNGS_AccuseReq* pMsg);
	void MakeContentHeader(AccuseBus& bus, string& game_name);
	// 세이클럽 접속 끊어진 경우
	void OnRcvLoginStateChangeNtf( LONG usn, MsgCliNGS_LoginStateChangeNtf* pMsg);
//	void OnRcvGetUserRankInfoReq(LONG usn, MsgCliNGS_GetUserRankInfoReq* pMsg);

protected:
	LONG SendMessageToServer(LONG lServerType,  DWORD wParam, string &sContents);

protected: // interface with Lrb
	//안쓰네?
	void OnLrbMsg();
	void OnRcvUserChatChannelExitNtf(MsgSMGWGLS_UserLeaveNtf* pMsg);
	void OnRcvDeleteRoomReq(const LRBAddress& header, MsgAMSGLS_RoomDeleteReq* pMsg);



	//	// LCS
	//	void GetUsers(LONG lPartKey, LONG lLCSCount, vector<LONG> & vecUSN, string & sOption);
	// GRC
	//안쓰네?
	void MakeOtherAddress(LRBAddress & addr, string m_sServerName, DWORD dwCastType, DWORD dwCategory);
	// GLS
#define _REMOVE_NTF_FROM_LOGIC_
#if !defined _REMOVE_NTF_FROM_LOGIC_
	void NotifyCreateRoomToChs(CUser* pUser);
	void NotifyAddUserToChs(CUser* pUser, LONG lCSN);
	void NotifyRemoveUserToChs(long lCSN, RoomID & rID );
	void NotifyRemoveRoomToChs();
	void NotifyChangeRoomOptionToChs();
	void NotifyChangeGameOptionToChs();
	void NotifyChangeRoomStateToChs(LONG lState);
	void NotifyChangeUserInfoToChs(ChangeUserGameDataList& lstUserGameData);
	// 세이클럽 접속 끊어진 경우
	void NotifyChangeLoginStateToChs(LONG lCSN, LONG lLoginState);
	void NotifyNGSInfoToNCS(BOOL bAlways = FALSE);

	void SendToCHS(const LRBAddress& dest, PayloadNGSCHS& pld);
	void SendToNCS(const LRBAddress& dest, PayloadNGSNCS& pld);
	void SendToAMS(const LRBAddress& dest, PayloadGLSAMS& pld);
	void SendToNLS(const LRBAddress& dest, PayloadCLINLS& pld);
	void SendToPLS(const LRBAddress& dest, GBuf& buf);
	void SendToIBB(const LRBAddress& dest, GBuf& buf);
#endif //#if defined _REMOVE_NTF_FROM_LOGIC_


protected :
	BOOL SendCastingData(const char *pBuf, int nLen);


protected:
	void NotifyChangeRoomOptionToClient();
	// 세이클럽 접속 끊어진 경우
	void NotifyChangeLoginStateToClient(LONG lCSN, LONG lLoginState);
	void NotifyRemoveRoomToBCS();

protected: // branch 해야 할 것들 여기다가 모아놓자. GRCContainerBranch.cpp에 넣을 것임.
	LONG RemoveTimer(LONG lIndex);
	LONG AddTimer(DWORD dwDue, DWORD dwPeriod, LONG lTimerIndex );

public:
	const RoomID & GetRoomID() const;
	const xstring & GetRoomIDStr( );

	void SetRoomAddr( const LRBAddress & Addr );
	const LRBAddress & GetRoomAddr();

	bool IsExistingUser( LONG usn )
	{
		return ( FindUser( usn ) == NULL ? false : true );
	}

	void OnCheckUserAliveTimer();
	void ResetRcvMsgCnt();

	BOOL PasswordCheck( CUser * pUser );

	void OnCutOutUser( string & sUserInfo, long lCSN, LinkCutReason reason );

	void SetIsInRoom(BOOL bFlag) { m_bIsInRoomInfoTable = bFlag; }
	BOOL GetIsInRoom() { return m_bIsInRoomInfoTable; }

	LONG GetRunStopFlag() { return m_nfRoomOption.m_lRoomStatus; }
	void SetRunStopFlag(LONG flag) { m_nfRoomOption.m_lRoomStatus = flag; }
	
	LONG GetState() const;

	void SetChatForward() { m_bChatForward = TRUE; }
	void ResetChatForward() { m_bChatForward = FALSE; }
	BOOL CheckChatForward() { return m_bChatForward; }

	void ChangeCHSAddr(const LRBAddress& oldAddr, const LRBAddress& newAddr) {
		if (m_CHSAddr == oldAddr) m_CHSAddr = newAddr;
	}
	const LRBAddress& GetCHSAddr() const { return m_CHSAddr; }
	void SetCHSAddr(const LRBAddress &chsAddr); 


	BOOL GetNextUserMsg( PayloadCliNGS & pld, LONG usn );

	BOOL IsExistPrevUser( LONG usn);
	BOOL IsOverMaxUserCnt();
	BOOL PasswordCheckOnJoin( CUser * pUser );
	const string & GetRoomPassword( ) { return m_nfRoomOption.m_sPassword; }

	BOOL OnJoinRoomReqSuccess( CUser * pUSer );
	

	BOOL OnCreateRoomNtfSuccess( LONG usn );
	BOOL OnJoinRoomNtfSuccess( LONG usn );
	
	BOOL ProcessJoinGRCFailure( LONG usn, LONG JoinResult );
	
	BOOL NotifyCreateRoomSucess( LONG usn );

	BOOL CheckPldCreateRoomNtf( LONG usn, PayloadCliNGS * pld );

	BOOL CheckPldJoinRoomNtf( LONG usn, PayloadCliNGS * pld );
	BOOL SendJoinNtfResultToAll( LONG usn );
	BOOL SendIBBAnswerToUser( LONG usn, LPXBUF buf );
	BOOL SendAccuseAnsToUser( LONG usn, CAccuseResultAnswer & AccuseResult );

	BOOL OnBroadCastMsg( const GBuf & buf );

	LONG RemoveAllTimer();
	BOOL ProcessTimerSignal( LONG & UserTimerIndex, const HSIGNAL &hObj, const WPARAM &wParam, const LPARAM &lParam, DWORD beforeLockTick );

	void OnAnnounceMsg();
	void SendAnnounceMsg();


protected:
	int DBResultParser(const DBGW_XString& strDBResult, list<xchar *>& lstData, BOOL bSP = FALSE);
	xchar *GetNextItemOfDBResult(xchar *&p);

protected:
	friend class CRoom;
protected:
//	IRoomContext* m_pGRC;

	RCPtrT<CRoomTimerManager> m_pTimerManager;

	RoomID m_RoomID;
	const DWORD m_dwRID;
	xstring m_RoomIDStr;

	LRBAddress m_addr;

//	GSafeBufQ mLrbMsg;
	GSafeBufQ mAnnounceMsg;
	LONG m_lRoomState;
	NFRoomOption m_nfRoomOption;
	string m_sGameOption;
	LONG m_lCapUSN;
	xstring m_sCapUID;
	// 신고
	ChatHistory m_ChatHistory;
	// DB update를 사용자 리스트로
	string m_sUserGameDataList;
	BOOL			m_bIsInRoomInfoTable; // RoomInfo 테이블에 insert 된 경우 TRUE;
	BOOL m_bChatForward;

	LRBAddress m_CHSAddr;

	long m_lock_cu_debug;
	long m_lock_this_debug;


	CRoomUserManager * UserManager;



	CNotifyToOthers * NotifyUtil;


// NF
protected:
	TotalLandingFish	m_totLandingFishTeamA;
	TotalLandingFish	m_totLandingFishTeamB;
	TMapTotalMapInfo	m_mapTotalMapInfo;

	LONG			m_lSignCurrentPoint;
	LONG			m_lSignMaxPoint;
	LONG			m_lSignType;
	LONG			m_lMapLodingTime;

	CUserSlot	m_UserSlot;


public:
	void SendToUser(LONG lCSN, const PayloadNGSCli& pld);

	BOOL GetNFCharInfo(CUser* pUser);
	BOOL GetRoomInfoInChannel(NFRoomInfoInChannel &roomInfo);
	void NtfStartSignMsg(LONG lSignType);
	void NtfEndSignMsg();
	void CheckInvalidItem(CUser* pUser, LONG lFighingResultType);
	BOOL DecrementEnduranceItem(CNFChar& nfChar, LONG lType, const EquipItem* pReelItem, const EquipItem* pRodItem);
	LONG GetFishDropItem(CUser* pUser, TMapInven& mapNewInven, TMapInven& mapOldInven);
	void SaveGameResult(CUser* pUser);
	BOOL PrevLoadLockedNote(CNFChar& nfChar, CFish& landingFish, NFLockedNoteMain& locked_main, LONG& lLengthIndex, LONG& lClassIndex, std::string& strFishGroup, std::string& strLength, std::string& strRegDate, std::string& strMapID);
	LONG SaveLandingFishInfoToDB(CNFChar& nfChar, CFish& landingFish);
	LONG CheckLockedNote(CNFChar& nfChar, CFish& landingFish);
	void SaveLandingResultTeam(CUser* pUser);
	BOOL GetFreeModeGaemResult(const CNFChar& nfChar, GameResult& gameResult);
	BOOL GetSingleGameResult(TLstGameResult& lstPlayer, BOOL bIsDetail=FALSE);
	BOOL GetTeamGameResult(TLstGameResult& lstPlayer, TLstGameResult& lstTeam, BOOL bIsSimple=FALSE);
	BOOL GetGameResult(CUser* pUser, TLstGameResult& lstPlayer, TLstGameResult& lstTeam, BOOL bIsSimple=FALSE, BOOL bIsUpdateDB=FALSE);
	BOOL CalcRankGameResultSingle(TLstGameResult& lstPlayer, LONG ulWinCondition);
	BOOL CalcRankGameResultTeam(TLstGameResult& lstPlayer, LONG lWinCondition);
	void SortingLandingFishResultTeam(TotalLandingFish* totResult, LONG lWinCondition, LONG lTopSize=5);
	void SortingGameResult(TLstGameResult& lst, LONG lWinCondition, LONG lTopSize=5);
	BOOL CheckSignPoint(FishInfo& biteFishInfo, LONG& lSignIndex);
	double CalcCastingDist(long lCastingType, double dUIFlyDist);
	double CalcAccuracy(long lCastingType, double dUIFlyDist);
	double CalcBacklash(long lCastingType, double dUIBacklash, double dDragIntensity, double dDragIntensityMax, double dCastingDistMax, double dRealCastingDist, LONG lLevel);
	Coordinates CalcRealCastingLocation(long lCastingType, double dMaxFlyDist, const Coordinates& UserLocation, const Coordinates& CastingLocation, long lDragIntensity, double dCastAccuracy);
	long GetInCounterScoreByCastingScore(double dCastingScore, int& nAchv_ScoreType);
	double CalcLuckyPoint(double dLuckPoint);
	double CalcHypote(Coordinates& to, Coordinates& from);
	double CalcCastingScore(Coordinates& CastingLocation, Coordinates& RealCastingLocation, NFAbility& NFAbility, double dHypote, double dCastingDist, double dCastAccuracy);
	BOOL ApplyInCounterSystem(CUser* pUser, long lInCounterType, long lAddGauge);
	void CheckLooseType(CNFChar& nfChar, double& dLineLoadPresent, double dRealLoadMax, double dRateDragLevel, LONG lCurDragLevel, MsgCliNGS_ReqFighting& RcvMsg);
	LONG SetLineLoose(CNFChar& nfChar, double& dLineLoadPresent, LONG lCurDragLevel, MsgCliNGS_ReqFighting& RcvMsg);
	void SetFishPatternByLineLength(CFish& BiteFish, CNFChar& nfChar, LONG& lFishHP);
	LONG LineBreakCheck_FeverMode(CUser* pUser, MsgNGSCli_AnsFigtingResult& SendMsg, MsgCliNGS_ReqFighting& RcvMsg, LineBreakInfo& lineBreakInfo);
	LONG LineBreakCheck(CUser* pUser, MsgNGSCli_AnsFigtingResult& SendMsg, MsgCliNGS_ReqFighting& RcvMsg, LineBreakInfo& lineBreakInfo);
	void OnTimerGNFGame();
	BOOL OnTimerStartGame(LONG lTimerDue);
	void GameOver();
	void InitTotalMap(LONG lFishMapIdx);
	BOOL SetSignPoint(LONG lFishMapIdx);
	void InitTeamPlayData();
	void InitFishing(CUser* pUser, LONG lFighingResultType=EC_FR_SUCCESS);
	double GetTirelessByLevel(long lCharLevel);
	long ApplyUseItem(CUser* pUser, LONG lItemCode, LONG lQuickSlotIndex, AnsUsedItem& usedItem);
	void SendNtfMapLoding();
	void SendNtfGameReadyProgress(CUser* pUser);
	string GetParseDataNot(string& sTarget, char* sFind);
	string GetParseData(string& sTarget, char* sFind);
	void ChatFilteringDebbungCode(CUser* pUser, xstring& sText);
	LONG ChangeFishingPoint(const FishingPoint* pFP, LONG lFishingPoint, CUser* pUser, LONG& lBoatOwnerCSN, LONG& lMovedMultiPortIndex, BOOL bOnlyRemove=FALSE);
	LONG ChangeFishingPoint_Tutorial(LONG lFishingPoint, CUser* pUser, LONG& lAnsFishingPointIndex);
	TMapTotalMapInfo& GetTotalMapInfo();
	BOOL IsCheckAllReady();
	BOOL SendNtfGameStart();
	BOOL AddUserSlot(CUser* pUser);
	LONG ChangeUserSlot(CUser* pUser, LONG lMoveSlot);
	LONG RemoveUserSlot(CUser* pUser);
	LONG GetSignType() { return m_lSignType; }
	void SetSigning(BOOL lSignType) { m_lSignType = lSignType; }
	LONG IncrementMapLoadingTime() { return ::InterlockedIncrement(&m_lMapLodingTime); }
	void InitMapLoadingTime() { m_lMapLodingTime = 0; }
	BOOL CheckValidItem(NFCharInfoExt& nfCharInfoExt, LONG lItemType);
	void CalcLineItemCountByInitFishing(CUser* pUser, FIGHTING_RESULT type, TlstInvenSlot& remainCountchangedInven);
	void CalcEnduranceItemCountByInitFishing(CUser* pUser, TlstInvenSlot& remainCountchangedInven, LONG lReduceCount, int nType);
	void CalcItemCountByInitFishing(CUser* pUser, FIGHTING_RESULT type, TlstInvenSlot& remainCountchangedInven);	
	void CalcLineItemCountByInitFishing(CUser* pUser, LONG lFighingResultType, TlstInvenSlot& remainCountchangedInven, TMapAchvFactor& mapFactorVal);
	void CalcEnduranceItemCountByInitFishing(CUser* pUser, TlstInvenSlot& remainCountchangedInven, LONG lReduceCount, int nType, TMapAchvFactor& mapFactorVal);
	void CalcItemCountByInitFishing(CUser* pUser, LONG lFighingResultType, TlstInvenSlot& remainCountchangedInven);

	void BoatOwnerLeaveThenSubRiderChangeFishingPoint(const LONG lPrevFishingPoint);
	

	// achv
	void achv_ProcessLanding(CUser* pUser);
//	void achv_ProcessLanding_fishattack(LONG l);

	// Community
	BOOL GetNFLetterList(const LONG lCSN, CONT_NF_LETTER& rkContNFLetter,const BOOL bNewLetter);
	BOOL GetNFLetterContent(const __int64 i64LetterIndex, string& rstrContent, string& rstrSendTime);
	BOOL SendNFLetter(const string& rstrReceiver, const CNFLetter& rnfLetter);
	BOOL DeleteNFLetter(const vector<__int64>& kContLetterIndex);

	BOOL GetNFFriendInfo(const LONG lCSN, CONT_NF_FRIEND& rkContNFFriend);
	BOOL GetNFCharKeyByCharName(const string& rstrCharName, TKey& rKey);
	BOOL NFFriendAccept( const string& rstrAcceptorCharName, const string& rstrApplicantCharName );
	BOOL DeleteNFFriend( const string& rstrCharName, const string& rstrDeleteCharName );

	void ResultLanding(CUser* pUser, MsgNGSCli_AnsFigtingResult& ans, LONG lLandingBonusType);
	void ResultLineBreak_FeverMode(CUser* pUser, MsgNGSCli_AnsFigtingResult& ans, MsgCliNGS_ReqFighting& reqMsg, LineBreakInfo& info);
	void ResultLineBreak(CUser* pUser, MsgNGSCli_AnsFigtingResult& ans, MsgCliNGS_ReqFighting& reqMsg, LineBreakInfo& info);

	// err_reporing
	void ErrorReportingToClient(CUser* pUser, LONG err_code);

	// tutorial
	// NFGame Message(Cli->NGS)
	void OnReqTutorial(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFCharInfo(LONG lCSN, PayloadCliNGS* pMsg);
	void OnRcvBanishReq(LONG lCSN, PayloadCliNGS* pPld);
	void OnReqBiteFishInfo(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqChangeUserInfo(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqMoveFishingPoint(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqTotalMapInfo(LONG lCSN, PayloadCliNGS* pMsg);
	void OnNFGameData(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqStartGame(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqJoinInGame(LONG lCSN, PayloadCliNGS* pMsg);
	void OnNtfGameReadyProgress(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqGameResult(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqCasting(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqCastingResult(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqAction(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqHookingResult(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqFighting(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqLanding(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqUseItem(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqGameOver(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqCorrectDirection(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqUpdateAchieveInfo(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqAchievement(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqFreeRoomList(LONG lCSN, PayloadCliNGS* pMsg);
	void OnCheatReq(LONG lCSN, PayloadCliNGS* pMsg);

	// NF_Community_Test
	void OnReqNFLetterList(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFLetterContent(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFLetterReceiverCheck(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFLetterSend(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFLetterDelete(LONG lCSN, PayloadCliNGS* pMsg);

	void OnReqNFFriendInfo(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFFriendAdd(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFFriendAccept(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFFriendReject(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFFriendDelete(LONG lCSN, PayloadCliNGS* pMsg);

	void OnReqNFBlockList(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFFriendApplicantList(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNFBlockOrUnBlock(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqFollowUser(LONG lCSN, PayloadCliNGS* pMsg);

	// CHS->NGS
	void OnAnsFreeRoomList(LONG lCSN, MsgCHSNGS_AnsFreeRoomList* pMsg);

	// NFMenu
	void OnReqChangeParts(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqBuyItem(LONG lCSN, PayloadCliNGS* pMgs);
	void OnReqRemoveItem(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqOpenCardPack(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqChangeCardSlot(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqExchangeCards(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqChangeQuickSlot(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqProductList(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqGetRewardItem(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqLandNote(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqUpdateStudyScenario(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqRepairEnduranceItem(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqNextEnchantInfo(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqItemEnchant(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqAquaInsert(LONG lCSN, PayloadCliNGS* pMsg);
	void OnReqAquaFish(LONG lCSN, PayloadCliNGS* pMsg);
// NF
};





#endif //!GRCCONTAINER_H
