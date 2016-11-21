// LCSManager.cpp: implementation of the LCSManagerData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NLSManager.h"
#include "Common.h"

#ifdef _GLSLCS
	#define _USELCS
#endif
#ifdef _CHSNLS
	#define _USELCS
#endif

#ifdef _USELCS

#ifdef _CHSNLS
#include "ChannelDir.h"
#include "Channel.h"
#include "LRBManager.h"
#include "Listener.h"
#define CRF_DBERROR CHS_UNKNOWN
#define JRF_DBERROR CHS_UNKNOWN
#endif

#ifdef _GLSLCS
#include "RoomTable.h"
#include "Room.h"
#include "LrbConnector.h"
#include "Listener.h"

#ifndef _NFGAME_
#include "ADL/MsgCHSGLS.h"
#else
#include "ADL/MsgCHSNGS.h"
#endif

#include "Control.h"
#endif

#include <PMSConnObject.h>


//////////////////////////////////////////////////////////////////////
// LCSIPSet
//////////////////////////////////////////////////////////////////////

LONG LCSIPSet::ExtractIndex(LONG lKey)					// USN���� ������ Index�� �����Ѵ�.
{
	if(lKey < 1) 
		return -1L;
	if(m_lLCSCount < 1) 
		return -1L;

	return (lKey % m_lLCSCount);
}

LONG LCSIPSet::FindIP(LRBAddress dwIP, LONG &lSubIndex, BOOL bOnOff)	// TRUE == ON, FALSE == On and Off return,  return Index
{
	if(bOnOff)
	{
		for(int i = 0; i < m_lLCSCount; i++)
		{
			if( (dwIP == m_vecIPSet[i].first.m_dwIP && m_vecIPSet[i].first.m_bOnOff) )
			{
				lSubIndex = SUBINDEX_MASTER;		// master
				return i;
			}
			if( (dwIP == m_vecIPSet[i].second.m_dwIP && m_vecIPSet[i].second.m_bOnOff) )
			{
				lSubIndex = SUBINDEX_SLAVE;		// slave
				return i;
			}
		}
	}
	else 
	{
		for(int i = 0; i < m_lLCSCount; i++)
		{
			if( dwIP == m_vecIPSet[i].first.m_dwIP)
			{
				lSubIndex = SUBINDEX_MASTER;
				return i;
			}
			if( dwIP == m_vecIPSet[i].second.m_dwIP)
			{
				lSubIndex = SUBINDEX_SLAVE;
				return i;
			}
		}
		
	}
	return -1L;
}

LONG LCSIPSet::FindAddr(LRBAddress dwAddr, LONG &lSubIndex, BOOL bOnOff)	// TRUE == ON, FALSE == On and Off return,  return Index
{
	if(bOnOff)
	{
		for(int i = 0; i < m_lLCSCount; i++)
		{
			if( (dwAddr == m_vecIPSet[i].first.m_dwAddr && m_vecIPSet[i].first.m_bOnOff) )
			{
				lSubIndex = SUBINDEX_MASTER;
				return i;
			}
			else if( (dwAddr == m_vecIPSet[i].second.m_dwAddr && m_vecIPSet[i].second.m_bOnOff) )
			{
				lSubIndex = SUBINDEX_SLAVE;
				return i;
			}
		}
	}
	else 
	{
		for(int i = 0; i < m_lLCSCount; i++)
		{
			if( dwAddr == m_vecIPSet[i].first.m_dwAddr )
			{
				lSubIndex = SUBINDEX_MASTER;
				return i;
			}
			if( dwAddr == m_vecIPSet[i].second.m_dwAddr)
			{
				lSubIndex = SUBINDEX_SLAVE;
				return i;
			}
		}
		
	}
	return -1L;
}

BOOL LCSIPSet::ChangeAddrSetOnOff(LRBAddress dwAddr, BOOL bOnOff)		// On or Off
{
	LONG _subindex = 0L;
	LONG index = FindAddr(dwAddr, _subindex, FALSE);
	if(index < 0)
		return FALSE;
	if(_subindex == SUBINDEX_MASTER)	// first �� Master..
		m_vecIPSet[index].first.m_bOnOff = bOnOff;
	else 
		m_vecIPSet[index].second.m_bOnOff = bOnOff;
	return TRUE;
}

BOOL LCSIPSet::ChangeAddrSetMode(LRBAddress dwAddr, LONG lMode)		// Master or Slave�� ����
{
	LONG _subindex = 0L;
	LONG index = FindAddr(dwAddr, _subindex, FALSE);
	if(index < 0)
		return FALSE;
	SET	_tempSET;
	if( (SUBINDEX_MASTER == _subindex && lMode == LCSMODE_SLAVE) || (SUBINDEX_SLAVE == _subindex && lMode == LCSMODE_MASTER) )
	{
		swap(m_vecIPSet[index].first, m_vecIPSet[index].second);
	}
	return TRUE;
}

// ó���� ��� ������ ����Ѵ�. ��� LCS�� IPSet ������ �����ؾ� ��.
// �� ���Ĵ� Master/Slave�� ���� �� On/Off�� �����Ѵ�.
BOOL LCSIPSet::InitIPSet(const vector<LRBAddress> & vecIPSet, LRBAddress dwMasterIP, LRBAddress dwAddr)
{	
	TLock lo(this);

	LONG lSize = vecIPSet.size();
	if(lSize < 1)
		return FALSE;
	m_lLCSCount = lSize / 2;

//	m_vecIPSet.clear();
	if(m_vecIPSet.size() < 1)	// ó�� �͸� ��� ����Ѵ�.
	{
		for(int i = 0; i < lSize; )
		{
			LRBAddress m_empty;
			m_empty.Clear();

			IPUNIT master(vecIPSet[i++], m_empty, FALSE);
			IPUNIT slave(vecIPSet[i++], m_empty, FALSE);
			SET set(master, slave);
			
			m_vecIPSet.push_back(set);
		}
		theLog.Put(WAR_UK, "GLS_LCSIPSet"_COMMA, "====== Registered LCS count : [", (i+1)/2, "] ======");
	}

	UpdateLogacalAddr(dwMasterIP, dwAddr);
	// Master�� ������ �̵���Ű��, ON ���·� �ٲ۴�.
	if(!ChangeAddrSetMode(dwAddr, LCSMODE_MASTER) || !ChangeAddrSetOnOff(dwAddr, TRUE))
	{
		theLog.Put(WAR_UK, "GLS_LCSIPSet"_COMMA, "Fatal Error -- not registered master ip[", dwAddr.GetString().c_str(), "] - LCSIPSet::InitIPSet");
		return FALSE;
	}
	return TRUE;
}

void LCSIPSet::UpdateLogacalAddr(LRBAddress dwIP, LRBAddress dwAddr)
{
	LONG _lsub = 0L;
	LONG _index = FindIP(dwIP, _lsub, FALSE);
	if(_index < 0)
		return;
	if(_lsub == SUBINDEX_MASTER)
		m_vecIPSet[_index].first.m_dwAddr = dwAddr;
	else
		m_vecIPSet[_index].second.m_dwAddr = dwAddr;
}

void LCSIPSet::StopLCS(LRBAddress dwAddr)	// Master���� �� �޼����� ������ �ִ�.
{
	TLock lo(this);
	LONG _lSubIdx = 0L;
	LONG lIdx = FindAddr(dwAddr, _lSubIdx);

	if(lIdx < 0 || _lSubIdx == SUBINDEX_SLAVE)	//�������� �ʰų�, slave LCS�̹Ƿ� �����Ѵ�.
	{
		theLog.Put(WAR_UK, "GLS_LCSIPSet"_COMMA, "not found masterIP or not master IP..[", dwAddr.GetString().c_str(), "]dwAddr");
		return ;
	}

	ChangeAddrSetMode(dwAddr, LCSMODE_SLAVE);
	if(!ChangeAddrSetOnOff(dwAddr, FALSE))
		//theLog.Put(INF_UK, "GLS_LCSIPSet"_COMMA, "fail StopLCS.. %d \n", dwAddr);
		theLog.Put(INF_UK, "GLS_LCSIPSet"_COMMA, "fail StopLCS.. ", dwAddr.GetString().c_str());
}

BOOL LCSIPSet::StartLCS(LRBAddress dwAddr)
{
	TLock lo(this);
	LONG _lSubIdx = 0L;

	LONG lIdx = FindAddr(dwAddr, _lSubIdx, FALSE);

	if(lIdx < 0)	// �������� ����.
	{
		string strAddr;
		dwAddr.GetStringFormat(strAddr);
		theLog.Put(WAR_UK, "GLS_LCSIPSet"_COMMA, "not found masterIP[", strAddr.c_str(), "] ..\n");
		return FALSE;
	}
	if(_lSubIdx == SUBINDEX_MASTER)	// �̹� master�� ��ϵ� ���̹Ƿ� .. �ѱ⸸ �Ѵ�.
	{
		string strAddr;
		dwAddr.GetStringFormat(strAddr);
		if(!ChangeAddrSetOnOff(dwAddr, TRUE))
		{
			//theLog.Put(INF_UK, "GLS_LCSIPSet", "fail StartLCS - MASTER : [%s] \n", strAddr.c_str());
			theLog.Put(INF_UK, "GLS_LCSIPSet"_COMMA, "fail StartLCS - MASTER : [", strAddr.c_str(), "] \n");
			return FALSE;
		}
		//theLog.Put(INF_UK, "GLS_LCSIPSet", "success LCS - MASTER Registration [%s] \n", strAddr.c_str());
		theLog.Put(INF_UK, "GLS_LCSIPSet"_COMMA, "success LCS - MASTER Registration [", strAddr.c_str(), "]");
		return TRUE;
	}

	ChangeAddrSetMode(dwAddr, LCSMODE_MASTER);
	if(!ChangeAddrSetOnOff(dwAddr, TRUE))
	{
		//theLog.Put(INF_UK, "GLS_LCSIPSet", "fail StartLCS : %d \n", dwAddr);
		theLog.Put(INF_UK, "GLS_LCSIPSet"_COMMA, "fail StartLCS : ", dwAddr.GetString().c_str());
		return FALSE;
	}
	else
	{
		//theLog.Put(INF_UK, "GLS_LCSIPSet", "success LCS Registration %d \n", dwAddr);
		theLog.Put(INF_UK, "GLS_LCSIPSet"_COMMA, "success LCS Registration ", dwAddr.GetString().c_str());
		return TRUE;
	}
}

// ������ LCS master�� Off�� ��� ������ return�Ѵ�.
LRBAddress LCSIPSet::SelectLCS(LONG lKey)
{
	LRBAddress m_empty;
	m_empty.Clear();

	TLock lo(this);
	if(lKey < 1)
		return m_empty;
	
	LONG lKeyIndex = ExtractIndex(lKey);
	if(lKeyIndex < 0)
		return m_empty;
	
	if(m_vecIPSet[lKeyIndex].first.m_bOnOff)
		return m_vecIPSet[lKeyIndex].first.m_dwAddr;
	return m_empty;
}

LRBAddress LCSIPSet::GetMasterIP(LONG lIndex, BOOL bOnOff)
{
	LRBAddress m_empty;
	m_empty.Clear();

	TLock lo(this);
	if(lIndex + 1 > m_lLCSCount)
		return m_empty;
	if(bOnOff)	// On�� ��쿡�� IP�� return �Ѵ�.
	{
		if(m_vecIPSet[lIndex].first.m_bOnOff)
			return m_vecIPSet[lIndex].first.m_dwIP;
		return m_empty;
	}
	return m_vecIPSet[lIndex].first.m_dwIP;
}

LRBAddress LCSIPSet::GetSlaveIP(LONG lIndex, BOOL bOnOff)
{
	LRBAddress m_empty;
	m_empty.Clear();

	TLock lo(this);
	if(lIndex + 1 > m_lLCSCount)
		return m_empty;
	if(bOnOff)	// On�� ��쿡�� IP�� return �Ѵ�.
	{
		if(m_vecIPSet[lIndex].second.m_bOnOff)
			return m_vecIPSet[lIndex].second.m_dwIP;
		return m_empty;
	}
	return m_vecIPSet[lIndex].second.m_dwIP;
}

LRBAddress LCSIPSet::GetMasterAddr(LONG lIndex, BOOL bOnOff)
{
	LRBAddress m_empty;
	m_empty.Clear();

	TLock lo(this);
	if(lIndex + 1 > m_lLCSCount)
		return m_empty;
	if(bOnOff)	// On�� ��쿡�� IP�� return �Ѵ�.
	{
		if(m_vecIPSet[lIndex].first.m_bOnOff)
			return m_vecIPSet[lIndex].first.m_dwAddr;
		return m_empty;
	}
	return m_vecIPSet[lIndex].first.m_dwAddr;
}

LRBAddress LCSIPSet::GetSlaveAddr(LONG lIndex, BOOL bOnOff )
{
	LRBAddress m_empty;
	m_empty.Clear();

	TLock lo(this);
	if(lIndex + 1 > m_lLCSCount)
		return m_empty;
	if(bOnOff)	// On�� ��쿡�� IP�� return �Ѵ�.
	{
		if(m_vecIPSet[lIndex].second.m_bOnOff)
			return m_vecIPSet[lIndex].second.m_dwAddr;
		return m_empty;
	}
	return m_vecIPSet[lIndex].second.m_dwAddr;
}

DWORD LCSIPSet::FindAddrIndex(LRBAddress dwLogicalAddr)
{
	TLock lo(this);

	LONG _subIndex = 0L;
	LONG lIndex = FindAddr(dwLogicalAddr, _subIndex, TRUE);
	if(lIndex < 0)
		return 0UL;
	return lIndex;
}

//////////////////////////////////////////////////////////////////////
// LCSManagerData
//////////////////////////////////////////////////////////////////////
LCSManager theNLSManager;


LCSManagerData::LCSManagerData()
{
	m_dwSerialNo = 0UL;
	m_LCSList.clear();
	SetTimeInterval(LCSVALIDATE_TIMERINTERVAL);
}

LCSManagerData::~LCSManagerData()
{
	m_LCSList.clear();
}

//
//	pUser�� �ʿ���� ���, �ӽ� ��ü�� ���� �����Ұ�..
//
LONG LCSManagerData::PushItem(LPNLSOBJECT lpObj, LONG lType, DWORD & dwSerial)
{
	TLock lo(this);
	LONG lRet = E_LCS_NOTDEFINE;
	if(!lpObj)
		return lRet;
	LONG lKey = lpObj->LcsGetKeyValue();//->GetUSN();
	if(!ISVALID_USN(lKey))
		return lRet;

	dwSerial = GetSerial();
	KeyItem item(lKey, dwSerial, lType);		
	LMT::iterator itr = Find(lKey);
	if(itr != m_LCSList.end())
	{
		// serial�� timespan�� �ٸ� �� �ۿ� �����Ƿ� �׳� �߰� �Ѵ�. ������ ���� validity check���� ���ŵȴ�.		
		// �̷��� �Ǹ�..serial�� ���� �ʿ� �������µ�..
		if((item.m_dwTimeSpan - (*itr).first.m_dwTimeSpan) > (DWORD)(m_lTimeInterval / 2))
		{
			m_LCSList.push_back(BaseT(item, lpObj));
			lRet = E_LCS_EXISTKEY;
		}
		else	// LCS�� �޼����� ������ �ʰ�, LCS���� ������ ���⸦ ��� ��ٸ���.
			lRet = E_LCS_IGNOR;
	}
	else // �������� ����. 
	{
		m_LCSList.push_back(BaseT(item, lpObj));
		lRet = S_LCS_NOTEXIST;
	}
	return lRet;
}

LONG LCSManagerData::PopItem(LONG lKey, DWORD dwSerial, LONG & lType, LPNLSOBJECT * lpObj)
{
	TLock lo(this);
	LONG lRet = E_LCS_NOTDEFINE;
	LMT::iterator itr = Find(lKey, dwSerial);
	if(itr != m_LCSList.end())	// USN, Serial�� ��ġ�ϹǷ�, ����Ʈ���� �����, pUser�� return.
	{
		if((*itr).second == NULL)
		{
			*lpObj = NULL;
			m_LCSList.erase(itr);
			return E_LCS_NOTEXIST;
		}
		lType = (*itr).first.m_lType;
		*lpObj = (*itr).second;
		m_LCSList.erase(itr);
		lRet = E_LCS_EXISTKEY;	// �̰� �����ε�...
	}
	else 
	{
		lRet = E_LCS_NOTEXIST;
	}
	
	return lRet;
}

void LCSManagerData::PopItem(LONG & lType, LPNLSOBJECT * lpObj)
{
	TLock lo(this);
	if(m_LCSList.empty())
	{
		lType = 0; 
		*lpObj = NULL;
		return;
	}
	LMT::iterator itr = m_LCSList.begin();
	*lpObj = (*itr).second;
	lType = (*itr).first.m_lType;
	m_LCSList.pop_front();
}

void LCSManagerData::RemItem(LONG lKey)
{
	TLock lo(this);
	LMT::iterator itr = Find(lKey);
	if(itr == m_LCSList.end())
		return;
	m_LCSList.erase(itr);
}

// �ִ� ����Ʈ ũ�⸦ �Ѵ°�, �ð� �����Ȱ�, (�̹� �����ϴ� USN) �� ����.
// List��Ƽ� �����ϰ�, ������ ������ ġ���ϵ��� ��.
LONG LCSManagerData::CheckValidity(UserPtrList & lstUserPtr, LONG & ltotcnt)
{
	TLock lo(this);
	lstUserPtr.clear();
	// max size check
	LONG lCount = m_LCSList.size();
	if(lCount < 1)
		return 0L;
	LONG lRestCnt = m_lMaxItemCnt - lCount;

	// time over check
	LONG lOverTimeCnt = 0L;
	DWORD tick = ::GetTickCount();
	ForEachElmt(LMT, m_LCSList, it, jt)
	{
		KeyItem & item = (*it).first;
		if((tick - item.m_dwTimeSpan) < (DWORD)m_lTimeInterval)
			break;
		lOverTimeCnt++;
	}

	// move & erase
	LONG lMoveCnt = ( lRestCnt > 0 ) ? lOverTimeCnt : m_lMaxItemCnt;
	LONG _movecnt = lMoveCnt;
	if(lMoveCnt < 1)
		return 0L;
	ForEachElmt(LMT, m_LCSList, it2, jt2)
	{
		if(!_movecnt) break;
		lstUserPtr.push_back(TempUserT((*it2).first.m_lType, (*it2).second));
		--_movecnt;
	}
	m_LCSList.erase(m_LCSList.begin(), it2);

	return lMoveCnt;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// LCSManager
////////////////////////////////////////////////////////////////////////////////////////////////

LCSManager::LCSManager()
{
	m_lMsgCount = 0L;
	m_dwLogicAddr = 0UL;
	m_dwTypeID = 0UL;

	m_dwRefCnt = 0UL;
}

LCSManager::~LCSManager()
{

}
STDMETHODIMP_(ULONG) LCSManager::AddRef() 
{
	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
	return dwRefCnt;
}

STDMETHODIMP_(ULONG) LCSManager::Release() 
{
	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
	if(dwRefCnt == 0)
	{
	}
	return dwRefCnt;
}

BOOL LCSManager::RunLCSManager(CLrbManager * pLRB, NSAP & nsap)
{
	TLock lo(this);
	if(!pLRB)
		return FALSE;
	VALIDATE(m_LCSTimer.Activate(GetThreadPool(), this, LCSVALIDATE_TIMERINTERVAL, LCSVALIDATE_TIMERINTERVAL));

	m_nsap = nsap;
	m_pLRBConnector = pLRB;

	// LCS ��� ��û...
#ifdef _GLSLCS
	LONG lSSN;

	for (int i = 0; i < MAX_ITEM_COUNT; i++)
	{
		char sSSN[10] = {0x00};

		sprintf(sSSN, "SSN%d", i);

		DWORD dwSSN = ::GetPrivateProfileIntA("CHECKDUP", sSSN, -1, theControl.m_confPath.GetConfPath()/*"GRCConfig.INI"*/);

		lSSN = (LONG)dwSSN;

		if (lSSN != -1)
			m_vecCheckDupAccessSSN.push_back(lSSN);
	}

	if (m_vecCheckDupAccessSSN.size() == 0)
	{
		m_vecCheckDupAccessSSN.push_back(2);	// ��Ŀ Poker  
		m_vecCheckDupAccessSSN.push_back(3);	// ���̷ο� HighLow  
		m_vecCheckDupAccessSSN.push_back(17);	// �ٵ���  Baduki  
		m_vecCheckDupAccessSSN.push_back(18);	// �Ƕ�  Hoola  
		m_vecCheckDupAccessSSN.push_back(20);	// Ȧ����Ŀ  -  (������� �ʴ� �����ε���)
		m_vecCheckDupAccessSSN.push_back(23);	// ����Ŀ  Poker3  
		m_vecCheckDupAccessSSN.push_back(25);	// ����  Sudda  
		m_vecCheckDupAccessSSN.push_back(26);	//  �������� -  
		m_vecCheckDupAccessSSN.push_back(53);	// Old Highlow
	}

	SendRegisterServiceAnsToLCS(theRoomTable.GetLcsAddr());
#endif
#ifdef _CHSNLS
	SendRegisterServiceAnsToLCS(theLRBHandler.GetLcsAddr());
#endif

	return TRUE;
}

// LCS���� ������ ���ų� ������ ���� LCS�� ���� ������ ó���Ѵ�.
void LCSManager::StopingLCSManager(LRBAddress dwLogicalAddr)
{
	//theLog.Put(INF_UK, "GLS_LCSManager", "===== StopingLCSManager by NACK from LCS ===== %d LogAddr ", dwLogicalAddr);
	theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "===== StopingLCSManager by NACK from LCS ===== LogAddr: ", dwLogicalAddr.GetString().c_str());

	TLock lo(this);
	// �׳� �� ����������.. �����ϴ°� ���ѵ�.. �̷���� resource�� ����... 
	LONG lMsgType = -1L;
	LPNLSOBJECT lpObj = NULL;
	m_LCSData.PopItem(lMsgType, &lpObj);	// ����Ʈ�� ó������ �ϳ��� ����.

	m_LCSIPSet.StopLCS(dwLogicalAddr);

	while(lpObj)
	{
		// �ش� LCS�� ���� ��쿡�� ��� ��ų��..
		RemUser(lpObj->LcsGetKeyValue());

		RoomID rid;
		VALIDATE(lpObj->LcsGetRoomID(rid));
#ifdef _CHSNLS
		ChannelID cid(rid.m_lSSN, rid.m_dwCategory, rid.m_dwGCIID);
		//CChannel * pChannel = theChannelDir.FindChannel(cid);
		CChannel* pChannel;
		theChannelDir.GetChannel(cid, &pChannel);
		if(!pChannel)
		{
			//theLog.Put(INF_UK, "GLS_LCSManager", "Not found channel .. StopingLCSManager  ");
			theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "Not found channel .. StopingLCSManager");
			m_LCSData.PopItem(lMsgType, &lpObj);	
			continue;
		}

		lpObj->LcsSetErrorCode(S_LCS_NOTEXIST);	// ����� ��� ���� ������ ������ ó���Ѵ�..
		if(lMsgType == LCSMSGTYPE_DJOINCHANNEL || lMsgType == LCSMSGTYPE_NJOINCHANNEL || lMsgType == LCSMSGTYPE_INVITCHANNEL)
		{
			::XsigQueueSignal(GetThreadPool(), pChannel, CHANNELSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
		}
#else
		CRoom* pRoom = NULL;
		theRoomTable.FindRoom(rid, &pRoom);
		if (!pRoom) 
		{
			theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "Not found room .. StopingLCSManager  ");
			m_LCSData.PopItem(lMsgType, &lpObj);	
			continue;
		}

		lpObj->LcsSetErrorCode(S_LCS_NOTEXIST);	// ����� ��� ���� ������ ������ ó���Ѵ�..
		if(lMsgType == LCSMSGTYPE_CREATEROOM || lMsgType == LCSMSGTYPE_JOINROOM)
		{
			::XsigQueueSignal(GetThreadPool(), pRoom, ROOMSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
		}

		pRoom->Release();
#endif	
		lpObj = NULL; 
		lMsgType = -1L;
		m_LCSData.PopItem(lMsgType, &lpObj);	
	}

}

// main or sub LCS ? -> �ٸ��� ó�� �Ǿ�� ��.
void LCSManager::LCSTerminateNtf(LRBAddress& _Addr)
{
	string strAddr;
	_Addr.GetStringFormat(strAddr);
	theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "----- LCS Terminate [",strAddr, "] -----");
	m_LCSIPSet.StopLCS(_Addr);
}

STDMETHODIMP_(void) LCSManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if(hSignal == 0)
	{

	}
	else
	{
		if(m_LCSTimer.IsHandle(hSignal))	
		{
			TLock lo(this);
			OnValidateCheck();
		}
	}

}

// �� LCS���� ������ validate, state check�� ���߿�..
//static gs_validatecheckcnt = 0L;
void LCSManager::OnValidateCheck()
{
	UserPtrList lstUserPtr;
	lstUserPtr.clear();
	LONG ltotcnt = 0L;
	LONG lcnt = m_LCSData.CheckValidity(lstUserPtr, ltotcnt);
	if(!lcnt)
		return;
	else 
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "Validate Check , Invalid user list count : ", ltotcnt, "-", lcnt);

	if(lcnt > LCSLIMIT_WAITINGUSER)
	{
		theLog.Put(WAR_UK, "GLS_LCSManager"_COMMA, "*************** over limit waiting user count : defined value [", LCSLIMIT_WAITINGUSER, "] **************");
	}
	ForEachElmt(UserPtrList, lstUserPtr, it, jt)
	{
		LPNLSOBJECT lpObj = (*it).second;
		LONG lMsgType = (*it).first;
		// LCS�� ����� ���Ÿ� �䱸.. Ȥ�� �߰��۾� ���� data�� ������ �Ѵ�.
		RoomID rid;
		VALIDATE(lpObj->LcsGetRoomID(rid));
		ReqRemUser(lpObj->LcsGetKeyValue(), rid);
#ifdef _CHSNLS
		ChannelID cid(rid.m_lSSN, rid.m_dwCategory, rid.m_dwGCIID);
		//CChannel * pChannel = theChannelDir.FindChannel(cid);
		CChannel* pChannel = NULL;
		theChannelDir.GetChannel(cid, &pChannel);
		if(!pChannel)
		{
			RemUser(lpObj->LcsGetKeyValue());
			continue;
		}
		lpObj->LcsSetErrorCode(S_LCS_NOTEXIST);	
		if(lMsgType == LCSMSGTYPE_DJOINCHANNEL || lMsgType == LCSMSGTYPE_NJOINCHANNEL || lMsgType == LCSMSGTYPE_INVITCHANNEL)
		{
			::XsigQueueSignal(GetThreadPool(), pChannel, CHANNELSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
		}
#else 
		CRoom* pRoom = NULL;
		theRoomTable.FindRoom(rid, &pRoom);
		if (!pRoom) 
		{
			RemUser(lpObj->LcsGetKeyValue());
			continue;
		}
		lpObj->LcsSetErrorCode(S_LCS_NOTEXIST);	// �ϴ� ���� ���� ������ ó���Ѵ�.
		if(lMsgType == LCSMSGTYPE_CREATEROOM || lMsgType == LCSMSGTYPE_JOINROOM)
		{
			::XsigQueueSignal(GetThreadPool(), pRoom, ROOMSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
		}
		pRoom->Release();
#endif
	}

}

//
// Room�̳� Listenerd���� User�� Link�� �������� ������ ����� ����, LCS�� ���� �޼��� ������.
//
void LCSManager::ReqRemUser(LONG lKey, RoomID & rid)
{
	TLock lo(this);

	RemUser(lKey);
#ifdef _GLSLCS
	if (rid.m_dwGRIID == ABSOLUTE_DELETE_VALUE)
		theLog.Put(WAR_UK, "GLS_LCSManager, ABSOLUTE_DELETE_VALUE GRIID, USN = ", lKey);
#else
	if (rid.m_dwGRIID == ABSOLUTE_DELETE_VALUE)
		theLog.Put(WAR_UK, "CHS_LCSManager, ABSOLUTE_DELETE_VALUE GRIID, USN = ", lKey);
#endif

	// LCS�� ����ڰ� ���� ������ �䱸.
	PayloadOUTLCS pld(PayloadOUTLCS::msgRemUserReq_Tag);
	pld.un.m_msgRemUserReq->m_lUSN = lKey;
	pld.un.m_msgRemUserReq->m_roomID = rid;

	LRBAddress dwLogicalAddr = m_LCSIPSet.SelectLCS(lKey);
	SendLCSMessage(pld, dwLogicalAddr);
	m_lMsgCount--;
	return;
}

//
// ������ �õ��ؿ��� �����.. LCSManagerData�� �׸��� �߰��ϰ�, LRB�� ���� LCS�� �޼��� ���ޱ��� ����
// 
static DWORD g_dwBypassCnt = 0UL;
BOOL LCSManager::AddUserToNLS(LPNLSOBJECT lpObj, LONG client_ip, string & sOption, LONG lMsgType, IXObject * pObj)//CRoomPtr & spRoom)
{
	TLock lo(this);
	LONG _Key = lpObj->LcsGetKeyValue();

	if(!lpObj)
	{
		theLog.Put(WAR_UK, "GLS_LCSManager"_COMMA, "Add User to LCSManager : invalid Item Argument");
		return FALSE;
	}

	// LCS �� �ʿ�� �ϴ� �����͸� ����.	
	LCSBaseInfo info;
	RoomID rid;
	lpObj->LcsGetRoomID(rid);
	info.m_lUSN = _Key;
	info.m_grid = rid;
	info.m_nsap = m_nsap;
	info.m_sOption = sOption;
	info.m_lClientIP = client_ip;	

	// LCS�� �׾� �ִ� ��� �׳�.. �������� ó��.
	LRBAddress _addr = m_LCSIPSet.SelectLCS(_Key);
	if(_addr.addr[0] == BYTE(' ') && pObj)//spRoom)
	{
		// LcS�� �������� ������ �˼� �־�� �ϴµ�.. 
		g_dwBypassCnt++;
		if(g_dwBypassCnt % 300UL == 0UL )
		{
			g_dwBypassCnt = 0UL;
			if(_addr.addr[0] != BYTE(' '))
				theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "LCS, Bypass AddUser Msg : invalid address");
			else 
				theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "LCS, Bypass AddUser Msg : invalid pObj point");
		}
		lpObj->LcsSetErrorCode(S_LCS_NOTEXIST);

		{
			PMSAWarningNtf msgNtf;
			msgNtf.m_sWarnMsg  = ::format("Can't Find LCS Server \n");
			msgNtf.m_sTreatMsg = ::format("Check the LCS Server [Key:%d] ",_Key);
			msgNtf.m_lErrLevel = FL_CRITICAL;
			PayloadHA pld(PayloadHA::msgPMSWarningNtf_Tag,msgNtf);

			thePMSConnector.SendWarningMsg(msgNtf.m_lErrLevel, msgNtf.m_sWarnMsg.c_str(), msgNtf.m_sTreatMsg.c_str(), 0, 0);
		}
#ifdef _CHSNLS
		::XsigQueueSignal(GetThreadPool(), pObj, CHANNELSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
#else
		::XsigQueueSignal(GetThreadPool(), pObj, ROOMSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
#endif
		return TRUE;
	}

	AddUser(info);

	DWORD dwSerialNo = 0L;
	LONG lRet = m_LCSData.PushItem(lpObj, lMsgType, dwSerialNo);
	if(lRet == E_LCS_IGNOR)	// �޼��� ������ �ʰ�, ��� ��ٸ�.
	{
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "waiting LCS Answer.. : USN ", _Key);
		return TRUE;
	}
	else if(lRet == E_LCS_NOTDEFINE)	// error check�� ����.. ���� �ٸ� ó���� ���� ����.
	{
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "fail AddUser to LCS .. Invalid argument ");
		return FALSE;
	}	
	else if(lRet == E_LCS_EXISTKEY || lRet == S_LCS_NOTEXIST)
	{
		//theLog.Put(INF_UK, "send AddUser Msg to LCS : %d USN ", pUser->GetUSN());
	}

	
	PayloadOUTLCS pld(PayloadOUTLCS::msgAddUserReq_Tag);
	pld.un.m_msgAddUserReq->m_bMode = TRUE;	// TRUE ��� �ߺ� ������ ��� ���� ����ڸ� ����, �ٷ� add ó���� �䱸
	pld.un.m_msgAddUserReq->m_lcsBaseInfo = info;
	pld.un.m_msgAddUserReq->m_dwSerial = dwSerialNo;

	SendLCSMessage(pld, _addr);

	m_lMsgCount++;
	return TRUE;
}

void LCSManager::SendLCSMessage(PayloadOUTLCS & pld, const LRBAddress& des)
{
#ifdef _GLSLCS
	const LRBAddress& addr = theRoomTable.GetAddr();
#else
	const LRBAddress& addr = theLrbManager.GetMyAddress();
#endif

	theLrbManager.SendToLCS(addr, des, pld);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////////////////////////
void LCSManager::SendRegisterServiceAnsToLCS(const LRBAddress& dwLCSAddr)
{
	MsgOUTLCS_ServerRegisterAns msg;
	const NSAP& nsap = GetNSAP();
#ifdef _GLSLCS
	memcpy(msg.m_addr.addr, theRoomTable.GetAddr().addr, sizeof(theRoomTable.GetAddr().addr));
#else
	memcpy(msg.m_addr.addr, theLrbManager.GetMyAddress().addr, sizeof(theLrbManager.GetMyAddress().addr));
#endif
	msg.m_nsap = nsap;
	PayloadOUTLCS pld(PayloadOUTLCS::msgServerRegisterAns_Tag, msg);

	SendLCSMessage(pld, dwLCSAddr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ���� �������� �޼��� ó��.. ���Ҽ� ����?
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LRB�� ���ؼ� ���ŵ� LCS �޼����� ó��..
//
void LCSManager::RecvLCSMessage(const PayloadLCSOUT * pld, const LRBAddress& src)	// posting���� ���� ���ΰ�?
{
	switch(pld->mTagID)
	{
	case PayloadLCSOUT::msgAddUserAns_Tag: // AddUser�� ���� ����..
		OnAddUserAnsFromLCS(*(pld->un.m_msgAddUserAns));
		break;
	case PayloadLCSOUT::msgFindUserAns_Tag:
		OnFindUserAnsFromLCS(*(pld->un.m_msgFindUserAns));
		break;
	case PayloadLCSOUT::msgGetUserLCAns_Tag:
		OnGetUserLCAnsFromLCS(*(pld->un.m_msgGetUserLCAns));
		break;
	case PayloadLCSOUT::msgUserListReq_Tag:
		OnUserListReqFromLCS(*(pld->un.m_msgUserListReq), src);
		break;
	case PayloadLCSOUT::msgDisconnectUserReq_Tag:
		OnDisconnectUserReqFromLCS(*(pld->un.m_msgDisconnectUserReq));
		break;
	case PayloadLCSOUT::msgServerRegisterReq_Tag:
		OnServerRegisterReqFromLCS(*(pld->un.m_msgServerRegisterReq), src);
		break;
	case PayloadLCSOUT::msgQueryUserStateReq_Tag:
		OnQueryUserStateReqFromLCS(*(pld->un.m_msgQueryUserStateReq), src);
		break;
	case PayloadLCSOUT::msgKickOutUserReq_Tag:
		OnKickOutUserReqFromLCS(*(pld->un.m_msgKickOutUserReq),src);
		break;
	case PayloadLCSOUT::msgKickOutUserNtf_Tag:
		OnKickOutUserNtfFromLCS(*(pld->un.m_msgKickOutUserNtf),src);
		break;
	default:
		{
			theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "CLrbConnector::OnRcvLCSMsg - Unknown message(Tag:", pld->mTagID, ")");
		}
		break;
	}
}

// LCSManagerData���� �����, Room�� ã�Ƽ� ������ ...
void LCSManager::OnAddUserAnsFromLCS(MsgLCSOUT_AddUserAns & msg)
{
	TLock lo(this);

	LPNLSOBJECT lpObj = NULL;
	LONG lMsgType = 0L;
	LONG lErrorCode = msg.m_lErrCode;
	LONG lRet = m_LCSData.PopItem(msg.m_lUSN, msg.m_dwSerial, lMsgType, &lpObj);
	if(lRet == E_LCS_NOTEXIST)
		return;//lErrorCode = lRet;
	if(!lpObj)
		return;

#ifdef _GLSLCS
	//////////// �ϴ� ���¡ ���õ� ��Ŀ, ���̷ο�, �ٵ���, �Ƕ� �ٽ� �����ϰ� �Ѵ�. kimsk
	if(lErrorCode == E_LCS_EXISTKEY + 100 && CheckDuplicatedAccessBySSN(msg.m_roomID.m_lSSN))
	{
		theLog.Put(DEV_UK, "Duplicated User: ", msg.m_lUSN, " SSN: ", msg.m_roomID.m_lSSN, " Message type: ", lMsgType);
		((CUser*)(lpObj))->SetErrorCode(E_LCS_EXISTKEY + 100);
		if(lMsgType == LCSMSGTYPE_JOINROOM)
		{
			lpObj->LcsSetErrorCode(CRF_DBERROR);
			::XsigQueueSignal(GetThreadPool(), &theListener, (HSIGNAL)0, (WPARAM)LISTENERMSG_JOINANS, (LPARAM)lpObj);
		}
		else
		{
			lpObj->LcsSetErrorCode(JRF_DBERROR);
			::XsigQueueSignal(GetThreadPool(), &theListener, (HSIGNAL)0, (WPARAM)LISTENERMSG_CREATEANS, (LPARAM)lpObj);
		}
		return;
	}
#endif

#ifdef _CHSNLS 	

	//////////// ä�� �������� ���� RoomID�� ������ UserJoinAns �޽��� ����...
	if(lErrorCode == E_LCS_EXISTKEY + 100) 
	{			
		RoomID newRoomID;
		lpObj->LcsGetRoomID(newRoomID);
		if(!(lMsgType == LCSMSGTYPE_INVITCHANNEL && msg.m_roomID.m_dwGCIID == newRoomID.m_dwGCIID && msg.m_roomID.m_dwGRIID != 0))	/// Channel ID�� ������ �׳� �Ѿ�� �ǰ���?
		{
			RoomID ridNULL;
			ridNULL.Clear();

			if (msg.m_roomID.m_lSSN == 25 || msg.m_roomID.m_lSSN == 26)
			{
				CRoomID_For_ChannelReJoin* rFC = new CRoomID_For_ChannelReJoin(lMsgType, ridNULL);
				::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_DUPLICATEJOINCHANNELANS, (WPARAM)lpObj, (LPARAM)rFC);
			}
			else
			{
				CRoomID_For_ChannelReJoin* rFC = new CRoomID_For_ChannelReJoin(lMsgType, msg.m_roomID);
				::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_DUPLICATEJOINCHANNELANS, (WPARAM)lpObj, (LPARAM)rFC);
			}

			return;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	ChannelID cid(msg.m_roomID.m_lSSN, msg.m_roomID.m_dwCategory, msg.m_roomID.m_dwGCIID);
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(cid, &spChannel)) 
	{
		lpObj->LcsSetErrorCode(CRF_DBERROR);	// �ӽ÷� DB Error�� ó��..		
		if(lMsgType == LCSMSGTYPE_NJOINCHANNEL)
			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_JOINCHANNELANS, (WPARAM)lpObj, (LPARAM)ERR_CHANNEL_NOTFOUND);
		else if(lMsgType == LCSMSGTYPE_INVITCHANNEL)
			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_INVITECHANNELANS, (WPARAM)lpObj, (LPARAM)ERR_CHANNEL_NOTFOUND);
		else if(lMsgType == LCSMSGTYPE_DJOINCHANNEL)
			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_DJOINCHANNELANS, (WPARAM)lpObj, (LPARAM)ERR_CHANNEL_NOTFOUND);
		return;		
	}
	lpObj->LcsSetErrorCode(lErrorCode);
	::XsigQueueSignal(GetThreadPool(), spChannel, CHANNELSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
#endif
#ifdef _GLSLCS
	CRoom* pRoom = NULL;
	theRoomTable.FindRoom(msg.m_roomID, &pRoom);
	if (!pRoom) 
	{
		lpObj->LcsSetErrorCode(CRF_DBERROR);	// �ӽ÷� DB Error�� ó��..
		if(lMsgType == LCSMSGTYPE_JOINROOM)
		{
			lpObj->LcsSetErrorCode(JRF_DBERROR);
			::XsigQueueSignal(GetThreadPool(), &theListener, 0, LISTENERMSG_JOINANS, (LPARAM)lpObj);
		}
		else
		{
			lpObj->LcsSetErrorCode(JRF_DBERROR);
			::XsigQueueSignal(GetThreadPool(), &theListener, 0, LISTENERMSG_CREATEANS, (LPARAM)lpObj);
		}
		return;
	}

	lpObj->LcsSetErrorCode(lErrorCode);
	::XsigQueueSignal(GetThreadPool(), pRoom, ROOMSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
	pRoom->Release();
#endif
}

#ifdef _GLSLCS
BOOL LCSManager::SetAutoPlayNotify(LONG lUSN, RoomID& rid)
{
	theLog.Put(DEV_UK, "GLS_LCSManager"_COMMA, "LCSManager::SetAutoPlayNotify, USN", lUSN);

	PayloadOUTLCS pld(PayloadOUTLCS::msgSetAutoPlayNtf_Tag);
	pld.un.m_msgSetAutoPlayNtf->m_lUSN = lUSN;
	pld.un.m_msgSetAutoPlayNtf->m_roomID = rid;

	LRBAddress dwLogicalAddr = m_LCSIPSet.SelectLCS(lUSN);
	SendLCSMessage(pld, dwLogicalAddr);

	return TRUE;
}

BOOL LCSManager::ResetAutoPlayNotify(LONG lUSN, RoomID& rid)
{
	theLog.Put(DEV_UK, "GLS_LCSManager"_COMMA, "LCSManager::ResetAutoPlayNotify, USN", lUSN);

	PayloadOUTLCS pld(PayloadOUTLCS::msgResetAutoPlayNtf_Tag);
	pld.un.m_msgResetAutoPlayNtf->m_lUSN = lUSN;
	pld.un.m_msgResetAutoPlayNtf->m_roomID = rid;

	LRBAddress dwLogicalAddr = m_LCSIPSet.SelectLCS(lUSN);
	SendLCSMessage(pld, dwLogicalAddr);

	return TRUE;
}

BOOL LCSManager::CheckDuplicatedAccessBySSN(LONG lSSN)
{
	int nCount = m_vecCheckDupAccessSSN.size();

	for (int i = 0; i < nCount; i++)
	{
		if (m_vecCheckDupAccessSSN[i] == lSSN)
			return TRUE;
	}

	return FALSE;
}
#endif

void LCSManager::OnFindUserAnsFromLCS(MsgLCSOUT_FindUserAns & msg)
{
}

void LCSManager::OnGetUserLCAnsFromLCS(MsgLCSOUT_GetUserLCAns & msg)
{
}

void LCSManager::OnUserListReqFromLCS(MsgLCSOUT_UserListReq & msg, const LRBAddress& des)
{
	TLock lo(this);

	LCSBaseInfoList lstUser;
	// m_nextRoomID�� [IN][OUT] parameter.. �̴�..
	LONG lPartKey = m_LCSIPSet.FindAddrIndex(des);
	if(lPartKey < 0)
		return;

	// deadlock ������ ����...
	// room table�� room instance�κ��� ���� user list�� ��°��� �Ұ�..
	// LCSM�� ������ user list�� ������ �� �ۿ� ����.
	//LONG lRet = theRoomTable.GetPartialUserList(nextRID, lPartKey, m_LCSIPSet.GetLCSCount(), LCSMAXCNT_USERLIST, lstUser);
	ExtractUser(lPartKey, m_LCSIPSet.GetLCSCount(), lstUser);

	// lNextIndex ���� < 0 �ΰ��� ���������� ������ �Ѵ�....
	PayloadOUTLCS pld(PayloadOUTLCS::msgUserListAns_Tag);
	pld.un.m_msgUserListAns->m_lstLCSBaseInfo = lstUser;	

	SendLCSMessage(pld, des);
}

// ����� ���縦 ���� �ش� ����� ���� :GLS
void LCSManager::OnKickOutUserReqFromLCS(MsgLCSOut_KickOutUserReq & msgFromLcs, const LRBAddress& des)
{
	TLock lo(this);
#ifdef _CHSNLS
	PayloadOUTLCS pldLCS(PayloadOUTLCS::msgKickOutUserAns_Tag);

	MsgOUTLCS_KickOutUserAns *pMsgToLcs = pldLCS.un.m_msgKickOutUserAns;
	pMsgToLcs->m_addr	= msgFromLcs.m_addr;
	pMsgToLcs->m_dwMID	= msgFromLcs.m_dwMID;	

	// CHS�� �ƹ��͵� ���� �ʰ� LCS�� ok�� ���� ������.
	pMsgToLcs->m_lResultCode	= 0x0002; // not error from chs
	pMsgToLcs->m_sResult		= "OK";

	theLog.Put(INF_UK, "CHS_LCSManager"_COMMA, "OnKickOutUserReqFromLCS. Type:", msgFromLcs.m_lType, ", GCIID: ", msgFromLcs.m_roomID.m_dwGCIID, ", USN: ", msgFromLcs.m_lUSN);	

	SendLCSMessage(pldLCS, des);

#else _GLSLCS
	//MsgOUTLCS_KickOutUserAns msgToLcs;
	//msgToLcs.m_addr		= msgFromLcs.m_addr;
	//msgToLcs.m_dwMID	= msgFromLcs.m_dwMID;	

	PayloadOUTLCS pldLCS(PayloadOUTLCS::msgKickOutUserAns_Tag);
	//pldLCS.un.m_msgKickOutUserAns =  &msgToLcs;	

	MsgOUTLCS_KickOutUserAns *pMsgToLcs = pldLCS.un.m_msgKickOutUserAns;
	pMsgToLcs->m_addr	= msgFromLcs.m_addr;
	pMsgToLcs->m_dwMID	= msgFromLcs.m_dwMID;	

	 	
	CRoom* pRoom = NULL;
	theRoomTable.FindRoom(msgFromLcs.m_roomID, &pRoom);
	
	if (pRoom == NULL)
	{	//���� �߻�
		pMsgToLcs->m_lResultCode	= -101; // GLS Not Found Room Error
		pMsgToLcs->m_sResult		= "GLS_Error: Can't Find Room";
		theLog.Put(WAR_UK, "GLS_LCSManager"_COMMA, "Not found room .. KickOutUserReq FromLCS, RoomID:",msgFromLcs.m_roomID.m_dwGRIID);		
	} 
	else
	{
		CUser* pUser = pRoom->FindUser(msgFromLcs.m_lUSN);

		if (pUser)
		{
			UserBaseInfo userInfo = pUser->GetUserData();
			string sUID = userInfo.m_sUID.substr(0, userInfo.m_sUID.length()-3);
			string sUserInfo = ::format("%s(%s***)", userInfo.m_sNickName.c_str(), sUID.c_str());
			// �濡���� ����.
			::XsigQueueSignal(GetThreadPool(), pRoom, (HSIGNAL)ROOM_USERLINKCUT, (WPARAM)(msgFromLcs.m_lUSN), 0);
			// if msgtype is terminate_room then notify it to GRC
			pRoom->AnnounceKickoutUser(KICKOUT_USER_CHEAT, sUserInfo);
			theLog.Put(WAR_UK, "GLS_LCSManager"_COMMA, "Kickout User: ", sUserInfo);
		}

		if (msgFromLcs.m_lType == 3 || msgFromLcs.m_lType == 4)
			pRoom->OnTerminateRoomReq(TERMINATE_ROOM_GRACEFUL);
		else if (msgFromLcs.m_lType == 5 || msgFromLcs.m_lType == 6)
			pRoom->OnTerminateRoomReq(TERMINATE_ROOM_IMMEDIATE);

		pMsgToLcs->m_lResultCode	= 0x0001; // not error from gls
		pMsgToLcs->m_sResult		= "OK";
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "OnKickOutUserReqFromLCS. Type:", msgFromLcs.m_lType, ", RoomID: ", msgFromLcs.m_roomID.m_dwGRIID, ", USN: ", msgFromLcs.m_lUSN);	
	}

	theLrbManager.SendToLCS(theRoomTable.GetAddr(), des, pldLCS);

	if (pRoom)
		pRoom->Release();
#endif
}

/*
ForEachElmt(TChannelDir, m_mapChannelDir, it, ij)
{
	ChannelID cid = (*it).first;
	if (cid.m_lSSN == lSSN)
	{
		msg.m_lstChannelID.push_back(cid);
	}
}
*/
void LCSManager::OnKickOutUserNtfFromLCS(MsgLCSOut_KickOutUserNtf & msgFromLcs, const LRBAddress& des)
{
	TLock lo(this);
#ifdef _CHSNLS
	MsgCHSGLS_ChannelIDList		msgChannelIDList;
	theChannelDir.OnChannelDListReq(msgFromLcs.m_lSSN, msgChannelIDList);

	ForEachElmt(CHANNELIDLIST, (msgChannelIDList.m_lstChannelID), i, j)
	{
		ChannelID cid = *i;
		CChannel * pChannel = NULL;
		theChannelDir.GetChannel(cid, &pChannel);
		if(pChannel &&  (pChannel->IsUserConnected(msgFromLcs.m_lUSN)))
		{
			::XsigQueueSignal(GetThreadPool(), pChannel, CHANNEL_USERLINKCUT_SIG, (WPARAM)(msgFromLcs.m_lUSN), 0);
			theLog.Put(INF_UK, "CHS_LCSManager, KickOutUserNtfFromLCS : SSN=",msgFromLcs.m_lSSN, ", USN=", msgFromLcs.m_lUSN);
		}
	}
#else _GLSLCS
// GLS �� �� �޽��� �� ����.
#endif
}

void LCSManager::OnDisconnectUserReqFromLCS(MsgLCSOUT_DisconnectUserReq & msg)
{
	TLock lo(this);
#ifdef _CHSNLS
	ChannelID cid(msg.m_roomID.m_lSSN, msg.m_roomID.m_dwCategory, msg.m_roomID.m_dwGCIID);
	CChannel * pChannel = NULL;
	theChannelDir.GetChannel(cid, &pChannel);
	if(!pChannel)
	{
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "Not found channel .. DisconnectUserReq FromLCS : SSN/Category/GCIID = [", \
			msg.m_roomID.m_lSSN, "/", msg.m_roomID.m_dwCategory, "/", msg.m_roomID.m_dwGCIID, "]");
		return;
	}

	bool bCheckDis = false;

	int ivecSize = theChannelDir.m_vecCHSGLSConSSN.size();
	if (ivecSize > 0)
	{
		for(int i=0; i<ivecSize; i++)
		{
			// INI���Ͽ� �����Ǿ� �ִ� SSN(LCS�� ���� LCSOUT �޼����� ������ ���� ���ƾߵǴ� ������ SSN)��
			// MSG�� ���� SSN�� �� ���� ������... USERLINKCUT_Signal�� ������ �ʴ´�.
			if (theChannelDir.m_vecCHSGLSConSSN[i] == msg.m_roomID.m_lSSN)
			{
				bCheckDis = true;
			}
		}
	}

	if (!bCheckDis)
	{
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "----- DisconnectUserReq FromLCS 2 = USN/SSN : [", msg.m_lUSN, "]/[", msg.m_roomID.m_lSSN,"] ----");
		::XsigQueueSignal(GetThreadPool(), pChannel, CHANNEL_USERLINKCUT_SIG, (WPARAM)(msg.m_lUSN), 0); // USN���� �ȵ�..
	}

#else
	CRoom* pRoom = NULL;
	theRoomTable.FindRoom(msg.m_roomID, &pRoom);
	if (!pRoom) 
	{
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "Not found room .. DisconnectUserReq FromLCS 1. mgs.m_roomID:",CRoom::RoomID2Str(msg.m_roomID),", USN:", msg.m_lUSN);
		return;
	} 
	else	
	{
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "Duplicated room .. DisconnectUserReq FromLCS 2. RoomID:", CRoom::RoomID2Str(msg.m_roomID), ", USN:", msg.m_lUSN);
//		::XsigQueueSignal(GetThreadPool(), pRoom, ROOM_USERLINKCUTTING, (WPARAM)(msg.m_lUSN), 0); // USN���� �ȵ�..
		::XsigQueueSignal(GetThreadPool(), pRoom, (HSIGNAL)ROOM_USERLINKCUT, (WPARAM)(msg.m_lUSN), 0); // USN���� �ȵ�..
		
	}
	pRoom->Release();
#endif
}

void LCSManager::OnServerRegisterReqFromLCS(MsgLCSOUT_ServerRegisterReq & msg, const LRBAddress& des)
{
	TLock lo(this);
	vector<LRBAddress> vecLCSIP;
	vecLCSIP.assign(msg.m_vecLCSIP.begin(), msg.m_vecLCSIP.end());
	// LCS IP List�� ����..
	m_LCSIPSet.InitIPSet(vecLCSIP, msg.m_dwMaster, msg.m_dwLogicalAddr);//dwLCSAddr);

	if (!m_LCSIPSet.StartLCS(msg.m_dwLogicalAddr))//dwLCSAddr);
	{
		PMSAWarningNtf msgNtf;
		msgNtf.m_sWarnMsg  = ::format("LCS Registration Fail\n");
		msgNtf.m_sTreatMsg = ::format("Inspect the LCS Server [Master:%s], [Logical:%s]\n",msg.m_dwMaster.GetString().c_str(), msg.m_dwLogicalAddr.GetString().c_str());
		msgNtf.m_lErrLevel = FL_CRITICAL;
		PayloadHA pld(PayloadHA::msgPMSWarningNtf_Tag,msgNtf);

		thePMSConnector.SendWarningMsg(msgNtf.m_lErrLevel, msgNtf.m_sWarnMsg.c_str(), msgNtf.m_sTreatMsg.c_str(), 0, 0);
	}

	for (unsigned int i = 0; i < vecLCSIP.size(); i++)
        theLog.Put(WAR_UK, "GLS_LCSManager"_COMMA, "OnServerRegisterReqFromLCS: LCS IP ", vecLCSIP[i].GetString().c_str());

	SendRegisterServiceAnsToLCS(des);//dwLCSAddr);
}

void LCSManager::OnQueryUserStateReqFromLCS(MsgLCSOUT_QueryUserStateReq & msg, const LRBAddress& des)
{
	TLock lo(this);

	PayloadOUTLCS pld(PayloadOUTLCS::msgQueryUserStateAns_Tag);
	pld.un.m_msgQueryUserStateAns->m_lUSN = msg.m_lUSN;
	pld.un.m_msgQueryUserStateAns->m_lReserved = msg.m_lReserved;

#ifdef _GLSLCS

	CRoom* pRoom = NULL;
	theRoomTable.FindRoom(msg.m_RoomID, &pRoom);
	if (!pRoom) 
	{
		theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "Not found room .. OnQueryUserStateReqFromLCS 1");

		pld.un.m_msgQueryUserStateAns->m_lResult = 2L;

		SendLCSMessage(pld, des);
	} 
	else	
	{
		LONG lResult = pRoom->GetUserState(msg.m_lUSN, msg.m_lReserved);

		pld.un.m_msgQueryUserStateAns->m_lResult = lResult;
		SendLCSMessage(pld, des);

		if (lResult == 0)
		{
			RoomEvent e(REV_USERDISCONNECT, msg.m_lUSN);
			pRoom->PushQueue(e);

			if (msg.m_lReserved != -1)
				theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "OnQueryUserStateReqFromLCS: User not exist or waiting SSN ", msg.m_RoomID.m_lSSN, ", User ", msg.m_lUSN);
		}

		pRoom->Release();
	}
#else  //_CHSNLS . �����ǸŰ��� ���� ó���� �Ѵ�.

	// msg.m_lReserved != -1 �� ��� ����� ���� �����̱� ������ ����ó�� �����ϰ� �Ѵ�.
	if (msg.m_lReserved != -1)
		return;

	// ä���� ã�´�.
	ChannelID cid(msg.m_RoomID.m_lSSN, msg.m_RoomID.m_dwCategory, msg.m_RoomID.m_dwGCIID);
	CChannel * pChannel = NULL;
	theChannelDir.GetChannel(cid, &pChannel);
	if(!pChannel)
	{
		theLog.Put(ERR_UK, "CHS_LCSManagerErr, Not found channel .. OnQueryUserStateReqFromLCS FromLCS : SSN/Category/GCIID = [",
			msg.m_RoomID.m_lSSN, "/", msg.m_RoomID.m_dwCategory, "/", msg.m_RoomID.m_dwGCIID, "]");
		pld.un.m_msgQueryUserStateAns->m_lResult = 2L; // �ش� ����� �������� ������.
		SendLCSMessage(pld, des);
		return;
	}
	if (pChannel->IsUserConnected(msg.m_lUSN))
	{	
		pld.un.m_msgQueryUserStateAns->m_lResult = 1;
		
		// ����ڰ� �ִٰ� ���� ��� ����� �α�
		LONG lSize = m_mapConnectUser.size();
		if (m_mapConnectUser.end() == m_mapConnectUser.find(msg.m_lUSN))
			LOG(ERR_UK, "CHS_LCSErr.OnQueryUserStateReqFromLCS(). mismatch case 1 m_mapConnectUser.size()=", lSize, ", USN:", msg.m_lUSN);
	}
	else 
	{ 
		pld.un.m_msgQueryUserStateAns->m_lResult = 2;		
		// ����ڰ� ���ٰ� ���� ��� ����� �α�
		LONG lSize = m_mapConnectUser.size();
		if (m_mapConnectUser.end() != m_mapConnectUser.find(msg.m_lUSN))
			LOG(ERR_UK, "CHS_LCSErr.OnQueryUserStateReqFromLCS(). mismatch case 2 m_mapConnectUser.size()=", lSize, ", USN:", msg.m_lUSN);
	}

	SendLCSMessage(pld, des);
	return;

	/*
	LONG lSize = m_mapConnectUser.size();

	pld.un.m_msgQueryUserStateAns->m_lResult = 2L;

	if (lSize > 0)
	{
		LONG lResult = 0;
		TotUserMapT::iterator it = m_mapConnectUser.begin();

		for (it; it != m_mapConnectUser.end(); it++)
		{
			if (it->first == msg.m_lUSN)
				lResult = 1;
		}

		pld.un.m_msgQueryUserStateAns->m_lResult = lResult;
	}    
	SendLCSMessage(pld, des);
	*/
#endif
}

// for total connect user
void LCSManager::AddUser(LCSBaseInfo & baseInfo)
{
	TLock lo(this);
	try {
		LCSBaseInfo & _user = m_mapConnectUser[baseInfo.m_lUSN];
		_user = baseInfo;
	} catch(...) {
		theLog.Put(WAR_UK, "GLS_LCSManager"_COMMA, "----------- Lcs manager.. fail add user.. ", baseInfo.m_lUSN);
	}
}

void LCSManager::RemUser(LONG lKey)
{
	TLock lo(this);
	try {
		m_mapConnectUser.erase(lKey);
	} catch(...) {
		theLog.Put(WAR_UK, "GLS_LCSManager"_COMMA, "----------- Lcs manager.. fail remove user.. ", lKey);
	}
}

LONG LCSManager::ExtractUser(LONG lPartKey, LONG lLCSCount, LCSBaseInfoList & lstUser)
{
	TLock lo(this);
	LONG lSize = m_mapConnectUser.size();
	if(!lSize) return -1L;

	try {
		TotUserMapT::iterator itr = m_mapConnectUser.begin();
		for(itr; itr != m_mapConnectUser.end(); itr++)
		{
			LCSBaseInfo & info = itr->second;
			if(lPartKey == (itr->first)%lLCSCount)
			{
				lstUser.push_back(info);
			}
		}
	} catch(...) {
		theLog.Put(WAR_UK, "GLS_LCSManager"_COMMA, "------------- LCS Manager.. assert ExtractUser ");
	}
	theLog.Put(INF_UK, "GLS_LCSManager"_COMMA, "Connected user List ", lstUser.size());
	return -1L;
}

#endif
