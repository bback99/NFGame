//
// GRCContainerADL.cpp
//

#include "stdafx.h"
#include "Room.h"
#include "RoomInternalLogic.h"
#include "RoomTable.h"
#include "StatisticsTable.h"
#include "UserTable.h"
#include "Agent.h"
#include "NotifyToOthers.h"
#include <NLSManager.h>
#include <NFVariant/NFDBManager.h>

extern void md5hash_say(char* tgt,const char* src);

#define ROOM_MD5_HEADER "n!E!o!W!i!Z"
#define ROOM_MD5_HEADER_SIZE 11
#define ROOM_MD5_PWD	"p!M!a!!!N!g"

#define ADMIN_USERID_HEADER "pmang_"

#define ERROR_BLACKLIST			-2

#include "ErrorLog.h"

///////////////////////////////////////////////////////////////////////////////////
// CRoomInternalLogic
BOOL CRoomInternalLogic::OnCreateRoomReq(CUser* pUser)
{
	LONG lCSN = pUser->GetCSN();
	CUser* pPrevUser = UserManager->FindUser(lCSN);
	if (pPrevUser)
	{
		theLog.Put(ERR_UK, "NGS_Not_Null"_COMMA, "pPrevUser USN: ", lCSN, " is not NULL in OnCreateRoomReq. RoomID:", GetRoomIDStr());
		KickOutUser( lCSN, TRUE );
	}

	if ( !UserManager->AddUser( pUser ) )
	{
		pUser->SetErrorCode(CRF_SYSTEM);
		theLog.Put(WAR_UK, "NGS_GRCContainer_Error"_COMMA, "AddUser CSN: ", lCSN, " Failure in OnCreateRoomReq");
		return FALSE;
	}

	// NF���ӿ��� �ʿ�� �ϴ� UserSlot �߰�
	if (!AddUserSlot(pUser))
	{
		pUser->SetErrorCode(CRF_SYSTEM);
		theLog.Put(WAR_UK, "NGS_GRCContainer_Error"_COMMA, "AddUserSlot CSN: ", lCSN, " Failure in OnCreateRoomReq");
		return FALSE;
	}

	m_lCapUSN = lCSN;
	m_sCapUID.assign(pUser->GetUserID().c_str(), pUser->GetUserID().length());

	{// NLS�� RoomID ������Ʈ
		RoomID roomID;
		pUser->NLSGetRoomID(roomID);
		TKey key(pUser->GetUSN(), pUser->GetCSN());
		theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_NFGAMESERVER, roomID, pUser->GetLevel(), pUser->GetNFRoomOption().m_lPlayType);
	}

	NFCharInfo* info = pUser->GetNFCharInfoExt();
	PayloadNGSCli pld(PayloadNGSCli::msgCreateRoomNtf_Tag);
	pld.un.m_msgCreateRoomNtf->Clear();
	pld.un.m_msgCreateRoomNtf->m_lErrorCode = CRF_SUCCESS;
	pld.un.m_msgCreateRoomNtf->m_nfCharInfo.BCopy(*info);

	// �κ��� �� �ְ� PartsIndex�� ã�Ƽ� �����;;
	// UserSlot : UI�����δ� 1���� �����̹Ƿ� ���� ���� ����, +1 ���ؼ� �����ش�.(���α׷������� 0���� ����) 
	pld.un.m_msgCreateRoomNtf->m_lUserSlot = pUser->GetUserSlot();
	pld.un.m_msgCreateRoomNtf->m_lUserStatus = pUser->GetUserStatus();

	RoomID& roomID = m_RoomID;
	const NSAP& nsap = theRoomTable.GetNSAP();
	pld.un.m_msgCreateRoomNtf->m_nfRoomBaseInfo.m_dwGRIID = roomID.m_dwGRIID;
	pld.un.m_msgCreateRoomNtf->m_nfRoomBaseInfo.m_nsapGLS = nsap;
	pld.un.m_msgCreateRoomNtf->m_nfRoomBaseInfo.m_lRoomState = GetRunStopFlag();
	pld.un.m_msgCreateRoomNtf->m_nfRoomBaseInfo.m_lRoomType = 0L;

	pld.un.m_msgCreateRoomNtf->m_nfRoomBaseInfo.m_roomOption.BCopy(GetNFRoomOption());
	// ���� �Ű� ���� ����

	pld.un.m_msgCreateRoomNtf->m_nfRoomBaseInfo.m_sGameOption.assign(m_sGameOption.c_str(), m_sGameOption.length());
	pld.un.m_msgCreateRoomNtf->m_nfRoomBaseInfo.m_sReserve1.erase();

	// Ŭ���̾�Ʈ�� ����ȭ ��Ű�� ���ؼ� server�� ���� �ð��� ������. - 2011/2/22
//	pld.un.m_msgCreateRoomNtf->m_dwServerTime = ::timeGetTime();

	UserManager->SendToUser(lCSN, pld);
	return TRUE;

}



BOOL CheckMD5Password( long lCSN, DWORD GRIID, string & UserPassword  )
{
	if ( UserPassword.size() > ROOM_MD5_HEADER_SIZE && UserPassword.substr(0, ROOM_MD5_HEADER_SIZE) == ROOM_MD5_HEADER )
	{
		string sHashedFingerprint = UserPassword.substr(ROOM_MD5_HEADER_SIZE, UserPassword.size() - ROOM_MD5_HEADER_SIZE);
		char sEncData[1024] = {0x00};
		sprintf(sEncData, "%d%d%s", lCSN, GRIID, ROOM_MD5_PWD);
		char sResult[1024] = {0x00};		
		md5hash_say(sResult, sEncData);

		if ( 0 ==strcmp(sHashedFingerprint.c_str(), sResult) )
			return true;
		else
			theLog.Put(WAR_UK, "NGS_GRCContainer_Error"_COMMA, "Invalid Password of CSN: ", lCSN, " in OnJoinRoomReq");
	}
	return false;
}

BOOL CRoomInternalLogic::PasswordCheck( CUser * pUser )
{
	xstring strPw = pUser->GetPassword();


	// Admin/Ghost(ID: "pmang_*") should be able to join in private room
	if (0 == ::strcmp(strPw.c_str(), m_nfRoomOption.m_sPassword.c_str()) )
		return true;
	else if (CheckMD5Password( pUser->GetUSN(), pUser->GetRoomID().m_dwGRIID,strPw ) )
		return true;
	else if( pUser->GetUserID().find(_X(ADMIN_USERID_HEADER)) != xstring::npos )	// Admin/Ghost(ID: "pmang_*") should be able to join in private room
		return true;

	return false;
}

BOOL CRoomInternalLogic::IsExistPrevUser(LONG lCSN)
{
	CUser* pPrevUser = UserManager->FindUser(lCSN);

	if( pPrevUser )
	{
		theLog.Put(ERR_UK, "NGS_Not_Null"_COMMA, "pPrevUser CSN: ", lCSN, " is not NULL in OnJoinRoomReq. RoomID:", GetRoomIDStr());
		return TRUE;
	}
	else 
		return FALSE;
}

BOOL CRoomInternalLogic::IsOverMaxUserCnt()
{
	return UserManager->GetUserCnt() >= (UINT)m_nfRoomOption.m_lMaxUserCnt;
}

BOOL CRoomInternalLogic::PasswordCheckOnJoin( CUser * pUser )
{
	if (m_nfRoomOption.m_sPassword.length()) 
	{
		if( false ==PasswordCheck( pUser ) )
		{	
			// Let user enter room to prevent from abusing(i.e do not return FALSE;), and logging 
			if (pUser->GetPassword().length() == 0)
			{
				theLog.Put(INF_UK, "NGS_Auth, Password Overwriting(NULL Password) user, USN: ", pUser->GetUSN());
			}
			else
			{
				theLog.Put(INF_UK, "NGS_Auth, Password Overwriting user, USN: ", pUser->GetUSN());
			}
			return FALSE;
			//Check the reason why password checking failed and logging

		}
		else// User matched lCSN is in room aleady, but password is wrong
		{

		}
	}
	return TRUE;
}


BOOL CRoomInternalLogic::OnJoinRoomReqSuccess( CUser * pUser )
{
	if ( FALSE == UserManager->AddUser(pUser) ) {
		pUser->SetErrorCode(JRF_SYSTEM);
		theLog.Put(WAR_UK, "NGS_CRoomInternalLogic_Error"_COMMA, "AddUser USN: ", pUser->GetUSN(), " Failure in OnJoinRoomReq");
		return FALSE;
	}

// NF
	pUser->SetRoomOption(m_nfRoomOption.m_lCreatorUSN, m_nfRoomOption.m_sCreatorID, GetNFRoomOption());		// RoomOption �����ϸ鼭 ������ ������ �Ѱ��ش�.

	// ������ ���, Room_Status üũ
	if (ROOMSTATE_RUN == GetRunStopFlag())				// �Ϲ������� ������ ���, ULS_ROOMLOBBY ����
		pUser->SetUserLocation(ULS_ROOMLOBBY);			
	else if (ROOMSTATE_START == GetRunStopFlag())		// �÷��� �߿� �������� ���, 
		pUser->SetUserLocation(ULS_INROOM);			
	else {
		PayloadNGSCli pldAll(PayloadNGSCli::msgJoinRoomAns_Tag);
		pldAll.un.m_msgJoinRoomAns->Clear();
		pldAll.un.m_msgJoinRoomAns->m_lErrorCode = JRF_INVALIDSTATE;
		UserManager->SendToUser(pUser->GetCSN(), pldAll);
		return FALSE;
	}

	// ������ ���, ���� ��ȸ���̰� ���� �ð��� 2�� ���Ϸ� ������ ���, ���� �Ұ�
	if (GetRunStopFlag() == ROOMSTATE_START && m_nfRoomOption.m_lRemainTime <= 2 * 60) {
		pUser->SetUserLocation(ULS_INVALID);	
		PayloadNGSCli pldAll(PayloadNGSCli::msgJoinRoomAns_Tag);
		pldAll.un.m_msgJoinRoomAns->Clear();
		pldAll.un.m_msgJoinRoomAns->m_lErrorCode = JRF_REMAINTIME;
		UserManager->SendToUser(pUser->GetCSN(), pldAll);
		return FALSE;
	}

	// NF���ӿ��� �ʿ�� �ϴ� UserSlot �߰�
	if (!AddUserSlot(pUser)) {
		theLog.Put(WAR_UK, "GLS_GRCContainer_Error"_COMMA, "AddUserSlot CSN: ", pUser->GetCSN(), " Failure in OnJoinRoomReq");
		pUser->SetErrorCode(JRF_SYSTEM);
		return FALSE;
	}
// NF

	PayloadNGSCli pldAll(PayloadNGSCli::msgJoinRoomAns_Tag);
	pldAll.un.m_msgJoinRoomAns->Clear();
	pldAll.un.m_msgJoinRoomAns->m_lErrorCode = CRF_SUCCESS;

	UserManager->GetAllUserBaseInfo( pldAll.un.m_msgJoinRoomAns->m_lstNFJoinUBI );

	RoomID& roomID = m_RoomID;
	const NSAP& nsap = theRoomTable.GetNSAP();
	pldAll.un.m_msgJoinRoomAns->m_roomInfo.m_dwGRIID = roomID.m_dwGRIID;
	pldAll.un.m_msgJoinRoomAns->m_roomInfo.m_nsapGLS = nsap;
	pldAll.un.m_msgJoinRoomAns->m_roomInfo.m_lRoomState = GetRunStopFlag();
	pldAll.un.m_msgJoinRoomAns->m_roomInfo.m_lRoomType = 0L;

	pldAll.un.m_msgJoinRoomAns->m_roomInfo.m_roomOption.BCopy(GetNFRoomOption());


	pldAll.un.m_msgJoinRoomAns->m_roomInfo.m_sGameOption.assign(m_sGameOption.c_str(), m_sGameOption.length());
	pldAll.un.m_msgJoinRoomAns->m_roomInfo.m_sReserve1.erase();

	UserManager->SendToUser(pUser->GetCSN(), pldAll);

	return TRUE;
}

BOOL CRoomInternalLogic::OnJoinRoomNtfSuccess( LONG lCSN  )
{
	CUser * pUser = UserManager->FindUser( lCSN );
	if( !pUser )
		return FALSE;

	pUser->SetValid(TRUE);

	UserStatInfo usi;
	usi.Copy(pUser->GetNFUser().GetNFChar().m_nfUserBaseInfo);
#ifdef __PMS_DEBUG__
	theLog.Put(INF_UK, "NGS_STAT_DEBUG, SetStatInfoCaller: OnRcvJoinRoomNtf, CSN: ", lCSN);
#endif

	theStatTable.SetStatInfo(pUser->GetRoomID(), usi, 1, 4);

	if (usi.m_lIsPCRoomUser != 0)
	{
		theStatTable.SetStatInfoPC(pUser->GetRoomID(), usi, 1, 4);
	}

	theUserTable.Add();

// NF
	NotifyUtil->NotifyAddUserToChs(pUser, lCSN);

//	// GLS_COMMON�� �����ϴ� UserJoinNtf�� ������ �ʱ⿡ �Ʒ� �κ� �߰� - NF���� ���
//	PayloadNGSCli pld(PayloadNGSCli::msgUserJoinNtf_Tag);
//	pld.un.m_msgUserJoinNtf->Clear();
//	pld.un.m_msgUserJoinNtf->m_lErrorCode = JRF_SUCCESS;
//	pld.un.m_msgUserJoinNtf->m_nfJoinUBI.m_nfUserBaseInfo = pUser->GetNFUser().GetNFChar().m_nfUserBaseInfo;
//	pld.un.m_msgUserJoinNtf->m_nfJoinUBI.m_nfCharBaseInfo = pUser->GetNFCharInfoExt().m_nfCharBaseInfo;
//	pld.un.m_msgUserJoinNtf->m_nfJoinUBI.m_lUserSlot = pUser->GetUserSlot();
//	pld.un.m_msgUserJoinNtf->m_nfJoinUBI.m_lUserStatus = pUser->GetUserStatus();
//	UserManager->SendToAllUser(pld);
// NF

	return TRUE;
}

BOOL CRoomInternalLogic::ProcessJoinGRCFailure( LONG lCSN, LONG JoinResult )
{	

	CUser * pUser = UserManager->FindUser( lCSN );


	UserStatInfo usi;
	usi.Copy(pUser->GetUserData());

	theLog.Put(DEV_UK, "ProcessJoinGRCFailure, ReqRemUser : CSN = ", lCSN );
	
	TKey key(pUser->GetUSN(), pUser->GetCSN());
	theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, pUser->GetRoomID(), pUser->GetLevel());


	// In case of Ntf flooding attack
	if (-3 == JoinResult)
	{
		theLog.Put(ERR_UK, "NGS_MSG_FLOODING, Message flooding in OnRcvCreateRoomNtf, CSN: ", lCSN);
		theStatTable.SetStatInfo(m_RoomID, usi, -1, 3);

		if (usi.m_lIsPCRoomUser != 0)
		{
			theStatTable.SetStatInfoPC( m_RoomID, usi, -1, 3);
		}
	}

	theLog.Put( INF_UK, "NGS_JoinRoom_Error : IRoomContext::OnAddUserInRoom Failed.  CSN :", lCSN, " RoomID :", RoomID2Str( m_RoomID ), " Error : ", JoinResult );

	NotifyUtil->NotifyNGSInfoToNCS();

	return TRUE;
}



BOOL CRoomInternalLogic::OnCreateRoomNtfSuccess( LONG lCSN )
{
	CUser * pUser = UserManager->FindUser( lCSN );

	pUser->SetValid(TRUE);

	NotifyUtil->NotifyCreateRoomToChs(pUser, GetNFRoomOption(), m_sGameOption );

	UserStatInfo usi;
	usi.Copy(pUser->GetUserData());
#ifdef __PMS_DEBUG__
	theLog.Put(INF_UK, "NGS_STAT_DEBUG, SetStatInfoCaller: OnRcvCreateRoomNtf, lCSN: ", lCSN);
#endif
	theStatTable.SetStatInfo(pUser->GetRoomID(), usi, 1, 2);

	if (usi.m_lIsPCRoomUser != 0)
	{
		theStatTable.SetStatInfoPC(pUser->GetRoomID(), usi, 1, 2);
	}

	theUserTable.Add();

// NF
	NotifyUtil->NotifyAddUserToChs(pUser, lCSN);
// NF

	if (m_nfRoomOption.m_lPlayType == PT_FREE)		// 
	{
		// TotalMap �ʱ�ȭ
		InitTotalMap(m_nfRoomOption.m_lIdxFishMap);
		// ¡�� ����Ʈ �ʱ�ȭ
		SetSignPoint(m_nfRoomOption.m_lIdxFishMap);

		NotifyUtil->NotifyChangeRoomStateToChs(ROOMSTATE_READY);
	}

	return TRUE;
}


BOOL CRoomInternalLogic::SendJoinNtfResultToAll( LONG lCSN )
{
	CUser * pUser = UserManager->FindUser( lCSN );

	NFUserBaseInfo& nfUBI = pUser->GetNFUser().GetNFChar().m_nfUserBaseInfo;
	NFCharBaseInfo& nfCBI = pUser->GetNFCharInfoExt()->m_nfCharBaseInfo;
	NFJoinUserBaseInfo	info2;
	info2.Clear();
	info2.m_nfUserBaseInfo = nfUBI;
	info2.m_nfCharBaseInfo = nfCBI;
	info2.m_lUserSlot = pUser->GetUserSlot();
	info2.m_lUserStatus = pUser->GetUserStatus();

	PayloadNGSCli pld(PayloadNGSCli::msgUserJoinNtf_Tag);
	pld.un.m_msgUserJoinNtf->Clear();
	pld.un.m_msgUserJoinNtf->m_lErrorCode = JRF_SUCCESS;
	pld.un.m_msgUserJoinNtf->m_nfJoinUBI.BCopy(info2);

	UserManager->SendToAllUser(pld);

	return TRUE;
}

BOOL CRoomInternalLogic::OnBroadCastMsg( const GBuf & buf )
{
	PayloadNGSCli pld;
	if( ::LLoad( pld, buf) )
	{
		UserManager->SendToAllUser( pld );
		return true;
	}
	else
	{
		theLog.Put( ERR_UK, "NGS_General_Error, BroadcastMessage Load Fail" );
	}
	return false;
}

BOOL CRoomInternalLogic::SendIBBAnswerToUser( LONG lCSN, LPXBUF xbuf )
{
	CUser* pUser = UserManager->FindUser(lCSN);
	if (pUser)
	{
	}
	return false;
}

BOOL CRoomInternalLogic::SendAccuseAnsToUser( LONG lCSN, CAccuseResultAnswer & AccuseResult )
{
	if( NULL ==  UserManager->FindUser( lCSN ) )
	{
		theLog.Put(WAR_UK, "NGS_Accusing_Error"_COMMA, "pUser or pUser->GetLink is NULL in SendAccuseAnsToUser");
		return false;
	}
	PayloadNGSCli pld(PayloadNGSCli::msgAccuseAns_Tag, \
		MsgNGSCli_AccuseAns(AccuseResult.GetAccusedID().c_str(), AccuseResult.GetErrorCode()) );
	/*	GBuf buf;
	if(!::LStore(buf, pld)) return;

	SendToLink(pUser->GetLink(), buf);
	*/
	UserManager->SendToUser( lCSN, pld );

	return true;
}

BOOL CRoomInternalLogic::CheckPldCreateRoomNtf( LONG lCSN, PayloadCliNGS * pPld )
{
	CUser * pUser = UserManager->FindUser( lCSN );
	if( pUser )
	{
		if( pUser->IsValid() )	//If this user(link) has sent CreateRoomNtf/JoinRoomNtf already, ignore this ntf.
		{
			theLog.Put(ERR_UK, "NGS_Auth, Invalid Create sequence: Multiple CreateRoomNtf msg from user, CSN: ", lCSN);
			return FALSE;
		}
	}

	MsgCliNGS_CreateRoomNtf* pMsg = pPld->un.m_msgCreateRoomNtf;

	if (pMsg->m_lErrorCode != CG_CRF_SUCCESS) { // Create Room Failed.
		theLog.Put(ERR_UK, "NGS_Auth, Invalid Create sequence: ", pMsg->m_lErrorCode, " CSN: ", lCSN);
		theLog.Put(ERR_UK, "NGS_Null,Create Room Failed.  pMsg->m_lErrorCode != CG_CRF_SUCCESS. roomID:", GetRoomIDStr());

		//OnUserDisconnect(usn);			
		KickOutUser( lCSN  );

		return FALSE;
	}

	return TRUE;
}

BOOL CRoomInternalLogic::CheckPldJoinRoomNtf( LONG lCSN, PayloadCliNGS * pPld )
{
	CUser * pUser = UserManager->FindUser( lCSN );
	if( pUser )
	{
		if( pUser->IsValid() )	//If this user(link) has sent CreateRoomNtf/JoinRoomNtf already, ignore this ntf.
		{
			theLog.Put(ERR_UK, "NGS_Auth, Invalid Create sequence: Multiple JoinRoomNtf msg from user, CSN: ", lCSN);
			return FALSE;
		}
	}

	MsgCliNGS_JoinRoomNtf* pMsg = pPld->un.m_msgJoinRoomNtf;

	if (pMsg->m_lErrorCode != CG_JRF_SUCCESS) { // Join Room Failed.
		theLog.Put(ERR_UK, "NGS_Auth, Invalid Join sequence: ", pMsg->m_lErrorCode, " CSN: ", lCSN);
		theLog.Put(ERR_UK, "NGS_Null,Join Room Failed. pUser->GetLink() is null. pMsg->m_lErrorCode != CG_CRF_SUCCESS. roomID:", GetRoomIDStr());
//		OnUserDisconnect(usn);
		KickOutUser( lCSN  );

		return FALSE;
	}

	return TRUE;
}

bool CRoomInternalLogic::ProcessExitedUser( LONG lCSN, BOOL bPrevious )
{
	CAutoLockCheck lc("CRoomInternalLogic::OnUserDisconnect", &m_lock_this_debug);

	theUserTable.Remove();

	CUser* pUser = UserManager->FindUser(lCSN);
	if(!pUser)
		return false;

	// ���� ���� ����ÿ� LogOutDate ������Ʈ
	// - �Ⱓ�� ������ �����ÿ� �� �ؾ� �ϹǷ�...
	// - �˴ٿ������� �����ؾ� �ϹǷ�...
	
	if (!theNFDBMgr.UpdateLastLogOutDate(pUser->GetGSN(), pUser->GetCSN()))
		theLog.Put(ERR_UK, "NGS_Null, ProcessExitedUser UpdateLastLogOutDate CSN :", lCSN);

	NotifyUtil->NotifyRemoveUserToChs( lCSN );

	// NF
	// NLS�� ������ ������ �����ٰ� ���¸� ������Ʈ �Ѵ�.
	RoomID roomId;
	roomId.Clear();
	pUser->SetRoomID(roomId);
	TKey key(pUser->GetUSN(), pUser->GetCSN());
	theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, pUser->GetRoomID(), pUser->GetLevel());

	// ��� �������� ���� ����
	// ��� �������� ���� ����
	if (pUser->IsValid()) 
	{
		pUser->SetValid(FALSE);
		CAutoLockCheck lc("CRoomInternalLogic::OnUserDisconnect", &m_lock_cu_debug);
		UserStatInfo info;
		info.Copy(pUser->GetUserData());
		theStatTable.SetStatInfo(m_RoomID, info, -1, 1);

		if (info.m_lIsPCRoomUser != 0) 
		{
			theStatTable.SetStatInfoPC(m_RoomID, info, -1, 1);
		}
	}

	// NF
	LONG lBoatOwnerCSN = 0;
	LONG lMultiPortIndex = 0;
	// �ǽ�����Ʈ���� ����...
	FishingPoint* pFP = theNFDataItemMgr.GetFishingPointByIndex(m_nfRoomOption.m_lIdxFishMap, pUser->GetPrevFishingPoint());
	if (pFP)
		ChangeFishingPoint(pFP, pUser->GetPrevFishingPoint(), pUser, lBoatOwnerCSN, lMultiPortIndex, TRUE);


	// update context
	RemoveUserSlot(pUser);

	//////////////////////////////////////////////////////////////////////////
	// Loading�ϴٰ� ��������, �ش� ���� ���� �ٽ� üũ�Ѵ�.
	SendNtfGameStart();

	// 2010/7/12 bback99
	// �������� üũ�Ͽ�, �����̸� ������ �ΰ��Ѵ�.
	if (m_nfRoomOption.m_lCapUSN == lCSN) {
		// ���� ������ ã�´�.
		// @@ NF renewal
		CUser* pCapUser = UserManager->GetNextCap(lCSN);
		if (pCapUser) {
			m_nfRoomOption.m_lCapUSN = pCapUser->GetCSN();
			m_sCapUID.assign(pCapUser->GetUserID().c_str(), pCapUser->GetUserID().length());

			NotifyUtil->NotifyChangeRoomOptionToChs(GetNFRoomOption());
			// ������ ����Ǹ� RoomOptionChangeNtf�� ������� ������.
			NotifyChangeRoomOptionToClient();
		}
	}
	// NF

	NotifyUtil->NotifyNGSInfoToNCS();

	PayloadNGSCli pld(PayloadNGSCli::msgUserDisconnectNtf_Tag);
	pld.un.m_msgUserDisconnectNtf->Clear();
	pld.un.m_msgUserDisconnectNtf->m_lCSN = lCSN;
	UserManager->SendToAllUsersExceptOne(lCSN, pld);



	UserManager->DestroyUser( lCSN );
	//������ ������� ��� �� ������ CHS���� ���� �޼����� MsgGLSCHS_RoomDeleteNtf 
	//	if (!bPrevious && (mUsers.size() == 0) && !m_IsEternityRoom)
	if (!bPrevious && (UserManager->GetUserCnt() == 0) )
	{

		if ( GetState() == ROOMSTATE_DEAD) {

			theRoomTable.RemoveRoom(m_RoomID);
		}
	}
	return true;
}

BOOL CRoomInternalLogic::KickOutUser(LONG lCSN, BOOL bPrevious )
{
	BOOL ret = UserManager->CutUser( lCSN );
	ProcessExitedUser(lCSN, bPrevious );
	return ret;
}

void CRoomInternalLogic::OnRcvChangeRoomOptionNtf( LONG usn, MsgCliNGS_ChangeRoomOptionNtf* pMsg)
{
	CUser * pUser = UserManager->FindUser( usn );

	if (!pUser) return;
	if (!pMsg) return;

	LONG	lMaxUserCnt		= m_nfRoomOption.m_lMaxUserCnt;
	LONG	lPrevCreatorUSN	= m_nfRoomOption.m_lCreatorUSN;
	xstring sPrevCreatorID	= m_nfRoomOption.m_sCreatorID;
	xstring	sPrevRoomTitle	= m_nfRoomOption.m_sRoomTitle;

	pMsg->m_nfRoomOption.m_lRoomStatus = m_nfRoomOption.m_lRoomStatus;
	pMsg->m_nfRoomOption.m_lRemainTime = m_nfRoomOption.m_lRemainTime;
	pMsg->m_nfRoomOption.m_lEnvAttribute = m_nfRoomOption.m_lEnvAttribute;
	m_nfRoomOption.BCopy(pMsg->m_nfRoomOption);
	// ���� �Ű� ���� �߰�  - client�� ������ ���ϱ� ���� ����
	//m_nfRoomOption.m_lCreatorUSN = pUser->GetUSN();//pMsg->m_nfRoomOption.m_lCreatorUSN;
	//m_nfRoomOption.m_sCreatorID = pUser->GetUserID().c_str();//pMsg->m_nfRoomOption.m_sCreatorID.c_str();
	
	// �� �ο��� ���¡ ���� ���� 1�� ���Ϸ� ������ ��� ���� MaxUser ������ �ٽ� ������ ����. �� MaxUser �� Ŭ���̾�Ʈ���� �������� ���ϰ� ��.
	if (m_nfRoomOption.m_lMaxUserCnt <= 1)
		m_nfRoomOption.m_lMaxUserCnt = lMaxUserCnt;

	// �� Ÿ��Ʋ�� ������� �ʾ�����, m_lCreatorUSN�� m_sCreatorID�� �������� ����
	if (m_nfRoomOption.m_sRoomTitle.compare(sPrevRoomTitle.c_str()) == 0)
	{
		m_nfRoomOption.m_lCreatorUSN = lPrevCreatorUSN;
		m_nfRoomOption.m_sCreatorID  = sPrevCreatorID;
	}
	else
	{
		m_nfRoomOption.m_lCreatorUSN = pUser->GetUSN();
		m_nfRoomOption.m_sCreatorID  = pUser->GetUserID();
	}

	NotifyUtil->NotifyChangeRoomOptionToChs( GetNFRoomOption() );

	NotifyChangeRoomOptionToClient();
}

void CRoomInternalLogic::OnRcvAlive( LONG usn)
{
	CUser * pUser = UserManager->FindUser( usn );
	ASSERT( pUser );
	if (!pUser) return;

	pUser->SetState(US_ALIVE);
}

void CRoomInternalLogic::OnRcvChatMsgTo( LONG Senderusn, MsgCliNGS_ChatMsgTo* pMsg)
{
	CUser * pUser = UserManager->FindUser( Senderusn );
	if (!pUser) return;
	if (!pMsg) return;

	long SenderCSN = pUser->GetCSN();
	long RecverCSN = pMsg->m_lToCSN;

	CUser* pToUser = UserManager->FindUser(RecverCSN);
	if (!pToUser) return;

	// �Ű�
	xstring& sUID = pUser->GetUserID();
	xstring& sNick = pUser->GetUserData().m_sNickName;
	xstring& sUIDTo = pToUser->GetUserID();



	m_ChatHistory.AddNewMsg( SenderCSN, RecverCSN, sUID, sNick, sUIDTo, pMsg->m_sText);

	PayloadNGSCli pld(PayloadNGSCli::msgChatMsgTo_Tag);
	pld.un.m_msgChatMsgTo->Clear();
	pld.un.m_msgChatMsgTo->m_lFromCSN = SenderCSN;
	pld.un.m_msgChatMsgTo->m_lToCSN = RecverCSN;
	pld.un.m_msgChatMsgTo->m_sText.assign(pMsg->m_sText.c_str(), pMsg->m_sText.length());
	pld.un.m_msgChatMsgTo->m_dwColor = pMsg->m_dwColor;
	pld.un.m_msgChatMsgTo->m_lFontSize = pMsg->m_lFontSize;

	UserManager->SendToUser( RecverCSN, pld );
	if (CheckChatForward())
	{
	}
}

void CRoomInternalLogic::OnRcvChatMsgAll( LONG Senderusn, MsgCliNGS_ChatMsgAll* pMsg)
{
	CUser * pUser = UserManager->FindUser( Senderusn );
	if (!pUser) return;
	if (!pMsg) return;

	// �Ű�
	xstring& sUID = pUser->GetUserID();
	xstring& sNick = pUser->GetUserData().m_sNickName;

	xstring temp = _X(" ");

	m_ChatHistory.AddNewMsg(pUser->GetUSN(), -1L, sUID, sNick, temp, pMsg->m_sText);

	PayloadNGSCli pld(PayloadNGSCli::msgChatMsgAll_Tag);
	pld.un.m_msgChatMsgAll->Clear();
	pld.un.m_msgChatMsgAll->m_lFromCSN = pUser->GetCSN();
	pld.un.m_msgChatMsgAll->m_sText.assign(pMsg->m_sText.c_str(), pMsg->m_sText.length());
	pld.un.m_msgChatMsgAll->m_dwColor = pMsg->m_dwColor;
	pld.un.m_msgChatMsgAll->m_lFontSize = pMsg->m_lFontSize;

	UserManager->SendToAllUser(pld);
	if (CheckChatForward())
	{
	}
}

// �Ű�
void CRoomInternalLogic::OnRcvAccuseReq( LONG usn, MsgCliNGS_AccuseReq* pMsg)
{
	CUser * pUser = UserManager->FindUser( usn );

	if( !pUser )
		return;
	LONG lECode = 0L;
	AccuseBus bus;
	bus.m_sType = pMsg->m_sType;
	bus.m_sAccusedID = pMsg->m_sAccusedID;
	bus.m_lAccusedUSN = pMsg->m_lAccusedUSN;
	bus.m_bRoomTitleAccuse = FALSE;

	if (pMsg->m_sType == "a2")
	{
		bus.m_lUSN = pUser->GetUSN();
		bus.m_sUserID = pUser->GetUserID();
		bus.m_bRoomTitleAccuse = TRUE;
		SYSTEMTIME systime;
		::GetLocalTime(&systime);
		bus.m_sDate = ::format("%04d-%02d-%02d %02d:%02d:%02d ", 
			systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

		bus.m_sReason = pMsg->m_sReason;
		bus.m_sContent = pMsg->m_sContent;
		bus.m_sContent1 = pMsg->m_sContent;
		time_t _lt;
		::time(&_lt);
		lECode = theAccuseAgent.PostAccusationMsg(bus, (LONG)_lt, m_RoomID);

		theLog.Put(DEV_UK, "AccuseMsg",
			", Type: ", bus.m_sType, 
			", AccusedID: ", bus.m_sAccusedID,
			", AccusedUSN: ", bus.m_lAccusedUSN, 
			", AccuserUSN: ", bus.m_lUSN,
			", Reason: ", bus.m_sReason,
			", Content: ", bus.m_sContent,
			", Content1: ", bus.m_sContent1);
	}
	else
	{
		xstring _temp;
		LONG len = m_ChatHistory.GetHistory(pUser->GetUSN(), bus.m_lAccusedUSN, _temp);
		if(len < 1) 
		{
			lECode = ERRAGENT_NOCHATDATA;
			PayloadNGSCli pld(PayloadNGSCli::msgAccuseAns_Tag, MsgNGSCli_AccuseAns(bus.m_sAccusedID.c_str(), lECode));
			UserManager->SendToUser(usn, pld);
		}
		else
		{
			SYSTEMTIME systime;
			::GetLocalTime(&systime);
			bus.m_sDate = ::format("%4d-%02d-%02d %02d:%02d:%02d ", 
						systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);


			bus.m_sReason = pMsg->m_sReason;

			bus.m_sContent = _X("�Ű� ���� : ") + pMsg->m_sContent + _X("\n\n") + _temp;

			bus.m_sUserID = pUser->GetUserID();
			bus.m_lUSN = pUser->GetUSN();
			MakeContentHeader(bus, pMsg->m_sGameName);

			string sContent1;
			if(pMsg->m_sType == "g2")
				bus.m_sContent1 =  m_nfRoomOption.m_sRoomTitle + _X(" : ") + pMsg->m_sContent;

			else if(pMsg->m_sType == "g1" || pMsg->m_sType == "k1")
				bus.m_sContent1 = m_nfRoomOption.m_sRoomTitle;
			else 
			{
				LOG( ERR_UK, " CRoomInternalLogic::OnRcvAccuseReq, Invalid msgType " );
				return;
			}

			time_t _lt;
			::time(&_lt);
			lECode = theAccuseAgent.PostAccusationMsg(bus, (LONG)_lt, m_RoomID);
			// client�� ������ ������. �׷��� ���⼭�� ������ �Ű� DB�� FileServe�� ������ ��ϵǾ����� �������� �ʴ´�.
			// ����, �ߺ� ���� ���� �Ű��̸�, Agent�� �Ű� process�� �����϶�� �޼����� ������������ �����Ѵ�.

			theLog.Put(DEV_UK, "AccuseMsg",
				", Type: ", bus.m_sType, 
				", AccusedID: ", bus.m_sAccusedID,
				", AccusedUSN: ", bus.m_lAccusedUSN, 
				", AccuserUSN: ", bus.m_lUSN,
				", Reason: ", bus.m_sReason,
				", Content: ", bus.m_sContent,
				", Content1: ", bus.m_sContent1);
		}
	}

	PayloadNGSCli pld(PayloadNGSCli::msgAccuseAns_Tag, MsgNGSCli_AccuseAns(bus.m_sAccusedID.c_str(), lECode));
	UserManager->SendToUser(usn, pld);
}

// �Ű�
void CRoomInternalLogic::MakeContentHeader(AccuseBus& bus, string& game_name)
{
	xstring token(_X(" \r\n"));
	xstring temp_content;
	temp_content = _X("[��ġ ����] \r\n") + str2xstr(game_name) + token + GetNFRoomOption().m_sRoomTitle +
		token + token + bus.m_sContent + token;

	bus.m_sContent = temp_content;
}

// ����Ŭ�� ���� ������ ���
void CRoomInternalLogic::OnRcvLoginStateChangeNtf( LONG usn, MsgCliNGS_LoginStateChangeNtf* pMsg)
{
	CUser * pUser = UserManager->FindUser( usn );

	if (!pUser) return;
	if (!pMsg) return;

	pUser->GetUserData().m_lLoginState = pMsg->m_lLoginState;
	NotifyUtil->NotifyChangeLoginStateToChs(pUser->GetUSN(), pMsg->m_lLoginState);
	NotifyChangeLoginStateToClient(pUser->GetUSN(), pMsg->m_lLoginState);
}

//void CRoomInternalLogic::OnRcvGetUserRankInfoReq( LONG usn, MsgCliNGS_GetUserRankInfoReq* pMsg)
//{
//	// ��ŷ ��û�ô� ��ŷ������ Request ������. 
//	// ��û�� ����ڿ��� ans�� �����ؾ� �ϹǷ�, SendRksReadReqMsg �� usn�� �ѱ��.
//}

//
// ADL Send
//
void CRoomInternalLogic::NotifyChangeRoomOptionToClient()
{
//	RoomOption& roomOption = GetNFRoomOption();

	PayloadNGSCli pld(PayloadNGSCli::msgChangeRoomOptionNtf_Tag);
	pld.un.m_msgChangeRoomOptionNtf->Clear();
	pld.un.m_msgChangeRoomOptionNtf->m_roomOption.BCopy(GetNFRoomOption());
	// ���� �Ű� ���� ����

	//if (pld.un.m_msgChangeRoomOptionNtf->m_nfRoomOption.m_sPassword.length() > 0)
	//	pld.un.m_msgChangeRoomOptionNtf->m_nfRoomOption.m_sPassword = "1";

	UserManager->SendToAllUser(pld);
}

void CRoomInternalLogic::NotifyChangeLoginStateToClient(LONG lCSN, LONG lLoginState)
{
	PayloadNGSCli pld(PayloadNGSCli::msgLoginStateChangeNtf_Tag);
	pld.un.m_msgLoginStateChangeNtf->m_lCSN = lCSN;
	pld.un.m_msgLoginStateChangeNtf->m_lLoginState = lLoginState;

	UserManager->SendToAllUser(pld);
}

//////////////////////////////////////////////////////////////////////////////
/*
void CRoomInternalLogic::UserManager->SendToUser(CUser* pUser, const PayloadNGSCli& pld)
{
	UserManager->SendToUser( pUser->GetUSN(), pld );


}
void CRoomInternalLogic::UserManager->SendToAllUser(const PayloadNGSCli& pld, CUser* pExcept)
{

	if( NULL != pExcept )
		UserManager->endToAllUserUsersExceptOne( pExcept->GetUSN(), pld );
	else
		UserManager->SendToAllUser( pld );
}
*/
/*void CRoomInternalLogic::UserManager->SendToAllUser( const PayloadNGSCli& pld )
{
	UserManager->UserManager->SendToAllUserUser( pld );
}*/

///////////////////////////////////////////////////////////////////////////////
// for LCS
/*
void CRoomInternalLogic::GetUsers(LONG lPartKey, LONG lLCSCount, vector<LONG> &vecUSN, string & sOption)
{
	LONG size = mUsers.size();
	if(!size)
		return;
	CUserList::iterator itr = mUsers.begin();
	for(int i = 0; i<size; i++)
	{
		if(itr == mUsers.end())
			break;
		LONG _USN = (*itr)->GetUSN();
		if(lPartKey == _USN%lLCSCount)
			vecUSN.push_back((*itr)->GetUSN());
		itr++;
	}

	sOption = m_sGameOption.c_str();
}
*/