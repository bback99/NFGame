// ChatAgent.h: interface for the CChatAgent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHATAGENT_H__E0C1742B_4D22_48C0_9EDC_45076C9F0955__INCLUDED_)
#define AFX_CHATAGENT_H__E0C1742B_4D22_48C0_9EDC_45076C9F0955__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "ErrorLog.h"
#include "Define.h"
#include "ADL/MsgCACVMsg.h"

struct ChatFlags
{
public:
	LONG m_lSSN;
	DWORD m_dwCategory;
	LONG m_lMsgType;	// 귓속말 여부

	ChatFlags()  { m_lSSN = 0L; m_dwCategory = 0UL; m_lMsgType = 0L; }
	ChatFlags(LONG lSSN, DWORD dwCat, LONG lType):m_lSSN(lSSN), m_dwCategory(dwCat), m_lMsgType(lType){}
};

class GSafeBufQEx //: public GSafeBufQ
{
	IMPLEMENT_TISAFEREFCNT(GSafeBufQEx)
public:
	typedef pair<ChatFlags, GBuf> ChatBlockT;
	typedef list<ChatBlockT> _QueueList;
	LONG Count() { return m_Queue.size(); } 

	void Push(ChatFlags & def, GBuf & buf) 
	{
		TLock lo(this);
		ChatBlockT argu(def, buf);
		m_Queue.push_back(argu);
	}
	BOOL Pop(ChatFlags & def, GBuf & buf) 
	{
		TLock lo(this);
		if(m_Queue.size() < 1) 
			return FALSE;
		ChatBlockT & cbt = (*m_Queue.begin());
		def = cbt.first;
		buf = cbt.second;
		m_Queue.erase(m_Queue.begin());
		return TRUE;
	}
	void Clear() 
	{
		TLock lo(this);
		m_Queue.clear();
	}
private:
	_QueueList m_Queue;
};

//class GSafeBufQEx : public GSafeBufQ
//{
//public:
//	LONG Count() { return mRcvBufQ.size(); } 
//};

class CChatLink : public GXLink//GArcSocket
{
public:
	typedef GXLink TBase;
	CChatLink();
	virtual ~CChatLink();

	BOOL IsBufOverflow();
	void SetIP(LPCSTR lpAddr) 
	{
		m_sClientIP = lpAddr;
	}
	string & GetIP()
	{	
		return m_sClientIP;
	}
private:
	string m_sClientIP;

public:
	xstring m_sUserID;
	LONG m_lSSN;
	DWORD m_dwCategory;
	LONG m_lChatType;
	string m_sOption;
};


class CChatAgent : public XLinkAdlManagerT<CChatLink, PayloadCVCA>
{
	IMPLEMENT_TISAFEREFCNT(CChatAgent)
public:
	typedef XLinkAdlManagerT<CChatLink, PayloadCVCA> TBase;
	typedef PayloadCVCA TMsg;

	typedef set<LONG> setLONG;
	typedef set<DWORD> setDWORD;
	typedef pair<xstring, xstring> UserAuthInfoPair;
	typedef list<UserAuthInfoPair> RegUserListT;

	typedef list<CChatLink *> CALinkGroupT;

public:
	CChatAgent();
	virtual ~CChatAgent();

	BOOL RunChatAgent(int nPort);
	BOOL StopChatAgent();

public:
	BOOL PushChatMsg(RoomID & roomID, LONG lFromCSN, xstring & sFromUserID, xstring & sFromNick, 
					 xstring sChatMsg = _X(""), LONG lToCSN = 0L, xstring sToUserID = _X(""), xstring sToNick = _X(""));
	BOOL PushChatMsg(ChannelID & channelID, LONG lFromCSN, xstring & sFromUserID, xstring & sFromNick, 
					 xstring sChatMsg = _X(""), LONG lToCSN = 0L, xstring sToUserID = _X(""), xstring sToNick = _X(""));
	BOOL IsCVConnected() { TLock lo(this); return m_bCVRunning; }
// 하나의 유저라도 요구하면.. channel에서는 CA queue에 담는다 ?.. 여러 문제..
// send 하는 쪽에서 구분하여 사용..
	BOOL IsSecret()	
	{ 
		TLock lo(this); 
		return m_lChatType; 
	}	// 1 = 귓속말 요구.
// 전달받은 category에 해당하는 사용자가 하나라도 있으면 TRUE.. 
	BOOL IsCategory(DWORD dwCategory) 
	{ 
		TLock lo(this); 
		if(m_listActiveLink.size() < 1)
			return FALSE;
		if(m_setCategory.size() < 1UL) 
			return TRUE;		
		setDWORD::iterator itr = m_setCategory.find(dwCategory);
		if(itr == m_setCategory.end())
			return FALSE;
		return TRUE;
	}
// 전달받은 SSN에 해당하는 사용자가 하나라도 있으면 TRUE.. 
	BOOL IsSSN(LONG lSSN) 
	{ 
		TLock lo(this); 
		if(m_listActiveLink.size() < 1)
			return FALSE;
		if(m_setSSN.size() < 1UL) 
			return TRUE;
		setLONG::iterator itr = m_setSSN.find(lSSN);
		if(itr == m_setSSN.end())
			return FALSE;
		return TRUE;
	}

	virtual BOOL OnListenerAccept(SOCKET hSocket, int nErrorCode, LPCSTR lpAddr, LONG lPort);

protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnError(CChatLink* pSocket, long lEvent, int nErrorCode);
	virtual BOOL OnRcvMsg(CChatLink* pLink, PayloadCVCA& pld);

	void SendChatHistory();
	void SendToAllLink(ChatFlags & flags, GBuf & buf);
	BOOL IsMatchFlags(ChatFlags & flags, CChatLink * pLink);
	void SendMsg(CChatLink* pLink, const PayloadCACV& pld);
	BOOL OnRecvLoginReq(CChatLink * pLink, MsgCVCA_CVLoginReq & pld);
	void SendLoginAns(CChatLink * pLink, xstring &sUserID, LONG lErrCode);
	void SendDisconnectNtf(CChatLink * pLink, LONG lReason, xstring & sNewUser, string & sOpt);
	BOOL IsCertifier(xstring & uid, xstring & password);
	void DestroyLink(CChatLink * pLink);
	void AllUserDisconnect(LONG lReason, string & temp1, string & temp2);
	BOOL InitCAInfo();

	BOOL AddCALink(CChatLink * pLink);
	void RemCALink(CChatLink * pLink);
	void RemCALink(xstring & sUID);
	CChatLink * FindLink(xstring & sUID);

	void CAClear();
private:
	GXSigTimer m_timerSendCycle;
	RegUserListT m_lstRegUser;

	CALinkGroupT m_listActiveLink;

	LONG m_lChatType;		// 귀속말을 요구하는 사용자가 하나라도 있으면.. TRUE..
	setLONG m_setSSN;
	setDWORD m_setCategory;

	GSafeBufQEx m_queueChat;

	BOOL m_bCAStarting;
	BOOL m_bCVRunning;		// client가 접속, 인증을 완료했음.

	DWORD m_dwListener;

public:
	LONG m_lConnectedUserCnt;
};

extern CChatAgent theChatAgent;

#endif // !defined(AFX_CHATAGENT_H__E0C1742B_4D22_48C0_9EDC_45076C9F0955__INCLUDED_)

