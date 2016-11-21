////////////////////////////////////////////////////////////////////////////////////////////
// Agent.cpp: implementation of the Agent class.
//
// for 신고 : gDBGW접근과 CGI call을 통한 file server접근 제공.
//			: PostAccusationMsg()를 신고 요청시 호출.
//			: OnAccuseReq(..)에서 신고에 필요한 테이터의 생성 및 setting..
//					--> 신고 데이터의 header정보에 해당. 적절히 정보를 setting해주어야 함.		

//
// Agent : 신고 타이틀만 gDBGW에 저장(방제신고, 대화신고).
// HTTPAgent : 대화 내용 보드서버에 저장. nfs_post.nwz 에서 IP 체크함. 
//			접근 서버	: boardw.pmang.com/80
//			제한 IP		: 현재 211.172.245.*, 211.43.216.* 체크하고 있음. 2005.4.15 by kimsk
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
// CGI Call을 통한 처리에 있어서의 문제.
// 1. 장시간 사용이 없을 경우 Process를 다시 띄워야 함. (10초 정도 걸린다고 함.)
// 2. 1. 및 기타 이유로 실패가 많음. (테스트 중에도 수시로 실패함.)
//
// => 이전에 문제라고 coment 되어 있었으나, 여전히 그러한지는 잘 모르겠음
// -snowyunee
//////////////////////////////////////////////////////////////////////

Agent theAccuseAgent;
HTTPAgent theHTTPAgent;


////////////////////// 신고 로그 카운트 2004.2.2 ////////////////////////
long glAC_TotalAccusing = 0L;		/// 총 신고수
long glAC_ContentsCnt = 0;			/// 채팅 내용 신고수
long glAC_FailTitle = 0L;			/// 타이틀(신고자, 신고시간, 사유 등 내용을 제외한 모든것)
long glAC_FailContents = 0L;		/// 상세 내용(채팅 내용)
long glAC_FailASN = 0L;				/// ASN 번호 받아오기.
long glAC_Dublicate = 0L;			/// 반복 신고.
long glAC_LogCount = 0L;			/// 몇번마다 찍을까? 5번에 한번찍게 하자.
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
	if((LONG)hSignal == 0L) // user define event , OutThreadPool을 사용.
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
						// 실패 했으므로, 다시 신고할 수 있도록 신고 history에서 제거.
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
				// 이 경우는 CGI call을 해서 채팅내용을 저장해야 하는 경우이다.
				// HTTPAgent 에서 응답할 것이다.
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
	// 티파니 적용에 따라 중복 신고를 체크하는 부분이 추가 되어야 함.(09/17) 
	if (!AddAccuseNote(dataAR))
	{
		// 추가에 실패했다면, 이미 신고한 사용자라는 뜻이다.
		glAC_Dublicate++;
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] Duplicate Accuse : USN ", bus.m_lUSN);
		SendAccuseResult(bus.m_lUSN, bus.m_lAccusedUSN, bus.m_sAccusedID, rid, lTimeSpan, ERRAGENT_DUPLICATE);
		return ERRAGENT_DUPLICATE;
	}

	if(m_AgentQ.Count() > 100)
	{
		m_AgentQ.Clear();
		theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] accusition queue full(100 < size)..so clear queue");
		//실패에 대한 응답도 client로 보내야 함. but.. 이 에러는 무시하자.. 발생할 가능성이 희박.
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


	// 무조건 파일로 보낸다. 2000)
	if(bus.m_sContent.length() > 1)
	{
		// DB query
		string *pstr = NULL;
		string temp;
		if(bus.m_bRoomTitleAccuse) // 방제 신고이면
		{
			dataAR.GetRoomID().GetInstanceID(temp);
			pstr = &temp;
		}
		if(!SaveChatHistoryToDB(bus, dwASN, pstr))
		{
			// 여기서 DB저장실패 client에 알려줄 수 있지만...Posting을 해야한다.
			glAC_FailTitle++;
			theLog.Put(WAR_UK, "GLS_Accuse"_COMMA, "[Accuse] fail SaveChatHistoryToDB() ");
			return ERRAGENT_SYSTEM;
		}
		// DB에 기록이 성공하면, 파일 기록을 위해 queuing..
		// 방제 신고이면 채팅내용을 저장할 필요없다. 
		else
		{
			if(bus.m_bRoomTitleAccuse)
			{
				return ERRAGENT_SUCCESS;
			}
			else
			{
				// CGI CALL 은 여기서 응답할 수 없다. HTTPAgent로 응답을 미룬다.
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
		else	// DBResultParser 도 true였는데.. 이런 일은 없을꺼다.
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
	* ’S’ + Delimiter + ‘0’(SP의 경우엔 SP 성공값) + Delimiter +결과 string
	* ’F’ + Delimiter + DB 에러코드 + Delimiter + Stored Proc 에러코드 + Delimiter + 에러 string
	*/
	//string strSrc = strDBResult.GetData();
	//string sResult = GetParseData(strSrc); // F or S	
	//string sTemp = GetParseData(strSrc); // return
	char *pStrSrc = strDBResult.GetData();
	//char *pResult = strtok(pStrSrc, "|");
	char *p = pStrSrc;

	// 성공 or 실패 (S/F)
	char cResult = *pStrSrc;

	//if( sResult == "F" )
	if(cResult == 'F')
	{
		/*
		// SP일때만 SP error code를 넣어준다.
		if( bSP )
		{
		//‘F’ + Delimiter + DB 에러코드 + Delimiter + Stored Proc 에러코드 + Delimiter + 에러 string
		// sTemp에 DB 에러코드가 들어가 있음
		// stored  proc 에러 코드
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

	// 아래 두 가지를 건너뛰기 위해
	// 'S'
	// '0'(SP의 경우엔 SP 성공값)
	GetNext(p);
	GetNext(p);

	if( NULL == p )
	{
		// row가 없음
		return 0;
	}
	else if( *p == '\0')
	{
		// row가 없음
		//theLog.Put(DEV_UK, " DB Result NO ROW ");
		return 0;
	}

	char *ptemp = p;
	while(TRUE)
	{
		// field에 data가 비어 있는 경우, ""을 넣어준다.
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
				pstrRIDAndRoomTitle == NULL ? "" : pstrRIDAndRoomTitle->c_str(), // 방제 조작으로 제제 수위 약화시키려는 사용자 대응 위해 "룸아이디@룸타이틀" 형태로 DB에 저장하기로 함.
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
				pstrRIDAndRoomTitle == NULL ? "" : pstrRIDAndRoomTitle->c_str(), // 방제 조작으로 제제 수위 약화시키려는 사용자 대응 위해 "룸아이디@룸타이틀" 형태로 DB에 저장하기로 함.
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

	//나중에 iterator로 처리할것..
	//시간이 오래된거 제거..앞에서 부터 검색
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

	// 찾아서 없는 신고였으면, 추가
	{
		//중복 신고조건에 해당하면 return, 아니면 리스트에 추가. 뒤에서(최근)부터 검색하는 것이 효과적.
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

	//뒤에서(최근)부터 검색하는 것이 효과적.
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

	// client 에게 응답하기.
	SendAccuseResult(m_pairCurAccuseData.second);

	// 상태 변화, 링크 정리
	m_lConnFlag = HTTPAGENTCONNSTATE_STOP;
	DestroyLink(NULL);


	// 아직 처리할 것이 남아있는지 보고.. 남아있다면, 계속 처리하자..
	// 지연처리 되는 경우
	//		이렇게 처리되는 경우는 이번 신고를 처리하는 동안에 들어온 신고요청인 경우에만 이렇게 처리되는 것이다.
	//		그 밖의 경우는 신고요청이 들어올 때마다 바로바로 처리하고, 처리에 실패하면, 에러를 return 한다.
	//		위의 경우를 제외하고, 큐에 넣어두고 지연되어 처리되는 일은 없다.
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

	// 이런 상황은 이전에 신고를 처리하는 도중에 다른 신고가 들어온 경우에 생긴다.
	// 오류상태는 아니므로 true를 return 하자.
	// 이 경우는 이전의 신고처리가 완료되고나서 다시 시도된다.
	if (m_lConnFlag != HTTPAGENTCONNSTATE_STOP)
	{
		theLog.Put(WAR_UK, "GLS_Accuse, HTTPAgent:RunHTTPAgent() m_lConnFlag != HTTPAGENTCONNSTATE_STOP, m_lConnFlag:", m_lConnFlag);
		return TRUE;
	}

	// 이런 경우는 있으면 안된다.
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

	// 이 이후로는 이미 attach 에 성공하였으므로,
	// OnError() 를 통해서 delete 등을 처리한다.
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
//	Header 부분, Contents를 제외한 모든 부분, contents부분으로 나누어 전송한다. 
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
//ano : DB에서 insert하고 받아온 ID
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

	// 한 번에 1개씩 해야한다.
	// 왜냐면, http connection 이기 때문이다.
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

	// 신고 데이터 (contents 제외)
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
	// 신고 데이터 (contents + end boundary)
	string s_rData3 = s_sbound + s_FN_text + newline + pairContent.GetContents() + newline +
						s_ebound;

	// 신고를 위한 전체 데이터 길이의 합을 구해야 한다. 
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
	//Boundary를 Unique하게 만들기 위해 ..session scope..에서 Unique하면 되기 때문에 아래와같은 방식으로 충분.
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
			// RunHTTPAgent() 에 실패했다면, 이번 메시지를 실패로 처리한다.
			PairAccuseData accuseData;
			if( PopAccuseData(accuseData) )
			{
				// 에러값을 지정해 주고, 결과를 알려준다.
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
		// 실패 했으므로, 다시 신고할 수 있도록 신고 history에서 제거.
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
	// 아래는 컴파일 오류가 발생하지 않도록..넣어둔 것임.
	nUSN;
	lTimeSpan;
	lErrorCode;
#endif

	return TRUE;
}
