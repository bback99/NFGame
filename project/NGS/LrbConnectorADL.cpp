//
// LrbConnectorADL.cpp
//

#include "stdafx.h"
#include "LrbConnector.h"
#include "Room.h"
#include "RoomTable.h"
#include "UserTable.h"
#include "StatisticsTable.h"
#include <NF/ADL/MsgNCSNGS.h>
#include <ADL/MsgAMSGLS.h>
#include <NF/ADL/MsgCHSNGS.h>

#include <NF/ADL/MsgNLSCli.h>
#include <ADL/MsgLCSMsg2.h>
#include "ErrorLog.h"

#include <ADL/MsgGLSGLS.h>
#include <NLSManager.h>
#include <NF/ADL/MsgNASCli.h>


//////////////////////////////////////////////////////////////////////////////
// adl receive related

#pragma oMSG("우선 빼 놓은 것임. LRB와 접속 완료시 NCS, CHS에게 알려줘야 할 것들 있음.")
/*
void CLrbHandler::OnRcvRegisterServiceAns(MsgRegisterServiceAns* pMsg)
{
	while (pMsg->m_lErrorCode == RDA_RECONNECT) {
		TLOG0("LrbConnector::OnRcvRegisterServiceAns() - Wait for LRB Config.\n");
		::Sleep(5000);
		SendRegisterServiceReq();
		return;
	}

	if (pMsg->m_lErrorCode == RDA_RECONNECTOTHER) {
		Stop();
		Run();
		return;
	}

	DWORD dwAddr = MAKELONG(pMsg->m_logicalAddr.m_wLinkID, pMsg->m_logicalAddr.m_wLRBID);
	theRoomTable.SetAddr(dwAddr);
	DWORD dwTypeID = MAKELONG(g_wSvcType, pMsg->m_serviceTypeID.m_wSvcCat);
	TLOG2("LrbConnector::OnRcvRegisterServiceAns() \n\t- LogicalAddr:[0x%08X], SvcTypeID:[0x%08X]\n", dwAddr, dwTypeID);
	theRoomTable.SetTypeID(dwTypeID);

	SendRegisterStaticMulticastReq(pMsg->m_logicalAddr, pMsg->m_serviceTypeID);
//	SendRegisterServiceToLB(dwAddr, dwTypeID);
	SendRegisterServiceNtfToNCS();

	if (theRoomTable.IsRegistered()) {
		theRoomTable.OnNotify(0);
		theUserTable.OnNotify(0);
		LONG lRoomCount = theRoomTable.GetRoomCount();
		LONG lUserCount = theUserTable.GetUserCount();
		BOOL bRoomNotiNeeded = theRoomTable.IsNotiNeeded(lRoomCount);
		BOOL bUserNotiNeeded = theUserTable.IsNotiNeeded(lUserCount);
		const NSAP& nsap = theRoomTable.GetNSAP();
		if (bRoomNotiNeeded || bUserNotiNeeded) {
			MsgNGSNCS_GLSInfoNtf Msg;
			Msg.m_lLogicalAddr = theRoomTable.GetAddr();
			Msg.m_lServiceTypeID = MAKELONG(g_wSvcType, SVCCAT_NGS);
			Msg.m_nsapNGS = nsap;
			Msg.m_lRoomCount = lRoomCount;
			Msg.m_lUserCount = lUserCount;

			MessageHeader header;
			header.m_cMessageType = MSGTYPE_ACK;
			header.m_cRoutingType = ROUTTYPE_ANYCASTING;
			header.m_dwSourceAddr = theRoomTable.GetAddr();
			header.m_dwSourceTypeID = theRoomTable.GetTypeID();
			header.m_dwTargetAddr = MAKELONG(SVCTYP_ANY, SVCCAT_LB);
			header.m_dwTargetTypeID = MAKELONG(SVCTYP_ANY, SVCCAT_LB);

			PayloadNGSNCS pld(PayloadNGSNCS::msgNGSInfoNtf_Tag, Msg);

			GBuf buf;
			pld.BStore(buf);

			PayloadServiceLRB pldLRB(PayloadServiceLRB::msgService_Tag);
			pldLRB.un.m_msgService->m_header = header;
			pldLRB.un.m_msgService->m_sRealMsg.assign((LPCSTR)buf.GetData(), buf.GetLength());
			PostSendToLrb(pldLRB);

			if (bRoomNotiNeeded) {
				theRoomTable.OnNotify(lRoomCount);
			}
			if (bUserNotiNeeded) {
				theUserTable.OnNotify(lUserCount);
			}
		}

		PayloadNGSCHS pldCHS(PayloadNGSCHS::msgReconnectNtf_Tag);
		pldCHS.un.m_msgReconnectNtf->m_nsapNGS = nsap;

		GBuf buf;
		pldCHS.BStore(buf);

		MessageHeader header;
		header.m_cMessageType = MSGTYPE_ACK;
		header.m_cRoutingType = ROUTTYPE_STATICMULTICASTING;
		header.m_dwSourceAddr = theRoomTable.GetAddr();
		header.m_dwSourceTypeID = theRoomTable.GetTypeID();
		header.m_dwTargetAddr = MAKELONG(SVCTYP_ANY, SVCCAT_CHS);
		header.m_dwTargetTypeID = MAKELONG(SVCTYP_ANY, SVCCAT_CHS);

		PayloadServiceLRB pldLRB(PayloadServiceLRB::msgService_Tag);
		pldLRB.un.m_msgService->m_header = header;
		pldLRB.un.m_msgService->m_sRealMsg.assign((LPCSTR)buf.GetData(), buf.GetLength());
		PostSendToLrb(pldLRB);
	} else {
		theRoomTable.SetRegister(TRUE);
	}
}
*/

#pragma oMSG("우선 빼 놓은 것임. Room으로 직접 보내는 메시지가 있을 경우 구현 고민할 것")
/*
void CLrbHandler::OnRcvServiceMsg(PayloadLRBService& pld)
{
	switch(pld.mTagID)
	{
	case PayloadLRBService::msgService_Tag:
		break;
	default:
		{
			TLOG1("LrbConnector::OnRcvServiceMsg - Unknown message(Tag:%d)\n", pld.mTagID);
		}
		return;
		break;
	}

	MsgService* pMsg = pld.un.m_msgService;
	if (pMsg->m_header.m_cRoutingType == (char)ROUTTYPE_SESSION_UNICASTING) {
		RoomID roomID(pMsg->m_header.m_sTargetInstanceID.m_sID.c_str());

		CRoom* pRoom = NULL;
		if (theRoomTable.FindRoom(roomID, &pRoom)) {
			pRoom->OnLrbMsg(pld);
			pRoom->Release();
			return;
		}
	}

	OnRcvRealMsg(pMsg);
}
*/

//////////////////////////////////////////////////////////////////////////////
// ADL Receive from AMS
//

void CLrbHandler::OnRcvAMSMsg(const PayloadInfo pldInfo, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "Nack returned in OnRcvAMSMsg");
		return;
	}
}

void CLrbHandler::OnRcvRoomDeleteReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_RoomDeleteReq* pMsg)
{
}

void CLrbHandler::OnRcvAnnounceReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_AnnounceReq* pMsg)
{
	TLOG0("CLrbHandler::OnRcvAnnounceReqFromAMS()");

	BOOL bRoomExist = FALSE;

	RoomList lstRoom;
	lstRoom.clear();

	switch (pMsg->m_lAnnounceType)
	{
	case ANNOUNCE_ALLSERVICE:
		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_ALLSERVICE, lstRoom);
		break;
	case ANNOUNCE_SSN:
		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_SSN, pMsg->m_roomID.m_lSSN, lstRoom);
		break;
	case ANNOUNCE_CATEGORY:
		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_CATEGORY, pMsg->m_roomID.m_lSSN, pMsg->m_roomID.m_dwCategory, lstRoom);
		break;
	case ANNOUNCE_CHANNEL:
		bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_CHANNEL, pMsg->m_roomID.m_lSSN, pMsg->m_roomID.m_dwCategory, pMsg->m_roomID.m_dwGCIID, lstRoom);
		break;
	case ANNOUNCE_ROOM:
		{
			bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_ROOM, pMsg->m_roomID.m_lSSN, pMsg->m_roomID.m_dwCategory, pMsg->m_roomID.m_dwGCIID, pMsg->m_roomID.m_dwGRIID, lstRoom);
		}
		break;
	default:
		bRoomExist = FALSE;
		break;
	}

	if (!bRoomExist || !lstRoom.size()) {
		return;
	}
}

void CLrbHandler::OnRcvServiceStopReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_ServiceStopReq* pMsg)
{
	TLOG0("LrbConnector::OnRcvServiceStopReqFromAMS() - DO NOTHING!");
}

void CLrbHandler::OnRcvHeartBeatReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_HeartBeatReq* pMsg)
{
	theLrbManager.SendHeartBeatAnsToAMS(src, dest, pMsg->m_lStep);
}

void CLrbHandler::OnRcvStatisticsReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_StatisticReq* pMsg)
{
	TLOG0("LrbConnector::OnRcvStatisticsReqFromAMS()");
	theLrbManager.SendStatisticsAnsToAMS(src, dest, pMsg->m_lStep);
}

void CLrbHandler::OnRcvComputerNameReqFromAMS(const LRBAddress& src, const LRBAddress& dest, MsgAMSGLS_ComputerNameReq* pMsg)
{
	theLrbManager.SendComputerNameAnsToAMS(src, dest);
}

//
// ADL Receive from NCS
void CLrbHandler::OnRcvNCSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK) {
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "Nack returned in OnRcvNCSMsg");
		return;
	}

	PayloadNCSNGS pld;
	BOOL bRet = pld.BLoad(buf);

	if (!bRet)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "BLoad Failure in OnRcvNCSMsg");
		return;
	}

	switch (pld.mTagID)
	{
	case PayloadNCSNGS::msgRegisterServiceReq_Tag:
		OnRcvRegisterServiceReqFromLB(src);
		break;
	case PayloadNCSNGS::msgNtfNFFriendAdd_Tag:
		OnRcvNtfFriendAdd(pld.un.m_msgNtfNFFriendAdd);
		break;
	case PayloadNCSNGS::msgNtfNFFriendAccept_Tag:
		OnRcvFromNCSNtfFriendAccept(pld.un.m_msgNtfNFFriendAccept);
		break;
	case PayloadNCSNGS::msgNtfNFLetterReceive_Tag:
		OnRcvFromNCSNtfNFLetterReceive(pld.un.m_msgNtfNFLetterReceive);
		break;
	default:
		{
			theLog.Put(DEV, "CLrbHandler::OnRcvNCSMsg - Unknown message(Tag:", pld.mTagID, ")");
		}
		break;
	}
}

//
// ADL Receive from NLS
void CLrbHandler::OnRcvNLSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "Nack Returned in OnRcvNLSMsg");
		return;
	}

	PayloadNLSCLI pld;
	BOOL bRet = pld.BLoad(buf);

	if (!bRet)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "BLoad Failure in OnRcvNLSMsg");
		return;
	}

	theNLSManager.RecvNLSMessage(&pld, src);
}

void CLrbHandler::OnRcvRegisterServiceReqFromLB(const LRBAddress& src)
{
	theLrbManager.SendRegisterServiceAnsToLB(src);
	theLrbManager.NotifyNGSInfoToNCS(src);
}

void CLrbHandler::OnRcvNtfFriendAdd(MsgNCSNGS_NtfNFFriendAdd* pMsg)
{
	CRoomPtr spRoom;
	if (!theRoomTable.FindRoom(pMsg->m_roomID, &spRoom)) {
		theLog.Put(ERR_UK, "NGS_LRBHandler_Error"_COMMA, "OnRcvNtfFriendAdd not Found Room!!! roomID : ", pMsg->m_roomID.m_dwGRIID);
		return;
	}

	MsgNGSCli_NtfNFFriendAdd ntf;
	ntf.m_strCharName = pMsg->m_strSender;

	PayloadNGSCli pld(PayloadNGSCli::msgNtfNFFriendAdd_Tag, ntf);
	spRoom->SendToUser(pMsg->m_lReceiverCSN, pld);
}

void CLrbHandler::OnRcvFromNCSNtfFriendAccept(MsgNCSNGS_NtfNFFriendAccept* pMsg)
{
	CRoomPtr spRoom;
	if (!theRoomTable.FindRoom(pMsg->m_roomID, &spRoom)) {
		theLog.Put(ERR_UK, "NGS_LRBHandler_Error"_COMMA, "OnRcvFromNCSNtfFriendAccept not Found Room!!! roomID : ", pMsg->m_roomID.m_dwGRIID);
		return;
	}

	MsgNGSCli_NtfNFFriendAccept ntf;
	ntf.m_nfFriend = pMsg->m_nfFriend;
	PayloadNGSCli pld(PayloadNGSCli::msgNtfNFFriendAccept_Tag, ntf);
	spRoom->SendToUser(pMsg->m_lReceiverCSN, pld);
}

void CLrbHandler::OnRcvFromNCSNtfNFLetterReceive(MsgNCSNGS_NtfNFLetterReceive* pMsg)
{
	CRoomPtr spRoom;
	if (!theRoomTable.FindRoom(pMsg->m_roomID, &spRoom)) {
		theLog.Put(ERR_UK, "NGS_LRBHandler_Error"_COMMA, "OnRcvFromNCSNtfFriendAccept not Found Room!!! roomID : ", pMsg->m_roomID.m_dwGRIID);
		return;
	}

	MsgNGSCli_NtfNFLetterReceive ntf;
	PayloadNGSCli pld(PayloadNGSCli::msgNtfNFLetterReceive_Tag, ntf);
	spRoom->SendToUser(pMsg->m_lReceiverCSN, pld);
}

//
// ADL Receive from CHS
void CLrbHandler::OnRcvCHSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "Nack Returned in OnRcvCHSMsg. src=",src.GetString(), ", dest=", pldInfo.addr.GetString());
		return;
	}

	PayloadCHSNGS pld;
	BOOL bRet = pld.BLoad(buf);

	if(!bRet)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "BLoad Failure in OnRcvCHSMsg");
		return;
	}

	switch (pld.mTagID)
	{
	case PayloadCHSNGS::msgChangeAddrNtf_Tag:
		OnRcvChangeCHSAddrFromCHS(pld.un.m_msgChangeAddrNtf);
		break;
	case PayloadCHSNGS::msgChannelIDList_Tag:
		OnRcvCannelIDListFromCHS(pld.un.m_msgChannelIDList, src);
		break;
	case PayloadCHSNGS::msgAnsFreeRoomList_Tag:
		OnRcvAnsFreeRoomList(pld.un.m_msgAnsFreeRoomList);
		break;
	case PayloadCHSNGS::msgNtfNFFriendAccept_Tag:
		OnRcvFromCHSNtfFriendAccept(pld.un.m_msgNtfNFFriendAccept);
		break;
	default:
		{
			theLog.Put(DEV, "CLrbHandler::OnRcvCHSMsg - Unknown message(Tag:", pld.mTagID, ")");
		}
		break;
	}
}

void CLrbHandler::OnRcvChangeCHSAddrFromCHS(MsgCHSNGS_ChangeAddrNtf* pMsg)
{
	RoomList lstRoom;
	lstRoom.clear();

	BOOL bRoomExist = theRoomTable.GetRoomList(lstRoom);
	if (!bRoomExist || !lstRoom.size()) {
		return;
	}

	ForEachElmt(RoomList, lstRoom, i, j) {
		CRoom* pRoom = (*i);
		if (!pRoom) continue;
		pRoom->ChangeCHSAddr(pMsg->m_dwOldAddr, pMsg->m_dwNewAddr);
		pRoom->Release();
	}
}

void CLrbHandler::OnRcvCannelIDListFromCHS(MsgCHSNGS_ChannelIDList* pMsg, const LRBAddress &src)
{
}

void CLrbHandler::OnRcvAnsFreeRoomList(MsgCHSNGS_AnsFreeRoomList* pMsg)
{
	CRoomPtr spRoom;;
	if (!theRoomTable.FindRoom(pMsg->m_roomID, &spRoom)) {
		theLog.Put(ERR_UK, "NGS_LRBHandler_Error"_COMMA, "OnRcvAnsFreeRoomList not Found Room!!! roomID : ", pMsg->m_roomID.m_dwGRIID);
		return;
	}

	spRoom->OnRcvAnsFreeRoomList(pMsg->m_lCSN, pMsg);
}

void CLrbHandler::OnRcvFromCHSNtfFriendAccept(MsgCHSNGS_NtfNFFriendAccept* pMsg)
{
	CRoomPtr spRoom;
	if (!theRoomTable.FindRoom(pMsg->m_roomID, &spRoom))
	{
		theLog.Put(ERR_UK, "NGS_LRBHandler_Error"_COMMA, "OnRcvFromCHSNtfFriendAccept not Found Room!!! roomID : ", pMsg->m_roomID.m_dwGRIID);
		return;
	}

	MsgNGSCli_NtfNFFriendAccept ntf;
	ntf.m_nfFriend = pMsg->m_nfFriend;
	PayloadNGSCli pld(PayloadNGSCli::msgNtfNFFriendAccept_Tag, ntf);
	spRoom->SendToUser(pMsg->m_lReceiverCSN, pld);
}

//
// ADL Receive from NGS
void CLrbHandler::OnRcvNGSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "Nack Returned in OnRcvNGSMsg");
		return;
	}

	PayloadGLSGLS pld;
	BOOL bRet = pld.BLoad(buf);
	if (!bRet)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "BLoad Failure in OnRcvNGSMsg");
		return;
	}

	theLog.Put(DEV, "CLrbHandler::OnRcvNGSMsg, Tag ID: ", pld.mTagID);

	switch (pld.mTagID)
	{
	case PayloadGLSGLS::msgMulticastNtf_Tag:
		{
		}
		break;
	default:
		{
			string sAddr;
			src.GetStringFormat(sAddr);
			theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "Unknown message(Tag ID: ", pld.mTagID, ") from ", sAddr.c_str(), " in OnRcvNGSMsg");
		}
		break;
	}
}

void CLrbHandler::OnRcvPLSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "Nack Returned in OnRcvPLSMsg");
		return;
	}
}

void CLrbHandler::OnRcvIBBMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "Nack Returned in OnRcvIBBMsg");

		theRoomTable.SetIBBTerminate();

		return;
	}
}

void CLrbHandler::OnRcvRKSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{		
		return;
	}
}

void CLrbHandler::OnRcvBCSMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{		
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error, Nack Returned in OnRcvBCSMsg");
		return;
	}
}

void CLrbHandler::OnRcvNASMsg(const PayloadInfo pldInfo, const LRBAddress& src, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	if (wMessageType & MESSAGE_NACK)
	{		
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error, Nack Returned in OnRcvNASMsg");
		return;
	}

	PayloadNASCLI pld;
	BOOL bRet = pld.BLoad(buf);
	if(!bRet)
	{
		theLog.Put(WAR_UK, "NGS_LRBHandler_Error"_COMMA, "BLoad Failure in OnRcvNASMsg");
		return;
	}

	switch (pld.mTagID)
	{	
	default:
		{
			theLog.Put(DEV, "CLrbHandler::OnRcvNASMsg - Unknown message(Tag:", pld.mTagID, ")");
		}
		break;
	}
}

void CLrbHandler::OnRcvODBGWMsg(const PayloadInfo pldInfo, const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol) 
{
}

void CLrbHandler::OnNGSMulticastNotify(LONG lSSN, GBuf& gBuf)
{
	BOOL bRoomExist = FALSE;

	RoomList lstRoom;
	lstRoom.clear();
	
	bRoomExist = theRoomTable.GetRoomList(ANNOUNCE_SSN, lSSN, lstRoom);
	
	if (!bRoomExist || !lstRoom.size()) 
		return;

	ForEachElmt(RoomList, lstRoom, i, j) 
	{
		CRoom* pRoom = (*i);
		if (!pRoom) 
			continue;
	
		GBuf gMultiBuf(gBuf);
		pRoom->MulticastNotify(gMultiBuf);
		pRoom->Release();
	}
}

//
// CLrbManager
BOOL CLrbManager::OnLrbRegistered()
{
	SendRegisterServiceNtfToNCS();

	if (theRoomTable.IsRegistered()) {
		theRoomTable.OnNotify(0);
		theUserTable.OnNotify(0);
		LONG lRoomCount = theRoomTable.GetRoomCount();
		LONG lUserCount = theUserTable.GetUserCount();
		BOOL bRoomNotiNeeded = theRoomTable.IsNotiNeeded(lRoomCount);
		BOOL bUserNotiNeeded = theUserTable.IsNotiNeeded(lUserCount);
		const NSAP& nsap = theRoomTable.GetNSAP();
		if (bRoomNotiNeeded || bUserNotiNeeded) {
			NotifyNGSInfoToNCS(theRoomTable.GetNCSAddr());

			if (bRoomNotiNeeded) {
				theRoomTable.OnNotify(lRoomCount);
			}
			if (bUserNotiNeeded) {
				theUserTable.OnNotify(lUserCount);
			}
		}

		PayloadNGSCHS pldCHS(PayloadNGSCHS::msgReconnectNtf_Tag);
		pldCHS.un.m_msgReconnectNtf->m_nsapNGS = nsap;

		GBuf buf;
		BOOL bRet = pldCHS.BStore(buf);
		if (!bRet) {
			return FALSE;
		}

		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&theRoomTable.GetAddr(), (const LPLRBADDRESS)&theRoomTable.GetChsAddr());
	} 
	else 
	{
		theRoomTable.SetRegister(TRUE);
	}
	return TRUE;
}

//
// adl send to AMS
void CLrbManager::SendHeartBeatAnsToAMS(const LRBAddress& src, const LRBAddress& dest, LONG lStep)
{
	theLog.Put(DEV, "LrbConnector::SendHeartBeatAnsToAMS() - lStep : ", lStep);

	const NSAP& nsap = theRoomTable.GetNSAP();
	MsgGLSAMS_HeartBeatAns msgNDL;
	msgNDL.m_lStep = lStep;
	msgNDL.m_nsapAnswer = nsap;
	msgNDL.m_lLogicalAddr = theRoomTable.GetAddr();
	msgNDL.m_lServiceTypeID = theRoomTable.GetTypeID();
	PayloadGLSAMS pld(PayloadGLSAMS::msgHeartBeatAns_Tag, msgNDL);

	SendToAMS(dest, src, pld);
}

void CLrbManager::SendStatisticsAnsToAMS(const LRBAddress& src, const LRBAddress& dest, LONG lStep)
{
	theLog.Put(DEV, "LrbConnector::SendStatisticsAnsToAMS() - lStep : ", lStep);

	MsgGLSAMS_StatisticAns msgNDL;
	msgNDL.m_lStep = lStep;
	msgNDL.m_lLogicalAddr = theRoomTable.GetAddr();
	theStatTable.GetStatInfo(msgNDL.m_statinfolist);
	PayloadGLSAMS pld(PayloadGLSAMS::msgStatisticAns_Tag, msgNDL);
	
	SendToAMS(dest, src, pld);
}

void CLrbManager::SendComputerNameAnsToAMS(const LRBAddress& src, const LRBAddress& dest)
{
	TLOG0("LrbConnector::SendComputerNameAnsToAMS()");

	string& sComName = theRoomTable.GetComputerName();

	MsgGLSAMS_ComputerNameAns msg;
	msg.m_nsapAnswer = theRoomTable.GetNSAP();
	msg.m_sComName.assign(sComName.c_str(), sComName.length());
	PayloadGLSAMS pld(PayloadGLSAMS::msgComputerNameAns_Tag, msg);

	SendToAMS(dest, src, pld);
}

//
// adl send to NCS
void CLrbManager::SendRegisterServiceNtfToNCS()
{
	const LRBAddress& addr = theRoomTable.GetAddr();

	MsgNGSNCS_RegisterServiceNtf msg;
	const NSAP& nsap = theRoomTable.GetNSAP();
	msg.m_NGSNCSRegister.m_nsapNGS = nsap;
	msg.m_NGSNCSRegister.m_lLogicalAddr = addr;
	msg.m_NGSNCSRegister.m_lServiceTypeID = theRoomTable.GetTypeID();

	long lTypeAnyGls = MAKELONG(SVCTYP_ANY, SVCCAT_NGS);

	if (msg.m_NGSNCSRegister.m_lServiceTypeID != lTypeAnyGls)
	{
		PayloadNGSNCS pld(PayloadNGSNCS::msgRegisterServiceNtf_Tag, msg);
		GBuf buf;
		pld.BStore(buf);

		SendToNCS(addr, theRoomTable.GetNCSAddr(), pld);
	}
	else // msg.m_NGSNCSRegister.m_lServiceTypeID == MAKELONG(SVCTYP_ANY, SVCCAT_NGS)
	{
		vector<DWORD> vecTypeID = theRoomTable.GetTypeIDVector();
		for (vector<DWORD>::iterator i = vecTypeID.begin(); i != vecTypeID.end() ; i++)
		{			
			msg.m_NGSNCSRegister.m_lServiceTypeID = *i;			

			PayloadNGSNCS pld(PayloadNGSNCS::msgRegisterServiceNtf_Tag, msg);
			GBuf buf;
			pld.BStore(buf);
			
			SendToNCS(addr, theRoomTable.GetNCSAddr(), pld);

			theLog.Put(DEV_UK, "NGS_MultiGRC,SendRegisterServiceNtfToNCS(),ServiceType:<", LOWORD(*i) ,",",HIWORD(*i),">");
		}
	}
}

void CLrbManager::SendRegisterServiceAnsToLB(const LRBAddress& src)
{
	const LRBAddress& addr = theRoomTable.GetAddr();

	MsgNGSNCS_RegisterServiceAns msg;
	const NSAP& nsap = theRoomTable.GetNSAP();
	msg.m_NGSNCSRegister.m_nsapNGS = nsap;
	msg.m_NGSNCSRegister.m_lLogicalAddr = addr;
	msg.m_NGSNCSRegister.m_lServiceTypeID = theRoomTable.GetTypeID();

	long lTypeAnyGls = MAKELONG(SVCTYP_ANY, SVCCAT_NGS);

	if (msg.m_NGSNCSRegister.m_lServiceTypeID != lTypeAnyGls)
	{
		PayloadNGSNCS pld(PayloadNGSNCS::msgRegisterServiceAns_Tag, msg);
		GBuf buf;
		pld.BStore(buf);

		SendToNCS(addr, src, pld);
	}
	else  // msg.m_NGSNCSRegister.m_lServiceTypeID == SVCTYP_ANY_
	{
		vector<DWORD> vecTypeID = theRoomTable.GetTypeIDVector();
		for (vector<DWORD>::iterator i = vecTypeID.begin(); i != vecTypeID.end() ; i++)
		{	
			msg.m_NGSNCSRegister.m_lServiceTypeID = *i;

			PayloadNGSNCS pld(PayloadNGSNCS::msgRegisterServiceAns_Tag, msg);
			GBuf buf;
			pld.BStore(buf);

			SendToNCS(addr, src, pld);

			theLog.Put(DEV_UK, "NGS_MultiGRC,SendRegisterServiceAnsToLB(),ServiceType:<",  LOWORD(*i),",",HIWORD(*i),">");
		}
	}
}

void CLrbManager::NotifyNGSInfoToNCS(const LRBAddress& src)
{
	const LRBAddress& addr = theRoomTable.GetAddr();
	const NSAP& nsap = theRoomTable.GetNSAP();

	PayloadNGSNCS pld(PayloadNGSNCS::msgNGSInfoNtf_Tag);
	pld.un.m_msgNGSInfoNtf->m_lLogicalAddr = addr;
	pld.un.m_msgNGSInfoNtf->m_lServiceTypeID = theRoomTable.GetTypeID();
	pld.un.m_msgNGSInfoNtf->m_nsapNGS = nsap;
	pld.un.m_msgNGSInfoNtf->m_lRoomCount = theRoomTable.GetRoomCount();
	pld.un.m_msgNGSInfoNtf->m_lUserCount = theUserTable.GetUserCount();

//	long lTypeAnyGls = MAKELONG(SVCTYP_ANY, SVCCAT_NGS);

// 	if (pld.un.m_msgNGSInfoNtf->m_lServiceTypeID != lTypeAnyGls)
// 	{
 		SendToNCS(addr, src, pld);
// 	}
// 	else // pld.un.m_msgGLSInfoNtf->m_lServiceTypeID == lTypeAnyGls
// 	{
// 		vector<DWORD> vecTypeID = theRoomTable.GetTypeIDVector();
// 		for (vector<DWORD>::iterator i = vecTypeID.begin(); i != vecTypeID.end() ; i++)
// 		{	
// 			pld.un.m_msgNGSInfoNtf->m_lServiceTypeID = *i;
// 			SendToNCS(addr, src,pld);
// 		}
// 	}
}

//
// adl send
void CLrbManager::SendToCHS(const LRBAddress& src, const LRBAddress& dest, PayloadNGSCHS& pld, BOOL bInvalidAddrLog )
{
	GBuf buf;

	if (dest.GetCastType() == CASTTYPE_INVALID && bInvalidAddrLog)
	{
		theLog.Put(WAR_UK, "NGS_theLRBManager_Error"_COMMA, "Invalid Destination Message: ", pld.mTagID, " in SendToCHS");
	}
	else
	{
		BOOL bRet = ::BStore(buf, pld);
		if (!bRet)
		{
			return;
		}

		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
	}
}

void CLrbManager::SendToServer(const LRBAddress& src, const LRBAddress& dest, GBuf &buf)
{
	if (dest.GetCastType() == CASTTYPE_INVALID)
	{
		theLog.Put(WAR_UK, "NGS_theLRBManager_Error"_COMMA, "Invalid Destination Message in SendToServer");
	}
	else
	{
		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
	}
}


void CLrbManager::SendToAMS(const LRBAddress& src, const LRBAddress& dest, PayloadGLSAMS& pld)
{
	if (dest.GetCastType() == CASTTYPE_INVALID)
	{
		theLog.Put(WAR_UK, "NGS_theLRBManager_Error"_COMMA, "Invalid Destination Message: ", pld.mTagID, " in SendToAMS");
	}
	else
	{
		GBuf buf;
		BOOL bRet = ::BStore(buf, pld);
		if (!bRet)
		{
			return;
		}

		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
	}
}

void CLrbManager::SendToNCS(const LRBAddress& src, const LRBAddress& dest, PayloadNGSNCS& pld)
{
	GBuf buf;

	if (dest.GetCastType() == CASTTYPE_INVALID) 
	{
		theLog.Put(WAR_UK, "NGS_theLRBManager_Error"_COMMA, "Invalid Destination Message: ", pld.mTagID, " in SendToNCS");
	}
	else
	{
		BOOL bRet = pld.BStore(buf);
		if (!bRet)
		{
			return;
		}

		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
	}
}

void CLrbManager::SendToNLS(const LRBAddress& src, const LRBAddress& dest, PayloadCLINLS& pld)
{
	GBuf buf;

	if (dest.GetCastType() == CASTTYPE_INVALID)
	{
		theLog.Put(WAR_UK, "NGS_theLRBManager_Error"_COMMA, "Invalid Destination Message: ", pld.mTagID, " in SendToNLS");
	}
	else
	{
		BOOL bRet = ::BStore(buf, pld);
		if (!bRet)
		{
			return;
		}

		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
	}
}

void CLrbManager::SendToNAS(const LRBAddress& src, const LRBAddress& dest, PayloadCLINAS& pld)
{
	GBuf buf;

	if (dest.GetCastType() == CASTTYPE_INVALID)
	{
		theLog.Put(WAR_UK, "NGS_theLRBManager_Error"_COMMA, "Invalid Destination Message: ", pld.mTagID, " in SendToNAS");
	}
	else
	{
		BOOL bRet = ::BStore(buf, pld);
		if (!bRet)
		{
			return;
		}

		::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
	}
}

void CLrbManager::SendToMGLS(const LRBAddress& src, const LRBAddress& dest, PayloadGLSGLS& pld)
{
	GBuf buf;
	BOOL bRet = ::BStore(buf, pld);
	if (!bRet)
	{
		return;
	}

	::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
}

void CLrbManager::SendToPLS(const LRBAddress& src, const LRBAddress& dest, GBuf& buf)
{
	::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
}

void CLrbManager::SendToIBB(const LRBAddress& src, const LRBAddress& dest, GBuf& buf)
{
	::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
}

BOOL CLrbManager::SendToRKS(const LRBAddress& src, const LRBAddress& dest, GBuf& buf)
{
	return ::XLRBConnectorAsyncSendTo(0, buf, PROTOCOL_LDTP, (const LPLRBADDRESS)&src, (const LPLRBADDRESS)&dest);
}