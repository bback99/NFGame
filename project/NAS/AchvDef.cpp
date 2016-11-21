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
// Dec: ������(Separator)�� �������� string�� �и��ϴ� �Լ�
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
// Dec: �̹� �Ϸ�� �������� üũ�ϴ� �Լ�
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
		if (G_INVALID_DATE != c_achv_state_itor->second.strCompleteDate) // �̹� �޼��� ����
		{
			return TRUE;
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------
// Name:GetProgress()
// Dec: ���� ���� ī��Ʈ ������ �Լ�
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
// Dec: ���������� �޼��Ͽ����� üũ�Ѵ�.
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
// Dec: ������ ��� factor���� �����ϰų� ������ �޼��ϸ� DB�� �޸� ������Ʈ
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
// Dec: Achv.xml�� �Ľ��Ͽ� m_kMapDefAchv �����̳ʿ� �ִ´�.
//--------------------------------------------------------------------
BOOL CAchvMgr::LoadAchvXML()
{
	// NAS.exe�� ���� �������� Achv.xml�� �д´�.
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

		// ��������: ���������� ����ID
		const char* pTemp = pAchvElement->Attribute("RELATIVE_PARENT");
		if (pTemp)
		{
			achv.lRelativeParent = atoi(pTemp);
		}

		// ��������: ������������ ����ID
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

		// ���Ӿ����� Group ID
		pTemp = pAchvElement->Attribute("SEQUENCE_GROUP");
		if (pTemp)
		{
			achv.lSequenceGroup = atoi(pTemp);
			achv.lSequenceOrder = atoi(pAchvElement->Attribute("SEQUENCE_ORDER")); // Group ID�� �����ϸ� Order ����
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
// Dec: ���� ������ �����ϴ���, �޼��Ǿ����� üũ�ϴ� �Լ�
//--------------------------------------------------------------------
void CAchvMgr::CheckAchv(const LONG lGSN, const LONG lCSN, const LONG lEvent, const map< string, LONG >& mapFactorVal)
{
	pair< MAP_DEF_ACHV::const_iterator, MAP_DEF_ACHV::const_iterator > pairRange;
	pairRange = m_kMapDefAchv.equal_range(lEvent); // �ش� �̺�Ʈ���� ������ ������ ����(first)�� ��(second)
	if (pairRange.first != pairRange.second)
	{
		MAP_DEF_ACHV::const_iterator itor = pairRange.first;
		while (pairRange.second != itor)
		{
			SAchvDef achvDef = itor->second;

			// �̹� �Ϸ�� ����
			if (IsComplete(lCSN, achvDef.lAchvID))
			{
				++itor;
				continue;
			}
			
			LONG lAddValue = 1; // ��� factor ���� �� ProgressValue�� ������ ��(�⺻ +1)
			bool bIsSatisfy = false;

			if (achvDef.mapFactor.size()) // üũ�ؾ� �� factor�� �ִ�.
			{
				size_t factor_success_count = 0;

				map< string, SAchvFactor >::const_iterator factor_def_itor = achvDef.mapFactor.begin();
				while (factor_def_itor != achvDef.mapFactor.end()) // XML�� ���ǵ� �ش� ������ factors...
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

					// ��Ÿ�Կ� ���� factor ����
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
					else if (ECT_RANGE == achvDefFactor.lCompareType) // �����ȿ� ���� ����
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

				if (achvDef.mapFactor.size() == factor_success_count) // OK. factor ��� ����
				{
					bIsSatisfy = true;
				}
			}
			else // üũ�ؾ� �� factor�� ���� == factor ��� ����
			{
				bIsSatisfy = true;
			}

			if (bIsSatisfy)
			{
				double dCurrentProgress = GetProgress(lCSN, achvDef.lAchvID);
				double dUpdateProgress = dCurrentProgress + lAddValue;
				bool bIsCompleted = (achvDef.lGoal <= dUpdateProgress); // ���� �޼� ����

				// ���� ����
				UpdateAchvState(lGSN, lCSN, achvDef.lAchvID, dUpdateProgress, bIsCompleted);

				// �������� üũ
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
// Dec: ������ �α��� �� �� ȣ��
//		������ ���� ������ �ε��Ѵ�.
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