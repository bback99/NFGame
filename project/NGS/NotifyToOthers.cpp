#include "stdafx.h"
#include "NotifyToOthers.h"

#include "RoomTable.h"
#include "UserTable.h"
#include "LrbConnector.h"

CNotifyToOthers::CNotifyToOthers()
:
dwRID( 0 )
{

}

void CNotifyToOthers::SetRoomData( const DWORD dwRIndex, const RoomID & rID )
{
	dwRID = dwRIndex;
	curRoomID = rID;
}


void CNotifyToOthers::NotifyCreateRoomToChs(CUser* pUser, const NFRoomOption & nfRoomOption, const xstring & GameOption)
{
	const NSAP& nsap = theRoomTable.GetNSAP();
	MsgNGSCHS_RoomCreateNtf Msg;

	//	MessageHeader header;
	//	SetMessageHeader(SVCCAT_CHS, header);

	PayloadNGSCHS pld(PayloadNGSCHS::msgRoomCreateNtf_Tag);
	pld.un.m_msgRoomCreateNtf->m_roomID = curRoomID;
	pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_dwGRIID = curRoomID.m_dwGRIID;
	pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_nsapGLS = nsap;
	pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_lRoomState = ROOMSTATE_RUN;
	pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_lRoomType = 0L;
	pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_roomOption.BCopy( nfRoomOption );	//GetNFRoomOption();


	pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_sGameOption.assign(GameOption.c_str(), GameOption.length());	//m_sGameOption
	pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_sReserve1.erase();
	if (pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_roomOption.m_sPassword.length() > 0 )
		pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_roomOption.m_sPassword = "1";

	SendToCHS(chsAddr, pld);
}

void CNotifyToOthers::NotifyAddUserToChs(CUser* pUser, LONG lCSN)
{
	//	ASSERT(pUser);
	if (!pUser) return;

	NFUserBaseInfo& nfUBI = pUser->GetUserData();

	PayloadNGSCHS pld(PayloadNGSCHS::msgUserRoomJoinNtf_Tag);
	pld.un.m_msgUserRoomJoinNtf->m_roomID = curRoomID;
	pld.un.m_msgUserRoomJoinNtf->m_nfUserBaseInfo.BCopy(nfUBI);

	CLink* pLink = pUser->GetLink();

	if(pLink != NULL)
	{
		pld.un.m_msgUserRoomJoinNtf->m_nfUserBaseInfo.m_lClientIp = pLink->GetIP();
		pld.un.m_msgUserRoomJoinNtf->m_nfUserBaseInfo.m_lGatewayIp = pLink->GetGatewayIP();
		// Use ReservedStr2 as Client's Mac Address
		pld.un.m_msgUserRoomJoinNtf->m_nfUserBaseInfo.m_sReserved2 = pLink->GetMacAddr();
	}
	else
		theLog.Put(WAR_UK, "NGS_Null"_COMMA, "pLink is NULL in NotifyAddUserToChs. USN: ", nfUBI.m_lUSN, ", CSN: ", nfUBI.m_lLastPlayCSN);

	SendToCHS(chsAddr, pld);
}


void CNotifyToOthers::NotifyRemoveUserToChs(long lCSN)
{
	PayloadNGSCHS pld(PayloadNGSCHS::msgUserRoomLeaveNtf_Tag);
	pld.un.m_msgUserRoomLeaveNtf->m_roomID = curRoomID;
	pld.un.m_msgUserRoomLeaveNtf->m_lCSN = lCSN;

	SendToCHS( chsAddr, pld);
}

void CNotifyToOthers::NotifyRemoveRoomToChs()
{
	PayloadNGSCHS pld(PayloadNGSCHS::msgRoomDeleteNtf_Tag);
	pld.un.m_msgRoomDeleteNtf->m_roomID = curRoomID;

	SendToCHS( chsAddr, pld);
}

void CNotifyToOthers::NotifyChangeRoomOptionToChs(const NFRoomOption & nfRoomOption)
{
	//	RoomOption& roomOption = GetNFRoomOption();

	PayloadNGSCHS pld(PayloadNGSCHS::msgChangeRoomOptionNtf_Tag);
	pld.un.m_msgChangeRoomOptionNtf->m_roomID = curRoomID;
	pld.un.m_msgChangeRoomOptionNtf->m_nfRoomOption.BCopy( nfRoomOption );	

	if (pld.un.m_msgChangeRoomOptionNtf->m_nfRoomOption.m_sPassword.length() > 0)
		pld.un.m_msgChangeRoomOptionNtf->m_nfRoomOption.m_sPassword = "1";
	SendToCHS( chsAddr, pld);
}

void CNotifyToOthers::NotifyChangeGameOptionToChs(const string & rOption)
{


	//	MessageHeader header;
	//	SetMessageHeader(SVCCAT_CHS, header);
	//	LRBAddress header;
	//	MakeOtherAddress(header, GetCHSAddr(), DWORD('M'), SVCCAT_CHS);

	PayloadNGSCHS pld(PayloadNGSCHS::msgChangeGameOptionNtf_Tag);
	pld.un.m_msgChangeGameOptionNtf->m_roomID = curRoomID;
	pld.un.m_msgChangeGameOptionNtf->m_sGameOption.assign(rOption.c_str(), rOption.length());

	SendToCHS( chsAddr, pld);
}

void CNotifyToOthers::NotifyChangeRoomStateToChs(LONG lState)
{

	PayloadNGSCHS pld(PayloadNGSCHS::msgRoomStatusChangeNtf_Tag);
	pld.un.m_msgRoomStatusChangeNtf->m_roomID = curRoomID;
	pld.un.m_msgRoomStatusChangeNtf->m_lRoomState = lState;

	SendToCHS( chsAddr, pld);
}


void CNotifyToOthers::NotifyChangeUserInfoToChs(ChangeUserGameDataList& lstUserGameData)
{
	//	MessageHeader header;
	//	SetMessageHeader(SVCCAT_CHS, header);
	//	LRBAddress header;
	//	MakeOtherAddress(header, GetCHSAddr(), DWORD('M'), SVCCAT_CHS);

	//	theLog.Put(ERR_UK, "CRoomInternalLogic_Error,NotifyChangeUserInfoToChs", roomID.m_lSSN, roomID.m_dwCategory, roomID.m_dwGRIID);		

	PayloadNGSCHS pld(PayloadNGSCHS::msgUserInfoInRoomChangeNtf_Tag);
	pld.un.m_msgUserInfoInRoomChangeNtf->m_roomID = curRoomID;
	pld.un.m_msgUserInfoInRoomChangeNtf->m_lstUserGameData.BCopy(lstUserGameData);

	SendToCHS( chsAddr, pld);
}

// 세이클럽 접속 끊어진 경우
void CNotifyToOthers::NotifyChangeLoginStateToChs(LONG lCSN, LONG lLoginState)
{
	PayloadNGSCHS pld(PayloadNGSCHS::msgChangeLoginStateNtf_Tag);
	pld.un.m_msgChangeLoginStateNtf->m_roomID = curRoomID;
	pld.un.m_msgChangeLoginStateNtf->m_lCSN = lCSN;
	pld.un.m_msgChangeLoginStateNtf->m_lLoginState = lLoginState;

	SendToCHS( chsAddr, pld);
}

//
// NDL Send to NCS
//
void CNotifyToOthers::NotifyNGSInfoToNCS(BOOL bAlways)
{
	LONG lRoomCount = 0;
	LONG lUserCount = 0;
	BOOL bRoomNotiNeeded = theRoomTable.IsNotiNeeded(lRoomCount);
	BOOL bUserNotiNeeded = theUserTable.IsNotiNeeded(lUserCount);
	if (bAlways || bRoomNotiNeeded || bUserNotiNeeded) {
		//		RoomID& roomID = GetRoomID();
		const NSAP& nsap = theRoomTable.GetNSAP();

		//		MessageHeader header;
		//		SetMessageHeader(SVCCAT_LB, header);

		PayloadNGSNCS pld(PayloadNGSNCS::msgNGSInfoNtf_Tag);
		pld.un.m_msgNGSInfoNtf->m_lLogicalAddr = theRoomTable.GetAddr();
		pld.un.m_msgNGSInfoNtf->m_lServiceTypeID = MAKELONG(g_wSvcType, SVCCAT_NGS);
		pld.un.m_msgNGSInfoNtf->m_nsapNGS = nsap;
		pld.un.m_msgNGSInfoNtf->m_lRoomCount = lRoomCount;
		pld.un.m_msgNGSInfoNtf->m_lUserCount = lUserCount;

		SendToNCS(theRoomTable.GetNCSAddr(), pld);

		if (bAlways || bRoomNotiNeeded) {
			theRoomTable.OnNotify(lRoomCount);
		}
		if (bAlways || bUserNotiNeeded) {
			theUserTable.OnNotify(lUserCount);
		}
	}
}

void CNotifyToOthers::SendToCHS(const LRBAddress& dest, PayloadNGSCHS& pld)
{
	if (dest.GetCastType() == CASTTYPE_INVALID)
	{
		theLog.Put(WAR_UK, "NGS_theLRBManager_Error,Invalid Destination Message in SendToCHS(). pld.mTagID:",pld.mTagID, ", RoomID:",RoomID2Str( curRoomID ),", RoomIndex:", dwRID,", RefCount:");		
	}
	theLrbManager.SendToCHS(theRoomTable.GetAddr(), dest, pld, FALSE );
}

// void CNotifyToOthers::SendToAllCHS(const LRBAddress& dest, PayloadNGSCHS& pld)
// {
// 	if (dest.GetCastType() == CASTTYPE_INVALID)
// 	{
// 		theLog.Put(WAR_UK, "NGS_theLRBManager_Error,Invalid Destination Message in SendToCHS(). pld.mTagID:",pld.mTagID, ", RoomID:",RoomID2Str( curRoomID ),", RoomIndex:", dwRID,", RefCount:");		
// 	}
// 	theLrbManager.SendToCHS(theRoomTable.GetAddr(), dest, pld, FALSE );
// }

void CNotifyToOthers::SendToNCS(const LRBAddress& dest, PayloadNGSNCS& pld)
{
	theLrbManager.SendToNCS(theRoomTable.GetAddr(), dest, pld);
}

void CNotifyToOthers::SendToAMS(const LRBAddress& dest, PayloadGLSAMS& pld)
{
	theLrbManager.SendToAMS(theRoomTable.GetAddr(), dest, pld);
}

void CNotifyToOthers::SendToNLS(const LRBAddress& dest, PayloadCLINLS& pld)
{
	theLrbManager.SendToNLS(theRoomTable.GetAddr(), dest, pld);
}

void CNotifyToOthers::SendToPLS(const LRBAddress& dest, GBuf& buf)
{
	theLrbManager.SendToPLS(theRoomTable.GetAddr(), dest, buf);
}

void CNotifyToOthers::SendToIBB(const LRBAddress& dest, GBuf& buf)
{
	theLrbManager.SendToIBB(theRoomTable.GetAddr(), dest, buf);
}

void CNotifyToOthers::SendToNAS(const LRBAddress& dest, PayloadCLINAS& pld)
{
	theLrbManager.SendToNAS(theRoomTable.GetAddr(), dest, pld);
}