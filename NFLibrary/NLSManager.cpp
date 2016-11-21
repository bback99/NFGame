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
// 	// NLS ��� ��û...
// #ifdef _NGSNLS
// 	SendRegisterServiceAnsToNLS(theRoomTable.GetNLSAddr());
// #endif
// #ifdef _CHSNLS
// 	SendRegisterServiceAnsToNLS(theLRBHandler.GetNLSAddr());
// #endif

	return TRUE;
}

// main or sub NLS ? -> �ٸ��� ó�� �Ǿ�� ��.
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
// ������ �õ��ؿ��� �����.. NLSManagerData�� �׸��� �߰��ϰ�, LRB�� ���� NLS�� �޼��� ���ޱ��� ����
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

// 	// NLS �� �ʿ�� �ϴ� �����͸� ����.	
// 	NLSBaseInfo info;
// 	RoomID rid;
// 	lpObj->NLSGetRoomID(rid);
// 	info.m_lKey = _Key;
// 	info.m_nsap = m_nsap;
// 	info.m_lClientIP = client_ip;	
// 
// 	// NLS�� �׾� �ִ� ��� �׳�.. �������� ó��.
// 	LRBAddress _addr = m_NLSIPSet.SelectNLS(_Key);
// 	if(_addr.addr[0] == BYTE(' ') && pObj)//spRoom)
// 	{
// 		// NLS�� �������� ������ �˼� �־�� �ϴµ�.. 
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
	LONG lGameMode = pUser->GetNFRoomOption().m_lPlayType; // ���󰡱��� �� ���п�, 1:FreeMode ���� �ƴ����� ������ �ָ� ��.
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
// Ŭ���̾�Ʈ�� ���� disconnect�� ��û ���� ��쿡��, NLS���� �����Ѵ�.
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

	// NLS�� ����ڰ� ���� ������ �䱸.
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
// �ý��ۿ��� �ڸ��ų�, ������ �־ ������ ���� �Ǵ� ��쿡�� Disconnect�� status�� �����Ѵ�.
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

	// NLS�� ����ڰ� ���� ������ �䱸.
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
// ���� �������� �޼��� ó��.. ���Ҽ� ����?
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LRB�� ���ؼ� ���ŵ� NLS �޼����� ó��..
//
void NLSManager::RecvNLSMessage(const PayloadNLSCLI * pld, const LRBAddress& src)	// posting���� ���� ���ΰ�?
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

// Insert�� ���� ����
void NLSManager::OnInsertCharacterAns(MsgNLSCLI_InsertCharacterAns & msg)
{
	// �����ڵ带 Ȯ���ϰ� ������ �����ϰ� �ִ� ����̰ų� Disconnect ���°� �ƴϾ��� ����̸� �߶������.
	if (msg.m_lErrorCode == S_INSERT_SUCCESS)
		return;

#ifdef _NCSNLS
	
#endif

#ifdef _CHSNLS 

	//////////// ä�� �������� ���� RoomID�� ������ UserJoinAns �޽��� ����...
	//if(msg.m_lErrorCode == E_INSERT_USEREXIST) 
	//{			
	//	RoomID newRoomID;
	//	lpObj->LcsGetRoomID(newRoomID);
	//	if(!(lMsgType == LCSMSGTYPE_INVITCHANNEL && msg.m_roomID.m_dwGCIID == newRoomID.m_dwGCIID && msg.m_roomID.m_dwGRIID != 0))	/// Channel ID�� ������ �׳� �Ѿ�� �ǰ���?
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
////		lpObj->LcsSetErrorCode(CRF_DBERROR);	// �ӽ÷� DB Error�� ó��..		
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

// ������ ���� ����
void NLSManager::OnDeleteCharacterAns(MsgNLSCLI_DeleteCharacterAns & msg)
{
	
}

// User ������ ���� ����
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


// ���¡ ������ �����...
void NLSManager::OnDisconnectUserReq(MsgNLSCLI_DisconnectUserReq & msg)
{
#ifdef _NCSNLS
	CCharLobby* pCharLobby = theCharLobbyManager.FindCharLobby(msg.m_Key.m_lMainKey);
	if (NULL != pCharLobby)
        ::XsigQueueSignal(GetThreadPool(), pCharLobby, (HSIGNAL)CCharLobby::CHARLOBBY_LINKCUT_FROMNLS, (WPARAM)(msg.m_Key.m_lMainKey), 0); // USN���� �ȵ�..
#endif

#ifdef _NGSNLS
	CRoomPtr spRoom;
	BOOL bRet = theRoomTable.FindRoom(msg.m_roomID, &spRoom);
	if(bRet)
	{
		::XsigQueueSignal(GetThreadPool(), spRoom, (HSIGNAL)ROOM_USERLINKCUT, (WPARAM)(msg.m_Key.m_lSubKey), LCS_LINKCUT_KICKOUT); // USN���� �ȵ�..
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

	::XsigQueueSignal(GetThreadPool(), spChannel, CHANNEL_USERLINKCUT_SIG_FROM_NLS, (WPARAM)(msg.m_Key.m_lSubKey), 0); // USN���� �ȵ�..
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
// 	// m_nextRoomID�� [IN][OUT] parameter.. �̴�..
// 	LONG lPartKey = m_NLSIPSet.FindAddrIndex(des);
// 	if(lPartKey < 0)
// 		return;
// 
// 	// lNextIndex ���� < 0 �ΰ��� ���������� ������ �Ѵ�....
// 	PayloadCLINLS pld(PayloadCLINLS::msgUserListAns_Tag);
// 	pld.un.m_msgUserListAns->m_lstNLSBaseInfo = lstUser;	
// 
// 	SendNLSMessage(pld, des);
// }

// ����� ���縦 ���� �ش� ����� ���� :GLS
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
// 	{	//���� �߻�
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
// 			// �濡���� ����.
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
// 	// GLS �� �� �޽��� �� ����.
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
// 			// INI���Ͽ� �����Ǿ� �ִ� SSN(NLS�� ���� NLSOUT �޼����� ������ ���� ���ƾߵǴ� ������ SSN)��
// 			// MSG�� ���� SSN�� �� ���� ������... USERLINKCUT_Signal�� ������ �ʴ´�.
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
// 		::XsigQueueSignal(GetThreadPool(), pChannel, CHANNEL_USERLINKCUT_SIG, (WPARAM)(msg.m_lGSN), 0); // GSN���� �ȵ�..
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
// 		//		::XsigQueueSignal(GetThreadPool(), pRoom, ROOM_USERLINKCUTTING, (WPARAM)(msg.m_lGSN), 0); // GSN���� �ȵ�..
// 		::XsigQueueSignal(GetThreadPool(), pRoom, (HSIGNAL)ROOM_USERLINKCUT, (WPARAM)(msg.m_lGSN), 0); // GSN���� �ȵ�..
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
// 	// NLS IP List�� ����..
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
// #else  //_CHSNLS . �����ǸŰ��� ���� ó���� �Ѵ�.
// 
// 	// msg.m_lReserved != -1 �� ��� ����� ���� �����̱� ������ ����ó�� �����ϰ� �Ѵ�.
// 	if (msg.m_lReserved != -1)
// 		return;
// 
// 	// ä���� ã�´�.
// 	ChannelID cid(msg.m_RoomID.m_lSSN, msg.m_RoomID.m_dwCategory, msg.m_RoomID.m_dwGCIID);
// 	CChannel * pChannel = NULL;
// 	theChannelDir.GetChannel(cid, &pChannel);
// 	if(!pChannel)
// 	{
// 		theLog.Put(ERR_UK, "CHS_NLSManagerErr, Not found channel .. OnQueryUserStateReqFromNLS FromNLS : SSN/Category/GCIID = [",
// 			msg.m_RoomID.m_lSSN, "/", msg.m_RoomID.m_dwCategory, "/", msg.m_RoomID.m_dwGCIID, "]");
// 		pld.un.m_msgQueryUserStateAns->m_lResult = 2L; // �ش� ����� �������� ������.
// 		SendNLSMessage(pld, des);
// 		return;
// 	}
// 	if (pChannel->IsUserConnected(msg.m_lGSN))
// 	{	
// 		pld.un.m_msgQueryUserStateAns->m_lResult = 1;
// 
// 		// ����ڰ� �ִٰ� ���� ��� ����� �α�
// 		LONG lSize = m_mapConnectUser.size();
// 		if (m_mapConnectUser.end() == m_mapConnectUser.find(msg.m_lGSN))
// 			LOG(ERR_UK, "CHS_NLSErr.OnQueryUserStateReqFromNLS(). mismatch case 1 m_mapConnectUser.size()=", lSize, ", GSN:", msg.m_lGSN);
// 	}
// 	else 
// 	{ 
// 		pld.un.m_msgQueryUserStateAns->m_lResult = 2;		
// 		// ����ڰ� ���ٰ� ���� ��� ����� �α�
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
