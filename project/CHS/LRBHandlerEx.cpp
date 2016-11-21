
// LRBHandlerEx.cpp


#include "stdafx.h"
#include "Common.h"
#include "LRBHandler.h"
#include "ChannelDir.h"
#include "Control.h"
#include "Listener.h"
#include "StatisticsTable.h"
#include "Reporter.h"

///////////////////////////////////////////////////////////////////////////////
// CLRBHandlerEx
///////////////////////////////////////////////////////////////////////////////

void CLRBHandler::OnRcvNGS(const PayloadInfo& dest, GBuf& buf, WORD wMessageType, WORD wProtocol)
{
	PayloadNGSCHS pld;
	if(!::BLoad(pld, buf))
	{
		LOG(INF_UK, "CHS_CLRBHandler"_LK, " !!!!!!!! OnRcvGLS : BLoad Error");
		return;
	}

	switch(pld.mTagID)
	{
	case PayloadNGSCHS::msgRoomCreateNtf_Tag:
		OnRoomCreateNtf(pld, dest);
		m_MsgCount.m_dwAddRoom++;
		break;
	case PayloadNGSCHS::msgUserRoomLeaveNtf_Tag:
		OnUserRoomLeaveNtf(pld);
		m_MsgCount.m_dwLeaveUser++;
		break;
	case PayloadNGSCHS::msgUserRoomJoinNtf_Tag:
		OnUserRoomJoinNtf(pld, dest);
		m_MsgCount.m_dwJoinUser++;
		break;
	case PayloadNGSCHS::msgRoomDeleteNtf_Tag:
		OnRoomDeleteNtf(pld);
		m_MsgCount.m_dwRemRoom++;
		break;
	case PayloadNGSCHS::msgRoomStatusChangeNtf_Tag:
		OnRoomStatusChangeNtf(pld);
		m_MsgCount.m_dwRoomState++;
		break;
	case PayloadNGSCHS::msgUserInfoInRoomChangeNtf_Tag:
		OnUserInfoInRoomChangeNtf(pld);
		break;
	case PayloadNGSCHS::msgChangeRoomOptionNtf_Tag:
		OnChangeRoomOptionNtf(pld);
		m_MsgCount.m_dwRoomOption++;
		break;
	case PayloadNGSCHS::msgChangeGameOptionNtf_Tag:
		OnChangeGameOptionNtf(pld);
		m_MsgCount.m_dwGameOption++;
		break;
	case PayloadNGSCHS::msgReconnectNtf_Tag:
		OnGLSReconnectNtf(pld, dest);
		break;
	case PayloadNGSCHS::msgChangeLoginStateNtf_Tag:
		OnChangeLoginStateNtf(pld);
		break;
	case PayloadNGSCHS::msgChannelIDListReq_Tag:
		OnChannelIDListReq(pld, dest);
		break;
	case PayloadNGSCHS::msgGameRoomListAns_Tag:
		OnGameRoomListAns(pld, dest);
		break;
	case PayloadNGSCHS::msgReqFreeRoomInfo_Tag:
		OnReqFreeRoomInfo(pld, dest);
		break;
	case PayloadNGSCHS::msgNtfNFFriendAdd_Tag:
		OnNtfNFFriendAdd(pld, dest);
		break;
	case PayloadNGSCHS::msgNtfNFFriendAccept_Tag:
		OnNtfNFFriendAccept(pld, dest);
		break;
	case PayloadNGSCHS::msgNtfNFLetterReceive_Tag:
		OnNtfNFLetterReceive(pld, dest);
		break;
	default:
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "++++ Received Unknown Message from GLS ++++ [Tag:", pld.mTagID, "]");
		break;
	}
}

////////////////   미리 만들어져 있는 방정보를 초기에 GLS로부터 얻어와야 한다.  ///////////
////////////////   OnRoomListAns는 Req를 GLS로 전송 했을 경우 받는 메세지		///////////
void CLRBHandler::OnReqFreeRoomInfo(PayloadNGSCHS & pld, const PayloadInfo & dest)
{
	ASSERT(pld.un.m_msgReqFreeRoomInfo);
	if(!pld.un.m_msgReqFreeRoomInfo)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnReqFreeRoomInfo +++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgReqFreeRoomInfo->m_roomID.m_lSSN, pld.un.m_msgReqFreeRoomInfo->m_roomID.m_dwCategory, pld.un.m_msgReqFreeRoomInfo->m_roomID.m_dwGCIID);

	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);	
}

void CLRBHandler::OnNtfNFFriendAdd(PayloadNGSCHS& pld, const PayloadInfo& dest)
{
	RCPtrT<CChannel> spChannel;
	if(theChannelDir.GetChannel(pld.un.m_msgNtfNFFriendAdd->m_channelID, &spChannel)) 
	{
		MsgCHSCli_NtfNFFriendAdd ntf;
		ntf.m_strCharName = pld.un.m_msgNtfNFFriendAdd->m_strSender;

		GBuf buf;
		::LStore(buf, ntf);
		spChannel->PostNtfNFFriendAdd(pld.un.m_msgNtfNFFriendAdd->m_lReceiverCSN, buf);
	}
}

void CLRBHandler::OnNtfNFLetterReceive(PayloadNGSCHS& pld, const PayloadInfo& dest)
{
	RCPtrT<CChannel> spChannel;
	if(theChannelDir.GetChannel(pld.un.m_msgNtfNFLetterReceive->m_channelID, &spChannel)) 
	{
		spChannel->PostNtfNFLetterReceive(pld.un.m_msgNtfNFLetterReceive->m_lReceiverCSN);
	}
}

void CLRBHandler::OnNtfNFFriendAccept(PayloadNGSCHS& pld, const PayloadInfo& dest)
{
	RCPtrT<CChannel> spChannel;
	if(theChannelDir.GetChannel(pld.un.m_msgNtfNFFriendAccept->m_channelID, &spChannel)) 
	{
		MsgCHSCli_NtfNFFriendAccept ntf;
		ntf.m_nfFriend = pld.un.m_msgNtfNFFriendAccept->m_nfFriend;		

		GBuf buf;
		::LStore(buf, ntf);
		spChannel->PostNtfNFFriendAccept(pld.un.m_msgNtfNFFriendAccept->m_lReceiverCSN, buf);
	}
}

void CLRBHandler::OnGameRoomListAns(PayloadNGSCHS & pld, const PayloadInfo & dest)
{
	ASSERT(pld.un.m_msgGameRoomListAns);
	if(!pld.un.m_msgGameRoomListAns)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnRoomListNtfAndAns +++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgGameRoomListAns->m_channelID.m_lSSN, pld.un.m_msgGameRoomListAns->m_channelID.m_dwCategory, pld.un.m_msgGameRoomListAns->m_channelID.m_dwGCIID);

	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);	
	
	if(pld.mTagID == PayloadNGSCHS::msgGameRoomListAns_Tag)
	{
		MsgNGSCHS_GameRoomListAns* msg = pld.un.m_msgGameRoomListAns;
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ CLRBHandler::OnGameRoomListAns -> Pachingko game : [" ,dest.addr.GetString().c_str() , "] +++++");
		theChannelDir.AddGLSLogicalAddr(dest.addr, msg->m_lstRoomInfo.begin()->m_nsapGLS);
	}
}

void CLRBHandler::OnChannelIDListReq(PayloadNGSCHS & pld, const PayloadInfo& dest)
{
	ASSERT(pld.un.m_msgChannelIDListReq);
	if(!pld.un.m_msgChannelIDListReq)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnChannelIDListReq +++++");
		return;
	}

	MsgCHSNGS_ChannelIDList		msgChannelIDList;
	theChannelDir.OnChannelDListReq(pld.un.m_msgChannelIDListReq->m_lSSN, msgChannelIDList);
	PayloadCHSNGS Send_pld(PayloadCHSNGS::msgChannelIDList_Tag, MsgCHSNGS_ChannelIDList(msgChannelIDList));

	SendToPCCNGS(Send_pld, dest);
}

void CLRBHandler::OnRoomCreateNtf(PayloadNGSCHS & pld, const PayloadInfo & dest)
{
	ASSERT(pld.un.m_msgRoomCreateNtf);
	if(!pld.un.m_msgRoomCreateNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnRoomCreateNtf+++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgRoomCreateNtf->m_roomID.m_lSSN, pld.un.m_msgRoomCreateNtf->m_roomID.m_dwCategory, pld.un.m_msgRoomCreateNtf->m_roomID.m_dwGCIID);

	theChannelDir.AddGLSLogicalAddr(dest.addr, pld.un.m_msgRoomCreateNtf->m_nfRoomBaseInfo.m_nsapGLS);

	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnRoomCreateNtf Not Found Channel+++++");
		return;
	}

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnRoomDeleteNtf(PayloadNGSCHS & pld)	
{
	ASSERT(pld.un.m_msgRoomDeleteNtf);
	if(!pld.un.m_msgRoomDeleteNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnRoomDeleteNtf+++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgRoomDeleteNtf->m_roomID.m_lSSN, pld.un.m_msgRoomDeleteNtf->m_roomID.m_dwCategory, pld.un.m_msgRoomDeleteNtf->m_roomID.m_dwGCIID);
	
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnRoomStatusChangeNtf(PayloadNGSCHS & pld)	
{
	ASSERT(pld.un.m_msgRoomStatusChangeNtf);
	if(!pld.un.m_msgRoomStatusChangeNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT :OnRoomStatusChangeNtf +++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgRoomStatusChangeNtf->m_roomID.m_lSSN, pld.un.m_msgRoomStatusChangeNtf->m_roomID.m_dwCategory, pld.un.m_msgRoomStatusChangeNtf->m_roomID.m_dwGCIID);
	
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnUserRoomJoinNtf(PayloadNGSCHS & pld, const PayloadInfo & dest)	
{
	ASSERT(pld.un.m_msgUserRoomJoinNtf);
	if(!pld.un.m_msgUserRoomJoinNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT :OnUserRoomJoinNtf +++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgUserRoomJoinNtf->m_roomID.m_lSSN, pld.un.m_msgUserRoomJoinNtf->m_roomID.m_dwCategory, pld.un.m_msgUserRoomJoinNtf->m_roomID.m_dwGCIID);

	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnUserRoomLeaveNtf(PayloadNGSCHS & pld)	
{
	ASSERT(pld.un.m_msgUserRoomLeaveNtf);
	if(!pld.un.m_msgUserRoomLeaveNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnUserRoomLeaveNtf+++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgUserRoomLeaveNtf->m_roomID.m_lSSN, pld.un.m_msgUserRoomLeaveNtf->m_roomID.m_dwCategory, pld.un.m_msgUserRoomLeaveNtf->m_roomID.m_dwGCIID);

	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnUserInfoInRoomChangeNtf(PayloadNGSCHS & pld)	
{
	ASSERT(pld.un.m_msgUserInfoInRoomChangeNtf);
	if(!pld.un.m_msgUserInfoInRoomChangeNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnUserInfoInRoomChangeNtf+++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgUserInfoInRoomChangeNtf->m_roomID.m_lSSN, pld.un.m_msgUserInfoInRoomChangeNtf->m_roomID.m_dwCategory, pld.un.m_msgUserInfoInRoomChangeNtf->m_roomID.m_dwGCIID);
	
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;
	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnChangeRoomOptionNtf(PayloadNGSCHS & pld)
{
	ASSERT(pld.un.m_msgChangeRoomOptionNtf);
	if(!pld.un.m_msgChangeRoomOptionNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnChangeRoomOptionNtf+++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgChangeRoomOptionNtf->m_roomID.m_lSSN, pld.un.m_msgChangeRoomOptionNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeRoomOptionNtf->m_roomID.m_dwGCIID);
	
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnChangeGameOptionNtf(PayloadNGSCHS & pld)
{
	ASSERT(pld.un.m_msgChangeGameOptionNtf);
	if(!pld.un.m_msgChangeGameOptionNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnChangeGameOptionNtf+++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgChangeGameOptionNtf->m_roomID.m_lSSN, pld.un.m_msgChangeGameOptionNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeGameOptionNtf->m_roomID.m_dwGCIID);
	
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnChangeLoginStateNtf(PayloadNGSCHS & pld)
{
	ASSERT(pld.un.m_msgChangeLoginStateNtf);
	if(!pld.un.m_msgChangeLoginStateNtf)
	{
		LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "+++++ ASSERT : OnChangeLoginStateNtf+++++");
		return;
	}
	ChannelID channelID(pld.un.m_msgChangeLoginStateNtf->m_roomID.m_lSSN, pld.un.m_msgChangeLoginStateNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeLoginStateNtf->m_roomID.m_dwGCIID);
	
	CChannelPtr spChannel;
	if(!theChannelDir.GetChannel(channelID, &spChannel))
		return;

	GBuf buf;
	VALIDATE(::BStore(buf, pld));
	spChannel->PushNGSMessage(buf);
}

void CLRBHandler::OnGLSReconnectNtf(PayloadNGSCHS & pld, const PayloadInfo & dest)
{
	string sIP;
	pld.un.m_msgReconnectNtf->m_nsapNGS.GetIP(sIP);
	LOG(INF_UK, "CHS_CLRBHandlerEx"_LK, "==== GLS ReconnectNtf : IP = ", sIP.c_str());

	theChannelDir.AddGLSLogicalAddr(dest.addr, pld.un.m_msgReconnectNtf->m_nsapNGS, TRUE);
}