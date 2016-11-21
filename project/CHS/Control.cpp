//
// Control.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LRBHandler.h"
#include "Listener.h"
#include "Control.h"
//#include "ADL/MsgLRBService.h"
#include "Channel.h"
#include "ChannelDir.h"
#include "Reporter.h"
#include "Agent.h"
#include <NFVariant/NFGameData.h>

#ifdef _USE_MSGCOUNT
#include "StatisticsTable.h"
#endif

#define MAX_INIT_CNT 60		// 30 second, create channel....이 시간 동안 응답이 없으면 system 종료

#define MAX_RUNLRB_CNT 40	// boot LRB.....이 시간 동안 응답이 없으면 system 종료

#define MAX_REGINST_CNT 10	// register All Instnace .....이 시간 동안(4초) NACK이나 Error가 없으면 성공한 것으로.
#define MAX_REGLB_CNT 10	// wait NACK msg .... 이 시간 동안(2초) NACK이나 Error가 없으면 성공한 것으로.

#ifdef _CHSNLS
	#include <NLSManager.h>
#endif

#include "ChatAgent.h"
///////////////////////////////////////////////////////////////////////////////////
// CControl

CControl theControl;

CControl::CControl()
{
	m_lCHSID = 0xFFFF;
	m_lInitCount = m_lRunLRBCount = m_lRegInstCnt = m_lRegLBCnt = 0L;

	// BLS와는 등록 메세지를 사용하지 않고, NACK을 받아 처리한다.
	// BLS는 기동시 무조건 registe 메세지를 multicating한다.
	m_bFlagRunBLS = TRUE;
	m_bFlagRunIBB = TRUE;    
}

CControl::~CControl()
{
}
BOOL CControl::InitChannel()
{
	//CHS가 이미 떠 있을 경우, 새로운 Process의 생성을 막는다.
	HANDLE hMutex = ::CreateMutex(NULL, FALSE, _T("_CHS_"));
	if(!hMutex || ::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(INF_UK, "CHS_CControl"_LK, "already run CHS");
		return FALSE;
	}
 
	string s_ip = GSocketUtil::GetHostIPString();
	//GetIpAddress(s_ip);
	NSAP nsap(0UL, CHS_BASE_PORT);
	nsap.SetIP(s_ip);
	theChannelDir.SetCHSNsap(nsap);	

	if(!theReporter.RunReport())
	{
		TLOG0("*** Fail to Reporter Running***\n");
		LOG(INF_UK, "CHS_CControl_Err"_LK, "Fail to Reporter Running");
		return FALSE;
	}

	if(!theAccuseAgent.RunAgent())
	{
		TLOG0("*** Fail to run a agent for accusement***\n");
		LOG(INF_UK, "CHS_CControl_Err"_LK, "Fail to run a agent for accusement");
	}
	if(!theChannelDir.CreateDefinedChannelX())
	{
		LOG(INF_UK, "CHS_CControl_Err"_LK, " fail Channel Creating : Initialize ");
		return FALSE; 
	}
	// 빠찡코 같이 미리 게임방을 만들어야 할 경우에 SSN을 읽어오는 작업을 해야 한다.
	theChannelDir.InitPreCreatedRoom();
	if (theChannelDir.m_mapPreRoom.size() > 0)
	{
		// 해당 CHS 가 미리게임방생성하는 게임의 SSN을 서비스 하는지 체크
		theChannelDir.SettingPreCreatedSSN();
	}

	// Select NF Resource Data 
	if (theNFDataItemMgr.GetAllResourceData())
		theLog.Put(INF_UK, "CHS_CControl_Err"_COMMA, "theNFDataItemMgr GetAllResourceData");
	else
		theLog.Put(ERR_UK, "CHS_CControl_Err"_COMMA, "theNFDataItemMgr GetAllResourceData is failed");

	//// ACHV BEGIN
//	if (false == g_achv.addReportCallback(CChannelContext::AchvReportCallback))
//	{
//		theLog.Put(ERR_UK, "NCS_theControl_Error"_COMMA, "achv::Bureau::init() failed.");
//		return FALSE;
//	}
	//// AHCV END

	// for ChatAgent
	theChatAgent.RunChatAgent(CHAT_AGENT_PORT);

	SetBootState(CHS_CONNECT_LRB);
#ifdef _USE_MSGCOUNT
	theStatTable.Init();
#endif

	return TRUE;
}

void CControl::SetBootState(LONG lState)
{
	TLock lo(this);
	m_lBootState = lState;
}

void CControl::SetBootStateOut(LONG lState)
{
	TLock lo(this);
	theReporter.ChangeInterval(2000, 500);
	m_lBootState = lState;
}

LONG CControl::GetBootState()
{
	TLock lo(this);
	return m_lBootState;
}



BOOL CControl::RunControl()
{
	//TLock lo(this);	// 이것에 따라서 전혀 다른 Sequence
	// 0.5초에 한번씩 호출됨...

	LONG lCHSINFO = 0L;
	char szTemp[256] = {0, };
	DWORD dwRet = 0;
	switch(GetBootState())
	{
	case CHS_STOP:	// service가 Init이 되기 전이거나 종료 process를 진행중인 상태.
		return FALSE;
		break;
	case CHS_CONNECT_LRB:	// LRB와 연결 
		SetBootState(CHS_CONNECTING_LRB);
		if(!theLRBHandler.RunLRBHandler()) 
		{			
			LOG(INF_UK, "CHS_CControl_Err"_LK, "fail Run LRBHandler.. ");
			SetBootState(CHS_CONNECT_LRB);
			return FALSE;
		}
		SetBootState(CHS_REG_LB);
		break;
	case CHS_REG_LB:	
		SetBootState(CHS_START_LISTENER);
		break;
	case CHS_START_LISTENER:
		{
			LOG(INF_UK, "CHS_CControl"_LK, "============================================");
			if(theListener.m_bStating)
			{
				SetBootState(CHS_RUN);
				theLRBHandler.TryServiceRegistToNCS(theChannelDir.GetCHSNsap());

				theChannelDir.SettingPreCreatedSSN();
				if (theChannelDir.m_bSetPreCreateSSN)
					theLRBHandler.SendChannelIDListToNGS();
				break;
			}
			theReporter.SetAllDiffFlag();

			if(!theListener.RunListen(CHS_BASE_PORT))
			{
				LOG(INF_UK, "CHS_CControl_Err"_LK, "*********** fail Starting Listener [", m_lCHSID, "]CHSID***********");
				return FALSE;
			}

	#ifdef _CHSNLS		
			if(!theNLSManager.RunNLSManager(theChannelDir.GetCHSNsap()))
			{
				LOG(INF_UK, "CHS_CControl_Err"_LK, "fail to run NLSManager.. ");
				return FALSE;
			}
			else
				LOG(INF_UK, "CHS_CControl"_LK, "=====> Start NLSManager to run NLSManager!!! ");
			
	#endif
	
			VALIDATE(theInviteListener.RunListen(CHS_BASE_PORT + 1));
			SetBootState(CHS_RUN);
			
			m_lRegInstCnt = m_lRegLBCnt = m_lRunLRBCount = 0L;

			dwRet = ::GetPrivateProfileStringA("TIMER", "CHSINFO", "", szTemp, sizeof(szTemp), theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/);
			if(dwRet == 0) {
				lCHSINFO = 60000L;
			}
			else 
				lCHSINFO = atoi(szTemp);

			if(theListener.m_bStating)
			{
				theReporter.ChangeInterval(lCHSINFO, lCHSINFO);
				LOG(INF_UK, "CHS_CControl"_LK, "*********** Completed Starting CHS ***********");
			} 
			theLRBHandler.TryServiceRegistToNCS(theChannelDir.GetCHSNsap());		// Return Value를 check 하지 않음, 무조건 성공으로 가정...--> LB

			theChannelDir.SettingPreCreatedSSN();

			// RoomList를 요청하는 경우이면...
			if (theChannelDir.m_bSetPreCreateSSN)
			{
				theLRBHandler.SendChannelIDListToNGS();
			}
			break;
		}

	case CHS_RUN:
		break;
	default:
		LOG(INF_UK, "CHS_CControl"_LK, "+++++ Rest Event signal or Unknown Boot State ", GetBootState(), "+++++++");
		break;
	}
	return TRUE;
}

BOOL CControl::StopControl()
{
	TLock lo(this);

	theReporter.StopReport();

	//나머지 메모리 해제나 클라이언트와의 접속해제는 무시한다. 
	if(theListener.StopListen()) 
	{ 
		LOG(INF_UK, "CHS_CControl"_LK, "=============== Stop CHS Listener ======================");
	}
	VALIDATE(theInviteListener.StopListen());

	theLRBHandler.StopLRBHandler();
	return TRUE;
}

BOOL CControl::GetIpAddress(string &s_ip)
{
	char cHostName[255] = {0, };
	char cOuterAddress[100] = {0, };
	HOSTENT* hostinfo;
	char *pTempAddr = NULL;

	hostinfo = ::gethostbyname(cHostName);
	memset(cHostName, 0, 100);
	::gethostname(cHostName, 100);
	if(hostinfo)
	{
		for(int i=0 ; *(hostinfo->h_addr_list) != NULL; i++)
		{
			memset(cOuterAddress, 0, strlen(cOuterAddress));
			pTempAddr = inet_ntoa(*(struct in_addr*)*( (hostinfo->h_addr_list)++ ));
			if(pTempAddr != NULL) 
			{
				memcpy(cOuterAddress, pTempAddr, strlen(pTempAddr));
			}
		}
		if(strlen(cOuterAddress) < 7) {
			return FALSE;
		}
	}
	s_ip = cOuterAddress;
	return TRUE;
}

BOOL CControl::GetComputerName(string & comp_name)
{
	char buff[100] = {0, };
	DWORD dwLen = sizeof(buff);
	if(!::GetComputerNameA(buff, &dwLen)) 
	{
		LOG(INF_UK, "CHS_CControl"_LK, "+++++ fail GetCoumpterName +++++");
		return FALSE;
	}
	comp_name = buff;
	return TRUE;
}

CConfPath::CConfPath()
{
	if(!SetConfPath(CONFIG_FILENAME))
		memset(m_sConfigPath, 0, sizeof(m_sConfigPath));
}

BOOL CConfPath::SetConfPath(const char* strfilename)
{
	strcpy(m_sConfigPath, strfilename);
	return TRUE;
}