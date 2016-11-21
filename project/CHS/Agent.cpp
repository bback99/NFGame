////////////////////////////////////////////////////////////////////////////////////////////
// Agent.cpp: implementation of the Agent class.
//
// for �Ű� : gDBGW���ٰ� CGI call�� ���� file server���� ����.
//			: PostAccusationMsg()�� �Ű� ��û�� ȣ��.
//			: OnAccuseReq(..)���� �Ű� �ʿ��� �������� ���� �� setting..
//					--> �Ű� �������� header������ �ش�. ������ ������ setting���־�� ��.		

//
// Agent : �Ű� Ÿ��Ʋ�� gDBGW�� ����(�����Ű�, ��ȭ�Ű�).
// HTTPAgent : ��ȭ ���� ���弭���� ����. nfs_post.nwz ���� IP üũ��. 
//			���� ����	: boardw.pmang.com/80
//			���� IP		: ���� 211.172.245.*, 211.43.216.* üũ�ϰ� ����. 2005.4.15 by kimsk
////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Agent.h"

#ifdef _CHS_ACCUSE
	#include "Channel.h"
	#include "ChannelDir.h"
	extern CChannelDir theChannelDir;
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// CGI Call�� ���� ó���� �־�� ����.
// 1. ��ð� ����� ���� ��� Process�� �ٽ� ����� ��. (10�� ���� �ɸ��ٰ� ��.)
// 2. 1. �� ��Ÿ ������ ���а� ����. (�׽�Ʈ �߿��� ���÷� ������.)
//
// => ������ ������� coment �Ǿ� �־�����, ������ �׷������� �� �𸣰���
// -snowyunee
//////////////////////////////////////////////////////////////////////

Agent theAccuseAgent;
HTTPAgent theHTTPAgent;


////////////////////// �Ű� �α� ī��Ʈ 2004.2.2 ////////////////////////
long glAC_TotalAccusing = 0L;		/// �� �Ű��
long glAC_ContentsCnt = 0;			/// ä�� ���� �Ű��
long glAC_FailTitle = 0L;			/// Ÿ��Ʋ(�Ű���, �Ű�ð�, ���� �� ������ ������ ����)
long glAC_FailContents = 0L;		/// �� ����(ä�� ����)
long glAC_FailASN = 0L;				/// ASN ��ȣ �޾ƿ���.
long glAC_Dublicate = 0L;			/// �ݺ� �Ű�.
long glAC_LogCount = 0L;			/// ������� ������? 5���� �ѹ���� ����.
/////////////////////////////////////////////////////////////////////////

Agent::Agent()
{
}

Agent::~Agent()
{
//	m_timerReconnect.Deactivate();
}

BOOL Agent::RunAgent()
{
	TLock lo(this);

	BOOL bRet;

	bRet = theHTTPAgent.ActivateHTTPAgent();
	VALIDATE(bRet);
	if (!bRet)
	{
		return FALSE;
	}

	/*
	bRet = m_timerReconnect.Activate(GetThreadPool(), this, ACCUSE_CHECKCONNECTION, ACCUSE_CHECKCONNECTION);
	if (!bRet)
	{
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "Failed to create reconnect timer");
		return FALSE;
	}
	*/

	return TRUE;
}


STDMETHODIMP_(void) Agent::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if((LONG)hSignal == 0L) // user define event , OutThreadPool�� ���.
	{	
		if(wParam == AGENT_POSTMSG)
		{
			TLock lo(this);

			CAccuseBus_RoomID dataAR;
			if(m_AgentQ.Pop(dataAR))
			{
				AccuseBus bus;
				bus = dataAR.GetAccuseBus();
				LONG lRet = Accuse(dataAR, bus);
				if(lRet >= 0)
				{
					if(lRet != ERRAGENT_SUCCESS)
					{
						// ���� �����Ƿ�, �ٽ� �Ű��� �� �ֵ��� �Ű� history���� ����.
						string *pstr = NULL;
						string strTitle;
						if (bus.m_bRoomTitleAccuse)
						{
							strTitle = dataAR.GetRoomTitle();
							pstr = &strTitle;							
						}
						DeleteAccuseNote(dataAR);
					}

					SendAccuseResult(bus.m_lUSN, bus.m_lAccusedUSN, bus.m_sAccusedID, dataAR.GetRoomID(), dataAR.GetTime(), lRet);
				}
				// �� ���� CGI call�� �ؼ� ä�ó����� �����ؾ� �ϴ� ����̴�.
				// HTTPAgent ���� ������ ���̴�.
				/*
				else
				{
				}
				*/
			}
			else 
			{
				theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] fatal error : occur event but empty Queue ");
			}
		}
	}
	/*
	else 
	{
		if (m_timerReconnect.IsHandle(hSignal))
		{
			if (!m_AgentQ.IsEmpty())
			{
				theLog.Put(DEV_UK, "GLS_Accuse"_COMMA, "[Accuse] timer expired");
				TLock lo(this);
				Accuse();
			}
		}
	}
	*/
}

LONG Agent::PostAccusationMsg(AccuseBus & bus, LONG lTimeSpan, RoomID & rid, string *pstrRoomTitle)
{
	TLock lo(this);

	CAccuseBus_RoomID dataAR(bus, rid, lTimeSpan, pstrRoomTitle);
	// Ƽ�Ĵ� ���뿡 ���� �ߺ� �Ű� üũ�ϴ� �κ��� �߰� �Ǿ�� ��.(09/17) 
	if (!AddAccuseNote(dataAR))
	{
		// �߰��� �����ߴٸ�, �̹� �Ű��� ����ڶ�� ���̴�.
		glAC_Dublicate++;
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] Duplicate Accuse : USN ", bus.m_lUSN);
		SendAccuseResult(bus.m_lUSN, bus.m_lAccusedUSN, bus.m_sAccusedID, rid, lTimeSpan, ERRAGENT_DUPLICATE);
		return ERRAGENT_DUPLICATE;
	}

	if(m_AgentQ.Count() > 100)
	{
		m_AgentQ.Clear();
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] accusition queue full(100 < size)..so clear queue");
		//���п� ���� ���䵵 client�� ������ ��. but.. �� ������ ��������.. �߻��� ���ɼ��� ���.
	}

	glAC_TotalAccusing++;
	glAC_LogCount++;
	if(glAC_LogCount > 10)
	{
		string sTemp = ::format("[Accuse] Log : Total/ContentsAccuse/FailTitle/FailContents/FailASN/Duplicate\n ==> [%d]/[%d]/[%d]/[%d]/[%d]/[%d]", \
								glAC_TotalAccusing, glAC_ContentsCnt, glAC_FailTitle, glAC_FailContents, glAC_FailASN, glAC_Dublicate);
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "", sTemp.c_str());

		glAC_LogCount = 0;
	}
	
	m_AgentQ.Push(dataAR);

	::XsigQueueSignal(::GetThreadPool(), this, 0, (WPARAM)AGENT_POSTMSG, 0);
	return ERRAGENT_SUCCESS;
}

LONG Agent::Accuse(CAccuseBus_RoomID& dataAR, AccuseBus& bus)
{
	LONG lRet = ERRAGENT_SUCCESS;

	// Get serialal number
	long lASN;
	{
		if(!GetSerialNoFromDB(lASN))
		{
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, " [Accuse] GetSerialNoFromDB ! ");
			return ERRAGENT_SYSTEM;
		}
		if(lASN < 0)
		{
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, " [Accuse] wrong lASN, \"lASN<0\" ! ");
			return ERRAGENT_SYSTEM;
		}
	}


	DWORD dwASN = lASN;


	// ������ ���Ϸ� ������. 2000)
	if(bus.m_sContent.length() > 1)
	{
		// DB query
		string *pstr = NULL;
		string temp;
		if(bus.m_bRoomTitleAccuse) // ���� �Ű��̸�
		{
			dataAR.GetRoomID().GetInstanceID(temp);
			pstr = &temp;
		}
		if(!SaveChatHistoryToDB(bus, dwASN, pstr))
		{
			// ���⼭ DB������� client�� �˷��� �� ������...Posting�� �ؾ��Ѵ�.
			glAC_FailTitle++;
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] fail SaveChatHistoryToDB() ");
			return ERRAGENT_SYSTEM;
		}
		// DB�� ����� �����ϸ�, ���� ����� ���� queuing..
		// ���� �Ű��̸� ä�ó����� ������ �ʿ����. 
		else
		{
			if(bus.m_bRoomTitleAccuse)
			{
				return ERRAGENT_SUCCESS;
			}
			else
			{
				// CGI CALL �� ���⼭ ������ �� ����. HTTPAgent�� ������ �̷��.
				// -snowyunee
				theHTTPAgent.PostHTTPMsg(bus.m_sUserID, bus.m_sAccusedID, bus.m_sContent, dwASN, bus.m_lUSN, bus.m_lAccusedUSN, dataAR.GetRoomID(), dataAR.GetTime());
				return -1;
			}
		}
	}
	else
	{
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, " Content Length() is low.... : length = [", bus.m_sContent.length(), "]");
		return ERRAGENT_SYSTEM;
	}

	return lRet;
}

BOOL Agent::GetSerialNoFromDB(long &lASN)
{
	xstring strQuery;
	DBGW_String strDBResult;
	BOOL bRet;
	int nDBErrorCode;

	strQuery = _X("PoliceDB|Q|SELECT PL_CASE_SEQ.NEXTVAL FROM DUAL");	

	DWORD before = ::GetTickCount();
	bRet = ExecuteQueryX( QT_NORMAL, strQuery.c_str(), &strDBResult, &nDBErrorCode );
	DWORD after = ::GetTickCount();
	if (after > before + 500)
		theLog.Put(WAR_UK,"GLS_DB_Delay, GetSerialNoFromDB(). during:", after - before, ", Query:", strQuery);

	if(bRet == FALSE)
	{
		theLog.Put(WAR_UK, " DB ExecuteQuery Error [", __FUNCTION__, "] , Query :", strQuery, ", ErrorCode : ", nDBErrorCode);
		return FALSE;
	}

	list<char *> lstData;
	int nDecodeError = DBResultParser(strDBResult, lstData);
	if((bRet == FALSE) || (nDBErrorCode > 0) || (nDecodeError <= 0))
	{
		theLog.Put(WAR_UK, " DB DBResultParser Fail ", "Query : ", strQuery, ", ErrorCode : ", nDBErrorCode, ", nDecodeError:", nDecodeError);
		return FALSE;
	}
	else
	{
		if(lstData.begin() != lstData.end())
		{
			lASN = atoi(*(lstData.begin()));
		}
		else	// DBResultParser �� true���µ�.. �̷� ���� ��������.
		{
			theLog.Put(WAR_UK, " DB DBResultParser Fail ", "Query : ", strQuery, ", ErrorCode : ", nDBErrorCode, ", nDecodeError:", nDecodeError);
			return FALSE;
		}
	}

	return TRUE;
}

int Agent::DBResultParser(const DBGW_String& strDBResult, list<char *>& lstData, char cDelimeter/*=QUERY_DELIMETER*/, BOOL bSP/*=FALSE*/)
{
	int nResult = 1;
	int nSize = strDBResult.GetSize();
	if(nSize <= 0)
		return -1;


	/* format
	* ��S�� + Delimiter + ��0��(SP�� ��쿣 SP ������) + Delimiter +��� string
	* ��F�� + Delimiter + DB �����ڵ� + Delimiter + Stored Proc �����ڵ� + Delimiter + ���� string
	*/
	//string strSrc = strDBResult.GetData();
	//string sResult = GetParseData(strSrc); // F or S	
	//string sTemp = GetParseData(strSrc); // return
	char *pStrSrc = strDBResult.GetData();
	//char *pResult = strtok(pStrSrc, "|");
	char *p = pStrSrc;

	// ���� or ���� (S/F)
	char cResult = *pStrSrc;

	//if( sResult == "F" )
	if(cResult == 'F')
	{
		/*
		// SP�϶��� SP error code�� �־��ش�.
		if( bSP )
		{
		//��F�� + Delimiter + DB �����ڵ� + Delimiter + Stored Proc �����ڵ� + Delimiter + ���� string
		// sTemp�� DB �����ڵ尡 �� ����
		// stored  proc ���� �ڵ�
		//string sSPErrorCode = GetParseData(strSrc);
		//lstData.push_back(sSPErrorCode);
		GetNext(p);
		GetNext(p);
		lstData.push_back(GetNext(p));
		}
		*/
		theLog.Put(DEV_UK, " DB Error : ", pStrSrc);

		return -1;
	}

	// �Ʒ� �� ������ �ǳʶٱ� ����
	// 'S'
	// '0'(SP�� ��쿣 SP ������)
	GetNext(p);
	GetNext(p);

	if( NULL == p )
	{
		// row�� ����
		return 0;
	}
	else if( *p == '\0')
	{
		// row�� ����
		//theLog.Put(DEV_UK, " DB Result NO ROW ");
		return 0;
	}

	char *ptemp = p;
	while(TRUE)
	{
		// field�� data�� ��� �ִ� ���, ""�� �־��ش�.
		lstData.push_back(ptemp);
		ptemp = GetNext(p);

		if( NULL == p)
		{
			break;
		}
		else if( *p == '\0')
		{
			break;
		}

	}

	if ( !lstData.size() )
		return 0;


	return nResult;
}


BOOL Agent::SaveChatHistoryToDB(const AccuseBus &bus, DWORD dwASN, string *pstrRIDAndRoomTitle)
{
	string strQuery;
	DBGW_String strDBResult;

	// insert into PL_CASE
	{
		if (bus.m_sReason.substr(0, 2) == "0|")
		{
			string sContent = xstr2str(bus.m_sContent1) + "[#*@$&]" + bus.m_sReason.substr(2, bus.m_sReason.length() - 2);
			strQuery = ::format("PoliceDB|Q|INSERT INTO PL_CASE (csrl, casedate ,bsrl, blusrid,lsrl, regdate, field, reason_code, memo) VALUES (%d, '%s', %d, '%s', %d, '%s', '%s', '0', '%s')", 
				dwASN, 
				pstrRIDAndRoomTitle == NULL ? "" : pstrRIDAndRoomTitle->c_str(), // ���� �������� ���� ���� ��ȭ��Ű���� ����� ���� ���� "����̵�@��Ÿ��Ʋ" ���·� DB�� �����ϱ�� ��.
				bus.m_lAccusedUSN,
				bus.m_sAccusedID.c_str(),                
				bus.m_lUSN,
				bus.m_sDate.c_str(), 
				bus.m_sType.c_str(), 
				sContent.c_str()
				);
		}
		else
		{
			strQuery = ::format("PoliceDB|Q|INSERT INTO PL_CASE (csrl, casedate ,bsrl, blusrid,lsrl, regdate, field, reason_code, memo) VALUES (%d, '%s', %d, '%s', %d, '%s', '%s', '%s', '%s')", 
				dwASN, 
				pstrRIDAndRoomTitle == NULL ? "" : pstrRIDAndRoomTitle->c_str(), // ���� �������� ���� ���� ��ȭ��Ű���� ����� ���� ���� "����̵�@��Ÿ��Ʋ" ���·� DB�� �����ϱ�� ��.
				bus.m_lAccusedUSN,
				bus.m_sAccusedID.c_str(),                
				bus.m_lUSN,
				bus.m_sDate.c_str(), 
				bus.m_sType.c_str(), 
				bus.m_sReason.c_str(),
				xstr2str(bus.m_sContent1).c_str()
				);	
		}
		BOOL bRet = FALSE;
		int nDBErrorCode;
		
		DWORD before = ::GetTickCount();
		bRet = ExecuteQueryX( QT_NORMAL, strQuery.c_str(), &strDBResult, &nDBErrorCode );
		DWORD after = ::GetTickCount();
		if (after > before + 500)
			theLog.Put(WAR_UK,"GLS_DB_Delay, SaveChatHistoryToDB() 1. during:", after - before, ", Query:", strQuery);

		if(bRet == FALSE)
		{
			theLog.Put(WAR_UK, "ExecuteQuery Error [", __FUNCTION__, "] , Query :", strQuery);
			return FALSE;
		}

		list<char *> lstData;
		int nDecodeError = DBResultParser(strDBResult, lstData);
		if((bRet == FALSE) || (nDBErrorCode > 0) || (nDecodeError < 0))
		{
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] Agent::SaveChatHistoryToDB() QUERY:", strQuery.c_str(), ", nDecodeError:", nDecodeError);
			return FALSE;
		}
	}


	// insert into PL_PROCESS
	{
		BOOL bRet;
		int nDBErrorCode;

		strQuery = ::format("PoliceDB|Q|INSERT INTO PL_PROCESS (csrl, asrl, decision_flag, field, auto_cnt) VALUES (%d, 0, 'N', '%s', 1)", dwASN, bus.m_sType.c_str());
		

		DWORD before = ::GetTickCount();
		bRet = ExecuteQueryX( QT_NORMAL, strQuery.c_str(), &strDBResult, &nDBErrorCode );
		DWORD after = ::GetTickCount();
		if (after > before + 500)
			theLog.Put(WAR_UK,"GLS_DB_Delay, SaveChatHistoryToDB() 2. during:", after - before, ", Query:", strQuery);

		if(bRet == FALSE)
		{
			theLog.Put(WAR_UK, "ExecuteQuery Error [", __FUNCTION__, "] , Query :", strQuery);
			return FALSE;
		}

		list<char *> lstData;
		int nDecodeError = DBResultParser(strDBResult, lstData);
		if((bRet == FALSE) || (nDBErrorCode > 0) || (nDecodeError < 0))
		{
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] Agent::SaveChatHistoryToDB() QUERY:", strQuery.c_str(), ", nDecodeError:", nDecodeError);
			return FALSE;
		}
	}

	return TRUE;
}


BOOL Agent::AddAccuseNote (CAccuseBus_RoomID &ARID)
{
	long lAccusedUSN = ARID.GetAccuseBus().m_lAccusedUSN;
	RoomID  rid = ARID.GetRoomID(); 	
	long lTimeSpan = ARID.GetTime();
	BOOL m_bRoomTitleAccuse =  ARID.GetAccuseBus().m_bRoomTitleAccuse;
	string strRoomTitle = ARID.GetRoomTitle();


	CAccuseNote note(lAccusedUSN, lTimeSpan, rid, m_bRoomTitleAccuse ? &strRoomTitle : NULL);

	//���߿� iterator�� ó���Ұ�..
	//�ð��� �����Ȱ� ����..�տ��� ���� �˻�
	{
		LONG lSize = m_AccuseNote.size();
		if(lSize == 500 || lSize == 1000)
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] Accused Item count room title: ", lSize);
		VecANoteT::iterator iter = m_AccuseNote.begin();
		for(iter; iter != m_AccuseNote.end(); iter++)
		{
			if((note.m_lTimeSpan - (*iter).m_lTimeSpan) < ACCUSE_TIMESPAN)
			{
				if(iter != m_AccuseNote.begin())
				{
					m_AccuseNote.erase(m_AccuseNote.begin(), iter);
				}
				break;
			}
		}
	}

	// ã�Ƽ� ���� �Ű�����, �߰�
	{
		//�ߺ� �Ű����ǿ� �ش��ϸ� return, �ƴϸ� ����Ʈ�� �߰�. �ڿ���(�ֱ�)���� �˻��ϴ� ���� ȿ����.
		VecANoteT::reverse_iterator riter = m_AccuseNote.rbegin();
		for(riter; riter != m_AccuseNote.rend(); riter++)
		{
			if ( (note.m_lUSN == (*riter).m_lUSN) && (note.m_RoomID == (*riter).m_RoomID))
			{
				if (m_bRoomTitleAccuse)
				{
					if (strRoomTitle == (*riter).m_strTitle)
						return FALSE;			
				}
				else
					return FALSE;				
			}
		}

		m_AccuseNote.push_back(note);
	}

	return TRUE;
}

BOOL Agent::DeleteAccuseNote(const long lAccusedUSN, const RoomID & rid, BOOL bRoomTitleAccuse, string *pstr)
{
	TLock lo(this);

	//�ڿ���(�ֱ�)���� �˻��ϴ� ���� ȿ����.
	VecANoteT::iterator iter = m_AccuseNote.begin();
	for(; iter!=m_AccuseNote.end(); iter++)
	{
		if ((lAccusedUSN == (*iter).m_lUSN) && (rid == (*iter).m_RoomID))
		{
			if (bRoomTitleAccuse)
			{
				if ((*iter).m_strTitle == (*pstr))
				{
					m_AccuseNote.erase(iter);
					return TRUE;
				}
			}
			else
			{				
				m_AccuseNote.erase(iter);
				return TRUE;
			}
		}	
	}

	return FALSE;
}

BOOL Agent::DeleteAccuseNote(CAccuseBus_RoomID &ARID)
{
	long lAccusedUSN = ARID.GetAccuseBus().m_lAccusedUSN;
	RoomID rid = ARID.GetRoomID();

	if (ARID.GetAccuseBus().m_bRoomTitleAccuse)
	{
		string strRoomTitle = ARID.GetRoomTitle();
		return DeleteAccuseNote(lAccusedUSN,rid, TRUE, &strRoomTitle);
	}
	else
		return DeleteAccuseNote(lAccusedUSN,rid, FALSE, NULL);
}


BOOL Agent::SendAccuseResult(long nUSN, long lAccusedUSN, xstring sAccusedID, RoomID rID, long lTimeSpan, long nErrorCode)
{
#ifdef _CHS_ACCUSE
	CChannel* pChannel = NULL;
	theChannelDir.GetChannel(ChannelID(rID), &pChannel);
	if(pChannel == NULL)
		return FALSE;

	CAccuseResultAnswer dataARA(nUSN, sAccusedID, nErrorCode, lTimeSpan);
	m_AccuseResultAns.Insert(nUSN, dataARA);
	::XsigQueueSignal(GetThreadPool(), pChannel, ACCUSESIGNAL_AGENTANSWER, (WPARAM)nUSN, (LPARAM)0);
#endif
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
HTTPAgent::HTTPAgent() //: m_dwRefCnt(0)
{
	m_psWData = NULL;
	m_lConnFlag = HTTPAGENTCONNSTATE_STOP;
	m_lHTTPConnFlag = HTTPAGENTSTATE_INVALID;
}

HTTPAgent::~HTTPAgent()
{
	m_lHTTPConnFlag = HTTPAGENTSTATE_INVALID;
	DeactivateSocket();
}

BOOL HTTPAgent::ActivateHTTPAgent()
{
	TLock lo(this);

	BOOL bRet = TRUE;

	if (m_lHTTPConnFlag != HTTPAGENTSTATE_ACTIVATED)
	{
		bRet = ActivateSocket(GetThreadPool());
		if (bRet)
		{
			m_lHTTPConnFlag = HTTPAGENTSTATE_ACTIVATED;
		}
	}
	return bRet;
}

BOOL HTTPAgent::DeactivateHTTPAgent()
{
	TLock lo(this);

	DestroyLink(NULL);
	m_lHTTPConnFlag = HTTPAGENTSTATE_DEACTIVATED;
	return TRUE;
}

void HTTPAgent::DestroyLink(HTTPLink* pLink)
{
	TLock lo(this);

	if (GetSocket() != NULL)
	{
		HTTPLink* pHTTPLink = Detach();
		if (pHTTPLink != NULL)
		{
			delete pHTTPLink;
		}
	}
}

BOOL HTTPAgent::StopHTTPAgent()
{
	TLock lo(this);

	// client ���� �����ϱ�.
	SendAccuseResult(m_pairCurAccuseData.second);

	// ���� ��ȭ, ��ũ ����
	m_lConnFlag = HTTPAGENTCONNSTATE_STOP;
	DestroyLink(NULL);


	// ���� ó���� ���� �����ִ��� ����.. �����ִٸ�, ��� ó������..
	// ����ó�� �Ǵ� ���
	//		�̷��� ó���Ǵ� ���� �̹� �Ű� ó���ϴ� ���ȿ� ���� �Ű��û�� ��쿡�� �̷��� ó���Ǵ� ���̴�.
	//		�� ���� ���� �Ű��û�� ���� ������ �ٷιٷ� ó���ϰ�, ó���� �����ϸ�, ������ return �Ѵ�.
	//		���� ��츦 �����ϰ�, ť�� �־�ΰ� �����Ǿ� ó���Ǵ� ���� ����.
	{
		int nAccuseDataQSize = GetAccuseDataQSize();
		if( nAccuseDataQSize > 0 )
		{
			QueueSignal(::GetThreadPool(), 0, (WPARAM)HTTP_POSTMSG, 0);
		}
	}

	return TRUE;
}

BOOL HTTPAgent::RunHTTPAgent()
{
	TLock lo(this);

	if(m_lHTTPConnFlag != HTTPAGENTSTATE_ACTIVATED)
	{
		theLog.Put(WAR_UK, " GLS_Accuse,HTTPAgent:RunHTTPAgent() ERROR, m_lHTTPConnFlag != HTTPAGENTSTATE_ACTIVATED, m_lHTTPConnFlag:", m_lHTTPConnFlag);
		return FALSE;
	}

	// �̷� ��Ȳ�� ������ �Ű� ó���ϴ� ���߿� �ٸ� �Ű� ���� ��쿡 �����.
	// �������´� �ƴϹǷ� true�� return ����.
	// �� ���� ������ �Ű�ó���� �Ϸ�ǰ��� �ٽ� �õ��ȴ�.
	if (m_lConnFlag != HTTPAGENTCONNSTATE_STOP)
	{
		theLog.Put(WAR_UK, "GLS_Accuse, HTTPAgent:RunHTTPAgent() m_lConnFlag != HTTPAGENTCONNSTATE_STOP, m_lConnFlag:", m_lConnFlag);
		return TRUE;
	}

	// �̷� ���� ������ �ȵȴ�.
	if(m_apSocket != NULL)
	{
		theLog.Put(WAR_UK, " GLS_Accuse,HTTPAgent:RunHTTPAgent() will return FALSE. Because m_apSocket!= NULL");
		return FALSE;
	}

	HTTPLink* pLink = new HTTPLink;

	if (!pLink->Socket()) 
	{
		delete pLink;
		theLog.Put(WAR_UK, " GLS_Accuse,HTTPAgent:RunHTTPAgent() will return FALSE. Because pLink->Socket()== NULL");
		return FALSE;
	}

	BOOL bRet = Attach(pLink);
	if (!bRet)
	{
		delete pLink;
		theLog.Put(WAR_UK, " GLS_Accuse,HTTPAgent:RunHTTPAgent() called SopHTTPAgent(). Because Attch(pLink) is Failed!");
		return FALSE;
	}

	m_lConnFlag = HTTPAGENTCONNSTATE_ATTACH;

	return TRUE;
}

BOOL HTTPAgent::ConnectFileServer(HTTPLink *pLink)
{
	TLock lo(this);

	if(m_lConnFlag != HTTPAGENTCONNSTATE_ATTACH)
	{
		theLog.Put(WAR_UK, "GLS_Accuse, HTTPAgent:Connect() m_lConnFlag != HTTPAGENTCONNSTATE_STOP, m_lConnFlag:", m_lConnFlag);
		return FALSE;
	}

	// �� ���ķδ� �̹� attach �� �����Ͽ����Ƿ�,
	// OnError() �� ���ؼ� delete ���� ó���Ѵ�.
	if(!pLink->Connect("boardw.pmang.com", 80))
	{
		if(::WSAGetLastError() != WSAEWOULDBLOCK) 
		{
			TRACE1("HTTPAgent : for file Write - fail to connect(%s)\n", WSAGetErrorDescription(::WSAGetLastError()));				
			theLog.Put(WAR_UK, " GLS_Accuse,HTTPAgent:RunHTTPAgent() called SopHTTPAgent(). Because pLink->Connect is Failed! and WSAGetLastError(",::WSAGetLastError(),") != WSAEWOULDBLOCK");
			return FALSE;
		}
	}

	m_lConnFlag = HTTPAGENTCONNSTATE_CONN;
	return TRUE;
}

//
//	Header �κ�, Contents�� ������ ��� �κ�, contents�κ����� ������ �����Ѵ�. 
// 
string s_FN_userid = ::format("content-disposition: form-data; name=\"usrid\"\r\n");
string s_FN_newsgroup = ::format("content-disposition: form-data; name=\"newsgroup\"\r\n");
string s_FN_ano = ::format("content-disposition: form-data; name=\"ano\"\r\n");	// serial
string s_FN_editor = ::format("content-disposition: form-data; name=\"editor\"\r\n");
string s_FN_event = ::format("content-disposition: form-data; name=\"event\"\r\n");
string s_FN_text = ::format("content-disposition: form-data; name=\"text\"\r\n");
////////////////////////////////////////////////////////////////////
//usrid= lusrid
//newsgroup= sayclub._nwz_police.board1
//ano : DB���� insert�ϰ� �޾ƿ� ID
//editor= 'text'
//event=post
//text= chat Contents
////////////////////////////////////////////////////////////////////

BOOL HTTPAgent::OnConnect(TSocket* pSocket, int nErrorCode)
{
	if(nErrorCode || !pSocket)
	{
		if(!pSocket)
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "+++++ ASSERT : HTTPAgent::OnConnect Error! : Socket NULL");
		else
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "+++++ ASSERT : HTTPAgent::OnConnect Error! : [", nErrorCode, "]");
		StopHTTPAgent();
		theLog.Put(WAR_UK, "GLS_Accuse,HTTPAgent:OnConnect() called SopHTTPAgent(). Because (nErrorCode[",nErrorCode,"] || !pSocket)");
		return FALSE;	
	}

	m_lConnFlag = HTTPAGENTCONNSTATE_RUN;
	OnWriteFile();
	return TRUE;
}

void HTTPAgent::OnWriteFile()
{
	DWORD dwASN = 0UL;
	CAccuseContent_RoomID pairContentT;

	// �� ���� 1���� �ؾ��Ѵ�.
	// �ֳĸ�, http connection �̱� �����̴�.
	// -snowyunee
	if(PopAccuseData(m_pairCurAccuseData))
	{
		dwASN = m_pairCurAccuseData.first;
		pairContentT = m_pairCurAccuseData.second;
		if(!WriteFile(dwASN, pairContentT))
		{
			StopHTTPAgent();
			theLog.Put(WAR_UK, "GLS_Accuse,HTTPAgent:OnWriteFile() called SopHTTPAgent(). Because WriteFile() is Failed. dwASN:",dwASN);
		}
	}

}

BOOL HTTPAgent::WriteFile(DWORD dwASN, CAccuseContent_RoomID& pairContent)
{
	string sContent;

	//string s_http("POST /~pesim/FO/board/nfs_post.nwz HTTP/1.0\r\n");	// for test
	string s_http("POST /board/nfs_post.nwz	HTTP/1.0\r\n");
	string s_bound;
	if(!MakeBoundary(s_bound))
	{
		glAC_FailContents++;
		theLog.Put(WAR_UK, "GLS_Accuse, HTTPAgent::WriteFile(). MakeBoundary(",s_bound,") failed");
		return FALSE;
	}
	string s_contenttype = ::format("Content-type: multipart/form-data, boundary=%s\r\n", s_bound.c_str());

	string s_sbound = ::format("--%s\r\n", s_bound.c_str());
	string s_ebound = ::format("--%s--\r\n", s_bound.c_str());

	// �Ű� ������ (contents ����)
	string _srl = ::format("%d", dwASN);
	string _newsgroup("sayclub._nwz_police.board1");
	string _editor("text");
	string _event("modify");
	string newline = "\r\n";

	string strUserID = "";
	strUserID = pairContent.GetUserID();
	string s_rData2 = s_sbound + s_FN_userid + newline + strUserID + newline +
						s_sbound + s_FN_newsgroup + newline + _newsgroup + newline + 
						s_sbound + s_FN_ano + newline + _srl + newline +
						s_sbound + s_FN_editor + newline + _editor + newline +
						s_sbound + s_FN_event + newline + _event + newline;
	// �Ű� ������ (contents + end boundary)
	string s_rData3 = s_sbound + s_FN_text + newline + pairContent.GetContents() + newline +
						s_ebound;

	// �Ű� ���� ��ü ������ ������ ���� ���ؾ� �Ѵ�. 
	string s_contentlen = ::format("Content-Length: %d\r\n\r\n", s_rData2.length() + s_rData3.length());
	string s_rData1 = s_http + s_contenttype + s_contentlen;


	if(glAC_FailContents%2 == 0)
	{
		if(SendHTTPData(s_rData1))
		{
			if(SendHTTPData(s_rData2))
			{
				if(SendHTTPData(s_rData3))
				{
					return TRUE;
				}
				else
				{
					string sLogTemp = ::format("[Accuse] HTTPAgent::WriteFile() : SendHTTPData(s_rData3) : Error[%d], size (%d)\n%s \n", \
												GetLastError(), s_rData3.size(), s_rData3.c_str());
					theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, sLogTemp.c_str());
				}
			}
			else
			{
				string sLogTemp = ::format("[Accuse] HTTPAgent::WriteFile() : SendHTTPData(s_rData2) : Error[%d], size (%d) \n%s \n", \
											GetLastError(), s_rData2.size(), s_rData2.c_str());
				theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, sLogTemp.c_str());
			}
		}
		else
		{
			string sLogTemp = ::format("[Accuse] HTTPAgent::WriteFile() : SendHTTPData(s_rData1) : Error[%d], size (%d) \n%s \n", \
										GetLastError(), s_rData1.size(), s_rData1.c_str());
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, sLogTemp.c_str());
		}
	}
	else
	{
		string strAllData = s_rData1;
		strAllData += s_rData2;
		strAllData += s_rData3;

		if(SendHTTPData(strAllData))
		{
			return TRUE;
		}
		else
		{
			string sLogTemp = ::format("[Accuse] HTTPAgent::WriteFile() : SendHTTPData(strAllData) [total] : Error[%d], size (%d)\n%s \n", \
										GetLastError(), strAllData.size(), strAllData.c_str());
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, sLogTemp.c_str());
		}
	}

	glAC_FailContents++;
	return FALSE;
}

BOOL HTTPAgent::MakeBoundary(string & sBound)
{
	//Boundary�� Unique�ϰ� ����� ���� ..session scope..���� Unique�ϸ� �Ǳ� ������ �Ʒ��Ͱ��� ������� ���.
	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	srand(sys_time.wMilliseconds);
	DWORD ret = rand();
	if(ret == m_dwBound)
		ret = rand();
	m_dwBound = ret;
	sBound = ::format("bound%d", ret);
	return TRUE;
}

BOOL HTTPAgent::OnClose(TSocket* pSocket, int nErrorCode)
{
	return OnError(pSocket, FD_CLOSE, nErrorCode);
}

STDMETHODIMP_(void) HTTPAgent::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	if ((hSignal == 0) && (wParam == HTTP_POSTMSG))
	{
		// attach
		if( ! RunHTTPAgent())
		{
			// RunHTTPAgent() �� �����ߴٸ�, �̹� �޽����� ���з� ó���Ѵ�.
			PairAccuseData accuseData;
			if( PopAccuseData(accuseData) )
			{
				// �������� ������ �ְ�, ����� �˷��ش�.
				accuseData.second.SetErrStatus(ERRAGENT_SYSTEM);
				SendAccuseResult(accuseData.second);
			}
		}
		else	// connect
		{
			HTTPLink *pLink = GetSocket();
			if( ! ConnectFileServer(pLink) )
			{
				StopHTTPAgent();
			}
		}
	}
	else
	{
		TLock lo(this);
		TBase::OnSignal(hSignal, wParam, lParam);
	}
}

BOOL HTTPAgent::OnError(HTTPLink* pLink, long lEvent, int nErrorCode)
{
	StopHTTPAgent();
	return FALSE;
}

BOOL HTTPAgent::OnReceive(HTTPLink* pLink, int lErrorCode)
{	
	if(!TBase::OnReceive(pLink, lErrorCode))
	{
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] TBase::OnReceive(pLink, lErrorCode) \n");
		return FALSE;
	}

	string& r_buff = pLink->GetRcvBuf();

	return ParseHTTP(r_buff);
}

BOOL HTTPAgent::SendHTTPData(string & sData)
{
	HTTPLink* pLink = (HTTPLink*)GetSocket();
	if(!pLink)
	{
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] fail GetSocket from SendHTTPData() \n");
		return FALSE;
	}

	return pLink->DoSend(sData);
}

void HTTPAgent::PostHTTPMsg(xstring sUserID, xstring sAccusedID, xstring sContent, DWORD dwASN, long lUSN, long lAccusedUSN, RoomID rID, long lTimeSpan)
{
	glAC_ContentsCnt++;
	PushAccuseData(sUserID, sAccusedID, sContent, dwASN, lUSN, lAccusedUSN, rID, lTimeSpan);
	QueueSignal(::GetThreadPool(), 0, (WPARAM)HTTP_POSTMSG, 0);
}

BOOL HTTPAgent::PushAccuseData(xstring sUserID, xstring sAccusedID, xstring sContent, DWORD dwASN, long nUSN, long lAccusedUSN, RoomID rID, long lTimeSpan)
{
	m_CS.Lock();
	if(m_vecAccuseDataQ.size() > 100)
	{
		m_vecAccuseDataQ.clear();
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] clear HTTP queue.. because queue full..");
	}

	CAccuseContent_RoomID dataAR(nUSN, lAccusedUSN, rID, sUserID, sAccusedID, xstr2str(sContent), lTimeSpan);
	PairAccuseData pairAD(dwASN, dataAR);
	m_vecAccuseDataQ.push_back(pairAD);
	m_CS.Unlock();
	return TRUE;
}

BOOL HTTPAgent::PopAccuseData(PairAccuseData& pairAccuseData)
{
	m_CS.Lock();
	if(m_vecAccuseDataQ.empty()) //.size() < 1)
	{
		m_CS.Unlock();
		return FALSE;
	}
	pairAccuseData = m_vecAccuseDataQ.front();
	m_vecAccuseDataQ.erase(m_vecAccuseDataQ.begin()); // m_vecContentQ.erase(Remove(..
	m_CS.Unlock();
	return TRUE;
}

int HTTPAgent::GetAccuseDataQSize()
{
	m_CS.Lock();
	int m_nRet = m_vecAccuseDataQ.size();
	m_CS.Unlock();
	return m_nRet;
}

BOOL HTTPAgent::ParseHTTP(string& r_buff)
{
	strpos spos = r_buff.find("200 success!");
	if(spos != string::npos)
	{
		m_pairCurAccuseData.second.SetErrStatus(ERRAGENT_SUCCESS);
		r_buff.erase(0, spos);
		return TRUE;
	}
	theLog.Put(WAR_UK, " GLS_Accuse"_COMMA, __FUNCTION__, "[Accuse] <<fail cgi call>> ", r_buff.c_str());

	//Send to client.. 

	return FALSE;
}

BOOL HTTPAgent::SendAccuseResult(CAccuseContent_RoomID& AccuseContentRoomID)
{
	long nUSN = AccuseContentRoomID.GetUSN();
	long lAccusedUSN = AccuseContentRoomID.GetAccusedUSN();
	xstring sAccusedID = AccuseContentRoomID.GetAccusedID();
	RoomID rID = AccuseContentRoomID.GetRoomID();
	long lTimeSpan = AccuseContentRoomID.GetTime();
	long lErrorCode = AccuseContentRoomID.GetErrStatus();

	if(lErrorCode != ERRAGENT_SUCCESS)
	{
		// ���� �����Ƿ�, �ٽ� �Ű��� �� �ֵ��� �Ű� history���� ����.
		theAccuseAgent.DeleteAccuseNote(lAccusedUSN, rID);
	}



#if defined(_CHS_ACCUSE)
	CChannel* pChannel = NULL;
	theChannelDir.GetChannel(ChannelID(rID), &pChannel);
	if(pChannel == NULL)
		return FALSE;

	CAccuseResultAnswer dataARA(nUSN, sAccusedID, lErrorCode, lTimeSpan);
	m_AccuseResultAns.Insert(nUSN, dataARA);
	::XsigQueueSignal(GetThreadPool(), pChannel, ACCUSESIGNAL_AGENTANSWER, (WPARAM)nUSN, (LPARAM)1);
#endif
#if defined(_GLS_ACCUSE)
	// �Ʒ��� ������ ������ �߻����� �ʵ���..�־�� ����.
	nUSN;
	lTimeSpan;
	lErrorCode;
#endif

	return TRUE;
}
