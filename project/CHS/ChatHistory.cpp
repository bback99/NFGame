//
// ChatHistory.cpp
//
//
// for 신고 : 채팅 메세지 저장을 위한 class ]
//			: CChannelContext에서 이 클래스를 멤버로 가짐.
//			: AddNewMsg - 채팅 메세지를 저장하기 위해 제공.
//				--> 3번째 인자로 귓속말 대상자 ID를 setting.
//			: GetHistory - 신고 요청이 왔을 경우 저장된 채팅 메세지를 얻어내기 위해 제공.
//			
//
#include "stdafx.h"
#include "ChatHistory.h"

//#include "ErrorLog.h"

#define	MAX_CHAT_HISTORY	500		
///////////////////////////////////////////////////////////////////////////////////
// ChatHistory

ChatHistory::ChatHistory()
{
	m_deqChatHistory.clear();
	m_lLineCount = 0L;
}

ChatHistory::~ChatHistory()
{
	m_deqChatHistory.clear();
	m_lLineCount = 0L;
}

//최대 라인수를 넘지않는 범위에서 채팅 메세지를 계속 추가한다. 
//최대 라인수를 넘으면 가장 오래된 메세지를 제거하고 새로운 메세지를 추가 한다. 
// MaxLine은 150을 Default value로 갖도록 한다. 

void ChatHistory::AddNewMsg(LONG lFromCSN, LONG lToCSN, xstring & sFromUID, xstring & snick, xstring & sToUID, xstring & sChatMsg)
{
	ChatMsg chat_msg(lFromCSN, lToCSN, sFromUID, snick, sToUID, sChatMsg);
	
	if(m_lLineCount >= MAX_CHAT_HISTORY)	// 지정된 라인 수를 넘으면 앞에서 부터 삭제해 나감. 
	{
		m_deqChatHistory.pop_front();
		m_deqChatHistory.push_back(chat_msg);
//		m_deqChatHistory.pop_back();
//		m_deqChatHistory.push_front(chat_msg);

	}
	else 
	{
		m_deqChatHistory.push_back(chat_msg);
//		m_deqChatHistory.push_front(chat_msg);
		m_lLineCount++;
	}
}

LONG ChatHistory::GetHistory(LONG lAccuseUSN, LONG lAccusedUSN, xstring & sHistory)
{
	xstring temp = std::for_each(m_deqChatHistory.begin(), m_deqChatHistory.end(), 
											FN2(lAccuseUSN, lAccusedUSN)).m_sHistory;
	LONG length = temp.length();
	if(length > MAX_CONTENT_LEN)
	{
		strpos pos = temp.find( _X("<br>"), length - MAX_CONTENT_LEN);
		sHistory = temp.substr(pos + 5);
		//sHistory.assign(temp.c_str() + pos + 5, length - (pos +5));
		return length - pos;
	}
	sHistory = temp.c_str();
	return temp.length(); 
}


void ChatHistory::ClearHistory()
{
	m_deqChatHistory.clear();
	m_lLineCount = 0L;
}

// 이건 한번만 호출하도록 수정할것.. --> 필요성이 없어 보임.
void ChatHistory::GetConfigInfo()
{
//	char szTemp[100] = {0, };
//	DWORD dwRet = ::GetPrivateProfileString("CHATTING", "MAXHISTORYCNT", _T(""), szTemp, sizeof(szTemp), "CHSConfig.INI");
//	if(dwRet) 
//	{
//		m_lMaxLine = atoi(szTemp);
//		if(m_lMaxLine < 10)
//			m_lMaxLine = 150L;
//	}
//	else 
//	{
//		theErr.LOG(2, "fail to get Max History Line count, so thar set default value 150 Lines ");
//		m_lMaxLine = 150L;
//	}
}

