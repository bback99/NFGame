// 
// GameSvrTable.cpp
// 

#include "stdafx.h"
#include "GameSvrTable.h"

#define LOAD_ROOM_FACTOR		70
#define LOAD_USER_FACTOR		30
#define LIMIT_ROOM_COUNT		3000
#define LIMIT_USER_COUNT		5000

GameSvrTable theGameSvrTable;

////////////////////////////////////////////////////////////////////
// GLSInfo
GLSInfo::GLSInfo()
{
	m_lRoomCount = 0L;
	m_lUserCount = 0L;
//	m_lServerID	= 0L;
	m_lServerType = 0L;
	m_NSAPInfo.Clear();
	m_lLastUpdateTime = 0L;
}

GLSInfo::GLSInfo(NSAP& nsap, LRBAddress lrbAddr)
{
	m_lRoomCount = 0L;
	m_lUserCount = 0L;
	m_lServerID	= lrbAddr;
	m_lServerType = 0L;
	m_NSAPInfo = nsap;
	m_lLastUpdateTime = 0L;
}

GLSInfo::GLSInfo(ServerRegistration& serverInfo)
{
	m_lRoomCount = 0L;
	m_lUserCount = 0L;
	m_lServerID = serverInfo.m_lServerID;
	m_lServerType = serverInfo.m_lServerType;
	m_NSAPInfo = serverInfo.m_NSAPInfo;
	m_lLastUpdateTime = 0L;
}

GLSInfo::~GLSInfo()
{
	Clear();
}

void GLSInfo::Clear()
{
	TBase::Clear();
	m_lRoomCount = -1L;
	m_lUserCount = -1L;
	m_lServiceTypeID = -1L;
	m_lLastUpdateTime = 0L;
}

BOOL GLSInfo::IsActive(time_t lTime)
{
	if (lTime - m_lLastUpdateTime < 35) return TRUE;
	return FALSE;
}

////////////////////////////////////////////////////////////////////
//extenal interface
LONG GLSInfo::ComputeLoad(time_t lTime)
{
	LONG lLoad = (IsActive(lTime) ? 0 : (LONG_MAX/2));

	lLoad += (LOAD_ROOM_FACTOR * m_lRoomCount) + (LOAD_USER_FACTOR * m_lUserCount);
	if (lLoad < 0) lLoad = LONG_MAX;
	return lLoad;
}

#ifdef _BELB_
// Relay-Room 
// 중계방은 Room의 수보다 인원의 비율이 높아야 하므로 normal-room의 비율 (7:3)의 반대로 계산한다. -bback99
LONG GLSInfo::ComputeLoad_RelayRoom(time_t lTime)
{
	LONG lLoad = (IsActive(lTime) ? 0 : (LONG_MAX/2));

	lLoad += (LOAD_USER_FACTOR * m_lRoomCount) + (LOAD_ROOM_FACTOR * m_lUserCount);
	if (lLoad < 0) lLoad = LONG_MAX;
	return lLoad;
}
#endif

////////////////////////////////////////////////////////////////////
// GameSvrList
GameSvrList::GameSvrList(LONG lServerTypeID) : m_lServiceTypeID(lServerTypeID)
{
}

GameSvrList::~GameSvrList()
{
	Clear();
}
////////////////////////////////////////////////////////////////////
// External Interface
BOOL GameSvrList::AddGameSvr(GLSInfo* pInfo)
{
	ASSERT(pInfo != NULL);
	{
	// ----------------------------------------
		ForEachElmt(TList, m_lstGLS, i, j)
		{
			GLSInfo* pGLS = *i;
			if (pGLS->m_lrbLogicAddr == pInfo->m_lrbLogicAddr)
				return FALSE;
		}
		m_lstGLS.push_back(pInfo);
		return TRUE;
	// ----------------------------------------
	}
}

BOOL GameSvrList::UpdateGameSvr(LRBAddress lrbLogicAddr, NSAP& nsap, LONG lRoomCount, LONG lUserCount, time_t lTime)
{
	{
	// -----------------------------------------
		ForEachElmt(TList, m_lstGLS, i, j)
		{
			GLSInfo* pInfo = *i;
			if (!pInfo)
				continue;
			if(pInfo->GetLogicAddr() == lrbLogicAddr)
			{
				pInfo->SetLoadInfo(lRoomCount, lUserCount, lTime);
				return TRUE;
			}
		}
		GLSInfo* pInfo = new GLSInfo();
		pInfo->SetLoadInfo(lRoomCount, lUserCount, lTime);
		pInfo->m_NSAPInfo = nsap;
		pInfo->SetLogicAddr(lrbLogicAddr);

		AddGameSvr(pInfo);
	// -----------------------------------------
		return TRUE;
	}
}

#ifdef _BELB_
BOOL GameSvrList::BetRoomIncGameSvr(LRBAddress lrbLogicAddr, time_t lTime)
{
	// -----------------------------------------
	ForEachElmt(TList, m_lstGLS, i, j)
	{
		GLSInfo* pInfo = *i;
		if (!pInfo)
			continue;
		if(pInfo->GetLogicAddr() == lrbLogicAddr)
		{
			pInfo->SetBetRoomInc();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GameSvrList::BetRoomDecGameSvr(LRBAddress lrbLogicAddr, time_t lTime)
{
	ForEachElmt(TList, m_lstGLS, i, j)
	{
		GLSInfo* pInfo = *i;
		if (!pInfo)
			continue;
		if(pInfo->GetLogicAddr() == lrbLogicAddr)
		{
			pInfo->SetBetRoomDec();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL GameSvrList::FindRelayRoomSvr(LRBAddress& lrbAddress)
{
	BOOL bFound = FALSE;
	GLSInfo* pMinInfo = NULL;
	LONG lMinLoad = LONG_MAX;
	time_t lTime = ::time(0);

	ForEachElmt(TList, m_lstGLS, i, j)
	{
		GLSInfo* pInfo = *i;
		if (!pInfo)
			continue;
		LONG lLoad = pInfo->ComputeLoad_RelayRoom(lTime);
		if (lMinLoad >= lLoad)
		{
			bFound = TRUE;
			pMinInfo = pInfo;
			lMinLoad = lLoad;
		}
	}
	if (bFound)
	{
		lrbAddress = pMinInfo->GetLogicAddr();
	}
	return bFound;
}
#endif

BOOL GameSvrList::DeleteGameSvr(LRBAddress lrbLogicAddr)
{	
	{
	// -----------------------------------------
		BOOL bRetVal = FALSE;

		ForEachElmt(TList, m_lstGLS, i, j)
		{
			GLSInfo* pInfo = *i;
			if (!pInfo)
				continue;
			if (pInfo->GetLogicAddr() == lrbLogicAddr)
			{
				m_lstGLS.erase(i);
				delete pInfo;
				bRetVal = TRUE;
			}
		}
		return bRetVal;
	// -----------------------------------------
	}
}

void GameSvrList::Clear()
{
	{
	// ----------------------------------------
		ForEachElmt(TList, m_lstGLS, i, j)
		{
			GLSInfo* pInfo = *i;
			if (!pInfo)
				continue;
			delete pInfo;
		}
		m_lstGLS.clear();
	// ----------------------------------------
	}
}

BOOL GameSvrList::FindMinGameSvr(NSAP& nsap)
{
	BOOL bFound = FALSE;
	GLSInfo* pMinInfo = NULL;
	LONG lMinLoad = LONG_MAX;
	time_t lTime = ::time(0);

	ForEachElmt(TList, m_lstGLS, i, j)
	{
		GLSInfo* pInfo = *i;
		if (!pInfo)
			continue;
		LONG lLoad = pInfo->ComputeLoad(lTime);
		if (lMinLoad >= lLoad)
		{
			bFound = TRUE;
			pMinInfo = pInfo;
			lMinLoad = lLoad;
		}
	}
	if (bFound)
	{
		nsap = pMinInfo->m_NSAPInfo;
	}
	return bFound;
}

BOOL GameSvrList::FindMinGameSvr(LRBAddress& lrbAddress)
{
	BOOL bFound = FALSE;
	GLSInfo* pMinInfo = NULL;
	LONG lMinLoad = LONG_MAX;
	time_t lTime = ::time(0);

	ForEachElmt(TList, m_lstGLS, i, j)
	{
		GLSInfo* pInfo = *i;
		if (!pInfo)
			continue;
		LONG lLoad = pInfo->ComputeLoad(lTime);
		if (lMinLoad >= lLoad)
		{
			bFound = TRUE;
			pMinInfo = pInfo;
			lMinLoad = lLoad;
		}
	}
	if (bFound)
	{
		lrbAddress = pMinInfo->GetLogicAddr();
	}
	return bFound;
}

/*BOOL GameSvrList::FindModGameSvr(NSAP& nsap, LONG lIndex, LONG& lCntOfGLS)
{
	BOOL bFound = FALSE;

	LONG lTempIndex = 0L;

	lCntOfGLS = GetRegistedGlsCount();
	LONG lModIndex = lIndex % lCntOfGLS;

	ForEachElmt(TList, m_lstGLS, i, j)
	{
		GLSInfo* pInfo = *i;
		if (!pInfo)
			continue;

		if (lModIndex == lTempIndex)
		{
			bFound = TRUE;
			nsap = pInfo->m_NSAPInfo;
		}

		lTempIndex++;
	}

	return bFound;
}*/

/////////////////////////////////////////////////////////////////////
// GameSvrTable
GameSvrTable::GameSvrTable()
{
}

GameSvrTable::~GameSvrTable()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// extenal interface
BOOL GameSvrTable::AddGameSvr(LONG lServiceTypeID, GLSInfo* pGlsInfo)
{
	ASSERT(pGlsInfo != NULL);
	
	TLock lo(this);
	{
	// -------------------------------------------
		TMap::iterator it = m_glsTable.find(lServiceTypeID);
		if (it == m_glsTable.end())
		{
			GameSvrList* pList = new GameSvrList(lServiceTypeID);
			pList->SetServiceTypeID(lServiceTypeID);
			pList->AddGameSvr(pGlsInfo);
			m_glsTable[lServiceTypeID] = pList;
			LOG (INF, "GameSvrTable : AddGameSvr ADD New ServiceType GLS : [",lServiceTypeID,"] ");
			return TRUE;
		}
		else
		{
			GameSvrList* pList = it->second;
			if (!pList)
				return FALSE;
			return pList->AddGameSvr(pGlsInfo);
		}
	// -------------------------------------------
	}
}

BOOL GameSvrTable::UpdateGameSvr(LONG lTypeID, LRBAddress lrbLogicAddr, NSAP& nsap, LONG lRoomCount, LONG lUserCount)
{
	TLock lo(this);
	time_t lTime = ::time(0);
	{
	// -------------------------------------------
		TMap::iterator it = m_glsTable.find(lTypeID);
		GameSvrList* pList = NULL;
		if (it == m_glsTable.end())
		{
			pList = new GameSvrList(lTypeID);

			//----------------------------------
			GLSInfo* pInfo = new GLSInfo;
			pInfo->SetLoadInfo(lRoomCount, lUserCount, lTime);
			pInfo->m_NSAPInfo = nsap;
			pInfo->SetLogicAddr(lrbLogicAddr);

			pList->AddGameSvr(pInfo);
			//-----------------------------------
//			pList->UpdateGameSvr(nsap, lRoomCount, lUserCount);
			m_glsTable[lTypeID] = pList;
			return TRUE;
		}
		else
		{
			pList = it->second;
			if (!pList)
				return FALSE;
			return pList->UpdateGameSvr(lrbLogicAddr, nsap, lRoomCount, lUserCount, lTime);
		}
	// -------------------------------------------
	}
}

BOOL GameSvrTable::DeleteGameSvr(LRBAddress lrgLogicAddr)
{
	BOOL bRet = FALSE;
	TLock lo(this);
	{
		ForEachElmt(TMap, m_glsTable, i, j)
		{
			GameSvrList* pGameSvrList = i->second;
			if(!pGameSvrList)
				continue;

			bRet = pGameSvrList->DeleteGameSvr(lrgLogicAddr);
			if (bRet) 
			{	//pGameSvrList는 있지만 실제 LogicAddr이 match되는 GLS가 없다.
				if(pGameSvrList->GetRegistedGlsCount() == 0)
				{
#pragma oMSG("TODO : 처리 할것")
					m_glsTable.erase(i);
					delete pGameSvrList;
				}
#pragma oMSG("TODO : GameSvr삭제후 서비스 type을 리턴받아 해당 타입의 list가 0라면 해당 타입을 map에서 제거 할것.")
				// break;
			}
		}
	}
	return bRet;
}


void GameSvrTable::Clear()
{
	TLock lo(this);
	{
	// -------------------------------------------------
		ForEachElmt(TMap, m_glsTable, i, j)
		{
			GameSvrList* pList = i->second;
			if(pList)
			{
				pList->Clear();
				delete pList;
			}
		}
		m_glsTable.clear();
	// -------------------------------------------------
	}
}

BOOL GameSvrTable::FindProperGameSvr(LONG lType, NSAP& nsap, DWORD dwRoomIndex, LONG& lCntOfGLS)
{
	BOOL bRet = FALSE;
	{
		TLock lo(this);

		// -------------------------------------------------
		if (dwRoomIndex == 0L)
		{
			TMap::iterator it = m_glsTable.find(lType);
			if (it == m_glsTable.end())
			{
				//search from GLS type ANY
				TMap::iterator it1 = m_glsTable.find(LOWORD(SVCTYP_ANY));
				if(it1 == m_glsTable.end())
					return bRet;
				else
				{
					GameSvrList* pList = it1->second;
					if (!pList)
						return bRet;
					bRet = pList->FindMinGameSvr(nsap);
				}		
			}
			else
			{
				GameSvrList* pList = it->second;
				if (!pList)
					return bRet;
				bRet = pList->FindMinGameSvr(nsap);
			}

			lCntOfGLS = 0;
		}
		else
		{
			LOG(ERR, "Invalid Call!");
/*			TMap::iterator it = m_glsTable.find(lType);

			if (it == m_glsTable.end())
			{
				lCntOfGLS = 0;

				return bRet;
			}
			else
			{
				GameSvrList* pList = it->second;
				if (!pList)
					return bRet;

				bRet = pList->FindModGameSvr(nsap, dwRoomIndex, lCntOfGLS);
			}*/
		}
	// -------------------------------------------------
	}
	return bRet;
}

#ifdef _BELB_
BOOL GameSvrTable::BetRoomIncrement(LONG lTypeID, LRBAddress lrbLogicAddr)
{
	TLock lo(this);
	time_t lTime = ::time(0);
	{
		// -------------------------------------------
		TMap::iterator it = m_glsTable.find(lTypeID);
		GameSvrList* pList = NULL;
		if (it == m_glsTable.end())
		{
			LOG (LRBERR, "GameSvrTable - BetRoomIncrement not Found");
			return TRUE;
		}
		else
		{
			pList = it->second;
			if (!pList)
				return FALSE;
			return pList->BetRoomIncGameSvr(lrbLogicAddr, lTime);
		}
		// -------------------------------------------
	}
}


BOOL GameSvrTable::BetRoomDecrement(LONG lTypeID, LRBAddress lrbLogicAddr)
{
	TLock lo(this);
	time_t lTime = ::time(0);
	{
		// -------------------------------------------
		TMap::iterator it = m_glsTable.find(lTypeID);
		GameSvrList* pList = NULL;
		if (it == m_glsTable.end())
		{
			LOG (LRBERR, "GameSvrTable - BetRoomDecrement not Found");
			return TRUE;
		}
		else
		{
			pList = it->second;
			if (!pList)
				return FALSE;
			return pList->BetRoomDecGameSvr(lrbLogicAddr, lTime);
		}
		// -------------------------------------------
	}
}

BOOL GameSvrTable::FindProperRoomSvr(LRBAddress& lrbAddress, LONG lIsRelayRoom)
{
	BOOL bRet = FALSE;

	if (!lIsRelayRoom)			// Normal-Room
	{
		TLock lo(this);

		TMap::iterator it = m_glsTable.find(BADUK_SSN);
		if (it == m_glsTable.end())
		{
			//search from GLS type ANY
			TMap::iterator it1 = m_glsTable.find(LOWORD(SVCTYP_ANY));
			if(it1 == m_glsTable.end())
				return bRet;
			else
			{
				GameSvrList* pList = it1->second;
				if (!pList)
					return bRet;
				bRet = pList->FindMinGameSvr(lrbAddress);
			}		
		}
		else
		{
			GameSvrList* pList = it->second;
			if (!pList)
				return bRet;
			bRet = pList->FindMinGameSvr(lrbAddress);
		}
	}
	else						// Relay-Room
	{
		TLock lo(this);

		TMap::iterator it = m_glsTable.find(BADUK_SSN_RELAY_ROOM);
		if (it == m_glsTable.end())
		{
			//search from GLS type ANY
			TMap::iterator it1 = m_glsTable.find(LOWORD(SVCTYP_ANY));
			if(it1 == m_glsTable.end())
				return bRet;
			else
			{
				GameSvrList* pList = it1->second;
				if (!pList)
					return bRet;
				bRet = pList->FindRelayRoomSvr(lrbAddress);
			}		
		}
		else
		{
			GameSvrList* pList = it->second;
			if (!pList)
				return bRet;
			bRet = pList->FindRelayRoomSvr(lrbAddress);
		}
	}

	return bRet;
}
#endif

BOOL GameSvrTable::Init()
{
	TLock lo(this);
	Clear();
	return TRUE;
}
void GameSvrTable::AllGameSvrList(AMS_GLSInfoList& rlstAMSGlsInfo)
{
	TLock lo(this);

	// -------------------------------------------------
	rlstAMSGlsInfo.clear();
	ForEachElmt(TMap, m_glsTable, i, j)
	{
		GameSvrList* plstGLSInfo = i->second;
		if (!plstGLSInfo)
			continue;

		ForEachElmt(GameSvrList::TList, plstGLSInfo->m_lstGLS, i1, j1)
		{
			GLSInfo* pGls = (*i1);
			if (!pGls)
				continue;

			rlstAMSGlsInfo.push_back(AMS_GLSInfo());

			AMS_GLSInfo& NDLGls = rlstAMSGlsInfo.back();

			NDLGls.m_lLogicalAddr	= pGls->GetLogicAddr();
			NDLGls.m_lRoomCount		= pGls->GetRoomCount();
			NDLGls.m_lSSN			= pGls->GetServiceTypeID(); //SSN == ServiceTypeID
			NDLGls.m_lUserCount		= pGls->GetUserCount();
			NDLGls.m_nsapGLS		= pGls->m_NSAPInfo;
		}
	}
	// -------------------------------------------------
}
