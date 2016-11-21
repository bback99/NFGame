#include "stdafx.h"
#include <achv/AchvDef.h>
#include "TinyXML/tinyxml.h"
#include <NFVariant/NFDBManager.h>

const string COMPARE_TYPE_GREATER = "GREATER";
const string COMPARE_TYPE_EQUAL = "EQUAL";
const string COMPARE_TYPE_LESS = "LESS";
const string COMPARE_TYPE_RANGE = "RANGE";

using namespace achv;
using namespace std;

/*static*/
PfnLogGetter_T achv::CAchvMgr::pfnLogGetter_ = &GetTheLog;   // @ ServerLog.h

//--------------------------------------------------------------------
// Name:BreakSeparator()
// Dec: 구분자(Separator)를 기준으로 string을 분리하는 함수
//--------------------------------------------------------------------
void BreakSeparator(const string& rkText, vector<string>& rkOut, const char* pSeparator)
{
	string kTempText = rkText;
	char* pToken = NULL;

	pToken = strtok(&kTempText[0], pSeparator);
	while (pToken)
	{
		rkOut.push_back(pToken);
		pToken = strtok(NULL, pSeparator);
	}	

	if (rkOut.empty())
	{
		rkOut.push_back(rkText);
	}
}


//--------------------------------------------------------------------
// Name:IsComplete()
// Dec: 이미 완료된 업적인지 체크하는 함수
//--------------------------------------------------------------------
BOOL CAchvMgr::IsComplete(const LONG lCSN, const LONG lAchvID)
{
	MAP_ACHV_STATE_ALL_CHAR::const_iterator c_achv_state_all_char_itor = m_kMapAchvStateAllChar.find(lCSN);
	if (c_achv_state_all_char_itor == m_kMapAchvStateAllChar.end())
	{
		iLog().Put(ERR_UK, "[achv] Bureau::IsComplete -----> cannot found achievement info!!! CSN: ", lCSN);
		return FALSE;
	}

	MAP_ACHV_STATE kMapAchvState = c_achv_state_all_char_itor->second;
	MAP_ACHV_STATE::const_iterator c_achv_state_itor = kMapAchvState.find(lAchvID);
	if (c_achv_state_itor != kMapAchvState.end())
	{
		if (G_INVALID_DATE != c_achv_state_itor->second.strCompleteDate) // 이미 달성한 업적
		{
			return TRUE;
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------
// Name:GetProgress()
// Dec: 업적 진행 카운트 얻어오는 함수
//--------------------------------------------------------------------
double CAchvMgr::GetProgress(const LONG lCSN, const LONG lAchvID)
{
	MAP_ACHV_STATE_ALL_CHAR::const_iterator c_achv_state_all_char_itor = m_kMapAchvStateAllChar.find(lCSN);
	if (c_achv_state_all_char_itor == m_kMapAchvStateAllChar.end())
	{
		iLog().Put(ERR_UK, "NAS_SERVICE_ERR, -----> cannot found achievement info!!! CSN: ", lCSN);
		return 0.0;
	}

	MAP_ACHV_STATE kMapAchvState = c_achv_state_all_char_itor->second;
	MAP_ACHV_STATE::const_iterator c_achv_state_itor = kMapAchvState.find(lAchvID);
	if (c_achv_state_itor != kMapAchvState.end())
	{
		return c_achv_state_itor->second.dProgress;
	}

	return 0.0;
}

//--------------------------------------------------------------------
// Name:CheckRelation()
// Dec: 연계업적을 달성하였는지 체크한다.
//--------------------------------------------------------------------
BOOL CAchvMgr::CheckRelation(const LONG lCSN, const LONG lParentID)
{
	vector< LONG > vecChild;
	pair< MAP_DEF_ACHV::const_iterator, MAP_DEF_ACHV::const_iterator > pairRange;		// 연계 업적은 항상 AE_ACHV_COMPLETE 로 검색해야 함
	pairRange = m_kMapDefAchv.equal_range(AE_ACHV_COMPLETE); 
	if (pairRange.first != pairRange.second)
	{
		MAP_DEF_ACHV::const_iterator itor = pairRange.first;
		while (pairRange.second != itor)
		{
			SAchvDef achvDef = itor->second;
			if (achvDef.lAchvID == lParentID)
			{
				vecChild = achvDef.vecRelativeChild;
				break;
			}
			else
				++itor;
		}
	}

	if (vecChild.size() <= 0)
		return FALSE;

	ForEachElmt(vector< LONG >, vecChild, it, ij)
	{
		if (!IsComplete(lCSN, (*it)))
			return FALSE;
	}
	return TRUE;
}

//--------------------------------------------------------------------
// Name:UpdateAchvState()
// Dec: 업적의 모든 factor들을 만족하거나 업적을 달성하면 DB와 메모리 업데이트
//--------------------------------------------------------------------
void CAchvMgr::UpdateAchvState(const LONG lGSN, const LONG lCSN, const LONG lAchvID, const double dProgress, const bool isCompleted, const RoomID& roomID, const LONG lAlwaysNoti)
{
	GCSLOCK lock(&gcsCSNMap_);
	string strOutLastUpdateTime;

	if (!theNFDBMgr.UpdateAchvProgress(strOutLastUpdateTime, lGSN, lCSN, lAchvID, dProgress, isCompleted))
	{
		iLog().Put(ERR_UK, "DB_ERR, -----> theNFDBMgr.UpdateAchvProgress fail!!!");
		return;
	}

	MAP_ACHV_STATE_ALL_CHAR::iterator achv_state_all_char_itor = m_kMapAchvStateAllChar.find(lCSN);
	if (achv_state_all_char_itor == m_kMapAchvStateAllChar.end())
	{
		iLog().Put(ERR_UK, "NAS_SERVICE_ERR, -----> cannot found achievement info!!! CSN: ", lCSN);
		return;
	}

	MAP_ACHV_STATE& kMapAchvState = achv_state_all_char_itor->second;
	MAP_ACHV_STATE::iterator achv_state_itor = kMapAchvState.find(lAchvID);
	if (achv_state_itor != kMapAchvState.end()) 
	{
		(*achv_state_itor).second.strLastUpdateDate = strOutLastUpdateTime;
		(*achv_state_itor).second.dProgress = dProgress;
		if (isCompleted)
			(*achv_state_itor).second.strCompleteDate = strOutLastUpdateTime;
	}
	else
	{
		SAchvState achv_state;
		achv_state.lAchvID = lAchvID;
		achv_state.strLastUpdateDate = strOutLastUpdateTime;
		achv_state.dProgress = dProgress;
		if (isCompleted)
			achv_state.strCompleteDate = strOutLastUpdateTime;

		achv_state_all_char_itor->second.insert(make_pair(lAchvID, achv_state));
	}


	EventItem_T	evt(dProgress, roomID);
	if (isCompleted)
		evt.result = PR_COMPLETED;
	else
	{
		if (lAlwaysNoti <= dProgress)
			evt.result = PR_CHANGED_NOTI;
		else
			evt.result = PR_CHANGED;					
	}

	runReportCallback(lGSN, lCSN, lAchvID, &evt);
}

//--------------------------------------------------------------------
// Name:LoadAchvXML()
// Dec: Achv.xml을 파싱하여 m_kMapDefAchv 컨테이너에 넣는다.
//--------------------------------------------------------------------
BOOL CAchvMgr::LoadAchvXML()
{
	// NAS.exe와 같은 폴더에서 Achv.xml을 읽는다.
	wchar_t szModuleFullPath[MAX_PATH];
	::GetModuleFileNameW( GetModuleHandle(0), szModuleFullPath, MAX_PATH );
	
	wstring kTemp = szModuleFullPath;
	kTemp = kTemp.substr(0, kTemp.rfind(L"\\"));
	kTemp += L"\\Achv.xml";

	string kPath;
	kPath.assign(kTemp.begin(), kTemp.end());

	TiXmlDocument doc(kPath.c_str());
	if (!doc.LoadFile() )
		return FALSE;

	TiXmlElement* pRoot = doc.FirstChildElement("NFGAME");
	if (!pRoot)
	{
		iLog().Put(ERR_UK, "LoadAchvXML, -----> not found root!!!");
		return FALSE;
	}

	TiXmlElement* pAchvElement = pRoot->FirstChildElement("ACHV");

	// <ACHV>
	while(pAchvElement)
	{
		SAchvDef achv;

		achv.lAchvID = atoi(pAchvElement->Attribute("ID"));
		achv.kCategory = pAchvElement->Attribute("CATEGORY");
		achv.kName = pAchvElement->Attribute("NAME");
		achv.lEvent = atoi(pAchvElement->Attribute("EVENT"));
		achv.lNoticeLevel = atoi(pAchvElement->Attribute("NOTICE_LEVEL"));
		achv.bIsHidden = (atoi(pAchvElement->Attribute("IS_HIDDEN")) == 1) ? true : false;

		// 연계업적: 상위업적의 업적ID
		const char* pTemp = pAchvElement->Attribute("ALWAYS_NOTI");
		if (pTemp)
		{
			achv.lAlways_noti = atoi(pTemp);
		}

		// 연계업적: 상위업적의 업적ID
		pTemp = pAchvElement->Attribute("RELATIVE_PARENT");
		if (pTemp)
		{
			achv.lRelativeParent = atoi(pTemp);
		}

		// 연계업적: 하위업적들의 업적ID
		pTemp = pAchvElement->Attribute("RELATIVE_CHILD");
		if (pTemp)
		{
			vector< string > kOut;
			BreakSeparator(string(pTemp), kOut, "/");

			vector< string >::iterator itor = kOut.begin();
			while (kOut.end() != itor)
			{
				LONG lVal = atoi((*itor).c_str());
				achv.vecRelativeChild.push_back(lVal);
				++itor;
			}
		}

		// 연속업적의 Group ID
		pTemp = pAchvElement->Attribute("SEQUENCE_GROUP");
		if (pTemp)
		{
			achv.lSequenceGroup = atoi(pTemp);
			achv.lSequenceOrder = atoi(pAchvElement->Attribute("SEQUENCE_ORDER")); // Group ID가 존재하면 Order 존재
		}

		// <REWARD>
		TiXmlElement* pRewardElement = pAchvElement->FirstChildElement("REWARD");
		if(pRewardElement)
		{
			achv.lPoint = atoi(pRewardElement->Attribute("POINT"));
			achv.lExp = atoi(pRewardElement->Attribute("EXP"));
			achv.lMoney = atoi(pRewardElement->Attribute("MONEY"));
			achv.lItemSelectCount = atoi(pRewardElement->Attribute("ITEM_SELECT_COUNT"));

			// <ITEM>
			TiXmlElement* pItemElement = pRewardElement->FirstChildElement("ITEM");
			while (pItemElement)
			{
				LONG lItemID = atoi(pAchvElement->Attribute("ID"));
				achv.vecRewardItem.push_back(lItemID);

				pItemElement = pItemElement->NextSiblingElement();
			}
		}

		// <CONDITION>
		TiXmlElement* pConditionElement = pAchvElement->FirstChildElement("CONDITION");
		if (pConditionElement)
		{
			achv.lGoal = atoi(pConditionElement->Attribute("GOAL"));

			// <FACTOR>
			TiXmlElement* pFactorElement = pConditionElement->FirstChildElement("FACTOR");
			while (pFactorElement)
			{
				SAchvFactor factor;

				factor.kType = pFactorElement->Attribute("TYPE");

				string kCompareType = pFactorElement->Attribute("COMPARE");
				if (COMPARE_TYPE_EQUAL == kCompareType)
				{
					factor.lCompareType = ECT_EQUAL;
				}
				else if (COMPARE_TYPE_GREATER == kCompareType)
				{
					factor.lCompareType = ECT_GREATER;
				}
				else if (COMPARE_TYPE_LESS == kCompareType)
				{
					factor.lCompareType = ECT_LESS;
				}
				else if (COMPARE_TYPE_RANGE == kCompareType.c_str())
				{
					factor.lCompareType = ECT_RANGE;
				}
				else
				{
					factor.lCompareType = ECT_NIL;
				}

				pTemp = pFactorElement->Attribute("VALUE");
				if (pTemp)
				{
					factor.lValue = atoi(pTemp);
				}

				pTemp = pFactorElement->Attribute("MIN_VALUE");
				if (pTemp)
				{
					factor.lMinValue = atoi(pTemp);
				}

				pTemp = pFactorElement->Attribute("MAX_VALUE");
				if (pTemp)
				{
					factor.lMaxValue = atoi(pTemp);
				}

				pTemp = pFactorElement->Attribute("IS_UPDATE_VALUE");
				if (pTemp)
				{
					if (atoi(pTemp) == 1)
						factor.bUpdateValue = TRUE;
				}

				achv.mapFactor.insert(make_pair(factor.kType, factor));

				pFactorElement = pFactorElement->NextSiblingElement();
			}
		}

		m_kMapDefAchv.insert(make_pair(achv.lEvent, achv));

		pAchvElement = pAchvElement->NextSiblingElement();
	}

	ForEachElmt(MAP_DEF_ACHV, m_kMapDefAchv, it, ij) 
	{
		iLog().Put(DEV_UK, "LoadXML, -----> [achv_ID] unknown compare type event : ", (*it).first, ", achv_id :", (*it).second.lAchvID);
	}
	

	return TRUE;
}

//--------------------------------------------------------------------
// Name:CheckAchv()
// Dec: 업적 조건을 만족하는지, 달성되었는지 체크하는 함수
//--------------------------------------------------------------------
void CAchvMgr::CheckAchv(const LONG lGSN, const LONG lCSN, const LONG lEvent, const RoomID& roomID, const map< string, LONG >& mapFactorVal)
{
	GCSLOCK lock(&gcsCSNMap_);
	pair< MAP_DEF_ACHV::const_iterator, MAP_DEF_ACHV::const_iterator > pairRange;
	pairRange = m_kMapDefAchv.equal_range(lEvent); // 해당 이벤트값을 가지는 업적의 시작(first)과 끝(second)
	if (pairRange.first != pairRange.second)
	{
		MAP_DEF_ACHV::const_iterator itor = pairRange.first;
		while (pairRange.second != itor)
		{
			SAchvDef achvDef = itor->second;

			// 이미 완료된 업적
			if (IsComplete(lCSN, achvDef.lAchvID))
			{
				++itor;
				continue;
			}
			
			LONG lAddValue = 1; // 모든 factor 만족 시 ProgressValue에 더해질 값(기본 +1)
			bool bIsSatisfy = false;

			if (achvDef.mapFactor.size()) // 체크해야 할 factor가 있다.
			{
				size_t factor_success_count = 0;

				map< string, SAchvFactor >::const_iterator factor_def_itor = achvDef.mapFactor.begin();
				while (factor_def_itor != achvDef.mapFactor.end()) // XML에 정의된 해당 업적의 factors...
				{
					SAchvFactor achvDefFactor = factor_def_itor->second;
					map< string, LONG >::const_iterator factor_val_itor = mapFactorVal.find(achvDefFactor.kType);
					if (factor_val_itor == mapFactorVal.end())
					{	
						break;
					}

					LONG lCheckVal = factor_val_itor->second;

					if (achvDefFactor.bUpdateValue)
					{
						lAddValue = lCheckVal;
					}

					// 비교타입에 따른 factor 만족
					if (ECT_EQUAL == achvDefFactor.lCompareType)
					{
						if (lCheckVal != achvDefFactor.lValue)
						{	
							break;
						}
					}
					else if (ECT_GREATER == achvDefFactor.lCompareType)
					{
						if (lCheckVal <= achvDefFactor.lValue)
						{	
							break;
						}
					}
					else if (ECT_LESS == achvDefFactor.lCompareType)
					{
						if (lCheckVal >= achvDefFactor.lValue)
						{
							break;
						}
					}
					else if (ECT_RANGE == achvDefFactor.lCompareType) // 범위안에 들어야 성공
					{
						if (lCheckVal < achvDefFactor.lMinValue || lCheckVal > achvDefFactor.lMaxValue)
						{
							break;
						}
					}
					else
					{
						iLog().Put(ERR_UK, "CheckAchv, -----> [CheckAchv] unknown compare type: ", achvDefFactor.lCompareType);
					}

					++factor_success_count;
					++factor_def_itor;
				}

				if (achvDef.mapFactor.size() == factor_success_count) // OK. factor 모두 만족
				{
					bIsSatisfy = true;
				}
			}
			else // 체크해야 할 factor가 없다 == factor 모두 만족
			{
				bIsSatisfy = true;
			}

			if (bIsSatisfy)
			{
				double dCurrentProgress = GetProgress(lCSN, achvDef.lAchvID);
				double dUpdateProgress = dCurrentProgress + lAddValue;
				bool bIsCompleted = (achvDef.lGoal <= dUpdateProgress); // 업적 달성 여부

				// 업적 갱신
				UpdateAchvState(lGSN, lCSN, achvDef.lAchvID, dUpdateProgress, bIsCompleted, roomID, achvDef.lAlways_noti);

				// 연계업적 체크
				if (bIsCompleted && CheckRelation(lCSN, achvDef.lRelativeParent))
				{
					UpdateAchvState(lGSN, lCSN, achvDef.lRelativeParent, 1, true, roomID, 0);
				}
			}

			++itor;
		}
	}
}



//--------------------------------------------------------------------
// Name:LoadCharAchvState()
// Dec: 유저가 로그인 할 때 호출
//		유저의 업적 정보를 로드한다.
//--------------------------------------------------------------------
bool CAchvMgr::LoadCharAchvState(const LONG lGSN, const LONG lCSN)
{
	GCSLOCK lock(&gcsCSNMap_);
	MAP_ACHV_STATE_ALL_CHAR::const_iterator itor = m_kMapAchvStateAllChar.find(lCSN);
	if (itor == m_kMapAchvStateAllChar.end())
	{
		MAP_ACHV_STATE mapAchvState;
		if (!theNFDBMgr.SelectNFAchvByChar(lGSN, lCSN, mapAchvState))
		{
			iLog().Put(ERR_UK, "LoadCharAchvState, -----> theNFDBMgr.SelectNFAchvByChar fail!! CSN: ", lCSN);
			return false;
		}

		m_kMapAchvStateAllChar.insert(make_pair(lCSN, mapAchvState));
	}

	return true;
}


bool CAchvMgr::login(const LONG lGSN, const LONG lCSN)
{
	return LoadCharAchvState(lGSN, lCSN);
}

bool CAchvMgr::logout(const LONG lCSN)
{
	GCSLOCK lock(&gcsCSNMap_);
	MAP_ACHV_STATE_ALL_CHAR::iterator iter = m_kMapAchvStateAllChar.find(lCSN);
	if (iter == m_kMapAchvStateAllChar.end())
		return false;
	
	m_kMapAchvStateAllChar.erase(iter);
	return true;
}

bool CAchvMgr::addReportCallback(ReportCallback cbReport)
{
	if (cbReport == NULL)
		return false;

	GCSLOCK lock(&gcsCallBack);

	m_cbReportVector.push_back(cbReport);

	return true;
}

bool CAchvMgr::runReportCallback(LONG lGSN, LONG lCSN, int achv_ID, const EventItem_T *pEvtItem)
{
	GCSLOCK lock(&gcsCallBack);

	(*m_cbReportVector[0])(lGSN, lCSN, achv_ID, pEvtItem);

	return true;
}

bool CAchvMgr::getedRewardItem(const LONG lCSN, int achv_ID)
{
	GCSLOCK lock(&gcsCSNMap_);
	MAP_ACHV_STATE_ALL_CHAR::const_iterator c_all_char_iter = m_kMapAchvStateAllChar.find(lCSN);
	if (c_all_char_iter == m_kMapAchvStateAllChar.end())
		return false;		// not_found_csn
	
	MAP_ACHV_STATE::const_iterator c_char_state_iter = (*c_all_char_iter).second.find(achv_ID);
	if (c_char_state_iter == (*c_all_char_iter).second.end())
		return false;		// not_found_achv_ID

	const SAchvState& achv_state = (*c_char_state_iter).second;
	if (achv_state.strRewardDate != G_INVALID_DATE)
		return false;		// 이미 받았다...

	return true;
}

bool CAchvMgr::getRewardItem(const LONG lCSN, int achv_ID, const std::string strRewardDate, int nRewardItemID[])
{
	GCSLOCK lock(&gcsCSNMap_);

	MAP_ACHV_STATE_ALL_CHAR::iterator all_char_iter = m_kMapAchvStateAllChar.find(lCSN);
	if (all_char_iter == m_kMapAchvStateAllChar.end())
		return false;		// not_found_csn

	MAP_ACHV_STATE::iterator char_state_iter = (*all_char_iter).second.find(achv_ID);
	if (char_state_iter == (*all_char_iter).second.end())
		return false;		// not_found_achv_ID

	SAchvState& achv_state = (*char_state_iter).second;
	if (achv_state.strRewardDate != G_INVALID_DATE)
		return false;		// 이미 받았다...

	achv_state.lItemID1 = nRewardItemID[0];
	achv_state.lItemID2 = nRewardItemID[1];
	achv_state.lItemID3 = nRewardItemID[2];
	achv_state.lItemID4 = nRewardItemID[3];
	achv_state.strRewardDate = strRewardDate;
	return true;
}