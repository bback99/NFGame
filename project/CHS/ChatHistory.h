//
// ChatHistory.h
//
// for 신고 : 채팅 메세지 저장을 위한 class

#ifndef ChatHistory_h
#define ChatHistory_h

#include <Define.h>
typedef string xstring;


/////////////////////////////////////////////////////////////////////////////////// 
// ChatHistory
#define MAX_CONTENT_LEN		4000	// 파일로 저장되는 채팅 history의 최대 길이.
//
//	sutucture.. for chat history 
//
class ChatMsg
{
public:
	LONG m_lFromCSN;
	LONG m_lToCSN;
private:
	xstring m_sFromUserID;
	xstring m_sNickName;
	xstring m_sToUserID;
	xstring m_sMessage;
public:
	ChatMsg(LONG lFromCSN, LONG lToCSN, xstring sFromUserID, xstring sNickName, xstring sToUserID, xstring sMsg) : 
			m_lFromCSN(lFromCSN),
			m_lToCSN(lToCSN),
			m_sFromUserID(sFromUserID.c_str(), sFromUserID.length()), 
			m_sNickName(sNickName.c_str(), sNickName.length()), 
			m_sToUserID(sToUserID.c_str(), sToUserID.length()), 
			m_sMessage(sMsg.c_str(), sMsg.length())
			{
			};
			~ChatMsg() {};

	xstring GetChatLine() const // 이것의 호출이 임시 객체를 통해서 되는가, 아니면 실제 객체를 ...
	{
		xstring nick, temp;
		xstring uid;
		temp.reserve(200);
		nick = m_sNickName;
		nick.resize(12, ' ');
		uid = m_sFromUserID;
		uid.resize(12, ' ');
		temp = ::format(_X("[%s (%s)] : [%s] %s <br>\r\n"), nick.c_str(), uid.c_str(), m_sToUserID.c_str(), m_sMessage.c_str());
		return temp;
	}
};

//
//	for User ID 
// 
//struct FN2
class FN2 : public unary_function<string, void>
{
	//typedef std::pair<string, string> ChatMsg2;
public: 
	FN2(LONG lAccuseUSN, LONG lAccusedUSN)
	{
		// foreach 내에서 한번만 호출됨.
		m_lSize = 4000L;
		m_sHistory.reserve(m_lSize);
		m_lAccuseUSN = lAccuseUSN;
		m_lAccusedUSN = lAccusedUSN;
	}

	~FN2() {};
	
	void operator()(const ChatMsg & msg)
	{
		if(ISVALID_USN(msg.m_lToCSN))	// 귓속말
		{
			///// 서로 주고받은 귓속말만 신고내용에 넣는다. 2004.5.25 kimsk
			if( !((msg.m_lToCSN == m_lAccuseUSN && msg.m_lFromCSN == m_lAccusedUSN) ||
				(msg.m_lToCSN == m_lAccusedUSN && msg.m_lFromCSN == m_lAccuseUSN)))
				return;
		}
		else	/// all chat.
		{
			if(msg.m_lFromCSN != m_lAccuseUSN && msg.m_lFromCSN != m_lAccusedUSN)
				return;
		}

		m_sHistory += msg.GetChatLine();
		LONG temp_size = m_sHistory.length();

		if(temp_size >= m_lSize)
		{
			m_lSize += 1000;
			m_sHistory.reserve(m_lSize);
		}
	}
	FN2 & operator=(const FN2 & fn)
	{
		m_sHistory = fn.m_sHistory;
		return *this;
	}
private:
	LONG m_lSize;
	LONG m_lAccuseUSN;
	LONG m_lAccusedUSN;
public:
	xstring m_sHistory;
};

class ChatHistory  
{
//	IMPLEMENT_TISAFE(ChatHistory)
private:
	//typedef std::vector<ChatMsg> VecStrT;	
	//
	// dequeue가 적당한 자료구조 같은데..by effective STL..
	// channel은 계속 존재하기 때문에 150라인 이상의 메세지가 항상 저장되는 것으로 가정.
	// vector를 사용할 경우 메모리를 미리 할당할 수는 없어 memory allocation이 150 까지 계속 발생하겠지만.. 
	// 150라인을 넘어 서는 순간.. vector에서 발생하는 메모리 copy를 deque가 효과적으로 감소시켜 준다면
	// 장기적으로는 더 효과적일 것임.
	//
	// 만약 이것이 game room에서 사용된다면.. ??

	typedef std::deque<ChatMsg> DeqStrT;

public:
	ChatHistory();
	~ChatHistory();

	void AddNewMsg(LONG lFromCSN, LONG lToCSN, xstring & sFromUID, xstring & snick, xstring & sToUID, xstring & sChatMsg);
	LONG GetHistory(LONG lAccuseUSN, LONG lAccusedUSN, xstring & sHistory);
	void ClearHistory();
	void GetConfigInfo();

	static BOOL fn(const ChatMsg &msg);
protected:
	
protected:
//	VecStrT m_vecChatHistory;

	DeqStrT m_deqChatHistory;
	LONG m_lLineCount;

	LONG m_lMaxLine;	//max history line count , default 150 Line

private:
//	string m_sOutput;
};

#endif // ChatHistory_h
