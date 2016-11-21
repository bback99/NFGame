//
//// LRBManagerEx.cpp
//
//
//#include "stdafx.h"
//#include "Common.h"
//#include "LRBManager.h"
//#include "ChannelDir.h"
//// #include "ChannelTable.h"
//#include "Control.h"
//#include "Listener.h"
//#include "StatisticsTable.h"
//#include "Reporter.h"
//
/////////////////////////////////////////////////////////////////////////////////
//// CLRBManagerEx
/////////////////////////////////////////////////////////////////////////////////
//
//void CLRBManager::OnRecvGLS(const MsgService * pMsg)
//{
//	GBuf buf( (LPVOID) pMsg->m_sRealMsg.c_str(), pMsg->m_sRealMsg.length() );
//	PayloadGLSCHS pld;
//	VALIDATE(::BLoad(pld, buf));
//
//	switch(pld.mTagID)
//	{
//	case PayloadGLSCHS::msgRoomCreateNtf_Tag:
//		OnRoomCreateNtf(pld, pMsg->m_header.m_dwSourceAddr);
//		m_MsgCount.m_dwAddRoom++;
//		break;
//	case PayloadGLSCHS::msgUserRoomLeaveNtf_Tag:
//		OnUserRoomLeaveNtf(pld);
//		m_MsgCount.m_dwLeaveUser++;
//		break;
//	case PayloadGLSCHS::msgUserRoomJoinNtf_Tag:
//		OnUserRoomJoinNtf(pld, pMsg->m_header.m_dwSourceAddr);
//		m_MsgCount.m_dwJoinUser++;
//		break;
//	case PayloadGLSCHS::msgRoomDeleteNtf_Tag:
//		OnRoomDeleteNtf(pld);
//		m_MsgCount.m_dwRemRoom++;
//		break;
//	case PayloadGLSCHS::msgRoomStatusChangeNtf_Tag:
//		OnRoomStatusChangeNtf(pld);
//		m_MsgCount.m_dwRoomState++;
//		break;
//	case PayloadGLSCHS::msgUserInfoInRoomChangeNtf_Tag:
//		OnUserInfoInRoomChangeNtf(pld);
//		break;
//	case PayloadGLSCHS::msgChangeAvatarNtf_Tag:
//		OnChangeAvatarNtf(pld);
//		m_MsgCount.m_dwAvatarChange++;
//		break;
//	case PayloadGLSCHS::msgChangeNickNameNtf_Tag:
//		OnChangeNickNameNtf(pld);
//		break;
//	case PayloadGLSCHS::msgChangeItemNtf_Tag:
//		OnChangeItemNtf(pld);
//		break;
//	case PayloadGLSCHS::msgChangeRoomOptionNtf_Tag:
//		OnChangeRoomOptionNtf(pld);
//		m_MsgCount.m_dwRoomOption++;
//		break;
//	case PayloadGLSCHS::msgChangeGameOptionNtf_Tag:
//		OnChangeGameOptionNtf(pld);
//		m_MsgCount.m_dwGameOption++;
//		break;
//	case PayloadGLSCHS::msgReconnectNtf_Tag:
//		OnGLSReconnectNtf(pld, pMsg->m_header.m_dwSourceAddr);
//		break;
//	case PayloadGLSCHS::msgChangeLoginStateNtf_Tag:
//		OnChangeLoginStateNtf(pld);
//		break;
//	default:
//		TLOG0("Received Unknown Message from GLS \n");
//		theErr.LOG(1, "++++ Received Unknown Message from GLS ++++");
//		break;
//	}
//
//}
//
//void CLRBManager::OnRoomCreateNtf(PayloadGLSCHS & pld, DWORD dwLogicalAddr)
//{
//	ASSERT(pld.un.m_msgRoomCreateNtf);
//	if(!pld.un.m_msgRoomCreateNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnRoomCreateNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgRoomCreateNtf->m_roomID.m_lSSN, pld.un.m_msgRoomCreateNtf->m_roomID.m_dwCategory, pld.un.m_msgRoomCreateNtf->m_roomID.m_dwGCIID);
//
//	theChannelDir.AddGLSLogicalAddr(dwLogicalAddr, pld.un.m_msgRoomCreateNtf->m_roombaseinfo.m_nsapGLS);
//
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnRoomDeleteNtf(PayloadGLSCHS & pld)	
//{
//	ASSERT(pld.un.m_msgRoomDeleteNtf);
//	if(!pld.un.m_msgRoomDeleteNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnRoomDeleteNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgRoomDeleteNtf->m_roomID.m_lSSN, pld.un.m_msgRoomDeleteNtf->m_roomID.m_dwCategory, pld.un.m_msgRoomDeleteNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnRoomStatusChangeNtf(PayloadGLSCHS & pld)	
//{
//	ASSERT(pld.un.m_msgRoomStatusChangeNtf);
//	if(!pld.un.m_msgRoomStatusChangeNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT :OnRoomStatusChangeNtf +++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgRoomStatusChangeNtf->m_roomID.m_lSSN, pld.un.m_msgRoomStatusChangeNtf->m_roomID.m_dwCategory, pld.un.m_msgRoomStatusChangeNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnUserRoomJoinNtf(PayloadGLSCHS & pld, DWORD dwLogicalAddr)	
//{
//	ASSERT(pld.un.m_msgUserRoomJoinNtf);
//	if(!pld.un.m_msgUserRoomJoinNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT :OnUserRoomJoinNtf +++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgUserRoomJoinNtf->m_roomID.m_lSSN, pld.un.m_msgUserRoomJoinNtf->m_roomID.m_dwCategory, pld.un.m_msgUserRoomJoinNtf->m_roomID.m_dwGCIID);
//
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnUserRoomLeaveNtf(PayloadGLSCHS & pld)	
//{
//	ASSERT(pld.un.m_msgUserRoomLeaveNtf);
//	if(!pld.un.m_msgUserRoomLeaveNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnUserRoomLeaveNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgUserRoomLeaveNtf->m_roomID.m_lSSN, pld.un.m_msgUserRoomLeaveNtf->m_roomID.m_dwCategory, pld.un.m_msgUserRoomLeaveNtf->m_roomID.m_dwGCIID);
//
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnUserInfoInRoomChangeNtf(PayloadGLSCHS & pld)	
//{
//	ASSERT(pld.un.m_msgUserInfoInRoomChangeNtf);
//	if(!pld.un.m_msgUserInfoInRoomChangeNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnUserInfoInRoomChangeNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgUserInfoInRoomChangeNtf->m_roomID.m_lSSN, pld.un.m_msgUserInfoInRoomChangeNtf->m_roomID.m_dwCategory, pld.un.m_msgUserInfoInRoomChangeNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnChangeAvatarNtf(PayloadGLSCHS & pld)
//{
//	ASSERT(pld.un.m_msgChangeAvatarNtf);
//	if(!pld.un.m_msgChangeAvatarNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnChangeAvatarNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgChangeAvatarNtf->m_roomID.m_lSSN, pld.un.m_msgChangeAvatarNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeAvatarNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnChangeNickNameNtf(PayloadGLSCHS & pld)
//{
//	ASSERT(pld.un.m_msgChangeNickNameNtf);
//	if(!pld.un.m_msgChangeNickNameNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnChangeNickNameNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgChangeNickNameNtf->m_roomID.m_lSSN, pld.un.m_msgChangeNickNameNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeNickNameNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnChangeItemNtf(PayloadGLSCHS & pld)
//{
//	ASSERT(pld.un.m_msgChangeItemNtf);
//	if(!pld.un.m_msgChangeItemNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnChangeItemNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgChangeItemNtf->m_roomID.m_lSSN, pld.un.m_msgChangeItemNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeItemNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
////	GBuf buf;
////	VALIDATE(::BStore(buf, pld));
////	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnChangeRoomOptionNtf(PayloadGLSCHS & pld)
//{
//	ASSERT(pld.un.m_msgChangeRoomOptionNtf);
//	if(!pld.un.m_msgChangeRoomOptionNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnChangeRoomOptionNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgChangeRoomOptionNtf->m_roomID.m_lSSN, pld.un.m_msgChangeRoomOptionNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeRoomOptionNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnChangeGameOptionNtf(PayloadGLSCHS & pld)
//{
//	ASSERT(pld.un.m_msgChangeGameOptionNtf);
//	if(!pld.un.m_msgChangeGameOptionNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnChangeGameOptionNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgChangeGameOptionNtf->m_roomID.m_lSSN, pld.un.m_msgChangeGameOptionNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeGameOptionNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnChangeLoginStateNtf(PayloadGLSCHS & pld)
//{
//	ASSERT(pld.un.m_msgChangeLoginStateNtf);
//	if(!pld.un.m_msgChangeLoginStateNtf)
//	{
//		theErr.LOG(1, "+++++ ASSERT : OnChangeLoginStateNtf+++++");
//		return;
//	}
//	ChannelID channelID(pld.un.m_msgChangeLoginStateNtf->m_roomID.m_lSSN, pld.un.m_msgChangeLoginStateNtf->m_roomID.m_dwCategory, pld.un.m_msgChangeLoginStateNtf->m_roomID.m_dwGCIID);
//	
//	CChannelPtr spChannel;
//	if(!theChannelDir.GetChannel(channelID, &spChannel))
//		return;
//
//	GBuf buf;
//	VALIDATE(::BStore(buf, pld));
//	spChannel->PushGLSMessage(buf);
//}
//
//void CLRBManager::OnGLSReconnectNtf(PayloadGLSCHS & pld, DWORD dwLogicalAddr)
//{
//	string sIP;
//	pld.un.m_msgReconnectNtf->m_nsapGLS.GetIP(sIP);
//	theErr.LOG(1, "==== GLS ReconnectNtf : %s IP ======", sIP.c_str());
//
//	theChannelDir.AddGLSLogicalAddr(dwLogicalAddr, pld.un.m_msgReconnectNtf->m_nsapGLS, TRUE);
//}
//
