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
		iLog().Put(ERR_UK, "[achv] Bureau::IsComplete -----> cannot found achievement info!!! CSN: ", lCSN);
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
// Dec: ��������� �޼��Ͽ����� üũ�Ѵ�.
//--------------------------------------------------------------------
BOOL CAchvMgr::CheckRelation(const LONG lCSN, const LONG lParentID)
{
	vector< LONG > vecChild;
	pair< MAP_DEF_ACHV::const_iterator, MAP_DEF_ACHV::const_iterator > pairRange;		// ���� ������ �׻� AE_ACHV_COMPLETE �� �˻��ؾ� ��
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
// Dec: ������ ��� factor���� �����ϰų� ������ �޼��ϸ� DB�� �޸� ������Ʈ
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
// Dec: Achv.xml�� �Ľ��Ͽ� m_kMapDefAchv �����̳ʿ� �ִ´�.
//--------------------------------------------------------------------
BOOL CAchvMgr::LoadAchvXML()
{
	// NAS.exe�� ���� �������� Achv.xml�� �д´�.
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

		// �������: ���������� ����ID
		const char* pTemp = pAchvElement->Attribute("ALWAYS_NOTI");
		if (pTemp)
		{
			achv.lAlways_noti = atoi(pTemp);
		}

		// �������: ���������� ����ID
		pTemp = pAchvElement->Attribute("RELATIVE_PARENT");
		if (pTemp)
		{
			achv.lRelativeParent = atoi(pTemp);
		}

		// �������: ������������ ����ID
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
// Dec: ���� ������ �����ϴ���, �޼��Ǿ����� üũ�ϴ� �Լ�
//--------------------------------------------------------------------
void CAchvMgr::CheckAchv(const LONG lGSN, const LONG lCSN, const LONG lEvent, const RoomID& roomID, const map< string, LONG >& mapFactorVal)
{
	GCSLOCK lock(&gcsCSNMap_);
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
					else if (ECT_RANGE == achvDefFactor.lCompareType) // �����ȿ� ���� ����
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
				UpdateAchvState(lGSN, lCSN, achvDef.lAchvID, dUpdateProgress, bIsCompleted, roomID, achvDef.lAlways_noti);

				// ������� üũ
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
// Dec: ������ �α��� �� �� ȣ��
//		������ ���� ������ �ε��Ѵ�.
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
		return false;		// �̹� �޾Ҵ�...

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
		return false;		// �̹� �޾Ҵ�...

	achv_state.lItemID1 = nRewardItemID[0];
	achv_state.lItemID2 = nRewardItemID[1];
	achv_state.lItemID3 = nRewardItemID[2];
	achv_state.lItemID4 = nRewardItemID[3];
	achv_state.strRewardDate = strRewardDate;
	return true;
}