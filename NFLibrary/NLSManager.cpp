// NLSManager.cpp: implementation of the NLSManagerData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _NGSNLS
	#include "../project/NGS/Common.h"
#elif _CHSNLS
	#include "../project/chs/common.h"
#endif


#include <NLSManager.h>

#ifdef _NGSNLS
#define _USENLS
#endif
#ifdef _CHSNLS
#define _USENLS
#endif
#ifdef _NCSNLS
#define _USENLS
#endif

#ifdef _USENLS

#ifdef _CHSNLS
#include "../project/chs/ChannelDir.h"
#include "../project/chs/Channel.h"
#include "../project/chs/LRBManager.h"
#include "../project/chs/Listener.h"
#define CRF_DBERROR CHS_UNKNOWN
#define JRF_DBERROR CHS_UNKNOWN
#endif

#ifdef _NGSNLS
#include "../project/NGS/RoomTable.h"
#include "../project/NGS/Room.h"
#include "../project/NGS/LrbConnector.h"
#include "../project/NGS/Listener.h"
#include <NF/ADL/MsgCHSNGS.h>
#include "../project/NGS/Control.h"
#endif

#ifdef _NCSNLS
#include <NF/ADL/MsgNCSCli.h>
#include "../project/ncs/CharLobby.h"
#include "../project/ncs/CharLobbyManager.h"
#include "../project/ncs/LRBhandler.h"
#include "../project/ncs/Listener.h"
#include "../project/ncs/User.h"
#endif

#include <PMSConnObject.h>

NLSManager	theNLSManager;

////////////////////////////////////////////////////////////////////////////////////////////////
// NLSManager
////////////////////////////////////////////////////////////////////////////////////////////////

NLSManager::NLSManager()
{
	m_lMsgCount = 0L;
	m_dwLogicAddr = 0UL;
	m_dwTypeID = 0UL;

	m_dwRefCnt = 0UL;
}

NLSManager::~NLSManager()
{

}
STDMETHODIMP_(ULONG) NLSManager::AddRef() 
{
	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
	return dwRefCnt;
}

STDMETHODIMP_(ULONG) NLSManager::Release() 
{
	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
	if(dwRefCnt == 0)
	{
	}
	return dwRefCnt;
}

BOOL NLSManager::RunNLSManager(NSAP & nsap)
{
	TLock lo(this);
//	VALIDATE(m_NLSTimer.Activate(GetThreadPool(), this, NLSVALIDATE_TIMERINTERVAL, NLSVALIDATE_TIMERINTERVAL));

	m_nsap = nsap;
// 	// NLS 등록 요청...
// #ifdef _NGSNLS
// 	SendRegisterServiceAnsToNLS(theRoomTable.GetNLSAddr());
// #endif
// #ifdef _CHSNLS
// 	SendRegisterServiceAnsToNLS(theLRBHandler.GetNLSAddr());
// #endif

	return TRUE;
}

// main or sub NLS ? -> 다르게 처리 되어야 함.
void NLSManager::NLSTerminateNtf(LRBAddress& _Addr)
{
	string strAddr;
	_Addr.GetStringFormat(strAddr);
	theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "----- NLS Terminate [",strAddr, "] -----");
}

STDMETHODIMP_(void) NLSManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if(hSignal == 0)
	{

	}
	else
	{
		if(m_NLSTimer.IsHandle(hSignal))	
		{
			TLock lo(this);
		}
	}
}

void NLSManager::SendNLSMessage(PayloadCLINLS & pld, const LRBAddress& des)
{
#ifdef _NGSNLS
	const LRBAddress& addr = theRoomTable.GetAddr();
#elif _CHSNLS
	const LRBAddress& addr = theLrbManager.GetMyAddress();
#elif _NCSNLS
	const LRBAddress& addr = theLRBHandler.GetMyAddr();
#endif

	theLrbManager.SendToNLS(addr, des, pld);
}

//
// 접속을 시도해오는 사용자.. NLSManagerData에 항목을 추가하고, LRB를 통해 NLS로 메세지 전달까지 수행
static DWORD g_dwBypassCnt = 0UL;
BOOL NLSManager::AddUserToNLS(LPNLSOBJECT lpObj, LONG client_ip, string & sOption, LONG lMsgType, LONG lStatus, IXObject * pObj)//CRoomPtr & spRoom)
{
	TLock lo(this);
	TKey	_key;
	lpObj->NLSGetKeyValue(_key);		// csn

	if(!lpObj)
	{
		theLog.Put(ERR_UK, "NLSManager"_COMMA, "Add User to NLSManager : invalid Item Argument");
		return FALSE;
	}

	CUser* pUser = (CUser*)lpObj;

#ifdef _NGSNLS
	LRBAddress _myAddr = theRoomTable.GetAddr();
	LRBAddress _addr = theRoomTable.GetNLSAddr();
#elif _CHSNLS
	LRBAddress _myAddr = theLrbManager.GetMyAddress();
	LRBAddress _addr = theLrbManager.GetNLSAddr();
#elif _NCSNLS
	LRBAddress _myAddr = theLrbManager.GetMyAddr();
	LRBAddress _addr = theLrbManager.GetNLSAddr();
#endif

// 	// NLS 가 필요로 하는 데이터를 제공.	
// 	NLSBaseInfo info;
// 	RoomID rid;
// 	lpObj->NLSGetRoomID(rid);
// 	info.m_lKey = _Key;
// 	info.m_nsap = m_nsap;
// 	info.m_lClientIP = client_ip;	
// 
// 	// NLS가 죽어 있는 경우 그냥.. 접속으로 처리.
// 	LRBAddress _addr = m_NLSIPSet.SelectNLS(_Key);
// 	if(_addr.addr[0] == BYTE(' ') && pObj)//spRoom)
// 	{
// 		// NLS와 연동되지 않음을 알수 있어야 하는데.. 
// 		g_dwBypassCnt++;
// 		if(g_dwBypassCnt % 300UL == 0UL )
// 		{
// 			g_dwBypassCnt = 0UL;
// 			if(_addr.addr[0] != BYTE(' '))
// 				theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "NLS, Bypass AddUser Msg : invalid address");
// 			else 
// 				theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "NLS, Bypass AddUser Msg : invalid pObj point");
// 		}
// 		lpObj->NLSSetErrorCode(S_NLS_NOTEXIST);
// 
// 		{
// 			PMSAWarningNtf msgNtf;
// 			msgNtf.m_sWarnMsg  = ::format("Can't Find NLS Server \n");
// 			msgNtf.m_sTreatMsg = ::format("Check the NLS Server [Key:%d] ",_Key);
// 			msgNtf.m_lErrLevel = FL_CRITICAL;
// 			PayloadHA pld(PayloadHA::msgPMSWarningNtf_Tag,msgNtf);
// 
// 			thePMSConnector.SendWarningMsg(msgNtf.m_lErrLevel, msgNtf.m_sWarnMsg.c_str(), msgNtf.m_sTreatMsg.c_str(), 0, 0);
// 		}
#ifdef _CHSNLS
	::XsigQueueSignal(GetThreadPool(), pObj, CHANNELSIGNAL_NLSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
	LONG lLevel = pUser->GetUserData().m_nfCharInfoExt.m_nfCharBaseInfo.m_lLevel;
	LONG lGameMode = pUser->GetGameMode();
#elif _NGSNLS
 	::XsigQueueSignal(GetThreadPool(), pObj, ROOMSIGNAL_NLSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
	LONG lLevel = pUser->GetNFCharInfoExt()->m_nfCharBaseInfo.m_lLevel;
	LONG lGameMode = pUser->GetNFRoomOption().m_lPlayType; // 따라가기할 때 구분용, 1:FreeMode 인지 아닌지만 구분해 주면 됨.
#elif _NCSNLS
	::XsigQueueSignal(GetThreadPool(), pObj, (HSIGNAL)CCharLobbyManager::CHARLOBBYMGR_NCSADDUSERANS, (WPARAM)lMsgType, (LPARAM)lpObj);
	LONG lGameMode = 0;
	LONG lLevel = 0;
	NFCharInfoExt nfCharInfoExt;
	if( pUser->FindNFCharInfoExt( pUser->GetCSN(), nfCharInfoExt) )
	{
		lLevel = nfCharInfoExt.m_nfCharBaseInfo.m_lLevel;
	}
#endif
// 		return TRUE;
// 	}

	PayloadCLINLS pld(PayloadCLINLS::msgInsertCharacterReq_Tag);
	pld.un.m_msgInsertCharacterReq->m_Key = _key;
	pld.un.m_msgInsertCharacterReq->m_lStatus = lStatus;
	RoomID	roomID;	
	pUser->NLSGetRoomID(roomID);
	pld.un.m_msgInsertCharacterReq->m_roomID = roomID;
	pld.un.m_msgInsertCharacterReq->m_lGameMode = lGameMode;
	pld.un.m_msgInsertCharacterReq->m_serverLRBAddr = _myAddr;
	pld.un.m_msgInsertCharacterReq->m_nsapServerCurrent = m_nsap;
	pld.un.m_msgInsertCharacterReq->m_lLevel = lLevel;
	SendNLSMessage(pld, _addr);

	m_lMsgCount++;
	return TRUE;
}

//
// 클라이언트가 직접 disconnect를 요청 했을 경우에만, NLS에서 삭제한다.
//
void NLSManager::DelUserToNLS(LONG lKey, RoomID & rid)
{
	TLock lo(this);

#ifdef _NGSNLS
	if (rid.m_dwGRIID == ABSOLUTE_DELETE_VALUE)
		theLog.Put(WAR_UK, "NGS_NLSManager, ABSOLUTE_DELETE_VALUE GRIID, USN = ", lKey);
#else
	if (rid.m_dwGRIID == ABSOLUTE_DELETE_VALUE)
		theLog.Put(WAR_UK, "CHS_NLSManager, ABSOLUTE_DELETE_VALUE GRIID, USN = ", lKey);
#endif

	// NLS에 사용자가 정보 삭제를 요구.
	PayloadCLINLS pld(PayloadCLINLS::msgDeleteCharacterReq_Tag);
//	pld.un.m_msgDeleteCharacterReq->m_lKey = lKey;

#ifdef _NGSNLS
	SendNLSMessage(pld, theRoomTable.GetNLSAddr());
#elif _CHSNLS
	SendNLSMessage(pld, theLrbManager.GetNLSAddr());
#elif _NCSNLS
	SendNLSMessage(pld, theLrbManager.GetNLSAddr());
#endif
	return;
}

//
// 시스템에서 자르거나, 문제가 있어서 접속이 종료 되는 경우에는 Disconnect로 status를 변경한다.
//
void NLSManager::UpdateUserToNLS(TKey& key, LONG lStatus, RoomID & rid, LONG lLevel, LONG lGameMode)
{
	TLock lo(this);

#ifdef _NGSNLS
	LRBAddress _myAddr = theRoomTable.GetAddr();
	LRBAddress _addr = theRoomTable.GetNLSAddr();
#elif _CHSNLS
	LRBAddress _myAddr = theLrbManager.GetMyAddress();
	LRBAddress _addr = theLrbManager.GetNLSAddr();
#elif _NCSNLS
	LRBAddress _myAddr = theLrbManager.GetMyAddr();
	LRBAddress _addr = theLrbManager.GetNLSAddr();
#endif

#ifdef _NGSNLS
	if (rid.m_dwGRIID == ABSOLUTE_DELETE_VALUE)
		theLog.Put(WAR_UK, "NGS_NLSManager, ABSOLUTE_UPDATE_VALUE GRIID, USN = ", key.m_lMainKey, ", CSN = ", key.m_lSubKey);	
#else
	if (rid.m_dwGRIID == ABSOLUTE_DELETE_VALUE)
		theLog.Put(WAR_UK, "CHS_NLSManager, ABSOLUTE_UPDATE_VALUE GRIID, USN = ", key.m_lMainKey, ", CSN = ", key.m_lSubKey);
#endif

	// NLS에 사용자가 정보 삭제를 요구.
	PayloadCLINLS pld(PayloadCLINLS::msgUpdateCharacterReq_Tag);
	pld.un.m_msgUpdateCharacterReq->m_Key = key;
	pld.un.m_msgUpdateCharacterReq->m_lStatus = lStatus;
	pld.un.m_msgUpdateCharacterReq->m_roomID = rid;
	pld.un.m_msgUpdateCharacterReq->m_serverLRBAddr = _myAddr;
	pld.un.m_msgUpdateCharacterReq->m_lLevel = lLevel;
	pld.un.m_msgUpdateCharacterReq->m_nsapServerCurrent = m_nsap;
	pld.un.m_msgUpdateCharacterReq->m_lGameMode = lGameMode;
	

#ifdef _NGSNLS
	SendNLSMessage(pld, theRoomTable.GetNLSAddr());
#elif _CHSNLS
	SendNLSMessage(pld, theLRBHandler.GetNLSAddr());
#elif _NCSNLS
	SendNLSMessage(pld, theLrbManager.GetNLSAddr());
#endif
	return;
}




///////////////////////////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////////////////////////
// void NLSManager::SendRegisterServiceAnsToNLS(const LRBAddress& dwNLSAddr)
// {
// 	MsgOUTNLS_ServerRegisterAns msg;
// 	const NSAP& nsap = GetNSAP();
// #ifdef _NGSNLS
// 	memcpy(msg.m_addr.addr, theRoomTable.GetAddr().addr, sizeof(theRoomTable.GetAddr().addr));
// #else
// 	memcpy(msg.m_addr.addr, theLrbManager.GetMyAddress().addr, sizeof(theLrbManager.GetMyAddress().addr));
// #endif
// 	msg.m_nsap = nsap;
// 	PayloadCLINLS pld(PayloadCLINLS::msgServerRegisterAns_Tag, msg);
// 
// 	SendNLSMessage(pld, dwNLSAddr);
// }

///////////////////////////////////////////////////////////////////////////////////////////////////
// 서비스 의존적인 메세지 처리.. 피할수 없나?
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LRB를 통해서 수신된 NLS 메세지를 처리..
//
void NLSManager::RecvNLSMessage(const PayloadNLSCLI * pld, const LRBAddress& src)	// posting으로 받을 것인가?
{
	switch(pld->mTagID)
	{
	case PayloadNLSCLI::msgInsertCharacterAns_Tag:
 		OnInsertCharacterAns(*(pld->un.m_msgInsertCharacterAns));
 		break;
	case PayloadNLSCLI::msgUpdateCharacterAns_Tag:
		break;
	case PayloadNLSCLI::msgDeleteCharacterAns_Tag:
		OnDeleteCharacterAns(*(pld->un.m_msgDeleteCharacterAns));
		break;
	case PayloadNLSCLI::msgGetCharacterAns_Tag:
 		OnGetCharacterAns(*(pld->un.m_msgGetCharacterAns));
 		break;
	case PayloadNLSCLI::msgDisconnectUserReq_Tag:
		OnDisconnectUserReq(*(pld->un.m_msgDisconnectUserReq));
		break;
	case PayloadNLSCLI::msgAnsLocation_Tag:
		OnAnsLocation(*(pld->un.m_msgAnsLocation));
		break;
// 	case PayloadNLSCLI::msgUserListReq_Tag:
// 		OnUserListReqFromNLS(*(pld->un.m_msgUserListReq), src);
// 		break;
// 	case PayloadNLSCLI::msgDisconnectUserReq_Tag:
// 		OnDisconnectUserReqFromNLS(*(pld->un.m_msgDisconnectUserReq));
// 		break;
// 	case PayloadNLSCLI::msgServerRegisterReq_Tag:
// 		OnServerRegisterReqFromNLS(*(pld->un.m_msgServerRegisterReq), src);
// 		break;
// 	case PayloadNLSCLI::msgQueryUserStateReq_Tag:
// 		OnQueryUserStateReqFromNLS(*(pld->un.m_msgQueryUserStateReq), src);
// 		break;
// 	case PayloadNLSCLI::msgKickOutUserReq_Tag:
// 		OnKickOutUserReqFromNLS(*(pld->un.m_msgKickOutUserReq),src);
// 		break;
// 	case PayloadNLSCLI::msgKickOutUserNtf_Tag:
// 		OnKickOutUserNtfFromNLS(*(pld->un.m_msgKickOutUserNtf),src);
// 		break;
	default:
		{
			theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "CLrbConnector::OnRcvNLSMsg - Unknown message(Tag:", pld->mTagID, ")");
		}
		break;
	}
}

// Insert에 대한 응답
void NLSManager::OnInsertCharacterAns(MsgNLSCLI_InsertCharacterAns & msg)
{
	// 에러코드를 확인하고 기존에 접속하고 있던 사람이거나 Disconnect 상태가 아니었던 사람이면 잘라버린다.
	if (msg.m_lErrorCode == S_INSERT_SUCCESS)
		return;

#ifdef _NCSNLS
	
#endif

#ifdef _CHSNLS 

	//////////// 채널 재접속을 위한 RoomID를 포함한 UserJoinAns 메시지 전달...
	//if(msg.m_lErrorCode == E_INSERT_USEREXIST) 
	//{			
	//	RoomID newRoomID;
	//	lpObj->LcsGetRoomID(newRoomID);
	//	if(!(lMsgType == LCSMSGTYPE_INVITCHANNEL && msg.m_roomID.m_dwGCIID == newRoomID.m_dwGCIID && msg.m_roomID.m_dwGRIID != 0))	/// Channel ID가 같으면 그냥 넘어가도 되겠지?
	//	{
	//		RoomID ridNULL;
	//		ridNULL.Clear();

	//		CRoomID_For_ChannelReJoin* rFC = new CRoomID_For_ChannelReJoin(lMsgType, msg.m_roomID);
	//		::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_DUPLICATEJOINCHANNELANS, (WPARAM)lpObj, (LPARAM)rFC);
	//		return;
	//	}
	//}
	//////////////////////////////////////////////////////////////////////////
//
//	ChannelID cid(msg.m_roomID.m_lSSN, msg.m_roomID.m_dwCategory, msg.m_roomID.m_dwGCIID);
//
//	CUser* pUser;
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(cid, &spChannel)) 
//{
////		pUser = spChannel->FindUser(msg.m_Key.m_lMainKey);
////		lpObj->LcsSetErrorCode(CRF_DBERROR);	// 임시로 DB Error로 처리..		
//		if(lMsgType == LCSMSGTYPE_NJOINCHANNEL)
//			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_JOINCHANNELANS, (WPARAM)lpObj, (LPARAM)ERR_CHANNEL_NOTFOUND);
//		else if(lMsgType == LCSMSGTYPE_INVITCHANNEL)
//			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_INVITECHANNELANS, (WPARAM)lpObj, (LPARAM)ERR_CHANNEL_NOTFOUND);
//		else if(lMsgType == LCSMSGTYPE_DJOINCHANNEL)
//			::XsigQueueSignal(GetThreadPool(), &theListener, LISTENER_DJOINCHANNELANS, (WPARAM)lpObj, (LPARAM)ERR_CHANNEL_NOTFOUND);
//		return;		
//	}
//	lpObj->LcsSetErrorCode(lErrorCode);
//	::XsigQueueSignal(GetThreadPool(), spChannel, CHANNELSIGNAL_LCSANSWER, (WPARAM)lMsgType, (LPARAM)lpObj);
#endif
}

// 삭제에 대한 응답
void NLSManager::OnDeleteCharacterAns(MsgNLSCLI_DeleteCharacterAns & msg)
{
	
}

// User 정보에 대한 응답
void NLSManager::OnGetCharacterAns(MsgNLSCLI_GetCharacterAns & msg)
{
	//msg.m_lstNLBBaseInfo
}

void NLSManager::OnAnsLocation(MsgNLSCLI_AnsLocation& msg)
{
	GBuf buf;
	::LStore(buf, msg);
	LPXBUF pXBuf = buf.Detach();
	VALIDATE(pXBuf);

#ifdef _NCSNLS

	HSIGNAL hSig = 0x00000000;

	switch( msg.m_lCause )
	{
	case NLRC_FRIEND_LIST:
		{
			hSig = (HSIGNAL)CCharLobby::CHARLOBBY_NLS_GET_FRIEND_LIST_INFO;
		}break;
	case NLRC_NTF_ADD_FRIEND:
		{
			hSig = (HSIGNAL)CCharLobby::CHARLOBBY_PROCESS_NF_FRIEND_APPLICATION;
		}break;
	case NLRC_NTF_ACCEPT_FRIEND:
		{
			hSig = (HSIGNAL)CCharLobby::CHARLOBBY_PROCESS_NF_FRIEND_ACCEPT;
		}break;
	case NLRC_NTF_AUTO_ACCEPT_FRIEND:
		{
			hSig = (HSIGNAL)CCharLobby::CHARLOBBY_PROCESS_NF_FRIEND_AUTO_ACCEPT;
		}break;
	case NLRC_FOLLOW_USER:
		{
			hSig = (HSIGNAL)CCharLobby::CHARLOBBY_PROCESS_FOLLOW_USER;
		}break;
	case NLRC_NEW_LETTER:
		{
			hSig = (HSIGNAL)CCharLobby::CHARLOBBY_PROCESS_NF_LETTER_NEW;
		}break;
	}

	CCharLobby* pCharLobby = theCharLobbyManager.FindCharLobby(msg.m_lCSN);
	if (NULL != pCharLobby)
		::XsigQueueSignal(GetThreadPool(), pCharLobby, hSig, (WPARAM)pXBuf, 0);
	
#endif

#ifdef _NGSNLS

	HSIGNAL hSig = 0x00000000;

	CRoomPtr spRoom;
	BOOL bRet = theRoomTable.FindRoom(msg.m_roomID, &spRoom);
	if(bRet)
	{
		switch( msg.m_lCause )
		{
		case NLRC_FRIEND_LIST:
			{
				hSig = ROOMSIGNAL_PROCESS_NF_FRIEND_LIST;
			}break;		
		case NLRC_NTF_ADD_FRIEND:
			{
				hSig = ROOMSIGNAL_PROCESS_NF_FRIEND_APPLICATION;
			}break;
		case NLRC_NTF_ACCEPT_FRIEND:
			{
				hSig = ROOMSIGNAL_PROCESS_NF_FRIEND_ACCEPT;
			}break;
		case NLRC_NTF_AUTO_ACCEPT_FRIEND:
			{
				hSig = ROOMSIGNAL_PROCESS_NF_FRIEND_AUTO_ACCEPT;
			}break;
		case NLRC_NEW_LETTER:
			{
				hSig = ROOMSIGNAL_PROCESS_NF_LETTER_NEW;
			}break;
		case NLRC_FOLLOW_USER:
			{
				hSig = ROOMSIGNAL_PROCESS_FOLLOW_USER;
			}break;
		}

		::XsigQueueSignal(GetThreadPool(), spRoom, hSig, (WPARAM)pXBuf, 0);
	}
	else
		theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "CLrbConnector::OnAnsLocation - NotFound Room : ", msg.m_roomID.m_dwGRIID);
#endif

#ifdef _CHSNLS

	ChannelID channelID;
	channelID = msg.m_roomID;

	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	if( NLRC_NTF_ACCEPT_FRIEND == msg.m_lCause )
	{
		::XsigQueueSignal(GetThreadPool(), spChannel, CHANNEL_PROCESS_NF_FRIEND_ACCEPT, (WPARAM)pXBuf, 0);
	}
	else if( NLRC_NTF_ADD_FRIEND == msg.m_lCause )
	{
		::XsigQueueSignal(GetThreadPool(), spChannel, CHANNEL_PROCESS_NF_FRIEND_ADD, (WPARAM)pXBuf, 0);
	}
	else if( NLRC_FOLLOW_USER == msg.m_lCause )
	{
		::XsigQueueSignal(GetThreadPool(), spChannel, CHANNEL_PROCESS_FOLLOW_USER, (WPARAM)pXBuf, 0);
	}
#endif
}


// 어뷰징 유저를 끊어라...
void NLSManager::OnDisconnectUserReq(MsgNLSCLI_DisconnectUserReq & msg)
{
#ifdef _NCSNLS
	CCharLobby* pCharLobby = theCharLobbyManager.FindCharLobby(msg.m_Key.m_lMainKey);
	if (NULL != pCharLobby)
        ::XsigQueueSignal(GetThreadPool(), pCharLobby, (HSIGNAL)CCharLobby::CHARLOBBY_LINKCUT_FROMNLS, (WPARAM)(msg.m_Key.m_lMainKey), 0); // USN으로 안됨..
#endif

#ifdef _NGSNLS
	CRoomPtr spRoom;
	BOOL bRet = theRoomTable.FindRoom(msg.m_roomID, &spRoom);
	if(bRet)
	{
		::XsigQueueSignal(GetThreadPool(), spRoom, (HSIGNAL)ROOM_USERLINKCUT, (WPARAM)(msg.m_Key.m_lSubKey), LCS_LINKCUT_KICKOUT); // USN으로 안됨..
	}
	else
		theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "CLrbConnector::OnInsertCharacterAns - NotFound Room : ", msg.m_roomID.m_dwGRIID);
#endif

#ifdef _CHSNLS
	ChannelID	channelID;
	channelID.m_lSSN = msg.m_roomID.m_lSSN;
	channelID.m_dwCategory = msg.m_roomID.m_dwCategory;
	channelID.m_dwGCIID = msg.m_roomID.m_dwGCIID;
	
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "----- OnDisconnectUserReq FromNLS = USN/CSN : [", msg.m_Key.m_lMainKey, "]/[", msg.m_Key.m_lSubKey,"] ----");

	::XsigQueueSignal(GetThreadPool(), spChannel, CHANNEL_USERLINKCUT_SIG_FROM_NLS, (WPARAM)(msg.m_Key.m_lSubKey), 0); // USN으로 안됨..
#endif
}

void NLSManager::GetUserLocation( PayloadCLINLS& pld )
{
#ifdef _NGSNLS
	SendNLSMessage(pld, theRoomTable.GetNLSAddr());
#elif _CHSNLS
	SendNLSMessage(pld, theLRBHandler.GetNLSAddr());
#elif _NCSNLS
	SendNLSMessage(pld, theLrbManager.GetNLSAddr());
#endif
	return;
}

// void NLSManager::OnUserListReqFromNLS(MsgNLSCLI_ & msg, const LRBAddress& des)
// {
// 	TLock lo(this);
// 
// 	NLSBaseInfoList lstUser;
// 	// m_nextRoomID는 [IN][OUT] parameter.. 이다..
// 	LONG lPartKey = m_NLSIPSet.FindAddrIndex(des);
// 	if(lPartKey < 0)
// 		return;
// 
// 	// lNextIndex 값이 < 0 인것을 마지막으로 보내야 한다....
// 	PayloadCLINLS pld(PayloadCLINLS::msgUserListAns_Tag);
// 	pld.un.m_msgUserListAns->m_lstNLSBaseInfo = lstUser;	
// 
// 	SendNLSMessage(pld, des);
// }

// 광고방 제재를 위해 해당 사용자 강퇴 :GLS
// void NLSManager::OnKickOutUserReqFromNLS(MsgNLSCLI_KickOutUserReq & msgFromNLS, const LRBAddress& des)
// {
// 	TLock lo(this);
// #ifdef _CHSNLS
// #else _NGSNLS
// 	//MsgOUTNLS_KickOutUserAns msgToNLS;
// 	//msgToNLS.m_addr		= msgFromNLS.m_addr;
// 	//msgToNLS.m_dwMID	= msgFromNLS.m_dwMID;	
// 
// 	PayloadCLINLS pldNLS(PayloadCLINLS::msgKickOutUserAns_Tag);
// 	//pldNLS.un.m_msgKickOutUserAns =  &msgToNLS;	
// 
// 	MsgOUTNLS_KickOutUserAns *pMsgToNLS = pldNLS.un.m_msgKickOutUserAns;
// 	pMsgToNLS->m_addr	= msgFromNLS.m_addr;
// 	pMsgToNLS->m_dwMID	= msgFromNLS.m_dwMID;	
// 
// 
// 	CRoom* pRoom = NULL;
// 	theRoomTable.FindRoom(msgFromNLS.m_roomID, &pRoom);
// 
// 	if (pRoom == NULL)
// 	{	//에러 발생
// 		pMsgToNLS->m_lResultCode	= -101; // GLS Not Found Room Error
// 		pMsgToNLS->m_sResult		= "GLS_Error: Can't Find Room";
// 		theLog.Put(WAR_UK, "NGS_NLSManager"_COMMA, "Not found room .. KickOutUserReq FromNLS, RoomID:",msgFromNLS.m_roomID.m_dwGRIID);		
// 	} 
// 	else
// 	{
// 		CUser* pUser = pRoom->FindUser(msgFromNLS.m_lGSN);
// 
// 		if (pUser)
// 		{
// 			UserBaseInfo userInfo = pUser->GetUserData();
// 			string sUID = userInfo.m_sUID.substr(0, userInfo.m_sUID.length()-3);
// 			string sUserInfo = ::format("%s(%s***)", userInfo.m_sNickName.c_str(), sUID.c_str());
// 			// 방에서만 강퇴.
// 			::XsigQueueSignal(GetThreadPool(), pRoom, (HSIGNAL)ROOM_USERLINKCUT, (WPARAM)(msgFromNLS.m_lGSN), 0);
// 			// if msgtype is terminate_room then notify it to GRC
// 			pRoom->AnnounceKickoutUser(KICKOUT_USER_CHEAT, sUserInfo);
// 			theLog.Put(WAR_UK, "NGS_NLSManager"_COMMA, "Kickout User: ", sUserInfo);
// 		}
// 
// 		if (msgFromNLS.m_lType == 3 || msgFromNLS.m_lType == 4)
// 			pRoom->OnTerminateRoomReq(TERMINATE_ROOM_GRACEFUL);
// 		else if (msgFromNLS.m_lType == 5 || msgFromNLS.m_lType == 6)
// 			pRoom->OnTerminateRoomReq(TERMINATE_ROOM_IMMEDIATE);
// 
// 		pMsgToNLS->m_lResultCode	= 0x0001; // not error	
// 		pMsgToNLS->m_sResult		= "OK";
// 		theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "OnKickOutUserReqFromNLS. Type:", msgFromNLS.m_lType, ", RoomID: ", msgFromNLS.m_roomID.m_dwGRIID, ", GSN: ", msgFromNLS.m_lGSN);	
// 	}
// 
// 	theLrbManager.SendToNLS(theRoomTable.GetAddr(), des, pldNLS);
// 
// 	if (pRoom)
// 		pRoom->Release();
// #endif
//}

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
// void NLSManager::OnKickOutUserNtfFromNLS(MsgNLSCLI_KickOutUserNtf & msgFromNLS, const LRBAddress& des)
// {
// 	TLock lo(this);
// #ifdef _CHSNLS
// 	MsgCHSGLS_ChannelIDList		msgChannelIDList;
// 	theChannelDir.OnChannelDListReq(msgFromNLS.m_lSSN, msgChannelIDList);
// 
// 	ForEachElmt(CHANNELIDLIST, (msgChannelIDList.m_lstChannelID), i, j)
// 	{
// 		ChannelID cid = *i;
// 		CChannel * pChannel = NULL;
// 		theChannelDir.GetChannel(cid, &pChannel);
// 		if(pChannel &&  (pChannel->IsUserConnected(msgFromNLS.m_lGSN)))
// 		{
// 			::XsigQueueSignal(GetThreadPool(), pChannel, CHANNEL_USERLINKCUT_SIG, (WPARAM)(msgFromNLS.m_lGSN), 0);
// 			theLog.Put(INF_UK, "CHS_NLSManager, KickOutUserNtfFromNLS : SSN=",msgFromNLS.m_lSSN, ", USN=", msgFromNLS.m_lGSN);
// 		}
// 	}
// #else _NGSNLS
// 	// GLS 는 이 메시지 안 받음.
// #endif
// }
// 
// void NLSManager::OnDisconnectUserReqFromNLS(MsgNLSCLI_DisconnectUserReq & msg)
// {
// 	TLock lo(this);
// #ifdef _CHSNLS
// 	ChannelID cid(msg.m_roomID.m_lSSN, msg.m_roomID.m_dwCategory, msg.m_roomID.m_dwGCIID);
// 	CChannel * pChannel = NULL;
// 	theChannelDir.GetChannel(cid, &pChannel);
// 	if(!pChannel)
// 	{
// 		theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "Not found channel .. DisconnectUserReq FromNLS : SSN/Category/GCIID = [", \
// 			msg.m_roomID.m_lSSN, "/", msg.m_roomID.m_dwCategory, "/", msg.m_roomID.m_dwGCIID, "]");
// 		return;
// 	}
// 
// 	bool bCheckDis = false;
// 
// 	int ivecSize = theChannelDir.m_vecCHSGLSConSSN.size();
// 	if (ivecSize > 0)
// 	{
// 		for(int i=0; i<ivecSize; i++)
// 		{
// 			// INI파일에 설정되어 있는 SSN(NLS로 부터 NLSOUT 메세지를 받으면 끊지 말아야되는 게임의 SSN)과
// 			// MSG를 받은 SSN이 한 개라도 같으면... USERLINKCUT_Signal을 보내지 않는다.
// 			if (theChannelDir.m_vecCHSGLSConSSN[i] == msg.m_roomID.m_lSSN)
// 			{
// 				bCheckDis = true;
// 			}
// 		}
// 	}
// 
// 	if (!bCheckDis)
// 	{
// 		theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "----- DisconnectUserReq FromNLS 2 = GSN/SSN : [", msg.m_lGSN, "]/[", msg.m_roomID.m_lSSN,"] ----");
// 		::XsigQueueSignal(GetThreadPool(), pChannel, CHANNEL_USERLINKCUT_SIG, (WPARAM)(msg.m_lGSN), 0); // GSN으로 안됨..
// 	}
// 
// #else
// 	CRoom* pRoom = NULL;
// 	theRoomTable.FindRoom(msg.m_roomID, &pRoom);
// 	if (!pRoom) 
// 	{
// 		theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "Not found room .. DisconnectUserReq FromNLS 1. mgs.m_roomID:",RoomID2Str(msg.m_roomID),", USN:", msg.m_lGSN);
// 		return;
// 	} 
// 	else	
// 	{
// 		theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "Duplicated room .. DisconnectUserReq FromNLS 2. RoomID:", RoomID2Str(msg.m_roomID), ", USN:", msg.m_lGSN);
// 		//		::XsigQueueSignal(GetThreadPool(), pRoom, ROOM_USERLINKCUTTING, (WPARAM)(msg.m_lGSN), 0); // GSN으로 안됨..
// 		::XsigQueueSignal(GetThreadPool(), pRoom, (HSIGNAL)ROOM_USERLINKCUT, (WPARAM)(msg.m_lGSN), 0); // GSN으로 안됨..
// 
// 	}
// 	pRoom->Release();
// #endif
// }

// void NLSManager::OnServerRegisterReqFromNLS(MsgNLSCLI_ServerRegisterReq & msg, const LRBAddress& des)
// {
// 	TLock lo(this);
// 	vector<LRBAddress> vecNLSIP;
// 	vecNLSIP.assign(msg.m_vecNLSIP.begin(), msg.m_vecNLSIP.end());
// 	// NLS IP List를 유지..
// 	m_NLSIPSet.InitIPSet(vecNLSIP, msg.m_dwMaster, msg.m_dwLogicalAddr);//dwNLSAddr);
// 
// 	if (!m_NLSIPSet.StartNLS(msg.m_dwLogicalAddr))//dwNLSAddr);
// 	{
// 		PMSAWarningNtf msgNtf;
// 		msgNtf.m_sWarnMsg  = ::format("NLS Registration Fail\n");
// 		msgNtf.m_sTreatMsg = ::format("Inspect the NLS Server [Master:%s], [Logical:%s]\n",msg.m_dwMaster.GetString().c_str(), msg.m_dwLogicalAddr.GetString().c_str());
// 		msgNtf.m_lErrLevel = FL_CRITICAL;
// 		PayloadHA pld(PayloadHA::msgPMSWarningNtf_Tag,msgNtf);
// 
// 		thePMSConnector.SendWarningMsg(msgNtf.m_lErrLevel, msgNtf.m_sWarnMsg.c_str(), msgNtf.m_sTreatMsg.c_str(), 0, 0);
// 	}
// 
// 	for (unsigned int i = 0; i < vecNLSIP.size(); i++)
// 		theLog.Put(WAR_UK, "NGS_NLSManager"_COMMA, "OnServerRegisterReqFromNLS: NLS IP ", vecNLSIP[i].GetString().c_str());
// 
// 	SendRegisterServiceAnsToNLS(des);//dwNLSAddr);
// }

// void NLSManager::OnQueryUserStateReqFromNLS(MsgNLSCLI_QueryUserStateReq & msg, const LRBAddress& des)
// {
// 	TLock lo(this);
// 
// 	PayloadCLINLS pld(PayloadCLINLS::msgQueryUserStateAns_Tag);
// 	pld.un.m_msgQueryUserStateAns->m_lGSN = msg.m_lGSN;
// 	pld.un.m_msgQueryUserStateAns->m_lReserved = msg.m_lReserved;
// 
// #ifdef _NGSNLS
// 
// 	CRoom* pRoom = NULL;
// 	theRoomTable.FindRoom(msg.m_RoomID, &pRoom);
// 	if (!pRoom) 
// 	{
// 		theLog.Put(INF_UK, "NGS_NLSManager"_COMMA, "Not found room .. OnQueryUserStateReqFromNLS 1");
// 
// 		pld.un.m_msgQueryUserStateAns->m_lResult = 2L;
// 
// 		SendNLSMessage(pld, des);
// 	} 
// 	else	
// 	{
// 		LONG lResult = 1;//pRoom->GetUserState(msg.m_lGSN, msg.m_lReserved);
// 
// 		pld.un.m_msgQueryUserStateAns->m_lResult = lResult;
// 		SendNLSMessage(pld, des);
// 
// 		if (lResult == 0)
// 		{
// 			RoomEvent e(REV_USERDISCONNECT, msg.m_lGSN);
// 			pRoom->PushQueue(e);
// 
// 			theLog.Put(WAR_UK, "NGS_NLSManager"_COMMA, "OnQueryUserStateReqFromNLS: User not exist or waiting SSN ", msg.m_RoomID.m_lSSN, ", GSN ", msg.m_lGSN);
// 		}
// 
// 		pRoom->Release();
// 	}
// #else  //_CHSNLS . 보험판매관련 쿼리 처리를 한다.
// 
// 	// msg.m_lReserved != -1 인 경우 사용자 접속 관련이기 때문에 기존처럼 무시하게 한다.
// 	if (msg.m_lReserved != -1)
// 		return;
// 
// 	// 채널을 찾는다.
// 	ChannelID cid(msg.m_RoomID.m_lSSN, msg.m_RoomID.m_dwCategory, msg.m_RoomID.m_dwGCIID);
// 	CChannel * pChannel = NULL;
// 	theChannelDir.GetChannel(cid, &pChannel);
// 	if(!pChannel)
// 	{
// 		theLog.Put(ERR_UK, "CHS_NLSManagerErr, Not found channel .. OnQueryUserStateReqFromNLS FromNLS : SSN/Category/GCIID = [",
// 			msg.m_RoomID.m_lSSN, "/", msg.m_RoomID.m_dwCategory, "/", msg.m_RoomID.m_dwGCIID, "]");
// 		pld.un.m_msgQueryUserStateAns->m_lResult = 2L; // 해당 사용자 없음으로 보낸다.
// 		SendNLSMessage(pld, des);
// 		return;
// 	}
// 	if (pChannel->IsUserConnected(msg.m_lGSN))
// 	{	
// 		pld.un.m_msgQueryUserStateAns->m_lResult = 1;
// 
// 		// 사용자가 있다고 나온 경우 디버그 로그
// 		LONG lSize = m_mapConnectUser.size();
// 		if (m_mapConnectUser.end() == m_mapConnectUser.find(msg.m_lGSN))
// 			LOG(ERR_UK, "CHS_NLSErr.OnQueryUserStateReqFromNLS(). mismatch case 1 m_mapConnectUser.size()=", lSize, ", GSN:", msg.m_lGSN);
// 	}
// 	else 
// 	{ 
// 		pld.un.m_msgQueryUserStateAns->m_lResult = 2;		
// 		// 사용자가 없다고 나온 경우 디버그 로그
// 		LONG lSize = m_mapConnectUser.size();
// 		if (m_mapConnectUser.end() != m_mapConnectUser.find(msg.m_lGSN))
// 			LOG(ERR_UK, "CHS_NLSErr.OnQueryUserStateReqFromNLS(). mismatch case 2 m_mapConnectUser.size()=", lSize, ", GSN:", msg.m_lGSN);
// 	}
// 
// 	SendNLSMessage(pld, des);
// 	return;
// 
// #endif
// }

#endif
