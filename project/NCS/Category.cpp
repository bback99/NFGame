// 
// Category.cpp
// 

#include "stdafx.h"
#include "GChannel.h"
#include "Category.h"
#include "ChannelSvrTable.h"
#include "LRBHandler.h"

ServiceTable	theServiceTable;
/////////////////////////////////////////////////////////////////////////////
// Category 
/////////////////////////////////////////////////////////////////////////////
// Category Construction & Destruction
Category::Category()
{
	m_channelPrefix.m_lSSN = 0L;
}

Category::Category(const ChannelPrefix& channelPrefix) : m_channelPrefix(channelPrefix)
{
	m_channelPrefix = channelPrefix;
}

Category::Category(LONG lSSN, DWORD dwCategoryNum)
{
	m_channelPrefix.m_lSSN = lSSN;
	m_channelPrefix.m_dwCategory = dwCategoryNum;
}
Category::~Category()
{
	m_set.flush();
}
/////////////////////////////////////////////////////////////////////////////
void Category::Clear()
{
	m_set.flush();
}
BOOL Category::AddGameChannel(GameChannel* pGameChannel)
{
	BOOL bRet = (FindGameChannel(pGameChannel->m_channelID.m_dwGCIID) == NULL);
	if(bRet)
	{
		m_set.add(pGameChannel);
	}
	return bRet;
}
BOOL Category::GetBalancedChannelList(ChannelInfoList& channelInfoList, LONG lRequiredChannelCount, LONG& lUserCount, LONG& lChannelCount)
{
	GBuf buf;
	{
		ChannelInfoList lst;
		//long lAvaliableCount = 10L;
		long lAvaliableCount = lRequiredChannelCount;
		long lCount = 0L;

		pvector<GameChannel> vt;

		lChannelCount = m_set.size();
		for (ChannelSet_T::iterator it = m_set.begin(); it != m_set.end(); ++it)
		{
			GameChannel* pGameChannel = *it;
			//add
			lUserCount += pGameChannel->m_lUserCount;
			//if load balancing stop state
			if(pGameChannel->GetChannelOpenState() == FALSE)
				continue;
			if(!pGameChannel->IsCreated())
				continue;

			if (pGameChannel->m_lUserCount >= pGameChannel->m_lLimitCount)
			{
				lst.push_back(ChannelInfo());
				ChannelInfo& cinfo = lst.back();

				cinfo.BCopy((ChannelInfo)*pGameChannel);
//				lUserCount += cinfo.m_lUserCount;
			}
			else
			{
				if (lCount < lAvaliableCount)
				{
					if (pGameChannel->m_lUserCount == 0)
					{
						vt.push_back(pGameChannel);
					}
					else
					{
//						lUserCount += pGameChannel->m_lUserCount;
						lCount++;
						if (lCount <= lAvaliableCount)
						{
							lst.push_back(ChannelInfo());
							ChannelInfo& cinfo = lst.back();

							cinfo.BCopy((ChannelInfo)*pGameChannel);
						}
					}
				}
			}
		}
		if (lCount < lAvaliableCount)
		{
			long lRemainCount = lAvaliableCount - lCount;
			long lSize = vt.size();
			if (lRemainCount < lSize)
				lSize = lRemainCount;
			for (long i = 0; i < lSize; i++)
			{
				GameChannel* pGameChannel = vt[i];

				lst.push_back(ChannelInfo());
				ChannelInfo& cinfo = lst.back();

				cinfo.BCopy((ChannelInfo)*pGameChannel);
			}
		}
		VALIDATE(lst.BStore(buf));
	}

	VALIDATE(channelInfoList.BLoad(buf));
	return TRUE;
}

// 모든 채널 리스트를 전달한다. MMI 의 경우, NF인 경우
BOOL Category::GetAllChannelList(ChannelInfoList& channelInfoList, LONG& lUserCount, LONG& lChannelCount)
{
	GBuf buf;
	{
		ChannelInfoList lst;
		pvector<GameChannel> vt;

		lChannelCount = m_set.size();

		for (ChannelSet_T::iterator it = m_set.begin(); it != m_set.end(); ++it)
		{
			GameChannel* pGameChannel = *it;
			//add
			lUserCount += pGameChannel->m_lUserCount;
			//if load balancing stop state
			if(pGameChannel->GetChannelOpenState() == FALSE)
				continue;
			if(!pGameChannel->IsCreated())
				continue;

			lst.push_back(ChannelInfo());
			ChannelInfo& cinfo = lst.back();
			cinfo.BCopy((ChannelInfo)*pGameChannel);
		}

		VALIDATE(lst.BStore(buf));
	}

	VALIDATE(channelInfoList.BLoad(buf));
	return TRUE;
}

BOOL Category::GetChannelBaseInfo(ChannelBaseInfo& channelBaseInfo)
{
	if (m_set.size() <= 0)
		return FALSE;

	GameChannel* pGameChannel = *(m_set.begin());
	if (pGameChannel) {
		channelBaseInfo.BCopy(pGameChannel->GetChannelBaseInfo());
	}
	else
		return FALSE;

	return TRUE;
}

BOOL Category::GetAllChannelList(ChannelBaseInfoList& channelBaseInfoList, LONG& lUserCount, LONG& lChannelCount)
{
	GBuf buf;
	{
		ChannelBaseInfoList lst;
		pvector<GameChannel> vt;

		lChannelCount = m_set.size();
		for (ChannelSet_T::iterator it = m_set.begin(); it != m_set.end(); ++it)
		{
			GameChannel* pGameChannel = *it;
			//add
			lUserCount += pGameChannel->m_lUserCount;
			//if load balancing stop state
			if(pGameChannel->GetChannelOpenState() == FALSE)
				continue;
			if(!pGameChannel->IsCreated())
				continue;

			lst.push_back(ChannelBaseInfo());
			ChannelBaseInfo& cinfo = lst.back();
			cinfo.BCopy((ChannelBaseInfo)*pGameChannel);
		}

		VALIDATE(lst.BStore(buf));
	}

	VALIDATE(channelBaseInfoList.BLoad(buf));
	return TRUE;
}

// 웹게임 CHS와 channel형 SGS의 선택 로직, 사용자 수가 가장 많은 채널을 가지고 온다.
GameChannel* Category::FindMinGameChannel(long& lErr)
{
	lErr = ESVLB_ERR_CHANNEL_NOTFOUND;
	GameChannel* pSelectedChannel = NULL;
	{
		pvector<GameChannel> vt;
		vector<LONG> vtFactor;
		LONG lCount = 0L;
		LONG lAvailableCHS = theChannelSvrTable.AvailableChannelSvrCount();
		if (lAvailableCHS == 0)
		{
			lErr = ESVLB_ERR_SERVER_NOTFOUND;
			return NULL;
		}

		for (ChannelSet_T::iterator it = m_set.begin(); it != m_set.end(); ++it)
		{
			GameChannel* pGameChannel = *it;

			LONG lFactor = 0L;
			if(pGameChannel->m_bCreate && pGameChannel->GetChannelOpenState() && pGameChannel->CanService(lFactor))
			{
				lCount++;
				vt.push_back(pGameChannel);
				vtFactor.push_back(lFactor);
			}
		}

		if (lCount > 0)
		{
			LONG lMaxFactor = 1L;
			long lIdx = 0L;
			for (unsigned j = 0; j < (unsigned)lCount; j++)
			{
				LONG lVecFactor = vtFactor[j];
				if ((lVecFactor > lMaxFactor))
				{
					lMaxFactor = lVecFactor;
					lIdx = j;
				}
			}

			pSelectedChannel = vt[lIdx];
			lErr = 0L;
		}
		else
			lErr = ESVLB_ERR_CHANNEL_MAXUSER;
	}

	return pSelectedChannel;
}

GameChannel* Category::FindGameChannel(DWORD dwGCIID)
{
	// create temporary ChannelInfo, whose ID is dwGCIID.
	ChannelInfo temp_cInfo;
	temp_cInfo.m_channelID.m_dwGCIID = dwGCIID;
	// create temporary GameChannel, whose ID is dwGCCID.
	GameChannel temp_gch;
	temp_gch.UpdateGameChannel(temp_cInfo);

	ChannelSet_T::iterator it = m_set.find(&temp_gch);
	if (it == m_set.end())
		return NULL;
	else
		return *it;
}

void Category::GetGCList(list<GameChannel*>& lstGC)
{
	for (ChannelSet_T::iterator it = m_set.begin(); it != m_set.end(); ++it)
	{
		lstGC.push_back(*it);
	}
}

//////////////////////////////////////////////////////////////////////
// Category Table
//////////////////////////////////////////////////////////////////////
// Category Table Construction & Destruction
CategoryTable::CategoryTable(long lSSN) : m_lSSN(lSSN)
{
	m_bOpenState = TRUE;
}

CategoryTable::~CategoryTable()
{
	Clear();
}
//////////////////////////////////////////////////////////////////////
void CategoryTable::Clear()
{
	// ----------------------------------------
	ForEachElmt(TMap, m_mapCategory, i, j)
	{
		Category* pCategory = i->second;
		if(pCategory)
		{
			delete pCategory;
		}
	}
	m_mapCategory.clear();
}

BOOL CategoryTable::AddCategory(Category* pCategory)
{
	ASSERT(pCategory != NULL);

	ChannelPrefix& channelPrefix = pCategory->GetCategoryInfo();
	DWORD dwCategoryNum = channelPrefix.m_dwCategory;
	{
	//------------------------------------------
		TMap::iterator it = m_mapCategory.find(dwCategoryNum);
		if (it == m_mapCategory.end())
			m_mapCategory[dwCategoryNum] = pCategory;
		else
			return FALSE;
	//------------------------------------------
	}
	return TRUE;
}

Category* CategoryTable::FindCategory(ChannelPrefix& channelPrefix)
{
	DWORD dwCategoryNum = channelPrefix.m_dwCategory;
	{
	//-------------------------------------------
		{
			TMap::iterator it = m_mapCategory.find(dwCategoryNum);
			if (it == m_mapCategory.end())
				return NULL;
			else
			{
				Category* pCategory = it->second;
				return pCategory;
			}
		}
	//-------------------------------------------
	}
}

Category* CategoryTable::FindCategory(DWORD dwCategory)
{
	DWORD dwCategoryNum = dwCategory;
	{
	//-------------------------------------------
		{
			TMap::iterator it = m_mapCategory.find(dwCategoryNum);
			if (it == m_mapCategory.end())
				return NULL;
			else
			{
				Category* pCategory = it->second;
				return pCategory;
			}
		}
	//-------------------------------------------
	}
}

BOOL CategoryTable::GetAllCategory(ChannelBaseInfoList& lstChannelBaseInfo)
{
	BOOL bRet = FALSE;
	ForEachElmt(TMap, m_mapCategory, it, ij)
	{
		Category* pCategory = (*it).second;
		if (pCategory)
		{
			//bRet = pCategory->GetAllChannelList(lstChannelBaseInfo, lUserCount, lChannelCount);
			ChannelBaseInfo baseInfo;
			if (pCategory->GetChannelBaseInfo(baseInfo))
                lstChannelBaseInfo.push_back(baseInfo);
			else
				return bRet;
		}
		else
			return bRet;
	}

	return TRUE;
}

BOOL CategoryTable::GetAllCategory(ChannelInfoList& lstChannelInfo)
{
	BOOL bRet = FALSE;
	ForEachElmt(TMap, m_mapCategory, it, ij)
	{
		Category* pCategory = (*it).second;
		if (pCategory)
		{
			LONG lUserCount = 0, lChannelCount = 0;
			bRet = pCategory->GetAllChannelList(lstChannelInfo, lUserCount, lChannelCount);

			if (!bRet)
				return FALSE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////
//Service Table

ServiceTable::ServiceTable()
{
}
ServiceTable::~ServiceTable()
{
	Clear();
}
//////////////////////////////////////////////////////////////////////
void ServiceTable::Clear()
{
	ForEachElmt(TMap, m_mapService, i, j)
	{
		CategoryTable* pCategoryTable = i->second;
		if(pCategoryTable)
		{
			delete pCategoryTable;
		}
	}
	m_mapService.clear();
}
CategoryTable* ServiceTable::SearchCategoryTable(LONG lServiceNum)
{
	TLock lo(this);
	if(lServiceNum == -1)
		return NULL;
	TMap::iterator it = m_mapService.find(lServiceNum);
	if(it == m_mapService.end())	return NULL;
	else
	{
		return it->second;
	}
}
BOOL ServiceTable::AddService(LONG lServiceNum, CategoryTable* pCategoryTable)
{
	ASSERT(pCategoryTable != NULL);
	{
		TLock lo(this);
	//------------------------------------------
		TMap::iterator it = m_mapService.find(lServiceNum);
		if (it == m_mapService.end())
		{
			m_mapService[lServiceNum] = pCategoryTable;
		}
		else
			return FALSE;
	//------------------------------------------
	}
	return TRUE;
}

void ServiceTable::AllServiceInfo(AMS_CategoryList& lst)
{
	TLock lo(this);

	lst.clear();
	ForEachElmt(TMap, m_mapService, i, j)
	{
		//---------------------------------
		CategoryTable* pCategoryTable = i->second;
		if(pCategoryTable != NULL)
		{
			ForEachElmt(CategoryTable::TMap, pCategoryTable->m_mapCategory, i1, j1)
			{
				Category* pCategory = i1->second;
				if(pCategory != NULL)
				{
					lst.push_back(AMS_Category());
					AMS_Category& amsCategory = lst.back();

					// get channel prefix
					amsCategory.m_channelPrefix.BCopy( pCategory->GetCategoryInfo() );

					// get channel list
					Category::ChannelSet_T &ref_set = pCategory->m_set;
					for (Category::ChannelSet_T::iterator it = ref_set.begin(); it != ref_set.end(); ++it)
					{
						GameChannel* pGameChannel = *it;
						if(!pGameChannel->IsCreated())
							continue;

						amsCategory.m_gclist.push_back(AMS_GCInfo());
						AMS_GCInfo& gcinfo = amsCategory.m_gclist.back();

						gcinfo.m_channelMode.m_lCACode		= pGameChannel->m_lCACode;
						gcinfo.m_channelMode.m_lChannelType	= pGameChannel->WhatChannelType();
						gcinfo.m_channelMode.m_lLimitCount	= pGameChannel->m_lLimitCount;
						gcinfo.m_channelMode.m_lMaxCount	= pGameChannel->m_lMaxCount;
						gcinfo.m_channelID					= pGameChannel->m_channelID;
						gcinfo.m_lGRCount					= pGameChannel->m_lRoomCount;
						gcinfo.m_lUserCount					= pGameChannel->m_lUserCount;
						gcinfo.m_nsapCHS					= pGameChannel->GetNSAP();
						gcinfo.m_sTitle						= pGameChannel->m_sTitle.c_str();
						gcinfo.m_sCategoryName				= pGameChannel->m_sCategoryName.c_str();
						gcinfo.m_bIsChannelOpen				= pGameChannel->GetChannelOpenState();

//						theErr.LOG(1, "AMSInfo", "AMS Category INFO : [%d], [%s], [%s]", AMSGCInfo.m_channelPrefix.m_lSSN, AMSGCInfo.m_channelPrefix.m_sCategory.c_str(), AMSGCInfo.m_sGCIID.c_str());
					}
					amsCategory.m_lChannelCount = amsCategory.m_gclist.size();
					//service open state
					amsCategory.m_bLBFlag = pCategoryTable->GetOpenState();
				}
				else
					continue;
			}
		}
		else
			continue;
		//--------------------------------
	}
}

BOOL ServiceTable::DeleteAllService()
{
	TLock lo(this);

	Clear();

	return TRUE;
}

BOOL ServiceTable::OnGameChannelCreate(const ChannelID& channelID, long lPort)
{
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(channelID.m_lSSN);
		if(pCategoryTable != NULL)
		{
			Category* pCategory = pCategoryTable->FindCategory(channelID.m_dwCategory);
			if(pCategory)
			{
				GameChannel* pGameChannel = pCategory->FindGameChannel(channelID.m_dwGCIID);
				if(pGameChannel)
				{
					pGameChannel->OnCreate(lPort);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL ServiceTable::OnGameChannelDestroy(const ChannelID& channelID)
{
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(channelID.m_lSSN);
		if(pCategoryTable != NULL)
		{
			Category* pCategory = pCategoryTable->FindCategory(channelID.m_dwCategory);
			if(pCategory)
			{
				GameChannel* pGameChannel = pCategory->FindGameChannel(channelID.m_dwGCIID);
				if(pGameChannel)
				{
					pGameChannel->OnDestroy();
					return TRUE;
				}
			}
		}
	}
	LOG (INF,  "ServiecTable::OnGameChannelDestroy - no channel like that : [",SVCCAT_CHS,"] ");

	return FALSE;
}

BOOL ServiceTable::UpdateChannelInfoList(const ChannelUpdateInfoList& lst)
{
	BOOL bRet = TRUE;
	TLock lo(this);
	{
		ForEachCElmt(ChannelUpdateInfoList, lst, i1, j1)
		{
			const ChannelUpdateInfo& chinfo = (*i1);
			GameChannel* pGameChannel = NULL;
			CategoryTable* pCategoryTable = SearchCategoryTable(chinfo.m_channelID.m_lSSN);
			if(pCategoryTable != NULL)
			{
				Category* pCategory = pCategoryTable->FindCategory(chinfo.m_channelID.m_dwCategory);
				if(pCategory)
				{
					pGameChannel = pCategory->FindGameChannel(chinfo.m_channelID.m_dwGCIID);
				}
			}
			if(pGameChannel)
				pGameChannel->UpdateGameChannel(chinfo);
			else
			{
				bRet = FALSE;
				string sBuf = ::format("%d:%d:%d", chinfo.m_channelID.m_lSSN, chinfo.m_channelID.m_dwCategory, chinfo.m_channelID.m_dwGCIID);
				LOG (INF "**ServiceTable::UpdateChannelInfoList(",sBuf.c_str(),") : No channel exist **");
			}
		}
	}
	return bRet;
}

//		ESVLB_ERR_CHANNEL_MAXUSER;
//		ESVLB_ERR_CHANNEL_NOTFOUND;
//		ESVLB_ERR_CATEGORY_NOTFOUND;
long ServiceTable::RecommendGameChannel(const ChannelPrefix& prefix, ChannelInfo& cinfo)
{
	long lErr = 0;
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(prefix.m_lSSN);
		if(!pCategoryTable)
		{
			lErr = ESVLB_ERR_SERVICE_NOTFOUND;
		}
		else if(pCategoryTable->GetOpenState() == FALSE)
		{
			lErr = ESVLB_ERR_SERVICE_CLOSE;
		}
		else
		{
			Category* pCategory = pCategoryTable->FindCategory(prefix.m_dwCategory);
			if(!pCategory)
			{
				lErr = ESVLB_ERR_CATEGORY_NOTFOUND;
			}
			else
			{
				GameChannel* pGameChannel = pCategory->FindMinGameChannel(lErr);
				if(pGameChannel)
				{
					GBuf buf;
					VALIDATE(pGameChannel->BStore(buf));
					VALIDATE(cinfo.BLoad(buf));

					pGameChannel->m_lTraffic += (LONG)(pGameChannel->m_lLimitCount / 10);
				}
			}
		}
	}
	return lErr;
}

long ServiceTable::VerifyGameChannel(const ChannelID& channelID, ChannelInfo& cinfo)
{
	long lErr = 0;
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(channelID.m_lSSN);
		if(!pCategoryTable)
		{
			lErr = ESVLB_ERR_SERVICE_NOTFOUND;
		}
		else if(pCategoryTable->GetOpenState() == FALSE)
		{
			lErr = ESVLB_ERR_SERVICE_CLOSE;
		}
		else
		{
			Category* pCategory = pCategoryTable->FindCategory(channelID.m_dwCategory);
			if(!pCategory)
			{
				lErr = ESVLB_ERR_CATEGORY_NOTFOUND;
			}
			else
			{
				GameChannel* pGameChannel = pCategory->FindGameChannel(channelID.m_dwGCIID);
				if(!pGameChannel)
				{
					lErr = ESVLB_ERR_CHANNEL_NOTFOUND;
					LOG (ERR, "ServiceTabel:--FAIL FindGameChannel[",channelID.m_dwGCIID,"]--");
				}
				else if(!pGameChannel->IsCreated())
				{
					lErr = ESVLB_ERR_CHANNEL_NOTFOUND;
					LOG (ERR, "ServiceTabel:--FAIL not created Channel[",channelID.m_dwGCIID,"]--");
				}
				else if(!pGameChannel->GetChannelOpenState())
				{
					lErr = ESVLB_ERR_CHANNEL_NOTFOUND;
					LOG (ERR, "ServiceTabel:--FAIL not opened Channel[",channelID.m_dwGCIID,"]--");
				}
				else if(!pGameChannel->CanGRJoin())
				{
					lErr = ESVLB_ERR_CHANNEL_MAXUSER;
				}
				else
				{
					GBuf buf;
					VALIDATE(pGameChannel->BStore(buf));
					VALIDATE(cinfo.BLoad(buf));

					pGameChannel->m_lUserCount++;
				}
			}
		}
	}
	return lErr;
}

long ServiceTable::FindGameChannel(const ChannelID& channelID, ChannelInfo& cinfo)
{
	long lErr = 0;
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(channelID.m_lSSN);
		if(!pCategoryTable)
		{
			lErr = ESVLB_ERR_SERVICE_NOTFOUND;
		}
		else if(pCategoryTable->GetOpenState() == FALSE)
		{
			lErr = ESVLB_ERR_SERVICE_CLOSE;
		}
		else
		{
			Category* pCategory = pCategoryTable->FindCategory(channelID.m_dwCategory);
			if(!pCategory)
			{
				lErr = ESVLB_ERR_CATEGORY_NOTFOUND;
			}
			else
			{
				GameChannel* pGameChannel = pCategory->FindGameChannel(channelID.m_dwGCIID);
				if(!pGameChannel)
				{
					lErr = ESVLB_ERR_CHANNEL_NOTFOUND;
				}
				else
				{
					pGameChannel->m_lUserCount = cinfo.m_lUserCount;
				}
			}
		}
	}
	return lErr;
}

void ServiceTable::GetChannelList(const ChannelID& channelID, ChannelInfoList& lstChannelInfo, LONG& lUserCount, LONG& lChannelCount)
{
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(channelID.m_lSSN);
		if(pCategoryTable)
		{
			Category* pCategory = pCategoryTable->FindCategory(channelID.m_dwCategory);
			if(pCategory)
			{
				// 
				if (channelID.m_lSSN == NF_SSN)
					pCategory->GetAllChannelList(lstChannelInfo, lUserCount, lChannelCount);
			}
		}
	}
}

BOOL ServiceTable::GetAllCategoryList(ChannelBaseInfoList& listChannelBaseInfo, LONG lSSN)
{
	TLock lo(this);
	BOOL bRet = FALSE;

	CategoryTable* pCategoryTable = SearchCategoryTable(lSSN);
	if (pCategoryTable)
	{
		bRet = pCategoryTable->GetAllCategory(listChannelBaseInfo);
	}
	return bRet;
}

BOOL ServiceTable::GetAllCategoryList(ChannelInfoList& listChannelInfo, LONG lSSN)
{
	TLock lo(this);
	BOOL bRet = FALSE;

	CategoryTable* pCategoryTable = SearchCategoryTable(lSSN);
	if (pCategoryTable)
	{
		bRet = pCategoryTable->GetAllCategory(listChannelInfo);
	}
	return bRet;
}

void ServiceTable::EnableChannelInChSvr(const NSAP& nsap, LRBAddress lrbAddr, ChannelRegInfoList lstChRegInfo)
{
	TLock lo(this);
	//channel count of chs
	DWORD dwCount = 0;
	//channel count of openstate is false
	DWORD dwClosedCount = 0;
	//Flag about channelsvr
	BOOL bIsChSvr = FALSE;
	ForEachElmt(TMap, m_mapService, i, j)
	{
		//---------------------------------
		CategoryTable* pCategoryTable = i->second;
		ASSERT(pCategoryTable != NULL);
		ForEachElmt(CategoryTable::TMap, pCategoryTable->m_mapCategory, i1, j1)
		{
			Category* pCategory = i1->second;
			ASSERT(pCategory != NULL);

			ForEachElmt(Category::ChannelSet_T, pCategory->m_set, it, jt)
			{
				GameChannel* pGameChannel = *it;
				ASSERT(pGameChannel);

				if(pGameChannel->m_nsapCHS.m_dwIP != nsap.m_dwIP)
					continue;
				if(pGameChannel->IsCreated())
				{
					LOG (ERR, "ServiceTable: --Warning : Try to Create already Created-Channel GCIID[",pGameChannel->m_channelID.m_dwGCIID,"]");
					bIsChSvr = TRUE;
					continue;
				}
				pGameChannel->OnCreate(nsap.m_dwPort);
				if(!theChannelSvrTable.AddChannelInfoInChannelSvr(lrbAddr, (ChannelInfo)*pGameChannel))
				{
					string strAddr;
					lrbAddr.GetStringFormat(strAddr);
					LOG (ERR, "ServiceTable: --Error : AddChanneelInfo In Not Registed CHS [",strAddr,"]");
				}
				//channel count of CHS
				dwCount++;
				if(!pGameChannel->GetChannelOpenState())
					dwClosedCount++;
			}
		}
	}
	//새로운 channel server이다.
	if(0 == dwCount && bIsChSvr == FALSE)
	{
		AddChannelsInSvcTb(nsap, lrbAddr, lstChRegInfo);
		LOG (INF, "ServiceTable : New Channel(Not regist service table) SVR Reg");
		return;
	}
	LOG (INF, "ServiceTable : EnableChannelInChSvr Total channels[",dwCount,"] , Disable channels[",dwClosedCount,"]");
	
	//중복 등록 이다.
	if(dwCount == 0 && bIsChSvr == TRUE)
		return;
	if(dwCount == dwClosedCount)
	{
		BOOL bRet = theChannelSvrTable.CloseChannelServer(lrbAddr);
		if(!bRet)
			LOG (ERR, "ServiceTable : ---Error CloseChannelSvr : Not registed Chs");
		LOG (ERR, "ServiceTable : INFO2 : Chs Close ! ");
	}
}

BOOL ServiceTable::CloseChannel(const ChannelID& channelID)
{
	{
		TLock lo(this);

		CategoryTable* pCategoryTable = SearchCategoryTable(channelID.m_lSSN);
		if(pCategoryTable != NULL)
		{
			Category* pCategory = pCategoryTable->FindCategory(channelID.m_dwCategory);
			if(pCategory)
			{
				GameChannel* pGameChannel = pCategory->FindGameChannel(channelID.m_dwGCIID);
				if(pGameChannel)
				{
					pGameChannel->CloseChannel();
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void ServiceTable::CloseChannel(const NSAP& nsap)
{
	{
		TLock lo(this);

		ForEachElmt(TMap, m_mapService, i, j)
		{
			//---------------------------------
			CategoryTable* pCategoryTable = i->second;
			ASSERT(pCategoryTable != NULL);
			ForEachElmt(CategoryTable::TMap, pCategoryTable->m_mapCategory, i1, j1)
			{
				Category* pCategory = i1->second;
				ASSERT(pCategory != NULL);

				ForEachElmt(Category::ChannelSet_T, pCategory->m_set, it, jt)
				{
					GameChannel* pGameChannel = *it;
					ASSERT(pGameChannel);

					if(pGameChannel->m_nsapCHS == nsap)
						pGameChannel->CloseChannel();
				}
			}
		}
	}
}

BOOL ServiceTable::CloseService(long lSSN)
{
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(lSSN);
		if(pCategoryTable != NULL)
		{
			pCategoryTable->SetOpenState(FALSE);
			return TRUE;
		}
	}
	return FALSE;
}


BOOL ServiceTable::OpenChannel(const ChannelID& channelID)
{
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(channelID.m_lSSN);
		if(pCategoryTable != NULL)
		{
			Category* pCategory = pCategoryTable->FindCategory(channelID.m_dwCategory);
			if(pCategory)
			{
				GameChannel* pGameChannel = pCategory->FindGameChannel(channelID.m_dwGCIID);
				if(pGameChannel)
				{
					pGameChannel->OpenChannel();
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void ServiceTable::OpenChannel(const NSAP& nsap)
{
	{
		TLock lo(this);

		ForEachElmt(TMap, m_mapService, i, j)
		{
			//---------------------------------
			CategoryTable* pCategoryTable = i->second;
			ASSERT(pCategoryTable != NULL);
			ForEachElmt(CategoryTable::TMap, pCategoryTable->m_mapCategory, i1, j1)
			{
				Category* pCategory = i1->second;
				ASSERT(pCategory != NULL);

				ForEachElmt(Category::ChannelSet_T, pCategory->m_set, it, jt)
				{
					GameChannel* pGameChannel = *it;
					ASSERT(pGameChannel);

					if(pGameChannel->m_nsapCHS == nsap)
						pGameChannel->OpenChannel();
				}
			}
		}
	}
}

BOOL ServiceTable::OpenService(long lSSN)
{
	{
		TLock lo(this);
		CategoryTable* pCategoryTable = SearchCategoryTable(lSSN);
		if(pCategoryTable != NULL)
		{
			pCategoryTable->SetOpenState(TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ServiceTable::Init()
{
	TLock lo(this);

	DBGW_String strResult;
	int nResult;
	
	string sQuery = "AdminDB|Q|select CHANNELTYPE,CHSIP,CATEGORYNUM,CATEGORYNAME,SSN,ACCODE,GCID,MAXCOUNT,LIMITCOUNT,TITLE,OPENSTATE from info_channel where ssn = 55 order by ssn, categorynum, gcid";

	BOOL bRet = ExecuteQuery(1, sQuery.c_str(), &strResult, &nResult);
	if(!bRet)
	{
		LOG (ERR, "ServiceTable : *** Fail Query from AdminDB, ADMIN_DB_GETCHDATA ");
		return FALSE;
	}

	string sBuf = strResult.GetData();
	DB_ChannelInfoList	channelInfolst; 

	bRet = GetParseData(sBuf, channelInfolst, QUERY_DELIMETER);
	if(!bRet)
	{
		LOG (ERR, "ServiceTable : *** Fail to parsing ADMIN_DB_GETCHDATA, Error : ", sBuf);
		return FALSE;
	}
	
	ForEachElmt(DB_ChannelInfoList, channelInfolst, i, j)
	{
		DB_ChannelInfo& dbinfo = *i;

		ChannelInfo info;
		info.m_channelID					= dbinfo.m_channelID;
		info.m_nsapCHS.m_dwIP				= dbinfo.m_dwCHSIP;
		info.m_lMaxCount					= dbinfo.m_lMaxCount;
		info.m_lLimitCount					= dbinfo.m_lLimitCount;
		info.m_lCACode						= dbinfo.m_lACCode;
		info.m_sTitle						= dbinfo.m_sTitle.c_str();
		info.m_sCategoryName				= dbinfo.m_sCategoryName.c_str();

		BOOL bOpenState = ( (dbinfo.m_lChannelOpenState > 0) ? TRUE : FALSE );
		theServiceTable.CreateGameChannel(info, dbinfo.m_lChannelType, bOpenState);
#ifdef LEGO_VERSION
		LOG (TESTINF, "ServiceTable : INFO : From DB ChannelID [",info.m_channelID.m_dwGCIID,"], IP[",info.m_nsapCHS.GetIP(),"], bOpenState [", bOpenState,"] ");
#endif
	}
	LOG (INF, "ServiceTable :INFO : Static ChannelInfo List Size : [",channelInfolst.size(),"] ");

	return TRUE;
}

BOOL ServiceTable::AddChannelsInSvcTb(const NSAP& nsap, LRBAddress lrbAddr, ChannelRegInfoList lstChRegInfo)
{
	TLock lo(this);

	DWORD dwClosedCount = 0;
	ForEachElmt(ChannelRegInfoList, lstChRegInfo, i, j)
	{
		ChannelRegistInfo& ChRegInfo = *i;

		ChannelInfo info;
		info.m_channelID					= ChRegInfo.m_channelID;
		info.m_nsapCHS						= nsap;
		info.m_lMaxCount					= ChRegInfo.m_lMaxCount;
		info.m_lLimitCount					= ChRegInfo.m_lLimitCount;
		info.m_lCACode						= ChRegInfo.m_lACCode;
		info.m_sTitle						= ChRegInfo.m_sTitle.c_str();
		info.m_sCategoryName				= ChRegInfo.m_sCategoryName.c_str();

		BOOL bOpenState = ( (ChRegInfo.m_lChannelOpenState > 0) ? TRUE : FALSE );
		if(bOpenState == FALSE)
		{
			dwClosedCount++;
			LOG (INF, "ServiceTable : Info : closed channel SSN [",ChRegInfo.m_channelID.m_lSSN,"] GCID[",ChRegInfo.m_channelID.m_dwGCIID,"]");
		}
		//service table에 채널 생성.
		theServiceTable.CreateGameChannel(info, ChRegInfo.m_lChannelType, bOpenState, TRUE);
		//channelsvr table에 해당 channel정보 입력.
		if(!theChannelSvrTable.AddChannelInfoInChannelSvr(lrbAddr, info))
		{
			string strAddr;
			lrbAddr.GetStringFormat(strAddr);
			LOG (ERR, "ServiceTable : --Error : AddChanneelInfo In Not Registed CHS [",strAddr,"]");
		}
	}
	if( (lstChRegInfo.size() == dwClosedCount) && (0 != lstChRegInfo.size()) )
	{
		BOOL bRet = theChannelSvrTable.CloseChannelServer(lrbAddr);
		if(!bRet)
			LOG (ERR, "ServiceTable : ---Error CloseChannelSvr : Not registed Chs");
		LOG (INF, "ServiceTable : INFO2 : Chs Close ! ");
	}
	LOG (INF, "ServiceTable : INFO : Static ChannelInfo List Size : [",lstChRegInfo.size(),"] ");
	return TRUE;
}

void ServiceTable::CreateGameChannel(const ChannelInfo& info, long lType, BOOL bOpenState, BOOL IsNewChSvr)
{
	TLock lo(this);

	CategoryTable* pCategoryTable = SearchCategoryTable(info.m_channelID.m_lSSN);
	if(!pCategoryTable)
	{
		pCategoryTable = new CategoryTable(info.m_channelID.m_lSSN);
		AddService(info.m_channelID.m_lSSN, pCategoryTable);
		pCategoryTable->SetOpenState(TRUE);
	}
	Category* pCategory = pCategoryTable->FindCategory(info.m_channelID.m_dwCategory);
	if(!pCategory)
	{
		pCategory = new Category(info.m_channelID.m_lSSN, info.m_channelID.m_dwCategory);
		pCategoryTable->AddCategory(pCategory);
	}
	GameChannel* pGameChannel = pCategory->FindGameChannel(info.m_channelID.m_dwGCIID);
	if(pGameChannel)
	{
		pGameChannel->UpdateGameChannel(info);
	}
	else
	{
		pGameChannel = new GameChannel;
		GBuf buf;
		VALIDATE(info.BStore(buf));
		VALIDATE(pGameChannel->BLoad(buf));
		pCategory->AddGameChannel(pGameChannel);
	}

	pGameChannel->SetChannelType(lType);
	if(IsNewChSvr == FALSE)
		pGameChannel->SetChannelOpenState(bOpenState);
	else
		pGameChannel->SetChannelOpenState2(bOpenState);
}

BOOL ServiceTable::GetParseData(string& sBuf, DB_ChannelInfoList& lstchannelInfo, char cDelimeter)
{
	int nSize = sBuf.size();
	char* sTempBuf = new char[nSize + 1];
	memset(sTempBuf, '\0', nSize + 1);
	sprintf(sTempBuf, "%s", sBuf.c_str());

	if (sTempBuf[0] != 'S' || sTempBuf[1] != cDelimeter)
		return FALSE;

	string sTemp;
	list<string> lstData;

	for (int nCursor = 4; nCursor < nSize + 1; nCursor++)
	{
		if (sTempBuf[nCursor] == cDelimeter)
		{
			lstData.push_back(sTemp);

			sTemp.erase();
		}
		else if (sTempBuf[nCursor] == '\\' && sTempBuf[nCursor+1] == cDelimeter)
			sTemp += sTempBuf[++nCursor];
		else if (sTempBuf[nCursor] == '\\' && sTempBuf[nCursor+1] == '\\')
			sTemp += sTempBuf[++nCursor];
		else
			sTemp += sTempBuf[nCursor];
	}

	LONG lChannelSize = lstData.size() / MAX_FIELD_COUNT;

	list<string>::iterator itr = lstData.begin();
	while (lChannelSize > 0)
	{
		DB_ChannelInfo ChannelInfo;
		LONG lSSN;
		DWORD dwCaNum, dwGCID;

		ChannelInfo.m_lChannelType = atoi((*itr).c_str());
		itr++;
		ChannelInfo.m_dwCHSIP = inet_addr ((*itr).c_str());
		itr++;
		dwCaNum = atoi((*itr).c_str());
		itr++;
#ifndef _VERSION_JAPAN_
		ChannelInfo.m_sCategoryName = (*itr).c_str();
#else
		ChannelInfo.m_sCategoryName = str2wstr((*itr).c_str());
#endif
		itr++;
		lSSN = atoi((*itr).c_str());
		itr++;
		ChannelInfo.m_lACCode = atoi((*itr).c_str());
		itr++;
		dwGCID = atoi((*itr).c_str());
		ChannelInfo.m_channelID = ChannelID( lSSN, dwCaNum, dwGCID);
		itr++;
		ChannelInfo.m_lMaxCount = atoi((*itr).c_str());
		itr++;
		ChannelInfo.m_lLimitCount = atoi((*itr).c_str());
		itr++;
#ifndef _VERSION_JAPAN_
		ChannelInfo.m_sTitle = (*itr).c_str();
#else
		ChannelInfo.m_sTitle = str2wstr((*itr).c_str());
#endif
		itr++;
		ChannelInfo.m_lChannelOpenState = atoi((*itr).c_str());
		itr++;

		lstchannelInfo.push_back(ChannelInfo);

		if (itr == lstData.end())
			break;
		lChannelSize--;
	}

	return TRUE;
}
