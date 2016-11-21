// ChatAgent.cpp: implementation of the CChatAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChatAgent.h"
#include "Control.h"

CChatLink::CChatLink()
{

}

CChatLink::~CChatLink()
{

}

BOOL CChatLink::IsBufOverflow()
{
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CChatAgent theChatAgent;

void __stdcall _ChatListenerCallback(DWORD dwHandle, SOCKET hSocket, int nErrorCode, LPCSTR lpRemoteAddr, LONG lRemotePort, LPVOID lpContext)
{
	theChatAgent.OnListenerAccept(hSocket, nErrorCode, lpRemoteAddr, lRemotePort);
}

CChatAgent::CChatAgent()
{
	m_bCAStarting = FALSE;
	m_lConnectedUserCnt = 0L;
	m_lChatType = 0L;
	m_bCVRunning = FALSE;
}

CChatAgent::~CChatAgent()
{
}

BOOL CChatAgent::RunChatAgent(int nPort)
{
	if(m_bCAStarting)
		return TRUE;

	if(!InitCAInfo())
	{
		LOG(INF_UK, "CHS_CChatAgent"_LK, "---------------fail Initialize Chat Agent-----------------");
		return FALSE;
	}

	BOOL bRet = TRUE;
	bRet = bRet && ::XlstnCreate(&m_dwListener, &_ChatListenerCallback, nPort, NULL, NULL);
	VALIDATE(bRet);

	bRet = bRet && m_timerSendCycle.Activate(GetThreadPool(), this, 10000, 1000);
	VALIDATE(bRet);

	if(!bRet)
	{
		StopChatAgent();
		return FALSE;
	}
	m_bCAStarting = TRUE;
	LOG(INF_UK, "CHS_CChatAgent"_LK, "*** start Chat Agent.. ***");
	return TRUE;
}

BOOL CChatAgent::StopChatAgent()
{
	TLock lo(this);
	if(m_dwListener)
	{
		::XlstnDestroy(m_dwListener);
		m_dwListener = 0;
	}

	ForEachElmt(TLinkMap, mLinkMap, i, j)
	{
		TLink* pLink = i->second;
		pLink->Unregister();
		delete pLink;
	}
	mLinkMap.clear();


	m_bCAStarting = FALSE;
	return TRUE;
}

STDMETHODIMP_(void) CChatAgent::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if(m_timerSendCycle.IsHandle(hSignal))	
	{
		TLock lo(this);
		SendChatHistory();
	}
	else
	{
		TLock lo(this);
		TBase::OnSignal(hSignal, wParam, lParam);
	}
}

BOOL CChatAgent::OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR lpAddr, LONG lPort)
{
	if(nErrorCode)
		return FALSE;

	CChatLink* pLink = new CChatLink;
	VALIDATE(pLink);
	if(!pLink) 
		return FALSE;
	pLink->SetIP(lpAddr);

	BOOL bRet = pLink->Register(hSocket, NULL, HSIGNAL_XLINKHANDLER);
	if (!bRet)
	{
		::closesocket(hSocket);
		delete(pLink);
		return FALSE;
	}
	TLock lo(this);
	VERIFY(AddLink(pLink));
	LOG(INF_UK, "CHS_CChatAgent"_LK, "CA ACCESS CHECK : ", lpAddr, " IP Connecting");
	return TRUE;
}

BOOL CChatAgent::OnError(CChatLink* pLink, long lEvent, int nErrorCode)
{
	// 접속해제 로그를 남기고.. 
	if(!pLink) return FALSE;
	xstring _temp(pLink->m_sUserID.c_str());

	if(FindLink(pLink->m_sUserID))
	{
		LOG(INF_UK, "CHS_CChatAgent"_LK, "CA ACCESS CHECK : ", pLink->m_sUserID, " User Disconnected");
		DestroyLink(pLink);
		m_lConnectedUserCnt--;
	}
	else 
	{
		RemoveLink(pLink);
		pLink->Unregister();
		delete pLink;
	}

	if(m_listActiveLink.size() < 1)
		m_bCVRunning = FALSE;
	return FALSE;
}

BOOL CChatAgent::OnRcvMsg(CChatLink* pLink, PayloadCVCA& pld)
{
	BOOL bRet = TRUE;
	switch(pld.mTagID)
	{
	case PayloadCVCA::msgCVLoginReq_Tag:
		bRet = OnRecvLoginReq(pLink, *pld.un.m_msgCVLoginReq);	
		break;
	default:
		LOG(INF_UK, "CHS_CChatAgent"_LK, "++++ CChatAgent  received Invalid Message Tag ");
		bRet = OnError(pLink, FD_READ, pld.mTagID );
		break;
	}
	return bRet;
}

void CChatAgent::SendChatHistory()	// 모든 queue의 data를 전송.
{
	LONG lSize = m_queueChat.Count();
	if(lSize < 1)
		return;

// ChatFlags에 의해서 .. 부분별로 전송.. 
	GBuf buf; 
	ChatFlags flags;
	for(int i = 0; i < lSize; i++)
	{
		m_queueChat.Pop(flags, buf);
		SendToAllLink(flags, buf);
		buf.Clear();
	}
}

void CChatAgent::SendToAllLink(ChatFlags & flags, GBuf & buf)
{
	ForEachElmt(CALinkGroupT, m_listActiveLink, it, jt)
	{
		CChatLink * pLink = *it; 
		if(!pLink) continue;

		if(!IsMatchFlags(flags, pLink))
			continue;

		if(pLink->IsBufOverflow())
		{
			//::XsigQueueSignal(GetThreadPool(), this, 
			DestroyLink(pLink);		// 지우고.. send 작업을 멈춘다(일부 data 손실 .. 상관없음.)
			return;
		}

		pLink->DoSend(buf);
	}
}

BOOL CChatAgent::IsMatchFlags(ChatFlags & flags, CChatLink * pLink)
{
	if(pLink->m_lChatType == 1L)	// 모든 메세지 요구(귓속말까지..)
	{
		if(pLink->m_lSSN < 0L)	// 모든 서비스의 메세지 요구.
		{
			if(pLink->m_dwCategory < 1UL)	// 모든 category의 메세지 요구.
				return TRUE;
			else
				return (pLink->m_dwCategory == flags.m_dwCategory); 
		}
		else if(pLink->m_lSSN == flags.m_lSSN)
		{
			if(pLink->m_dwCategory < 1UL)
				return TRUE;
			else
				return (pLink->m_dwCategory == flags.m_dwCategory); 
		}
		else 
			return FALSE;
	}
	else if(pLink->m_lChatType == flags.m_lMsgType)
	{
		if(pLink->m_lSSN < 0L)	
		{
			if(pLink->m_dwCategory < 1UL)
				return TRUE;
			else
				return (pLink->m_dwCategory == flags.m_dwCategory); 
		}
		else if(pLink->m_lSSN == flags.m_lSSN)
		{
			if(pLink->m_dwCategory < 1UL)
				return TRUE;
			else
				return (pLink->m_dwCategory == flags.m_dwCategory); 
		}
		else 
			return FALSE;
	}

	return FALSE;
}

void CChatAgent::SendMsg(CChatLink* pLink, const PayloadCACV& pld)
{
	if(!pLink) 
		return;
	GBuf buf; 
	::LStore(buf, pld);

	pLink->DoSend(buf);
}

BOOL CChatAgent::OnRecvLoginReq(CChatLink * pLink, MsgCVCA_CVLoginReq & pld)
{
	// 인증 및 결과를 client에 전달.
	if(m_lConnectedUserCnt >= 10L)  // 접속 허용 수..인증이 완료된 사용자를 넘어서면 접속을 해제한다.
	{
		if(pld.m_lUserLevel == 0L) 
		{
			SendLoginAns(pLink, pld.m_sUserID, 1L); 
			return TRUE;
		}
		else if(pld.m_lUserLevel == 1L)	// 이전 접속자 접속해제..
		{
			CChatLink * pLink = FindLink(pld.m_sUserID);
			if(pLink)
			{
				SendDisconnectNtf(pLink, 2L, pld.m_sUserID, pld.m_sOption); // 응답은 없다.
				DestroyLink(pLink);
				m_lConnectedUserCnt--;
			}
		}
		else 
		{	
			SendLoginAns(pLink, pld.m_sUserID, 1L); 
			LOG(INF_UK, "CHS_CChatAgent"_LK, "Unknown User level .. from CA.. Login..", pld.m_lUserLevel);
			return TRUE;
		}
	}

	if(!IsCertifier(pld.m_sUserID, pld.m_sPassword))	// 사용자 인증
	{
		SendLoginAns(pLink, pld.m_sUserID, 2L);
		return TRUE;
	}

	if(FindLink(pld.m_sUserID))
	{
		SendLoginAns(pLink, pld.m_sUserID, 4L);
		return TRUE;
	}

	// 인증된 사용자가 요구한 정보 셋팅.
	pLink->m_sUserID = pld.m_sUserID.c_str();
	pLink->m_lSSN = pld.m_lSSN;
	pLink->m_dwCategory = pld.m_dwCategory;
	pLink->m_lChatType = pld.m_lReqType;
	pLink->m_sOption = pld.m_sOption.c_str();

	AddCALink(pLink);	

	SendLoginAns(pLink, pld.m_sUserID, 0L);
	m_lConnectedUserCnt++;
	m_bCVRunning = TRUE;
	string logstr = format("CA ACCESS CHECK : %s IP, %s User Connected %d SSN", (pLink->GetIP()).c_str(), pLink->m_sUserID.c_str(), pld.m_lSSN);
	LOG(INF_UK, "CHS_CChatAgent"_LK, logstr);
	return TRUE;
}

BOOL CChatAgent::AddCALink(CChatLink * pLink)
{
	if(!pLink) return FALSE;
	if(FindLink(pLink->m_sUserID))
		return FALSE;

	// 여기서 ChatFlags의 멤버를 적적히 셋팅...// 무조건 push_back..
	// 후에 메세지에서 vector Category를 전달해 오면.. link에 벡터를 만들고.. 전체를 셋팅.
	if(pLink->m_dwCategory > 1)
		m_setCategory.insert(pLink->m_dwCategory);
	if(pLink->m_lSSN > 0)
		m_setSSN.insert(pLink->m_lSSN);

	m_listActiveLink.push_back(pLink);
	return TRUE;
}

void CChatAgent::RemCALink(CChatLink * pLink)
{
	if(!pLink) return;
	LONG _lSSNCount = 0L;
	LONG _lCatCount = 0L;
	xstring _sUID = pLink->m_sUserID;
	CALinkGroupT::iterator itr = m_listActiveLink.end();
	ForEachElmt(CALinkGroupT, m_listActiveLink, it, jt)
	{
		CChatLink * _pLink = (*it);
		if(!_pLink) 
			continue;
		if(!_sUID.compare(_pLink->m_sUserID))	
			itr = it;
		if(pLink->m_lSSN == _pLink->m_lSSN)		// ssn이 list일 경우 .. -.-
			_lSSNCount++;
		if(pLink->m_dwCategory == _pLink->m_dwCategory)
			_lCatCount++;
	}

	// 여기서 ChatFlags의 멤버를 적적히 리셋....// 제거하려는 flag가 1개일때만 제거해야 함..
	if(_lSSNCount == 1)
		m_setSSN.erase(pLink->m_lSSN);
	if(_lCatCount == 1)
		m_setCategory.erase(pLink->m_dwCategory);
	
	if(*itr)
	{
		delete *itr;
		m_listActiveLink.erase(itr);
	}

	if(m_listActiveLink.size() < 1)
	{
		CAClear();
	}
}

void CChatAgent::CAClear()
{
	m_setSSN.clear();
	m_setCategory.clear();
	m_bCVRunning = FALSE;
	m_queueChat.Clear();
	m_lChatType = 0L;
	m_lConnectedUserCnt = 0L;
}

void CChatAgent::RemCALink(xstring & sUID)
{
	CChatLink * pLink = FindLink(sUID);
	if(!pLink)
		return;
	RemCALink(pLink);
}

CChatLink * CChatAgent::FindLink(xstring & sUID)
{
	ForEachElmt(CALinkGroupT, m_listActiveLink, it, jt)
	{
		CChatLink * pLink = *it;
		if(!pLink) 
			return NULL;
		if(!sUID.compare(pLink->m_sUserID))
			return *it;
	}
	return NULL;
}

void CChatAgent::SendLoginAns(CChatLink * pLink, xstring &sUserID, LONG lErrCode)
{
	PayloadCACV pld(PayloadCACV::msgCVLoginAns_Tag);
	pld.un.m_msgCVLoginAns->m_lErrCode = lErrCode; 
	pld.un.m_msgCVLoginAns->m_sUserID = sUserID;
	pld.un.m_msgCVLoginAns->m_sOption = "";

	SendMsg(pLink, pld);
}

void CChatAgent::SendDisconnectNtf(CChatLink * pLink, LONG lReason, xstring & sNewUser, string & sOpt)
{
	PayloadCACV pld(PayloadCACV::msgCVDisconnectNtf_Tag);
	pld.un.m_msgCVDisconnectNtf->m_lReason = lReason; 
	pld.un.m_msgCVDisconnectNtf->m_sNewUserID = sNewUser;
	pld.un.m_msgCVDisconnectNtf->m_sReserved = "";

	SendMsg(pLink, pld);

}

BOOL CChatAgent::IsCertifier(xstring & uid, xstring & password)
{
	if(uid.length() < 4 || password.length() < 4)
	{
		return FALSE;
	}

	ForEachElmt(RegUserListT, m_lstRegUser, it, jt)
	{
		UserAuthInfoPair & uinfo = *it;
		if(!uinfo.first.compare(uid) && !uinfo.second.compare(password))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CChatAgent::DestroyLink(CChatLink * pLink)
{
	if(!pLink)
		return;

	RemoveLink(pLink);
	pLink->Unregister();

	RemCALink(pLink);

}

void CChatAgent::AllUserDisconnect(LONG lReason, string & temp1, string & temp2)
{
	TLock lo(this);
	ForEachElmt(CALinkGroupT, m_listActiveLink, it, jt)
	{
		CChatLink * pLink = *it;
		if(!pLink) continue;

		RemoveLink(pLink);
		pLink->Unregister();
		delete pLink;
	}

	CAClear();
}

BOOL CChatAgent::InitCAInfo()
{
	const char* fileName = theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/;
	char szUserID[256] = {0, };
	char szPass[256] = {0, };
	char szVal[256] = {0, };

	DWORD dwRet = 1UL;
	LONG index = 0L;
	while(dwRet)
	{
		::ZeroMemory(szUserID, 0);
		::ZeroMemory(szPass, 0);

		sprintf(szVal, "CVUSER%02d", index);

		dwRet = ::GetPrivateProfileStringA("CAINFO", szVal, "", szUserID, sizeof(szUserID), fileName);
		if(!dwRet) continue;
		memset(szVal, 0, sizeof(szVal));

		sprintf(szVal, "CVPASS%02d", index);
		dwRet = ::GetPrivateProfileStringA("CAINFO", szVal, "", szPass, sizeof(szPass), fileName);
		if(!dwRet) continue;
		memset(szVal, 0, sizeof(szVal));

		UserAuthInfoPair userInfo(szUserID, szPass);
		m_lstRegUser.push_back(userInfo);

		index++;
	}

	if(index < 1)
		return FALSE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CChatAgent::PushChatMsg(RoomID & roomID, LONG lFromCSN, xstring & sFromUserID, xstring & sFromNick, 
							 xstring sChatMsg, LONG lToCSN, xstring sToUserID, xstring sToNick)
{
	//TLock lo(this);	// queue 단위로 locking.. 
	string sEmpty("");
	if(m_queueChat.Count() > 1000)
	{
		m_queueChat.Clear();

		AllUserDisconnect(1L, sEmpty, sEmpty);

		LOG(INF_UK, "CHS_CChatAgent"_LK, "overflow CA buffer queue.. ", 1000);
		return FALSE;
	}

	PayloadCACV pld(PayloadCACV::msgCVChatHistoryNtf_Tag);
	pld.un.m_msgCVChatHistoryNtf->m_roomID = roomID;
	pld.un.m_msgCVChatHistoryNtf->m_lFromCSN = lFromCSN;
	pld.un.m_msgCVChatHistoryNtf->m_sFromUserID = sFromUserID;
	pld.un.m_msgCVChatHistoryNtf->m_sFromNick = sFromNick;
	pld.un.m_msgCVChatHistoryNtf->m_sChatMsg = sChatMsg;

	pld.un.m_msgCVChatHistoryNtf->m_lToCSN = lToCSN;
	pld.un.m_msgCVChatHistoryNtf->m_sToUserID = sToUserID;
	pld.un.m_msgCVChatHistoryNtf->m_sToNick = sToNick;

	GBuf buf;
	::LStore(buf, pld);

	LONG lType = 0L;
	if(ISVALID_USN(lToCSN)) lType = 1L; //  귓속말.
	ChatFlags flags(roomID.m_lSSN, roomID.m_dwCategory, lType);
	m_queueChat.Push(flags, buf);
	
	return TRUE;
}

BOOL CChatAgent::PushChatMsg(ChannelID & cid, LONG lFromCSN, xstring & sFromUserID, xstring & sFromNick, 
							 xstring sChatMsg, LONG lToCSN, xstring sToUserID, xstring sToNick)
{
	RoomID roomID(cid.m_lSSN, cid.m_dwCategory, cid.m_dwGCIID, 0UL);
	return PushChatMsg(roomID, lFromCSN, sFromUserID, sFromNick, sChatMsg, lToCSN, sToUserID, sToNick);
}