//
// RoomInternalLogic.cpp
//

#include "stdafx.h"
#include "common.h"
#include "RoomInternalLogic.h"
#include "RoomTable.h"
#include "ErrorLog.h"

//#include "DBGWManager.h"

#include "NotifyToOthers.h"
#include "RoomTimerManager.h"

BOOL CRoomInternalLogic::GetNextUserMsg( PayloadCliNGS & pld, LONG usn )
{
	CUser * pUser = UserManager->FindUser( usn );

	if( pUser )
	{
		 return pUser->PopMsg( pld );
	}
	else return false;
}

LONG CRoomInternalLogic::SendMessageToServer(LONG lServerType,  DWORD wParam, string &sContents)
{
	GBuf buf;
	buf.AddRight((void*)sContents.c_str(), sContents.size());

	switch (lServerType)
	{
	default:
		return CTERR_INVALIDPARAM;
	}
}

long RegUser(UserBaseInfo& ud)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// CRoomInternalLogic

CRoomInternalLogic::CRoomInternalLogic(const DWORD dwRID, IRoomEventHandler * pHandler)
:
m_dwRID( dwRID ),
m_bChatForward( FALSE ),
m_lock_cu_debug( 0 ),
m_lock_this_debug( 0 ),
m_lRoomState( 0L ),
m_bIsInRoomInfoTable( FALSE ),
UserManager( new CRoomUserManager (pHandler) ),
m_pTimerManager( new CRoomTimerManager() ),
NotifyUtil( new CNotifyToOthers() )
{
	m_lSignCurrentPoint = 0;
	m_lSignMaxPoint = 0;
	m_lSignType = 0;
	m_lMapLodingTime = 0;

	m_totLandingFishTeamA.Clear();
	m_totLandingFishTeamB.Clear();

	m_nfRoomOption.Clear();
	m_nfRoomOption.m_lRoomStatus = ROOMSTATE_STOP;
}

CRoomInternalLogic::~CRoomInternalLogic()
{
	UserManager->clear();
	if( NotifyUtil )
	{
		delete NotifyUtil;
		NotifyUtil = NULL;
	}
	//	ASSERT(!m_pGRC);
	//	ASSERT(!m_pRoom);
}




/*
BOOL CRoomInternalLogic::AddUser(CUser* pUser)
{
return UserManager->AddUser( pUser );

CLink* pLink = pUser->GetLink();
//	ASSERT(pLink);
if (!pLink) return FALSE;

BOOL bRet = AddLink(pLink);

//	ASSERT(bRet);
if (!bRet) return FALSE;

bRet = mUsers.Add(pUser);
if (!bRet) return FALSE;

return TRUE;

}
*/

/*
//void CRoomInternalLogic::RemoveUser(CUser* pUser)
void CRoomInternalLogic::RemoveUser(long lCSN)
{
// jsp : for LCS 
//	UserManager->RemoveUser( pUser->GetUSN());
UserManager->RemoveUser( lCSN);
*
mUsers.Remove(pUser);
CLink* pLink = pUser->GetLink();
if(pLink)
{
RemoveLink(pLink);
pLink->Unregister();
}
*
}
*/

CUser* CRoomInternalLogic::FindUser(long lCSN)
{
	return UserManager->FindUser( lCSN );
}

/*
CUser* CRoomInternalLogic::FindUser(xstring sLUserID)
{
return mUsers.Find(sLUserID);
}
*/
LONG CRoomInternalLogic::GetState() const
{
#pragma oMSG("TODO : GetState()")
	return ROOMSTATE_DEAD;
}



//string CRoomInternalLogic::GetTitle() const
//{
//#pragma oMSG("TODO : GetTitle()")
//	return string(_T(""));
//}

//BOOL CRoomInternalLogic::OnRun(const RoomID& roomID, const RoomOption& roomOption)
BOOL CRoomInternalLogic::OnRun(const RoomID& roomID, const NFRoomOption& nfRoomOption, const string& sGameOption, IXObject * pTimerHandler )
{

	if( false ==  UserManager->Start( GetThreadPool() ) )
		return FALSE;

	m_bIsInRoomInfoTable = FALSE;
	m_nfRoomOption.BCopy(nfRoomOption);

	m_CHSAddr.Clear();
	SetRoomID( roomID );
	NotifyUtil->SetRoomData( m_dwRID, m_RoomID );

	m_pTimerManager->InitializeVar( GetRoomIDStr(), pTimerHandler ); 

	m_sGameOption.erase();
	m_sGameOption.assign(sGameOption.c_str(), sGameOption.length());

	return TRUE;
}



void CRoomInternalLogic::OnStop()
{
	UserManager->Stop();
	UserManager->DestroyAllUser();

	UserManager->clear();

	NotifyUtil->NotifyRemoveRoomToChs();
	m_ChatHistory.ClearHistory();
	m_sUserGameDataList.erase();
}

BOOL CRoomInternalLogic::GetRoomInfoInChannel(NFRoomInfoInChannel &roomInfo)
{
	roomInfo.m_dwGRIID = m_RoomID.m_dwGRIID;
	roomInfo.m_nsapGLS = theRoomTable.GetNSAP();
	roomInfo.m_lRoomState = m_lRoomState;
	roomInfo.m_lRoomType =0x00; // 어떤 값을 줘야 될까?
	roomInfo.m_roomOption = m_nfRoomOption;
	roomInfo.m_sGameOption = m_sGameOption;	

	UserManager->GetAllUserBaseInfo( roomInfo.m_lstNFUserBaseInfo );
	return TRUE;
}

void CRoomInternalLogic::OnCutOutUser( string & sUserInfo, long lCSN, LinkCutReason reason )
{
	CUser * pUser = UserManager->FindUser( lCSN );
	if( NULL == pUser )
		return;

	const NFCharBaseInfo& nfCBI = pUser->GetNFCharInfoExt()->m_nfCharBaseInfo;
	string sUID = nfCBI.m_strCharName.substr(0, nfCBI.m_strCharName.length()-3);
	
	sUserInfo.clear();
	sUserInfo = ::format("%s(%s***)", nfCBI.m_strCharName.c_str(), sUID.c_str());
	
	
	theLog.Put(DEV_UK, "OnCutOutUser,  USN = ", lCSN );
	switch( reason )
	{
	case LCS_LINKCUT_KICKOUT:
		{
			// 방에서만 강퇴.
			// if msgtype is terminate_room then notify it to GRC
			theLog.Put(WAR_UK, "NGS_CROOM"_COMMA, "Kickout User by LCS: ", sUserInfo);
		}
		break;
	case LCS_LINKCUT_DISCONNECT:

		theLog.Put(ERR_UK, "NGS_CROOM"_COMMA, lCSN, "is disconnect by LCS: ", sUserInfo);
		break;
	default:
		break;
	}



}

void CRoomInternalLogic::SetRoomID( const RoomID & rID )
{
	m_RoomID = rID;
	RoomID2Str( m_RoomIDStr, m_RoomID );
}

const RoomID & CRoomInternalLogic::GetRoomID() const
{
	return m_RoomID;
}

void CRoomInternalLogic::SetRoomAddr( const LRBAddress & Addr )
{
	m_addr = Addr;
}

const LRBAddress & CRoomInternalLogic::GetRoomAddr()
{
	return m_addr;
}

const xstring & CRoomInternalLogic::GetRoomIDStr(  )
{
	return m_RoomIDStr;
}


int CRoomInternalLogic::DBResultParser(const DBGW_XString& strDBResult, list<xchar *>& lstData, BOOL bSP/* = FALSE*/)
{
	int nResult = 1;
	int nSize = strDBResult.GetSize();
	if(nSize <= 0)
		return -1;

	/* format
	* ’S’ + Delimiter + ‘0’(SP의 경우엔 SP 성공값) + Delimiter +결과 string
	* ’F’ + Delimiter + DB 에러코드 + Delimiter + Stored Proc 에러코드 + Delimiter + 에러 string
	*/
	xchar *pStrSrc = strDBResult.GetData();
	xchar *p = pStrSrc;

	// 성공 or 실패 (S/F)
	xchar cResult = *pStrSrc;

	if(cResult == _X('F'))
	{
		// FORMAT
		// Delimiter : '|'
		//‘F’ + Delimiter + DB 에러코드 + Delimiter + Stored Proc 에러코드 + Delimiter + 에러 string

		theLog.Put(INF_UK, "NGS_DB_INF,", " DB Failed : ", pStrSrc);

		// 'F' => 버린다.
		GetNextItemOfDBResult(p);

		// DB 에러코드
		lstData.push_back(GetNextItemOfDBResult(p));

		// SP일때만 SP error code를 넣어준다.
		if( bSP )
		{
			// stored  proc 에러 코드
			lstData.push_back(GetNextItemOfDBResult(p));
		}

		// 에러 string => 버린다.

		return -1;
	}

	// 아래 두 가지를 건너뛰기 위해
	// 'S' : 성공을 나타내는 문자
	// DB 에러코드
	GetNextItemOfDBResult(p);
	GetNextItemOfDBResult(p);

	//if( strSrc.length()  == 0 ) 
	if( NULL == p )
	{
		// row가 없음
		return 0;
	}
	else if( *p == _X('\0'))
	{
		// row가 없음
		theLog.Put(DEV_UK, "NGS_DB_DEV,", " DB Result NO ROW ");
		return 0;
	}

	xchar *ptemp = p;
	while(TRUE)
	{
		// field에 data가 비어 있는 경우, ""을 넣어준다.
		lstData.push_back(ptemp);
		ptemp = GetNextItemOfDBResult(p);

		if( NULL == p)
		{
			break;
		}
		else if( *p == _X('\0'))
		{
			break;
		}

	}

	if ( !lstData.size() )
		return 0;


	return nResult;
}

LONG CRoomInternalLogic::AddTimer(DWORD dwDue, DWORD dwPeriod, LONG lTimerIndex )
{
	return m_pTimerManager->AddTimer( dwDue, dwPeriod, lTimerIndex );
}

LONG CRoomInternalLogic::RemoveTimer(LONG lIndex)
{
	return m_pTimerManager->RemoveTimer( lIndex );
}

LONG CRoomInternalLogic::RemoveAllTimer()
{
	return m_pTimerManager->RemoveAllTimer();
}

BOOL CRoomInternalLogic::ProcessTimerSignal( LONG & UserTimerIndex, const HSIGNAL &hObj, const WPARAM &wParam, const LPARAM &lParam, DWORD queueTime )
{
	if( m_pTimerManager->ProcessTimerSignal( UserTimerIndex, hObj, wParam, lParam, queueTime ) )
	{
		CAutoLockCheck lc("CRoomInternalLogic::OnTimer", &m_lock_this_debug);
		return true;
	}

	return false;
}

void CRoomInternalLogic::SetCHSAddr(const LRBAddress &chsAddr)
{ 
	m_CHSAddr = chsAddr;
	NotifyUtil->SetCHSAddr( chsAddr );
}

xchar *CRoomInternalLogic::GetNextItemOfDBResult(xchar *&p)
{
	p = xstrstr(p, _X("|"));
	if(NULL == p) return p;
	*p = _X('\0');
	++p;
	return p;
}