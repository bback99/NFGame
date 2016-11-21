//
// Control.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "Agent.h"
#include "Control.h"
#include "ErrorLog.h"
#include "Listener.h"
#include "LrbConnector.h"
#include "RoomTable.h"
#include <NLSManager.h>

#ifdef _USE_PMS
#include <PMSConnObject.h>
#endif // _USE_PMS

///////////////////////////////////////////////////////////////////////////////////
// CControl
extern CLrbManager theLrbManager;

CControl theControl;

CControl::CControl()
{
}

CControl::~CControl()
{
}

BOOL CControl::Run()
{
	TLock lo(this);

	//Global Object 관련 코드 dll 이름, SSN 정보를 알기위해 임시 저장
	map<LONG, string> mapGlobalTemp;
	DWORD dwRet = 0;
	CHAR szSSN[512];
	DWORD dwPort = 0;

	dwRet = ::GetPrivateProfileStringA("SSN", "1", "0", szSSN, sizeof(szSSN), theControl.m_confPath.GetConfPath()/*"GRCConfig.INI"*/);
	if (dwRet) 
	{
		g_wSvcType = (WORD)::atoi(szSSN);
		if (!g_wSvcType)
			g_wSvcType = (WORD)SVCTYP_ANY;
	}
	else 
	{
		g_wSvcType = (WORD)SVCTYP_ANY;
	}
	dwPort = NGS_PORT_CLIENT_ANY;

	BOOL bRet = FALSE;

	//TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	CHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	szComputerName[0] = 0;
	DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
	bRet = ::GetComputerNameA(szComputerName, &dwSize);
	if (!bRet)
	{
		theLog.Put(ERR_UK, "NGS_General_Error"_COMMA, "*** Failed to get host name ***");
		return FALSE;
	}
	theRoomTable.SetComputerName(szComputerName, dwSize);

	theRoomTable.SetTypeID(MAKELONG(g_wSvcType, SVCCAT_NGS));

	LRBAddress tempaddr;
	tempaddr.Clear();

	CHAR szTempAddr[4] = {0};
	CHAR szTempMyAddr[LRBADDRESS_SIZE+1] = {0};

	string strLRBAddrIni = theControl.m_confPath.GetConfDir();
	strLRBAddrIni += "LRB_ADDR.ini";
	::GetPrivateProfileStringA("UNICAST", "NGS", "NGS", szTempAddr, 4, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);
	DWORD dwTempLength = strlen(szTempAddr);
	memset(szTempAddr + dwTempLength, LRBADDRESS_NULL, 3 - dwTempLength);

	DWORD dwIP = GSocketUtil::GetHostIP(true);
	string sID = adl::LRBAddress::GetIDFromIP(dwIP);

	memcpy(szTempMyAddr, szTempAddr, 3);
	memcpy(szTempMyAddr+3, sID.c_str(), sID.size());

	tempaddr.SetAddress(CASTTYPE_UNICAST, szTempMyAddr);

	theRoomTable.SetAddr(tempaddr);

	/////////// 추가 MyAddress(Uni, Any, Bro, Multi)
	theRoomTable.SetMyAddr(tempaddr, 0);
 
	for(int i = 0; i < MAX_MYADDR_CNT ; i++)
	{
		memset(szTempMyAddr, 0x00, LRBADDRESS_SIZE+1);

		tempaddr.Clear();
		if (i == NGSME_MUL)
			::GetPrivateProfileStringA("MULTICAST", "NGS", "NGS", szTempAddr, 4, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);
		else if (i == NGSME_ANY)
			::GetPrivateProfileStringA("ANYCAST", "NGS", "NGS", szTempAddr, 4, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);
		else if (i == NGSME_BRO)
			::GetPrivateProfileStringA("BROADCAST", "NGS", "NGS", szTempAddr, 4, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);

		dwTempLength = strlen(szTempAddr);
		memset(szTempAddr + dwTempLength, LRBADDRESS_NULL, 3 - dwTempLength);

		memcpy(szTempMyAddr, szTempAddr, 3);

		if (i == NGSME_UNI)
		{
			memcpy(szTempMyAddr+3, sID.c_str(), sID.size());
			tempaddr.SetAddress(CASTTYPE_UNICAST, szTempMyAddr);

			theRoomTable.SetMyAddr(tempaddr, i);
		}
		
		if (i == NGSME_MUL)
		{
			tempaddr.SetAddress(CASTTYPE_MULTICAST, szTempMyAddr);

			theRoomTable.SetMyAddr(tempaddr, i);

			theRoomTable.SetMulticastAddr(tempaddr);
		}
	}

	// Set NCS Address
	memset(szTempMyAddr, 0x00, LRBADDRESS_SIZE+1);

	tempaddr.Clear();
	::GetPrivateProfileStringA("MULTICAST", "NCS", "NCS", szTempAddr, 4, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);

	dwTempLength = strlen(szTempAddr);
	memset(szTempAddr + dwTempLength, LRBADDRESS_NULL, 3 - dwTempLength);
	memcpy(szTempMyAddr, szTempAddr, 3);

	tempaddr.SetAddress(CASTTYPE_MULTICAST, szTempMyAddr);
	theRoomTable.SetNCSAddr(tempaddr);

	// Set CHS Adddess
	memset(szTempMyAddr, 0x00, LRBADDRESS_SIZE+1);

	tempaddr.Clear();
	::GetPrivateProfileStringA("MULTICAST", "CHS", "CHS", szTempAddr, 4, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);

	dwTempLength = strlen(szTempAddr);
	memset(szTempAddr + dwTempLength, LRBADDRESS_NULL, 3 - dwTempLength);

	memcpy(szTempMyAddr, szTempAddr, 3);
	//memcpy(szTempMyAddr+3, sID.c_str(), sID.size());

	tempaddr.SetAddress(CASTTYPE_MULTICAST, szTempMyAddr);
	/*szTempAddr[14] = SVCCAT_CHS;
	tempaddr.SetAddress(CASTTYPE_MULTICAST, (LPBYTE)szTempAddr, LRBADDRESS_SIZE);*/
	theRoomTable.SetChsAddr(tempaddr);

	// Set NLS Address
	memset(szTempMyAddr, 0x00, LRBADDRESS_SIZE+1);
	tempaddr.Clear();
	::GetPrivateProfileStringA("MULTICAST", "NLS", "NLS", szTempAddr, 4, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);
	dwTempLength = strlen(szTempAddr);
	memset(szTempAddr + dwTempLength, LRBADDRESS_NULL, 3 - dwTempLength);
	memcpy(szTempMyAddr, szTempAddr, 3);
	tempaddr.SetAddress(CASTTYPE_MULTICAST, szTempMyAddr);
	theRoomTable.SetNLSAddr(tempaddr);

	// Set NAS Address
	memset(szTempMyAddr, 0x00, LRBADDRESS_SIZE+1);
	tempaddr.Clear();
	::GetPrivateProfileStringA("MULTICAST", "NAS", "NAS", szTempAddr, 4, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);
	dwTempLength = strlen(szTempAddr);
	memset(szTempAddr + dwTempLength, LRBADDRESS_NULL, 3 - dwTempLength);
	memcpy(szTempMyAddr, szTempAddr, 3);
	tempaddr.SetAddress(CASTTYPE_MULTICAST, szTempMyAddr);
	theRoomTable.SetNASAddr(tempaddr);

	bRet = theLrbManager.Init();
	if (!bRet)
	{
		theLog.Put(ERR_UK, "NGS_theControl_Error"_COMMA, "LRBConnector Table instance initializing failed");
		return FALSE;
	}
	else
		theLog.Put(INF_UK, "NGS_General"_COMMA, "LRBConnector table instances are initialized");

	if ((!theConstSetFlag.m_bLRBRun )&&(theConstSetFlag.m_bLRBActive))
	{
		bRet = theLrbManager.Run();
		if (!bRet)
		{
			theLog.Put(ERR_UK, "NGS_theControl_Error"_COMMA, "Running theLrbManager failed");
			return FALSE;
		}

		theLog.Put(INF_UK, "NGS_General"_COMMA, "theLrbManager is running");
	}

	int nRoomTitleMonitor = ::GetPrivateProfileIntA("MONITOR", "ROOMTITLE", 0, theControl.m_confPath.GetConfPath());
	if (nRoomTitleMonitor == 0)
	{
		theLog.Put(INF_UK, "NGS_General"_COMMA, "Room title monitor disabled");
		theRoomTable.SetRoomTitleMonitoring(FALSE);
	}
	else
	{
		theLog.Put(INF_UK, "NGS_General"_COMMA, "Room title monitor activated");
		theRoomTable.SetRoomTitleMonitoring(TRUE);
	}

	// 2009.07.06 - searer
	// Message Encryption
	//int nMsgEncrypt = ::GetPrivateProfileIntA("COMMON", "MsgEncrypt", 1, theControl.m_confPath.GetConfPath());
	//if (0 == nMsgEncrypt)
	//{
	//	theLog.Put(INF_UK, "NGS_General"_COMMA, "Message Encryption is disabled.");
		theEncryptMgr.SetMsgEncrypt(FALSE);
	//}
	//else
	//{
	//	theLog.Put(INF_UK, "NGS_General"_COMMA, "Message Encryption is enabled.");
	//	theEncryptMgr.SetMsgEncrypt(TRUE);
	//}
	
	bRet = theListener.Run(dwPort);
	VERIFY(bRet);
	if (!bRet) return FALSE;

	theAccuseAgent.RunAgent();

	NSAP nsap = theRoomTable.GetNSAP();

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Only - NFGame
	bRet = theNLSManager.RunNLSManager(nsap);
	if (bRet)
		theLog.Put(INF_UK, "NGS_General"_COMMA, "theNLSManager is running");
	else
		theLog.Put(ERR_UK, "NGS_theControl_Error"_COMMA, "Running theNLSManager is failed");

	// Select NF Resource Data 
	bRet = theNFDataItemMgr.GetAllResourceData(true);
	if (bRet)
		theLog.Put(INF_UK, "NGS_General"_COMMA, "theNFDataItemMgr GetAllResourceData");
	else
		theLog.Put(ERR_UK, "NGS_theControl_Error"_COMMA, "theNFDataItemMgr GetAllResourceData is failed");

	// Calc Distance-FP2FP
	bRet = theNFDataItemMgr.CalcDistFP2FP();
	if (bRet)
		theLog.Put(INF_UK, "NGS_General"_COMMA, "theNFDataItemMgr CalcDistFP2FP");
	else
		theLog.Put(ERR_UK, "NGS_theControl_Error"_COMMA, "theNFDataItemMgr CalcDistFP2FP is failed");
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// PMS
	DWORD dwPMSRet = thePMSConnector.Run();
	if (PMSC_ERR_OK != dwPMSRet)
	{
		theLog.Put(ERR_UK, "NGS_theControl_Error"_COMMA, "PMS : Run() FAILED /// ErrorCode : ", dwPMSRet);
		return FALSE;
	}
	else
	{
		theLog.Put(INF_UK, "NGS_General"_COMMA, "thePMSConnector is running");
	}

	return TRUE;
}

BOOL CControl::Stop()
{
	TLock lo(this);

	BOOL bRet = TRUE;

//	bRet = theManager.Stop(); // bcz this is outer thread...
//	VERIFY(bRet);
	bRet = theListener.Stop(); // bcz this is outer thread...
	VERIFY(bRet);

	bRet = theLrbManager.Stop();

	thePMSConnector.Stop();
	theNFDataItemMgr.DeleteAllResourceData();

	return TRUE;
}

string CControl::GetParseData(string& sTarget)
{
	string sRet;
	int nIndex = sTarget.find_first_of(",");
	if ( nIndex != NPOS )
	{
		sRet = sTarget.substr(0, nIndex);
		sTarget.erase( 0, nIndex + 1 );
	}
	else
	{
		sRet = sTarget;
		sTarget.erase();
	}
	return sRet;
}

CConfPath::CConfPath()
{
	if(!SetConfPath("GRCConfig.ini"))
		memset(m_sConfigPath, 0, sizeof(m_sConfigPath));
}

BOOL CConfPath::SetConfPath(const char* strfilename)
{
	ZeroMemory(m_sConfigDir, sizeof(m_sConfigDir));
	strcpy(m_sConfigPath, strfilename);

#ifdef _DEBUG
	printf("  !!! Configure File Path = [%s]\n", m_sConfigPath);
#endif
	return TRUE;
}