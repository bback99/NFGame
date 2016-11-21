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
	// ###################### NFUser ���� �о���� �κ� ######################
	NFUserBaseInfo nfUBI = pUser->GetUserData();
	pUser->GetNFUser().SetCUser(pUser);

	//////////////////////////////////////////////////////////////////////////
	// 1. CSN�� �ش��ϴ� NFChar���� �о����...
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
	//// 2. CSN�� �ش��ϴ� Inven���� �о����
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

	// ������ ����� ����
	if (!theNFMenu.GetAquaFish(pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfAquaFish))
	{
		pUser->SetErrorCode(EC_NA_NOT_FOUND_AQUA_FISH);
		theLog.Put(ERR, "theNFMenu.GetAquaFish is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	// WORKING(acepm83@neowiz.com) �ɷ�ġ�� �����ϱ����� ������ ������ �о�� �Ѵ�.(Beacause ������ ����)

	LONG lElapsedClearHour = 0; // û�� ����ð�
	LONG lElapsedFeedHour = 0;	// ���� ����ð�

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

	// ȯ��Ӽ� ������� üũ�Ѵ�. ���⼭ �ϸ� �� �ǰ�, InGame�� ���� �������ش�.
	pUser->SetEnvDebuff(theNFMenu.CheckEnvDebuff(pUser->GetNFCharInfoExt(), m_nfRoomOption.m_lEnvAttribute));

	////////////////////////////////////////////////////////////////////////
	// 4-1. Aquarium
	
	
	// 4-2. LockedNote(+Main) ���� �б�
	if (!theNFDBMgr.SelectNFLockedNote(pUser->GetGSN(), pUser->GetCSN(), pUser->GetLockedNote()))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "SelectNFLockedNote is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	// 5-1. ���� ���� �о����
	if (!theNFDBMgr.SelectNFAchvList(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfCharAchievement.m_nfCharAchieve, lErrorCode))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "SelectNFAchvList is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	// 5-2. ���� ����Ʈ ���� �о����
	if (!theNFDBMgr.SelectNFAchvPoint(pUser->GetGSN(), pUser->GetCSN(), pUser->GetNFCharInfoExt()->m_nfCharAchievement.m_nfCharAP))
	{
		pUser->SetErrorCode(CRF_DBERROR);
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "SelectNFAchvPoint is Fail!!!, Char USN: ", pUser->GetUSN(), ", CSN : ", pUser->GetCSN());
	}

	return TRUE;
}

BOOL CRoomInternalLogic::AddUserSlot(CUser* pUser)
{
	// NF���ӿ��� �ʿ�� �ϴ� UserSlot �߰�
	LONG lIndex = m_UserSlot.AddSlot(pUser->GetCSN());
	if (lIndex == -1) return FALSE;
	pUser->SetUserSlot(lIndex);
	
	if (lIndex > -1)
		return TRUE;
	return FALSE;
}

LONG CRoomInternalLogic::ChangeUserSlot(CUser* pUser, LONG lMoveSlot)
{
	// NF���ӿ��� �ʿ�� �ϴ� UserSlot �߰�
	LONG lIndex = m_UserSlot.ChangeSlot(pUser->GetUserSlot(), lMoveSlot, pUser->GetCSN());
	if (lIndex == -1) return -1;
	pUser->SetUserSlot(lIndex);
	return lIndex;
}

LONG CRoomInternalLogic::RemoveUserSlot(CUser* pUser)
{
	// NF���ӿ��� �ʿ�� �ϴ� UserSlot �߰�
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
	// Ʃ�丮��� ���� ��� ó���� Level_UP�� ��û�Ѵ�.
	ArcMapT< std::string,LONG > mapFactorVal;
	mapFactorVal.insert(make_pair(achv::ACHV_FACTOR[achv::AF_LEVEL], pUser->GetLevel()));
	g_achv.CheckAchv(pUser->GetGSN(), pUser->GetCSN(), achv::AE_LEVEL, GetRoomID(), mapFactorVal);

	LONG lErrorCode = NF::G_NF_ERR_SUCCESS;

	// ��� �ִ� ����Ʈ�� ������ ã�ƺ���.
	ForEachElmt(TMapTotalMapInfo, m_mapTotalMapInfo, it, ij)
	{
		if ((*it).second.m_lstFPUserList.size() <= 0)
		lAnsFishingPointIndex = (*it).first;
		break;
	}

	// ��� ����Ʈ�� �Ѹ� ��������, �ƹ��ų� �˻� ���� �Ǵ� ������ ã�´�. 
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


// Boat ���� �߰� : 2010.02.08
// Sequence 
// 1. PointType�� Walk �̸�, MaxUsers �� üũ�Ѵ�.
// 2. PointType�� Boat �̸�, 
//		2-1. ó�� ������, Boat Owner�̳�? -> ����
//		2-2. ó�� ��������, Boat Owner�� �ƴϴ�. -> ����
//		2-3. �ι�° ���� ���ʹ� Boat�� Owner�� ��� ���� MaxUser�� üũ���Ѵ�.
// 3. bOnlyRemove�� TRUE�� ���� lFishingPoint�� remove�ϴ� FishingPoint�̴�.
// 4. PointType�� MultiPort �̸�, FishingPoint�� m_vecMultiPort�� ����
LONG CRoomInternalLogic::ChangeFishingPoint(const FishingPoint* pFP, LONG lFishingPoint, CUser* pUser, LONG& lBoatOwnerCSN, LONG& lMovedMultiPortIndex, BOOL bOnlyRemove)
{
	LONG lErrorCode = NF::G_NF_ERR_SUCCESS;

	CNFChar& nfChar = pUser->GetNFUser(); 
	FPInfo addNFFPInfo;
	addNFFPInfo.Clear();

	if (!bOnlyRemove) {
		// �ű���� ����Ʈ�� ������...
		if (pUser->GetPrevFishingPoint() == lFishingPoint)
			return NF::G_NF_ERR_MOVE_FP_SAME;

		// Check FishingPoint Valid
		TMapTotalMapInfo::iterator iterAdd = m_mapTotalMapInfo.find(lFishingPoint);
		if (iterAdd == m_mapTotalMapInfo.end())
			return NF::G_NF_ERR_MOVE_FP_NOT_FOUND;	// TotalMap�� �߰��� �� �Ǿ� �ִ� FishingPoint�̴�.

		addNFFPInfo = (*iterAdd).second;
		if (addNFFPInfo.m_lFPStatus == 0)		// �̵� �Ұ� �����̸�, ����....
			return NF::G_NF_ERR_MOVE_FP_IMPOSSIBLE;		// FishingPoint�� Ǯ�̴�.
	}

	bool isBoatOwnerLeave = false;
	LONG lPrevFishingPoint = pUser->GetPrevFishingPoint();

	// Remove , ������ ���� �����
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
				if (nfFPInfo.m_lFPType == 0)		// walk �̸�, 
					nfFPInfo.m_lFPStatus = 1;		
				else if (nfFPInfo.m_lFPType == 1)	// boat �̸�, 
				{
					// ��Ʈ ���ʰ� ������ ���...	�ʱ�ȭ, Todo : �ش� ��Ʈ�� Ÿ�� �ִ� ��� �������� �������� �Ѵ�.....
					if (nfFPInfo.m_lBoatOwnerCSN == pUser->GetCSN()) {
						nfFPInfo.m_lBoatOwnerCSN = 0;
						nfFPInfo.m_lBoatMaxUsers = 0;
						nfFPInfo.m_lFPStatus = 1;

						isBoatOwnerLeave = true;
					}
					else
						nfFPInfo.m_lFPStatus = 2;
				}
				else if (nfFPInfo.m_lFPType == 2)	// multi-port �̸�, 
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
		// �߰��Ϸ��� UserList���� FP�� ����� ��ġ�� ������ ����
		// ��Ʈ�� �����ο� ���� 2010.2.11
		// FishingPoint�� MaxUsers���� Boat�� MaxUser�� Ŭ �� ����.
		// Max���� üũ�ϱ�����, ���� ����Ʈ���� üũ�ϰ� ���� ����Ʈ�� d��쿡 ��Ʈ ������ ��Ʈ�������� NFFPInfo�� MaxUser���� NF�����Ѵ�.
		FPUserInfo		newFPUI;
		newFPUI.m_lCSN = pUser->GetCSN();

		LONG lMaxUsers = pFP->m_lMaxUsers;
		// ��Ʈ����Ʈ üũ
		if (addNFFPInfo.m_lFPType == 1)		// Boat
		{
			if (addNFFPInfo.m_lstFPUserList.size() <= 0)		// ó�����´�.
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
			addNFFPInfo.m_lFPStatus = 2;	// ��Ʈ�� �־ �̵� �����ϴ�.
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

		// map�� �߰�
		addNFFPInfo.m_lstFPUserList.push_back(newFPUI);

		// Full���� üũ (Boat�� ���� boat�� ������ �񱳸�..)
		if (lMaxUsers <= (LONG)addNFFPInfo.m_lstFPUserList.size())
			addNFFPInfo.m_lFPStatus = 0;	// ��Ʈ��, Walk�� Max�� �Ǹ� �̵� �Ұ�!!!
		
		m_mapTotalMapInfo[lFishingPoint] = addNFFPInfo;
		lBoatOwnerCSN = addNFFPInfo.m_lBoatOwnerCSN;
		lErrorCode = NF::G_NF_ERR_SUCCESS;

		// ����
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
	TMapTotalMapInfo::iterator prevItor = m_mapTotalMapInfo.find(lPrevFishingPoint); // ��Ʈ������ ���������� �ִ� �ǽ�����Ʈ
	if (prevItor != m_mapTotalMapInfo.end())
	{
		FPInfo& nfPrevFPInfo = (*prevItor).second;

		ForEachElmt(TlstFPUserList, nfPrevFPInfo.m_lstFPUserList, it, ij) // ��Ʈ�� Ÿ���ִ� ������
		{
			TMapTotalMapInfo::iterator itor = m_mapTotalMapInfo.begin(); // �ο��� ������ ���� �ǽ�����Ʈ�� ã�´�
			while (m_mapTotalMapInfo.end() != itor)
			{
				FPInfo& nfFPInfo = (*itor).second;
				if (nfFPInfo.m_lFPStatus != 0) // �� �� �ִ� �ǽ�����Ʈ
				{
					if (nfFPInfo.m_lFPStatus == 1) // ��Ʈ?
					{
						CUser* pUser = UserManager->FindUser((*it).m_lCSN);
						if (pUser)
						{
							TMapInven::iterator findItor = pUser->GetNFCharInfoExt()->m_nfCharInven.m_mapUsingItem.find(eItemType_Boat);
							if (findItor == pUser->GetNFCharInfoExt()->m_nfCharInven.m_mapUsingItem.end())
							{
								++itor;
								continue; // ��Ʈ�� ������ ��������Ʈ�� ������.
							}
						}
					}

					// ������ �ǽ�����Ʈ ���� ��Ŷ ������ ó�� ó��
					MsgCliNGS_ReqMoveFishingPoint req;
					req.m_lCSN = (*it).m_lCSN;
					req.m_lFPIndex = nfFPInfo.m_lFishingPoint;
					req.m_bIsBoatOwnerLeave = TRUE;
					PayloadCliNGS pld(PayloadCliNGS::msgReqMoveFishingPoint_Tag, req);
					OnReqMoveFishingPoint((*it).m_lCSN, &pld);

					return; // ������������!
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

// �ʷε� ���� ������ �����鿡�� ������ �Լ�
void CRoomInternalLogic::SendNtfGameReadyProgress(CUser* pUser)
{
	MsgNGSCli_NtfGameReadyProgress		ntf;

	ntf.m_lstMapLoadingProgress.push_back(pUser->GetNFUser().GetMapLodingProgress());

	PayloadNGSCli pld(PayloadNGSCli::msgNtfGameReadyProgress_Tag, ntf);
	UserManager->SendToAllUser(pld);
}

// �ʷε��� �����϶�� �����鿡�� ������ �Լ�
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

	// ���� ������ ��� �޼����� �����Ѵ�.
	MsgNGSCli_NotifyGameResult		notify;
	if (!GetGameResult(NULL, notify.m_lstPlayer, notify.m_lstTeam, FALSE, TRUE))		// UpdateDB
		theLog.Put(WAR_UK, "NGS_LOGIC, GameOver(). GetGameResult Failed. RoomID : ", RoomID2Str(GetRoomID()));

	PayloadNGSCli pld(PayloadNGSCli::msgNotifyGameResult_Tag, notify);
	UserManager->SendToAllUser(pld);

	// ���� ���õ� ������ �ʱ�ȭ�Ѵ�.
	InitTeamPlayData();

	MsgNGSCli_NotifyChangeUserInfo	ntf;
	ntf.m_lErrorCode = NF::G_NF_ERR_SUCCESS;
	ntf.m_lCSN = -1;
	ntf.m_lInfoType = 2;
	ntf.m_lMoveUserSlot = 0;
	ntf.m_lUserStatus = UIS_INVALID;

	// Room-Lobby�� �ִ� �������� ���¸� ������ �����Ѵ�.
	PayloadNGSCli pld2(PayloadNGSCli::msgNotifyChangeUserInfo_Tag, ntf);
	UserManager->SendToAllUser(pld2);
}


// ��ȸ�� ������ �� Room�� Room���� �������� �ʱ� ������ �ʱ�ȭ �ϴ� �Լ�
void CRoomInternalLogic::InitTeamPlayData()
{
	// ¡���� �ʱ�ȭ
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
// ĳ���� ��Ÿ�(�Ҽ��� ���ڸ�)
double CRoomInternalLogic::CalcCastingDist(long lCastingType, double dUIFlyDist)
{
	if (dUIFlyDist < AI_CASTING_FLY_DIST_MIN)
		dUIFlyDist = AI_CASTING_FLY_DIST_MIN;	// �ּҰ� ����

	if (dUIFlyDist > AI_CASTING_FLY_DIST_MAX)
		dUIFlyDist = AI_CASTING_FLY_DIST_MAX;	// �ִ밪 ����

	// �巡�� ĳ���� ��Ÿ�(�Ҽ��� ���ڸ�)
	// (SQRT(��Ÿ�)*2.4) + 8
	double dDragCastingTemp = sqrt(dUIFlyDist) * AI_DRAG_CASTING_RATE + AI_DRAG_CASTING_ADD_DIST;
	double dCastingDist = ROUND(dDragCastingTemp, 1);

	if (2 == lCastingType)		// ClickCasting�̸�
	{
		// Ŭ�� ĳ���� ��Ÿ�(�Ҽ��� ���ڸ�)
		// 2011.09.26 ����: // �巡�� ĳ���� �� * 0.8
		double dClickCastingTemp = dCastingDist * AI_CLICK_CASTING_RATE;
		dCastingDist = ROUND(dClickCastingTemp, 1);
	}
	return dCastingDist;
}

// ĳ���� ������ ���� (�Ҽ��� ���ڸ�)
double CRoomInternalLogic::CalcAccuracy(long lCastingType, double dUIFlyDist)
{
	if (dUIFlyDist < AI_CASTING_FLY_DIST_MIN)
		dUIFlyDist = AI_CASTING_FLY_DIST_MIN;	// �ּҰ� ����

	if (dUIFlyDist > AI_CASTING_FLY_DIST_MAX)
		dUIFlyDist = AI_CASTING_FLY_DIST_MAX;	// �ִ밪 ����

	// �巡�� ĳ���� ������ ���� (�Ҽ��� ���ڸ�, m)
	double dDragAccuracyTemp = (double)((AI_CASTING_ACC-(sqrt(dUIFlyDist)*AI_DRAG_CASTING_ACC_RATE)+AI_DRAG_CASTING_ACC_ADD))/100;
	double dCastAccuracy = (double)(ROUND(dDragAccuracyTemp, 2));

	if (2 == lCastingType)		// ClickCasting�̸�
	{
		// Ŭ�� ĳ���� ������ ���� (�Ҽ��� ���ڸ�)
		double dClickAccuracyTemp = dCastAccuracy * AI_CLICK_CASTING_ACC_RATE;
		dCastAccuracy = ROUND(dClickAccuracyTemp, 2);
	}
	return dCastAccuracy;
}

//// ĳ���� �鷯�� �� �ٲ��� Ȯ��
//double CRoomInternalLogic::CalcBacklash(long lCastingType, double dUIBacklash, double dDragIntensity, double dDragIntensityMax, double dCastingDistMax, double dRealCastingDist, LONG lLevel)
//{
//	// �巡�� ĳ���� �鷯�� �� �ٲ��� Ȯ��
//	// 2011.09.26 ����
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
//	if (2 == lCastingType)		// ClickCasting�̸�
//	{
//		// Ŭ�� ĳ���� �鷯�� �� �ٲ��� Ȯ��
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
//	// ������ ���� ����
//	if( 1 <= lLevel && lLevel <= 10 ) // ����
//	{
//		dBachlashResult -= 20;
//	}
//	else if( 11 <= lLevel && lLevel <= 20 ) // ��Ű
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

	//// �� ������ ����
	//double dPointDist = sqrt(dGapX*dGapX + dGapY*dGapY);

	//double dDragIntensity = (double)lDragIntensity / 300; 
	//if (dDragIntensity >= 1.0)
	//	dDragIntensity = 1.0;

	//double dRealDist = 0.0;
	//if (1 == lCastingType)			// �巡��...
	//	// ���� �� �ִ� �Ÿ����� ���� ������ ���Ѵ�.
	//	dRealDist = dMaxFlyDist * dDragIntensity;
	//else							// Ŭ��
	//	// �� ������ ����
	//	dRealDist = dPointDist;

	//// ���� �Ÿ��� ���� �Ÿ��� ������ ���Ѵ�.
	//ResultCastingLocation.m_dX = UserLocation.m_dX + dGapX/dPointDist * dRealDist;
	//ResultCastingLocation.m_dY = UserLocation.m_dY + dGapY/dPointDist * dRealDist;


	
	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);

	// y <= sqrt( r^2 - x^2 )
	// rand() % y
	int nCastAccuracyCm = (int)(dCastAccuracy * 100);			// m���� �̱� ������, �Ҽ����� �����Ƿ� cm ������ �ٲ㼭 ���...
	if (nCastAccuracyCm <= 0)
		nCastAccuracyCm = 1;
	double dRandX = (double)(urandom(nCastAccuracyCm));		
	int nRandY = (int) sqrt(double((nCastAccuracyCm*nCastAccuracyCm) - (dRandX*dRandX)));
	if (nRandY <= 0)
		nRandY = 1;
	double dRandY = (double)(urandom(nRandY));

	// Ŭ���̾�Ʈ ��ǥ�� 10cm���� ����� �Ѵ�.
	double dRandResultX = ROUND((double)dRandX, 1);
	double dRandResultY = ROUND((double)dRandY, 1);

	// �����ϰ� ���� ������ x, y�� ��ġ�� �ٽ� �����ϰ� ���Ѵ�. 0�̸� -, 1�̸� +
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

// CastingScore�� InCounterScore �������� �Լ�
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

// Lucky Point ���
double CRoomInternalLogic::CalcLuckyPoint(double dLuckPoint)
{
	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);

	long lLuckyPoint = (long) ( ( dLuckPoint * 0.3 ) + 5 ) * 10;		// �Ҽ��� ���ڸ��� ���ֱ� ���ؼ�
	long lRandLucky = urandom(1000);		// �Ҽ��� ���ڸ��� ���� �����Ƿ�, 100% Ȯ���� �ƴ϶� 1000% Ȯ���� ���
	if (lLuckyPoint > lRandLucky)		// Ȯ�� ���� �ȿ� ���� ���̹Ƿ� Ȯ�� ����
		dLuckPoint = 1.2 + (urandom(9)) * 0.1;
	return dLuckPoint;
}

// ��ǥto ���� ��ǥfrom������ �Ÿ� ���ϱ�
// ������ġ, ĳ������ġ ��ǥ�� ū�ʿ��� ���� ���� �� ���� x, y ���� �غ�, ���̷� �Ͽ� ���� �ﰢ���� ������ ���̰� �������� �Ÿ��̴�.
// �غ�, ����
double CRoomInternalLogic::CalcHypote(Coordinates& to, Coordinates& from)
{
	return sqrt( (to.m_dX - from.m_dX)*(to.m_dX - from.m_dX) + (to.m_dY - from.m_dY)*(to.m_dY - from.m_dY) + (to.m_dZ - from.m_dZ)*(to.m_dZ - from.m_dZ) );
}

// ĳ���� ���ھ� ���ϱ�
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
			bRet = nfChar.IsIncounterGaugeFull();			// ��ī���� �������� Ǯ���� üũ
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

// 1. Fish AI Timer�� 1�� �������� �̺�Ʈ �߻��� ȣ���ϴ� �Լ�
// Ÿ�̸� �߻���, FishAI Queue�� ��ϵǾ� �ִ� User���� ���ʴ�� �ҷ� ans�޼����� �����Ѵ�.
// 2. ��ȸ ���� �ð��� ǥ�����ִ� Ÿ�̸�, ��ȸ ���߿� ������ ����鿡�� ���� �ð��� �˷��ֱ� ���ؼ� �ʿ�
// 3. �ʷε���, üũ �Ͽ� ���� �Ⱓ ������ ó�� ���� ���� ���� �ɷ�����

void CRoomInternalLogic::OnTimerGNFGame()
{
	// ���� ��, ���� �ð� üũ
	if (ROOMSTATE_START == GetRunStopFlag())
	{
		// �����ִ� �ð��� ������ ���ӽð����� Ŀ���� ���ӿ���!!! (������ �ð��� ������ ��)
		if (--m_nfRoomOption.m_lRemainTime <= 0)
		{
			m_nfRoomOption.m_lRemainTime = 0;
			GameOver();
		}
	}
	// MapLoading
	else if (ROOMSTATE_READY == GetRunStopFlag())
	{
		if (IncrementMapLoadingTime() >= 3*60)		// 3��
		{
			UserManager->IsCheckMapLoading(TRUE);

			// ����...
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

	// �Ⱓ�� ������ üũ
	ntf.m_lErrorCode = theNFItem.Check_TotalItemValid(pUser->GetGSN(), pUser->GetNFCharInfoExt()->m_nfCharInven, ntf.m_lstRemovedInven);
	if (NF::G_NF_ERR_SUCCESS == ntf.m_lErrorCode)
		// ������ ��ȿ�� üũ�� �Ѵ�...
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

	// ItemManger���� ������ ���� �Ƿε��� �����´�.

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

	// 2010-12-13 ����
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
// 2010-12-13 �ּ�
	// ��ų�� �켱���� üũ
	// ���� ��ų Ÿ��, ���� ����� ������ 
//	if (nfChar.GetCurrentFishSkillType() == 4)
//	{
//		if (nfChar.IsSameDirection())
//			SetLineLoose(NFUser, dLineLoadPresent, lCurDragLevel, RcvMsg);
//	}
//	// ��Ƽ�� ��ų Ÿ��
//	else if (nfChar.GetCurrentFishSkillType() == 3)
//		SetLineLoose(NFUser, dLineLoadPresent, lCurDragLevel, RcvMsg);
//	// Drag�� ���� ��Ǯ�� ���
//	else 
	if ((dLineLoadPresent > dRealLoadMax * dRateDragLevel) && (nfChar.GetCurFishHP() > 0))
		SetLineLoose(nfChar, dLineLoadPresent, lCurDragLevel, RcvMsg);
}

//
void CRoomInternalLogic::SetFishPatternByLineLength(CFish& BiteFish, CNFChar& nfChar, LONG& lFishHP)
{
	// ����� ���� ü��% ���ؿ���
	double dRemainHP = (double)BiteFish.GetCurFishHP() / (double)BiteFish.GetMAXFishHP();
	LONG lRemainRateFishHP = (LONG)(dRemainHP * 100);

	FishInfo& fishInfo = BiteFish.GetFish();

	if (nfChar.GetLineLength() >= 10)		// Ǯ�� �������� ���̰� 10m �̻��� ����� ������� HP�� �̵� ���Ͽ� ���� �κ��̴�.
	{
		if (lRemainRateFishHP >= 50)
		{
			// �� 100% ~ 50 %
			// ������� ü���� ������ ����. DB�� Attackrate, Restrate�� 100%�� �ݿ��Ѵ�.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_50100, 0, 0);
		}
		else if (lRemainRateFishHP <= 20 && lRemainRateFishHP > 50)
		{	
			// �� 49% ~ 20%
			// ������� ü���� ���� ��Ű�� �����ϴ� ����. DB�� Attackrate �ð��� 100% �ݿ��ϰ� Restrate �ð��� 1������ �þ�� �ȴ�. �׸��� ������� ���ݷ��� 90% ���� �ݿ��ȴ�.
			lFishHP = (LONG)(lFishHP * 0.9);
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_2050, 0, 1);
		}
		else if (lRemainRateFishHP <= 1 && lRemainRateFishHP > 20)
		{
			// �� 19%~ 1%
			// ������� ü���� �ٴڳ��� ������ �����̴�. DB�� Attackrate �ð����� 1������ �پ��� Restrate �ð��� 1������ �þ�� �ȴ�. �׸��� ������� ���ݷ��� 70% ���� �ݿ��ȴ�.
			lFishHP = (LONG)(lFishHP * 0.7);

			// ���� �ߵ��ǰ� �ִ� ��ų�� Jump �̸�, lAttackRate���� �������� �ʴ´�. 2010.2.11
			LONG lAttackRate = fishInfo.m_lAttackRate;
			if (BiteFish.GetFishSkillType() != NF::G_NF_ERR_SUCCESS)
				lAttackRate = lAttackRate-1;
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_0120, lAttackRate, fishInfo.m_lRestRate+1);
		}
		else
		{
			// �� 0%
			// ������� ü���� �ٴڳ� ���´�. ������ �ѹ������� �������� �ȴ�.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_0000, 0, 0);
			BiteFish.SetPrevFishDirection(nfChar.GetPRNG()%2 + 3);
		}
	}
	else		// Ǯ�� ������ <= 10m
	{
		// Ǯ�� �������� 10m �̳���� ���� ����Ⱑ ĳ������ �ٷ� �ձ��� �Դٴ� ����.
		// NF���� ����� ���� ������ ������� HP�� 0�̰� �������� ����(Length)�� 4m �̳���� �� ������ �����ϴ� ��쿡 �߻��ϰ� �ȴ�.
		// ������ �����߿� ����� �Ÿ����� ����⸦ ��ŷ�ϰų� ���� ��ų ������ ������� ü���� ���� ���¿��� 10m�̳��� �����ϰ� �Ǵ� ��찡 �߻��� ���̴�. �̷��� ������ ������ ����ǵ��� �Ѵ�.
		if (lRemainRateFishHP >= 50)
		{
			// 1-3-1-1. HP > 50% and Ǯ�� �������� �Ÿ�(Length) <= 10m
			// �� ������� ���ݷ� ����
			// ������� ���ݷ��� 20% ���� �����Ѵ�. ������� ���ݷ��� �������� �巡�� �ܰ�� ���� ����Ⱑ ���� Ǯ�� ������ �����Ѵ�.
			lFishHP = (LONG)(lFishHP * 1.2);
			// �� ����� RestRate = 1
			// ����Ⱑ ���� ���� �ʰ� ĳ���Ϳ��� �������� �ð��� 1�ʷ� �����. �̰͵� ���� ���� �Բ� ����Ⱑ ���� Ǯ�� ��� �������� �����ϴ� ����̴�.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_50100, 0, 1);
		}
		else if (lRemainRateFishHP <= 20 && lRemainRateFishHP > 50)
		{
			// 1-3-1-2. 50% > HP >= 20% and Ǯ�� �������� �Ÿ�(Length) <= 10m ��
			// �� ������� ���ݷ� ����
			// ������� ���ݷ��� 10% ���� �����Ѵ�. 
			lFishHP = (LONG)(lFishHP * 1.1);
			// �� ����� RestRate = 1
			// ����Ⱑ ���� ���� �ʰ� ĳ���Ϳ��� �������� �ð��� 1������ �����. �̰͵� ���� ���� �Բ� ����Ⱑ ���� Ǯ�� ��� �������� �����ϴ� ����̴�.
			nfChar.ChangeFishPatternByFishHP(EN_FISHHP_2050, 0, 1);
		}
		else
		{
			// 1-3-1-3. HP < 20% and Ǯ�� �������� �Ÿ�(Length) <= 10m ��
			// �� ����� RestRate = 1
			// ����Ⱑ 100%�� ���� ��������� ĳ���Ϳ��� �������� �ð��� 1�ʷ� ���鵵�� �Ѵ�.
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
	NFCharInfoExt& nfCharInfoExt= nfChar.GetNFChar().m_nfCharInfoExt;		// ������ ������ �����Ϸ��� �� ������...
	NFAbilityExt& abilityExt	= nfChar.GetAbilityExt();
	CFish&		biteFish		= nfChar.GetBiteFish();
	FishInfo&	biteFishInfo	= biteFish.GetFish();

	// NFUser(�⺻ �ɷ�ġ + EquipItem + ClothItem) + NFUserItem(UsableItem + SkillItem)
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
	// Lure ������ ��������
	pLureItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
	if (!pLureItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
		return NF::EC_FR_LURE_NOT_FOUND;
	}

	// Line ������ ��������
	pLineItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode);
	if (!pLineItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode);
		return NF::EC_FR_LINE_NOT_FOUND;
	}

	// Reel ������ ��������
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
	// Char�� �� : MAX, DragLevel�� ���� ��
	double dMAXCharacterPower = nfChar.GetCharacterMaxPower();
	if (dMAXCharacterPower <= 0)
	{
		dMAXCharacterPower = ROUND( (sqrt(nfCharInfoExt.m_nfAbility.m_dStrength)*CHAR_POWER_ROD) + (sqrt(nfCharInfoExt.m_nfAbility.m_dAgility)*CHAR_POWER_REEL), 1 );
		nfChar.SetCharacterMaxPower(dMAXCharacterPower);

		if ((long)nfChar.GetCharacterMaxPower() != (long)RcvMsg.m_dCharacterMaxPower)
			return NF::EC_FR_CHAR_MAX_POWER;

		// 2011.02.16 ���� �ʱⰪ MAX/2�� ����
		nfChar.SetCurrentCharacterPower(dMAXCharacterPower/MAX_CHAR_POWER);
		biteFish.SetCurrentFishAttack(biteFishInfo.m_dAttack/MAX_FISH_ATTACK);

		// Commented by hankhwang - 2011�� 7�� 7��
		nfChar.SetLowRodAngleMax(dMAXCharacterPower * ROD_ANGLE_MAX);
		nfChar.SetHighRodAngleMax(dMAXCharacterPower * ROD_ANGLE_MAX);

		double dTempCharTensileStrength = (dMAXCharacterPower/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;
		double dTempFishTensileStrength = (biteFishInfo.m_dAttack/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH + (sqrt(biteFish.GetFishResultWeight())*FISH_TENSILE_RATE);
		double dTempLoadPresent = dTempCharTensileStrength + dTempFishTensileStrength;

		//Load_Present (100%) < Load_Max �� ��� (�ٴú����� ���� - 2010.5.6)
		if  ((dTempLoadPresent*COMPARE_PRESENT_MAX) <= dLineLoadMaxAdd)
		{
			// i) ĳ���� ���尭�� > ( ����� ���尭��+ ����� ���� ) * 3 ��, RE_Load_Max, RE_Load_Limit �� ���� ������ ����.
			// ii) ĳ���� ���尭�� <= ( ����� ���尭�� + ����� ���� ) * 3 ��, RE_Load_Max, RE_Load_Limit �� ���� ������ ����.
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

		// �⺻ Fatigue + ������ �ִ� ��� �����ۿ� ���� ���� Fatigue���� ���ļ� Stress(2�� ���� �����Ѵ�.)
		dMaxCharTireless = (long)ROUND(GetTirelessByLevel(nfCharInfoExt.m_nfCharBaseInfo.m_lLevel) + (sqrt(nfCharInfoExt.m_nfAbility.m_dHealth + abilityExt.m_nfAbility.m_dHealth) * GET_TIRELESS_BY_LEVEL), 1);

		nfChar.SetTireless(0, dMaxCharTireless);
		SendMsg.m_dMaxCharTireless = dMaxCharTireless;
	}

	//////////////////////////////////////////////////////////////////////////
	// ����� ���尭�� ���� 2011-02-15
	double dFishPowerAcceleration = biteFishInfo.m_dAttackAmount;
	double dFishPowerDropAcceleration = dFishPowerAcceleration * FISH_DROP_ACCEL;
	if (lineBreakInfo.m_dFishAttackTime < 0.0f)
		dFishPowerAcceleration *= FISH_DROP_ACCEL;

	double dAddFishPower = lineBreakInfo.m_dFishAttackTime * dFishPowerAcceleration;
	biteFish.AddCurrentFishAttack(dAddFishPower);

	//////////////////////////////////////////////////////////////////////////
	// ����� ��ų ��� ���尭�� ���� 2011-02-24
	// ����Ⱑ ��ų�� ������̶��, ����� ��ų�� ������ ������ �ͼ� Attack���� ���Ѵ�.
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
	// Load_Present�� MAX�� �� ���� ���ؼ� �缳�� �Ǵ� LoadMax, LoadLimit
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
	// 1-1. ������ �����Ǵ� �� ( ���ӷ� )
	// ĳ������ ���� �����Ǿ��ٸ� ������ ������ �ִ� ������ �ʴ� ��� ������ ���� �����ϰ� �����ϴ����� ���� ���� �ʿ��ϴ�.
	// �� ���� REEL POWER�� ��ġ�� ���ؼ� ���� �ȴ�.
	// �巡���� �ܰ迡 ���� �Ǵ� ���� ���� ���� �ʴ´�.
	// �� ���� �� �� ���� ��( �Ҽ��� ���ڸ�, kg )
	// �����Ǵ� �� = SQRT( REEL POWER ) * 0.3
	// note) ���� �Ǵ� ���� �ʸ� �������� ���� �Ǵ� ���� ���ڴ�. ƽ�� �ʹ� ������..
	double dCharPowerAcceleration	= sqrt(nfCharInfoExt.m_nfAbility.m_dAgility) * CHAR_POWER_ACCEL;
	double dCharReleaseAccleration	= dCharPowerAcceleration * CHAR_RELEASE_ACCEL;
	double dAddCharPower			= lineBreakInfo.m_dReelClickAddCharPower * dCharPowerAcceleration;
	double dReleaseCharPower		= lineBreakInfo.m_dReelClickReleaseCharPower * dCharReleaseAccleration;
	double dCurrentCharacterPower	= nfChar.GetCurrentCharacterPower() + dAddCharPower + dReleaseCharPower;

	nfChar.SetCurrentCharacterPower(dCurrentCharacterPower);

	double dCharTensileStrength = (nfChar.GetCurrentCharacterPower()/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;

	if ((long)nfChar.GetCurrentCharacterPower() != (long)lineBreakInfo.m_dCharacterCurPower)
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck CurrentCharPower, C: ", lineBreakInfo.m_dCharacterCurPower, " VS S :", dCurrentCharacterPower);

	// Commented by hankhwang - 2011�� 7�� 7��
	// Rod�� ���尭��	
	double dRodAngleAcceleration = sqrt(nfCharInfoExt.m_nfAbility.m_dAgility) * ROD_ANGLE_ACCEL;	
	double dRodAngleReleaseAcceleration = dRodAngleAcceleration * ROD_ANGLE_RELEASE_ACCEL;
	for(int i= 0; i<2; i++)
	{
		double dAddRodPower = lineBreakInfo.m_dRodAngleAddTime[i] * dRodAngleAcceleration;
		double dReleaseRodPower = lineBreakInfo.m_dRodAngleSubTime[i] * dRodAngleReleaseAcceleration;

		// 2���� ��������
		if(i)
			nfChar.SetCurrentHighRodAngle(nfChar.GetCurrentHighRodAngle() + (dAddRodPower + dReleaseRodPower));
		else
			nfChar.SetCurrentLowRodAngle(nfChar.GetCurrentLowRodAngle() + (dAddRodPower + dReleaseRodPower));

	}

	// Rod�� ���尭��
	double dRodTensileStrength = ((nfChar.GetCurrentHighRodAngle() + nfChar.GetCurrentLowRodAngle())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;
	double dLineLoadPresent = dCharTensileStrength + dRodTensileStrength + dFishTensileStrength + (sqrt(biteFish.GetFishResultWeight())*FISH_TENSILE_RATE);

	// 2011.02.24 New DropAcceleration �߰�
	double dDropAccelerationValue = (dCharReleaseAccleration + dFishPowerDropAcceleration) * DROP_ACCEL_VALUE;
	double dDropAcceleration = nfChar.GetDropAcceleration() + (dDropAccelerationValue * lineBreakInfo.m_dDropAccelTime);
	if (dDropAcceleration < 0.0f)
		dDropAcceleration = 0.0f;
	if (dDropAcceleration > dLineLoadPresent)
		dDropAcceleration = dLineLoadPresent;

	nfChar.SetDropAcceleration(dDropAcceleration);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2011.02.24 New Drag Level �߰�
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
	// �巡�׿� ���ؼ� �����, ĳ���� ���尭�� ����
	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 ����� 
	double dFishAttack = biteFish.GetCurrentFishAttack();
	dFishAttack -= dSubFishPower;
	if (dFishAttack < 0.0f)
		dFishAttack = 0;
	if (dFishAttack > biteFishInfo.m_dAttack)
		dFishAttack = biteFishInfo.m_dAttack;
	biteFish.SetCurrentFishAttack(dFishAttack);

	dFishTensileStrength = ((biteFish.GetCurrentFishAttack()+biteFish.GetCurrentFishSkillAttack())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH;


	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 ĳ���� 
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

	// ��Ǯ���� ���� �ε带 ����ϰ� ���������� ���Ѵ�.
	if ((long)dLineLoadPresent != (long)lineBreakInfo.m_fLineLoadPresent)
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, ~~~~~~~~~~~~~~ C : ", lineBreakInfo.m_fLineLoadPresent, " VS S : ", dLineLoadPresent, " ~~~~~~~~~~~~~~");
	theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, $$$$$ CharTensile : ", dCharTensileStrength, ", FishTensile : ", dFishTensileStrength, ", FishWeight :", biteFish.GetFishResultWeight());
	theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, $$$$$ LineLoadPresent : ", dLineLoadPresent, ", MaxLineTension : ", dRealLoadMax);

	// Usable ������ ����
	// ����� UsableItem ��������, Load Control ����
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
	//4-2. ����� HP
	//������� HP�� DB���� ���� �����ȴ�. �׸��� ��� ���ο� ���ؼ� HP�� �����ϰ� �ȴ�.
	//i) �ڿ� ����
	//������� HP�� �ð��� ������ ���� �ڿ������� �����ϰ� �Ǹ� �� ���� ����� HP�� ������ ���ȴ�.
	//������� HP�� �����ϴ� �ð��� ����Ⱑ ������ ���� �ӵ��� �����ϴ�. 
	//�ڿ� ���� HP = ����� ��ü HP * 0.02 ( ������ )
	//������� �ڿ� ���� HP�� �ڽ��� ��ü ü���� �� �Ҹ��� ���� �ִ�.
	long lFishHP = (long)(biteFishInfo.m_lHitPoint * FISH_HP);


	//iii) ĳ���� �� ����
	//ĳ������ �������� ���ҵǴ� ������� HP�� �ǹ��Ѵ�.y
	//ĳ������ ���� ����� DB�� ���� �ӵ�(�̵�����)�� ���ŵ� �� ����⿡�� ���޵ȴ�.
	//������ ĳ������ ���� ���� �ټ��� ����(%)�� ���ؼ� �����ȴ�.
	//ĳ���� �� = ĳ���� �� * ( Load_Present / Load_Max ) ( ������ )
	//��, ���⿡��  Load_Present (100%) > Load_Max �϶� ĳ���� ���尭�� > ( ����� ���尭��+ ����� ���� ) * 2 �� ��쿡��
	//ĳ������ ���� 100% ����ϵ��� �Ѵ�. ( ĳ������ �ɷ��� ����� ���� ������ ���ٴ� ���� �ǹ��ϱ� �����̴�. )
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
	// Bullet ��� �� ���, �ڿ� ���Ҵ� ���� �ʴ´�... 2011/6/8
	// ������� ���, ����� ü���� ���� ���� �ʱ� ���ؼ� CharPower�� 0����...
	if (fishSkillInfo.m_lIsBullet != NF::G_NF_ERR_SUCCESS)
	{
		lCharPower = (long)(dCharMaxPower * dLineLoadPresent / (dRealLoadMax * FISH_HP_CHAR_POWER));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ȯ�濡 ���� ����� ���� ƽ�� ü�¿��� ����� �ϱ� ������ ���⼭ ����Ѵ�. 
	// ������� ������ �����Ѵٴ� ��������, ����� ���ݷ��� * 15% ����
	if (pUser->GetEnvDebuff())
		lFishHP = (long)(lFishHP * FISH_HP_DEBUFF);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2010/7/21 ���� - bback99 
	// ���� �ɷ�ġ�� �������� HP = 0 �� ���� �ʴ���� ������ �� �� �ִ�.											
	// ������� �ִ� HP�� HP_MAX ��� �ϰ�, ������ HP�� HP, ĳ������ ���� �ɷ�ġ�� ���� ������� ������ ������ HP�� Landing HP(%) ��� �ϸ�,											
	// �켱 ���� ������� HP �ۼ�Ʈ�� ������ ���� �� �� �ִ�.											
	// ���� HP �ۼ�Ʈ(%, �Ҽ��� ���ڸ�) = HP / HP_MAX * 100
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

// Ŭ���̾�Ʈ���� ���� MsgCliNGS_ReqFightingAction�� �ʵ尪���� �����ϵ��� ����(2010/10/27 - ��)
// 2010/12/13 - FishAI�� Ŭ���̾�Ʈ�� �����鼭 ���������� �����ֱ��� SnapShop ������ �����ϵ��� ����
// SendMsg.ErrorCode -> 1:LineBreak, 2:FightingFail(ĳ�����Ƿε�==MAX), 3:Randing(FishHP==0), 4:continue...
LONG CRoomInternalLogic::LineBreakCheck(CUser* pUser, MsgNGSCli_AnsFigtingResult& SendMsg, MsgCliNGS_ReqFighting& RcvMsg, LineBreakInfo& lineBreakInfo)
{
	if (!pUser)
		return FALSE;

	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);

	CNFChar&	nfChar			= pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt= nfChar.GetNFChar().m_nfCharInfoExt;		// ������ ������ �����Ϸ��� �� ������...
	NFAbilityExt& abilityExt	= nfChar.GetAbilityExt();
	CFish&		biteFish		= nfChar.GetBiteFish();
	FishInfo&	biteFishInfo	= biteFish.GetFish();

	// NFUser(�⺻ �ɷ�ġ + EquipItem + ClothItem) + NFUserItem(UsableItem + SkillItem)
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
	// Lure ������ ��������
	pLureItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
	if (!pLureItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Lure].m_lItemCode);
		return NF::EC_FR_LURE_NOT_FOUND;
	}

	// Line ������ ��������
	pLineItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode);
	if (!pLineItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Line].m_lItemCode);
		return NF::EC_FR_LINE_NOT_FOUND;
	}

	// Reel ������ ��������
	pReelItem = theNFDataItemMgr.GetEquipItemByIndex(nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Reel].m_lItemCode);
	if (!pReelItem)
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck Not Found Item :", nfCharInfoExt.m_nfCharInven.m_mapUsingItem[eItemType_Reel].m_lItemCode);
		return NF::EC_FR_REEL_NOT_FOUND;
	}

	// Rod ������ ��������
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
	// Char�� �� : MAX, DragLevel�� ���� ��
	double dMAXCharacterPower = nfChar.GetCharacterMaxPower();
	if (dMAXCharacterPower <= 0)
	{
		// ĳ���� �� MAX
		// ĳ������ ���� �� ĳ������ ���ݷ��� �ǹ� �ϸ� ������� ü���� �Ҹ� ��Ű�� ��ġ��.
		// ROD POWER, REEL POWER ��ġ�� �ݿ��Ѵ�.
		// �� ĳ���� ��(���ݷ�) ( �Ҽ��� ���ڸ�, N )
		dMAXCharacterPower = ROUND( (sqrt(nfCharInfoExt.m_nfAbility.m_dStrength)*CHAR_POWER_ROD) + (sqrt(nfCharInfoExt.m_nfAbility.m_dAgility)*CHAR_POWER_REEL), 1 );
		nfChar.SetCharacterMaxPower(dMAXCharacterPower);

		if ((long)nfChar.GetCharacterMaxPower() != (long)RcvMsg.m_dCharacterMaxPower)
			return NF::EC_FR_CHAR_MAX_POWER;
		
		// 2011.02.16 ���� �ʱⰪ MAX/2�� ����
		nfChar.SetCurrentCharacterPower(dMAXCharacterPower/MAX_CHAR_POWER);
		biteFish.SetCurrentFishAttack(biteFishInfo.m_dAttack/MAX_FISH_ATTACK);

		// Commented by hankhwang - 2011�� 7�� 7��
		// Rod�� �ƽ���(ĳ���� ���� 15%+15% = 30%)
		nfChar.SetLowRodAngleMax(dMAXCharacterPower * ROD_ANGLE_MAX);
		nfChar.SetHighRodAngleMax(dMAXCharacterPower * ROD_ANGLE_MAX);


		//2009.9.22 �߰� ����
		// Load_Revision �� ���ϱ� ����, Load_Prensent�� Load_Max�� ���ؼ� ���� ��츦 �ε��� �Ѵ�.
		// ĳ����, ����� �ɷ�ġ 100%�� �ݿ��� Load_Present �� ���� Load_Max ���� ���� ��찡 �ΰ����� �߻� �Ѵ�. 
		// ù��° ���� ĳ������ ���尭���� ����� ���� ������ ���
		// �ι�°�� ĳ������ ���尭���� ������ ����ϰų� ���� ���� ���еȴ�.
		// �̷� ��쿡�� �Ʒ��� ������ ���ؼ� RE_Load_Max, Re_Load_Limit �� ���ؼ� ���� �극��ũ Ȯ���� ���Ѵ�.
		double dTempCharTensileStrength = (dMAXCharacterPower/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;
		double dTempFishTensileStrength = (biteFishInfo.m_dAttack/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH + (sqrt(biteFish.GetFishResultWeight())*FISH_TENSILE_RATE) ;
		double dTempLoadPresent = dTempCharTensileStrength + dTempFishTensileStrength;

	
		//Load_Present (100%) < Load_Max �� ��� (�ٴú����� ���� - 2010.5.6)
		if  ((dTempLoadPresent*COMPARE_PRESENT_MAX) <= dLineLoadMaxAdd)
		{
			// i) ĳ���� ���尭�� > ( ����� ���尭��+ ����� ���� ) * 3 ��, RE_Load_Max, RE_Load_Limit �� ���� ������ ����.
			// ii) ĳ���� ���尭�� <= ( ����� ���尭�� + ����� ���� ) * 3 ��, RE_Load_Max, RE_Load_Limit �� ���� ������ ����.
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


		//4. ĳ���� �Ƿε�
		//ĳ������ �Ƿε��� LV�� ���� ���� �Ƿε� �������� ����� ������ ���ȴ�.
		//ĳ���� �Ƿε� = LV �Ƿε� + �Ƿε� ���� ����
		//i) LV�� ���� �Ƿε� 
		//LV�� ���� �Ƿε� �������� ���ĺ��ٴ� DB���� ���� ������ ����� ����ϴ°��� ���ڴ�.

		//ii) �Ƿε� ���� ����
		//ĳ������ �Ƿε� ������ �翡 ���ؼ� �����Ǵ� �Ƿε��� ���̴�.
		//�Ƿε� ���� ���� = SQRT( �Ƿε� ) * 50 ( ������ )

		// �⺻ Fatigue + ������ �ִ� ��� �����ۿ� ���� ���� Fatigue���� ���ļ� Stress(2�� ���� �����Ѵ�.)
		dMaxCharTireless = (long)ROUND(GetTirelessByLevel(nfCharInfoExt.m_nfCharBaseInfo.m_lLevel) + (sqrt(nfCharInfoExt.m_nfAbility.m_dHealth + abilityExt.m_nfAbility.m_dHealth) * GET_TIRELESS_BY_LEVEL), 1);

		nfChar.SetTireless(0, dMaxCharTireless);
		SendMsg.m_dMaxCharTireless = dMaxCharTireless;
	}

	//////////////////////////////////////////////////////////////////////////
	// ����� ���尭�� ���� 2011-02-15
	double dFishPowerAcceleration = biteFishInfo.m_dAttackAmount;
	double dFishPowerDropAcceleration = dFishPowerAcceleration * FISH_DROP_ACCEL;
	if (lineBreakInfo.m_dFishAttackTime < 0.0f)
		dFishPowerAcceleration *= FISH_DROP_ACCEL;

	double dAddFishPower = lineBreakInfo.m_dFishAttackTime * dFishPowerAcceleration;
	biteFish.AddCurrentFishAttack(dAddFishPower);

	//////////////////////////////////////////////////////////////////////////
	// ����� ��ų ��� ���尭�� ���� 2011-02-24
	// ����Ⱑ ��ų�� ������̶��, ����� ��ų�� ������ ������ �ͼ� Attack���� ���Ѵ�.
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
	// Load_Present�� MAX�� �� ���� ���ؼ� �缳�� �Ǵ� LoadMax, LoadLimit
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
	// 1-1. ������ �����Ǵ� �� ( ���ӷ� )
	// ĳ������ ���� �����Ǿ��ٸ� ������ ������ �ִ� ������ �ʴ� ��� ������ ���� �����ϰ� �����ϴ����� ���� ���� �ʿ��ϴ�.
	// �� ���� REEL POWER�� ��ġ�� ���ؼ� ���� �ȴ�.
	// �巡���� �ܰ迡 ���� �Ǵ� ���� ���� ���� �ʴ´�.
	// �� ���� �� �� ���� ��( �Ҽ��� ���ڸ�, kg )
	// �����Ǵ� �� = SQRT( REEL POWER ) * 0.3
	// note) ���� �Ǵ� ���� �ʸ� �������� ���� �Ǵ� ���� ���ڴ�. ƽ�� �ʹ� ������..
	double dCharPowerAcceleration	= sqrt(nfCharInfoExt.m_nfAbility.m_dAgility) * CHAR_POWER_ACCEL;
	double dCharReleaseAccleration	= dCharPowerAcceleration * CHAR_RELEASE_ACCEL;
	double dAddCharPower			= lineBreakInfo.m_dReelClickAddCharPower * dCharPowerAcceleration;
	double dReleaseCharPower		= lineBreakInfo.m_dReelClickReleaseCharPower * dCharReleaseAccleration;
	double dCurrentCharacterPower	= nfChar.GetCurrentCharacterPower() + dAddCharPower + dReleaseCharPower;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ����� ��
	// ĳ���� ���޵Ǿ� ĳ������ �Ƿε��� ���� ��Ű�� ������� ���ݷ�.
	// ������� ���� ������� ���� �ӵ��� �����ؼ� �� �ð� ���� ���� ����.
	// ������� ���� �Ƿε��� �ٷ� ���� �� ���� �ְ�, ĳ������ Ư�� ��ġ�� ������� ���� �ٿ��� �ݿ��� ���� �ִ�.
	// ( ���ڰ� �� ���� ������ �ϴµ�?? )
	// �� ����� ��
	// ����� DB�� ���� �״�� ��� �Ѵ�.


	// ĳ���� ���� ����
	// ������ ���尭���� �������� ĳ������ ���� ���尭��
	nfChar.SetCurrentCharacterPower(dCurrentCharacterPower);

	double dCharTensileStrength = (nfChar.GetCurrentCharacterPower()/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;

	if ((long)nfChar.GetCurrentCharacterPower() != (long)lineBreakInfo.m_dCharacterCurPower)
		theLog.Put(DEV_UK, "NGS_LOGIC, LineBreakCheck CurrentCharPower, C: ", lineBreakInfo.m_dCharacterCurPower, " VS S :", dCurrentCharacterPower);

	// Commented by hankhwang - 2011�� 7�� 7��
	// Rod�� ���尭��	
	double dRodAngleAcceleration = sqrt(nfCharInfoExt.m_nfAbility.m_dAgility) * ROD_ANGLE_ACCEL;	
	double dRodAngleReleaseAcceleration = dRodAngleAcceleration * ROD_ANGLE_RELEASE_ACCEL;
	for(int i= 0; i<2; i++)
	{
		double dAddRodPower = lineBreakInfo.m_dRodAngleAddTime[i] * dRodAngleAcceleration;
		double dReleaseRodPower = lineBreakInfo.m_dRodAngleSubTime[i] * dRodAngleReleaseAcceleration;

		// 2���� ��������
		if(i)
			nfChar.SetCurrentHighRodAngle(nfChar.GetCurrentHighRodAngle() + (dAddRodPower + dReleaseRodPower));
		else
			nfChar.SetCurrentLowRodAngle(nfChar.GetCurrentLowRodAngle() + (dAddRodPower + dReleaseRodPower));
		
	}

	// Rod�� ���尭��
	double dRodTensileStrength = ((nfChar.GetCurrentHighRodAngle() + nfChar.GetCurrentLowRodAngle())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// �������� ĳ����~����� �Ÿ�(Distance), Ǯ�� ������ (Line_length)
	// ����Ⱑ �� ���� �������� ���۵Ǹ� Distance�� Line_Length�� ���� ���Ǹ� ������� �̵� ���⿡ ������ ���� �޴´�.
	// ������� 3,4,5 ������ ĳ���� ������ ��������� �����̸� �̶� Distance�� Line_Length�� ���̳��� �ȴ�.
	// �켱 Distance > Line_length �� ���� ����� �߻����� �ʴ� ������ �����ϵ��� �Ѵ�.
	// Distance < Line_length �� �Ǵ� ����
	// i) ������� �̵��ӵ� > REEL ���� ȸ���� 
	// ii) ĳ���Ͱ� REEL�� ���� �ʴ� ���
	// �� ������ ���ִ�.
	// �� ���� Line_Length�� Distance �� �������� ���ؼ� ������ ������ ��� �ؾ��ϸ� �̶� ���� �ټ��� ���� ������
	// �پ��� �ȴ�.
	// �� ��츦 ����ȭ �غ��� �Ʒ��� ����.
	// Distance < ( Line_Length + (Distance * 5%) ) �� �Ǹ�, Load_Prensent �� 0�� �Ǹ�
	// ������ ���� ���ӷ� ������ �ټ��� ������ �����ϰ� �ȴ�.
	// ���� ���ӷ� = ( ĳ���� ���ӷ� * 2 ) + ( ����� ���ӷ� * 2 )
	// �ټ��� �����ϴٰ� Distance >= ( Line_Length + (Distance * 5%) ) �� �Ǹ�
	// �ٽ� �������� �ټ����� �����ϰ� �Ǹ� ĳ����, ����� ���ӷ� ��ŭ �����ϰ� �ȴ�.


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� ���� ���
	// ���ο� �ɸ��� ���ϸ� ��� �ϴ� �κ��̴�.
	// ���� ���ο� �ɸ��� ���ϸ� Load_Present ��� �ϸ�
	// �� ���� ����( ( �Ҽ��� ���ڸ�, kg )
	// Load_Present = ĳ���� ���� ���� + ����� ���� ���� + ����� ���� + ��� ���� + ��Ÿ - ���� ���ӷ�
	// 2011/02/24 - Load_Present = ĳ���� ���� ���� + ����� ���� ���� + ����� ���� + ����� ��ų + ���Acceleration �� ����
	// " ( ��Ÿ�� ���ɿ� ���� ������ ���� ���� �ǹ��Ѵ�. �������� ���δ� ����ؾ� �Ѵ�. �׸��� ROD�� ����� ������� ���⿡
	// ���� �߰��Ǵ� ��(����)�� �ݿ��� ���α׷� ȸ�� �Ŀ� �����ϵ��� ����. )"

	double dLineLoadPresent = dCharTensileStrength + dRodTensileStrength + dFishTensileStrength + (sqrt(biteFish.GetFishResultWeight()) * FISH_TENSILE_RATE);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ������� �̵� ���⿡ ���� Load_Present �� ���� ������Ű�� ������ �����ϵ��� �Ѵ�.
	// ����Ⱑ ĳ���Ϳ� �־����� �������� �̵��Ϸ��ϸ� ���� ���� �ټ��� ������ ���� �̵� �Ÿ��� �����ϰ� �ִ�.
	// ������ �� �κ��� �����Ű�� ������ 0, 1, 7 �� �����̸� 2, 6�� ������ �������� �ʰ� �ִ�.
	// ���⼭ Distace �� Line_Lengh�� ���ؼ� ������� �̵��� �ټ��� ������ �����Ű�� ��� ������ �̵� ������ �����ϴ�.
	// Distance = Line_length �϶� ������� �̵� ������ Distance�� ������Ű�� �����̸� �̵� ������ �θ� �׷��� ������
	// �̵������� ���� �ʴ´�.
	// �̶� Load_Present * 10% �� �ټ� ���ϰ� �߰��ȴ�.

	// 2011.02.24 New DropAcceleration �߰�
	double dDropAccelerationValue = (dCharReleaseAccleration + dFishPowerDropAcceleration) * DROP_ACCEL_VALUE;
	double dDropAcceleration = nfChar.GetDropAcceleration() + (dDropAccelerationValue * lineBreakInfo.m_dDropAccelTime);
	if (dDropAcceleration < 0.0f)
		dDropAcceleration = 0.0f;
	if (dDropAcceleration > dLineLoadPresent)
		dDropAcceleration = dLineLoadPresent;

	nfChar.SetDropAcceleration(dDropAcceleration);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2011.02.24 New Drag Level �߰�
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
	// �巡�׿� ���ؼ� �����, ĳ���� ���尭�� ����
	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 ����� 
	double dFishAttack = biteFish.GetCurrentFishAttack();
	dFishAttack -= dSubFishPower;
	if (dFishAttack < 0.0f)
		dFishAttack = 0;
	if (dFishAttack > biteFishInfo.m_dAttack)
		dFishAttack = biteFishInfo.m_dAttack;
	biteFish.SetCurrentFishAttack(dFishAttack);

	dFishTensileStrength = ((biteFish.GetCurrentFishAttack()+biteFish.GetCurrentFishSkillAttack())/NEWTON_GRAVITY)*NEWTON_GRAVITY_RATE_FISH;
	

	//////////////////////////////////////////////////////////////////////////
	// 2011.02.24 ĳ���� 
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

	// ��Ǯ���� ���� �ε带 ����ϰ� ���������� ���Ѵ�.
	if ((long)dLineLoadPresent != (long)lineBreakInfo.m_fLineLoadPresent)
		theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, ~~~~~~~~~~~~~~ C : ", lineBreakInfo.m_fLineLoadPresent, " VS S : ", dLineLoadPresent, " ~~~~~~~~~~~~~~");
	theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, $$$$$ CharTensile : ", dCharTensileStrength, ", FishTensile : ", dFishTensileStrength, ", FishWeight :", biteFish.GetFishResultWeight());
	theLog.Put(WAR_UK, "NGS_LOGIC, LineBreakCheck, $$$$$ LineLoadPresent : ", dLineLoadPresent, ", MaxLineTension : ", dRealLoadMax);


	// 2010/11/30 - Ŭ���̾�Ʈ�� �����ִ� LoadPresent�� ����Ѵ�.

	// 2011/11/24 - ������ ������ ���� ���� �߰�
	DecrementEnduranceItem(nfChar, lineBreakInfo.m_lEnduranceType, pReelItem, pRodItem);


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� ���Ͽ� ���� �ٴ�����(Word ����) 
	// ���ο� �ɸ��� ���ϰ� 0�� �� �� �ڿ������� ����Ⱑ �ٴ��� ���� ������ ������ ó���Ѵ�.
	// 0�� �Ǵ� �������ٴ� 0���� 1�� ���� �������� �� �����ϵ��� ����.
	// �� ���� Ȯ���� ���ο� �ɸ��� ���ϰ� ���� �� �߻��ϰ� �ȴ�. ��, ���� �극��ũ Ȯ������ �ݴ��� �ǹ����� ��⸦ ��ġ�� ���� �����ϴ�.
	// ����Ⱑ �� ���� Ȯ���� ���� ���ο� �ɸ��� ���Ͽ� ���ؼ� �����ȴ�.
	// LoadPresent���� LoadMax�� ������ 30% �Ʒ��� 3�� �̻� ���� ���� ���(3�� ī��Ʈ�� �Ǹ�, ������ �ٴ� ����...)

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� �극��ũ Ȯ��
	// ������ ������ Ȯ���� ����ϴ� �κ��̴�.
	// ���ο� ������ �ִ� ����, �Ѱ� ������ ���� Load_Max, Load_Limit�̶�� �����ϰ�, ���� ���ο� �ɸ��� ���ϸ� Load_Present��� �ϸ�
	// ������ �������� Ȯ�� (LineBreak )�� ������ ����.
	// Load_Revision = Load_Prensent - Load_Max, ( ��, Load_Revison < 0, LineBreak = 0(%) )
	// ���� ���ο� �ɸ��� ���ϰ� �ִ� ���� ���� ���� ��쿡 ���� �극��ũ Ȯ���� 0 �̶�� �ǹ��̴�.
	// Linebreak(%) = ( Load_Revison ) / ( Load_Limit - Load_Max ) * 100
	// ���� �극��ũ Ȯ���� �Ҽ��� ���ڸ� ���� �ݿ��Ѵ�. 
	if (lineBreakInfo.m_lErrorCode == NF::EC_FR_LINEBREAK)
		return lineBreakInfo.m_lErrorCode;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// LineBreak�� �ƴϸ�... 
	// ĳ���Ϳ� ������� �Ƿε��� üũ�Ѵ�.
	// ĳ������ �Ƿε��� (== MAX�̸�, Fighting ����)
	// ĳ������ �Ƿε��� (!= MAX�̸�, Continue...)

	// ����� HP�� (== 0�̸�, Randing...)
	// ����� HP�� (!= 0�̸�, Continue...)
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Usable ������ ����
	// ����� UsableItem ��������, Load Control ����
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


	//4-1. �Ƿε� ����
	//�Ƿε� ������ ���� ������ ����.
	//i) ����� �Ƿε�
	//����� �� �Ƿε� = ����� ��
	//������� ���� ����� DB�� ���ݼӵ�(�̵�����)�� ���ŵ� �� ĳ���Ϳ��� ���޵ȴ�.
	// Bullet ��� �� ���, �ڿ� ���Ҵ� ���� �ʴ´�... 2011/6/8
	if (fishSkillInfo.m_lIsBullet != NF::G_NF_ERR_SUCCESS) {
		bIsFightingFail = nfChar.SetTireless(2, biteFishInfo.m_dAttack, nfChar.GetCurFishHP());
		if (bIsFightingFail)
		{
			theLog.Put(WAR_UK, "!! TirelessZero_Tick");
			return lineBreakInfo.m_lErrorCode;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//4-2. ����� HP
	//������� HP�� DB���� ���� �����ȴ�. �׸��� ��� ���ο� ���ؼ� HP�� �����ϰ� �ȴ�.
	//i) �ڿ� ����
	//������� HP�� �ð��� ������ ���� �ڿ������� �����ϰ� �Ǹ� �� ���� ����� HP�� ������ ���ȴ�.
	//������� HP�� �����ϴ� �ð��� ����Ⱑ ������ ���� �ӵ��� �����ϴ�. 
	//�ڿ� ���� HP = ����� ��ü HP * 0.02 ( ������ )
	//������� �ڿ� ���� HP�� �ڽ��� ��ü ü���� �� �Ҹ��� ���� �ִ�.
	long lFishHP = (long)(biteFishInfo.m_lHitPoint * FISH_HP);


	//ii) ��ų ��� ����
	//����Ⱑ DB�� ������ ��ų�� ����Ҷ� HP�� �Ҹ��ϰ� �ȴ�.
	//HP�� �����ϸ� ��ų�� ������� ���Ѵ�. 
	//��ų ��� ���� HP�� ��ų DB���� ���� �Ѵ�.
	//long lSkillFishHP = 0;


	//iii) ĳ���� �� ����
	//ĳ������ �������� ���ҵǴ� ������� HP�� �ǹ��Ѵ�.y
	//ĳ������ ���� ����� DB�� ���� �ӵ�(�̵�����)�� ���ŵ� �� ����⿡�� ���޵ȴ�.
	//������ ĳ������ ���� ���� �ټ��� ����(%)�� ���ؼ� �����ȴ�.
	//ĳ���� �� = ĳ���� �� * ( Load_Present / Load_Max ) ( ������ )
	//��, ���⿡��  Load_Present (100%) > Load_Max �϶� ĳ���� ���尭�� > ( ����� ���尭��+ ����� ���� ) * 2 �� ��쿡��
	//ĳ������ ���� 100% ����ϵ��� �Ѵ�. ( ĳ������ �ɷ��� ����� ���� ������ ���ٴ� ���� �ǹ��ϱ� �����̴�. )
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
	// Bullet ��� �� ���, �ڿ� ���Ҵ� ���� �ʴ´�... 2011/6/8
	// ������� ���, ����� ü���� ���� ���� �ʱ� ���ؼ� CharPower�� 0����...
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
	// ȯ�濡 ���� ����� ���� ƽ�� ü�¿��� ����� �ϱ� ������ ���⼭ ����Ѵ�. 
	// ������� ������ �����Ѵٴ� ��������, ����� ���ݷ��� * 15% ����
	if (pUser->GetEnvDebuff())
		lFishHP = (long)(lFishHP * FISH_HP_DEBUFF);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ������ �浹 �ڽ��� ���� ������, Ŭ���̾�Ʈ�� ������ �ٲٶ�� �޼����� ���´ٸ�, 
	// Ǯ�� ������ >= 10m �̰� ����� ü���� 0�� ��쿡 �� �������� �����ϴ� �� ���� �켱 �Ǿ�� �ϹǷ�.. SetFishPatternByLineLength�� ���� �����Ѵ�.
	// HP�� ���� ���� ���̷� ���� ����� ���� ���ϱ�
	//SetFishPatternByLineLength(BiteFish, NFUser, lFishHP); 

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2010/7/21 ���� - bback99 
	// ���� �ɷ�ġ�� �������� HP = 0 �� ���� �ʴ���� ������ �� �� �ִ�.											
	// ������� �ִ� HP�� HP_MAX ��� �ϰ�, ������ HP�� HP, ĳ������ ���� �ɷ�ġ�� ���� ������� ������ ������ HP�� Landing HP(%) ��� �ϸ�,											
	// �켱 ���� ������� HP �ۼ�Ʈ�� ������ ���� �� �� �ִ�.											
	// ���� HP �ۼ�Ʈ(%, �Ҽ��� ���ڸ�) = HP / HP_MAX * 100
	//double dFishHPRate = ROUND(nfChar.CalcFishHP(lSkillFishHP + lCharPower), 2);
	nfChar.CalcFishHP(lCharPower);

	//// �׸��� ������ ������ �ۼ�Ʈ�� ���ϴ� ������ ������ ����.
	//// Landing HP(%, �Ҽ��� ���ڸ�) = SQRT( ���� �ɷ�ġ ) * 5.5
	//double dLandingHP = ROUND(sqrt(nfCharInfoExt.m_nfAbility.m_dLanding) * LANDING, 2);

	//// Landing HP > ���� HP �ۼ�Ʈ �� ��� ���� �ٴ����̸� ����ϰ� �ٴ����� Ȯ���� ����� ���� �ϵ��� �Ѵ�.
	//if (nfChar.GetCurFishHP() == 0 || (dLandingHP >= dFishHPRate))
	//{
	//	theLog.Put(WAR_UK, "NGS_LOGIC, %%%%% Lading, CurFishHP : ", nfChar.GetCurFishHP());
	//	theLog.Put(WAR_UK, "NGS_LOGIC, %%%%% Lading, dLandingHP :", dLandingHP, " >= dFishHPRate :", dFishHPRate);
	//}

// Ŭ���̾�Ʈ�� ��û���� 2010-12-17 �ּ�ó��
// 		// 2010/7/21 ���� - bback99
// 		// ���� ������ ������ �Ÿ��� 400cm �� �����Ǿ� �ִ�. �� �Ÿ��� 300cm�� ���̰� ���� �ɷ�ġ�� �ݿ��ؼ� ���� ������ �Ÿ��� ���δ�.
// 		// ĳ������ ���� ���� �Ÿ�(Can_Landing_Distance) �� ������ ���İ� ����.
// 		double dLandingDistance = 120 + (sqrt(NFUI.m_nfAbility.m_dLanding) * 12);
// 
// 		if (nfChar.GetLineLength()*100 <= dLandingDistance)
// 		{
// 			theLog.Put(WAR_UK, "NGS_LOGIC, %%%%% Lading, Distance : ", dLandingDistance);
// 
// 			// ���� �ٴ�����
// 			// " �������� ���������� �߻��ϴ� �̺�Ʈ�� �ٴ�����.
			// ���� �ٴ������� �߻� Ȯ���� ����� DB���� �����ؼ� �����Ǹ�, �̺�Ʈ�� �߻��ϸ� '����' ��ġ�� �����ؼ� '���� �ٴ�����' Ȯ���� ���Ѵ�."
// 			// �� ���� �ٴ����� Ȯ�� ( �Ҽ��� ���ڸ�, % )
// 			// ���� �ٴ����� Ȯ�� = 50 - ( SQRT ( ��ŷ + ( ��Ű����Ʈ / 2 ) ) * 1.9 + 15 )  ��, ��� �ڵ�ĸ �ʱ� 10%, �ϼ� 5% �� �߰� Ȯ���� ���ֵ��� �Ѵ�.
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

// 			// �������� �ٴ����� Ȯ������ ������, �ٴ� ���̿� �ɸ�
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
		if (nRate >= nRand)		// Ȯ���� ���°���...
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
			LONG lDropItemRate = urandom(1000);			// �Ҽ��� ���ڸ�����...

			ForEachElmt(TlstDropItemRate, (*iter).second, it, ij)
			{
				lRateSum += (*it).m_lDropRate;
				if (lRateSum > 1000)
					break;

				if (lRateSum >= lDropItemRate)		// ������ ��...
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

					// DB�� Query
					theNFDBMgr.InsertBuyItem(1, (*it).m_lDropItemID, pUser->GetGSN(), pUser->GetCSN(), inven, (int&)lErr);
					if (NF::G_NF_ERR_SUCCESS != lErr)		// inven�� full�� ���, ���Ϸ� ���´ٴ� �� �˷���� �Ѵ�...
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

// Landing ������ �Ǹ�, ���� ����� �����ϱ� ���� ȣ��Ǵ� �Լ�
// ���� ��� �޼��� ����, 
// Single �̸�, ���ǿ� �ش��ϴ� ���� 5�� ���� ���� �����ֱ�
// Team �̸�, ���ǿ� ���� ������ �����ֱ�
void CRoomInternalLogic::SaveGameResult(CUser* pUser)
{
	CNFChar& nfChar = pUser->GetNFUser();
	CFish& biteFish = nfChar.GetBiteFish();
	LONG lCSN = pUser->GetCSN();

	MsgNGSCli_NotifyLandingResult		landingNotify;

	// 1. ���� ����� DB�� �����Ѵ�.
	// 2. ������ ���� ������ ��ȿ� �ִ� ��� �����鿡�� �����Ѵ�.
	// 3. ������ ���� ������ NGS�� �����մ� CHS�� �����Ѵ�.
	if (!biteFish.ValidCheck())
	{
		theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult(). BiteFish InValide. RoomID : ", RoomID2Str(GetRoomID()), ", CSN :", pUser->GetCSN());

		landingNotify.m_llGetGameMoney = 0;
		landingNotify.m_lGetExp = 0;

		PayloadNGSCli pldLanding(PayloadNGSCli::msgNotifyLandingResult_Tag, landingNotify);
		UserManager->SendToUser(lCSN, pldLanding);

		return;
	}

	// �������ʽ�
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

	// �������ʽ� ����
	pUser->SetAbil( AT_LANDING_BONUS, static_cast<int>( eLandingBonusType_None ) );

	// ����ġ ��������
	LONG lErr = theNFMenu.CalcLevelByAddExp(pUser->GetGSN(), pUser->GetNFCharInfoExt(), biteFish.GetFishResultExp(), GetRoomID());
	if (NF::G_NF_ERR_SUCCESS != lErr) 
	{
		if( NF::G_ERR_GET_NF_EXP_MAX == lErr ) // �����̸� ���̻� ���� ����ġ ����.
			biteFish.SetFishResultExp(0);
		else
		{
			theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult(). CalcLevelByAddExp Error!!!. RoomID : ", RoomID2Str(GetRoomID()), ", CSN :", pUser->GetCSN(), ", ErrorCode :", lErr);
			return;
		}
	}

	// ����� ��� �����ϱ�(�����ϱ�) - ����
	// ���� �� ������ ���ñ��� �Ѵ�.
	nfChar.SaveLandingFish(m_nfRoomOption.m_lWinCondition);

	// ��ȸ�� ���, ����� ��� �����ϱ� - TeamPlay
	if (m_nfRoomOption.m_lPlayType == PT_BATTLE_TEAM)
		SaveLandingResultTeam(pUser);

	// Ȥ�� ���� �߻��� ������ 1��
	LONG lSignType = 0;

	// ¡�� ����Ʈ�� ���Ѵ�.
	if (CheckSignPoint(biteFish.GetFish(), lSignType))
		theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult, CheckSignPoint Success!!! RoomID :", RoomID2Str(m_RoomID), " / SignType :", lSignType);

	//////////////////////////////////////////////////////////////////////////
	// DB�� ���� ����� ���� & ��� ��Ʈ�� ������Ʈ
	lErr = SaveLandingFishInfoToDB(nfChar, biteFish);
	if (NF::G_NF_ERR_SUCCESS != lErr)
		theLog.Put(ERR_UK, "NGS_LOGIC, SaveGameResult, SaveLockedNote Failed!!! RoomID :", RoomID2Str(m_RoomID), " / CSN :", pUser->GetCSN(), " / ErrorType :", lErr);

	//////////////////////////////////////////////////////////////////////////
	// LandingNotify ���� �����Ѵ�...
	theNFDataItemMgr.GetNFExp(nfChar.GetLevel(), landingNotify.m_lMaxExp);

	// ȹ�� �������� �����Ѵ�...
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
	// Fish Landing ��, GameResult ������ ��ü �������� Simple�ϰ� ����
	MsgNGSCli_NotifySimpleGameReuslt	gameResultAns;

	if (!GetGameResult(pUser, gameResultAns.m_lstPlayer, gameResultAns.m_lstTeam, TRUE, FALSE))		// Detail ������ ��û���� ����
		theLog.Put(WAR_UK, "NGS_LOGIC, SaveGameResult(). GetGameResult Failed. RoomID : ", RoomID2Str(GetRoomID()), ", CSN :", pUser->GetCSN());

	PayloadNGSCli pldGameResult(PayloadNGSCli::msgNotifySimpleGameResult_Tag, gameResultAns);
	UserManager->SendToAllUser(pldGameResult);
}

BOOL CRoomInternalLogic::PrevLoadLockedNote(CNFChar& nfChar, CFish& landingFish, NFLockedNoteMain& locked_main, LONG& lLengthIndex, LONG& lClassIndex, std::string& strFishGroup, std::string& strLength, std::string& strRegDate, std::string& strMapID)
{
	nfChar.GetLockedNoteMain(locked_main);

	// �ְ� ���� 2��
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

	// �ְ� Ŭ���� 2��
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

	// �ֱٿ� ���� �����
	if (locked_main.m_vecRecentlyLandingFish.size() != G_LOCKED_NOTE_RECENTLY_CNT)
		return FALSE;

	Recently_Landing_Fish fish;
	fish.Clear();
	landingFish.GetLockedNoteMainFishInfo(fish);

	TVecRecentlyLandingFish::iterator del_iter = locked_main.m_vecRecentlyLandingFish.begin();
	locked_main.m_vecRecentlyLandingFish.erase(del_iter);
	locked_main.m_vecRecentlyLandingFish.push_back(fish);

	// string���� ��ȯ
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
		// �ش� �ʿ��� �� ������ ���� ���...
		TMapLockedNote::iterator iterFish = (*iter).second.m_TblLockedNote.find(lLockedNoteFishID);	
		if (iterFish == (*iter).second.m_TblLockedNote.end()) 
		{
			// ���� ����Ⱑ ���� ���, ������ insert
			(*iter).second.m_TblLockedNote.insert(make_pair(lLockedNoteFishID, landFish));
			(*iter).second.m_lTotUnlockFishCNT++;
			nfChar.GetNFChar().m_nfCharInfoExt.m_nfLockedNote.m_nfLockedNoteMain.m_lTotCNTLandFish++;
		}
		else
		{
			// ���� ����Ⱑ �ִ� ���, ������ ���� ������ ���ؼ� ������Ʈ ���� �Ǵ�...
			if ((*iterFish).second.m_lLength < landFish.m_lLength)
				(*iterFish).second = landFish;
			
		}
		(*iter).second.m_lTotLockedScore += nfChar.GetBiteFish().GetResultFishScore();	// ����� ������ ������ Score�� �����ǰ�, �������� ������ �������� ����߸�, ++�ȴ�.
	}
	else
	{	
		// �ش� �ʿ��� ó������ ���� ���..
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

// ��ȸ�� ���, �� ���� �ش��ϴ� ������ LandingFish������ �� ���� �����Ѵ�.
void CRoomInternalLogic::SaveLandingResultTeam(CUser* pUser)
{
	CNFChar& nfChar = pUser->GetNFUser();
	CFish& BiteFish = nfChar.GetBiteFish();
	TotalLandingFish* totResult = NULL;

	// ���� ��ȣ�� 0 or ¦���̸�, A��
	if (pUser->GetUserSlot() % 2 == 0)
		totResult = &m_totLandingFishTeamA;
	else
		// ���� ��ȣ�� Ȧ���̸�, B��
		totResult = &m_totLandingFishTeamB;

	LandingFish landingFish;
	BiteFish.GetLandingFish(landingFish);
	totResult->m_lstLandingFish.push_back(landingFish);

	// get_result_sorting(5)
	SortingLandingFishResultTeam(totResult, m_nfRoomOption.m_lWinCondition, 5);
}

void CRoomInternalLogic::SortingLandingFishResultTeam(TotalLandingFish* totResult, LONG lWinCondition, LONG lTopSize)
{
	// ���� ������ �ʱ�ȭ!!!
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
		return bRet;		// FreeMode �� ��쿡�� ���� �Ұ��� ����..
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

	// DB ����...
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
		
		// ������ �� ��쿡��, �Ѹ����� �� ������ �û�Ĵ뿡 �ø��� �ʾƾ� �ϱ⿡...
		// GetSingleGameResult���� -1�� �����س��� ������ Rank�� -1�̱⿡ .. �״�� -1�� ����
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

		// ������ ���� ��ŷ�� 1���� �����ϰ�, ������ ��쿡 ������ 1, 1�� �ȴ�. 
		// ���� ���� �Ǹ�, 1, 1, 3 �̷�������...
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

// FreeMode Play��,
BOOL CRoomInternalLogic::GetFreeModeGaemResult(const CNFChar& nfChar, GameResult& gameResult)
{
	GameResult tempGameResult(nfChar.GetSaveLandingFish(), 0, nfChar.GetLevel(), nfChar.GetGSN(), nfChar.GetCSN(), nfChar.GetExp());
	gameResult = tempGameResult;
	return TRUE;
}

// Single Play��, 
BOOL CRoomInternalLogic::GetSingleGameResult(TLstGameResult& lstPlayer, BOOL bIsSimple)
{
	UserManager->GetSingleGameResult(lstPlayer, bIsSimple);

	// Sorting
	if (bIsSimple)		// ������ ������ ���̸�, ���� 5��
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

// Team ������,
BOOL CRoomInternalLogic::GetTeamGameResult(TLstGameResult& lstPlayer, TLstGameResult& lstTeam, BOOL bIsSimple)
{	
	BOOL bRet = TRUE;

	TLstGameResult		lstATeamPlayer, lstBTeamPlayer;

	if (!bIsSimple)		// Detail�̸�, 
	{
		// 1. lstPlayer�� �������� ������.
		UserManager->GetTeamGameResult(lstATeamPlayer, lstBTeamPlayer, bIsSimple);

		// 2. A, B���� Sorting�Ѵ�.
		SortingGameResult(lstATeamPlayer, m_nfRoomOption.m_lWinCondition);
		SortingGameResult(lstBTeamPlayer, m_nfRoomOption.m_lWinCondition);
		
		// 2-1-1. ������ Ranking
		CalcRankGameResultSingle(lstATeamPlayer, m_nfRoomOption.m_lWinCondition);
		CalcRankGameResultSingle(lstBTeamPlayer, m_nfRoomOption.m_lWinCondition);

		// 2-1, A, B�� �������� lstPlayer�� �߰��Ѵ�.
		ForEachElmt(TLstGameResult, lstATeamPlayer, it, ij)
			lstPlayer.push_back((*it));

		// 2-1, A, B�� �������� lstPlayer�� �߰��Ѵ�.
		ForEachElmt(TLstGameResult, lstBTeamPlayer, it, ij)
			lstPlayer.push_back((*it));

		// 3. Team�� ���� �����Ѵ�.
		// A���� 0, B���� 1�� ������ �����Ѵ�. - ���߿� ���� ������ ���� ��� �ٽ� ��� �ؾ� ��!!!
		GameResult gameATeamResult(m_totLandingFishTeamA, 1), gameBTeamResult(m_totLandingFishTeamB, 2);
		lstTeam.push_back(gameATeamResult);
		lstTeam.push_back(gameBTeamResult);
		
		// 4. �¸����� �����Ѵ�.
		SortingGameResult(lstTeam, m_nfRoomOption.m_lWinCondition);
		CalcRankGameResultTeam(lstTeam, m_nfRoomOption.m_lWinCondition);

		
		TMapTeamData	mapTeamData;

		// 5.�¸��� ���� 
		ForEachElmt(TLstGameResult, lstTeam, it, ij) 
		{
			CRankNWinType		team;
			team.lRank = (*it).m_lRank;
			team.lWinType = (*it).m_lWinType;
			mapTeamData[(*it).m_lBattleType-1] = team;
		}

		// 6. �����ִ� �÷��̾���� ��ŷ�� ���Ѵ�.
		ForEachElmt(TLstGameResult, lstPlayer, it, ij) {
			TMapTeamData::iterator	iter = mapTeamData.find((*it).m_lBattleType-1);
			if (iter == mapTeamData.end())
				return FALSE;

			(*it).m_lWinType = mapTeamData[(*it).m_lBattleType-1].lWinType;
			(*it).m_lRank = mapTeamData[(*it).m_lBattleType-1].lRank;
		}
	}
	else		// �����ϰ� ������� �ȴٸ�, 
	{
		GameResult gameATeamResult(m_totLandingFishTeamA), gameBTeamResult(m_totLandingFishTeamB);
		lstTeam.push_back(gameATeamResult);
		lstTeam.push_back(gameBTeamResult);
	}

	return bRet;
}

// ��ų(SkillItem, UsableItem) ���� ȣ��Ǵ� �Լ�
// Invalid Check, DB update (activate_gameitem procedure call), send ans msg to client 
long CRoomInternalLogic::ApplyUseItem(CUser* pUser, LONG lItemCode, LONG lQuickSlotIndex, AnsUsedItem& usedItem)
{
	CNFChar& nfChar = pUser->GetNFUser();
	NFCharInfoExt& nfCharInfoExt = nfChar.GetNFChar().m_nfCharInfoExt;

	//// �ڽ��� Inven���� �˻��ؿ´�. usable�� skill���� �����ؾ� ��
	//TMapInvenSlotList::iterator iter = nfCharInfoExt.m_nfCharInven.m_mapCountableItem.find(lItemCode);
	//if (iter == nfCharInfoExt.m_nfCharInven.m_mapCountableItem.end())
	//	return NF::G_NF_ERR_NOT_FOUND_ITEM_MY_INVEN_COUNTABLE;		// not found to my usable inven

	//// �κ����� ������ ī��尡 ���� ���� �κ����� ������ �´�...
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

	// �������� ������ ���� ��� �������� üũ
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

		if (GetSignType() >= 1) // �̹� ¡�� �߻����ε�
		{
			if( 0 < lForceSignIndex ) // ġƮ�� �߻���Ų �Ÿ� ���� ¡�� ���ְ� ���ο� ¡�� �߻���Ų��.
			{
				NtfEndSignMsg();
			}
			else
			{
				return FALSE;
			}
		}	

		// � ¡���� �߻� �� ������....
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

					// ���� ¡����
					SetSigning(lSignIndex);

					// ¡���� ���۵����� Ÿ�̸Ӹ� ���� �˸���.
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
	// ���� �����ϰ� �ִ� Line�������� m�� üũ
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

	// ����Ʈ ���������� ��ü �ߴٰ� �˷���.. �ɷ�ġ�� ���� �ؾ� ��
	if (!bIsSuccess)
	{
	}
	return TRUE;
}

// �ʼ� : LineItem�� List������ ������������ ���ĵǾ� �־�� �ҵ�...
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
			if (theNFDataItemMgr.CheckItemTypeByItemCode("E", (*itLine3).m_lItemCode, (*itLine3).m_lPartsIndex))		// ������
				return;

			LONG lDBRemainCount = 0;
			LONG lReduceGauge = lRemainLength;
			if (lReduceGauge <= 0)
				break;

			// ���Ŀ� �ý��ۿ��� �ڵ����� �����ؾ� �� ��ƾ���� �Ѱܾ� �ϱ� ������, ���� �޸𸮻����θ� 0���� ���ܵд�.(DB�� ��¥�� ġ�ﲨ�� ������ ������Ʈ �� ��)
			if ((*itLine3).m_lRemainCount < lRemainLength)
				lReduceGauge = (*itLine3).m_lRemainCount;

			lDBRemainCount = lReduceGauge;

			// ��Ÿ������ DBȣ��
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
		if (theNFDataItemMgr.CheckItemTypeByItemCode("E", inven.m_lItemCode, inven.m_lPartsIndex))		// ������
			return;		// ������ ������

		LONG lReduceGauge = inven.m_lRemainCount;
		if (inven.m_lRemainCount >= lReduceCount)
			lReduceGauge = lReduceCount;
			
		// ��Ÿ������ DBȣ��
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

// �ʼ� : LineItem�� List������ ������������ ���ĵǾ� �־�� �ҵ�...
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
			if (theNFDataItemMgr.CheckItemTypeByItemCode("E", inven.m_lItemCode, inven.m_lPartsIndex))		// ������
				return;

			LONG lReduceGauge = 1;
			inven.m_lRemainCount -= lReduceGauge;
			if (inven.m_lRemainCount < 0)
				lReduceGauge = 0;
			else
			{
				// ��Ÿ������ DBȣ��
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
	// ģ�� ���·� ������Ʈ
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
