// LCInfoMap.cpp: implementation of the CLCMainDB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LCInfoMap.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define LIMITSIZE_LCSBASEINFO	250

CLocationMainDB theLocationMainDB;
//CServerListEx theServerListEx;
map<long, long> gMapIndexForMainDB;

CLocationMainDB::CLocationMainDB()
{
#ifdef USE_HEAPMEMPOOL
//	LONG l_cnt = m_LCMemoryPool.CreatePool(l_limitsize);
	LONG l_cnt = m_LCMemoryPool.CreatePool(LIMITSIZE_LCSBASEINFO, 1000);//LIMITSIZE_LCSBASEINFO
	printf("---- created heap memory pool %d pool size : %d block size ---- \n", l_cnt, LIMITSIZE_LCSBASEINFO);
#endif

	m_lAddCount = 0;
}

CLocationMainDB::~CLocationMainDB()
{

}

LONG CLocationMainDB::AddItem(TKey key, NLSBaseInfo & newBaseInfo, NLSBaseInfo & tempBaseInfo)
{
	TLock lo(this);

	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(!usnlist)
	{
		if(!_AddItem(key, newBaseInfo)){
			//theErr.LOG(1, "CLocationMainDB", "Unknown Error : from AddItem \n");
			LOG(ERR_UK,"CLSMainDB , Unknown Error : from AddItem ");
		}

		//////// Log 남기기		
		if(m_lAddCount%2000 == 1)
		{
			//theErr.LOG(2, "CLocationMainDB", "NLS Total User cnt from ADDItem() : [%d]", m_mapUSN.Size());
			LOG(INF_UK, "CLocationMainDB, NLS Total User cnt from ADDItem() : [", m_mapUSN.Size(),"]");
			if(m_lAddCount >= 100000000)	m_lAddCount = 1;
		}
		m_lAddCount++;

		return S_INSERT_SUCCESS;
	}
	
	long lRealItemCnt = usnlist->size();
	if(lRealItemCnt < 1)
	{
		RemItem(key);
		return E_INSERT_UNKNOWN;
	}
	
	for(LIST_UINFO_ITER iter = usnlist->begin() ; iter != usnlist->end() ; iter++)
	{
#ifndef _NFGAME_
		if(baseInfo == *(NLSBaseInfo*)(*iter))
#endif
		{
			NLSBaseInfo* bInfo = (*iter);

			LONG lErr = S_INSERT_SUCCESS;
			if (NLSCLISTATUS_DISCONNECT != bInfo->m_lStatus)
			{
				tempBaseInfo = *bInfo;
				lErr = E_INSERT_USEREXIST;
			}
			else
				*bInfo = newBaseInfo;

			return lErr;
		}
	}

//  Don't Apply to NFGame
//	LONG lIdx = 0;
//	if(FindValue(key, baseInfo, lIdx))	// 같은 값이 있다는 건, 어딘가에서 메세지를 놓쳤다는 것.
//	{
//		//theErr.LOG(1, "CLocationMainDB", "ADDItem : exist same location info \n");
//		LOG(INF_UK, "CLocationMainDB, ADDItem : exist same location info \n");
//		return E_INSERT_USEREXIST;	
//	}
//
// 	if(lRealItemCnt >= MAX_LCSVALUE)		
// 	{
// 		//theErr.LOG(1, "CLocationMainDB", "ADDItem : over max value count  - additem [%d]USN : \n", Key);
// 		LOG(INF_UK, "CLocationMainDB, ADDItem : over max value count  - additem MainKey : ", key.m_lMainKey, ", SubKey : ", key.m_lSubKey, "\n");
// 		return E_INSERT_UNKNOWN;
// 	}
// 
// 	if(!_AddItem(key, baseInfo))
// 	{
// 		//theErr.LOG(1, "CLocationMainDB", "Unknown Error : from AddItem \n");
// 		LOG(INF_UK, "CLocationMainDB, Unknown Error : from AddItem \n");
// 	}

	return E_INSERT_UNKNOWN;	
}

LONG CLocationMainDB::UpdateItem(TKey key, NLSBaseInfo & baseInfo)
{
	TLock lo(this);

	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(!usnlist)
	{
		if(!_AddItem(key, baseInfo)){
			//theErr.LOG(1, "CLocationMainDB", "Unknown Error : from UpdateItem \n");
			LOG(ERR_UK, "CLocationMainDB, Unknown Error : from UpdateItem \n");
		}

//Log 를 남기기 위함.
		LONG _lcnt = m_mapUSN.Size();
		if(!(_lcnt%100))
		{
			LOG(INF_UK, "CLocationMainDB", "LCS Total User cnt from Update 1: ",_lcnt ,"\n");
		}

		return S_INSERT_SUCCESS;
	}
	LONG lRealItemCnt = usnlist->size();
	if(lRealItemCnt < 1)
	{
		RemItem(key);
		return E_INSERT_UNKNOWN;
	}

	for(LIST_UINFO_ITER iter = usnlist->begin() ; iter != usnlist->end() ; iter++)
	{
#ifndef _NFGAME_
		if(baseInfo == *(NLSBaseInfo*)(*iter))
#endif
		{
			NLSBaseInfo* bInfo = (*iter);
			*bInfo = baseInfo;
			return S_INSERT_SUCCESS;
		}
	}
	// Don't Apply to NFGame
	//if(usnlist->size() >= MAX_LCSVALUE)	
	//{
	//	//theErr.LOG(1, "CLocationMainDB", "ADDItem : over max value count - update \n");
	//	LOG(ERR_UK,"CLocationMainDB,ADDItem : over max value count - update \n");
	//	NLSBaseInfo * pInfo = (NLSBaseInfo *)(*(usnlist->begin()));
	//	_RemItem(pInfo);
	//	delete pInfo;

	//	usnlist->erase(usnlist->begin());
	//	if(!_AddItem(key, baseInfo))
	//	{
	//		//theErr.LOG(1, "CLocationMainDB", "Unknown Error : from UpdateItem \n");
	//		LOG(ERR_UK,"CLocationMainDB,Unknown Error : from UpdateItem \n");
	//	}
	//}
	return S_INSERT_SUCCESS;
}

BOOL CLocationMainDB::_AddItem(TKey key, NLSBaseInfo & info)
{

#ifdef USE_HEAPMEMPOOL
	LONG _size = info.GetSize();
	if( _size > LIMITSIZE_LCSBASEINFO)
	{
		LOG(INF_UK, "CLocationMainDB, Too large, NLSBaseInfo option data length  [ ",_size," ]\n");
		return FALSE;
	}
	NLSBaseInfo * pbase = (NLSBaseInfo *)(m_LCMemoryPool.Pop());
	if(!pbase) return FALSE;

	*pbase = info;
	pbase->m_sOption.assign(info.m_sOption.c_str(), info.m_sOption.length());

#else
	NLSBaseInfo *pbase = new NLSBaseInfo(info);
	if(!pbase) return FALSE;

#endif

	LIST_UINFO	lstUInfo;
	lstUInfo.push_back(pbase);
	return m_mapUSN.Insert(key.m_lMainKey, lstUInfo);

	/*
	LIST_UINFO* roomlist = m_mapRoomID.Find(info.m_grid);
	if(!roomlist) {
	LIST_UINFO lstUInfo;
	m_mapRoomID.Insert(info.m_grid, lstUInfo);
	roomlist = m_mapRoomID.Find(info.m_grid);
	}
	roomlist->push_back(pbase);
	*/
}

void CLocationMainDB::RemItem(TKey key)		//usn 하나를 제거.
{
	TLock lo(this);

	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(!usnlist)
		return; 

	LONG _cnt = usnlist->size();
	if(_cnt > 0)
	{
		ForEachElmt(LIST_UINFO, *usnlist, iter, iter1)
		{
			NLSBaseInfo* pInfo = (*iter);
			if(!pInfo) continue;
			_RemItem(pInfo);
//			theServerListEx.RemUSN(pInfo->m_nsap, Key);
			pInfo->Clear();
//#pragma oMSG("Heap Memory Pool 관련 처리.. 보완 필요")
#ifdef USE_HEAPMEMPOOL
			m_LCMemoryPool.Push(pInfo);
#else
			delete pInfo;
			usnlist->erase(iter);
#endif
		}
	}
	m_mapUSN.Delete(key.m_lMainKey);
}

void CLocationMainDB::_RemItem(NLSBaseInfo * pbaseInfo)	// usn 하나를 제거.
{
	// m_mapRoomID : room ID별로 관리되는 map에서 일치하는 정보의 link만 해제
	// 실제 instance의 제거는 이 함수를 caller가 담당한다.
	if(!pbaseInfo) return;
}

void CLocationMainDB::RemValue(TKey key, NLSBaseInfo & baseInfo)
{
}



void CLocationMainDB::RemValue(TKey key, NSAP & nsap)
{
	TLock lo(this);
	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(!usnlist) return;
	if(usnlist->empty())
	{
		//theErr.LOG(1, "CLocationMainDB", "exist empty location info. item : RemValue %d ", Key);
		LOG(INF_UK, "CLocationMainDB,exist empty location info. item : RemValue Key");
		RemItem(key);
		return;
	}
	ForEachElmt(LIST_UINFO, *usnlist, iter, iter1)
	{
		NLSBaseInfo * pInfo = (NLSBaseInfo *)(*iter);
		if(!pInfo) usnlist->erase(iter);
		else
		{
			_RemItem(pInfo);
//			theServerListEx.RemUSN(pInfo->m_nsap, Key);
			pInfo->Clear();
//#pragma oMSG("Heap Memory Pool 관련 처리.. 보완 필요")
#ifdef USE_HEAPMEMPOOL
			m_LCMemoryPool.Push(pInfo);
#else if
			delete pInfo;
#endif
			usnlist->erase(iter);
		}
	}
	if(usnlist->empty()) RemItem(key);
}

void CLocationMainDB::RemoveValue(const LRBAddress& addr)
{
	TLock lo(this);
	LONG lCnt = 0;
	typedef map<LONGLONG, LIST_UINFO> TMAPUINFO;
	TMAPUINFO* mapUSN = m_mapUSN.GetData();
	ForEachElmt(TMAPUINFO, *mapUSN, iter, iter1)
	{
		LIST_UINFO& userlist = iter->second;
		if(userlist.empty()) continue;
		ForEachElmt(LIST_UINFO, userlist, itu, itu1)
		{
			NLSBaseInfo * pInfo = (NLSBaseInfo *)(*itu);
			if (pInfo != NULL)
			{
				if (pInfo->m_serverLRBAddr == addr) {
					pInfo->Clear();
					delete pInfo;
				}
				userlist.erase(itu);
			}
		}
		mapUSN->erase(iter);
		lCnt++;
	}
	LOG(ERR_UK, "CLocationMainDB, RemoveValue totUsercnt :", lCnt, " // dis server addr :", addr.GetString());
}

void CLocationMainDB::ClearLCInfo()
{
	TLock lo(this);
	map<LONGLONG, LIST_UINFO>* mapUSN = m_mapUSN.GetData();
	for(map<LONGLONG, LIST_UINFO>::iterator iter = mapUSN->begin() ; iter != mapUSN->end() ; iter++)
	{
		LIST_UINFO& userlist = iter->second;
		if(userlist.empty()) continue;
		for(LIST_UINFO_ITER itu = userlist.begin() ; itu != userlist.end(); itu++)
		{
			NLSBaseInfo * pInfo = (NLSBaseInfo *)(*itu);
			if (pInfo != NULL)
			{
				pInfo->Clear();
//#pragma oMSG("Heap Memory Pool 관련 처리.. 보완 필요")
#ifdef USE_HEAPMEMPOOL
				m_LCMemoryPool.Push(pInfo);
#else if
				delete pInfo;
#endif
			}
		}
	}
	m_mapUSN.Clear();
}

BOOL CLocationMainDB::FindKey(TKey key)
{
	TLock lo(this);
	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(!usnlist) return FALSE;
	return TRUE;
}

BOOL CLocationMainDB::FindKeyEx(TKey key, LONG lIP)
{
	TLock lo(this);
	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(!usnlist) return FALSE;
	else 
	{
		ForEachElmt(LIST_UINFO, *usnlist, it1, jt1)
		{
			NLSBaseInfo * info = (NLSBaseInfo *)(*it1);
			if (info->m_lClientIP == lIP) return TRUE;
		}
	}
	return FALSE;
}

//#pragma oMSG("나중에 이 함수에 의해 중복허용을 어디까지 할 수 있을지 결정할 수 있음")
BOOL CLocationMainDB::FindValue(TKey key, const NLSBaseInfo & baseInfo, LONG& lIndex)		
{
	TLock lo(this);
	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(!usnlist) return FALSE;
	if(usnlist->empty())
	{
		m_mapUSN.Delete(key.m_lMainKey);
		return FALSE;
	}
	lIndex = 0L;
	if(usnlist->size() >= MAX_LCSVALUE) return TRUE;
	ForEachElmt(LIST_UINFO, *usnlist, it1, jt1)
	{
		NLSBaseInfo * pinfo = (NLSBaseInfo *)(*it1);
		if((*pinfo) == baseInfo) return TRUE;
		lIndex++;
	}
	return FALSE;
}

BOOL CLocationMainDB::GetItem(TKey key, NLSBaseInfo & baseInfo)
{
	BOOL bRet = FALSE;

	TLock lo(this);

	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(usnlist)
	{
		if(usnlist->empty())
			m_mapUSN.Delete(key.m_lMainKey);
		else
		{
			for(LIST_UINFO_ITER iter = usnlist->begin() ; iter != usnlist->end() ; iter++)
			{
				NLSBaseInfo* bInfo = (*iter);
				baseInfo = *bInfo;
				bRet = TRUE;
				break;
			}
		}
	}
	return bRet;
}


BOOL CLocationMainDB::GetItem(TKey key, NLSBaseInfoList & lstBaseInfo)
{
	BOOL bRet = FALSE;
	TLock lo(this);

	LIST_UINFO* usnlist = m_mapUSN.Find(key.m_lMainKey);
	if(usnlist)
	{
		if(usnlist->empty())
			m_mapUSN.Delete(key.m_lMainKey);
		else
		{
			lstBaseInfo.clear();
			for(LIST_UINFO_ITER iter = usnlist->begin() ; iter != usnlist->end() ; iter++)
				lstBaseInfo.push_back(**iter);	/// iter은 포인터 데이타 값이라서리...
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL CLocationMainDB::GetItem(TKey key, LONG lIndex, NLSBaseInfo & info)
{
	TLock lo(this);

	NLSBaseInfoList lstInfo;
	if(!GetItem(key, lstInfo))
		return FALSE;
	if(lIndex > (LONG)lstInfo.size())
		return FALSE;

	return TRUE;	
}


LONG CLocationMainDB::GetItemCnt()
{
	TLock lo(this);
	return m_mapUSN.Size();
}

LONG CLocationMainDB::GetValueCnt(char Key)
{
	TLock lo(this);
	LIST_UINFO* usnlist = m_mapUSN.Find(Key);
	if(!usnlist) return 0L;
	return usnlist->size();
}

BOOL CLocationMainDB::AddRegionStat(char regionID)
{
	long *lstat;
	
	if (!(lstat = m_mapRegionStat.Find(regionID)))	{
		m_mapRegionStat.Insert(regionID, 1);
		return 0;
	}
	else	{
		m_mapRegionStat.Update(regionID, (*lstat) + 1);
	}

	return true;
}

BOOL CLocationMainDB::SubstractRegionStat(char regionID)
{
	long *lstat;
	if (!(lstat = m_mapRegionStat.Find(regionID)))	{
		return 0;
	}
	else	{
		if (*lstat > 0)	{
			m_mapRegionStat.Update(regionID, (*lstat) - 1);
		}
	}

	return true;
}


LONG CLocationMainDB::GetRegionStat(char regionID)
{
	long *lstat;
	if (!(lstat = m_mapRegionStat.Find(regionID)))	{
		return 0;
	}
	else	{
		return *lstat;
	}
}



int GetLocationMainDB(long ssn)
{
	map<long, long>::iterator iter = gMapIndexForMainDB.find(ssn);
	if(iter == gMapIndexForMainDB.end())
		return 0;
	else
		return iter->second;
}

