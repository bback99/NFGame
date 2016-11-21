//
// GRCContainerADL.cpp
//
#include "stdafx.h"
#include "common.h"
#include "Room.h"
#include "RoomTable.h"
#include "StatisticsTable.h"
#include "RoomInternalLogic.h"
#include "UserTable.h"
#include "Agent.h"
#include <NF/ADL/MsgNFCommonStruct.h>
#include "ErrorLog.h"
#include "FishAI.h"
#include <NFVariant/NFGameData.h>
#include <NFVariant/NFDBManager.h>
#include "NotifyToOthers.h"
#include <NFVariant/NFMenu.h>
#include <NFVariant/NFItem.h>


bool CompareSize_LandingFish(const LandingFish& elem1, const LandingFish& elem2);
bool CompareWeight_LandingFish(const LandingFish& elem1, const LandingFish& elem2);

template<class T>
bool CompareCount_GameResult(const T& elem1, const T& elem2)
{
	return elem1.m_lCatchCount > elem2.m_lCatchCount;
}

template<class T>
bool CompareScore_GameResult(const T& elem1, const T& elem2)
{
	return elem1.m_lTotScore > elem2.m_lTotScore;
}

template<class T>
bool CompareMaxSize_GameResult(const T& elem1, const T& elem2)
{
	return elem1.m_dMaxSize > elem2.m_dMaxSize;
}

template<class T>
bool CompareTotSize_GameResult(const T& elem1, const T& elem2)
{
	return elem1.m_dTotSize > elem2.m_dTotSize;
}

template<class T>
bool CompareMaxWeight_GameResult(const T& elem1, const T& elem2)
{
	return elem1.m_dMaxWeight > elem2.m_dMaxWeight;
}

template<class T>
bool CompareTotWeight_GameResult(const T& elem1, const T& elem2)
{
	return elem1.m_dTotWeight > elem2.m_dTotWeight;
}

void CRoomInternalLogic::SendToUser(LONG lCSN, const PayloadNGSCli& pld)
{
	UserManager->SendToUser(lCSN, pld);
}

BOOL CRoomInternalLogic::GetNFCharInfo(CUser* pUser)
{
	//////////////////////////////////////////////////////////////////////////
	// ###################### NFUser 정보 읽어오는 부분 ######################
	NFUserBaseInfo nfUBI = pUser->GetUserData();
	pUser->GetNFUser().SetCUser(pUser);

	//////////////////////////////////////////////////////////////////////////
	// 1. CSN에 해당하는 NFChar정보 읽어오기...
	LONG lErrorCode = theNFDBMgr.SelectNFCharBaseNExteriorInfo(*(pUser->GetNFCharInfoExt()), pUser->GetGSN(), pUser->GetCSN());
	if (lErrorCode != NF::G_NF_ERR_SUCCESS)
	{
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "SelectNFCharByCSN is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
		pUser->SetErrorCode(CRF_DBERROR);
		return FALSE;
	}

	LONG lMaxExp = 0;
	theNFDataItemMgr.GetNFExp(pUser->GetLevel(), lMaxExp);
	pUser->SetMaxExp(lMaxExp);

	////////////////////////////////////////////////////////////////////////////
	//// 2. CSN에 해당하는 Inven정보 읽어오기
	TlstInvenSlot lst;
	if (!theNFDBMgr.SelectNFCharInven(lst, pUser->GetLastestLogOutDate(), pUser->GetNFCharInfoExt()->m_nfCharInven, pUser->GetGSN(), pUser->GetCSN(), lErrorCode))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "SelectNFCharInvenByCSN is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}
	else
	{
		if (NF::G_NF_ERR_SUCCESS != lErrorCode)
			theLog.Put(DEV_UK, "NGS_Null"_COMMA, "----- SelectNFCharInvenByCSN is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN(), ", Err :", lErrorCode);
	}

	// 수족관 물고기 정보
	if (!theNFMenu.GetAquaFish(pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfAquaFish))
	{
		pUser->SetErrorCode(EC_NA_NOT_FOUND_AQUA_FISH);
		theLog.Put(ERR, "theNFMenu.GetAquaFish is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	// WORKING(acepm83@neowiz.com) 능력치를 셋팅하기전에 수족관 정보를 읽어야 한다.(Beacause 수족관 버프)

	LONG lElapsedClearHour = 0; // 청소 경과시간
	LONG lElapsedFeedHour = 0;	// 밥준 경과시간

	if (!theNFDBMgr.SelectNFCharAqua(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfAqua, lElapsedClearHour, lElapsedFeedHour))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(ERR, "SelectNFCharAqua is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}
	else
	{
		theNFMenu.CalcNFAquaGauge(pUser->GetNFCharInfoExt(), lElapsedClearHour, lElapsedFeedHour);
	}

	lErrorCode = theNFItem.GetCharAbility(pUser->GetNFCharInfoExt());
	if (lErrorCode != NF::G_NF_ERR_SUCCESS)
		pUser->SetErrorCode(lErrorCode);

	// 환경속성 디버프를 체크한다. 여기서 하면 안 되고, InGame에 들어가서 설정해준다.
	pUser->SetEnvDebuff(theNFMenu.CheckEnvDebuff(pUser->GetNFCharInfoExt(), m_nfRoomOption.m_lEnvAttribute));

	////////////////////////////////////////////////////////////////////////
	// 4-1. Aquarium
	
	
	// 4-2. LockedNote(+Main) 정보 읽기
	if (!theNFDBMgr.SelectNFLockedNote(pUser->GetGSN(), pUser->GetCSN(), pUser->GetLockedNote()))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "SelectNFLockedNote is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	// 5-1. 업적 정보 읽어오기
	if (!theNFDBMgr.SelectNFAchvList(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfCharAchievement.m_nfCharAchieve, lErrorCode))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "SelectNFAchvList is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	// 5-2. 업적 포인트 정보 읽어오기
	if (!theNFDBMgr.SelectNFAchvPoint(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfCharAchievement.m_nfCharAP))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "SelectNFAchvPoint is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	return TRUE;
}

BOOL CRoomInternalLogic::AddUserSlot(CUser* pUser)
{
	// NF게임에서 필요로 하는 UserSlot 추가
	LONG lIndex = m_UserSlot.AddSlot(pUser->GetCSN());
	if (lIndex == -1) return FALSE;
	pUser->SetUserSlot(lIndex);
	
	if (lIndex > -1)
		return TRUE;
	return FALSE;
}

LONG CRoomInternalLogic::ChangeUserSlot(CUser* pUser, LONG lMoveSlot)
{
	// NF게임에서 필요로 하는 UserSlot 추가
	LONG lIndex = m_UserSlot.ChangeSlot(pUser->GetUserSlot(), lMoveSlot, pUser->GetCSN());
	if (lIndex == -1) return -1;
	pUser->SetUserSlot(lIndex);
	return lIndex;
}

LONG CRoomInternalLogic::RemoveUserSlot(CUser* pUser)
{
	// NF게임에서 필요로 하는 UserSlot 추가
	LONG lIndex = m_UserSlot.RemoveSlot(pUser->GetUserSlot(), pUser->GetCSN());
	if (lIndex == 1) return FALSE;
	pUser->SetUserSlot(lIndex);
	return lIndex;
}

string CRoomInternalLogic::GetParseDataNot(string& sTarget, char* sFind)
{
	string sRet;
	size_t nIndex = sTarget.find_first_not_of(sFind);
	if ( nIndex != NPOS )
	{
		sRet = sTarget.substr(0, nIndex);
		sTarget.erase( 0, nIndex + 1 );
	}
	else
	{
		sRet = sTarget;
		sTarget.erase();
	}
	return sRet;
}

string CRoomInternalLogic::GetParseData(string& sTarget, char* sFind)
{
	string sRet;
	size_t nIndex = sTarget.find_first_of(sFind);
	if ( nIndex != NPOS )
	{
		sRet = sTarget.substr(0, nIndex);
		sTarget.erase( 0, nIndex + 1 );
	}
	else
	{
		sRet = sTarget;
		sTarget.erase();
	}
	return sRet;
}


void CRoomInternalLogic::InitTotalMap(LONG lFishMapIdx)
{	
	m_mapTotalMapInfo.clear();

	FishMap* pFishMap = theNFDataItemMgr.GetFishMapByIndex(lFishMapIdx);
	if (pFishMap)
	{
		ForEachElmt(TMapFishingPoint, pFishMap->m_mapFishingPoint, it, ij)
		{
			FPInfo addNFFPInfo;
			addNFFPInfo.Clear();

			addNFFPInfo.m_lFishingPoint = (*it).second.m_lIndex;
			addNFFPInfo.m_lFPType = (*it).second.m_lPointerType;
			addNFFPInfo.m_lFPStatus = 1;
			if (addNFFPInfo.m_lFPType == 2)	// MultiPort
			{
				addNFFPInfo.m_vecMultiPort.reserve((*it).second.m_lMaxUsers);
				for(int i=0; i<(int)(*it).second.m_lMaxUsers; i++)
					addNFFPInfo.m_vecMultiPort.push_back(0);
			}

			m_mapTotalMapInfo[(*it).first] = addNFFPInfo;
		}
	}
}

BOOL CRoomInternalLogic::SetSignPoint(LONG lFishMapIdx)
{
	FishMap* pFishMap = theNFDataItemMgr.GetFishMapByIndex(lFishMapIdx);
	if (pFishMap)
	{
		m_lSignMaxPoint = pFishMap->m_lSignMapPoint;
		return TRUE;
	}
	return FALSE;
}

LONG CRoomInternalLogic::ChangeFishingPoint_Tutorial(LONG lFishingPoint, CUser* pUser, LONG& lAnsFishingPointIndex)
{
	// 튜토리얼로 들어올 경우 처음에 Level_UP을 요청한다.
	ArcMapT< std::string,LONG > mapFactorVal;
	mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_LEVEL], pUser->GetLevel()));
	g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_LEVEL, GetRoomID(), mapFactorVal);

	LONG lErrorCode = NF::G_NF_ERR_SUCCESS;

	// 비어 있는 포인트가 있으면 찾아본다.
	ForEachElmt(TMapTotalMapInfo, m_mapTotalMapInfo, it, ij)
	{
		if ((*it).second.m_lstFPUserList.size() <= 0)
		lAnsFishingPointIndex = (*it).first;
		break;
	}

	// 모든 포인트에 한명씩 차있으면, 아무거나 검색 먼저 되는 순서로 찾는다. 
	if (lAnsFishingPointIndex <= 0)
	{
		ForEachElmt(TMapTotalMapInfo, m_mapTotalMapInfo, it, ij)
		{
			if ((*it).second.m_lFPStatus != 0)
			{
				lAnsFishingPointIndex = (*it).first;
				break;
			}
		}
	}
	
	if (lAnsFishingPointIndex > 0)
	{
		FishingPoint* pFP = theNFDataItemMgr.GetFishingPointByIndex(m_nfRoomOption.m_lIdxFishMap, lAnsFishingPointIndex);
		if (pFP)
		{
			LONG lDummy = 0;
			lErrorCode = ChangeFishingPoint(pFP, lAnsFishingPointIndex, pUser, lDummy, lDummy);
		}
		else
			lErrorCode = NF::G_NF_ERR_MOVE_FP_TUTORIAL_NOT_FOUND;
	}
	else
		lErrorCode = NF::G_NF_ERR_MOVE_FP_TUTORIAL_FULL;

	return lErrorCode;
}


// Boat 설정 추가 : 2010.02.08
// Sequence 
// 1. PointType이 Walk 이면, MaxUsers 만 체크한다.
// 2. PointType이 Boat 이면, 
//		2-1. 처음 들어오고, Boat Owner이냐? -> 성공
//		2-2. 처음 들어오지만, Boat Owner가 아니다. -> 실패
//		2-3. 두번째 이후 부터는 Boat의 Owner에 상관 없이 MaxUser로 체크만한다.
// 3. bOnlyRemove가 TRUE인 경우는 lFishingPoint는 remove하는 FishingPoint이다.
// 4. PointType이 MultiPort 이면, FishingPoint의 m_vecMultiPort를 쓴다
LONG CRoomInternalLogic::ChangeFishingPoint(const FishingPoint* pFP, LONG lFishingPoint, CUser* pUser, LONG& lBoatOwnerCSN, LONG& lMovedMultiPortIndex, BOOL bOnlyRemove)
{
	LONG lErrorCode = NF::G_NF_ERR_SUCCESS;

	CNFChar& nfChar = pUser->GetNFUser(); 
	FPInfo addNFFPInfo;
	addNFFPInfo.Clear();

	if (!bOnlyRemove) {
		// 옮기려는 포인트가 같으면...
		if (pUser->GetPrevFishingPoint() == lFishingPoint)
			return NF::G_NF_ERR_MOVE_FP_SAME;

		// Check FishingPoint Valid
		TMapTotalMapInfo::iterator iterAdd = m_mapTotalMapInfo.find(lFishingPoint);
		if (iterAdd == m_mapTotalMapInfo.end())
			return NF::G_NF_ERR_MOVE_FP_NOT_FOUND;	// TotalMap에 추가가 안 되어 있는 FishingPoint이다.

		addNFFPInfo = (*iterAdd).second;
		if (addNFFPInfo.m_lFPStatus == 0)		// 이동 불가 상태이면, 에러....
			return NF::G_NF_ERR_MOVE_FP_IMPOSSIBLE;		// FishingPoint가 풀이다.
	}

	bool isBoatOwnerLeave = false;
	LONG lPrevFishingPoint = pUser->GetPrevFishingPoint();

	// Remove , 기존꺼 먼저 지우고
	if (pUser->GetPrevFishingPoint() != -1) 
	{
		TMapTotalMapInfo::iterator iterRemove = m_mapTotalMapInfo.find(lPrevFishingPoint);
		if (iterRemove != m_mapTotalMapInfo.end())
		{
			FPInfo& nfFPInfo = (*iterRemove).second;

//			theLog.Put(DEV_UK, "NGS_LOGIC, ChangeFishingPoint(). Before Remove UI, CNT:", nfFPInfo.m_lstFPUserList.size());
//			ForEachElmt(TlstFPUserList, nfFPInfo.m_lstFPUserList, it, ij)
//				theLog.Put(DEV_UK, "NGS_LOGIC, ChangeFishingPoint(). Before CSN : ", (*it).m_lCSN);

			ForEachElmt(TlstFPUserList, nfFPInfo.m_lstFPUserList, it, ij)
			{
				if ((*it).m_lCSN == pUser->GetCSN())
					break;
			}

			if (it != nfFPInfo.m_lstFPUserList.end())
			{
				if (nfFPInfo.m_lFPType == 0)		// walk 이면, 
					nfFPInfo.m_lFPStatus = 1;		
				else if (nfFPInfo.m_lFPType == 1)	// boat 이면, 
				{
					// 보트 오너가 나갔을 경우...	초기화, Todo : 해당 보트에 타고 있던 모든 유저들을 내보내야 한다.....
					if (nfFPInfo.m_lBoatOwnerCSN == pUser->GetCSN()) {
						nfFPInfo.m_lBoatOwnerCSN = 0;
						nfFPInfo.m_lBoatMaxUsers = 0;
						nfFPInfo.m_lFPStatus = 1;

						isBoatOwnerLeave = true;
					}
					else
						nfFPInfo.m_lFPStatus = 2;
				}
				else if (nfFPInfo.m_lFPType == 2)	// multi-port 이면, 
				{
					nfFPInfo.m_lFPStatus = 1;
				
					for(int i=0; i<pFP->m_lMaxUsers; i++)
					{
						if (nfFPInfo.m_vecMultiPort[i] == pUser->GetCSN())
							nfFPInfo.m_vecMultiPort[i] = 0;
					}
				}
				nfFPInfo.m_lstFPUserList.erase(it);
			}

//			theLog.Put(DEV_UK, "NGS_LOGIC, ChangeFishingPoint(). After Remove UI, CNT:", nfFPInfo.m_lstFPUserList.size());
//			ForEachElmt(TlstFPUserList, nfFPInfo.m_lstFPUserList, it, ij)
//				theLog.Put(DEV_UK, "NGS_LOGIC, ChangeFishingPoint(). After CSN : ", (*it).m_lCSN);
		}
	}

	if (!bOnlyRemove)
	{
		// Add 
		// 추가하려는 UserList보다 FP의 사이즈가 넘치면 무조건 에러
		// 보트의 수용인원 적용 2010.2.11
		// FishingPoint의 MaxUsers보다 Boat의 MaxUser가 클 순 없다.
		// Max값을 체크하기전에, 수중 포인트인지 체크하고 수중 포인트일 d경우에 보트 오너의 보트정보에서 NFFPInfo에 MaxUser값을 NF저장한다.
		FPUserInfo		newFPUI;
		newFPUI.m_lCSN = pUser->GetCSN();

		LONG lMaxUsers = pFP->m_lMaxUsers;
		// 보트포인트 체크
		if (addNFFPInfo.m_lFPType == 1)		// Boat
		{
			if (addNFFPInfo.m_lstFPUserList.size() <= 0)		// 처음들어온다.
			{
				EquipItem* pBoatItem = nfChar.GetBoatEquipItem();
				if (!pBoatItem)
					return 0;

				newFPUI.m_lStatus = 1;		// Boat Owner
				addNFFPInfo.m_lBoatOwnerCSN = pUser->GetCSN();
				addNFFPInfo.m_lBoatMaxUsers = pBoatItem->m_lBoatMaxUsers;
			}
			else
				newFPUI.m_lStatus = 2;		// Boat Sub-Rider

			lMaxUsers = addNFFPInfo.m_lBoatMaxUsers;
			addNFFPInfo.m_lFPStatus = 2;	// 보트가 있어서 이동 가능하다.
			lErrorCode = NF::G_NF_ERR_SUCCESS;
		}
		else if (addNFFPInfo.m_lFPType == 0)	// Walker
			newFPUI.m_lStatus = 0;			
		else if (addNFFPInfo.m_lFPType == 2)	// Multi-Port
		{
			for(int i=0; i<(int)pFP->m_lMaxUsers; i++)
			{
				if (addNFFPInfo.m_vecMultiPort[i] == 0) {
					addNFFPInfo.m_vecMultiPort[i] = pUser->GetCSN();
					lMovedMultiPortIndex = i;
					break;
				}
			}
		}

		// map에 추가
		addNFFPInfo.m_lstFPUserList.push_back(newFPUI);

		// Full인지 체크 (Boat일 경우는 boat의 값으로 비교를..)
		if (lMaxUsers <= (LONG)addNFFPInfo.m_lstFPUserList.size())
			addNFFPInfo.m_lFPStatus = 0;	// 보트든, Walk든 Max가 되면 이동 불가!!!
		
		m_mapTotalMapInfo[lFishingPoint] = addNFFPInfo;
		lBoatOwnerCSN = addNFFPInfo.m_lBoatOwnerCSN;
		lErrorCode = NF::G_NF_ERR_SUCCESS;

		// 성공
		nfChar.SetFPStatus(newFPUI.m_lStatus);
		pUser->SetCurrentFishingPoint(lFishingPoint);
	}

	if (isBoatOwnerLeave)
	{
		BoatOwnerLeaveThenSubRiderChangeFishingPoint(lPrevFishingPoint);
	}

	return lErrorCode;
}

void CRoomInternalLogic::BoatOwnerLeaveThenSubRiderChangeFishingPoint(const LONG lPrevFishingPoint)
{
	TMapTotalMapInfo::iterator prevItor = m_mapTotalMapInfo.find(lPrevFishingPoint); // 보트주인이 나가기전에 있던 피싱포인트
	if (prevItor != m_mapTotalMapInfo.end())
	{
		FPInfo& nfPrevFPInfo = (*prevItor).second;

		ForEachElmt(TlstFPUserList, nfPrevFPInfo.m_lstFPUserList, it, ij) // 보트에 타고있던 유저들
		{
			TMapTotalMapInfo::iterator itor = m_mapTotalMapInfo.begin(); // 인원이 꽉차지 않은 피싱포인트를 찾는다
			while (m_mapTotalMapInfo.end() != itor)
			{
				FPInfo& nfFPInfo = (*itor).second;
				if (nfFPInfo.m_lFPStatus != 0) // 갈 수 있는 피싱포인트
				{
					if (nfFPInfo.m_lFPStatus == 1) // 보트?
					{
						CUser* pUser = UserManager->FindUser((*it).m_lCSN);
						if (pUser)
						{
							TMapInven::iterator findItor = pUser->GetNFCharInfoExt()->m_nfCharInven.m_mapUsingItem.find(eItemType_Boat);
							if (findItor == pUser->GetNFCharInfoExt()->m_nfCharInven.m_mapUsingItem.end())
							{
								++itor;
								continue; // 보트가 없으면 수중포인트는 못간다.
							}
						}
					}

					// 유저가 피싱포인트 변경 패킷 보낸것 처럼 처리
					MsgCliNGS_ReqMoveFishingPoint req;
					req.m_lCSN = (*it).m_lCSN;
					req.m_lFPIndex = nfFPInfo.m_lFishingPoint;
					req.m_bIsBoatOwnerLeave = TRUE;
					PayloadCliNGS pld(PayloadCliNGS::msgReqMoveFishingPoint_Tag, req);
					OnReqMoveFishingPoint((*it).m_lCSN, &pld);

					return; // 지우지마세요!
				}

				++itor;
			}
		}
	}
}

TMapTotalMapInfo& CRoomInternalLogic::GetTotalMapInfo()
{
	return m_mapTotalMapInfo;
}

BOOL CRoomInternalLogic::IsCheckAllReady()
{
	return UserManager->IsCheckMapLoading();
}

BOOL CRoomInternalLogic::SendNtfGameStart()
{
	if (ROOMSTATE_READY != GetRunStopFlag())
		return FALSE;

	if (IsCheckAllReady())
	{
	 	MsgNGSCli_NotifyStartGame		ans;
	 	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	
	 	PayloadNGSCli pld(PayloadNGSCli::msgNotifyStartGame_Tag, ans);
	 	UserManager->SendToAllUser(pld);
	
	 	SetRunStopFlag(ROOMSTATE_START);
		
		SYSTEMTIME systime;
		memset(&systime, 0, sizeof(SYSTEMTIME));
		::GetLocalTime(&systime);
		string strTemp = ::format("%04d-%02d-%02d %02d:%02d:%02d ", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
		theLog.Put(WAR_UK, "NGS_LOGIC, ########## SendNtfGameStart : ", strTemp.c_str());
		InitMapLoadingTime();

		NotifyUtil->NotifyChangeRoomStateToChs(ROOMSTATE_START);
	 	return TRUE;
	}
	return FALSE;
}

// 맵로딩 진행 정보를 유저들에게 날리는 함수
void CRoomInternalLogic::SendNtfGameReadyProgress(CUser* pUser)
{
	MsgNGSCli_NtfGameReadyProgress		ntf;

	ntf.m_lstMapLoadingProgress.push_back(pUser->GetNFUser().GetMapLodingProgress());

	PayloadNGSCli pld(PayloadNGSCli::msgNtfGameReadyProgress_Tag, ntf);
	UserManager->SendToAllUser(pld);
}

// 맵로딩을 시작하라고 유저들에게 날리는 함수
void CRoomInternalLogic::SendNtfMapLoding()
{
	MsgNGSCli_NtfMapLoading		ntf;
	PayloadNGSCli pld(PayloadNGSCli::msgNtfMapLoading_Tag, ntf);
	UserManager->SendToAllUser(pld);
}

// GameOver
void CRoomInternalLogic::GameOver()
{
	theLog.Put(WAR_UK, "NGS_LOGIC, GameOver() GameOver!!!  ", RoomID2Str(GetRoomID()));
	SYSTEMTIME systime;
	memset(&systime, 0, sizeof(SYSTEMTIME));
	::GetLocalTime(&systime);
	string strTemp = ::format("%04d-%02d-%02d %02d:%02d:%02d ", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	theLog.Put(WAR_UK, "NGS_LOGIC, ########## GameOver : ", strTemp.c_str());


	SetRunStopFlag(ROOMSTATE_RUN);
	NotifyUtil->NotifyChangeRoomStateToChs(ROOMSTATE_RUN);
	NotifyChangeRoomOptionToClient();

	// 게임 종료후 결과 메세지를 전송한다.
	MsgNGSCli_NotifyGameResult		notify;
	if (!GetGameResult(NULL, notify.m_lstPlayer, notify.m_lstTeam, FALSE, TRUE))		// UpdateDB
		theLog.Put(WAR_UK, "NGS_LOGIC, GameOver(). GetGameResult Failed. RoomID : ", RoomID2Str(GetRoomID()));

	PayloadNGSCli pld(PayloadNGSCli::msgNotifyGameResult_Tag, notify);
	UserManager->SendToAllUser(pld);

	// 대전 관련된 내용을 초기화한다.
	InitTeamPlayData();

	MsgNGSCli_NotifyChangeUserInfo	ntf;
	ntf.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	ntf.m_lCSN = -1;
	ntf.m_lInfoType = 2;
	ntf.m_lMoveUserSlot = 0;
	ntf.m_lUserStatus = UIS_INVALID;

	// Room-Lobby에 있는 유저들의 상태를 서버가 변경한다.
	PayloadNGSCli pld2(PayloadNGSCli::msgNotifyChangeUserInfo_Tag, ntf);
	UserManager->SendToAllUser(pld2);
}


// 대회가 끝났을 때 Room과 Room안의 유저들의 초기 내용을 초기화 하는 함수
void CRoomInternalLogic::InitTeamPlayData()
{
	// 징조값 초기화
	if (GetSignType() >= 0 )
		RemoveTimer(TIMER_INDEX_SIGN);

	m_lSignType = 0;
	m_lSignCurrentPoint = 0;
	m_lSignMaxPoint = 0;
	
	//
	m_totLandingFishTeamA.Clear();
	m_totLandingFishTeamB.Clear();
	m_mapTotalMapInfo.clear();


	UserManager->InitTeamPlayData();

}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// NF Game Logic
// 캐스팅 비거리(소수점 한자리)
double CRoomInternalLogic::CalcCastingDist(long lCastingType, double dUIFlyDist)
{
	if (dUIFlyDist < AI_CASTING_FLY_DIST_MIN)
		dUIFlyDist = AI_CASTING_FLY_DIST_MIN;	// 최소값 보정

	if (dUIFlyDist > AI_CASTING_FLY_DIST_MAX)
		dUIFlyDist = AI_CASTING_FLY_DIST_MAX;	// 최대값 보정

	// 드래그 캐스팅 비거리(소수점 한자리)
	// (SQRT(비거리)*2.4) + 8
	double dDragCastingTemp = sqrt(dUIFlyDist) * AI_DRAG_CASTING_RATE + AI_DRAG_CASTING_ADD_DIST;
	double dCastingDist = ROUND(dDragCastingTemp, 1);

	if (2 == lCastingType)		// ClickCasting이면
	{
		// 클릭 캐스팅 비거리(소수점 한자리)
		// 2011.09.26 변경: // 드래그 캐스팅 값 * 0.8
		double dClickCastingTemp = dCastingDist * AI_CLICK_CASTING_RATE;
		dCastingDist = ROUND(dClickCastingTemp, 1);
	}
	return dCastingDist;
}

// 캐스팅 반지름 산출 (소수점 두자리)
double CRoomInternalLogic::CalcAccuracy(long lCastingType, double dUIFlyDist)
{
	if (dUIFlyDist < AI_CASTING_FLY_DIST_MIN)
		dUIFlyDist = AI_CASTING_FLY_DIST_MIN;	// 최소값 보정

	if (dUIFlyDist > AI_CASTING_FLY_DIST_MAX)
		dUIFlyDist = AI_CASTING_FLY_DIST_MAX;	// 최대값 보정

	// 드래그 캐스팅 반지름 산출 (소수점 두자리, m)
	double dDragAccuracyTemp = (double)((AI_CASTING_ACC-(sqrt(dUIFlyDist)*AI_DRAG_CASTING_ACC_RATE)+AI_DRAG_CASTING_ACC_ADD))/100;
	double dCastAccuracy = (double)(ROUND(dDragAccuracyTemp, 2));

	if (2 == lCastingType)		// ClickCasting이면
	{
		// 클릭 캐스팅 반지름 산출 (소수점 두자리)
		double dClickAccuracyTemp = dCastAccuracy * AI_CLICK_CASTING_ACC_RATE;
		dCastAccuracy = ROUND(dClickAccuracyTemp, 2);
	}
	return dCastAccuracy;
}

//// 캐스팅 백러쉬 및 줄꼬임 확률
//double CRoomInternalLogic::CalcBacklash(long lCastingType, double dUIBacklash, double dDragIntensity, double dDragIntensityMax, double dCastingDistMax, double dRealCastingDist, LONG lLevel)
//{
//	// 드래그 캐스팅 백러쉬 및 줄꼬임 확률
//	// 2011.09.26 변경
//	double dBacklash = ROUND(75 - (sqrt(dUIBacklash) * 4.4 ), 1);
//	double dBachlashResult = dBacklash;
//
//	if (dDragIntensity > dDragIntensityMax )
//		dDragIntensity = dDragIntensityMax;
//
//	double dDragRate = 0.0f;
//	if (dDragIntensityMax > 0)
//	{
//		dDragRate = dDragIntensity / dDragIntensityMax;
//		if (dDragRate > 0)
//			dBachlashResult *= dDragRate;
//	}
//
//	if (2 == lCastingType)		// ClickCasting이면
//	{
//		// 클릭 캐스팅 백러쉬 및 줄꼬임 확률
//		dBachlashResult = dBacklash / 2;
//
//		if (dRealCastingDist > dCastingDistMax)
//			dRealCastingDist = dCastingDistMax;
//
//		double dFlyDistRate = dRealCastingDist / dCastingDistMax;
//		if (dFlyDistRate > 0)
//			dBachlashResult *= dFlyDistRate;
//	}
//
//	// 레벨에 따른 보정
//	if( 1 <= lLevel && lLevel <= 10 ) // 비기너
//	{
//		dBachlashResult -= 20;
//	}
//	else if( 11 <= lLevel && lLevel <= 20 ) // 루키
//	{
//		dBachlashResult -= 10;
//	}
//
//	return dBachlashResult < 0 ? 0 : dBachlashResult;
//}

Coordinates CRoomInternalLogic::CalcRealCastingLocation(long lCastingType, double dMaxFlyDist, const Coordinates& UserLocation, const Coordinates& CastingLocation, long lDragIntensity, double dCastAccuracy)
{
	Coordinates	ResultCastingLocation;
	ResultCastingLocation.Clear();

	//double dGapX = CastingLocation.m_dX - UserLocation.m_dX;
	//double dGapY = CastingLocation.m_dY - UserLocation.m_dY;

	//// 두 점간의 길이
	//double dPointDist = sqrt(dGapX*dGapX + dGapY*dGapY);

	//double dDragIntensity = (double)lDragIntensity / 300; 
	//if (dDragIntensity >= 1.0)
	//	dDragIntensity = 1.0;

	//double dRealDist = 0.0;
	//if (1 == lCastingType)			// 드래그...
	//	// 던질 수 있는 거리에서 강도 비율을 곱한다.
	//	dRealDist = dMaxFlyDist * dDragIntensity;
	//else							// 클릭
	//	// 두 점간의 길이
	//	dRealDist = dPointDist;

	//// 실제 거리와 찍은 거리를 비율로 구한다.
	//ResultCastingLocation.m_dX = UserLocation.m_dX + dGapX/dPointDist * dRealDist;
	//ResultCastingLocation.m_dY = UserLocation.m_dY + dGapY/dPointDist * dRealDist;


	
	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);

	// y <= sqrt( r^2 - x^2 )
	// rand() % y
	int nCastAccuracyCm = (int)(dCastAccuracy * 100);			// m단위 이기 때문에, 소수점이 나오므로 cm 단위로 바꿔서 계산...
	if (nCastAccuracyCm <= 0)
		nCastAccuracyCm = 1;
	double dRandX = (double)(urandom(nCastAccuracyCm));		
	int nRandY = (int) sqrt(double((nCastAccuracyCm*nCastAccuracyCm) - (dRandX*dRandX)));
	if (nRandY <= 0)
		nRandY = 1;
	double dRandY = (double)(urandom(nRandY));

	// 클라이언트 좌표는 10cm단위 맞춰야 한다.
	double dRandResultX = ROUND((double)dRandX, 1);
	double dRandResultY = ROUND((double)dRandY, 1);

	// 랜덤하게 계산된 곳에서 x, y의 위치를 다시 랜덤하게 정한다. 0이면 -, 1이면 +
	int nXSign = urandom(2);
	if (!nXSign)
		dRandResultX = -1 * dRandResultX;

	int nYSign = urandom(2);
	if (!nYSign)
		dRandResultY = -1 * dRandResultY;

	//ResultCastingLocation.m_dX = ResultCastingLocation.m_dX + dRandResultX/100;
	//ResultCastingLocation.m_dY = ResultCastingLocation.m_dY + dRandResultY/100;

	ResultCastingLocation.m_dX = CastingLocation.m_dX + dRandResultX/100;
	ResultCastingLocation.m_dY = CastingLocation.m_dY + dRandResultY/100;
	ResultCastingLocation.m_dZ = CastingLocation.m_dZ;

	return ResultCastingLocation;
}

// CastingScore로 InCounterScore 가져오는 함수
long CRoomInternalLogic::GetInCounterScoreByCastingScore(double dCastingScore, int& nAchv_ScoreType)
{
	long lIncounterGauge = 0;

	if (dCastingScore < CS_NOVALUE)
		lIncounterGauge = 0;
//	else if (dCastingScore <= CS_NOTBAD) {
//		lIncounterGauge = 7;	nAchv_ScoreType = achv::EVT_Casting_Score_NOT_BAD;
//	}
	else if (dCastingScore <= CS_COOL) {
		lIncounterGauge = 11;	nAchv_ScoreType = CS_COOL;
	}
	else if (dCastingScore <= CS_GOOD) {
		lIncounterGauge = 17;	nAchv_ScoreType = CS_GOOD;
	}
	else if (dCastingScore <= CS_GREAT) {
		lIncounterGauge = 25;	nAchv_ScoreType = CS_GREAT;
	}
	else if (dCastingScore <= CS_EXCELLENT)	{
		lIncounterGauge = 33;	nAchv_ScoreType = CS_EXCELLENT;
	}
	else if (dCastingScore <= CS_PERFECT) {
		lIncounterGauge = 42;	nAchv_ScoreType = CS_PERFECT;
	}
	else if (dCastingScore <= CS_WONDERFUL) {
		lIncounterGauge = 51;	nAchv_ScoreType = CS_WONDERFUL;
	}
	//else									// update 2
	//	lIncounterScore = 70;

	return lIncounterGauge;
}

// Lucky Point 계산
double CRoomInternalLogic::CalcLuckyPoint(double dLuckPoint)
{
	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);

	long lLuckyPoint = (long) ( ( dLuckPoint * 0.3 ) + 5 ) * 10;		// 소수점 한자리를 없애기 위해서
	long lRandLucky = urandom(1000);		// 소수점 한자리를 증가 했으므로, 100% 확률이 아니라 1000% 확률로 계산
	if (lLuckyPoint > lRandLucky)		// 확률 범위 안에 들어온 값이므로 확률 적용
		dLuckPoint = 1.2 + (urandom(9)) * 0.1;
	return dLuckPoint;
}

// 좌표to 에서 좌표from까지의 거리 구하기
// 유저위치, 캐스팅위치 좌표중 큰쪽에서 작은 쪽을 뺀 값의 x, y 값을 밑변, 높이로 하여 직각 삼각형의 빗변의 길이가 실제던진 거리이다.
// 밑변, 높이
double CRoomInternalLogic::CalcHypote(Coordinates& to, Coordinates& from)
{
	return sqrt( (to.m_dX - from.m_dX)*(to.m_dX - from.m_dX) + (to.m_dY - from.m_dY)*(to.m_dY - from.m_dY) + (to.m_dZ - from.m_dZ)*(to.m_dZ - from.m_dZ) );
}

// 캐스팅 스코어 구하기
double CRoomInternalLogic::CalcCastingScore(Coordinates& CastingLocation, Coordinates& RealCastingLocation, NFAbility& NFAbility, double dHypote, double dCastingDist, double dCastAccuracy)
{
	double dHypoteGap = CalcHypote(CastingLocation, RealCastingLocation);
	if (dHypoteGap <= 0)
		dHypoteGap = 0.01;
	double dCastingScore = ( NFAbility.m_dFlyDist * (dHypote/dCastingDist) * AI_CASTING_SCORE_RATE ) + ( NFAbility.m_dFlyDist * (dCastAccuracy/dHypoteGap) );

	if (dCastingScore < AI_CASTING_SCORE_MIN)
		dCastingScore = AI_CASTING_SCORE_MIN;

	if (dCastingScore > AI_CASTING_SCORE_MAX)
		dCastingScore = AI_CASTING_SCORE_MAX;

	return dCastingScore;
}

// InCounterSystem
// FALSE : InCounter Score 
BOOL CRoomInternalLogic::ApplyInCounterSystem(CUser* pUser, long lInCounterType, long lAddGauge)
{
	CNFChar& nfChar = pUser->GetNFUser();
	BOOL bRet = FALSE;

	switch(lInCounterType)
	{
		// +
	case ICS_CASTING_SUCCESS:
		{
			nfChar.IncounterGaugeIncrement(lAddGauge);
			break;
		}
	case ICS_CHARM:
		{		
			//nfChar.IncounterGaugeIncrement(25);
			nfChar.IncounterGaugeIncrement(lAddGauge*3);
			bRet = nfChar.IsIncounterGaugeFull();			// 인카운터 게이지가 풀인지 체크
			break;
		}
		// -
	case ICS_CASTING_FAIL:
		{
			long lGauge = (long)ROUND(nfChar.GetIncounterGauge()*0.2, 1);
			nfChar.IncounterGaugeDecrement(lGauge);
			break;
		}
	case ICS_BITE_FAIL:
		{
			long lGauge = (long)ROUND(nfChar.GetIncounterGauge()*0.4, 1);
			nfChar.IncounterGaugeDecrement(lGauge);
			break;
		}
	case ICS_HOOKING_FAIL:
		{
			long lGauge = (long)ROUND(nfChar.GetIncounterGauge()*0.5, 1);
			nfChar.IncounterGaugeDecrement(lGauge);
			break;
		}
	case ICS_BITE_SUCCESS:
		{
			nfChar.SetIncounterGauge(0);
			break;
		}	
	default:
		break;
	}
	theLog.Put(DEV_UK, "NGS_LOGIC, ApplyInCounterSystem(). CSN: ", nfChar.GetCSN(), ", IncounterGauge: ", lAddGauge, ", Tot_Gauge :", nfChar.GetIncounterGauge());
	return bRet;
}

// 1. Fish AI Timer가 1초 간격으로 이벤트 발생시 호출하는 함수
// 타이머 발생시, FishAI Queue에 등록되어 있는 User들을 차례대로 불러 ans메세지를 전송한다.
// 2. 대회 남은 시간을 표시해주는 타이머, 대회 도중에 난입한 사람들에게 남은 시간을 알려주기 위해서 필요
// 3. 맵로딩시, 체크 하여 일정 기간 지나도 처리 되지 않은 유저 걸러내기

void CRoomInternalLogic::OnTimerGNFGame()
{
	// 게임 중, 남은 시간 체크
	if (ROOMSTATE_START == GetRunStopFlag())
	{
		// 남아있는 시간이 설정한 게임시간보다 커지면 게임오버!!! (설정한 시간의 단위는 분)
		if (--m_nfRoomOption.m_lRemainTime <= 0)
		{
			m_nfRoomOption.m_lRemainTime = 0;
			GameOver();
		}
	}
	// MapLoading
	else if (ROOMSTATE_READY == GetRunStopFlag())
	{
		if (IncrementMapLoadingTime() >= 3*60)		// 3분
		{
			UserManager->IsCheckMapLoading(TRUE);

			// 시작...
			if (ROOMSTATE_READY == GetRunStopFlag())
				SendNtfGameStart();

			InitMapLoadingTime();
		}
	}
}

void CRoomInternalLogic::NtfStartSignMsg(LONG lSignType)
{
	MsgNGSCli_NotifyStartSign		notifySign;
	notifySign.m_lSignType = lSignType;
	PayloadNGSCli pldSign(PayloadNGSCli::msgNotifyStartSign_Tag, notifySign);
	UserManager->SendToAllUser(pldSign);
}

void CRoomInternalLogic::NtfEndSignMsg()
{
	RemoveTimer(TIMER_INDEX_SIGN);	
	SetSigning(0);

	MsgNGSCli_NotifyEndSign		notifySign;
	PayloadNGSCli pldSign(PayloadNGSCli::msgNotifyEndSign_Tag, notifySign);
	UserManager->SendToAllUser(pldSign);
}

void CRoomInternalLogic::CheckInvalidItem(CUser* pUser, LONG lFighingResultType)
{
	CNFChar& nfChar = pUser->GetNFUser();
	MsgNGSCli_NtfChangedTotalItem ntf;
	ntf.Clear();

	// Line, Lure
	CalcItemCountByInitFishing(pUser, lFighingResultType, ntf.m_lstRemainCountChangedInven);

	// 기간제 아이템 체크
	ntf.m_lErrorCode = theNFItem.Check_TotalItemValid(pUser->GetGSN(), pUser->GetNFCharInfoExt()->m_nfCharInven, ntf.m_lstRemovedInven);
	if (NF::G_NF_ERR_SUCCESS == ntf.m_lErrorCode)
		// 아이템 유효성 체크를 한다...
		ntf.m_lErrorCode = theNFItem.AutoChange_DefaultItem(pUser->GetGSN(), GetRoomID(), pUser->GetNFCharInfoExt(), ntf.m_lstRemovedInven, ntf.m_lstDefaultChangedInven);

	ntf.m_nfAbility = nfChar.GetAbility();
	ntf.m_nfQuickSlot = nfChar.GetQuickSlot();

	ForEachElmt(TlstInvenSlot, ntf.m_lstRemainCountChangedInven, it, ij)
		theLog.Put(DEV_UK, "CheckInvalidItem - ########## changed_Inven Error, GSN, ", pUser->GetCSN(), ", Inven_Srl :", (*it).m_lInvenSRL);

	ForEachElmt(TlstInvenSlot, ntf.m_lstRemovedInven, it2, ij2)
		theLog.Put(DEV_UK, "CheckInvalidItem - ########## removed_inven Error, GSN, ", pUser->GetCSN(), ", Inven_Srl :", (*it2).m_lInvenSRL);

	PayloadNGSCli pld(PayloadNGSCli::msgNtfChangedTotalItem_Tag, ntf);
	UserManager->SendToUser(pUser->GetCSN(), pld);

	TMapAchvFactor	mapFactorVal;
	mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_FIGHTING_RESULT], lFighingResultType));
	g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_FIGHTING, GetRoomID(), mapFactorVal);
}

void CRoomInternalLogic::InitFishing(CUser* pUser, LONG lFighingResultType)
{
	CNFChar& nfChar = pUser->GetNFUser();
	theLog.Put(DEV_UK, "NGS_LOGIC, ##### InitFishing #####   CSN:", pUser->GetCSN());
	CheckInvalidItem(pUser, lFighingResultType);
	nfChar.InitNFCharInfo();
}

double CRoomInternalLogic::GetTirelessByLevel(long lCharLevel)
{
	double dHealth = 0.0;

	// ItemManger에서 레벨에 따른 피로도를 가져온다.

	TMapNFLevel& mapNFLevel = theNFDataItemMgr.GetNFLevel();

	TMapNFLevel::iterator it = mapNFLevel.find(lCharLevel);
	if (it != mapNFLevel.end())
	{
		NFLevel* pLevel = mapNFLevel[lCharLevel];
		if (pLevel)
			dHealth = pLevel->m_ability.m_dHealth;
		else
			theLog.Put(WAR_UK, "NGS_LOGIC, GetTirelessByLevel Not Found Level :", lCharLevel);
	}

	return dHealth;
}

LONG CRoomInternalLogic::SetLineLoose(CNFChar& nfChar, double& dLineLoadPresent, LONG lCurDragLevel, MsgCliNGS_ReqFighting& RcvMsg)
{
	LONG lErrorCode = 0;
	double dSubLine = 0, dSubLoad = 0;

	switch(lCurDragLevel)
	{
	case 1: dSubLine = 0.9; dSubLoad = 600; break;
	case 2: dSubLine = 0.5; dSubLoad = 400; break;
	case 3: dSubLine = 0.3; dSubLoad = 200; break;
	case 4: dSubLine = 0.2; dSubLoad = 100; break;
	case 5: dSubLine = 0; dSubLoad = 0; break;
	default : break;
	}

	// 2010-12-13 수정
	//dLineLoadPresent -= dSubLoad;

	if (RcvMsg.m_dLineLoose != dSubLine)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, SetLineLoose Client SubLine vs Server SubLine Mismatch : ", RcvMsg.m_dLineLoose, " vs ", dSubLine);
		//return FR_LINE_LOOSE;
	}

	nfChar.SetLineLength(nfChar.GetLineLength() + dSubLine);
	return lErrorCode;
}

void CRoomInternalLogic::CheckLooseType(CNFChar& nfChar, double& dLineLoadPresent, double dRealLoadMax, double dRateDragLevel, LONG lCurDragLevel, MsgCliNGS_ReqFighting& RcvMsg)
{
// 2010-12-13 주석
	// 스킬을 우선으로 체크
	// 돌진 스킬 타입, 현재 방향과 같으면 
//	if (nfChar.GetCurrentFishSkillType() == 4)
//	{
//		if (nfChar.IsSameDirection())
//			SetLineLoose(NFUser, dLineLoadPresent, lCurDragLevel, RcvMsg);
//	}
//	// 버티기 스킬 타입
//	else if (nfChar.GetCurrentFishSkillType() == 3)
//		SetLineLoose(NFUser, dLineLoadPresent, lCurDragLevel, RcvMsg);
//	// Drag에 의한 줄풀림 계산
//	else 
	if ((dLineLoadPresent > dRealLoadMax * dRateDragLevel) && (nfChar.GetCurFishHP() > 0))
		SetLineLoose(nfChar, dLineLoadPresent, lCurDragLevel, RcvMsg);
}

//
void CRoomInternalLogic::SetFishPatternByLineLength(CFish& BiteFish, CNFChar& nfChar, LONG& lFishHP)
{
	// 물고기 남은 체력% 구해오기
	double dRemainHP = (double)BiteFish.GetCurFishHP() / (double)BiteFish.GetMAXFishHP();
	LONG lRemainRateFishHP = (LONG)(dRemainHP * 100);

	FishInfo& fishInfo = BiteFish.GetFish();

	if (nfChar.GetLineLength() >= 10)		// 풀린 낚시줄의 길이가 10m 이상이 경우의 물고기의 HP와 이동 패턴에 대한 부분이다.
	{
		if (lRemainRateFishHP >= 50)
		{
			// ① 100% ~ 50 %
			// 물고기의 체력이 가득한 상태. DB의 Attackrate, Restrate의 100%를 반영한다.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_50100, 0, 0);
		}
		else if (lRemainRateFishHP <= 20 && lRemainRateFishHP > 50)
		{	
			// ② 49% ~ 20%
			// 물고기의 체력이 빠져 지키기 시작하는 상태. DB의 Attackrate 시간은 100% 반영하고 Restrate 시간이 1초정도 늘어나게 된다. 그리고 물고기의 공격력이 90% 정도 반영된다.
			lFishHP = (LONG)(lFishHP * 0.9);
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_2050, 0, 1);
		}
		else if (lRemainRateFishHP <= 1 && lRemainRateFishHP > 20)
		{
			// ③ 19%~ 1%
			// 물고기의 체력이 바닥나기 직전의 상태이다. DB의 Attackrate 시간에서 1초정도 줄어들고 Restrate 시간도 1초정도 늘어나게 된다. 그리고 물고기의 공격력이 70% 정도 반영된다.
			lFishHP = (LONG)(lFishHP * 0.7);

			// 현재 발동되고 있는 스킬이 Jump 이면, lAttackRate값을 변경하지 않는다. 2010.2.11
			LONG lAttackRate = fishInfo.m_lAttackRate;
			if (BiteFish.GetFishSkillType() != NF::G_NF_ERR_SUCCESS)
				lAttackRate = lAttackRate-1;
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_0120, lAttackRate, fishInfo.m_lRestRate+1);
		}
		else
		{
			// ④ 0%
			// 물고기의 체력이 바닥난 상태다. 물고기는 한방향으로 끌려오게 된다.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_0000, 0, 0);
			BiteFish.SetPrevFishDirection(nfChar.GetPRNG()%2 + 3);
		}
	}
	else		// 풀린 낚시줄 <= 10m
	{
		// 풀린 낚시줄이 10m 이내라는 것은 물고기가 캐릭터의 바로 앞까지 왔다는 경우다.
		// NF에서 물고기 랜딩 조건이 물고기의 HP가 0이고 낚시줄의 길이(Length)가 4m 이내라는 두 조건을 만족하는 경우에 발생하게 된다.
		// 하지만 게임중에 가까운 거리에서 물고기를 후킹하거나 돌진 스킬 등으로 물고기의 체력이 많은 상태에서 10m이내로 접근하게 되는 경우가 발생할 것이다. 이럴때 다음의 조건이 적용되도록 한다.
		if (lRemainRateFishHP >= 50)
		{
			// 1-3-1-1. HP > 50% and 풀린 낚시줄의 거리(Length) <= 10m
			// ① 물고기의 공격력 증가
			// 물고기의 공격력이 20% 정도 증가한다. 물고기의 공격력을 증가시켜 드래그 단계는 낮춰 물고기가 줄을 풀고 가도록 유도한다.
			lFishHP = (LONG)(lFishHP * 1.2);
			// ② 물고기 RestRate = 1
			// 물고기가 힘을 쓰지 않고 캐릭터에게 끌려오는 시간을 1초로 만든다. 이것도 ①의 경우와 함께 물고기가 줄을 풀고 계속 나가도록 유도하는 방법이다.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_50100, 0, 1);
		}
		else if (lRemainRateFishHP <= 20 && lRemainRateFishHP > 50)
		{
			// 1-3-1-2. 50% > HP >= 20% and 풀린 낚시줄의 거리(Length) <= 10m 면
			// ① 물고기의 공격력 증가
			// 물고기의 공격력이 10% 정도 증가한다. 
			lFishHP = (LONG)(lFishHP * 1.1);
			// ② 물고기 RestRate = 1
			// 물고기가 힘을 쓰지 않고 캐릭터에게 끌려오는 시간을 1초으로 만든다. 이것도 ①의 경우와 함께 물고기가 줄을 풀고 계속 나가도록 유도하는 방법이다.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_2050, 0, 1);
		}
		else
		{
			// 1-3-1-3. HP < 20% and 풀린 낚시줄의 거리(Length) <= 10m 면
			// ① 물고기 RestRate = 1
			// 물고기가 100%의 힘을 사용하지만 캐릭터에게 끌려오는 시간만 1초로 만들도록 한다.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_0120, 0, 1);
		}
	}
}

LONG CRoomInternalLogic::LineBreakCheck_FeverMode(CUser* pUser, MsgNGSCli_AnsFigtingResult& SendMsg, MsgCliNGS_ReqFighting& RcvMsg, LineBreakInfo& lineBreakInfo)
{
	if (!pUser)
		return FALSE;

	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);

	CNFChar&	nfChar			= pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt= nfChar.GetNFChar().m_nfCharInfoExt;		// 데이터 내용을 변경하려면 이 변수로...
	NFAbilityExt& abilityExt	= nfChar.GetAbilityExt();
	CFish&		biteFish		= nfChar.GetBiteFish();
	FishInfo&	biteFishInfo	= biteFish.GetFish();

	// NFUser(기본 능력치 + EquipItem + ClothItem) + NFUserItem(UsableItem + SkillItem)
	NFAbility	totAbility;
	totAbility += nfCharInfoExt.m_nfAbility;
	totAbility += abilityExt.m_nfAbility;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	EquipItem* pLureItem = NULL, *pLineItem = NULL, *pReelItem = NULL;
	double dReLoadMax = 0, dReLoadLimit = 0;
	double dRealLoadMax = 0, dRealLoadLimit = 0;
	double dMaxCharTireless = 0.0f;
	BOOL bIsFightingFail = FALSE;
	double dRate = 0.0;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Lure 아이템 가져오기
	pLureItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
	if (!pLureItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
		return NF::EC_FR_LURE_NOT_FOUND;
	}

	// Line 아이템 가져오기
	pLineItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode);
	if (!pLineItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode);
		return NF::EC_FR_LINE_NOT_FOUND;
	}

	// Reel 아이템 가져오기
	pReelItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Reel].m_lItemCode);
	if (!pReelItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Reel].m_lItemCode);
		return NF::EC_FR_REEL_NOT_FOUND;
	}

	pReelItem->m_dRetrievalLine = 0.5; 
	double dAddLine = sqrt(nfCharInfoExt.m_nfAbility.m_dControl)*ADD_LINE;
	double dLineLoadMaxAdd = pLineItem->m_dLineLoadMax + dAddLine;
	double dLineLoadLimitAdd = pLineItem->m_dLineLoadLimit + dAddLine;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Char의 힘 : MAX, DragLevel에 따른 힘
	double dMAXCharacterPower = nfChar.GetCharacterMaxPower();
	if (dMAXCharacterPower <= 0)
	{
		dMAXCharacterPower = ROUND( (sqrt(nfCharInfoExt.m_nfAbility.m_dStrength)*CHAR_POWER_ROD) + (sqrt(nfCharInfoExt.m_nfAbility.m_dAgility)*CHAR_POWER_REEL), 1 );
		nfChar.SetCharacterMaxPower(dMAXCharacterPower);

		if ((long)nfChar.GetCharacterMaxPower() != (long)RcvMsg.m_dCharacterMaxPower)
			return NF::EC_FR_CHAR_MAX_POWER;

		// 2011.02.16 시작 초기값 MAX/2로 수정
		nfChar.SetCurrentCharacterPower(dMAXCharacterPower/MAX_CHAR_POWER);
		biteFish.SetCurrentFishAttack(biteFishInfo.m_dAttack/MAX_FISH_ATTACK);

		// Commented by hankhwang - 2011년 7월 7일
		nfChar.SetLowRodAngleMax(dMAXCharacterPower * ROD_ANGLE_MAX);
		nfChar.SetHighRodAngleMax(dMAXCharacterPower * ROD_ANGLE_MAX);

		double dTempCharTensileStrength = (dMAXCharacterPower/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;
		double dTempFishTensileStrength = (biteFishInfo.m_dAttack/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH + (sqrt(biteFish.GetFishResultWeight())*FISH_TENSILE_RATE);
		double dTempLoadPresent = dTempCharTensileStrength + dTempFishTensileStrength;

		//Load_Present (100%) < Load_Max 인 경우 (바늘빠짐에 적용 - 2010.5.6)
		if  ((dTempLoadPresent*COMPARE_PRESENT_MAX) <= dLineLoadMaxAdd)
		{
			// i) 캐릭터 인장강도 > ( 물고기 인장강도+ 물고기 무게 ) * 3 면, RE_Load_Max, RE_Load_Limit 의 값은 다음과 같다.
			// ii) 캐릭터 인장강도 <= ( 물고기 인장강도 + 물고기 무게 ) * 3 면, RE_Load_Max, RE_Load_Limit 의 값은 다음과 같다.
			if (dTempCharTensileStrength > (dTempFishTensileStrength * COMPARE_CHAR_FISH_STRENGTH))		// i)
				dRate = CHAR_BIGGER_RATE;
			else if (dTempCharTensileStrength <= (dTempFishTensileStrength * COMPARE_CHAR_FISH_STRENGTH))		// ii)
				dRate = FISH_BIGGER_RATE;

			dReLoadMax = dTempLoadPresent * dRate;
			dReLoadLimit = dReLoadMax * RELOAD_LIMIT_RATE;

			if (dRate) {
				nfChar.SetIsTempLoadPresent(TRUE);
				nfChar.SetTempLineLoadMax(dReLoadMax);
				nfChar.SetTempLineLoadLimit(dReLoadLimit);
				nfChar.SetRate(dRate);
			}
			theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck ##### ReLoadMax :", dReLoadMax);
		}

		// 기본 Fatigue + 가지고 있는 모든 아이템에 의해 계산된 Fatigue까지 합쳐서 Stress(2차 값을 산출한다.)
		dMaxCharTireless = (long)ROUND(GetTirelessByLevel(nfCharInfoExt.m_nfCharBaseInfo.m_lLevel) + (sqrt(nfCharInfoExt.m_nfAbility.m_dHealth + abilityExt.m_nfAbility.m_dHealth) * GET_TIRELESS_BY_LEVEL), 1);

		nfChar.SetTireless(0, dMaxCharTireless);
		SendMsg.m_dMaxCharTireless = dMaxCharTireless;
	}

	//////////////////////////////////////////////////////////////////////////
	// 물고기 인장강도 수정 2011-02-15
	double dFishPowerAcceleration = biteFishInfo.m_dAttackAmount;
	double dFishPowerDropAcceleration = dFishPowerAcceleration * FISH_DROP_ACCEL;
	if (lineBreakInfo.m_dFishAttackTime < 0.0f)
		dFishPowerAcceleration *= FISH_DROP_ACCEL;

	double dAddFishPower = lineBreakInfo.m_dFishAttackTime * dFishPowerAcceleration;
	biteFish.AddCurrentFishAttack(dAddFishPower);

	//////////////////////////////////////////////////////////////////////////
	// 물고기 스킬 사용 인장강도 수정 2011-02-24
	// 물고기가 스킬을 사용중이라면, 사용한 스킬의 정보를 가지고 와서 Attack값을 더한다.
	FishSkillInfo		fishSkillInfo;
	fishSkillInfo.Clear();
	if (nfChar.GetCurrentPatternType() == FS_SKILL) {
		biteFish.GetFishSkill(fishSkillInfo);
		biteFish.SetFishSkillMaxAttack(fishSkillInfo.m_lAddFishAttack);
	}
	else
		fishSkillInfo.m_lAddFishAttack = biteFish.GetFishSkillMaxAttack();

	double dFishSkillPowerAcceleration = biteFishInfo.m_dAttackAmount;
	if (lineBreakInfo.m_dFishSkillAttackTime < 0.0f)
		dFishSkillPowerAcceleration *= ATTACK_ACCEL;

	double dAddFishSkillPower = lineBreakInfo.m_dFishSkillAttackTime * dFishSkillPowerAcceleration;
	biteFish.AddCurrentFishSkillAttack(dAddFishSkillPower);

	if (biteFish.GetCurrentFishSkillAttack() < 0.0f)
		biteFish.SetCurrentFishSkillAttck(0.0f);

	if (biteFish.GetCurrentFishSkillAttack() > fishSkillInfo.m_lAddFishAttack)
		biteFish.SetCurrentFishSkillAttck(fishSkillInfo.m_lAddFishAttack);

	double dFishTensileStrength = ((biteFish.GetCurrentFishAttack()+biteFish.GetCurrentFishSkillAttack()) / NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Load_Present를 MAX로 한 값에 의해서 재설정 되는 LoadMax, LoadLimit
	double dTempLineLoadMax = nfChar.GetTempLineLoadMax();
	if (dTempLineLoadMax <= 0)
	{
		dRealLoadMax = dLineLoadMaxAdd;
		dRealLoadLimit = dLineLoadLimitAdd;
	}
	else
	{
		dRealLoadMax = dTempLineLoadMax;
		dRealLoadLimit = nfChar.GetTempLineLoadLimit();
	}

	if ((long)dRealLoadMax != (long)RcvMsg.m_dRealLoadMax)
		return NF::EC_FR_LOAD_MAX;
	if ((long)dRealLoadLimit != (long)RcvMsg.m_dRealLoadLimit)
		return NF::EC_FR_LOAD_LIMIT;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1-1. 릴링시 증가되는 힘 ( 가속력 )
	// 캐릭터의 힘이 결정되었다면 릴링을 했을때 최대 힘으로 초당 어느 정도의 힘이 증가하고 감소하는지의 힘의 값이 필요하다.
	// 이 값은 REEL POWER의 수치에 의해서 결정 된다.
	// 드래그의 단계에 증가 되는 힘은 적용 받지 않는다.
	// ① 증가 힘 및 감소 힘( 소수점 한자리, kg )
	// 증가되는 힘 = SQRT( REEL POWER ) * 0.3
	// note) 증가 되는 힘은 초를 기준으로 증가 되는 것이 좋겠다. 틱은 너무 느릴듯..
	double dCharPowerAcceleration	= sqrt(nfCharInfoExt.m_nfAbility.m_dAgility) * CHAR_POWER_ACCEL;
	double dCharReleaseAccleration	= dCharPowerAcceleration * CHAR_RELEASE_ACCEL;
	double dAddCharPower			= lineBreakInfo.m_dReelClickAddCharPower * dCharPowerAcceleration;
	double dReleaseCharPower		= lineBreakInfo.m_dReelClickReleaseCharPower * dCharReleaseAccleration;
	double dCurrentCharacterPower	= nfChar.GetCurrentCharacterPower() + dAddCharPower + dReleaseCharPower;

	nfChar.SetCurrentCharacterPower(dCurrentCharacterPower);

	double dCharTensileStrength = (nfChar.GetCurrentCharacterPower()/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;

	if ((long)nfChar.GetCurrentCharacterPower() != (long)lineBreakInfo.m_dCharacterCurPower)
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck CurrentCharPower, C: ", lineBreakInfo.m_dCharacterCurPower, " VS S :", dCurrentCharacterPower);

	// Commented by hankhwang - 2011년 7월 7일
	// Rod휨 인장강도	
	double dRodAngleAcceleration = sqrt(nfCharInfoExt.m_nfAbility.m_dAgility) * ROD_ANGLE_ACCEL;	
	double dRodAngleReleaseAcceleration = dRodAngleAcceleration * ROD_ANGLE_RELEASE_ACCEL;
	for(int i= 0; i<2; i++)
	{
		double dAddRodPower = lineBreakInfo.m_dRodAngleAddTime[i] * dRodAngleAcceleration;
		double dReleaseRodPower = lineBreakInfo.m_dRodAngleSubTime[i] * dRodAngleReleaseAcceleration;

		// 2개가 합쳐진값
		if(i)
			nfChar.SetCurrentHighRodAngle(nfChar.GetCurrentHighRodAngle() + (dAddRodPower + dReleaseRodPower));
		else
			nfChar.SetCurrentLowRodAngle(nfChar.GetCurrentLowRodAngle() + (dAddRodPower + dReleaseRodPower));

	}

	// Rod휨 인장강도
	double dRodTensileStrength = ((nfChar.GetCurrentHighRodAngle() + nfChar.GetCurrentLowRodAngle())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;
	double dLineLoadPresent = dCharTensileStrength + dRodTensileStrength + dFishTensileStrength + (sqrt(biteFish.GetFishResultWeight())*FISH_TENSILE_RATE);

	// 2011.02.24 New DropAcceleration 추가
	double dDropAccelerationValue = (dCharReleaseAccleration + dFishPowerDropAcceleration) * DROP_ACCEL_VALUE;
	double dDropAcceleration = nfChar.GetDropAcceleration() + (dDropAccelerationValue * lineBreakInfo.m_dDropAccelTime);
	if (dDropAcceleration < 0.0f)
		dDropAcceleration = 0.0f;
	if (dDropAcceleration > dLineLoadPresent)
		dDropAcceleration = dLineLoadPresent;

	nfChar.SetDropAcceleration(dDropAcceleration);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2011.02.24 New Drag Level 추가
	double dSubFishPower = 0.0f;
	double dSubCharPower = 0.0f;
	double dFishPowerDragDropAcceleration = dFishPowerAcceleration * FISH_DRAG_DROP_ACCEL;

	for(int i=0; i<5; i++)
	{
		float fRate = DRAG_LEVEL_5;
		switch(i+1)
		{
		case 1: fRate = DRAG_LEVEL_1; break;
		case 2: fRate = DRAG_LEVEL_2; break;
		case 3: fRate = DRAG_LEVEL_3; break;
		case 4: fRate = DRAG_LEVEL_4; break;
		default: break;
		}


		dSubFishPower += dFishPowerDragDropAcceleration * fRate * lineBreakInfo.m_dFishDragOverTime[i];
		dSubCharPower += dCharReleaseAccleration * fRate * lineBreakInfo.m_dCharDragOverTime[i];
	}

	//////////////////////////////////////////////////////////////////////////
	// 드래그에 의해서 물고기, 캐릭터 인장강도 재계산
	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 물고기 
	double dFishAttack = biteFish.GetCurrentFishAttack();
	dFishAttack -= dSubFishPower;
	if (dFishAttack < 0.0f)
		dFishAttack = 0;
	if (dFishAttack > biteFishInfo.m_dAttack)
		dFishAttack = biteFishInfo.m_dAttack;
	biteFish.SetCurrentFishAttack(dFishAttack);

	dFishTensileStrength = ((biteFish.GetCurrentFishAttack()+biteFish.GetCurrentFishSkillAttack())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH;


	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 캐릭터 
	double dCharPower = nfChar.GetCurrentCharacterPower();
	dCharPower -= dSubCharPower;
	if (dCharPower < 0.0f)
		dCharPower = 0;
	if (dCharPower > nfChar.GetCharacterMaxPower())
		dCharPower = nfChar.GetCharacterMaxPower();
	nfChar.SetCurrentCharacterPower(dCharPower);

	dCharTensileStrength = (nfChar.GetCurrentCharacterPower()/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;

	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 LoadPresent
	dLineLoadPresent = dCharTensileStrength + dRodTensileStrength + dFishTensileStrength + (sqrt(biteFish.GetFishResultWeight())*FISH_TENSILE_RATE) - nfChar.GetDropAcceleration();
	SendMsg.m_fLineLoadPresent = (float)dLineLoadPresent;

	// 줄풀림에 따라 로드를 계산하고 최종적으로 비교한다.
	if ((long)dLineLoadPresent != (long)lineBreakInfo.m_fLineLoadPresent)
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, ~~~~~~~~~~~~~~ C : ", lineBreakInfo.m_fLineLoadPresent, " VS S : ", dLineLoadPresent, " ~~~~~~~~~~~~~~");
	theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, $$$$$ CharTensile : ", dCharTensileStrength, ", FishTensile : ", dFishTensileStrength, ", FishWeight :", biteFish.GetFishResultWeight());
	theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, $$$$$ LineLoadPresent : ", dLineLoadPresent, ", MaxLineTension : ", dRealLoadMax);

	// Usable 아이템 적용
	// 사용한 UsableItem 가져오기, Load Control 적용
	ForEachElmt(list<TiressType>, lineBreakInfo.m_lstTiress, it, ij)
	{
		double dHealth = 0.0;
		if ((*it).m_lType == TIRELESS_USEABLE_ITEM)
		{
			UsableItem* pItem = theNFDataItemMgr.GetUsableItemByIndex((*it).m_lValue);
			if (!pItem)
			{
				theLog.Put(WAR_UK, "!! FR_USABLE_ITEM_NOT_FOUND");
				return NF::EC_FR_USABLE_ITEM_NOT_FOUND;
			}
			dHealth = pItem->m_lHP;
		}
		bIsFightingFail = nfChar.SetTireless((*it).m_lType, dHealth);
		if (bIsFightingFail)
		{
			theLog.Put(WAR_UK, "!! TirelessZero");
			return lineBreakInfo.m_lErrorCode;
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//4-2. 물고기 HP
	//물고기의 HP는 DB값에 의해 결정된다. 그리고 몇가지 요인에 의해서 HP가 감소하게 된다.
	//i) 자연 감소
	//물고기의 HP는 시간이 지남에 따라 자연적으로 감소하게 되며 이 값은 물고기 HP의 비율로 계산된다.
	//물고기의 HP가 감소하는 시간은 물고기가 가지는 공격 속도와 동일하다. 
	//자연 감소 HP = 물고기 전체 HP * 0.02 ( 정수형 )
	//물고기의 자연 감소 HP로 자신의 전체 체력을 다 소모할 수도 있다.
	long lFishHP = (long)(biteFishInfo.m_lHitPoint * FISH_HP);


	//iii) 캐릭터 힘 감소
	//캐릭터의 힘에의해 감소되는 물고기의 HP를 의미한다.y
	//캐릭터의 힘은 물고기 DB의 공격 속도(이동패턴)이 갱신될 때 물고기에게 전달된다.
	//하지만 캐릭터의 힘은 현재 텐션의 비율(%)에 의해서 가감된다.
	//캐릭터 힘 = 캐릭터 힘 * ( Load_Present / Load_Max ) ( 정수형 )
	//단, 여기에서  Load_Present (100%) > Load_Max 일때 캐릭터 인장강도 > ( 물고기 인장강도+ 물고기 무게 ) * 2 의 경우에는
	//캐릭터의 힘을 100% 사용하도록 한다. ( 캐릭터의 능력이 물고기 보다 월등히 높다는 것을 의미하기 때문이다. )
	double dRateDragLevel = 0.0f;
	switch(lineBreakInfo.m_lDragLevel)
	{
	case 1: dRateDragLevel = RATE_DRAG_LEVEL_1; 
		break;
	case 2: dRateDragLevel = RATE_DRAG_LEVEL_2; 
		break;
	case 3: dRateDragLevel = RATE_DRAG_LEVEL_3; 
		break;
	case 4: dRateDragLevel = RATE_DRAG_LEVEL_4; 
		break;
	case 5: dRateDragLevel = RATE_DRAG_LEVEL_5;
		break;
	default : 
		break;
	}
	double dCharMaxPower = (nfChar.GetCharacterMaxPower() * dRateDragLevel)*IF_FEVER_MODE_CHAR_MAX_POWER;

	long lCharPower = 0;
	// Bullet 모드 일 경우, 자연 감소는 하지 않는다... 2011/6/8
	// 블릿모드일 경우, 물고기 체력을 감소 하지 않기 위해서 CharPower를 0으로...
	if (fishSkillInfo.m_lIsBullet != NF::G_NF_ERR_SUCCESS)
	{
		lCharPower = (long)(dCharMaxPower * dLineLoadPresent / (dRealLoadMax * FISH_HP_CHAR_POWER));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 환경에 의한 디버프 값을 틱당 체력에서 빼줘야 하기 때문에 여기서 계산한다. 
	// 물고기의 공격이 증가한다는 개념으로, 물고기 공격력의 * 15% 증가
	if (pUser->GetEnvDebuff())
		lFishHP = (long)(lFishHP * FISH_HP_DEBUFF);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2010/7/21 수정 - bback99 
	// 랜딩 능력치가 높아지면 HP = 0 이 되지 않더라고 랜딩을 할 수 있다.											
	// 물고기의 최대 HP를 HP_MAX 라고 하고, 현재의 HP를 HP, 캐릭터의 랜딩 능력치에 따른 물고기의 랜딩이 가능한 HP를 Landing HP(%) 라고 하면,											
	// 우선 현재 물고기의 HP 퍼센트는 다음과 같이 알 수 있다.											
	// 현재 HP 퍼센트(%, 소수점 한자리) = HP / HP_MAX * 100
	nfChar.CalcFishHP(lCharPower);

	SendMsg.m_lMaxFishHP = biteFishInfo.m_lHitPoint;
	SendMsg.m_lFishHP = nfChar.GetCurFishHP();
	SendMsg.m_dMaxCharTireless = nfChar.GetMaxTireless();
	SendMsg.m_dCharacterTireless = nfChar.GetCurrentTireless();
	SendMsg.m_dLineLength = nfChar.GetLineLength();
	SendMsg.m_dDistance = nfChar.GetDistanceFromFish();

	SendMsg.m_lPRNGCount = nfChar.GetPRNG_count();
	SendMsg.m_lPRNGValue = nfChar.GetPRNG_value();

	//return FR_CONTINUE;
	return lineBreakInfo.m_lErrorCode;
}

// 클라이언트에서 보낸 MsgCliNGS_ReqFightingAction의 필드값들을 검증하도록 변경(2010/10/27 - 슝)
// 2010/12/13 - FishAI를 클라이언트로 내리면서 서버에서는 일정주기의 SnapShop 만으로 검증하도록 수정
// SendMsg.ErrorCode -> 1:LineBreak, 2:FightingFail(캐릭터피로도==MAX), 3:Randing(FishHP==0), 4:continue...
LONG CRoomInternalLogic::LineBreakCheck(CUser* pUser, MsgNGSCli_AnsFigtingResult& SendMsg, MsgCliNGS_ReqFighting& RcvMsg, LineBreakInfo& lineBreakInfo)
{
	if (!pUser)
		return FALSE;

	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);

	CNFChar&	nfChar			= pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt= nfChar.GetNFChar().m_nfCharInfoExt;		// 데이터 내용을 변경하려면 이 변수로...
	NFAbilityExt& abilityExt	= nfChar.GetAbilityExt();
	CFish&		biteFish		= nfChar.GetBiteFish();
	FishInfo&	biteFishInfo	= biteFish.GetFish();

	// NFUser(기본 능력치 + EquipItem + ClothItem) + NFUserItem(UsableItem + SkillItem)
	NFAbility	totAbility;
	totAbility += nfCharInfoExt.m_nfAbility;
	totAbility += abilityExt.m_nfAbility;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	EquipItem* pLureItem = NULL, *pLineItem = NULL, *pReelItem = NULL, *pRodItem = NULL;
	double dReLoadMax = 0, dReLoadLimit = 0;
	double dRealLoadMax = 0, dRealLoadLimit = 0;
	double dMaxCharTireless = 0.0f;
	BOOL bIsFightingFail = FALSE;
	double dRate = 0.0;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Lure 아이템 가져오기
	pLureItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
	if (!pLureItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
		return NF::EC_FR_LURE_NOT_FOUND;
	}

	// Line 아이템 가져오기
	pLineItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode);
	if (!pLineItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode);
		return NF::EC_FR_LINE_NOT_FOUND;
	}

	// Reel 아이템 가져오기
	pReelItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Reel].m_lItemCode);
	if (!pReelItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Reel].m_lItemCode);
		return NF::EC_FR_REEL_NOT_FOUND;
	}

	// Rod 아이템 가져오기
	pRodItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Rod].m_lItemCode);
	if (!pRodItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Rod].m_lItemCode);
		return NF::EC_FR_ROD_NOT_FOUND;
	}

	pReelItem->m_dRetrievalLine = 0.5; 
	double dAddLine = sqrt(nfCharInfoExt.m_nfAbility.m_dControl)*ADD_LINE;
	double dLineLoadMaxAdd = pLineItem->m_dLineLoadMax + dAddLine;
	double dLineLoadLimitAdd = pLineItem->m_dLineLoadLimit + dAddLine;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Char의 힘 : MAX, DragLevel에 따른 힘
	double dMAXCharacterPower = nfChar.GetCharacterMaxPower();
	if (dMAXCharacterPower <= 0)
	{
		// 캐릭터 힘 MAX
		// 캐릭터의 힘은 곧 캐릭터의 공격력을 의미 하며 물고기의 체력을 소모 시키는 수치다.
		// ROD POWER, REEL POWER 수치를 반영한다.
		// ① 캐릭터 힘(공격력) ( 소수점 한자리, N )
		dMAXCharacterPower = ROUND( (sqrt(nfCharInfoExt.m_nfAbility.m_dStrength)*CHAR_POWER_ROD) + (sqrt(nfCharInfoExt.m_nfAbility.m_dAgility)*CHAR_POWER_REEL), 1 );
		nfChar.SetCharacterMaxPower(dMAXCharacterPower);

		if ((long)nfChar.GetCharacterMaxPower() != (long)RcvMsg.m_dCharacterMaxPower)
			return NF::EC_FR_CHAR_MAX_POWER;
		
		// 2011.02.16 시작 초기값 MAX/2로 수정
		nfChar.SetCurrentCharacterPower(dMAXCharacterPower/MAX_CHAR_POWER);
		biteFish.SetCurrentFishAttack(biteFishInfo.m_dAttack/MAX_FISH_ATTACK);

		// Commented by hankhwang - 2011년 7월 7일
		// Rod휨 맥스값(캐릭터 힘의 15%+15% = 30%)
		nfChar.SetLowRodAngleMax(dMAXCharacterPower * ROD_ANGLE_MAX);
		nfChar.SetHighRodAngleMax(dMAXCharacterPower * ROD_ANGLE_MAX);


		//2009.9.22 추가 사항
		// Load_Revision 을 구하기 전에, Load_Prensent와 Load_Max를 비교해서 예외 경우를 두도록 한다.
		// 캐릭터, 물고기 능력치 100%를 반영한 Load_Present 의 값이 Load_Max 보다 작을 경우가 두가지로 발생 한다. 
		// 첫번째 경우는 캐릭터의 인장강도가 물고기 보다 월등한 경우
		// 두번째는 캐릭터의 인장강도가 물고기와 비슷하거나 낮은 경우로 구분된다.
		// 이런 경우에는 아래의 공식을 통해서 RE_Load_Max, Re_Load_Limit 를 구해서 라인 브레이크 확률을 구한다.
		double dTempCharTensileStrength = (dMAXCharacterPower/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;
		double dTempFishTensileStrength = (biteFishInfo.m_dAttack/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH + (sqrt(biteFish.GetFishResultWeight())*FISH_TENSILE_RATE) ;
		double dTempLoadPresent = dTempCharTensileStrength + dTempFishTensileStrength;

	
		//Load_Present (100%) < Load_Max 인 경우 (바늘빠짐에 적용 - 2010.5.6)
		if  ((dTempLoadPresent*COMPARE_PRESENT_MAX) <= dLineLoadMaxAdd)
		{
			// i) 캐릭터 인장강도 > ( 물고기 인장강도+ 물고기 무게 ) * 3 면, RE_Load_Max, RE_Load_Limit 의 값은 다음과 같다.
			// ii) 캐릭터 인장강도 <= ( 물고기 인장강도 + 물고기 무게 ) * 3 면, RE_Load_Max, RE_Load_Limit 의 값은 다음과 같다.
			if (dTempCharTensileStrength > (dTempFishTensileStrength * COMPARE_CHAR_FISH_STRENGTH))		// i)
				dRate = CHAR_BIGGER_RATE;
			else if (dTempCharTensileStrength <= (dTempFishTensileStrength * COMPARE_CHAR_FISH_STRENGTH))		// ii)
				dRate = FISH_BIGGER_RATE;

			dReLoadMax = dTempLoadPresent * dRate;
			dReLoadLimit = dReLoadMax * RELOAD_LIMIT_RATE;

			if (dRate) {
				nfChar.SetIsTempLoadPresent(TRUE);
				nfChar.SetTempLineLoadMax(dReLoadMax);
				nfChar.SetTempLineLoadLimit(dReLoadLimit);
				nfChar.SetRate(dRate);
			}
			theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck ##### ReLoadMax :", dReLoadMax);
		}


		//4. 캐릭터 피로도
		//캐릭터의 피로도는 LV에 따른 값과 피로도 스탯으로 산출된 값으로 계산된다.
		//캐릭터 피로도 = LV 피로도 + 피로도 스탯 산출
		//i) LV에 따른 피로도 
		//LV에 따른 피로도 증가분은 공식보다는 DB에서 값을 얻어오는 방법을 사용하는것이 좋겠다.

		//ii) 피로도 스탯 산출
		//캐릭터의 피로도 스탯의 양에 의해서 결정되는 피로도의 값이다.
		//피로도 스탯 산출 = SQRT( 피로도 ) * 50 ( 정수형 )

		// 기본 Fatigue + 가지고 있는 모든 아이템에 의해 계산된 Fatigue까지 합쳐서 Stress(2차 값을 산출한다.)
		dMaxCharTireless = (long)ROUND(GetTirelessByLevel(nfCharInfoExt.m_nfCharBaseInfo.m_lLevel) + (sqrt(nfCharInfoExt.m_nfAbility.m_dHealth + abilityExt.m_nfAbility.m_dHealth) * GET_TIRELESS_BY_LEVEL), 1);

		nfChar.SetTireless(0, dMaxCharTireless);
		SendMsg.m_dMaxCharTireless = dMaxCharTireless;
	}

	//////////////////////////////////////////////////////////////////////////
	// 물고기 인장강도 수정 2011-02-15
	double dFishPowerAcceleration = biteFishInfo.m_dAttackAmount;
	double dFishPowerDropAcceleration = dFishPowerAcceleration * FISH_DROP_ACCEL;
	if (lineBreakInfo.m_dFishAttackTime < 0.0f)
		dFishPowerAcceleration *= FISH_DROP_ACCEL;

	double dAddFishPower = lineBreakInfo.m_dFishAttackTime * dFishPowerAcceleration;
	biteFish.AddCurrentFishAttack(dAddFishPower);

	//////////////////////////////////////////////////////////////////////////
	// 물고기 스킬 사용 인장강도 수정 2011-02-24
	// 물고기가 스킬을 사용중이라면, 사용한 스킬의 정보를 가지고 와서 Attack값을 더한다.
	FishSkillInfo		fishSkillInfo;
	fishSkillInfo.Clear();
	if (nfChar.GetCurrentPatternType() == FS_SKILL) {
		biteFish.GetFishSkill(fishSkillInfo);
		biteFish.SetFishSkillMaxAttack(fishSkillInfo.m_lAddFishAttack);
	}
	else
		fishSkillInfo.m_lAddFishAttack = biteFish.GetFishSkillMaxAttack();
		
	double dFishSkillPowerAcceleration = biteFishInfo.m_dAttackAmount;
	if (lineBreakInfo.m_dFishSkillAttackTime < 0.0f)
		dFishSkillPowerAcceleration *= ATTACK_ACCEL;

	double dAddFishSkillPower = lineBreakInfo.m_dFishSkillAttackTime * dFishSkillPowerAcceleration;
	biteFish.AddCurrentFishSkillAttack(dAddFishSkillPower);

	if (biteFish.GetCurrentFishSkillAttack() < 0.0f)
		biteFish.SetCurrentFishSkillAttck(0.0f);
	
	if (biteFish.GetCurrentFishSkillAttack() > fishSkillInfo.m_lAddFishAttack)
		biteFish.SetCurrentFishSkillAttck(fishSkillInfo.m_lAddFishAttack);

	theLog.Put(DEV_UK, "NGS_LOGIC, dAddFishSkillPower : ", dAddFishSkillPower, " / GetFishSkillAttack :", biteFish.GetCurrentFishSkillAttack(), " / fishSkillInfo.m_lAddFishAttack :", fishSkillInfo.m_lAddFishAttack);

	double dFishTensileStrength = ((biteFish.GetCurrentFishAttack()+biteFish.GetCurrentFishSkillAttack())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Load_Present를 MAX로 한 값에 의해서 재설정 되는 LoadMax, LoadLimit
	double dTempLineLoadMax = nfChar.GetTempLineLoadMax();
	if (dTempLineLoadMax <= 0)
	{
		dRealLoadMax = dLineLoadMaxAdd;
		dRealLoadLimit = dLineLoadLimitAdd;
	}
	else
	{
		dRealLoadMax = dTempLineLoadMax;
		dRealLoadLimit = nfChar.GetTempLineLoadLimit();
	}

	if ((long)dRealLoadMax != (long)RcvMsg.m_dRealLoadMax)
		return NF::EC_FR_LOAD_MAX;
	if ((long)dRealLoadLimit != (long)RcvMsg.m_dRealLoadLimit)
		return NF::EC_FR_LOAD_LIMIT;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1-1. 릴링시 증가되는 힘 ( 가속력 )
	// 캐릭터의 힘이 결정되었다면 릴링을 했을때 최대 힘으로 초당 어느 정도의 힘이 증가하고 감소하는지의 힘의 값이 필요하다.
	// 이 값은 REEL POWER의 수치에 의해서 결정 된다.
	// 드래그의 단계에 증가 되는 힘은 적용 받지 않는다.
	// ① 증가 힘 및 감소 힘( 소수점 한자리, kg )
	// 증가되는 힘 = SQRT( REEL POWER ) * 0.3
	// note) 증가 되는 힘은 초를 기준으로 증가 되는 것이 좋겠다. 틱은 너무 느릴듯..
	double dCharPowerAcceleration	= sqrt(nfCharInfoExt.m_nfAbility.m_dAgility) * CHAR_POWER_ACCEL;
	double dCharReleaseAccleration	= dCharPowerAcceleration * CHAR_RELEASE_ACCEL;
	double dAddCharPower			= lineBreakInfo.m_dReelClickAddCharPower * dCharPowerAcceleration;
	double dReleaseCharPower		= lineBreakInfo.m_dReelClickReleaseCharPower * dCharReleaseAccleration;
	double dCurrentCharacterPower	= nfChar.GetCurrentCharacterPower() + dAddCharPower + dReleaseCharPower;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 물고기 힘
	// 캐릭터 전달되어 캐릭터의 피로도를 증가 시키는 물고기의 공격력.
	// 물고기의 힘은 물고기의 공격 속도를 참고해서 그 시간 마다 전해 진다.
	// 물고기의 힘을 피로도에 바로 전달 할 수도 있고, 캐릭터의 특정 수치로 물고기의 힘을 줄여서 반영할 수도 있다.
	// ( 후자가 더 좋지 않을까 하는데?? )
	// ① 물고기 힘
	// 물고기 DB의 힘을 그대로 사용 한다.


	// 캐릭터 인장 강도
	// 라인의 인장강도에 더해지는 캐릭터의 힘의 인장강도
	nfChar.SetCurrentCharacterPower(dCurrentCharacterPower);

	double dCharTensileStrength = (nfChar.GetCurrentCharacterPower()/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;

	if ((long)nfChar.GetCurrentCharacterPower() != (long)lineBreakInfo.m_dCharacterCurPower)
		theLog.Put(DEV_UK, "NGS_LOGIC, LineBreakCheck CurrentCharPower, C: ", lineBreakInfo.m_dCharacterCurPower, " VS S :", dCurrentCharacterPower);

	// Commented by hankhwang - 2011년 7월 7일
	// Rod휨 인장강도	
	double dRodAngleAcceleration = sqrt(nfCharInfoExt.m_nfAbility.m_dAgility) * ROD_ANGLE_ACCEL;	
	double dRodAngleReleaseAcceleration = dRodAngleAcceleration * ROD_ANGLE_RELEASE_ACCEL;
	for(int i= 0; i<2; i++)
	{
		double dAddRodPower = lineBreakInfo.m_dRodAngleAddTime[i] * dRodAngleAcceleration;
		double dReleaseRodPower = lineBreakInfo.m_dRodAngleSubTime[i] * dRodAngleReleaseAcceleration;

		// 2개가 합쳐진값
		if(i)
			nfChar.SetCurrentHighRodAngle(nfChar.GetCurrentHighRodAngle() + (dAddRodPower + dReleaseRodPower));
		else
			nfChar.SetCurrentLowRodAngle(nfChar.GetCurrentLowRodAngle() + (dAddRodPower + dReleaseRodPower));
		
	}

	// Rod휨 인장강도
	double dRodTensileStrength = ((nfChar.GetCurrentHighRodAngle() + nfChar.GetCurrentLowRodAngle())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 파이팅의 캐릭터~물고기 거리(Distance), 풀린 낚시줄 (Line_length)
	// 물고기가 루어를 물고 파이팅이 시작되면 Distance와 Line_Length가 따로 계산되며 물고기의 이동 방향에 영향을 많이 받는다.
	// 물고기의 3,4,5 방향이 캐릭터 쪽으로 가까워지는 방향이며 이때 Distance와 Line_Length가 차이나게 된다.
	// 우선 Distance > Line_length 의 경우는 절대로 발생하지 않는 전제를 가정하도록 한다.
	// Distance < Line_length 가 되는 경우는
	// i) 물고기의 이동속도 > REEL 라인 회수값 
	// ii) 캐릭터가 REEL을 감지 않는 경우
	// 로 구분할 수있다.
	// 이 때의 Line_Length가 Distance 와 같아지기 위해서 유저는 릴링을 계속 해야하며 이때 라인 텐션은 아주 빠르게
	// 줄어들게 된다.
	// 이 경우를 공식화 해보면 아래와 같다.
	// Distance < ( Line_Length + (Distance * 5%) ) 가 되면, Load_Prensent 는 0이 되며
	// 다음과 같은 가속력 값으로 텐션이 빠르게 감소하게 된다.
	// 감소 가속력 = ( 캐릭터 가속력 * 2 ) + ( 물고기 가속력 * 2 )
	// 텐션이 감소하다가 Distance >= ( Line_Length + (Distance * 5%) ) 이 되면
	// 다시 정상적인 텐션으로 증가하게 되며 캐릭터, 물고기 가속력 만큼 증가하게 된다.


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 라인 부하 계산
	// 라인에 걸리는 부하를 계산 하는 부분이다.
	// 현재 라인에 걸리는 부하를 Load_Present 라고 하면
	// ① 라인 부하( ( 소수점 한자리, kg )
	// Load_Present = 캐릭터 인장 강도 + 물고기 인장 강도 + 물고기 무게 + 루어 무게 + 기타 - 감소 가속력
	// 2011/02/24 - Load_Present = 캐릭터 인장 강도 + 물고기 인장 강도 + 물고기 무게 + 물고기 스킬 + 드랍Acceleration 로 수정
	// " ( 기타는 수심에 따른 무게의 증가 값을 의미한다. 적용할지 여부는 고민해야 한다. 그리고 ROD의 방향과 물고기의 방향에
	// 따라 추가되는 힘(부하)의 반영은 프로그램 회의 후에 결정하도록 하자. )"

	double dLineLoadPresent = dCharTensileStrength + dRodTensileStrength + dFishTensileStrength + (sqrt(biteFish.GetFishResultWeight()) * FISH_TENSILE_RATE);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 물고기의 이동 방향에 따라 Load_Present 의 양을 증가시키는 변수를 설정하도록 한다.
	// 물고기가 캐릭터와 멀어지는 방향으로 이동하려하면 현재 라인 텐션의 비율에 따라 이동 거리를 제한하고 있다.
	// 하지만 이 부분을 적용시키는 방향은 0, 1, 7 의 방향이며 2, 6의 방향은 적용하지 않고 있다.
	// 여기서 Distace 와 Line_Lengh를 비교해서 물고기의 이동을 텐션의 비율에 적용시키면 모든 방향의 이동 제한이 가능하다.
	// Distance = Line_length 일때 물고기의 이동 방향이 Distance를 증가시키는 방향이면 이동 제한을 두며 그렇지 않으면
	// 이동제한을 두지 않는다.
	// 이때 Load_Present * 10% 의 텐션 부하가 추가된다.

	// 2011.02.24 New DropAcceleration 추가
	double dDropAccelerationValue = (dCharReleaseAccleration + dFishPowerDropAcceleration) * DROP_ACCEL_VALUE;
	double dDropAcceleration = nfChar.GetDropAcceleration() + (dDropAccelerationValue * lineBreakInfo.m_dDropAccelTime);
	if (dDropAcceleration < 0.0f)
		dDropAcceleration = 0.0f;
	if (dDropAcceleration > dLineLoadPresent)
		dDropAcceleration = dLineLoadPresent;

	nfChar.SetDropAcceleration(dDropAcceleration);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2011.02.24 New Drag Level 추가
	double dSubFishPower = 0.0f;
	double dSubCharPower = 0.0f;
	double dFishPowerDragDropAcceleration = biteFishInfo.m_dAttackAmount * FISH_DRAG_DROP_ACCEL;

	for(int i=0; i<5; i++)
	{
		float fRate = DRAG_LEVEL_5;
		switch(i+1)
		{
		case 1: fRate = DRAG_LEVEL_1; break;
		case 2: fRate = DRAG_LEVEL_2; break;
		case 3: fRate = DRAG_LEVEL_3; break;
		case 4: fRate = DRAG_LEVEL_4; break;
		default: break;
		}
		
		dSubFishPower += dFishPowerDragDropAcceleration * fRate * lineBreakInfo.m_dFishDragOverTime[i];
		dSubCharPower += dCharReleaseAccleration * fRate * lineBreakInfo.m_dCharDragOverTime[i];
	}

	//////////////////////////////////////////////////////////////////////////
	// 드래그에 의해서 물고기, 캐릭터 인장강도 재계산
	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 물고기 
	double dFishAttack = biteFish.GetCurrentFishAttack();
	dFishAttack -= dSubFishPower;
	if (dFishAttack < 0.0f)
		dFishAttack = 0;
	if (dFishAttack > biteFishInfo.m_dAttack)
		dFishAttack = biteFishInfo.m_dAttack;
	biteFish.SetCurrentFishAttack(dFishAttack);

	dFishTensileStrength = ((biteFish.GetCurrentFishAttack()+biteFish.GetCurrentFishSkillAttack())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH;
	

	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 캐릭터 
	double dCharPower = nfChar.GetCurrentCharacterPower();
	dCharPower -= dSubCharPower;
	if (dCharPower < 0.0f)
		dCharPower = 0;
	if (dCharPower > nfChar.GetCharacterMaxPower())
		dCharPower = nfChar.GetCharacterMaxPower();
	nfChar.SetCurrentCharacterPower(dCharPower);

	dCharTensileStrength = (nfChar.GetCurrentCharacterPower()/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;

	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 LoadPresent
	dLineLoadPresent = dCharTensileStrength + dRodTensileStrength + dFishTensileStrength + (sqrt(biteFish.GetFishResultWeight())*FISH_TENSILE_RATE) - nfChar.GetDropAcceleration();
	SendMsg.m_fLineLoadPresent = (float)dLineLoadPresent;

	// 줄풀림에 따라 로드를 계산하고 최종적으로 비교한다.
	if ((long)dLineLoadPresent != (long)lineBreakInfo.m_fLineLoadPresent)
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, ~~~~~~~~~~~~~~ C : ", lineBreakInfo.m_fLineLoadPresent, " VS S : ", dLineLoadPresent, " ~~~~~~~~~~~~~~");
	theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, $$$$$ CharTensile : ", dCharTensileStrength, ", FishTensile : ", dFishTensileStrength, ", FishWeight :", biteFish.GetFishResultWeight());
	theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, $$$$$ LineLoadPresent : ", dLineLoadPresent, ", MaxLineTension : ", dRealLoadMax);


	// 2010/11/30 - 클라이언트가 보내주는 LoadPresent로 계산한다.

	// 2011/11/24 - 아이템 내구도 감소 로직 추가
	DecrementEnduranceItem(nfChar, lineBreakInfo.m_lEnduranceType, pReelItem, pRodItem);


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 라인 부하에 따른 바늘털이(Word 참조) 
	// 라인에 걸리는 부하가 0이 될 때 자연스럽게 물고기가 바늘을 빼고 도망간 것으로 처리한다.
	// 0이 되는 순간보다는 0에서 1초 정도 유지했을 때 실행하도록 하자.
	// 루어를 뱉을 확률을 라인에 걸리는 부하가 적을 때 발생하게 된다. 즉, 라인 브레이크 확률과는 반대의 의미지만 고기를 놓치는 것은 동일하다.
	// 물고기가 루어를 뱉을 확률은 현재 라인에 걸리는 부하에 의해서 결정된다.
	// LoadPresent값이 LoadMax의 값보다 30% 아래로 3초 이상 유지 됐을 경우(3번 카운트가 되면, 무조건 바늘 털이...)

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 라인 브레이크 확률
	// 라인이 끊어질 확률을 계산하는 부분이다.
	// 라인에 가지는 최대 부하, 한계 부하을 각각 Load_Max, Load_Limit이라고 설정하고, 현재 라인에 걸리는 부하를 Load_Present라고 하면
	// 라인이 끊어지는 확률 (LineBreak )은 다음과 같다.
	// Load_Revision = Load_Prensent - Load_Max, ( 단, Load_Revison < 0, LineBreak = 0(%) )
	// 현재 라인에 걸리는 부하가 최대 부하 보다 작은 경우에 라인 브레이크 확률은 0 이라는 의미이다.
	// Linebreak(%) = ( Load_Revison ) / ( Load_Limit - Load_Max ) * 100
	// 라인 브레이크 확률은 소수점 한자리 까지 반영한다. 
	if (lineBreakInfo.m_lErrorCode == NF::EC_FR_LINEBREAK)
		return lineBreakInfo.m_lErrorCode;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// LineBreak가 아니면... 
	// 캐릭터와 물고기의 피로도를 체크한다.
	// 캐릭터의 피로도가 (== MAX이면, Fighting 실패)
	// 캐릭터의 피로도가 (!= MAX이면, Continue...)

	// 물고기 HP가 (== 0이면, Randing...)
	// 물고기 HP가 (!= 0이면, Continue...)
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Usable 아이템 적용
	// 사용한 UsableItem 가져오기, Load Control 적용
	ForEachElmt(list<TiressType>, lineBreakInfo.m_lstTiress, it, ij)
	{
		double dHealth = 0.0;
		if ((*it).m_lType == TIRELESS_USEABLE_ITEM)
		{
			UsableItem* pItem = theNFDataItemMgr.GetUsableItemByIndex((*it).m_lValue);
			if (!pItem)
			{
				theLog.Put(WAR_UK, "!! FR_USABLE_ITEM_NOT_FOUND");
				return NF::EC_FR_USABLE_ITEM_NOT_FOUND;
			}
			dHealth = pItem->m_lHP;
		}
		bIsFightingFail = nfChar.SetTireless((*it).m_lType, dHealth);
		if (bIsFightingFail)
		{
			theLog.Put(WAR_UK, "!! TirelessZero");
			return lineBreakInfo.m_lErrorCode;
		}
	}


	//4-1. 피로도 증가
	//피로도 증가의 경우는 다음과 같다.
	//i) 물고기 피로도
	//물고기 힘 피로도 = 물고기 힘
	//물고기의 힘은 물고기 DB의 공격속도(이동패턴)이 갱신될 때 캐릭터에게 전달된다.
	// Bullet 모드 일 경우, 자연 감소는 하지 않는다... 2011/6/8
	if (fishSkillInfo.m_lIsBullet != NF::G_NF_ERR_SUCCESS) {
		bIsFightingFail = nfChar.SetTireless(2, biteFishInfo.m_dAttack, nfChar.GetCurFishHP());
		if (bIsFightingFail)
		{
			theLog.Put(WAR_UK, "!! TirelessZero_Tick");
			return lineBreakInfo.m_lErrorCode;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//4-2. 물고기 HP
	//물고기의 HP는 DB값에 의해 결정된다. 그리고 몇가지 요인에 의해서 HP가 감소하게 된다.
	//i) 자연 감소
	//물고기의 HP는 시간이 지남에 따라 자연적으로 감소하게 되며 이 값은 물고기 HP의 비율로 계산된다.
	//물고기의 HP가 감소하는 시간은 물고기가 가지는 공격 속도와 동일하다. 
	//자연 감소 HP = 물고기 전체 HP * 0.02 ( 정수형 )
	//물고기의 자연 감소 HP로 자신의 전체 체력을 다 소모할 수도 있다.
	long lFishHP = (long)(biteFishInfo.m_lHitPoint * FISH_HP);


	//ii) 스킬 사용 감소
	//물고기가 DB에 지정된 스킬을 사용할때 HP를 소모하게 된다.
	//HP가 부족하면 스킬을 사용하지 못한다. 
	//스킬 사용 감소 HP는 스킬 DB에서 참조 한다.
	//long lSkillFishHP = 0;


	//iii) 캐릭터 힘 감소
	//캐릭터의 힘에의해 감소되는 물고기의 HP를 의미한다.y
	//캐릭터의 힘은 물고기 DB의 공격 속도(이동패턴)이 갱신될 때 물고기에게 전달된다.
	//하지만 캐릭터의 힘은 현재 텐션의 비율(%)에 의해서 가감된다.
	//캐릭터 힘 = 캐릭터 힘 * ( Load_Present / Load_Max ) ( 정수형 )
	//단, 여기에서  Load_Present (100%) > Load_Max 일때 캐릭터 인장강도 > ( 물고기 인장강도+ 물고기 무게 ) * 2 의 경우에는
	//캐릭터의 힘을 100% 사용하도록 한다. ( 캐릭터의 능력이 물고기 보다 월등히 높다는 것을 의미하기 때문이다. )
	double dRateDragLevel = 0.0f;
	switch(lineBreakInfo.m_lDragLevel)
	{
	case 1: dRateDragLevel = RATE_DRAG_LEVEL_1; 
		break;
	case 2: dRateDragLevel = RATE_DRAG_LEVEL_2; 
		break;
	case 3: dRateDragLevel = RATE_DRAG_LEVEL_3; 
		break;
	case 4: dRateDragLevel = RATE_DRAG_LEVEL_4; 
		break;
	case 5: dRateDragLevel = RATE_DRAG_LEVEL_5;
		break;
	default : 
		break;
	}
	double dCharMaxPower = nfChar.GetCharacterMaxPower() * dRateDragLevel;

	long lCharPower = 0;
	// Bullet 모드 일 경우, 자연 감소는 하지 않는다... 2011/6/8
	// 블릿모드일 경우, 물고기 체력을 감소 하지 않기 위해서 CharPower를 0으로...
	if (fishSkillInfo.m_lIsBullet != NF::G_NF_ERR_SUCCESS)
	{
		lCharPower = (long)(dCharMaxPower * dLineLoadPresent / (dRealLoadMax * FISH_HP_CHAR_POWER));
	}
	theLog.Put(DEV_UK, "NGS_LOGIC, LoadPresent : ", dLineLoadPresent, " / CharMAXPower :", dCharMaxPower, " / RealLoadMax : ", dRealLoadMax);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	float fRate = 0.0;
	switch(lineBreakInfo.m_lJumpBulletResult)
	{
	case FISHHP_TYPE_ROD_BLOCK_GOOD:
		fRate = BULLET_ROD_BLOCK_GOOD;
		break;
	case FISHHP_TYPE_BULLET_JUMP_GOOD:
	case FISHHP_TYPE_BULLET_RUSH_GOOD:
	case FISHHP_TYPE_ROD_BLOCK_GREAT:
	case FISHHP_TYPE_ROD_DIRECTION_SUCCESS:
		fRate = BULLET_BULLET_S_GOOD;
		break;
	case FISHHP_TYPE_BULLET_JUMP_GREAT:
	case FISHHP_TYPE_BULLET_RUSH_GREAT:
		fRate = BULLET_BULLET_S_GREAT;
		break;
	default: 
		theLog.Put(WAR_UK, "NGS_LOGIC, %%%%% lineBreakInfo.m_lJumpBulletResult not define CSN : ", nfChar.GetCSN(), " // Define :", lineBreakInfo.m_lJumpBulletResult);
		break;
	}
	LONG lBulletFishHP = (LONG)(nfChar.GetCharacterMaxPower() * fRate);
    nfChar.CalcFishHP(lBulletFishHP);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 환경에 의한 디버프 값을 틱당 체력에서 빼줘야 하기 때문에 여기서 계산한다. 
	// 물고기의 공격이 증가한다는 개념으로, 물고기 공격력의 * 15% 증가
	if (pUser->GetEnvDebuff())
		lFishHP = (long)(lFishHP * FISH_HP_DEBUFF);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 서버는 충돌 박스가 없기 때문에, 클라이언트가 방향을 바꾸라고 메세지를 보냈다면, 
	// 풀린 낚시줄 >= 10m 이고 물고기 체력이 0일 경우에 한 방향으로 설정하는 것 보다 우선 되어야 하므로.. SetFishPatternByLineLength을 먼저 실행한다.
	// HP와 남은 줄의 길이로 인한 물고기 패턴 구하기
	//SetFishPatternByLineLength(BiteFish, NFUser, lFishHP); 

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2010/7/21 수정 - bback99 
	// 랜딩 능력치가 높아지면 HP = 0 이 되지 않더라고 랜딩을 할 수 있다.											
	// 물고기의 최대 HP를 HP_MAX 라고 하고, 현재의 HP를 HP, 캐릭터의 랜딩 능력치에 따른 물고기의 랜딩이 가능한 HP를 Landing HP(%) 라고 하면,											
	// 우선 현재 물고기의 HP 퍼센트는 다음과 같이 알 수 있다.											
	// 현재 HP 퍼센트(%, 소수점 한자리) = HP / HP_MAX * 100
	//double dFishHPRate = ROUND(nfChar.CalcFishHP(lSkillFishHP + lCharPower), 2);
	nfChar.CalcFishHP(lCharPower);

	//// 그리고 랜딩이 가능한 퍼센트를 구하는 공식은 다음과 같다.
	//// Landing HP(%, 소수점 한자리) = SQRT( 랜딩 능력치 ) * 5.5
	//double dLandingHP = ROUND(sqrt(nfCharInfoExt.m_nfAbility.m_dLanding) * LANDING, 2);

	//// Landing HP > 현재 HP 퍼센트 면 즉시 랜딩 바늘털이를 계산하고 바늘털이 확률을 벗어나면 랜딩 하도록 한다.
	//if (nfChar.GetCurFishHP() == 0 || (dLandingHP >= dFishHPRate))
	//{
	//	theLog.Put(WAR_UK, "NGS_LOGIC, %%%%% Lading, CurFishHP : ", nfChar.GetCurFishHP());
	//	theLog.Put(WAR_UK, "NGS_LOGIC, %%%%% Lading, dLandingHP :", dLandingHP, " >= dFishHPRate :", dFishHPRate);
	//}

// 클라이언트의 요청으로 2010-12-17 주석처리
// 		// 2010/7/21 수정 - bback99
// 		// 현재 랜딩이 가능한 거리는 400cm 로 설정되어 있다. 이 거리를 300cm로 줄이고 랜딩 능력치를 반영해서 랜딩 가능한 거리를 늘인다.
// 		// 캐릭터의 랜딩 가능 거리(Can_Landing_Distance) 는 다음의 공식과 같다.
// 		double dLandingDistance = 120 + (sqrt(NFUI.m_nfAbility.m_dLanding) * 12);
// 
// 		if (nfChar.GetLineLength()*100 <= dLandingDistance)
// 		{
// 			theLog.Put(WAR_UK, "NGS_LOGIC, %%%%% Lading, Distance : ", dLandingDistance);
// 
// 			// 랜딩 바늘털이
// 			// " 랜딩에서 마지막으로 발생하는 이벤트적 바늘털이.
			// 랜딩 바늘털이의 발생 확률은 물고기 DB에서 참조해서 생성되며, 이벤트가 발생하면 '랜딩' 수치를 참고해서 '랜딩 바늘털이' 확률을 구한다."
// 			// ① 랜딩 바늘털이 확률 ( 소수점 한자리, % )
// 			// 랜딩 바늘털이 확률 = 50 - ( SQRT ( 후킹 + ( 럭키포인트 / 2 ) ) * 1.9 + 15 )  단, 계급 핸디캡 초급 10%, 하수 5% 의 추가 확률을 빼주도록 한다.
// 			double dLadingShakeRate = 50 - (sqrt (NFUI.m_nfAbility.m_dHooking + (NFUI.m_nfAbility.m_dLuckyPoint / 2)) * 1.9 + 15);
// 			double dAddRate = 0;
// 			switch(NFUI.m_nfCharBaseInfo.m_lLevel)
// 			{
// 			case 1: dAddRate = 15; break;
// 			case 2: dAddRate = 7; break;
// 			default: break;
// 			}
// 			dLadingShakeRate -= dAddRate;
// 
//
//			long lLadingShakeRate = (long)(dLadingShakeRate * 10);
//			long lRand = nfChar.GetPRNG()%1000;

// 			// 랜덤값이 바늘털이 확률보다 작으면, 바늘 털이에 걸림
// 			if (lLadingShakeRate > lRand)
// 			{
// 				if (nfChar.GetIsTempLoadPresent()) {
// 					theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Failed!!!  LureLoss~~  // LandingShakeRate: ", lLadingShakeRate, " >>>>>  RandRate: ", lRand);
// 					//return FR_LURELOSS;
// 					return RcvMsg.m_lErrorCode;
// 				}
// 				else {
// 					theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Failed!!!  HEADSHAKE~~  // LandingShakeRate: ", lLadingShakeRate, " >>>>>  RandRate: ", lRand);
// 					//return FR_HEADSHAKE_LANDING;
// 					return RcvMsg.m_lErrorCode;
// 				}
// 			}
// 			else
// 			{
// 				theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Landing!!!!");
// 				//return FR_LANDING;
// 				return RcvMsg.m_lErrorCode;
// 			}
//			return RcvMsg.m_lErrorCode;
//		}
//		else
//			theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Distance :", nfChar.GetDistanceFromFish());

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SendMsg.m_lMaxFishHP = biteFishInfo.m_lHitPoint;
	SendMsg.m_lFishHP = nfChar.GetCurFishHP();
	SendMsg.m_dMaxCharTireless = nfChar.GetMaxTireless();
	SendMsg.m_dCharacterTireless = nfChar.GetCurrentTireless();
	SendMsg.m_dLineLength = nfChar.GetLineLength();
	SendMsg.m_dDistance = nfChar.GetDistanceFromFish();

	SendMsg.m_lPRNGCount = nfChar.GetPRNG_count();
	SendMsg.m_lPRNGValue = nfChar.GetPRNG_value();

	//return FR_CONTINUE;
	return lineBreakInfo.m_lErrorCode;
}

BOOL CRoomInternalLogic::DecrementEnduranceItem(CNFChar& nfChar, LONG lType, const EquipItem* pReelItem, const EquipItem* pRodItem)
{
// test
// nfChar.AccountEndurance(pReelItem->m_lEndurance, pRodItem->m_lEndurance);
// return TRUE;

	if (pReelItem != NULL && pRodItem != NULL)
	{
		if (lType == ENDURANCE_INIT)
			return FALSE;

		int nRate = 0;

		switch(lType)
		{
		case ENDURANCE_CONTROL_FAIL:
		case ENDURANCE_BLOCK_FAIL:
			{
				nRate = 10;
				break;
			}
		case ENDURANCE_BULLET_FAIL:
			{
				nRate = 20;
				break;
			}
		default: return FALSE;
		}

		int nRand = urandom(100);
		if (nRate >= nRand)		// 확률에 들어온거임...
		{
			nfChar.AccountEndurance(pReelItem->m_lEndurance, pRodItem->m_lEndurance);
			return TRUE;
		}
	}
	return FALSE;
}

LONG CRoomInternalLogic::GetFishDropItem(CUser* pUser, TMapInven& mapNewInven, TMapInven& mapOldInven)
{
	LONG lErr = NF::G_NF_ERR_SUCCESS;
	CNFChar& nfChar = pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt = nfChar.GetNFChar().m_nfCharInfoExt;
	CFish BiteFish = nfChar.GetBiteFish();

	FishInfo& fishInfo = BiteFish.GetFish();

	TMapDropItemRate& mapDropItemRate = theNFDataItemMgr.GetDropItemRate();
	TMapDropItemRate::iterator iter = mapDropItemRate.find(fishInfo.m_lDropItemIdx);
	if (iter != mapDropItemRate.end())
	{
		for(int i=0; i<fishInfo.m_lDropItemMax; i++)
		{
			LONG lRateSum = 0;
			LONG lDropItemRate = urandom(1000);			// 소수점 한자리까지...

			ForEachElmt(TlstDropItemRate, (*iter).second, it, ij)
			{
				lRateSum += (*it).m_lDropRate;
				if (lRateSum > 1000)
					break;

				if (lRateSum >= lDropItemRate)		// 아이템 득...
				{
					NFInvenSlot	inven; 	inven.Clear();
					Product get_product;	get_product.Clear();
					if (!theNFDataItemMgr.GetProductFromGiveItem((*it).m_lDropItemID, get_product)) {
						lErr = NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMID;
						continue;
					}
					LONG lCapacity = 0;
					lErr = theNFMenu.GetInvenSRLFromExistInven(nfCharInfoExt.m_nfCharInven, inven, get_product.m_lItemCode, lCapacity);
					if (NF::G_NF_ERR_SUCCESS != lErr)
						continue;

					LONG lTempInvenSRL = inven.m_lInvenSRL;
					if (0 == inven.m_lInvenSRL)
					{
						inven.m_lItemCode = get_product.m_lItemCode;
						inven.m_lPartsIndex = get_product.m_lItemCode/G_VALUE_CONVERT_PARTS;
					}

					// DB로 Query
					theNFDBMgr.InsertBuyItem(1, (*it).m_lDropItemID, pUser->GetGSN(), pUser->GetCSN(), inven, (int&)lErr);
					if (NF::G_NF_ERR_SUCCESS != lErr)		// inven이 full일 경우, 메일로 보냈다는 걸 알려줘야 한다...
						return -2;		// DB Insert Failed

					NFInvenSlot newInven; newInven.Clear();
					lErr = theNFItem.ProcessCapacityInven(nfCharInfoExt.m_nfCharInven, lTempInvenSRL, inven, newInven, get_product.m_lItemCNT, lCapacity, mapOldInven, mapNewInven);
					if (NF::G_NF_ERR_SUCCESS != lErr)
						return lErr;
					break;
				}
			}
		}
	}
	else {
		theLog.Put(WAR_UK, "NGS_LOGIC, GetFishDropItem(). Not Found DropITem Index : ", fishInfo.m_lDropItemIdx);
		return -1;		// not found lDropItemIdx
	}
	return 1;
}

// Landing 조건이 되면, 게임 결과를 저장하기 위해 호출되는 함수
// 게임 결과 메세지 전송, 
// Single 이면, 조건에 해당하는 상위 5명에 대한 정보 보내주기
// Team 이면, 조건에 대한 정보만 보내주기
void CRoomInternalLogic::SaveGameResult(CUser* pUser)
{
	CNFChar& nfChar = pUser->GetNFUser();
	CFish& biteFish = nfChar.GetBiteFish();
	LONG lCSN = pUser->GetCSN();

	MsgNGSCli_NotifyLandingResult		landingNotify;

	// 1. 랜딩 결과를 DB에 저장한다.
	// 2. 랜딩에 대한 정보를 방안에 있는 모든 유저들에게 전송한다.
	// 3. 랜딩에 대한 정보를 NGS에 물려잇는 CHS로 전송한다.
	if (!biteFish.ValidCheck())
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult(). BiteFish InValide. RoomID : ", RoomID2Str(GetRoomID()), ", CSN :", pUser->GetCSN());

		landingNotify.m_llGetGameMoney = 0;
		landingNotify.m_lGetExp = 0;

		PayloadNGSCli pldLanding(PayloadNGSCli::msgNotifyLandingResult_Tag, landingNotify);
		UserManager->SendToUser(lCSN, pldLanding);

		return;
	}

	// 랜딩보너스
	eLandingBonusType landing_bonus_type = static_cast<eLandingBonusType>( pUser->GetAbil( AT_LANDING_BONUS ) );
	switch( landing_bonus_type )
	{
	case eLandingBonusType_ExpX2:
		{
			biteFish.SetFishResultExp(biteFish.GetFishResultExp()*2);
		}break;
	case eLandingBonusType_ExpX3:
		{
			biteFish.SetFishResultExp(biteFish.GetFishResultExp()*3);
		}break;
	case eLandingBonusType_ExpX4:
		{
			biteFish.SetFishResultExp(biteFish.GetFishResultExp()*4);
		}break;
	case eLandingBonusType_ExpX5:
		{
			biteFish.SetFishResultExp(biteFish.GetFishResultExp()*5);
		}break;
	case eLandingBonusType_GoldX2:
		{
			biteFish.SetRewardGameMoney(biteFish.GetRewardGameMoney()*2);
		}break;
	case eLandingBonusType_GoldX3:
		{
			biteFish.SetRewardGameMoney(biteFish.GetRewardGameMoney()*3);
		}break;
	case eLandingBonusType_GoldX4:
		{
			biteFish.SetRewardGameMoney(biteFish.GetRewardGameMoney()*4);
		}break;
	case eLandingBonusType_GoldX5:
		{
			biteFish.SetRewardGameMoney(biteFish.GetRewardGameMoney()*5);
		}break;
	default:
		{
		}break;
	}

	// 랜딩보너스 리셋
	pUser->SetAbil( AT_LANDING_BONUS, static_cast<int>( eLandingBonusType_None ) );

	// 경험치 가져오기
	LONG lErr = theNFMenu.CalcLevelByAddExp(pUser->GetGSN(), pUser->GetNFCharInfoExt(), biteFish.GetFishResultExp(), GetRoomID());
	if (NF::G_NF_ERR_SUCCESS != lErr) 
	{
		if( NF::G_ERR_GET_NF_EXP_MAX == lErr ) // 만렙이면 더이상 먹을 경험치 없다.
			biteFish.SetFishResultExp(0);
		else
		{
			theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult(). CalcLevelByAddExp Error!!!. RoomID : ", RoomID2Str(GetRoomID()), ", CSN :", pUser->GetCSN(), ", ErrorCode :", lErr);
			return;
		}
	}

	// 물고기 결과 저장하기(누적하기) - 개인
	// 누적 할 때마다 소팅까지 한다.
	nfChar.SaveLandingFish(m_nfRoomOption.m_lWinCondition);

	// 대회일 경우, 물고기 결과 저장하기 - TeamPlay
	if (m_nfRoomOption.m_lPlayType == PT_BATTLE_TEAM)
		SaveLandingResultTeam(pUser);

	// 혹시 에러 발생시 무조건 1번
	LONG lSignType = 0;

	// 징조 포인트에 더한다.
	if (CheckSignPoint(biteFish.GetFish(), lSignType))
		theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult, CheckSignPoint Success!!! RoomID :", RoomID2Str(m_RoomID), " / SignType :", lSignType);

	//////////////////////////////////////////////////////////////////////////
	// DB에 잡은 물고기 저장 & 잠금 노트도 업데이트
	lErr = SaveLandingFishInfoToDB(nfChar, biteFish);
	if (NF::G_NF_ERR_SUCCESS != lErr)
		theLog.Put(ERR_UK, "NGS_LOGIC, SaveGameResult, SaveLockedNote Failed!!! RoomID :", RoomID2Str(m_RoomID), " / CSN :", pUser->GetCSN(), " / ErrorType :", lErr);

	//////////////////////////////////////////////////////////////////////////
	// LandingNotify 값을 셋팅한다...
	theNFDataItemMgr.GetNFExp(nfChar.GetLevel(), landingNotify.m_lMaxExp);

	// 획득 아이템을 결정한다...
	LONG lError = GetFishDropItem(pUser, landingNotify.m_mapNewInven, landingNotify.m_mapOldInven);
	if (1 != lError)
		theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult, GetFishDropItem Failed!!!!! CSN:", landingNotify.m_lCSN, ", Err :", lError);

	theLog.Put(WAR_UK, "NGS_LOGIC, OnReqFighting(), ---------------- Ntf CSN : ", landingNotify.m_lCSN, ", Money : ", landingNotify.m_llGetGameMoney);

	landingNotify.m_lCSN = nfChar.GetCSN();
	landingNotify.m_lLevel = nfChar.GetLevel();
	landingNotify.m_lExp = nfChar.GetExp();
	landingNotify.m_llGetGameMoney = biteFish.GetRewardGameMoney();
	landingNotify.m_lGetExp = biteFish.GetFishResultExp();
	landingNotify.m_nfAbility = nfChar.GetAbility();
	
	LONG lMapID = m_nfRoomOption.m_lIdxFishMap;
	TMapLockedNoteMap& mapLandNote = nfChar.GetNFChar().m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMap;
	TMapLockedNoteMap::iterator iter = mapLandNote.find(lMapID);
	if (iter != mapLandNote.end())
	{
		landingNotify.m_lTotLockedScore = iter->second.m_lTotLockedScore;
	}
	else
	{
		landingNotify.m_lTotLockedScore = 0;
		theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult(). NFLockedNoteMap not found. MapID : ", lMapID);
	}

	PayloadNGSCli pldLanding(PayloadNGSCli::msgNotifyLandingResult_Tag, landingNotify);
	UserManager->SendToUser(lCSN, pldLanding);


	///////////////////////////////////////////////////////////
	// Fish Landing 시, GameResult 정보를 전체 유저에게 Simple하게 전송
	MsgNGSCli_NotifySimpleGameReuslt	gameResultAns;

	if (!GetGameResult(pUser, gameResultAns.m_lstPlayer, gameResultAns.m_lstTeam, TRUE, FALSE))		// Detail 정보를 요청하지 않음
		theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult(). GetGameResult Failed. RoomID : ", RoomID2Str(GetRoomID()), ", CSN :", pUser->GetCSN());

	PayloadNGSCli pldGameResult(PayloadNGSCli::msgNotifySimpleGameResult_Tag, gameResultAns);
	UserManager->SendToAllUser(pldGameResult);
}

BOOL CRoomInternalLogic::PrevLoadLockedNote(CNFChar& nfChar, CFish& landingFish, NFLockedNoteMain& locked_main, LONG& lLengthIndex, LONG& lClassIndex, std::string& strFishGroup, std::string& strLength, std::string& strRegDate, std::string& strMapID)
{
	nfChar.GetLockedNoteMain(locked_main);

	// 최고 길이 2종
	LONG lCNT = 0;
	if (locked_main.m_vecTopLengthFish.size() != G_LOCKED_NOTE_MAIN_TOP_CNT)
		return FALSE;

	ForEachElmt(TVecRecentlyLandingFish, locked_main.m_vecTopLengthFish, it, ij)
	{
		++lCNT;
		if (landingFish.GetFishResultLength() > (*it).m_lLength) {
			landingFish.GetLockedNoteMainFishInfo((*it));
			lLengthIndex = lCNT;
			break;
		}
	}

	// 최고 클래스 2종
	lCNT = 0;
	if (locked_main.m_vecTopClassFish.size() != G_LOCKED_NOTE_MAIN_TOP_CNT)
		return FALSE;

	ForEachElmt(TVecRecentlyLandingFish, locked_main.m_vecTopClassFish, it, ij)
	{
		++lCNT;
		if (landingFish.GetFish().m_lFishClass > (*it).m_lClass) {
			landingFish.GetLockedNoteMainFishInfo((*it));
			lClassIndex = lCNT;
			break;
		}
	}

	// 최근에 잡은 물고기
	if (locked_main.m_vecRecentlyLandingFish.size() != G_LOCKED_NOTE_RECENTLY_CNT)
		return FALSE;

	Recently_Landing_Fish fish;
	fish.Clear();
	landingFish.GetLockedNoteMainFishInfo(fish);

	TVecRecentlyLandingFish::iterator del_iter = locked_main.m_vecRecentlyLandingFish.begin();
	locked_main.m_vecRecentlyLandingFish.erase(del_iter);
	locked_main.m_vecRecentlyLandingFish.push_back(fish);

	// string으로 변환
	ForEachElmt(TVecRecentlyLandingFish, locked_main.m_vecRecentlyLandingFish, it2, ij2)
	{
		char szTemp[255] = {0, };
		sprintf(szTemp, "%d_", (*it2).m_lLockedNoteFishID);
		strFishGroup += szTemp;

		memset(szTemp, 0x00, 255);
		sprintf(szTemp, "%d_", (*it2).m_lLength);
		strLength += szTemp;

		memset(szTemp, 0x00, 255);
		sprintf(szTemp, "%d_", (*it2).m_strUpdateDate);
		strRegDate += szTemp;

		memset(szTemp, 0x00, 255);
		sprintf(szTemp, "%d_", (*it2).m_lMapID);
		strMapID += szTemp;
	}

	return TRUE;
}

LONG CRoomInternalLogic::SaveLandingFishInfoToDB(CNFChar& nfChar, CFish& landingFish)
{
	LONG lErr = NF::G_NF_ERR_SUCCESS;

	GameResult	result;
	result.Clear();
	LONG lErrorCode = 0;

	GetFreeModeGaemResult(nfChar, result);

	NFLockedNoteMain	locked_main;
	locked_main.Clear();

	string strFishGroup, strLength, strRegDate, strMapID;

	LONG lLengthIndex = 0, lClassIndex = 0;
	if (!PrevLoadLockedNote(nfChar, landingFish, locked_main, lLengthIndex, lClassIndex, strFishGroup, strLength, strRegDate, strMapID)) {
		theLog.Put(WAR_UK, "NGS_LOGIC, PrevLoadLockedNote Failed... CSN:", result.m_lCSN, "// Err:", lErrorCode);
		return NF::G_NF_ERR_PREV_LOADING_LOCKED_MAIN;
	}

	if (!theNFDBMgr.UpdateLandingFish(result, m_nfRoomOption, lLengthIndex, lClassIndex, strFishGroup, strLength, strRegDate, strMapID, lErrorCode)) {
		theLog.Put(WAR_UK, "NGS_LOGIC, UpdateLandingFish Failed... CSN:", result.m_lCSN, "// Err:", lErrorCode);
		return NF::G_NF_ERR_DB_UPDATE_LOCKED_NOTE;
	}

	lErr = CheckLockedNote(nfChar, landingFish);

	nfChar.GetSaveLandingFish().Clear();
	return lErr;
}

LONG CRoomInternalLogic::CheckLockedNote(CNFChar& nfChar, CFish& landingFish)
{
	LONG lErr = NF::G_NF_ERR_SUCCESS;

	////////////////// LandNote update
	LONG lMapID = landingFish.GetFishMapID();
	if (lMapID <= 0)
		return NF::EC_LN_INVALID_MAPID;

	// LandNote FishInfo
	LockedNote_Fish_Info		landFish;
	landFish.Clear();

	landFish.m_lLength = (LONG)landingFish.GetFishResultLength();
	landFish.m_lWeight = (LONG)landingFish.GetFishResultWeight();
	landFish.m_strUpdateDate = landingFish.GetLandingDate();
	landFish.m_lScore = landingFish.GetResultFishScore();

	LONG lLockedNoteFishID = landingFish.GetLandNoteFishID();
	if (lLockedNoteFishID < 0)
		return NF::EC_LN_INVALID_FISHID;

	// server memory-check
	TMapLockedNoteMap& mapLandNote = nfChar.GetNFChar().m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMap;
	TMapLockedNoteMap::iterator iter = mapLandNote.find(lMapID);
	if (iter != mapLandNote.end())
	{
		// 해당 맵에서 한 마리라도 잡은 경우...
		TMapLockedNote::iterator iterFish = (*iter).second.m_TblLockedNote.find(lLockedNoteFishID);	
		if (iterFish == (*iter).second.m_TblLockedNote.end()) 
		{
			// 잡은 물고기가 없은 경우, 무조건 insert
			(*iter).second.m_TblLockedNote.insert(make_pair(lLockedNoteFishID, landFish));
			(*iter).second.m_lTotUnlockFishCNT++;
			nfChar.GetNFChar().m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMain.m_lTotCNTLandFish++;
		}
		else
		{
			// 잡은 물고기가 있는 경우, 기존에 잡은 물고기와 비교해서 업데이트 유무 판단...
			if ((*iterFish).second.m_lLength < landFish.m_lLength)
				(*iterFish).second = landFish;
			
		}
		(*iter).second.m_lTotLockedScore += nfChar.GetBiteFish().GetResultFishScore();	// 물고기 잡으면 무조건 Score는 누적되고, 마리수는 기존에 잡은적이 없어야만, ++된다.
	}
	else
	{	
		// 해당 맵에서 처음으로 잡은 경우..
		NFLockedNoteMap  lockedNoteMap;
		lockedNoteMap.Clear();
		lockedNoteMap.m_lTotLockedScore = nfChar.GetBiteFish().GetResultFishScore();
		lockedNoteMap.m_lTotUnlockFishCNT = 1;

		lockedNoteMap.m_TblLockedNote.insert(make_pair(lLockedNoteFishID, landFish));
		mapLandNote.insert(make_pair(lMapID, lockedNoteMap));
		nfChar.GetNFChar().m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMain.m_lTotCNTLandFish++;
	}

	if (NF::G_NF_ERR_SUCCESS != lErr)
		return NF::G_NF_ERR_DB_UPDATE_LOCKED_NOTE;

	return lErr;
}

// 대회일 경우, 각 팀에 해당하는 유저의 LandingFish정보를 각 팀에 저장한다.
void CRoomInternalLogic::SaveLandingResultTeam(CUser* pUser)
{
	CNFChar& nfChar = pUser->GetNFUser();
	CFish& BiteFish = nfChar.GetBiteFish();
	TotalLandingFish* totResult = NULL;

	// 슬롯 번호가 0 or 짝수이면, A팀
	if (pUser->GetUserSlot() % 2 == 0)
		totResult = &m_totLandingFishTeamA;
	else
		// 슬롯 번호가 홀수이면, B팀
		totResult = &m_totLandingFishTeamB;

	LandingFish landingFish;
	BiteFish.GetLandingFish(landingFish);
	totResult->m_lstLandingFish.push_back(landingFish);

	// get_result_sorting(5)
	SortingLandingFishResultTeam(totResult, m_nfRoomOption.m_lWinCondition, 5);
}

void CRoomInternalLogic::SortingLandingFishResultTeam(TotalLandingFish* totResult, LONG lWinCondition, LONG lTopSize)
{
	// 누적 때문에 초기화!!!
	totResult->m_lTotScore = 0;
	totResult->m_dTotSize = 0;
	totResult->m_dTotWeight = 0;

	// count
	totResult->m_lCatchCount = totResult->m_lstLandingFish.size();

	// 
	// 
	switch(lWinCondition)
	{
	case ERW_MAXSIZE:
	case ERW_TOT_SIZE_COUNT:
		{
			totResult->m_lstLandingFish.sort(CompareSize_LandingFish);	
		}
		break;

	case ERW_MAXWEIGHT:
	case ERW_TOT_WEIGHT_COUNT:
		{
			totResult->m_lstLandingFish.sort(CompareWeight_LandingFish);
		}
		break;

	default:
		break;
	}

	LONG lCnt = 0;
	ForEachElmt(TLstLandingFish, totResult->m_lstLandingFish, it, ij)
	{
		totResult->m_lTotScore += (*it).m_lResultScore;

		if (lTopSize >= ++lCnt)
			totResult->m_dTotSize += (*it).m_dResultSize;

		if (lTopSize >= lCnt)
			totResult->m_dTotWeight += (*it).m_dResultWeight;

		if (lCnt == 1) {
			totResult->m_dMaxSize = (*it).m_dResultSize;
			totResult->m_dMaxWeight = (*it).m_dResultWeight;
		}
		else {
			if (totResult->m_dMaxSize < (*it).m_dResultSize)
				totResult->m_dMaxSize = (*it).m_dResultSize;

			if (totResult->m_dMaxWeight < (*it).m_dResultWeight)
				totResult->m_dMaxWeight = (*it).m_dResultWeight;
		}

		totResult->m_lTotExp = (*it).m_lExp;
		totResult->m_lBonusExp = (*it).m_lBonusExp;
		totResult->m_llTotMoney = (*it).m_llMoney;
	}
}

void CRoomInternalLogic::SortingGameResult(TLstGameResult& lst, LONG lWinCondition, LONG lTopSize)
{
	// 
	switch(lWinCondition)
	{
	case ERW_MAXCATCHCOUNT:
		{
			lst.sort(CompareCount_GameResult<GameResult>);
			break;
		}
	case ERW_MAXSIZE:
		{
			lst.sort(CompareMaxSize_GameResult<GameResult>);
			break;
		}
	case ERW_TOT_SIZE_COUNT:
		{
			lst.sort(CompareTotSize_GameResult<GameResult>);
			break;
		}
	case ERW_MAXWEIGHT:
		{
			lst.sort(CompareMaxWeight_GameResult<GameResult>);
			break;
		}
	case ERW_TOT_WEIGHT_COUNT:
		{
			lst.sort(CompareTotWeight_GameResult<GameResult>);
			break;
		}
	case ERW_TOT_SCORE:
		{
			lst.sort(CompareScore_GameResult<GameResult>);	
			break;
		}
	default:
		break;
	}
}

BOOL CRoomInternalLogic::GetGameResult(CUser* pUser, TLstGameResult& lstPlayer, TLstGameResult& lstTeam, BOOL bIsSimple, BOOL bIsUpateDB)
{
	BOOL bRet = TRUE;

	if (PT_FREE == m_nfRoomOption.m_lPlayType)
		return bRet;		// FreeMode 일 경우에는 저장 할것이 없음..
	else if (PT_BATTLE_SINGLE == m_nfRoomOption.m_lPlayType)
	{
		if (!GetSingleGameResult(lstPlayer, bIsSimple))
			theLog.Put(WAR_UK, "NGS_LOGIC, GetGameResult(). Failed. RoomID : ", RoomID2Str(GetRoomID()));
	}
	else if (PT_BATTLE_TEAM == m_nfRoomOption.m_lPlayType)
	{
		if (!GetTeamGameResult(lstPlayer, lstTeam, bIsSimple))
			theLog.Put(WAR_UK, "NGS_LOGIC, GetGameResult(). Failed. RoomID : ", RoomID2Str(GetRoomID()));
	}

	// DB 저장...
	ForEachElmt(TLstGameResult, lstPlayer, it, ij)
	{
		if (bIsUpateDB)
		{
			LONG lErrorCode = 0;
			if (!theNFDBMgr.UpdateNFCharDataByCSN((*it), m_nfRoomOption, lErrorCode))
				theLog.Put(WAR_UK, "NGS_LOGIC, UpdateNFCharDataByCSN Failed... CSN:", (*it).m_lCSN, "// ErrorCode: ", lErrorCode);
		}
		theLog.Put(WAR_UK, "NGS_LOGIC, ##### NickName : ", (*it).m_strName.c_str(), ", Rank : ", (*it).m_lRank, ", MaxSize : ", (*it).m_dMaxSize, ", MaxWeight : ", (*it).m_dMaxWeight);
	}

	ForEachElmt(TLstGameResult, lstTeam, it, ij)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, ##### NickName : ", (*it).m_strName.c_str(), ", MaxSize : ", (*it).m_dMaxSize, ", MaxWeight : ", (*it).m_dMaxWeight);
	}

	return bRet;
}

BOOL CRoomInternalLogic::CalcRankGameResultSingle(TLstGameResult& lstPlayer, LONG lWinCondition)
{
	double	dCurValue = 0.0, dPrevValue = 0.0;
	LONG	lRank = 0;
	LONG	lSameCnt = 1;

	ForEachElmt(TLstGameResult, lstPlayer, it, ij)
	{
		// 
		switch(lWinCondition)
		{
		case ERW_MAXCATCHCOUNT:
			{
				dCurValue = (*it).m_lCatchCount;
				break;
			}
		case ERW_MAXSIZE:
			{
				dCurValue = (*it).m_dMaxSize;
				break;
			}
		case ERW_TOT_SIZE_COUNT:
			{
				dCurValue = (*it).m_dTotSize;
				break;
			}
		case ERW_MAXWEIGHT:
			{
				dCurValue = (*it).m_dMaxWeight;
				break;
			}
		case ERW_TOT_WEIGHT_COUNT:
			{
				dCurValue = (*it).m_dTotWeight;
				break;
			}
		case ERW_TOT_SCORE:
			{
				dCurValue = (*it).m_lTotScore;
				break;
			}
		default:
			break;
		}

		//
		if ((lstPlayer.size() != 1) && (dCurValue == dPrevValue))
			++lSameCnt;
		else
		{
			if (lSameCnt != 1)
			{
				lRank += lSameCnt;
				lSameCnt = 1;
			}
			else
				++lRank;
		}
		
		// 개인전 일 경우에만, 한마리도 못 잡으면 시상식대에 올리지 않아야 하기에...
		// GetSingleGameResult에서 -1로 설정해놓은 유저는 Rank가 -1이기에 .. 그대로 -1로 셋팅
		if ((*it).m_lRank != -1)
			(*it).m_lRank = lRank;
		dPrevValue = dCurValue;
	}

	if (lSameCnt == (LONG)(lstPlayer.size() + 1))		// Draw
		return TRUE;
	return FALSE;
}

BOOL CRoomInternalLogic::CalcRankGameResultTeam(TLstGameResult& lstPlayer, LONG lWinCondition)
{
	double	dCurValue = 0.0, dPrevValue = 0.0;
	LONG	lRank = 0;
	BOOL	bIsDraw = FALSE;

	ForEachElmt(TLstGameResult, lstPlayer, it, ij)
	{
		// 
		switch(lWinCondition)
		{
		case ERW_MAXCATCHCOUNT:
			{
				dCurValue = (*it).m_lCatchCount;
				break;
			}
		case ERW_MAXSIZE:
			{
				dCurValue = (*it).m_dMaxSize;
				break;
			}
		case ERW_TOT_SIZE_COUNT:
			{
				dCurValue = (*it).m_dTotSize;
				break;
			}
		case ERW_MAXWEIGHT:
			{
				dCurValue = (*it).m_dMaxWeight;
				break;
			}
		case ERW_TOT_WEIGHT_COUNT:
			{
				dCurValue = (*it).m_dTotWeight;
				break;
			}
		case ERW_TOT_SCORE:
			{
				dCurValue = (*it).m_lTotScore;
				break;
			}
		default:
			break;
		}

		// 팀전일 경우는 랭킹이 1부터 시작하고, 두팀일 경우에 동률은 1, 1이 된다. 
		// 여러 팀이 되면, 1, 1, 3 이런식으로...
		if (dCurValue != dPrevValue)
			++lRank;
		else
			bIsDraw = TRUE;

		(*it).m_lRank = lRank;
	
		dPrevValue = dCurValue;

		if (bIsDraw)
			(*it).m_lWinType = 3;
		else
		{
			if ((*it).m_lRank == 1)
				(*it).m_lWinType = 1;
			else
				(*it).m_lWinType = 2;
		}			
	}
	return FALSE;
}

// FreeMode Play시,
BOOL CRoomInternalLogic::GetFreeModeGaemResult(const CNFChar& nfChar, GameResult& gameResult)
{
	GameResult tempGameResult(nfChar.GetSaveLandingFish(), 0, nfChar.GetLevel(), nfChar.GetGSN(), nfChar.GetCSN(), nfChar.GetExp());
	gameResult = tempGameResult;
	return TRUE;
}

// Single Play시, 
BOOL CRoomInternalLogic::GetSingleGameResult(TLstGameResult& lstPlayer, BOOL bIsSimple)
{
	UserManager->GetSingleGameResult(lstPlayer, bIsSimple);

	// Sorting
	if (bIsSimple)		// 간단히 보여줄 것이면, 상위 5명만
		SortingGameResult(lstPlayer, m_nfRoomOption.m_lWinCondition, 5);
	else
		SortingGameResult(lstPlayer, m_nfRoomOption.m_lWinCondition, 0);

	BOOL bIsDraw = CalcRankGameResultSingle(lstPlayer, m_nfRoomOption.m_lWinCondition);

	// Win/Lose/Draw Check
	ForEachElmt(TLstGameResult, lstPlayer, it, ij)
	{
		if (bIsDraw)
			(*it).m_lWinType = 3;
	}

	return TRUE;
}

// Team 대전시,
BOOL CRoomInternalLogic::GetTeamGameResult(TLstGameResult& lstPlayer, TLstGameResult& lstTeam, BOOL bIsSimple)
{	
	BOOL bRet = TRUE;

	TLstGameResult		lstATeamPlayer, lstBTeamPlayer;

	if (!bIsSimple)		// Detail이면, 
	{
		// 1. lstPlayer를 두팀으로 나눈다.
		UserManager->GetTeamGameResult(lstATeamPlayer, lstBTeamPlayer, bIsSimple);

		// 2. A, B팀을 Sorting한다.
		SortingGameResult(lstATeamPlayer, m_nfRoomOption.m_lWinCondition);
		SortingGameResult(lstBTeamPlayer, m_nfRoomOption.m_lWinCondition);
		
		// 2-1-1. 소팅후 Ranking
		CalcRankGameResultSingle(lstATeamPlayer, m_nfRoomOption.m_lWinCondition);
		CalcRankGameResultSingle(lstBTeamPlayer, m_nfRoomOption.m_lWinCondition);

		// 2-1, A, B팀 유저들을 lstPlayer에 추가한다.
		ForEachElmt(TLstGameResult, lstATeamPlayer, it, ij)
			lstPlayer.push_back((*it));

		// 2-1, A, B팀 유저들을 lstPlayer에 추가한다.
		ForEachElmt(TLstGameResult, lstBTeamPlayer, it, ij)
			lstPlayer.push_back((*it));

		// 3. Team만 따로 소팅한다.
		// A팀은 0, B팀은 1로 강제로 셋팅한다. - 나중에 팀이 여러개 생길 경우 다시 고려 해야 함!!!
		GameResult gameATeamResult(m_totLandingFishTeamA, 1), gameBTeamResult(m_totLandingFishTeamB, 2);
		lstTeam.push_back(gameATeamResult);
		lstTeam.push_back(gameBTeamResult);
		
		// 4. 승리팀을 결정한다.
		SortingGameResult(lstTeam, m_nfRoomOption.m_lWinCondition);
		CalcRankGameResultTeam(lstTeam, m_nfRoomOption.m_lWinCondition);

		
		TMapTeamData	mapTeamData;

		// 5.승리팀 순위 
		ForEachElmt(TLstGameResult, lstTeam, it, ij) 
		{
			CRankNWinType		team;
			team.lRank = (*it).m_lRank;
			team.lWinType = (*it).m_lWinType;
			mapTeamData[(*it).m_lBattleType-1] = team;
		}

		// 6. 팀에있는 플레이어들의 랭킹을 정한다.
		ForEachElmt(TLstGameResult, lstPlayer, it, ij) {
			TMapTeamData::iterator	iter = mapTeamData.find((*it).m_lBattleType-1);
			if (iter == mapTeamData.end())
				return FALSE;

			(*it).m_lWinType = mapTeamData[(*it).m_lBattleType-1].lWinType;
			(*it).m_lRank = mapTeamData[(*it).m_lBattleType-1].lRank;
		}
	}
	else		// 간단하게 보여줘야 된다면, 
	{
		GameResult gameATeamResult(m_totLandingFishTeamA), gameBTeamResult(m_totLandingFishTeamB);
		lstTeam.push_back(gameATeamResult);
		lstTeam.push_back(gameBTeamResult);
	}

	return bRet;
}

// 스킬(SkillItem, UsableItem) 사용시 호출되는 함수
// Invalid Check, DB update (activate_gameitem procedure call), send ans msg to client 
long CRoomInternalLogic::ApplyUseItem(CUser* pUser, LONG lItemCode, LONG lQuickSlotIndex, AnsUsedItem& usedItem)
{
	CNFChar& nfChar = pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt = nfChar.GetNFChar().m_nfCharInfoExt;

	//// 자신의 Inven에서 검색해온다. usable지 skill인지 구분해야 함
	//TMapInvenSlotList::iterator iter = nfCharInfoExt.m_nfCharInven.m_mapCountableItem.find(lItemCode);
	//if (iter == nfCharInfoExt.m_nfCharInven.m_mapCountableItem.end())
	//	return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;		// not found to my usable inven

	//// 인벤에서 아이템 카운드가 제일 적은 인벤부터 가지고 온다...
	//ForEachElmt(TlstInvenSlot, (*iter).second, it, ij) 
	//{
	//	if (usedItem.m_usedInven.m_lItemCode <= 0)
	//		usedItem.m_usedInven = (*it);

	//	if (usedItem.m_usedInven.m_lRemainCount > (*it).m_lRemainCount) {
	//		usedItem.m_usedInven = (*it);
	//		break;
	//	}
	//}

	//if (usedItem.m_usedInven.m_lInvenSRL == 0 || usedItem.m_usedInven.m_lItemCode == 0)
	//	return NF::G_NF_ERR_INVALID_ITEM_SRL;

	usedItem.m_lErrorCode = theNFMenu.FindReduceInvenSlot(nfCharInfoExt.m_nfCharInven.m_mapCountableItem, usedItem.m_usedInven, lItemCode);
	if (NF::G_NF_ERR_SUCCESS != usedItem.m_lErrorCode)
		return usedItem.m_lErrorCode;

	// 아이템이 레벨에 따라 사용 가능한지 체크
	ItemCommon* itemCommon = NULL;
	if (usedItem.m_usedInven.m_lPartsIndex == eItemType_SkillItem) {
		itemCommon = theNFDataItemMgr.GetSkillItemByIndex(usedItem.m_usedInven.m_lItemCode);
		usedItem.m_usedInven.m_strItemCategory = "S";
	}
	else if (usedItem.m_usedInven.m_lPartsIndex == eItemType_UsableItem) {
		itemCommon = theNFDataItemMgr.GetUsableItemByIndex(usedItem.m_usedInven.m_lItemCode);
		usedItem.m_usedInven.m_strItemCategory = "U";
	}
	else 
		return NF::G_NF_ERR_PARTS_INDEX;

	if (!itemCommon)		
		return NF::G_NF_ERR_NOT_FOUND_ITEM_BY_ITEMCODE;

	if (itemCommon->m_lLv > nfCharInfoExt.m_nfCharBaseInfo.m_lLevel)
		return NF::G_NF_ERR_ITEM_LEVEL;

	LONG lGauge = 1;
	LONG lExcpectationGauge = usedItem.m_usedInven.m_lRemainCount - lGauge;

	if (!theNFDBMgr.UseGameFunc(pUser->GetGSN(), pUser->GetCSN(), usedItem.m_usedInven, "Y", lGauge, usedItem.m_lErrorCode))
		return NF::G_NF_ERR_DB_USE_GAMEFUNC;		// db failed
	else
	{
		if (lExcpectationGauge != lGauge)
			theLog.Put(ERR_UK, "NGS_LOGIC, ApplyUseItem!!! DB : ", lGauge, " vs Server : ", lExcpectationGauge, " Not match Gauge, CSN :", pUser->GetCSN());

		usedItem.m_usedInven.m_lRemainCount = lGauge;

		if (usedItem.m_usedInven.m_lRemainCount <= 0) {
			theNFItem.RemoveItem(nfCharInfoExt.m_nfCharInven, usedItem.m_usedInven);
			usedItem.m_bIsRemove = TRUE;
		}
		else {
			usedItem.m_lErrorCode = theNFMenu.SetExistCountableItemToInven(nfCharInfoExt.m_nfCharInven, usedItem.m_usedInven);
			if (NF::G_NF_ERR_SUCCESS != usedItem.m_lErrorCode)
				return usedItem.m_lErrorCode;
		}
	}

	return usedItem.m_lErrorCode;
}

BOOL CRoomInternalLogic::CheckSignPoint(FishInfo& biteFishInfo, LONG& lSignIndex)
{
	//m_lSignCurrentPoint += biteFishInfo.m_lSignFishPoint;
	m_lSignCurrentPoint += 50;

	const LONG lForceSignIndex = GetAbil(AT_REQSIGN);

	if (m_lSignCurrentPoint >= m_lSignMaxPoint || 0 < lForceSignIndex)
	{
		m_lSignCurrentPoint = 0;

		if (GetSignType() >= 1) // 이미 징조 발생중인데
		{
			if( 0 < lForceSignIndex ) // 치트로 발생시킨 거면 기존 징조 없애고 새로운 징조 발생시킨다.
			{
				NtfEndSignMsg();
			}
			else
			{
				return FALSE;
			}
		}	

		// 어떤 징조를 발생 할 것인지....
		TMapIndexFishMap mapNFMap = theNFDataItemMgr.GetFishMap();
		TMapSignProb mapSignProb = mapNFMap[m_nfRoomOption.m_lIdxFishMap]->m_mapSignProb;

		if (mapSignProb.size() > 0)
		{
			int nRand = urandom(100);
			double dSignRand = 0;
			ForEachElmt(TMapSignProb, mapSignProb, it, ij)
			{
				dSignRand += (*it).second.m_dSignProb;

				if (dSignRand >= (double)nRand)
				{
					if( 0 < lForceSignIndex )
					{
						lSignIndex = lForceSignIndex;
						SetAbil(AT_REQSIGN, 0);
					}
					else
					{
						lSignIndex = (*it).first;
					}

					NtfStartSignMsg(lSignIndex);

					// 현재 징조중
					SetSigning(lSignIndex);

					// 징조가 시작됐음을 타이머를 통해 알린다.
					AddTimer(SIGN_INTERVAL, SIGN_INTERVAL, TIMER_INDEX_SIGN);

					return TRUE;
				}
			}

			theLog.Put(WAR_UK, "NGS_LOGIC, CheckSignPoint, Error!!! SignProb Total :", dSignRand);
		}
		else
			theLog.Put(WAR_UK, "NGS_LOGIC, CheckSignPoint, Error!!! SignProb Size :", mapSignProb.size());
	}
	return FALSE;
}

BOOL CRoomInternalLogic::CheckValidItem(NFCharInfoExt& nfCharInfoExt, LONG lItemType)
{
	LONG lItemCode = 0;
	// 현재 착용하고 있는 Line아이템의 m를 체크
	TMapInven::iterator iterCode = nfCharInfoExt.m_nfCharInven.m_mapUsingItem.find(lItemType);
	if (iterCode != nfCharInfoExt.m_nfCharInven.m_mapUsingItem.end())
		lItemCode = (*iterCode).second.m_lItemCode;
	else
		return FALSE;

	BOOL bIsSuccess = FALSE;
	int nItemMaxCount = 0;
	if (lItemType == eItemType_Line)
		nItemMaxCount = AI_LINE_MIN_REMAIN_COUNT;

	TMapInvenSlotList::iterator iter = nfCharInfoExt.m_nfCharInven.m_mapCountableItem.find(lItemCode);
	if (iter != nfCharInfoExt.m_nfCharInven.m_mapCountableItem.end())
	{
		int nTotCount = 0;
		ForEachElmt(TlstInvenSlot, (*iter).second, it, ij)
		{
			nTotCount += (*it).m_lRemainCount;
			if (nTotCount > nItemMaxCount) {
				bIsSuccess = TRUE;
				break;
			}
		}
	}
	else
		return FALSE;

	// 디폴트 아이템으로 교체 했다고 알려줌.. 능력치도 재계산 해야 함
	if (!bIsSuccess)
	{
	}
	return TRUE;
}

// 필수 : LineItem은 List내에서 오름차순으로 정렬되어 있어야 할듯...
void CRoomInternalLogic::CalcLineItemCountByInitFishing(CUser* pUser, LONG lFighingResultType, TlstInvenSlot& remainCountchangedInven, TMapAchvFactor& mapFactorVal)
{
	CNFChar& nfChar					= pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt	= nfChar.GetNFChar().m_nfCharInfoExt;
	LONG lErrorCode					= NF::G_NF_ERR_SUCCESS;

	// Line_Item
	LONG lLineLength = 0;
	if (EC_FR_LINEBREAK == lFighingResultType)
		lLineLength = (LONG)(nfChar.GetLineLength() * 0.2);
	else if (EC_FR_CUT_LINE_BY_USER_ACTION == lFighingResultType)
		lLineLength = (LONG)(nfChar.GetLineLength() * 0.9);
	else
	{	
		// FIGHTING_RESULT::EC_FR_LANDING
		long lRate = urandom(50);
		if (lRate > 50) lLineLength = 1;
	}

	// test
	//lLineLength = 15;

	if (lLineLength > 0)
	{
		TMapInven::iterator itLine1 = nfCharInfoExt.m_nfCharInven.m_mapUsingItem.find(eItemType_Line);
		if (itLine1 == nfCharInfoExt.m_nfCharInven.m_mapUsingItem.end())
			return;

		LONG lItemCode = (*itLine1).second.m_lItemCode;
		TMapInvenSlotList::iterator itLine2 = nfCharInfoExt.m_nfCharInven.m_mapCountableItem.find(lItemCode);
		if (itLine2 == nfCharInfoExt.m_nfCharInven.m_mapCountableItem.end())
			return;

		LONG lRemainLength = lLineLength;									// ex) 70m
		ForEachElmt(TlstInvenSlot, (*itLine2).second, itLine3, ijLine3)		// ex) first_inven:20m, second_inven:50m
		{
			if (theNFDataItemMgr.CheckItemTypeByItemCode("E", (*itLine3).m_lItemCode, (*itLine3).m_lPartsIndex))		// 영구제
				return;

			LONG lDBRemainCount = 0;
			LONG lReduceGauge = lRemainLength;
			if (lReduceGauge <= 0)
				break;

			// 이후에 시스템에서 자동으로 삭제해야 할 루틴으로 넘겨야 하기 때문에, 서버 메모리상으로만 0으로 남겨둔다.(DB는 어짜피 치울꺼기 때문에 업데이트 안 함)
			if ((*itLine3).m_lRemainCount < lRemainLength)
				lReduceGauge = (*itLine3).m_lRemainCount;

			lDBRemainCount = lReduceGauge;

			// 델타값으로 DB호출
			if (!theNFDBMgr.UseGameFunc(pUser->GetGSN(), pUser->GetCSN(), (*itLine3), "Y", lDBRemainCount, lErrorCode))
			{
			}
			else 
			{
				lRemainLength -= lReduceGauge;
				(*itLine3).m_lRemainCount = lDBRemainCount;
			}

			theNFMenu.SetExistCountableItemToInven(nfCharInfoExt.m_nfCharInven, (*itLine3));
			if ((*itLine3).m_lRemainCount > 0)
				remainCountchangedInven.push_back((*itLine3));
		}

		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_ITEM], eItemType_Line));
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_CONSUME_VAL], lLineLength));
	}
}

void CRoomInternalLogic::CalcEnduranceItemCountByInitFishing(CUser* pUser, TlstInvenSlot& remainCountchangedInven, LONG lReduceCount, int nType, TMapAchvFactor& mapFactorVal)
{
	CNFChar& nfChar					= pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt	= nfChar.GetNFChar().m_nfCharInfoExt;
	LONG lErrorCode					= NF::G_NF_ERR_SUCCESS;

	if (lReduceCount > 0)
	{
		TMapInven::iterator itItem = nfCharInfoExt.m_nfCharInven.m_mapUsingItem.find(nType);
		if (itItem == nfCharInfoExt.m_nfCharInven.m_mapUsingItem.end())
			return;

		NFInvenSlot& inven = (*itItem).second;
		if (theNFDataItemMgr.CheckItemTypeByItemCode("E", inven.m_lItemCode, inven.m_lPartsIndex))		// 영구제
			return;		// 영구제 아이템

		LONG lReduceGauge = inven.m_lRemainCount;
		if (inven.m_lRemainCount >= lReduceCount)
			lReduceGauge = lReduceCount;
			
		// 델타값으로 DB호출
		if (!theNFDBMgr.UseGameFunc(pUser->GetGSN(), pUser->GetCSN(), inven, "N", lReduceGauge, lErrorCode))
		{
		}
		inven.m_lRemainCount = lReduceGauge;
		theNFMenu.SetExistCountableItemToInven(nfCharInfoExt.m_nfCharInven, inven);

		if (inven.m_lRemainCount > 0)
			remainCountchangedInven.push_back(inven);

		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_ITEM], nType));
		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_CONSUME_VAL], lReduceCount));
	}
}

// 필수 : LineItem은 List내에서 오름차순으로 정렬되어 있어야 할듯...
void CRoomInternalLogic::CalcItemCountByInitFishing(CUser* pUser, LONG lFighingResultType, TlstInvenSlot& remainCountchangedInven)
{
	if (NULL == pUser) return;

	CNFChar& nfChar					= pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt	= nfChar.GetNFChar().m_nfCharInfoExt;
	LONG lErrorCode					= NF::G_NF_ERR_SUCCESS;

	TMapAchvFactor	mapFactorVal;

	// Rod, Reel
	CalcEnduranceItemCountByInitFishing(pUser, remainCountchangedInven, nfChar.ResultEnduranceRodItem(), eItemType_Rod, mapFactorVal);
	CalcEnduranceItemCountByInitFishing(pUser, remainCountchangedInven, nfChar.ResultEnduranceReelItem(), eItemType_Reel, mapFactorVal);

	// Line
	CalcLineItemCountByInitFishing(pUser, lFighingResultType, remainCountchangedInven, mapFactorVal);

	// Lure_Item
	if (EC_FR_LINEBREAK == lFighingResultType)
	{
		TMapInven::iterator itLure1 = nfCharInfoExt.m_nfCharInven.m_mapUsingItem.find(eItemType_Lure);
		if (itLure1 == nfCharInfoExt.m_nfCharInven.m_mapUsingItem.end())
			return;

		LONG lItemCode = (*itLure1).second.m_lItemCode;
		TMapInvenSlotList::iterator itLure2 = nfCharInfoExt.m_nfCharInven.m_mapCountableItem.find(lItemCode);
		if (itLure2 == nfCharInfoExt.m_nfCharInven.m_mapCountableItem.end())
			return;

		NFInvenSlot inven;
		inven.Clear();
		inven.m_lItemCode = lItemCode;

		LONG lErr = theNFMenu.FindReduceInvenSlot(nfCharInfoExt.m_nfCharInven.m_mapCountableItem, inven, lItemCode);
		if (NF::G_NF_ERR_SUCCESS == lErr)
		{
			if (theNFDataItemMgr.CheckItemTypeByItemCode("E", inven.m_lItemCode, inven.m_lPartsIndex))		// 영구제
				return;

			LONG lReduceGauge = 1;
			inven.m_lRemainCount -= lReduceGauge;
			if (inven.m_lRemainCount < 0)
				lReduceGauge = 0;
			else
			{
				// 델타값으로 DB호출
				if (!theNFDBMgr.UseGameFunc(pUser->GetGSN(), pUser->GetCSN(), inven, "Y", lReduceGauge, lErrorCode))
				{
				}
			}
			inven.m_lRemainCount = lReduceGauge;
			theNFMenu.SetExistCountableItemToInven(nfCharInfoExt.m_nfCharInven, inven);
			if (inven.m_lRemainCount > 0)
				remainCountchangedInven.push_back(inven);
		}
		else
			return;

		mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_ITEM], eItemType_Lure));
	}
	g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_CONSUME, GetRoomID(), mapFactorVal);
}

BOOL CRoomInternalLogic::GetNFLetterList(const LONG lCSN, CONT_NF_LETTER& rkContNFLetter, const BOOL bNewLetter)
{
	if( FALSE == theNFDBMgr.SelectNFLetterList( lCSN, rkContNFLetter, bNewLetter ) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CRoomInternalLogic::GetNFLetterContent(const __int64 i64LetterIndex, string& rstrContent, string& rstrSendTime )
{
	if( FALSE == theNFDBMgr.SelectNFLetterContent( i64LetterIndex, rstrContent, rstrSendTime ) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CRoomInternalLogic::SendNFLetter(const string& rstrReceiver, const CNFLetter& rnfLetter)
{
	if( FALSE == theNFDBMgr.InsertNFLetter( rstrReceiver, rnfLetter ) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CRoomInternalLogic::DeleteNFLetter(const vector<__int64>& kContLetterIndex)
{
	if( FALSE == theNFDBMgr.DeleteNFLetter( kContLetterIndex ) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CRoomInternalLogic::GetNFFriendInfo(const LONG lCSN, CONT_NF_FRIEND& rkContNFFriend)
{
	if( FALSE == theNFDBMgr.SelectNFFriendInfo( lCSN, rkContNFFriend, FR_FRIEND ) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CRoomInternalLogic::GetNFCharKeyByCharName(const string& rstrCharName, TKey& rKey)
{
	if( FALSE == theNFDBMgr.SelectNFCharKeyByCharName(rstrCharName, rKey) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CRoomInternalLogic::NFFriendAccept( const string& rstrAcceptorCharName, const string& rstrApplicantCharName )
{
	// 친구 상태로 업데이트
	if( FALSE == theNFDBMgr.UpdateNFFriendStatusToFriend( rstrAcceptorCharName, rstrApplicantCharName ) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CRoomInternalLogic::DeleteNFFriend( const string& rstrCharName, const string& rstrDeleteCharName )
{
	if( FALSE == theNFDBMgr.DeleteNFFriend( rstrCharName, rstrDeleteCharName ) )
	{
		return FALSE;
	}

	return TRUE;
}
