/////////////////////////////////////////////////////////////////////////
// XLRBMsgHandlerEx.cpp

#include "StdAfx.h"
#include "Manager.h"
#include "LRBHandler.h"
#include "Category.h"
#include "ChannelSvrTable.h"
#include "GameStatTable.h"
#include "GameSvrTable.h"
#include "CharLobby.h"
#include "CharLobbyManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SendToServer
//void CLRBHandler::SendToCHS(const PayloadNCSCHS& pld, const LRBAddress& dest)
void CLRBHandler::SendToCHS(const PayloadNCSCHS& pld, const PayloadInfo& pldInfo)
{
	// protocol, dest type 어떻게 할건지 생각 
	// registerservicereq 일 경우 multicasting 된다 그외는 unicast
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		LOG (ERR, "CLRBHandler : --Error LBCHSMsg BStore FAIL--");
		return;
	}

	// m_addrRegistered 주소 바뀔수도 있음에 유의 
	ConnectorSendTo( pldInfo.dwMID, buf, PROTOCOL_LDTP, m_serverAddr, pldInfo.addr ) ;
}
void CLRBHandler::SendToNGS(const PayloadNCSNGS& pld, const PayloadInfo& pldInfo)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		LOG (ERR, "CLRBHandler : --Error NCSNGSMsg BStore FAIL--");
		return;
	}
	ConnectorSendTo( pldInfo.dwMID, buf, PROTOCOL_LDTP, m_serverAddr, pldInfo.addr ) ;
}
void CLRBHandler::SendToNLS(const PayloadNLSCLI& pld, const PayloadInfo& pldInfo)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		LOG (ERR, "CLRBHandler : --Error NLSCLIMsg BStore FAIL--");
		return;
	}
	ConnectorSendTo( pldInfo.dwMID, buf, PROTOCOL_LDTP, m_serverAddr, pldInfo.addr ) ;
}
void CLRBHandler::SendToNLS(const LRBAddress& src, const PayloadInfo& pldInfo, PayloadCLINLS& pld)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if (!bRet) {
		return;
	}
	if(pldInfo.addr.IsNULL())
	{
		LOG (ERR, "CLRBHandler : --Error NLSCLIMsg BStore FAIL--");
		return;
	}
	ConnectorSendTo( pldInfo.dwMID, buf, PROTOCOL_LDTP, m_serverAddr, pldInfo.addr ) ;	
}

void CLRBHandler::SendToNAS(const PayloadCLINAS& pld, const PayloadInfo& pldInfo)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if (!bRet)
	{
		LOG(ERR, "CLRBHandler : --Error NCSNASMsg BStore FAIL--");
		return;		
	}
	ConnectorSendTo(pldInfo.dwMID, buf, PROTOCOL_LDTP, m_serverAddr, pldInfo.addr);
}

void CLRBHandler::SendToGBS(const PayloadGBSCLI& pld, const PayloadInfo& pldInfo)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if(!bRet)
	{
		LOG (ERR, "CLRBHandler : --Error GBSCLIMsg BStore FAIL--");
		return;
	}
	ConnectorSendTo( pldInfo.dwMID, buf, PROTOCOL_LDTP, m_serverAddr, pldInfo.addr ) ;
}
void CLRBHandler::SendToGBS(const LRBAddress& src, const PayloadInfo& pldInfo, PayloadCLIGBS& pld)
{
	GBuf buf;
	BOOL bRet = pld.BStore(buf);
	if (!bRet) {
		return;
	}
	if(pldInfo.addr.IsNULL())
	{
		LOG (ERR, "CLRBHandler : --Error GBSCLIMsg BStore FAIL--");
		return;
	}
	ConnectorSendTo( pldInfo.dwMID, buf, PROTOCOL_LDTP, m_serverAddr, pldInfo.addr ) ;
}

void CLRBHandler::ConnectorSendTo(DWORD dwMID, GBuf& buf, WORD wProtocol, LRBAddress& SrcAddr, const LRBAddress& DestAddr)
{
	::XLRBConnectorAsyncSendTo(dwMID, buf, wProtocol, &SrcAddr, (LRBAddress*)&DestAddr);
}

//////////////////////////////////////////////////////////////////////////////
// CHS
BOOL CLRBHandler::OnRegisterServiceNtf(MsgCHSNCS_RegisterServiceNtf* pMsg, const PayloadInfo& pldInfo)
{
	BOOL bRet = theChannelSvrTable.AddChannelSvr(pMsg->m_CHSNCSRegister.m_lLogicalAddr, pMsg->m_CHSNCSRegister.m_nsapCHS);
	string sSrc;
	pMsg->m_CHSNCSRegister.m_lLogicalAddr.GetStringFormat(sSrc);
	LOG (INF, "CXLRBMsgHandler : CHS Service RegistNtf IP[",pMsg->m_CHSNCSRegister.m_nsapCHS.GetIP(),"] LogicAddr[",sSrc,"] ret [",bRet,"]");

	//already 
	if(!bRet)
	{
		LOG (INF, "CXLRBMsgHandler : Warning : already Registed CHS");
		return TRUE;
	}
	//새로 등록된 CHS에 channel을 할당, channel 생성 
	theServiceTable.EnableChannelInChSvr(pMsg->m_CHSNCSRegister.m_nsapCHS, pMsg->m_CHSNCSRegister.m_lLogicalAddr, pMsg->m_CHSNCSRegister.m_lstChRegInfo);
	return TRUE;
}

BOOL CLRBHandler::OnRegisterServiceAns(MsgCHSNCS_RegisterServiceAns* pMsg, const PayloadInfo& pldInfo)
{
	BOOL bRet = theChannelSvrTable.AddChannelSvr(pMsg->m_CHSNCSRegister.m_lLogicalAddr, pMsg->m_CHSNCSRegister.m_nsapCHS);
	string sSrc;
	pMsg->m_CHSNCSRegister.m_lLogicalAddr.GetStringFormat(sSrc);
	LOG (INF, "CXLRBMsgHandler : CHS Service RegistAns IP[", pMsg->m_CHSNCSRegister.m_nsapCHS.GetIP(),"] LogicAddr[",sSrc,"]");

	//already 
	if(!bRet)
	{
		LOG (ERR, "CXLRBMsgHandler : Warning : already Registed CHS");
		return TRUE;
	}
	theServiceTable.EnableChannelInChSvr(pMsg->m_CHSNCSRegister.m_nsapCHS, pMsg->m_CHSNCSRegister.m_lLogicalAddr, pMsg->m_CHSNCSRegister.m_lstChRegInfo);
	return TRUE;
}

BOOL CLRBHandler::OnChannelListReq(MsgCHSNCS_RChannelListReq* pMsg, const PayloadInfo& pldInfo)
{
	ASSERT(pMsg != NULL);
	ChannelInfoList lstChannel;
	lstChannel.clear();

	MsgNCSCHS_RChannelListAns mAns;
	mAns.m_channelID = pMsg->m_channelID;
	mAns.m_lCSN = pMsg->m_lCSN;

	ChannelID& findChannelID = pMsg->m_channelID;
	findChannelID.m_dwCategory = pMsg->m_lCategory;
	mAns.m_lIsAllCategory = pMsg->m_lCategory;

	if (-1 == mAns.m_lIsAllCategory) {
		ChannelBaseInfoList lstBaseInfo;
		theServiceTable.GetAllCategoryList(lstBaseInfo, NF_SSN);

		ForEachElmt(ChannelBaseInfoList, lstBaseInfo, it, ij) 
			mAns.m_lstChannelInfoList.push_back(ChannelInfo(*it, 0));
	}
	else
		theServiceTable.GetChannelList(findChannelID, mAns.m_lstChannelInfoList, mAns.m_lTotalUserCount, mAns.m_lTotalChannelCount);

	PayloadNCSCHS pld(PayloadNCSCHS::msgRChannelListAns_Tag, mAns);
	SendToCHS(pld, pldInfo);

	return TRUE;
}

BOOL CLRBHandler::OnCHSInfoNtf(MsgCHSNCS_CHSInfoNtf* pMsg)
{
	if(!theChannelSvrTable.UpdateChannelSvr(pMsg->m_lLogicalAddr, pMsg->m_lstChannelUpdateInfo))
	{
		//등록되지 않은 CHS로 부터 메시지가 도착했다.
		string sSrc;
		pMsg->m_lLogicalAddr.GetStringFormat(sSrc);
		LOG (ERR, "LRBConnector::OnCHSInfoNtf : This Msg From Not Registed CHS Svr[",sSrc,"] --");
	}
	if(!theServiceTable.UpdateChannelInfoList(pMsg->m_lstChannelUpdateInfo))
	{
		//등록되지 않은 CHS로 부터 메시지가 도착했다.
		string sSrc;
		pMsg->m_lLogicalAddr.GetStringFormat(sSrc);
		LOG (ERR, "LRBConnector::OnCHSInfoNtf : This Msg contain Non-Existing channel from CHS[",sSrc,"] --");
	}

	theGameStatTable.OnUpdateCHSStatInfo(pMsg->m_lstChannelUpdateInfo);
	return TRUE;
}

BOOL CLRBHandler::OnNGSInfoReq(MsgCHSNCS_NGSInfoReq* pMsg, const PayloadInfo& pldInfo)
{
	MsgNCSCHS_NGSInfoAns mAns;
	mAns.m_channelID = pMsg->m_channelID;
	mAns.m_lCSN = pMsg->m_lCSN;
	mAns.m_lType = pMsg->m_lType;

	NSAP nsap;
	LONG lCntOfGLS;

	BOOL bRet = theGameSvrTable.FindProperGameSvr(mAns.m_channelID.m_lSSN, nsap, 0, lCntOfGLS);
	if (!bRet)
	{
		LOG (INF, "--LRBConnector::OnRcvNGSInfoReq() ERROR - NO GAME SERVER[SSN : ",mAns.m_channelID.m_lSSN,"]");

		mAns.m_lErrorCode = CHSLB_ERR_NOTFOUND_GLS;
		PayloadNCSCHS pld(PayloadNCSCHS::msgNGSInfoAns_Tag, mAns);
		SendToCHS(pld, pldInfo);
	}
	else
	{
		mAns.m_nsapNGS = nsap;

		mAns.m_lErrorCode = CHSLB_ERR_NO;
		PayloadNCSCHS pld(PayloadNCSCHS::msgNGSInfoAns_Tag, mAns);
		SendToCHS(pld, pldInfo);
	}
	return TRUE;
}

BOOL CLRBHandler::OnNtfNFFriendAccept(MsgCHSNCS_NtfNFFriendAccept* pMsg, const PayloadInfo& pldInfo)
{
	GBuf buf;
	::LStore(buf, *pMsg);
	LPXBUF pXBuf = buf.Detach();
	VALIDATE(pXBuf);

	SendToCharLobby(pMsg->m_lReceiverCSN, (HSIGNAL)CCharLobby::CHARLOBBY_NOTIFY_ACCEPT_FRIEND_FROM_CHS, (WPARAM)pXBuf, (LPARAM)0);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// NGS
BOOL CLRBHandler::OnRegisterServiceNtf(MsgNGSNCS_RegisterServiceNtf* pMsg, const PayloadInfo& pldInfo)
{
	GLSInfo* pGLSInfo = new GLSInfo;
	pGLSInfo->m_NSAPInfo = pMsg->m_NGSNCSRegister.m_nsapNGS;
	pGLSInfo->SetLogicAddr(pMsg->m_NGSNCSRegister.m_lLogicalAddr);
	pGLSInfo->SetServiceTypeID((LONG)LOWORD(pMsg->m_NGSNCSRegister.m_lServiceTypeID));

	string sSrc;
	pMsg->m_NGSNCSRegister.m_lLogicalAddr.GetStringFormat(sSrc);
	LOG (INF, "CXLRBMsgHandler: GLS : OnRegisterServiceNtf : LogicAddr[",sSrc,"] ServiceTypeID [",(LONG)LOWORD(pMsg->m_NGSNCSRegister.m_lServiceTypeID),"]");

	BOOL bRet = theGameSvrTable.AddGameSvr((LONG)LOWORD(pMsg->m_NGSNCSRegister.m_lServiceTypeID), pGLSInfo);
	if(!bRet)
		LOG (ERR, "CXLRBMsgHandler : --Warning : already Registed NGS IP[",pGLSInfo->m_NSAPInfo.GetIP(),"], LogicAddr[",sSrc,"]");
	return TRUE;
}

BOOL CLRBHandler::OnRegisterServiceAns(MsgNGSNCS_RegisterServiceAns* pMsg, const PayloadInfo& pldInfo)
{
	GLSInfo* pGLSInfo = new GLSInfo;
	pGLSInfo->m_NSAPInfo = pMsg->m_NGSNCSRegister.m_nsapNGS;
	pGLSInfo->SetLogicAddr(pMsg->m_NGSNCSRegister.m_lLogicalAddr);
	pGLSInfo->SetServiceTypeID((LONG)LOWORD(pMsg->m_NGSNCSRegister.m_lServiceTypeID));
	string sSrc;
	pMsg->m_NGSNCSRegister.m_lLogicalAddr.GetStringFormat(sSrc);
	LOG (INF, "CXLRBMsgHandler: GLS : OnRegisterServiceAns : LogicAddr[",sSrc,"] ServiceTypeID [",(LONG)LOWORD(pMsg->m_NGSNCSRegister.m_lServiceTypeID),"]");

	BOOL bRet = theGameSvrTable.AddGameSvr((LONG)LOWORD(pMsg->m_NGSNCSRegister.m_lServiceTypeID), pGLSInfo);
	if(!bRet)
		LOG (ERR,  "CXLRBMsgHandler : --Warning : already Registed NGS IP[",pGLSInfo->m_NSAPInfo.GetIP(),"], LogicAddr[",sSrc,"]");
	return TRUE;
}

BOOL CLRBHandler::OnNGSInfoNtf(MsgNGSNCS_NGSInfoNtf* pMsg)
{
	BOOL bRet = theGameSvrTable.UpdateGameSvr((LONG)LOWORD(pMsg->m_lServiceTypeID), 
		pMsg->m_lLogicalAddr, 
		pMsg->m_nsapNGS, 
		pMsg->m_lRoomCount, 
		pMsg->m_lUserCount);
	if(!bRet)
		LOG (ERR, "CXLRBMsgHandler : -- Error LRBConnector::OnNGSInfoNtf");
	return TRUE;
}

BOOL CLRBHandler::OnNtfNFFriendAdd(MsgNGSNCS_NtfNFFriendAdd* pMsg)
{
	GBuf buf;
	::LStore(buf, *pMsg);
	LPXBUF pXBuf = buf.Detach();
	VALIDATE(pXBuf);

	SendToCharLobby(pMsg->m_lReceiverCSN, (HSIGNAL)CCharLobby::CHARLOBBY_ADDFRIEND_NOTIFY, (WPARAM)pXBuf, (LPARAM)0);

	return TRUE;
}

BOOL CLRBHandler::OnNtfNFLetterReceive(MsgNGSNCS_NtfNFLetterReceive* pMsg)
{
	GBuf buf;
	::LStore(buf, *pMsg);
	LPXBUF pXBuf = buf.Detach();
	VALIDATE(pXBuf);

	SendToCharLobby(pMsg->m_lReceiverCSN, (HSIGNAL)CCharLobby::CHARLOBBY_NOTIFY_NEW_LETTER_FROM_NGS, (WPARAM)pXBuf, (LPARAM)0);

	return TRUE;
}

BOOL CLRBHandler::OnNtfNFFriendAccept(MsgNGSNCS_NtfNFFriendAccept* pMsg)
{
	GBuf buf;
	::LStore(buf, *pMsg);
	LPXBUF pXBuf = buf.Detach();
	VALIDATE(pXBuf);

	SendToCharLobby(pMsg->m_lReceiverCSN, (HSIGNAL)CCharLobby::CHARLOBBY_NOTIFY_ACCEPT_FRIEND_FROM_NGS, (WPARAM)pXBuf, (LPARAM)0);

	return TRUE;
}

void CLRBHandler::OnSvrTerminateNtf(LRBAddress& lrbAddr)
{
	// 서버 타입 별로 나누기 프로토콜 타입, 메시지 타입 확인

	string srcAddr;
	lrbAddr.GetStringFormat(srcAddr);

	LOG (INF, "*** OnSvrTerminateNtf LRBAddress ", srcAddr);

	if (CASTTYPE_UNICAST == lrbAddr.GetCastType())
	{
		BYTE cSvcCat = lrbAddr.GetServiceCategory(); //service category 
		switch(cSvcCat)
		{
		case SVCCAT_CHS:
			break;
		case SVCCAT_NGS:
			{
				LOG (INF, "CXLRBMsgHandler : Terminate INFO : Server Type : [",SVCCAT_NGS,"] ServiceTypdID[",(int)(char)lrbAddr.addr[14],"] LogicAddr[",srcAddr,"]");
				BOOL bRet = theGameSvrTable.DeleteGameSvr(lrbAddr);
				if(!bRet)
					LOG (ERR, "CXLRBMsgHandler : -------GLS Not Terminated : not found gls ServiceTypeID[",(int)(char)lrbAddr.addr[14],"] LoggicAddr[",srcAddr,"]--");

				break;
			}
			break;
		case SVCCAT_ELB:
			break;
		case SVCCAT_LRB:
			break;
		default:
			break;
		}
	}
}

#define NCS_BINARY_PATH		_T("C:\\Pmang\\NCS")

BOOL CLRBHandler::GetRealIPAddressFile(string& sIP)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	TCHAR szFileName[1024] = {0x00};
	_stprintf(szFileName, _T("%s\\get_realip.cmd"), NCS_BINARY_PATH);

	if (!CreateProcess(NULL, szFileName, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		theLog.Put(ERR, " ExecuteBatchJob - CreateProcess Failed!!! ");
		return FALSE;
	}

	// Wait until child process exits.
	DWORD dwRet = WaitForSingleObject( pi.hProcess, INFINITE );
	if (dwRet == WAIT_OBJECT_0)
	{
		_stprintf(szFileName, _T("%s\\result_realip.txt"), NCS_BINARY_PATH);

		// 배치잡에서 만들어 놓은 파일을 읽는다.
		HANDLE hFile = ::CreateFileW(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) 
		{
			theLog.Put(ERR, "Do Not Create realipaddress.txt !! ", ::GetLastError());
			return FALSE;
		}

		char szTemp[1024] = {0x00};
		DWORD dwReadBytes = 1024, dwOutReadBytes = 0;
		BOOL bRes = ::ReadFile(hFile, szTemp, dwReadBytes, &dwOutReadBytes, NULL);
		if (FALSE == bRes)
		{
			theLog.Put(ERR, "Do Not ReadFile realipaddress.txt !!");
			return FALSE;
		}

		sIP.assign(szTemp);

		::CloseHandle(hFile);
	}
	else 
		return FALSE;

	return TRUE;
}

void CLRBHandler::SendToCharLobby(LONG lCSN, HSIGNAL hSignal, WPARAM w, LPARAM l)
{
	CCharLobby* pCharLobby = theCharLobbyManager.FindCharLobby(lCSN);
	if (NULL != pCharLobby)
		::XsigQueueSignal(GetThreadPool(), pCharLobby, hSignal, w, l);
}