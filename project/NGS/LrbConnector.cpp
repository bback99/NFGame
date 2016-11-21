//
// LrbConnector.cpp
//

#include "stdafx.h"
#include "LrbConnector.h"
#include "RoomTable.h"
#include "ErrorLog.h"
#include "NGS.h"
#include <NLSManager.h>


extern CService _service;

#include <PMSConnObject.h>

///////////////////////////////////////////////////////////////////////////////////
ConstSetFlag theConstSetFlag;
///////////////////////////////////////////////////////////////////////////////////
CLrbManager theLrbManager;
///////////////////////////////////////////////////////////////////////////////////
// CLrbHandler

CLrbHandler::CLrbHandler(CLrbManager* pManager)
{
	m_pManager = pManager;
}

CLrbHandler::~CLrbHandler()
{
}

////////////////////////////////////////////////////////////////////////////////////////
void CLrbHandler::OnXLRBError(LONG lError)
{
	//theErr.LOG(1, "NGS_LRBHandler_Error", "%ld Error occured in OnXLRBError", lError);
	theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, lError, " Error occured in OnXLRBError");

	{
		// Local Address를 얻어냄. - AMS에게 제공하기 위함.
		string sHostAddr, sLRBIP;
		GetHostAddress(sHostAddr, TRUE);
		LPSTR lpszLRBIP = GetLRBIP();

		PMSAWarningNtf msgNtf;
		if(lpszLRBIP != NULL)
		{
			msgNtf.m_sWarnMsg  = ::format("NGS is disconnected from LRB. [IP:%s] [LRBIP:%s]\n", sHostAddr.c_str(), lpszLRBIP);
		}
		else
		{
			msgNtf.m_sWarnMsg  = ::format("NGS is disconnected from LRB. [IP:%s] [LRBIP:??]\n", sHostAddr.c_str());
		}
		msgNtf.m_sTreatMsg = ::format("Check the LRB Servers \n");
		msgNtf.m_lErrLevel = FL_CRITICAL;
		msgNtf.m_unitID.m_dwSSN = 0;
		msgNtf.m_unitID.m_dwCategory = 0;

		thePMSConnector.SendWarningMsg(msgNtf.m_lErrLevel, msgNtf.m_sWarnMsg.c_str(), msgNtf.m_sTreatMsg.c_str(), 0, 0);
	}

	//if (lError == LRBERROR_CONNECTIONCLOSE)
	{
		::InterlockedExchange((LPLONG)&theConstSetFlag.m_bLRBRun, (LONG)FALSE);
		::XLRBConnectorShutdown();
	}

	::SetEvent( _service.m_hEvent );
}

void CLrbHandler::OnXLRBRcvMsg(const DWORD dwMID, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	string sDestAddr;
	dest.GetStringFormat(sDestAddr);

	if (wProtocol == PROTOCOL_LDTP)
	{
		string sSrcAddr;
		src.GetStringFormat(sSrcAddr);

		BYTE cSvcCat = src.GetServiceCategory();

		PayloadInfo pldInfo;
		pldInfo.dwMID = dwMID;
		pldInfo.addr = src;

		switch(cSvcCat)
		{
		case SVCCAT_CHS:
			OnRcvCHSMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_NCS:
			OnRcvNCSMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_NLS:
			OnRcvNLSMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_AMS:
			OnRcvAMSMsg(pldInfo, src, dest, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_NGS:
			OnRcvNGSMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_PLS:
			OnRcvPLSMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_IBB:
			OnRcvIBBMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_RTRKS:
			OnRcvRKSMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_PCCHS:
			OnRcvCHSMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_BCS:
			OnRcvBCSMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_ODBGW:
			OnRcvODBGWMsg(pldInfo, src, dest, buf, wMessageType, wProtocol);
			break;
		case SVCCAT_NAS:
			OnRcvNASMsg(pldInfo, src, buf, wMessageType, wProtocol);
			break;
		default:
			{
				theLog.Put(WAR_UK, "NGS_LRBHandler_General"_COMMA, "Unknown service category: ", (DWORD)cSvcCat, "in OnXLRBRcvMsg");
			}
			break;
		}
	}
}

void CLrbHandler::OnXLRBRegister(LONG lErrorCode, LRBAddress& addr)
{
	static LONG lCount = 0;
	if (lErrorCode == RDA_SUCCESS)
	{
		VALIDATE(::XLRBConnectorIsRegisterd(&addr));
		LONG lRet = ::InterlockedIncrement((LPLONG)&lCount);
		if (lRet == 1)
		{
			::InterlockedExchange((LPLONG)&theConstSetFlag.m_bLRBRun, (LONG)TRUE);
			::InterlockedExchange((LPLONG)&theConstSetFlag.m_bLRBActive, (LONG) FALSE);
			::InterlockedExchange((LPLONG)&lCount, 0);
			m_pManager->InitComplete();
		}
	}	
	else
	{
		::InterlockedExchange((LPLONG)&theConstSetFlag.m_bLRBRun, (LONG)FALSE);
		::InterlockedExchange((LPLONG)&lCount, 0);
		m_pManager->InitComplete();
	}
}

void CLrbHandler::OnXLRBTerminateNtf(ListAddress& lstAddr)
{
	ForEachElmt(LRBAddressList, lstAddr.m_lstAddr, i, j)
	{
		LRBAddress _addr = *i;

		if (CASTTYPE_UNICAST == _addr.GetCastType())
		{
			string sAddr;
			_addr.GetStringFormat(sAddr);

			string strIP;
			NSAP nsapItem;

			BYTE cSvcCat = _addr.GetServiceCategory();

			switch(cSvcCat)
			{
			case SVCCAT_CHS:				
			case SVCCAT_NCS:
			case SVCCAT_LRB:
				break;
			case SVCCAT_NLS:
				theNLSManager.NLSTerminateNtf(_addr);
				break;
			case SVCCAT_PLS:
				break;
			case SVCCAT_BCS:
				break;
			case SVCCAT_IBB:							
			case SVCCAT_LRBGW:
			case SVCCAT_MGS:
			case SVCCAT_MUS:
			case SVCCAT_RTRKS:
			default:
				{
					theLog.Put(WAR_UK, "NGS_LRBHandler_General"_COMMA, "Unknown service category: ", (DWORD)cSvcCat, " in OnXLRBTerminateNtf");
				}
				break;
			}
		}
	}
}

void CLrbHandler::OnXLRBUnknownEvent(UINT pEvent, LONG lErrorCode, LPXBUF ppXBuf, LRBAddress& srcAddr, LRBAddress& destAddr)
{
}

///////////////////////////////////////////////////////////////////////////////////
// CLrbManager

CLrbManager::CLrbManager()
{
	m_dwServiceTypeID = MAKEDWORD(SVCTYP_ANY, SVCCAT_AMS);
	m_bInitialized = FALSE;
	m_pHandler = NULL;
	m_evInit.Create();

	m_dwRefCnt = 0UL;
}

CLrbManager::~CLrbManager()
{
}

STDMETHODIMP_(ULONG) CLrbManager::AddRef() 
{
	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
	return dwRefCnt;
}

STDMETHODIMP_(ULONG) CLrbManager::Release() 
{
	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
	if(dwRefCnt == 0)
	{
	}
	return dwRefCnt;
}

////////////////////////////////////////////////////////////////////////////////////////
BOOL CLrbManager::Init()
{
	BOOL bRet = FALSE;
	{
		// ----------------------------------------------------------------
		TLock lo(this);
		bRet = ::XLRBConnectorInit(GetThreadPool());
		m_bInitialized = bRet;
		::InterlockedExchange((LPLONG)&theConstSetFlag.m_bLRBActive, (LONG) bRet);
		// ----------------------------------------------------------------
	}
	return bRet;
}

BOOL CLrbManager::Run()
{
	BOOL bRet = FALSE;
	{
		TLock lo(this);
		if (m_bInitialized)
		{
			// Address를 Allocation한다.
			if (!m_pHandler)
			{
				m_pHandler = new CLrbHandler(this);
				ASSERT(m_pHandler != NULL);
			}

			for(int i = 0; i < 5; i++)
			{
				bRet = ::XLRBConnectorStartup((const LPLRBADDRESS)&theRoomTable.GetAddr(), m_pHandler);
				if (bRet)
				{
					break;
				}
			}
			for(int i = 0; i < MAX_MYADDR_CNT; i++)
			{
				if (theRoomTable.GetMyAddr(i).IsNULL() == FALSE)
					VALIDATE(::XLRBConnectorRegister((const LPLRBADDRESS)&theRoomTable.GetMyAddr(i), m_pHandler));
			}
		}
	}

	bRet = bRet && m_timerNGSInfo.Activate(GetThreadPool(), this, NGSINFO_INTERVAL, NGSINFO_INTERVAL);

	if (bRet)
	{	
		m_evInit.Lock(INFINITE);
		::InterlockedExchange((LPLONG)&bRet, (LONG)theConstSetFlag.m_bLRBRun);
		bRet = OnLrbRegistered();
	}

	return bRet;
}
void CLrbManager::RegisterMultiAddr(char *szAddr)
{		
	LRBAddress addr;
	memcpy(addr.addr, szAddr, strlen(szAddr));
	BOOL bResult = ::XLRBConnectorRegister((const LPLRBADDRESS)&addr, m_pHandler);
	VALIDATE(bResult);
	string strAddr;
	addr.GetStringFormat(strAddr);				
	theLog.Put(INF_UK, "NGS_General,Registered MulticastAddr: ", strAddr.c_str(), " Loaded. bResult=", bResult);                
}

BOOL CLrbManager::RegisterAddress(const LRBAddress& addr)
{   	
	if('U' == addr.GetCastType())
	{
		DWORD dwIP = GSocketUtil::GetHostIP();
		string sID = LRBAddress::GetIDFromIP(dwIP);
		string strAddr = std::string("U") + addr.ExtractServiceName() + sID;
		LRBAddress addrUNGS;
		addrUNGS.SetAddress(strAddr.c_str());
		theRoomTable.SetAddr(addrUNGS);
		for(int i=0; i<5; i++)
		{
			if(::XLRBConnectorStartup((const LPLRBADDRESS)&theRoomTable.GetAddr(), m_pHandler)) 
			{
				return TRUE;
			}
			theLog.Put(ERR_UK, __FUNCTION__, "::Fail to Register UNICAST address, but try again ", i);
		}
		theLog.Put(ERR_UK, __FUNCTION__, "::Fail to Register UNICAST address");
		return FALSE;
	}
	else { 
		if(!::XLRBConnectorRegister((const LPLRBADDRESS)&addr, m_pHandler))
		{
			theLog.Put(ERR_UK, __FUNCTION__, "::Fail to Register address");
			return FALSE;
		}
	}
	theLog.Put(INF_UK, "Success to  Register address ", addr.GetString());
	return TRUE;
}

BOOL CLrbManager::Stop()
{
	BOOL bRet = FALSE;
	{
		TLock lo(this);
		m_bInitialized = FALSE;
		bRet = ::XLRBConnectorShutdown();

		m_timerNGSInfo.Deactivate();
	}
	return bRet;
}

STDMETHODIMP_(void) CLrbManager::OnSignal(HSIGNAL hObj, WPARAM wParam, LPARAM lParam)
{
	if (hObj == 0)
	{
	}	
	else
	{
		TLock lo(this);

		if (m_timerNGSInfo.IsHandle(hObj))
		{
			NotifyNGSInfoToNCS(theRoomTable.GetNCSAddr());
		}
	}
}