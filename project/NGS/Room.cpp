//
// Room.cpp
//

#include "stdafx.h"
#include "Room.h"
#include "RoomInternalLogic.h"
#include "RoomTable.h"
#include "Listener.h"
#include "StatisticsTable.h"
#include "ErrorLog.h"
#include "ADL/MsgLCSCommon2.h"
#include "NF/ADL/MsgCHSNGS.h"
#include "ADL/msgLCSMSG2.h"
#include "LRBConnector.h"
#include "NF/ADL/MsgNCSNGS.h"

#include "Agent.h"
#include "Control.h"


typedef LONG (*CreateGRCFunc)(LONG, const IGRCContainer* const, IRoomContext**);
typedef LONG (*DestroyGRCFunc)(LONG, IRoomContext*);

CIRCManager::CIRCManager( CRoomInternalLogicPtr pLogic)
{

}

CIRCManager::~CIRCManager()
{
	
}

///////////////////////////////////////////////////////////////////////////////////
// CRoom

CRoom::CRoom() : m_dwRefCnt(0),  m_pIRC( NULL )
{
	static s_dwRID = 0;
	m_dwRID = ::InterlockedIncrement((LPLONG)&s_dwRID);

	m_pInternalLogic = new CRoomInternalLogic( m_dwRID, this );
	m_pIRCManager = new CIRCManager( m_pInternalLogic );
	

//	MC_CHECK_AUTOPTR(m_pInternalLogic, m_dwStatus);

//	m_bIsInRoomInfoTable	= FALSE;
}

CRoom::~CRoom() 
{
}

STDMETHODIMP_(ULONG) CRoom::AddRef()
{
	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
	return dwRefCnt;
}

STDMETHODIMP_(ULONG) CRoom::Release()
{
	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
	if(dwRefCnt == 0)
	{
		FreeRoom(this);
	}
	return dwRefCnt;
}

LONG CRoom::GetRunStopFlag()
{
	return m_pInternalLogic->GetRunStopFlag();
}
void CRoom::SetRunStopFlag(LONG flag)
{
	m_pInternalLogic->SetRunStopFlag( flag );
}

long CRoom::GetState() const 
{
	return m_pInternalLogic->GetState();
}

void CRoom::SetChatForward()
{ 
	m_pInternalLogic->SetChatForward();
}
void CRoom::ResetChatForward()
{ 
	m_pInternalLogic->ResetChatForward(); 
}


LONG CRoom::GetUserState(LONG lCSN, LONG lType)
{
	TLock lo(this);
	LONG ret = INVALID_USN;
	return (ret == lCSN) ? 1:0;
}
//string CRoom::GetTitle() const 
//{
//	return m_pInternalLogic->GetTitle(); 
//}

const RoomID& CRoom::GetRoomID() const
{
	return m_pInternalLogic->GetRoomID();
//	return m_pInternalLogic->GetRoomID();
}

/*
RoomID& CRoom::GetRoomID()
{
	return m_pInternalLogic->GetRoomID();
}
*/

//BOOL CRoom::Run(const RoomID& roomID, DWORD dwAddr, DWORD dwTypeID, const RoomOption& roomOption)
BOOL CRoom::Run(const RoomID & roomID,
				const LRBAddress& addr,
				DWORD dwTypeID, const NFRoomOption& nfRoomOption, const string& sGameOption)
{
	TLock lo(this);

	//m_addr = addr;
	m_pInternalLogic->SetRoomAddr( addr );
	m_dwTypeID = dwTypeID;


	BOOL bRet = TRUE;
	bRet = bRet && m_timerAlive.Activate(GetThreadPool(), this, USER_ALIVE_INTERVAL, USER_ALIVE_INTERVAL);

// NF
	bRet = bRet && m_timerGNFGame.Activate(GetThreadPool(), this, FISH_AI_INTERVAL, FISH_AI_INTERVAL);
	if(!bRet)
	{
		Stop();
		return FALSE;
	}

	// BanishUser들을 KickOut 하기 위한 Timer
	bRet = bRet && m_timerBanishUser.Activate(GetThreadPool(), this, 2000, 2000);
	if(!bRet)
	{
		Stop();
		return FALSE;
	}

	if (!(m_pInternalLogic->OnRun(roomID, nfRoomOption, sGameOption, this)))
	{
		Stop();
		theLog.Put( ERR_UK, "CRoom::Run, OnRun() Failed!!" );
		return FALSE;
	}
// NF	

	SetRunStopFlag(ROOMSTATE_RUN);
	return TRUE;
}

BOOL CRoom::Stop()
{
	TLock lo(this);

	m_pInternalLogic->OnStop();
	RemoveAllTimer();

	// now final clean up
	return TRUE;
}

BOOL CRoom::ProcessDisconnectedUser( long lCSN )
{
	
	if( m_pInternalLogic->IsExistingUser( lCSN ) )
	{
		theLog.Put(DEV_UK, "ProcessDisconnectedUser : USN = ", lCSN );
	}
	m_pInternalLogic->ProcessExitedUser( lCSN, FALSE );
	return true;
}


// TIMER
BOOL CRoom::ProcessCreateRoomReq( CUser * pUser )
{
	m_pInternalLogic->SetCHSAddr( pUser->GetCHSAddr() );
	if ( m_pInternalLogic->GetCHSAddr().GetCastType() == CASTTYPE_INVALID)
	{
		theLog.Put(WAR_UK, "NGS_CreateRoom_Error"_COMMA, "CHS Address is Invalid in OnSignal. USN: ", pUser->GetUSN(), ", Room ID: ", m_pInternalLogic->GetRoomIDStr() );
	}
	if(!m_pInternalLogic->OnCreateRoomReq(pUser))
	{
		::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_CREATEANS, (LPARAM)pUser);
	}

	return true;
}


BOOL CRoom::ProcessJoinRoomReq( CUser * pUser )
{
	const LONG lCSN = pUser->GetCSN();

	if( TRUE == m_pInternalLogic->IsExistPrevUser( lCSN ) )
	{
		
		m_pInternalLogic->KickOutUser( pUser->GetCSN(), TRUE );
	}

	if ( m_pInternalLogic->IsOverMaxUserCnt() ) 
	{
		pUser->SetErrorCode(JRF_USERMAX);
		return false;
	}

//	if ( m_pIRC->IsBlackListUser(lCSN) == TRUE)
//	{
//		pUser->SetErrorCode(JRF_NOACCESS);
//		return false;
//	}

	if( FALSE == m_pInternalLogic->PasswordCheckOnJoin( pUser ) )
	{
	
		//User matched lCSN is not in room, and has wrong password. 
		theLog.Put(INF_UK, "NGS_AuthFail, room_password[",m_pInternalLogic->GetRoomPassword(),"], user password[",pUser->GetPassword(),"], CSN:", pUser->GetCSN());
		pUser->SetErrorCode(JRF_WRONGPASSWORD);
		return false;
	}

	m_pInternalLogic->OnJoinRoomReqSuccess( pUser );

	return true;
}

void CRoom::ProcessAccuseSignal( const HSIGNAL &hObj, const WPARAM &wParam, const LPARAM &lParam )
{
	CAccuseResultAnswer dataARAns;
	if(lParam == 0)
	{
		if(!theAccuseAgent.m_AccuseResultAns.Get((long)wParam, dataARAns))
		{
			theLog.Put(WAR_UK, "NGS_Accusing_Error"_COMMA, "theAccuseAgent.m_AccuseResultAns.Get() Error: 0 in OnSignal");
			return;
		}
	}
	else if(lParam == 1)
	{
		if(!theHTTPAgent.m_AccuseResultAns.Get((long)wParam, dataARAns))
		{
			theLog.Put(WAR_UK, "NGS_Accusing_Error"_COMMA, "theAccuseAgent.m_AccuseResultAns.Get() Error: 1 in OnSignal");
			return;
		}
	}
	else
	{
		theLog.Put(WAR_UK, "NGS_Accusing_Error"_COMMA, "theAccuseAgent.m_AccuseResultAns.Get() Error: -1 in OnSignal");
		return;
	}

	m_pInternalLogic->SendAccuseAnsToUser( wParam, dataARAns );


	// 요부분에 lock 을 잡아야 될 듯. 
	// 신고관련 snowyunee 님이 작업할 때 kimsk님이 어떤 부분 터져서 뭔가를 못했다 그랬었는데 이곳과 관련있는지 재검토 필요. 2007.03.19 kts123
//	CUser* pUser = FindUser(wParam);
}

void CRoom::OnKickOutBanashUser()
{
	ForEachElmt(TMapBanishUser, m_mapBanishUser, it, ij)
	{
		DWORD dwCurTime = GetTickCount();
		if (dwCurTime - (DWORD)((*it).second >= 2000))
		{
			m_pInternalLogic->KickOutUser( (*it).first );
			m_mapBanishUser.erase((*it).first);
		}
	}
}

void CRoom::ProccssCutoutUser( LONG usn, LinkCutReason reason )
{
	string sUserInfo;
	m_pInternalLogic->OnCutOutUser( sUserInfo, usn, (LinkCutReason)reason );

	if( LCS_LINKCUT_KICKOUT == reason)
	{
		// NF
		// NLS가 끊으라고 하는거면, 끊기전에 알려주기 위해서 Message를 보낸다.
		MsgNGSCli_ReqBanishUser		req;
		PayloadNGSCli	pld(PayloadNGSCli::msgReqBanishUser_Tag, req);
		m_pInternalLogic->UserManager->SendToUser( usn, pld );
	}

	// 2초후에 끊기 위해서 Timer에 등록한다.
	m_mapBanishUser[usn] = GetTickCount();
}

void CRoom::ProcessNFFriendList(const MsgNLSCLI_AnsLocation& msg)
{
	CUser* pUser = m_pInternalLogic->FindUser(msg.m_lCSN);
	if (!pUser)
	{
		return;
	}

	CONT_NF_FRIEND kContNFFriend = pUser->GetNFFriendInfo();

	// NLS에서 받아온 유저들의 위치
	ArcVectorT< NLSBaseInfo >::const_iterator nls_iter = msg.m_kContNLSBaseInfo.begin();
	while( nls_iter != msg.m_kContNLSBaseInfo.end() )
	{
		// 친구의 정보 업데이트
		CONT_NF_FRIEND::iterator find_iter = kContNFFriend.find( nls_iter->m_Key );
		if( kContNFFriend.end() != find_iter )
		{
			CONT_NF_FRIEND::mapped_type& element = find_iter->second;
			element.m_bIsOnline = TRUE;
			element.m_roomID = nls_iter->m_roomID;
			element.m_lLevel = nls_iter->m_lLevel;
			element.m_lStatus = nls_iter->m_lStatus;
		}

		++nls_iter;
	}

	// 클라이언트에서는 다른 유저의 Key값을 들고 있으면 안되서
	// Key가 없는 vector로 보내준다.
	MsgNGSCli_AnsNFFriendInfo ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	CONT_NF_FRIEND::iterator iter = kContNFFriend.begin();
	while( iter != kContNFFriend.end() )
	{
		ans.m_kContFriendInfo.push_back(iter->second);
		++iter;
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendInfo_Tag, ans);
	SendToUser(msg.m_lCSN, pld);
}

// 친구요청 처리
void CRoom::ProcessNFFriendApplication(const MsgNLSCLI_AnsLocation& msg)
{
	MsgNGSCli_AnsNFFriendAdd ans;	// 신청자에게
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;

	// 신청자
	CUser* pApplicant = m_pInternalLogic->FindUser( msg.m_lCSN );
	if( !pApplicant )
	{
		// 신청자가 없을 수가 없는데..
		return;
	}

	std::string strApplicantName = pApplicant->GetCharName();

	ArcVectorT< NLSBaseInfo >::const_iterator acceptor_iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != acceptor_iter )
	{
		// 수락자의 위치
		switch( acceptor_iter->m_lStatus )
		{
		case NLSCLISTATUS_NFCHARLOBBY://NCS
			{
				// NCS서버로 전달
				MsgNGSNCS_NtfNFFriendAdd ntf;
				ntf.m_lReceiverCSN = acceptor_iter->m_Key.m_lSubKey;
				ntf.m_strSender = strApplicantName;
				PayloadNGSNCS pld(PayloadNGSNCS::msgNtfNFFriendAdd_Tag, ntf);
				theLrbManager.SendToNCS( theRoomTable.GetAddr(), acceptor_iter->m_serverLRBAddr, pld );
			}break;
		case NLSCLISTATUS_NFCHANNELSERVER://CHS
			{
				// CHS서버로 전달
				MsgNGSCHS_NtfNFFriendAdd ntf;
				ntf.m_lReceiverCSN = acceptor_iter->m_Key.m_lSubKey;
				ntf.m_channelID = acceptor_iter->m_roomID;
				ntf.m_strSender = strApplicantName;
				PayloadNGSCHS pld(PayloadNGSCHS::msgNtfNFFriendAdd_Tag, ntf);				
				theLrbManager.SendToCHS( theRoomTable.GetAddr(), acceptor_iter->m_serverLRBAddr, pld );
			}break;
		case NLSCLISTATUS_NFGAMESERVER://NGS
			{
				// 수락자의 방
				CRoomPtr spRoom;
				BOOL bRet = theRoomTable.FindRoom( acceptor_iter->m_roomID, &spRoom );
				if( bRet )
				{
					// 수락자
					CUser* pAcceptor = spRoom->m_pInternalLogic->FindUser( acceptor_iter->m_Key.m_lSubKey );
					if( !pAcceptor )
					{
						ans.m_lErrorCode = NF::EC_FE_NOT_FOUND_CHARACTER;
						break;
					}

					// 수락자에게 친구요청이 왔음을 알려준다.
					MsgNGSCli_NtfNFFriendAdd ntf;
					ntf.m_strCharName = strApplicantName;
					PayloadNGSCli pld(PayloadNGSCli::msgNtfNFFriendAdd_Tag, ntf);
					SendToUser( pAcceptor->GetCSN(), pld );	
				}
			}break;
		default:
			{
				ans.m_lErrorCode = NF::EC_FE_NOT_FOUND_CHARACTER;
			}break;
		}
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAdd_Tag, ans);
	SendToUser(pApplicant->GetCSN(), pld);
}

void CRoom::ProcessFollowUser(const MsgNLSCLI_AnsLocation& msg)
{
	CUser* pUser = m_pInternalLogic->FindUser( msg.m_lCSN );
	if( !pUser )
	{
		return;
	}

	MsgNGSCli_AnsFollowUser ans;
	ans.Clear();

	ArcVectorT< NLSBaseInfo >::const_iterator iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != iter )
	{
		switch( iter->m_lStatus )
		{
		case NLSCLISTATUS_NFCHARLOBBY://NCS
			{
				ans.m_lServerType = NLSCLISTATUS_NFCHARLOBBY;
				ans.m_addr = iter->m_nsap;
			}break;
		case NLSCLISTATUS_NFCHANNELSERVER://CHS
			{
				ans.m_lServerType = NLSCLISTATUS_NFCHANNELSERVER;
				ans.m_roomID = iter->m_roomID;
				ans.m_lGameMode = iter->m_lGameMode;
				ans.m_addr = iter->m_nsap;
			}break;
		case NLSCLISTATUS_NFGAMESERVER://NGS
			{
				ans.m_lServerType = NLSCLISTATUS_NFGAMESERVER;
				ans.m_roomID = iter->m_roomID;
				ans.m_lGameMode = iter->m_lGameMode;
				ans.m_addr = iter->m_nsap;
			}break;
		}
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAnsFollowUser_Tag, ans);
	SendToUser(pUser->GetCSN(), pld);
}

void CRoom::ProcessNFLetterReceive(const MsgNLSCLI_AnsLocation& msg)
{
	ArcVectorT< NLSBaseInfo >::const_iterator iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != iter )
	{
		switch( iter->m_lStatus )
		{
		case NLSCLISTATUS_NFCHARLOBBY://NCS
			{	
				MsgNGSNCS_NtfNFLetterReceive ntf;
				ntf.m_lReceiverCSN = iter->m_Key.m_lSubKey;
				PayloadNGSNCS pld(PayloadNGSNCS::msgNtfNFLetterReceive_Tag, ntf);
				theLrbManager.SendToNCS( theRoomTable.GetAddr(), iter->m_serverLRBAddr, pld );
			}break;
		case NLSCLISTATUS_NFCHANNELSERVER://CHS
			{
				MsgNGSCHS_NtfNFLetterReceive ntf;
				ntf.m_channelID = iter->m_roomID;
				ntf.m_lReceiverCSN = iter->m_Key.m_lSubKey;
				PayloadNGSCHS pld(PayloadNGSCHS::msgNtfNFLetterReceive_Tag, ntf);
				theLrbManager.SendToCHS( theRoomTable.GetAddr(), iter->m_serverLRBAddr, pld );
			}break;
		}
	}
}

void CRoom::ProcessNFFriendAutoAccept(const MsgNLSCLI_AnsLocation& msg)
{
	// !!신청자
	CUser* pApplicant = m_pInternalLogic->FindUser( msg.m_lCSN );
	if( !pApplicant )
	{
		return;
	}

	// 신청자의 친구 리스트
	CONT_NF_FRIEND kContApplicantFriend = pApplicant->GetNFFriendInfo();

	// 수락자 정보
	CNFFriend nfFriendAcceptor;
	nfFriendAcceptor.Clear();
	nfFriendAcceptor.m_strCharName = msg.m_strCharName;

	ArcVectorT< NLSBaseInfo >::const_iterator acceptor_iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != acceptor_iter )
	{
		// 신청자의 친구정보 중에서 수락자의 레벨, 위치를 업데이트
		CONT_NF_FRIEND::iterator applicant_friend_iter = kContApplicantFriend.find( acceptor_iter->m_Key );
		if( applicant_friend_iter != kContApplicantFriend.end() )
		{
			CONT_NF_FRIEND::mapped_type& element = applicant_friend_iter->second;
			element.m_lLevel = acceptor_iter->m_lLevel;
			element.m_roomID = acceptor_iter->m_roomID;
			element.m_bIsOnline = TRUE;
			element.m_lStatus = acceptor_iter->m_lStatus;
			nfFriendAcceptor = element;
		}
	}

	// 자동 수락된것을 알린다.
	MsgNGSCli_NtfNFFriendAccept ntf;
	ntf.m_nfFriend = nfFriendAcceptor;
	PayloadNGSCli pld(PayloadNGSCli::msgNtfNFFriendAccept_Tag, ntf);
	SendToUser(pApplicant->GetCSN(), pld);
}

// 친구요청 수락 후 처리(이미 DB에 추가됐음)
void CRoom::ProcessNFFriendAccept(const MsgNLSCLI_AnsLocation& msg)
{
	// 수락자
	CUser* pAcceptor = m_pInternalLogic->FindUser( msg.m_lCSN );
	if( !pAcceptor )
	{
		return;
	}

	// 수락자 정보
	RoomID roomID;
	pAcceptor->NLSGetRoomID(roomID);
	CNFFriend nfFriendAcceptor;
	nfFriendAcceptor.m_bIsOnline = TRUE;
	nfFriendAcceptor.m_lLevel = pAcceptor->GetNFUser().GetNFChar().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel;
	nfFriendAcceptor.m_strCharName = pAcceptor->GetCharName();
	nfFriendAcceptor.m_roomID = roomID;
	nfFriendAcceptor.m_lStatus = NLSCLISTATUS_NFGAMESERVER;

	// 수락자의 친구 리스트
	CONT_NF_FRIEND kContAcceptorFriend = pAcceptor->GetNFFriendInfo();

	// 신청자 정보
	CNFFriend nfFriendApplicant;
	nfFriendApplicant.Clear();
	nfFriendApplicant.m_strCharName = msg.m_strCharName;

	ArcVectorT< NLSBaseInfo >::const_iterator applicant_iter = msg.m_kContNLSBaseInfo.begin();
	if( msg.m_kContNLSBaseInfo.end() != applicant_iter )
	{
		// 수락자의 친구정보 중에서 신청자의 레벨, 위치를 업데이트
		CONT_NF_FRIEND::iterator acceptor_friend_iter = kContAcceptorFriend.find( applicant_iter->m_Key );
		if( acceptor_friend_iter != kContAcceptorFriend.end() )
		{
			CONT_NF_FRIEND::mapped_type& element = acceptor_friend_iter->second;
			element.m_lLevel = applicant_iter->m_lLevel;
			element.m_roomID = applicant_iter->m_roomID;
			element.m_bIsOnline = TRUE;
			element.m_lStatus = applicant_iter->m_lStatus;
			nfFriendApplicant = element;
		}

		// 신청자의 위치
		switch( applicant_iter->m_lStatus )
		{
		case NLSCLISTATUS_NFCHARLOBBY://NCS
			{
				// NCS로 전달
				MsgNGSNCS_NtfNFFriendAccept ntf;
				ntf.m_lReceiverCSN = applicant_iter->m_Key.m_lSubKey;
				ntf.m_nfFriend = nfFriendAcceptor;

				PayloadNGSNCS pld(PayloadNGSNCS::msgNtfNFFriendAccept_Tag, ntf);
				theLrbManager.SendToNCS( theRoomTable.GetAddr(), applicant_iter->m_serverLRBAddr, pld );
			}break;
		case NLSCLISTATUS_NFCHANNELSERVER://CHS
			{
				// CHS로 전달
				MsgNGSCHS_NtfNFFriendAccept ntf;
				ntf.m_lReceiverCSN = applicant_iter->m_Key.m_lSubKey;
				ntf.m_channelID = applicant_iter->m_roomID;
				ntf.m_nfFriend = nfFriendAcceptor;

				PayloadNGSCHS pld(PayloadNGSCHS::msgNtfNFFriendAccept_Tag, ntf);
				theLrbManager.SendToCHS( theRoomTable.GetAddr(), applicant_iter->m_serverLRBAddr, pld );
			}break;
		case NLSCLISTATUS_NFGAMESERVER://NGS
			{
				// 신청자의 친구목록에 수락자의 정보를 더하고,
				// 신청자에게 수락자의 정보를 보내준다.
				CUser* pApplicant = m_pInternalLogic->FindUser( applicant_iter->m_Key.m_lSubKey );
				if( pApplicant )
				{
					pApplicant->AddNFFriend( TKey( msg.m_lUSN, msg.m_lCSN ), nfFriendAcceptor );

					MsgNGSCli_NtfNFFriendAccept ntf;
					ntf.m_nfFriend = nfFriendAcceptor;
					PayloadNGSCli pld(PayloadNGSCli::msgNtfNFFriendAccept_Tag, ntf);
					SendToUser( pApplicant->GetCSN(), pld );
				}
			}break;
		default:
			{
			}break;
		}
	}

	// 수락자에게 신청자의 정보를 보내준다.
	MsgNGSCli_AnsNFFriendAccept ans;
	ans.m_lErrorCode = NF::EC_FE_SUCCESS;
	ans.m_nfFriend = nfFriendApplicant;
	PayloadNGSCli pld(PayloadNGSCli::msgAnsNFFriendAccept_Tag, ans);
	SendToUser(pAcceptor->GetCSN(), pld);
}

STDMETHODIMP_(void) CRoom::OnSignal(HSIGNAL hObj, WPARAM wParam, LPARAM lParam)
{

	
	if(hObj == ROOMSIGNAL_NLSANSWER)	// LCS Manager에서 signaling
	{		
		TLock lo(this);
		CUser* pUser = (CUser*)lParam;
		RoomCreateAndJoin(pUser, (LONG)wParam);		
	}

	else if ( m_timerBanishUser.IsHandle( hObj ) )
	{
		TLock lo(this);
		OnKickOutBanashUser();
	}
	
	else if (hObj == (HSIGNAL)ROOM_USERLINKCUT)
	{
		TLock lo(this);
		ProccssCutoutUser( wParam, (LinkCutReason)lParam );
	}
	else if (hObj == ROOMSIGNAL_PROCESS_NF_FRIEND_LIST)
	{
		TLock lo(this);
		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wParam))
			return;
		ProcessNFFriendList(msg);
	}
	else if ( hObj == ROOMSIGNAL_PROCESS_NF_FRIEND_APPLICATION)
	{
		TLock lo(this);
		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wParam))
			return;
		ProcessNFFriendApplication(msg);
	}
	else if ( hObj == ROOMSIGNAL_PROCESS_NF_FRIEND_ACCEPT)
	{
		TLock lo(this);
		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wParam))
			return;
		ProcessNFFriendAccept(msg);
	}
	else if ( hObj == ROOMSIGNAL_PROCESS_NF_FRIEND_AUTO_ACCEPT )
	{
		TLock lo(this);
		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wParam))
			return;
		ProcessNFFriendAutoAccept(msg);
	}
	else if( hObj == ROOMSIGNAL_PROCESS_NF_LETTER_NEW )
	{
		TLock lo(this);
		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wParam))
			return;
		ProcessNFLetterReceive(msg);
	}
	else if( hObj == ROOMSIGNAL_PROCESS_FOLLOW_USER )
	{
		TLock lo(this);
		MsgNLSCLI_AnsLocation msg;
		if(!::LLoad(msg, (LPXBUF)wParam))
			return;
		ProcessFollowUser(msg);
	}

	if(hObj == 0)
	{
		if(wParam == ROOMMSG_CREATEREQ)
		{
			TLock lo(this);
			ProcessCreateRoomReq( (CUser *)lParam );
			//TBase::OnSignal(hObj, wParam, lParam);
		}
		else if(wParam == ROOMMSG_JOINREQ)
		{
			TLock lo(this);
			CUser* pUser = (CUser*)lParam;
			//if(!m_pInternalLogic->OnJoinRoomReq(pUser))
			if(!this->ProcessJoinRoomReq( pUser ) )
			{
				::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_JOINANS, (LPARAM)pUser);
			}
			//TBase::OnSignal(hObj, wParam, lParam);
		}
		else
		{
			TLock lo(this);
			//TBase::OnSignal(hObj, wParam, lParam);
		}

		return;
	}
#ifdef _NGS_ACCUSE
	else if(hObj == ACCUSESIGNAL_AGENTANSWER)
	{
		ProcessAccuseSignal( hObj, wParam, lParam );
	}
#endif
	else
	{
		DWORD beforeLockTick = ::GetTickCount(); // 타이머 디버그. 시그널이 터진 시간을 기록해야 하기 때문에 반드시 lock 잡기 전에 있어야 한다.
		TLock lo(this);
		
		ProcessTimerSignal( hObj, wParam, lParam, beforeLockTick );
		//TBase::OnSignal(hObj, wParam, lParam);
	}

}

void CRoom::RoomCreateAndJoin(CUser * pUser, LONG lMsgType)
{
	if (!pUser)
	{
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "pUser is NULL in RoomCreateAndJoin, Msg Type: ", lMsgType);
		return;
	}
	if (lMsgType == LCSMSGTYPE_CREATEROOM)	// message 구분.. create or join
	{
		LONG lErrorCode = pUser->GetErrorCode();
		// 이 응답을 받기 전에 Queck update mode, slow update mode로 메세지를 보낼건지 결정해야 한다. 
		// 게임 한판이 끝나기 전에 접속을 해제하지 않는 게임의 경우.. slow update mode로 진행해야 한다.
		// 원한다면 client에게 다른 접속이 있었음을 알려줄 수도 있다.
		if(lErrorCode == E_LCS_EXISTKEY || lErrorCode == E_LCS_EXISTVALUE)
		{
		// 이 Error code는 Slow Update Mode일 경우에만 수신된다. 이 경우 Quick 지우고자 할 경우 quick update로 다시 메세지를 전송해야 한다.
		// 이 부분에 대해서는 아직 client와 협의된 바가 없어, 모든 경우 Quick update 한다.
		}
		else if(lErrorCode == E_LCS_NOTEXIST || lErrorCode == E_LCS_NOTDEFINE)
		{
			pUser->SetErrorCode(CRF_DBERROR);	// 임시로 DB Error로 처리..
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_CREATEANS, (LPARAM)pUser);
			return;
		}
//// ACHV BEGIN
		if (!g_achv.login(pUser->GetGSN(), pUser->GetCSN()))
		{
			pUser->SetErrorCode(CRF_DBERROR);
			theLog.Put(ERR_UK, "NGS_Null"_COMMA, "achv:login() failed @ RoomCreateAndJoin: ", lMsgType);
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_CREATEANS, (LPARAM)pUser);
			return;
		}
//// ACHV END

		BOOL bRet = m_pInternalLogic->GetNFCharInfo(pUser);
		if (!bRet)
		{
			pUser->SetErrorCode(CRF_DBERROR);
			theLog.Put(WAR_UK, "NGS_Null"_COMMA, "pUser is NULL in RoomCreateAndJoin, GetNFCharInfo Failed: ", lMsgType);
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_CREATEANS, (LPARAM)pUser);
			return;
		}

		m_pInternalLogic->SetCHSAddr( pUser->GetCHSAddr() );
		if (m_pInternalLogic->GetCHSAddr().GetCastType() == CASTTYPE_INVALID)
		{
			theLog.Put(WAR_UK, "NGS_CreateRoom_Error"_COMMA, "CHS Address is Invalid, USN: ", pUser->GetUSN(), ", Room ID: ", m_pInternalLogic->GetRoomID().m_dwGRIID, " in RoomCreateAndJoin");
		}
		pUser->SetErrorCode(CRF_SUCCESS);
		if(!m_pInternalLogic->OnCreateRoomReq(pUser))
		{
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_CREATEANS, (LPARAM)pUser);
		}
	}
	else if(lMsgType == LCSMSGTYPE_JOINROOM)
	{
		LONG lErrorCode = pUser->GetErrorCode();
		// 이 응답을 받기 전에 Queck update mode, slow update mode로 메세지를 보낼건지 결정해야 한다. 
		// 게임 한판이 끝나기 전에 접속을 해제하지 않는 게임의 경우.. slow update mode로 진행해야 한다.
		// 원한다면 client에게 다른 접속이 있었음을 알려줄 수도 있다.
		if(lErrorCode == E_LCS_EXISTKEY || lErrorCode == E_LCS_EXISTVALUE)
		{
		// 이 Error code는 Slow Update Mode일 경우에만 수신된다. 이 경우 Quick 지우고자 할 경우 quick update로 다시 메세지를 전송해야 한다.
		// 이 부분에 대해서는 아직 client와 협의된 바가 없어, 모든 경우 Quick update 한다.
		}
		else if(lErrorCode == E_LCS_NOTEXIST || lErrorCode == E_LCS_NOTDEFINE)
		{
			pUser->SetErrorCode(JRF_DBERROR);	
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_JOINANS, (LPARAM)pUser);
			return;
		}
// NF
		//// ACHV BEGIN
		if (!g_achv.login(pUser->GetGSN(), pUser->GetCSN()))
		{
			pUser->SetErrorCode(CRF_DBERROR);
			theLog.Put(ERR_UK, "NGS_Null"_COMMA, "achv::login failed @ RoomCreateAndJoin: ", lMsgType);
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_JOINANS, (LPARAM)pUser);
			return;
		}
		//// ACHV END

		BOOL bRet = m_pInternalLogic->GetNFCharInfo(pUser);
		if (!bRet)
		{
			pUser->SetErrorCode(JRF_DBERROR);
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_JOINANS, (LPARAM)pUser);
			return;
		}

// NF
		if (GetRunStopFlag() == ROOMSTATE_STOP)
		{
			theLog.Put(WAR_UK, "NGS_JoinRoom_Error"_COMMA, "Trying to join dead room in RoomCreateAndJoin");
			pUser->SetErrorCode(JRF_INVALIDSTATE);
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_JOINANS, (LPARAM)pUser);
			return;
		}

		pUser->SetErrorCode(JRF_SUCCESS);
		if(!ProcessJoinRoomReq( pUser ) )
		{
			::XsigQueueSignal(GetListenerThreadPool(), &theListener, 0, LISTENERMSG_JOINANS, (LPARAM)pUser);
		}
	}
}


BOOL CRoom::ProcessRoomEvent(const RoomEvent& e)
{
	TLock lo( this );

		
	switch(e.m_nTag)
	{
	case REV_USERLINKERROR:
		theLog.Put(DEV_UK, "NGS_DevInfo"_COMMA, "NGS_LINKERROR. Disconnected User CSN : ", e.m_lCSN );
		/*현재 사용자가 방에서 정상적으로 나가는 경우에도 별다른 작업 없이 link를 끊기 때문에
		 이 경우는 사용자가 정말로 연결을 끊었던가, 아니면 정상적으로 나갔던가 둘 중 하나이다.
		 GLS는 둘을 구분할 필요가 없으며, IRC에서 필요한 처리는 IRC가 알아서 한다.*/
		ProcessDisconnectedUser( e.m_lCSN );
		break;
	case REV_USERCUT:
		theLog.Put(INF_UK, "NGS_DevInfo"_COMMA, "NGS_LINKCUT. usn of cut user CSN : ", e.m_lCSN );
		ProcessDisconnectedUser( e.m_lCSN );
		//m_pInternalLogic->OnUserDisconnect(e.m_lCSN);
		//theLog.Put(DEV_UK, "NGS_DevInfo"_COMMA, "RoomEvent.m_nTag == REV_USERDISCONNECT");
		break;

	case REV_USERMSG:
		OnUserMsg(e.m_lCSN);
		break;
	case REV_LRBMSG:
		break;
	case REV_ANNOUNCE:
		m_pInternalLogic->SendAnnounceMsg();
		break;
	case REV_JACKPOTNTF:
		break;
	default:
//		FATAL();
		break;
	}

	return true;
}

void CRoom::OnUserMsg(LONG lCSN)
{

	PayloadCliNGS pld;
	if(! m_pInternalLogic->GetNextUserMsg( pld, lCSN ) ) return;

	ProcessUserMsg(lCSN, &pld); 
}

void CRoom::ProcessUserMsg(LONG lCSN, PayloadCliNGS* pPld)
{
	if (!pPld) return;

	m_pInternalLogic->OnRcvAlive( lCSN );
	
	switch (pPld->mTagID)
	{
	case PayloadCliNGS::msgChatMsgTo_Tag:
		m_pInternalLogic->OnRcvChatMsgTo(lCSN, pPld->un.m_msgChatMsgTo);
		break;
	case PayloadCliNGS::msgChatMsgAll_Tag:
		m_pInternalLogic->OnRcvChatMsgAll(lCSN, pPld->un.m_msgChatMsgAll);
		break;
	case PayloadCliNGS::msgIsAlive_Tag:
		break;
	case PayloadCliNGS::msgChangeRoomOptionNtf_Tag:
		OnRcvChangeRoomOptionNtf(lCSN, pPld->un.m_msgChangeRoomOptionNtf);
		break;
	case PayloadCliNGS::msgCreateRoomNtf_Tag:
		OnRcvCreateRoomNtf(lCSN, pPld);
		break;
	case PayloadCliNGS::msgJoinRoomNtf_Tag:
		OnRcvJoinRoomNtf(lCSN, pPld);
		break;
	case PayloadCliNGS::msgLeaveRoomReq_Tag:
		OnRcvLeaveRoomReq(lCSN, pPld);
		break;
		// 신고
	case PayloadCliNGS::msgAccuseReq_Tag:
		m_pInternalLogic->OnRcvAccuseReq(lCSN, pPld->un.m_msgAccuseReq);
		break;
		// 세이클럽 접속 끊어진 경우
	case PayloadCliNGS::msgLoginStateChangeNtf_Tag:
		m_pInternalLogic->OnRcvLoginStateChangeNtf(lCSN, pPld->un.m_msgLoginStateChangeNtf);
		break;
//	case PayloadCliNGS::msgGetUserRankInfoReq_Tag:
//		m_pInternalLogic->OnRcvGetUserRankInfoReq(lCSN, pPld->un.m_msgGetUserRankInfoReq);
//		break;

// NF
	case PayloadCliNGS::msgReqTutorial_Tag:
		m_pInternalLogic->OnReqTutorial(lCSN, pPld);
		break;
	case PayloadCliNGS::msgBanishReq_Tag:
		m_pInternalLogic->OnRcvBanishReq(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFCharInfo_Tag:
		m_pInternalLogic->OnReqNFCharInfo(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqBiteFishInfo_Tag:
		m_pInternalLogic->OnReqBiteFishInfo(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqBuyItem_Tag:
		m_pInternalLogic->OnReqBuyItem(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqRemoveItem_Tag:
		m_pInternalLogic->OnReqRemoveItem(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqOpenCardPack_Tag:
		m_pInternalLogic->OnReqOpenCardPack(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqChangeParts_Tag:
		m_pInternalLogic->OnReqChangeParts(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqChangeCardSlot_Tag:
		m_pInternalLogic->OnReqChangeCardSlot(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqExchangeCards_Tag:
		m_pInternalLogic->OnReqExchangeCards(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqChangeQuickSlot_Tag:
		m_pInternalLogic->OnReqChangeQuickSlot(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqGetRewardItem_Tag:
		m_pInternalLogic->OnReqGetRewardItem(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqChangeUserInfo_Tag:
		m_pInternalLogic->OnReqChangeUserInfo(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqMoveFishingPoint_Tag:
		m_pInternalLogic->OnReqMoveFishingPoint(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqTotalMapInfo_Tag:
		m_pInternalLogic->OnReqTotalMapInfo(lCSN, pPld);
		break;
	case PayloadCliNGS::msgNFGameData_Tag:
		m_pInternalLogic->OnNFGameData(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqGameResult_Tag:
		m_pInternalLogic->OnReqGameResult(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqStartGame_Tag:
		m_pInternalLogic->OnReqStartGame(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqJoinInGame_Tag:
		m_pInternalLogic->OnReqJoinInGame(lCSN, pPld);
		break;
	case PayloadCliNGS::msgNtfGameReadyProgress_Tag:
		m_pInternalLogic->OnNtfGameReadyProgress(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqCasting_Tag:
		m_pInternalLogic->OnReqCasting(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqCastingResult_Tag:
		m_pInternalLogic->OnReqCastingResult(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqAction_Tag:
		m_pInternalLogic->OnReqAction(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqHookingResult_Tag:
		m_pInternalLogic->OnReqHookingResult(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqFighting_Tag:
		m_pInternalLogic->OnReqFighting(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqLanding_Tag:
		m_pInternalLogic->OnReqLanding(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqUseItem_Tag:
		m_pInternalLogic->OnReqUseItem(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqGameOver_Tag:
		m_pInternalLogic->OnReqGameOver(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqCorrectDirection_Tag:
		m_pInternalLogic->OnReqCorrectDirection(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqAchvInfo_Tag:
		m_pInternalLogic->OnReqAchievement(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqUpdateAchvInfo_Tag:
		m_pInternalLogic->OnReqUpdateAchieveInfo(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqFreeRoomList_Tag:
		m_pInternalLogic->OnReqFreeRoomList(lCSN, pPld);
		break;
	case PayloadCliNGS::msgCheatReq_Tag:
		m_pInternalLogic->OnCheatReq(lCSN, pPld);
		break;		
	case PayloadCliNGS::msgReqNFLetterList_Tag:		
		m_pInternalLogic->OnReqNFLetterList(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFLetterContent_Tag:
		m_pInternalLogic->OnReqNFLetterContent(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFLetterReceiverCheck_Tag:
		m_pInternalLogic->OnReqNFLetterReceiverCheck(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFLetterSend_Tag:
		m_pInternalLogic->OnReqNFLetterSend(lCSN, pPld);
	case PayloadCliNGS::msgReqNFLetterDelete_Tag:
		m_pInternalLogic->OnReqNFLetterDelete(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFFriendInfo_Tag:
		m_pInternalLogic->OnReqNFFriendInfo(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFFriendAdd_Tag:
		m_pInternalLogic->OnReqNFFriendAdd(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFFriendAccept_Tag:
		m_pInternalLogic->OnReqNFFriendAccept(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFFriendReject_Tag:
		m_pInternalLogic->OnReqNFFriendReject(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFFriendDelete_Tag:
		m_pInternalLogic->OnReqNFFriendDelete(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFBlockList_Tag:
		m_pInternalLogic->OnReqNFBlockList(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFFriendApplicantList_Tag:
		m_pInternalLogic->OnReqNFFriendApplicantList(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNFBlockOrUnBlock_Tag:
		m_pInternalLogic->OnReqNFBlockOrUnBlock(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqFollowUser_Tag:
		m_pInternalLogic->OnReqFollowUser(lCSN, pPld);
		break;
		//LandNote
	case PayloadCliNGS::msgReqLockedNote_Tag:
		m_pInternalLogic->OnReqLandNote(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqUpdateStudyScenario_Tag:
		m_pInternalLogic->OnReqUpdateStudyScenario(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqNextEnchantInfo_Tag:
		m_pInternalLogic->OnReqNextEnchantInfo(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqItemEnchant_Tag:
		m_pInternalLogic->OnReqItemEnchant(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqAquaInsert_Tag:
		m_pInternalLogic->OnReqAquaInsert(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqAquaFish_Tag:
		m_pInternalLogic->OnReqAquaFish(lCSN, pPld);
		break;
	case PayloadCliNGS::msgReqProductList_Tag:
		m_pInternalLogic->OnReqProductList(lCSN, pPld);
		break;
// NF

	default:
		{
			theLog.Put(DEV, "CRoomInternalLogic::OnRcvMsg() - Unknown message(Tag:", pPld->mTagID, ")");
			theLog.Put(INF_UK, "NGS_Null,pPld->mTagID is default in CRoomInternalLogic::OnRcvMsg()  RoomID:", m_pInternalLogic->GetRoomIDStr());
			m_pInternalLogic->KickOutUser( lCSN );

			return;
		}
		break;
	}
}

void CRoom::OnRcvChangeRoomOptionNtf( LONG usn, MsgCliNGS_ChangeRoomOptionNtf* pMsg)
{
	if (!pMsg) return;

	m_pInternalLogic->OnRcvChangeRoomOptionNtf( usn, pMsg );
}

void CRoom::OnRcvCreateRoomNtf( LONG usn , PayloadCliNGS* pPld)
{
	if (!pPld) return;

	if( FALSE == m_pInternalLogic->CheckPldCreateRoomNtf( usn, pPld ) )
		return;

	m_pInternalLogic->OnCreateRoomNtfSuccess( usn );	
}

void CRoom::OnRcvJoinRoomNtf( LONG usn , PayloadCliNGS* pPld)
{
	if (!pPld) return;

	if( FALSE == m_pInternalLogic->CheckPldJoinRoomNtf( usn, pPld ) )
		return;
	
	m_pInternalLogic->OnJoinRoomNtfSuccess( usn );
	m_pInternalLogic->SendJoinNtfResultToAll( usn );
}

void CRoom::OnRcvLeaveRoomReq( LONG usn , PayloadCliNGS* pPld)
{
	if (!pPld) return;

	//	MsgCliNGS_LeaveRoomReq* pMsg = pPld->un.m_msgLeaveRoomReq;
	//GRC로 사용자 삭제요구.. CHS로 MsgGLSCHS_UserRoomLeaveNtf까지..
	theLog.Put(INF_UK, "NGS_Unused_Msg"_COMMA, "OnRcvLeaveRoomReq won't be called! USN: ", usn);

	//	//사용자 삭제 및 링크 삭제..
	//	OnUserDisconnect(pUser, TRUE);
}

void CRoom::OnRcvAnsFreeRoomList(LONG csn, MsgCHSNGS_AnsFreeRoomList* pMsg)
{
	if (!pMsg) return;

	m_pInternalLogic->OnAnsFreeRoomList(csn, pMsg);
}

LONG CRoom::GetAchvUserInfo(LONG lCSN, string& strNickName)
{ 
	CUser* pUser = m_pInternalLogic->FindUser(lCSN);
	if (pUser) {
		strNickName = pUser->GetCharName();
		return pUser->GetGSN();
	}
	return -1;
};

void CRoom::SendToUser(LONG lCSN, const PayloadNGSCli& pld)
{
	m_pInternalLogic->SendToUser(lCSN, pld);
}

void CRoom::OnAnnounceMsg(const xstring& sAnnounce)
{
	TLock lo(this);
	GBuf buf((LPVOID)sAnnounce.c_str(), sAnnounce.length()*sizeof(xchar));
	m_pInternalLogic->mAnnounceMsg.Push(buf);
	m_pInternalLogic->OnAnnounceMsg();
}
/*
void CRoom::OnAnnounceMsg(const xstring& sAnnounce)
{
	TLock lo(this);
	GBuf buf((LPVOID)sAnnounce.c_str(), sAnnounce.length()*sizeof(xchar));
	m_pInternalLogic->mAnnounceMsg.Push(buf);
	PostAnnounceMsg();
}

void CRoom::PostUserDisconnect(LONG lCSN)
{
//	m_queRoomEvent.push( RoomEvent(REV_USERDISCONNECT, pUser) );
	RoomEvent e(REV_USERDISCONNECT, lCSN);
	PushQueue(e);
}

void CRoom::PostUserMsg(LONG lCSN)
{
//	m_queRoomEvent.push( RoomEvent(REV_USERMSG, pUser) );
	RoomEvent e(REV_USERMSG, lCSN);
	PushQueue(e);
}

void CRoom::PostLrbMsg()
{
//	m_queRoomEvent.push( RoomEvent(REV_CHSMSG, NULL) );
	RoomEvent e(REV_LRBMSG, NULL);
	PushQueue(e);
}

void CRoom::PostAnnounceMsg()
{
	RoomEvent e(REV_ANNOUNCE, NULL);
	PushQueue(e);
}
*/

LONG CRoom::RemoveAllTimer()
{
	m_pInternalLogic->RemoveAllTimer();

	m_timerAlive.Deactivate();
// NF
	m_timerGNFGame.Deactivate();
	m_timerBanishUser.Deactivate();
// NF
	return 0;
}

void OnTimerLogging()
{
	static long g_OnTimerCallNum = 0;
	static long g_OnTimerCallNumPrev = 0;
	static long  g_OnTimerLoggingTime = (long)time(NULL);
	static long  g_OnTimerLoggingStartTime = (long)time(NULL);

	if (g_OnTimerCallNum < 0)
	{
		g_OnTimerCallNum = 0;
		g_OnTimerCallNumPrev = 0;
	}

	long OnTimerCallNum = InterlockedIncrement(&g_OnTimerCallNum);

	long now = (long) time(NULL);	
	if (now < g_OnTimerLoggingTime + 100) // 로깅한지 100초 이내면 리턴
		return;

	long prev = ::InterlockedExchange(&g_OnTimerLoggingTime, now); // 새로운 값으로 설정
	if (now >= prev + 100)  
	{
		long DebugTimerFireCount1, DebugTimerFireCount2;
		::XsigGetDebugInfo(TIMER_FIRE_COUNT1, &DebugTimerFireCount1);
		::XsigGetDebugInfo(TIMER_FIRE_COUNT2, &DebugTimerFireCount2);
		theLog.Put(INF_UK, "NGS_RoomTimer_Inf, OnTimerCallNum Per ",now - prev," seconds :", OnTimerCallNum - g_OnTimerCallNumPrev, ",  g_OnTimerCallNum:", g_OnTimerCallNum," in time:",now - g_OnTimerLoggingStartTime);
		theLog.Put(INF_UK, "NGS_RoomTimer_Inf, DebugTimerFireCount1 :",DebugTimerFireCount1," , DebugTimerFireCount2 : ", DebugTimerFireCount2);
		g_OnTimerCallNumPrev = OnTimerCallNum;
	}
}

void CRoom::ProcessTimerSignal( const HSIGNAL &hObj, const WPARAM &wParam, const LPARAM &lParam, DWORD beforeLockTick )
{
	DWORD afterLock = ::GetTickCount(); 
	DWORD queueTime = afterLock - beforeLockTick;


	LONG TimerIndex = 0;
	BOOL IsIRCTimerSignal = false;
	if( TRUE == (IsIRCTimerSignal =m_pInternalLogic->ProcessTimerSignal( TimerIndex, hObj, wParam, lParam, queueTime ) ) )
	{
		DWORD before = ::GetTickCount();
		{
		}
		
		DWORD after  = ::GetTickCount();
		OnTimerLogging();
		{	
			// 타이머 디버그.					
			if (after >  before + 500)
				theLog.Put(WAR_UK, "NGS_RoomTimer_Error,OnSignal()\t Too long OnTimer(). call_time:", after - before , ",\tqueueTime:", queueTime,  ", RoomID:", m_pInternalLogic->GetRoomIDStr(), ", index:", TimerIndex, ", hObj:", hObj);
		}
	}
	if(m_timerAlive.IsHandle(hObj)) {
		m_pInternalLogic->OnCheckUserAliveTimer();
		OnTimerLogging();
	}
	// NF
	if(m_timerGNFGame.IsHandle(hObj)) {
		m_pInternalLogic->OnTimerGNFGame();
	}
	if (TimerIndex == TIMER_INDEX_SIGN)
		m_pInternalLogic->NtfEndSignMsg();
	// NF
}

void CRoom::MulticastNotify(GBuf& gBuf)
{	
	//이미 OnNGSMulticastNotify 에서 해당 SSN을 서비하는 룸에 대해서만 이 함수를 호출함.
	//따라서 Global Object 사용 안하는 룸에서 호출될 일 없음
	TLock lo(this);
}

BOOL CRoom::GetRoomInfoInChannel(NFRoomInfoInChannel &roomInfo)
{
	return m_pInternalLogic->GetRoomInfoInChannel(roomInfo);	
}


/*void CRoom::OnIBBAns(LONG lCSN, GBuf& gBuf)
{
	TLock lo(this);
	m_pInternalLogic->NotifyIBBAns(lCSN, gBuf);
}*/

/*
string CRoom::GetRoomIDStr()
{
	return RoomID2Str(m_pInternalLogic->GetRoomID());
}

string CRoom::GetRoomIDStr64()
{
	return ::RoomIDToStr64(m_pInternalLogic->GetRoomID());
}
*/
void CRoom::OnRKSAns(long lMsgType, DWORD dwRankType, GBuf &buf, DWORD dwTargetUSN)
{
	TLock lo(this);
}

void CRoom::OnIBBAns(long lCSN, string& sUserGameData, LONG lMsgType)
{
	TLock lo(this);

	theLog.Put(DEV_UK, "NGS_Room, OnIBBAns - CSN: ", lCSN, " MsgType: ", lMsgType );
	switch (lMsgType)
	{
	default:
		theLog.Put(WAR_UK, "OnIBBAns"_COMMA, "Wrong MessageType: ", lMsgType);
		//break;
		// Request from Client->GLS->IBB


	case 1:
		{
			theLog.Put(DEV_UK, "NGS_Room, OnIBBAns - MSGTYPE : 1 CSN: ", lCSN, " MsgType: ", lMsgType );
		}
		break;
		// Request from Client->GLS->IBB
	case 2:
		{
			break;
		}
	}
}

void CRoom::OnTerminateRoomReq(LONG lType)
{
}

void CRoom::OnBroadCastMessage(GBuf & buf )
{
	TLock lo(this);
	m_pInternalLogic->OnBroadCastMsg( buf );
}


void CRoom::GetNFRoomOption(NFRoomOption& nfRoomOption)
{
	if (m_pInternalLogic)
		nfRoomOption = m_pInternalLogic->GetNFRoomOption();
}

void CRoom::InitUserSlot()
{
	m_pInternalLogic->m_UserSlot.Init();
}


const LRBAddress& CRoom::GetAddress() const
{
	return m_pInternalLogic->GetRoomAddr();
}

void CRoom::ChangeCHSAddr(const LRBAddress& oldAddr, const LRBAddress& newAddr) 
{
	TLock lo(this);
	return m_pInternalLogic->ChangeCHSAddr( oldAddr, newAddr );
}

void CRoom::SetCHSAddr(const LRBAddress &chsAddr) 
{ 
	m_pInternalLogic->SetCHSAddr( chsAddr );
}