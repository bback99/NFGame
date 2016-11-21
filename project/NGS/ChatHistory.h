//
// ChatHistory.h
//
// for �Ű� : ä�� �޼��� ������ ���� class

#ifndef ChatHistory_h
#define ChatHistory_h

#include <NF/NFServerDefine.h>

typedef string xstring;


/////////////////////////////////////////////////////////////////////////////////// 
// ChatHistory
#define MAX_CONTENT_LEN		4000	// ���Ϸ� ����Ǵ� ä�� history�� �ִ� ����.
//
//	sutucture.. for chat history 
//
class ChatMsg
{
public:
	LONG m_lFromUSN;
	LONG m_lToUSN;
private:
	xstring m_sFromUserID;
	xstring m_sNickName;
	xstring m_sToUserID;
	xstring m_sMessage;
public:
	ChatMsg(LONG lFromUSN, LONG lToUSN, xstring sFromUserID, xstring sNickName, xstring sToUserID, xstring sMsg) : 
			m_lFromUSN(lFromUSN),
			m_lToUSN(lToUSN),
			m_sFromUserID(sFromUserID.c_str(), sFromUserID.length()), 
			m_sNickName(sNickName.c_str(), sNickName.length()), 
			m_sToUserID(sToUserID.c_str(), sToUserID.length()), 
			m_sMessage(sMsg.c_str(), sMsg.length())
			{
			};
			~ChatMsg() {};

	xstring GetChatLine() const // �̰��� ȣ���� �ӽ� ��ü�� ���ؼ� �Ǵ°�, �ƴϸ� ���� ��ü�� ...
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
		// foreach ������ �ѹ��� ȣ���.
		m_lSize = 4000L;
		m_sHistory.reserve(m_lSize);
		m_lAccuseUSN = lAccuseUSN;
		m_lAccusedUSN = lAccusedUSN;
	}

	~FN2() {};
	
	void operator()(const ChatMsg & msg)
	{
		if(ISVALID_USN(msg.m_lToUSN))	// �ӼӸ�
		{
			///// ���� �ְ���� �ӼӸ��� �Ű��뿡 �ִ´�. 2004.5.25 kimsk
			if( !((msg.m_lToUSN == m_lAccuseUSN && msg.m_lFromUSN == m_lAccusedUSN) ||
				(msg.m_lToUSN == m_lAccusedUSN && msg.m_lFromUSN == m_lAccuseUSN)))
				return;
		}
		else	/// all chat.
		{
			if(msg.m_lFromUSN != m_lAccuseUSN && msg.m_lFromUSN != m_lAccusedUSN)
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
	IMPLEMENT_TISAFE(ChatHistory)
private:	
	long m_lock_this_debug;
	//typedef std::vector<ChatMsg> VecStrT;	
	//
	// dequeue�� ������ �ڷᱸ�� ������..by effective STL..
	// channel�� ��� �����ϱ� ������ 150���� �̻��� �޼����� �׻� ����Ǵ� ������ ����.
	// vector�� ����� ��� �޸𸮸� �̸� �Ҵ��� ���� ���� memory allocation�� 150 ���� ��� �߻��ϰ�����.. 
	// 150������ �Ѿ� ���� ����.. vector���� �߻��ϴ� �޸� copy�� deque�� ȿ�������� ���ҽ��� �شٸ�
	// ��������δ� �� ȿ������ ����.
	//
	// ���� �̰��� game room���� ���ȴٸ�.. ??

	typedef std::deque<ChatMsg> DeqStrT;

public:
	ChatHistory();
	~ChatHistory();

	void AddNewMsg(LONG lFromUSN, LONG lToUSN, xstring & sFromUID, xstring & snick, xstring & sToUID, xstring & sChatMsg);
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

class CAutoLockCheck
{
public:
	CAutoLockCheck(char *pszDebugMsg, long *pdebug_count, long assert_count=1) : m_pdebug_count(pdebug_count)
	{
		long increment_count = ::InterlockedIncrement(m_pdebug_count);
		if (increment_count != assert_count)
			theLog.Put(INF_UK, "NGS_LockDebug, increment_count:", increment_count, ", assert_count:", assert_count, pszDebugMsg);		
	}
	~CAutoLockCheck()
	{
		::InterlockedDecrement(m_pdebug_count);
	}
private:
	long* m_pdebug_count;
};

#endif // ChatHistory_h
