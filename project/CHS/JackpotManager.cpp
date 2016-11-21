#include "stdafx.h"
#include ".\jackpotmanager.h"
#include "LRBHandler.h"
#include "Control.h"
#include "ODBGWManager.h"
#include "ChannelDir.h"

#ifdef _JACKPOT_INDICATION_
CJackpotManager theJackpotManager;

CJackpotManager::CJackpotManager(void) : m_bServiceFlagByGLS(FALSE)
{
}

CJackpotManager::~CJackpotManager(void)
{
}

BOOL CJackpotManager::Init()
{
	TLock lo(this);
	char sGetData[1024] = {0x00};

	// 잭팟 서비스 CHS인지 Config 파일 확인
	::GetPrivateProfileStringA("JACKPOT", "GAME", "0", sGetData, sizeof(sGetData)/sizeof(char), theControl.m_confPath.GetConfPath());
	if(0 != strlen(sGetData))
	{
		vector<string> vecData;
		::split(sGetData, ',', vecData);
		for(unsigned int i=0; i<vecData.size(); i++)
		{
			LONG lSSN = atoi(vecData[i].c_str());
			SetJackpotMoneyBySSN(lSSN, "0");
			LOG(INF, "Game Jackpot Service On(SSN:", lSSN, ")");
		}						

		m_bServiceFlagByGLS = TRUE;
	}
	else
	{
		m_bServiceFlagByGLS = FALSE;
		LOG(INF, "This GLS is not Configurated for Game Jackpot Service");
	}

	memset((void*)sGetData, 0, 1024);
	::GetPrivateProfileStringA("JACKPOT", "PERIOD", "30", sGetData, sizeof(sGetData)/sizeof(char), theControl.m_confPath.GetConfPath());
	m_lPeriod = atoi(sGetData) * 1000;

	if(TRUE == m_bServiceFlagByGLS)
	{
		// 잭팟 예시 및 당첨 브로드캐스팅 어드레스 등록
		if(FALSE == RegisterAddressFromConfigFile("JACKPOT", "MULTICAST_ADDRESS", "MJACKPOT"))
		{
			LOG(INF, "Set Default Game Jackpot Broadcasting Address \"MJACKPOT\"");
		}

		// GLS로만 발송 되는 메시지 주소 등록
		if(FALSE ==	RegisterAddressFromConfigFile("JACKPOT", "SERVICE_ADDRESS", "MCHSJACKPOT"))
		{
			LOG(INF, "Set Default Service Address \"MGLSJACKPOT\"");
		}

		char szTempAddr[LRBADDRESS_SIZE + 1] = {0};
		if (0 != ::GetPrivateProfileStringA("JACKPOT", "ODBGW_ADDRESS", "AODBJACKPOT", szTempAddr, LRBADDRESS_SIZE, theControl.m_confPath.GetConfPath()))
		{
			if (0 < ::strlen(szTempAddr))
			{
				m_addrAODBGW.SetAddress(szTempAddr);
			}
			else
			{
				LOG(ERR, "Wrong ODBGW Address");
				return FALSE;
			}
		}
		else
		{
			LOG(INF, "Set Default ODBGW Address \"AODBJACKPOT\"");
		}
	}
	
	
	if(FALSE == m_jackpotMoneyUpdateTimer.Activate(GetThreadPool(), this, 10000, m_lPeriod))
	{
		LOG(ERR, "Fail to Activate Jackpot Timer");
		return FALSE;
	}
	
	return TRUE;
}

void CJackpotManager::SetJackpotMoneyBySSN(LONG lSSN, string strJackpotMoney)
{
	TLock lo(this);
	m_mapSSNJackpotMoney[lSSN] = strJackpotMoney;
}

string CJackpotManager::GetJackpotMoneyBySSN(LONG lSSN)
{
	TLock lo(this);
	if(TRUE == IsServiceOn(lSSN))
	{
		return m_mapSSNJackpotMoney[lSSN];
	}
	return string("0");
}

BOOL CJackpotManager::IsServiceOn(LONG lSSN) const
{
	if(m_mapSSNJackpotMoney.find(lSSN) != m_mapSSNJackpotMoney.end())
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CJackpotManager::IsServiceOn() const
{
	return m_bServiceFlagByGLS;
}

BOOL CJackpotManager::RegisterAddressFromConfigFile(const char* szSectionName, const char* szKeyName, const char* szDefault)
{
  	char szTempAddr[LRBADDRESS_SIZE + 1];
	::memset(szTempAddr, 0, sizeof(szTempAddr));
	if (0 != ::GetPrivateProfileStringA(szSectionName, szKeyName, szDefault, szTempAddr, LRBADDRESS_SIZE, theControl.m_confPath.GetConfPath()))
	{
		if (0 < ::strlen(szTempAddr))
		{
			LRBAddress addr;
			addr.SetAddress(szTempAddr);
			if (FALSE == theLRBHandler.RegisterAddress(addr))
			{
				LOG(ERR, "Register Jackpot Address Error");
				return FALSE;
			}

			return TRUE;
		}
		else
		{
			LOG(ERR, "Register Jackpot Address Error");
			return FALSE;
		}
	}
	return FALSE;
}

void CJackpotManager::NotifyIndicationBySSN(LONG lSSN)
{
	/*
	RoomList lstRoom;
	theRoomTable.GetRoomList(ANNOUNCE_SSN, lSSN, lstRoom);

	LOG(INF, "Jackpot Indicaction(SSN:", lSSN, ")");
	ForEachElmt(RoomList, lstRoom, i, j)
	{
		CRoom* pRoom = (*i);
		if (!pRoom) 
		{
			continue;
		}
		pRoom->OnJackpotNotifyIndication();
		pRoom->Release();
	}
	*/
}

STDMETHODIMP_(void) CJackpotManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)		
{
	if(m_jackpotMoneyUpdateTimer.IsHandle(hSignal))
	{
		if(TRUE == m_bServiceFlagByGLS)
		{
			theChannelDir.SendJackpotMoney();
			theODBGWMgr.SendSelectJackpotMoney();
		}
	}
}

void CJackpotManager::Stop()
{
	m_jackpotMoneyUpdateTimer.Deactivate();
}

#endif