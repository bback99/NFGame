//
// ChatHistory.cpp
//
//
// for �Ű� : ä�� �޼��� ������ ���� class ]
//			: CChannelContext���� �� Ŭ������ ����� ����.
//			: AddNewMsg - ä�� �޼����� �����ϱ� ���� ����.
//				--> 3��° ���ڷ� �ӼӸ� ����� ID�� setting.
//			: GetHistory - �Ű� ��û�� ���� ��� ����� ä�� �޼����� ���� ���� ����.
//			
//
#include "stdafx.h"
#include "ChatHistory.h"

#include "ErrorLog.h"

#define	MAX_CHAT_HISTORY	500		
///////////////////////////////////////////////////////////////////////////////////
// ChatHistory

ChatHistory::ChatHistory()
{
	m_lock_this_debug = 0;	
	m_deqChatHistory.clear();
	m_lLineCount = 0L;
}

ChatHistory::~ChatHistory()
{
	m_deqChatHistory.clear();
	m_lLineCount = 0L;
}

//�ִ� ���μ��� �����ʴ� �������� ä�� �޼����� ��� �߰��Ѵ�. 
//�ִ� ���μ��� ������ ���� ������ �޼����� �����ϰ� ���ο� �޼����� �߰� �Ѵ�. 
// MaxLine�� 150�� Default value�� ������ �Ѵ�. 

void ChatHistory::AddNewMsg(LONG lFromUSN, LONG lToUSN, xstring & sFromUID, xstring & snick, xstring & sToUID, xstring & sChatMsg)
{	
	// ���� �� ������ ������� ���� �߰�. 2007.03.19 by kts123
	CAutoLockCheck lc("ChatHistory::AddNewMsg", &m_lock_this_debug);
	TLock lo(this);

	ChatMsg chat_msg(lFromUSN, lToUSN, sFromUID, snick, sToUID, sChatMsg);
	
	if(m_lLineCount >= MAX_CHAT_HISTORY)	// ������ ���� ���� ������ �տ��� ���� ������ ����. 
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
	// ���� �� ������ ������� ���� �߰�. 2007.03.19 by kts123
	CAutoLockCheck lc("ChatHistory::GetHistory", &m_lock_this_debug);
	TLock lo(this);

	xstring temp = std::for_each(m_deqChatHistory.begin(), m_deqChatHistory.end(), 
											FN2(lAccuseUSN, lAccusedUSN)).m_sHistory;
	LONG length = temp.length();
	if(length > MAX_CONTENT_LEN)
	{
		xstrpos pos = temp.find( _X("<br>"), length - MAX_CONTENT_LEN);
		sHistory = temp.substr(pos + 5);
		//sHistory.assign(temp.c_str() + pos + 5, length - (pos +5));
		return length - pos;
	}
	sHistory = temp.c_str();
	return temp.length(); 
}


void ChatHistory::ClearHistory()
{
	// ���� �� ������ ������� ���� �߰�. 2007.03.19 by kts123
	CAutoLockCheck lc("ChatHistory::ClearHistory", &m_lock_this_debug);
	TLock lo(this);
	m_deqChatHistory.clear();
	m_lLineCount = 0L;
}

// �̰� �ѹ��� ȣ���ϵ��� �����Ұ�.. --> �ʿ伺�� ���� ����.
void ChatHistory::GetConfigInfo()
{
//	char szTemp[100] = {0, };
////	DWORD dwRet = ::GetPrivateProfileString("CHATTING", "MAXHISTORYCNT", _T(""), szTemp, sizeof(szTemp), theControl.m_confPath.GetConfPath()/*"CHSConfig.INI"*/);
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
