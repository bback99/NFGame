//
// Agent.h: interface for the Agent class.
//
// for 신고 : gDBGW접근과 CGI call을 통한 file server접근 제공.
//

#if !defined(AFX_AGENT_H__836D6658_DC93_40FD_BCFF_BAEEE4FDC76C__INCLUDED_)
#define AFX_AGENT_H__836D6658_DC93_40FD_BCFF_BAEEE4FDC76C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AgentLink.h"

// for 신고 : 아래의 ADL에 MsgCliCHS_AccuseReq와 MsgCHSCli_AccuseAns, AccuseBus 추가됨.
//			: _Ans에 해당하는 것은 client의 요구가 있을 경우를 대비. 현재 사용되지 않음.
//			: AccuseBus 는 신고 Queue에 Posting할때 편리하게 사용하기 위해 추가.
#include <NF/ADL/MsgNGSCli.h>


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Accuse History 
//////////////////////////////////////////////////////////////////////////////////////////////////////
#define ACCUSE_TIMESPAN		6 * 3600

#define ACCUSE_CHECKCONNECTION	60000

class CAccuseNote
{
public:
	LONG m_lUSN; 
	LONG m_lTimeSpan;
	RoomID m_RoomID;
	string m_strTitle;

	CAccuseNote() 
	{
		m_lUSN = m_lTimeSpan = 0L;
	}
	CAccuseNote(LONG lUSN, LONG lTSpan, RoomID roomID, string *pstr = NULL) : m_lUSN(lUSN), m_lTimeSpan(lTSpan), m_RoomID(roomID) { m_strTitle = pstr ? *pstr : "";};
	CAccuseNote& operator=(const CAccuseNote & obj) 
	{
		m_lUSN = obj.m_lUSN;
		m_lTimeSpan = obj.m_lTimeSpan;
		m_RoomID = obj.m_RoomID;
		return(*this);
	}	
	~CAccuseNote() { };
};

//////////// Accuse Notify 위해서 복잡한 ADL에서 수정하지 말고 자체적으로 수정하도록... /////////////
//// CHS : _CHS_ACCUSE
//// GLS : _NGS_ACCUSE 
//// predefine 해야 함. 

/// Agent 에서 사용.
class CAccuseBus_RoomID
{
public:	
	CAccuseBus_RoomID(AccuseBus& accuseBus, RoomID& rID, long lTime, string *pstrRoomTitle = NULL) 
		: m_AccuseBus(accuseBus), m_RoomID(rID), m_lTime(lTime)
	{
		m_strRoomTitle = pstrRoomTitle ? *pstrRoomTitle : "";
	}

	CAccuseBus_RoomID() {}
	~CAccuseBus_RoomID() {}

	BOOL SetAccuseBus(AccuseBus& accuseBus) { m_AccuseBus = accuseBus; return TRUE;}
	BOOL SetRoomID(RoomID& rID) { m_RoomID = rID; return TRUE; }
	BOOL SetTime(long lTime) { m_lTime = lTime; }

	AccuseBus	GetAccuseBus() { return m_AccuseBus; }
	RoomID		GetRoomID() { return m_RoomID; }
	long		GetTime() { return m_lTime; }
	string		GetRoomTitle() {return m_strRoomTitle;}	

private:
	AccuseBus	m_AccuseBus;
	RoomID		m_RoomID;	
	long		m_lTime;
	string		m_strRoomTitle;
};

/// HTTPAgent 에서 사용.
class CAccuseContent_RoomID
{
public:
	CAccuseContent_RoomID() {};
	CAccuseContent_RoomID(long lUSN, long lAccusedUSN, RoomID rID, xstring sID, xstring sAccusedID, string sContents, long lTime) \
		: m_lUSN(lUSN), m_lAccusedUSN(lAccusedUSN), m_RoomID(rID), m_sID(sID), m_sAccusedID(sAccusedID), m_sContents(sContents), m_lTime(lTime), m_lErrState(ERRAGENT_SYSTEM){}
		/*
		BOOL SetUSN(long nUSN) { m_nUSN = nUSN; return TRUE;}
		BOOL SetRoomID(RoomID& rID) { m_RoomID = rID; }
		BOOL SetUserID(xstring sID) { m_sID = sID; }
		BOOL SetAccusedID(xstring sAccusedID) { m_sAccusedID = sAccusedID; }
		BOOL SetContents(string sContents) { m_sContents = sContents; }
		BOOL SetTime(long lTime) { m_lTime = lTime; }
		*/
		BOOL SetErrStatus(long lErrState) { m_lErrState = lErrState;  return TRUE;}

		long GetUSN() { return m_lUSN;}
		long GetAccusedUSN() { return m_lAccusedUSN;}
		RoomID GetRoomID() { return m_RoomID; }
		xstring GetUserID() { return m_sID; }
		xstring GetAccusedID() { return m_sAccusedID; }
		string GetContents() { return m_sContents; }
		long GetTime() { return m_lTime; }
		long GetErrStatus() { return m_lErrState; }

private:
	long		m_lUSN; 
	long		m_lAccusedUSN; 
	RoomID		m_RoomID;
	xstring		m_sID;
	xstring		m_sAccusedID;
	string		m_sContents;
	long		m_lTime;
	long		m_lErrState;
};

/// 신고 결과 리턴하기 위한 데이타 -,.-
class CAccuseResultAnswer
{
public:
	CAccuseResultAnswer() {}
	CAccuseResultAnswer(long nUSN, xstring sAccusedID, long nErrorCode, long lTimeSpan) \
		: m_nUSN(nUSN), m_sAccusedID(sAccusedID), m_nErrorCode(nErrorCode), m_lTimeSpan(lTimeSpan) {}
		xstring GetAccusedID() { return m_sAccusedID; }
		long GetUSN() { return m_nUSN; }
		long GetErrorCode() { return m_nErrorCode; }
		long GetTime() { return m_lTimeSpan; }

		CAccuseResultAnswer & operator=(const CAccuseResultAnswer & acuAns)
		{
			m_nUSN = acuAns.m_nUSN;
			m_sAccusedID.assign(acuAns.m_sAccusedID.c_str(), acuAns.m_sAccusedID.length());
			m_nErrorCode = acuAns.m_nErrorCode;
			m_lTimeSpan = acuAns.m_lTimeSpan;
			return *this;
		}
private:
	long	m_nUSN;	
	//long	m_nAccusingMsgFlag;			//// Accusing Msg Type : Chatting? 0 Game Title? 1
	xstring	m_sAccusedID;
	long	m_nErrorCode;
	long	m_lTimeSpan;
};

template<class T>
class CSafeDataQ
{
	IMPLEMENT_TISAFE(CSafeDataQ)
public:
	void Push(const T& buf) {
		TLock lo(this);
		mRcvBufQ.push_back(buf); 
	}
	BOOL Pop(T& buf) {
		TLock lo(this);
		if(IsEmpty()) 
			return FALSE;
		buf = mRcvBufQ.front();
		mRcvBufQ.pop_front();
		return TRUE;
	}
	void Clear()
	{
		TLock lo(this);
		mRcvBufQ.clear();
	}
	LONG Count()
	{
		return mRcvBufQ.size();
	}
	BOOL IsEmpty()
	{		
		return mRcvBufQ.empty();
	}
private:
	list<T> mRcvBufQ;
};

template<class TKey, class TData>
class CSafeDataMap
{
	typedef map<TKey, TData>	MAP_DATA;
	//typedef MAP_DATA::iterator TIter;

	IMPLEMENT_TISAFE(CSafeDataMap)		
public:
	void Insert(TKey key, TData& buf) {
		TLock lo(this);
		CheckData(buf.GetTime());
		mRcvBufQ.insert(MAP_DATA::value_type(key, buf)); 
	}
	BOOL Get(TKey key, TData& buf) {
		TLock lo(this);
		if(IsEmpty()) 
			return FALSE;
		MAP_DATA::iterator iter = mRcvBufQ.find(key);
		if(iter == mRcvBufQ.end())
			return FALSE;
		buf = iter->second;
		mRcvBufQ.erase(key);
		return TRUE;
	}
	void Clear()
	{
		TLock lo(this);
		mRcvBufQ.clear();
	}
	LONG Count()
	{
		return mRcvBufQ.size();
	}
	BOOL IsEmpty()
	{		
		return mRcvBufQ.empty();
	}
	//// 이건 의존적임... -,.-
	BOOL CheckData(long nTime)
	{
		if(mRcvBufQ.size() <= 0)
			return FALSE;
		ForEachElmt(MAP_DATA, mRcvBufQ, iter, j) {
			if(iter->second.GetTime() + 30000 < nTime )
				mRcvBufQ.erase(iter);
			else	break;
		}
		return TRUE;
	}
private:
	MAP_DATA mRcvBufQ;
};

//#define ACCUSESIGNAL_AGENTANSWER		((HSIGNAL)0xff000001)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	AGENT_POSTMSG = 1
};

class Agent : public IXObject
{
	IMPLEMENT_TISAFEREFCNT(Agent)
public:
	typedef vector<CAccuseNote> VecANoteT;

public:
	Agent();
	virtual ~Agent();
private:
	Agent(const Agent&);


public: 
	virtual BOOL RunAgent();

	// 외부에서 신고에 필요한 데이타를 Queue에 저장한다..
	LONG PostAccusationMsg(AccuseBus & bus, LONG lTimeSpan, RoomID & rid, string *pstr = NULL);	
	CSafeDataMap<long, CAccuseResultAnswer> m_AccuseResultAns;

	// for History
	// 중복신고를 막기 위해.. HISTORY를 남김.
	BOOL AddAccuseNote(CAccuseBus_RoomID &ARID);
	BOOL DeleteAccuseNote(CAccuseBus_RoomID &ARID);
	BOOL DeleteAccuseNote(const long lAccusedUSN, const RoomID & rid, BOOL bRoomTitleAccuse = FALSE, string *pstr = NULL);

protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);	

	LONG Accuse(CAccuseBus_RoomID& dataAR, AccuseBus& bus);

	// DB Query
	BOOL GetSerialNoFromDB(long &lASN);
	BOOL SaveChatHistoryToDB(const AccuseBus & bus, DWORD dwASN, string *pstr = NULL);

	// Client(GLS or CHS 겠지?) 에게 응답메시지를 보냄.
	// 하지만, GLS 는 가끔 터지는 경우가 있어서 이 함수를 통해 결과를 알려주는 것이 빠져있다.
	// 터지는 경우는 결과를 알려주려 하는 시점에는 방이 이미 사라지고 없는 경우 등? 이란다.
	// -snowyunee
	BOOL SendAccuseResult(long nUSN, long lAccusedUSN, xstring sAccusedID, RoomID rID, LONG lTimeSpan, long nErrorCode);

	// DB Query 를 parsing 함.
	int DBResultParser(const DBGW_String& strDBResult, list<char *>& lstData, char cDelimeter = QUERY_DELIMETER, BOOL bSP = FALSE);
	char *GetNext(char *&p)
	{
		p = strstr(p, "|");
		if(NULL == p) return p;
		*p = '\0';
		++p;
		return p;
	}

protected:
	CSafeDataQ<CAccuseBus_RoomID> m_AgentQ;

	// 중복신고를 방지하기 위해 일정기간 동안의 신고를 저장해 둠.
	VecANoteT m_AccuseNote;

	//GXSigTimer m_timerReconnect;
};


extern Agent theAccuseAgent;

///////////////////////////////////////////////////////////////////////////////////////////////////
// for file write  ... by HTTP protocol
///////////////////////////////////////////////////////////////////////////////////////////////////
enum 
{
	HTTP_POSTMSG = 1
};

enum
{
	HTTPAGENTSTATE_INVALID = -1,
	HTTPAGENTSTATE_DEACTIVATED,
	HTTPAGENTSTATE_ACTIVATED
};
enum 
{
	HTTPAGENTCONNSTATE_STOP = 0,
	HTTPAGENTCONNSTATE_ATTACH,
	HTTPAGENTCONNSTATE_CONN,
	HTTPAGENTCONNSTATE_RUN
};


class HTTPAgent : public XSigBufSocketSingleT<HTTPLink>
{
	IMPLEMENT_TISAFEREFCNT(HTTPAgent)
public:
	typedef std::pair<DWORD, CAccuseContent_RoomID> PairAccuseData;
	typedef std::vector<PairAccuseData> VecAccuseData;

	typedef XSigBufSocketSingleT<HTTPLink> TBase;

private:
	HTTPAgent(const HTTPAgent &);
public:
	HTTPAgent();
	virtual ~HTTPAgent();

public: 
	BOOL ActivateHTTPAgent();
	BOOL DeactivateHTTPAgent();
	// 외부에서 신고에 필요한 데이타를 Queue에 저장한다..
	void PostHTTPMsg(xstring sUserID, xstring sAccusedID, xstring sContent, DWORD dwASN, long lUSN, long lAccusedUSN, RoomID rID, long lTimeSpan);

private:
	virtual BOOL RunHTTPAgent();
	virtual BOOL StopHTTPAgent();

protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);	

	virtual void DestroyLink(HTTPLink* pLink);
	BOOL OnConnect(TSocket* pSocket, int nErrorCode);
	virtual BOOL OnClose(TSocket* pSocket, int nErrorCode);
	BOOL OnError(HTTPLink* pLink, long lEvent, int nErrorCode);
	BOOL OnReceive(HTTPLink* pLink, int lErrorCode);

	BOOL SendHTTPData(string & sData);
	BOOL MakeBoundary(string & sBound);
	BOOL ParseHTTP(string& s_command);
	BOOL WriteFile(DWORD dwASN, CAccuseContent_RoomID& pairContent);	
	BOOL ConnectFileServer(HTTPLink *pLink);
	void OnWriteFile();

private:
	BOOL PushAccuseData(xstring sUserID, xstring sAccusedID, xstring sContent, DWORD dwASN, long lUSN, long lAccusedUSN, RoomID rID, long lTimeSpan);
	BOOL PopAccuseData(PairAccuseData& pairAccuseData);
	int GetAccuseDataQSize();

	BOOL SendAccuseResult(CAccuseContent_RoomID& AccuseContentRoomID);

public:
	CSafeDataMap<long, CAccuseResultAnswer> m_AccuseResultAns;

private:
	CComAutoCriticalSection m_CS;
	VecAccuseData m_vecAccuseDataQ;

	// 현재 Accuse 관련 데이타들.
	PairAccuseData m_pairCurAccuseData;
	string * m_psWData;
	DWORD m_dwASN;
	DWORD m_dwBound;
	//////////////////////////

	// 현재 상태
	LONG m_lHTTPConnFlag;
	LONG m_lConnFlag;
	//////////////////////////
};

extern HTTPAgent theHTTPAgent;

/*
*	//// 호출시 리턴값이 new 로 생성되므로 auto_ptr 을 사용하여 호출하는게 안전합니다. (2005.11.30 kimsk)
*/

using namespace std;

#endif // !defined(AFX_AGENT_H__836D6658_DC93_40FD_BCFF_BAEEE4FDC76C__INCLUDED_)
