#include "stdafx.h"
#include "GameStatTable.h"

GameStatTable theGameStatTable;
GameStatTable::GameStatTable(void)
{
}

GameStatTable::~GameStatTable(void)
{
}

void GameStatTable::OnUpdateCHSStatInfo(const ChannelUpdateInfoList& list)
{
	ForEachCElmt(ChannelUpdateInfoList, list, i1, j1)
	{
		const ChannelUpdateInfo& chinfo = (*i1);
		SetCHSStatInfo(chinfo.m_channelID.m_lSSN, chinfo.m_channelID, chinfo.m_lUserCount);
	}
}

void GameStatTable::OnUpdateGLSStatInfo(const LONG& lSSN, const LRBAddress& address, const LONG& lUserCount)
{
	SetGLSStatInfo(lSSN, address, lUserCount);
}


void GameStatTable::SetCHSStatInfo(const LONG& lSSN, const ChannelID& channelID, const LONG& lUserCount)
{
	// CHS 내 사용자 총수 설정
	CHSStatList* pList = NULL;
	bool bFind = false;
	GCSLOCK lo(&m_csCHSLock);
	ITER_CHSSTATREPO it_pos = m_chsStatRepo.find(lSSN);
	if (it_pos != m_chsStatRepo.end())
	{
		pList = it_pos->second;
		if (pList)
		{
			ITER_CHSSTAT it_info = pList->begin();
			while (it_info != pList->end())
			{
				if ((*it_info)->GetChannelD() == channelID)
				{
					(*it_info)->SetUserCount(lUserCount);
					bFind = true;
					break;
				}
				++it_info;
			}
		}
	}
	else
	{
		pList = new CHSStatList;
		if (!pList)
		{
			return;
		}
		m_chsStatRepo.insert(make_pair(lSSN, pList));
	}

	if (!bFind)
	{
		if (pList)
		{
			pList->push_back(new CHSStatInfo(channelID, lUserCount));
		}
	}
}

void GameStatTable::SetGLSStatInfo(const LONG& lSSN, const LRBAddress& address, const LONG& lUserCount)
{
	// GLS 내 사용자 총수 설정
	GLSStatList* pList = NULL;
	bool bFind = false;
	GCSLOCK lo(&m_csGLSLock);
	ITER_GLSSTATREPO it_pos = m_glsStatRepo.find(lSSN);
	if (it_pos != m_glsStatRepo.end())
	{
		pList = it_pos->second;
		if (pList)
		{
			ITER_GLSSTAT it_info = pList->begin();
			while (it_info != pList->end())
			{
				if ((*it_info)->GetAddress() == address)
				{
					(*it_info)->SetUserCount(lUserCount);
					bFind = true;
					break;
				}
				++it_info;
			}
		}
	}
	else
	{
		pList = new GLSStatList;
		if (!pList)
		{
			return;
		}
		m_glsStatRepo.insert(make_pair(lSSN, pList));
	}

	if (!bFind)
	{
		if (pList)
		{
			pList->push_back(new GLSStatInfo(address, lUserCount));
		}
	}
}

void GameStatTable::SetGLSStatInfo(const LRBAddress& address, const LONG& lUserCount)
{
	GCSLOCK lo(&m_csGLSLock);
	ITER_GLSSTATREPO it_pos = m_glsStatRepo.begin();
	while (it_pos != m_glsStatRepo.end())
	{
		GLSStatList* pList = it_pos->second;
		if (pList)
		{
			ITER_GLSSTAT it_info = pList->begin();
			while (it_info != pList->end())
			{
				if ((*it_info)->GetAddress() == address)
				{
					(*it_info)->SetUserCount(lUserCount);
				}
				++it_info;
			}
		}
		++it_pos;
	}
}

const LONG GameStatTable::GetTotalUserCount(const LONG& lSSN)
{
	LONG totalUserCount = 0;
	{
		// CHS 내 사용자 총수 합산
		GCSLOCK lo(&m_csCHSLock);
		ITER_CHSSTATREPO it_pos = m_chsStatRepo.find(lSSN);
		if (it_pos != m_chsStatRepo.end())
		{
			CHSStatList* pList = it_pos->second;
			if (pList)
			{
				ITER_CHSSTAT it_info = pList->begin();
				while (it_info != pList->end())
				{
					totalUserCount += (*it_info)->GetUserCount();
					++it_info;
				}
			}
		}
	}
	return totalUserCount;
}

void GameStatTable::OnShowChannelUserCount(const LONG& lSSN, const DWORD& dwCategoryNum)
{
	LONG totalUserCount = 0;
	LONG channelCount = 0;
	{
		// CHS 내 사용자 총수 합산
		GCSLOCK lo(&m_csCHSLock);
		ITER_CHSSTATREPO it_pos = m_chsStatRepo.find(lSSN);
		if (it_pos != m_chsStatRepo.end())
		{
			CHSStatList* pList = it_pos->second;
			if (pList)
			{
				ITER_CHSSTAT it_info = pList->begin();
				while (it_info != pList->end())
				{
					if ((*it_info)->GetCategoryNumber() == dwCategoryNum)
					{
						string channelID;
						(*it_info)->GetChannelD().GetInstanceID(channelID);
						LONG count = (*it_info)->GetUserCount();
						LOG (INF, "User Count in Channel [CHANNELID : ", channelID, "][COUNT : ", count, "]");
						totalUserCount += count;
						++channelCount;
					}
					++it_info;
				}
			}
		}
	}
	LOG (INF, "Total User Count in Channel [SSN : ", lSSN, "][CATEGORY : ", dwCategoryNum, "][CHANNELCOUNT : ", channelCount, "][TOTALUSER : ", totalUserCount, "]");
}

void GameStatTable::OnDestroyCHS(const ChannelID& channelID)
{
	SetCHSStatInfo(channelID.m_lSSN, channelID, 0);
}

void GameStatTable::OnDestroyGLS(const LRBAddress& address)
{
	SetGLSStatInfo(address);
}
