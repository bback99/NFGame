// 
// ChannelSvrTable.cpp
// 

#include "stdafx.h"
#include "ChannelSvrTable.h"
#include "Category.h"

ChannelSvrTable theChannelSvrTable;

#define LIMIT_USER_FACTOR		30
#define LIMIT_ROOM_FACTOR		30
#define LIMIT_WAITING_FACTOR	30
#define LIMIT_GLS_FACTOR		10

/////////////////////////////////////////////////////////////////////////////
// Construction & Destruction
ChannelSvrInfo::ChannelSvrInfo()
{
	m_bIsChsOpen = TRUE;
	m_lServerType = 0L;
	m_NSAPInfo.Clear();

	//default CHS;
	m_lSvrCatType = SVCCAT_CHS;
}

ChannelSvrInfo::ChannelSvrInfo(NSAP& nsap, LRBAddress lrbSvrAddr)
{
	m_NSAPInfo = nsap;
	m_lServerType = SVCCAT_CHS;
	m_lServerID = lrbSvrAddr;
	m_bIsChsOpen = TRUE;

	//default CHS;
	m_lSvrCatType = SVCCAT_CHS;
}

ChannelSvrInfo::~ChannelSvrInfo()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////////
// external interface
void ChannelSvrInfo::Clear()
{
	TBase::Clear();

	m_lstChannelInfo.clear();
	m_bIsChsOpen = TRUE;
}

void ChannelSvrInfo::CloseCHS()
{
	m_bIsChsOpen = FALSE; 
}
void ChannelSvrInfo::OpenCHS()
{
	m_bIsChsOpen = TRUE;
}

void ChannelSvrInfo::AddChannelInfo(const ChannelInfo& cinfo)
{
	m_lstChannelInfo.push_back(ChannelInfo());
	ChannelInfo& info = m_lstChannelInfo.back();
	info.BCopy(cinfo);
}

void ChannelSvrInfo::UpdateChannelList(ChannelUpdateInfoList& lstChannelInfo)
{
	ForEachElmt(ChannelUpdateInfoList, lstChannelInfo, it1, jt1)
	{
		ChannelUpdateInfo& info1 = *it1;

		ForEachElmt(ChannelInfoList, m_lstChannelInfo, it2, jt2)
		{
			ChannelInfo& info2 = *it2;


			if (info2.m_channelID.m_dwGCIID == info1.m_channelID.m_dwGCIID)
			{
				info2.m_lUserCount = info1.m_lUserCount;
				info2.m_lWaitingUserCount = info1.m_lWaitingUserCount;
				info2.m_lRoomCount = info1.m_lRoomCount;
				break;
			}
		}
		if(it2 == m_lstChannelInfo.end())
		{
			string sBuf = ::format("%d:%d:%d", info1.m_channelID.m_lSSN, info1.m_channelID.m_dwCategory, info1.m_channelID.m_dwGCIID);
			LOG (INF, "**ChannelSvrInfo::UpdateChannelList(",sBuf,") : No channel exist **");
		}
	}
}

BOOL ChannelSvrInfo::GetChannelList(ChannelInfoList& channelInfoList)
{
	GBuf buf;
	VALIDATE(m_lstChannelInfo.BStore(buf));
	VALIDATE(channelInfoList.BLoad(buf));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// ChannelSvrTable
ChannelSvrTable::ChannelSvrTable()
{
}

ChannelSvrTable::~ChannelSvrTable()
{
	TLock lo(this);
	// ----------------------------------------
	{
		ForEachElmt(TMap, m_channelsvrTable, it, jt)
		{
			ChannelSvrInfo* pChannelSvrInfo = it->second;
			if (!pChannelSvrInfo)
				delete pChannelSvrInfo;
		}
		m_channelsvrTable.clear();
	}
	// ----------------------------------------
}

BOOL ChannelSvrTable::AddChannelSvr(LRBAddress lrbAddr, NSAP& nsap, LONG lSvrCatType)
{
	TLock lo(this);
	// ----------------------------------------
	TMap::iterator it = m_channelsvrTable.find(lrbAddr);
	if (it == m_channelsvrTable.end())
	{
		ChannelSvrInfo* pChannelSvrInfo = new ChannelSvrInfo(nsap, lrbAddr);
		//CGS??---------
		pChannelSvrInfo->m_lSvrCatType = lSvrCatType;
		LOG (INF, "ChannelSvrTable : This Service's Category is [",lSvrCatType,"]");
		//-----------
		m_channelsvrTable[lrbAddr] = pChannelSvrInfo;
		return TRUE;
	}
	else
		return FALSE;
	// ----------------------------------------
}

BOOL ChannelSvrTable::UpdateChannelSvr(LRBAddress lrbAddr, ChannelUpdateInfoList& lstChannelInfo)
{
	TLock lo(this);
	// ----------------------------------------
	TMap::iterator it = m_channelsvrTable.find(lrbAddr);
	if (it != m_channelsvrTable.end())
	{
		ChannelSvrInfo* pChannelSvrInfo = it->second;
		ASSERT(pChannelSvrInfo);
		pChannelSvrInfo->UpdateChannelList(lstChannelInfo);
		return TRUE;
	}
	return FALSE;
	// ----------------------------------------
}
BOOL ChannelSvrTable::AddChannelInfoInChannelSvr(LRBAddress lrbAddr, const ChannelInfo& ChInfo)
{
	BOOL bRet = FALSE;

	TLock lo(this);
	{
		TMap::iterator it = m_channelsvrTable.find(lrbAddr);
		bRet = (it != m_channelsvrTable.end());
		if(bRet)
		{
			ChannelSvrInfo* pInfo = it->second;
			pInfo->AddChannelInfo(ChInfo);
		}
	}

	return bRet;
}

BOOL ChannelSvrTable::DeleteChannelSvr(LRBAddress lrbAddr)
{
	TLock lo(this);
	// ----------------------------------------
	TMap::iterator it = m_channelsvrTable.find(lrbAddr);
	if (it != m_channelsvrTable.end())
	{
		ChannelSvrInfo* pInfo = it->second;
		if(pInfo)
			delete pInfo;
		m_channelsvrTable.erase(it);
		return TRUE;
	}
	// ----------------------------------------
	return FALSE;
}

BOOL ChannelSvrTable::GetGameChannelList(LRBAddress lrbAddr, ChannelInfoList& lst)
{
	{
		TLock lo(this);

		TMap::iterator it = m_channelsvrTable.find(lrbAddr);
		if (it != m_channelsvrTable.end())
		{
			ChannelSvrInfo* pInfo = it->second;
			pInfo->GetChannelList(lst);
			return TRUE;
		}
		return FALSE;
	}
}

void ChannelSvrTable::Clear()
{
	TLock lo(this);
	{
	// -------------------------------------------------
		ForEachElmt(TMap, m_channelsvrTable, i, j)
		{
			ChannelSvrInfo* pInfo = i->second;
			if(pInfo)
			{
				//해당 Chs소유의 channel의 상태를 destory상태로 돌린다.
				ChannelInfoList lst;
				pInfo->GetChannelList(lst);
				ForEachElmt(ChannelInfoList, lst, i1, j1)
				{
					ChannelInfo& ChInfo = (*i1);
					if(!theServiceTable.OnGameChannelDestroy(ChInfo.m_channelID))
					{
						LOG (ERR, "ChannelSvrTable : ERROR :  channel not destory[%d]", ChInfo.m_channelID.m_dwGCIID);
					}
				}
				delete pInfo;
			}
		}
		m_channelsvrTable.clear();
	// -------------------------------------------------
	}

}

BOOL ChannelSvrTable::CloseChannelServer(LRBAddress lrbAddr, NSAP& nsap)
{
	TLock lo(this);
	{
		TMap::iterator it = m_channelsvrTable.find(lrbAddr);
		if (it != m_channelsvrTable.end())
		{
			ChannelSvrInfo* pInfo = it->second;
			nsap = pInfo->m_NSAPInfo;
			pInfo->CloseCHS();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ChannelSvrTable::CloseChannelServer(LRBAddress lrbAddr)
{
	TLock lo(this);
	{
		TMap::iterator it = m_channelsvrTable.find(lrbAddr);
		if (it != m_channelsvrTable.end())
		{
			ChannelSvrInfo* pInfo = it->second;
			pInfo->CloseCHS();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ChannelSvrTable::StartChannelServer(LRBAddress lrbAddr, NSAP& nsap)
{
	TLock lo(this);
	{
		TMap::iterator it = m_channelsvrTable.find(lrbAddr);
		if (it != m_channelsvrTable.end())
		{
			ChannelSvrInfo* pInfo = it->second;
			nsap = pInfo->m_NSAPInfo;
			pInfo->OpenCHS();
			return TRUE;
		}
	}
	return FALSE;
}

LONG ChannelSvrTable::AvailableChannelSvrCount()
{
	LONG lCount = 0L;
	{
		TLock lo(this);
		ForEachElmt(TMap, m_channelsvrTable, i, j)
		{
			ChannelSvrInfo* pInfo = i->second;
			if (pInfo->m_bIsChsOpen)
				lCount++;
		}
	}
	return lCount;
}
