#include "stdafx.h"
#include "AchvDef.h"
#include "TinyXML/tinyxml.h"
#include <NFVariant/NFDBManager.h>

const string COMPARE_TYPE_GREATER = "GREATER";
const string COMPARE_TYPE_EQUAL = "EQUAL";
const string COMPARE_TYPE_LESS = "LESS";
const string COMPARE_TYPE_RANGE = "RANGE";

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
		LOG(ERR_UK, "NAS_SERVICE_ERR, -----> cannot found achievement info!!! CSN: ", lCSN);
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
		LOG(ERR_UK, "NAS_SERVICE_ERR, -----> cannot found achievement info!!! CSN: ", lCSN);
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
// Dec: 연관업적을 달성하였는지 체크한다.
//--------------------------------------------------------------------
BOOL CAchvMgr::CheckRelation(const LONG lCSN, const LONG lParentID)
{
	MAP_DEF_ACHV::const_iterator parent_itor = m_kMapDefAchv.find(AE_ACHV_COMPLETE);
	if (parent_itor == m_kMapDefAchv.end())
	{
		return FALSE;
	}

	vector< LONG >::const_iterator child_itor = parent_itor->second.vecRelativeChild.begin();
	while (child_itor != parent_itor->second.vecRelativeChild.end())
	{
		if (!IsComplete(lCSN, (*child_itor)))
		{
			return FALSE;
		}

		++child_itor;
	}

	return TRUE;
}

//--------------------------------------------------------------------
// Name:UpdateAchvState()
// Dec: 업적의 모든 factor들을 만족하거나 업적을 달성하면 DB와 메모리 업데이트
//--------------------------------------------------------------------
void CAchvMgr::UpdateAchvState(const LONG lGSN, const LONG lCSN, const LONG lAchvID, const double dProgress, const bool isCompleted)
{
	string strOutLastUpdateTime;

	if (!theNFDBMgr.UpdateAchvProgress(strOutLastUpdateTime, lGSN, lCSN, lAchvID, dProgress, isCompleted))
	{
		LOG(ERR_UK, "DB_ERR, -----> theNFDBMgr.UpdateAchvProgress fail!!!");
		return;
	}

	MAP_ACHV_STATE_ALL_CHAR::const_iterator c_achv_state_all_char_itor = m_kMapAchvStateAllChar.find(lCSN);
	if (c_achv_state_all_char_itor == m_kMapAchvStateAllChar.end())
	{
		LOG(ERR_UK, "NAS_SERVICE_ERR, -----> cannot found achievement info!!! CSN: ", lCSN);
		return;
	}

	MAP_ACHV_STATE kMapAchvState = c_achv_state_all_char_itor->second;
	MAP_ACHV_STATE::iterator achv_state_itor = kMapAchvState.find(lAchvID);
	if (achv_state_itor != kMapAchvState.end())
	{
		achv_state_itor->second.dProgress = dProgress;
		achv_state_itor->second.strLastUpdateDate = strOutLastUpdateTime;

		if (isCompleted)
		{
			achv_state_itor->second.strCompleteDate = strOutLastUpdateTime;
		}
	}
}

//--------------------------------------------------------------------
// Name:LoadAchvXML()
// Dec: Achv.xml을 파싱하여 m_kMapDefAchv 컨테이너에 넣는다.
//--------------------------------------------------------------------
BOOL CAchvMgr::LoadAchvXML()
{
	// NAS.exe와 같은 폴더에서 Achv.xml을 읽는다.
	TCHAR szModuleFullPath[MAX_PATH];
	::GetModuleFileName( GetModuleHandle(0), szModuleFullPath, MAX_PATH );
	
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
		cout << "not found root" << endl;
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

		// 연관업적: 상위업적의 업적ID
		const char* pTemp = pAchvElement->Attribute("RELATIVE_PARENT");
		if (pTemp)
		{
			achv.lRelativeParent = atoi(pTemp);
		}

		// 연관업적: 하위업적들의 업적ID
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

				if (pFactorElement->Attribute("IS_UPDATE_VALUE"))
				{
					factor.bUpdateValue = TRUE;
				}

				achv.mapFactor.insert(make_pair(factor.kType, factor));

				pFactorElement = pFactorElement->NextSiblingElement();
			}
		}

		m_kMapDefAchv.insert(make_pair(achv.lEvent, achv));

		pAchvElement = pAchvElement->NextSiblingElement();
	}

	return TRUE;
}

//--------------------------------------------------------------------
// Name:CheckAchv()
// Dec: 업적 조건을 만족하는지, 달성되었는지 체크하는 함수
//--------------------------------------------------------------------
void CAchvMgr::CheckAchv(const LONG lGSN, const LONG lCSN, const LONG lEvent, const map< string, LONG >& mapFactorVal)
{
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
						if (lCheckVal < achvDefFactor.lValue)
						{	
							break;
						}
					}
					else if (ECT_LESS == achvDefFactor.lCompareType)
					{
						if (lCheckVal > achvDefFactor.lValue)
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
						LOG(ERR_UK, "NAS_SERVICE_ERR, -----> [CheckAchv] unknown compare type: ", achvDefFactor.lCompareType);
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
				UpdateAchvState(lGSN, lCSN, achvDef.lAchvID, dUpdateProgress, bIsCompleted);

				// 연관업적 체크
				if (bIsCompleted && CheckRelation(lCSN, achvDef.lRelativeParent))
				{
					UpdateAchvState(lGSN, lCSN, achvDef.lRelativeParent, 1, true);
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
BOOL CAchvMgr::LoadCharAchvState(const LONG lGSN, const LONG lCSN)
{
	MAP_ACHV_STATE_ALL_CHAR::const_iterator itor = m_kMapAchvStateAllChar.find(lCSN);
	if (itor == m_kMapAchvStateAllChar.end())
	{
		MAP_ACHV_STATE mapAchvState;
		if (!theNFDBMgr.SelectNFAchvByChar(lGSN, lCSN, mapAchvState))
		{
			LOG(ERR_UK, "NAS_SERVICE_ERR, -----> theNFDBMgr.SelectNFAchvByChar fail!! CSN: ", lCSN);
			return FALSE;
		}

		m_kMapAchvStateAllChar.insert(make_pair(lCSN, mapAchvState));
	}

	return TRUE;
}