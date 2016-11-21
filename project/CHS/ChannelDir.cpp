//
// ChannelDir.cpp
//

#include "stdafx.h"
#include "ChannelDir.h"
#include "LRBHandler.h"
#include "Control.h"
#include "TestChannelMgr.h"

CChannelDir theChannelDir;
///////////////////////////////////////////////////////////////////////////////////
//
void CChannelDir::AddGLSLogicalAddr(const LRBAddress & lrbAddr, const NSAP & nsap, BOOL bMode)
{
	TLock lo(this);
	TGLSTable::iterator it = m_mapGLS.find(lrbAddr);
	if(it != m_mapGLS.end() && !bMode)
	{
		return;
	}
	else if( it != m_mapGLS.end() && bMode)
	{
		m_mapGLS.erase(it);
	}

	NSAP& nsap2 = m_mapGLS[lrbAddr];
	nsap2.BCopy(nsap);
	
	string sip;
	nsap2.GetIP(sip);
	LOG(INF_UK, "CHS_CChannelDir"_LK, "==== AddGLSLogicalAddr : new gls register : IP =[", sip, "], LRBAddress : [", lrbAddr.GetString(), "]!!!");
}

void CChannelDir::RemGLSLogicalAddr(LRBAddress & lrbAddr)
{	//이건 GLS가 죽었다고 알려올 때에만 동작.
	//GLS가 연결된 LRB가 죽었을 경우에도 동작해야 함. --> GLS가 다른 LRB에 붙었을 경우 재등록 하기로 함. 12/21
	TLock lo(this);
	TGLSTable::iterator it = m_mapGLS.find(lrbAddr);
	if(it == m_mapGLS.end())
	{
		LOG(INF_UK, "CHS_CChannelDir"_LK, "===== RemGLSLogicalAddr : Not found gls logical address : [", lrbAddr.GetString().c_str(), "] =====");
		return;
	}	
	////////////// 해당 게임방 채널에서 삭제. kimsk 2004.1.19 //////////////
	RemNGS(it->second);
	////////////////////////////////////////////////////////////////////////////////////

	m_mapGLS.erase(it);
}

BOOL CChannelDir::GetGLSNsap(LRBAddress & lrbAddr, NSAP & nsap)
{
	TLock lo(this);
	TGLSTable::iterator it = m_mapGLS.find(lrbAddr);
	if(it == m_mapGLS.end())
	{
		return FALSE;
	}
	nsap = it->second;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// to LRB

void CChannelDir::RegisterAllInstance()
{
	TLock lo(this);
}

void CChannelDir::DeregisterAllInstance()
{
	TLock lo(this);
}

void CChannelDir::PMSAnnounceReqInSystem(xstring & sMsg)
{
	vector<CChannelPtr> vt;
	{
		TLock lo(this);
		ForEachElmt(TChannelDir, m_mapChannelDir, i, j)
			vt.push_back(i->second);
	}

	if(vt.size() == 0)
		return;

	PayloadCHSCli msg(PayloadCHSCli::msgAnnounceNtf_Tag, MsgCHSCli_AnnounceNtf(0L, /*str2xstr*/(sMsg)));
	GBuf buf;
	VALIDATE(::LStore(buf, msg));

	for(unsigned i = 0; i < vt.size(); i++)
	{
		CChannelPtr& spChannel = vt[i];
		PMSAnnounceAgent(spChannel, buf);
	}
}
void CChannelDir::PMSAnnounceReqInCategory(long lSSN, DWORD lCat, xstring & sMsg)
{
	vector<CChannelPtr> vt;
	{
		TLock lo(this);
		ForEachElmt(TChannelDir, m_mapChannelDir, i, j)
		{
			if( i->first.m_lSSN == lSSN && i->first.m_dwCategory == lCat)
				vt.push_back(i->second);
		}
	}

	if(vt.size() == 0)
		return;
	PayloadCHSCli msg(PayloadCHSCli::msgAnnounceNtf_Tag, MsgCHSCli_AnnounceNtf(0L, /*str2xstr*/(sMsg)));
	GBuf buf;
	VALIDATE(::LStore(buf, msg));

	for(unsigned i = 0; i < vt.size(); --i)
	{
		CChannelPtr& spChannel = vt[i];
		PMSAnnounceAgent(spChannel, buf);
	}
}
void CChannelDir::PMSAnnounceReqInSSN(long lSSN, xstring & sMsg)
{
	vector<CChannelPtr> vt;
	{
		TLock lo(this);
		ForEachElmt(TChannelDir, m_mapChannelDir, i, j)
		{
			if(i->first.m_lSSN == lSSN)
				vt.push_back(i->second);
		}
	}

	if(vt.size() == 0)
		return;

	PayloadCHSCli msg(PayloadCHSCli::msgAnnounceNtf_Tag, MsgCHSCli_AnnounceNtf(0L, /*str2xstr*/(sMsg)));
	GBuf buf;
	VALIDATE(::LStore(buf, msg));

	for(unsigned i = 0; i < vt.size(); i++)
	{
		CChannelPtr& spChannel = vt[i];
		PMSAnnounceAgent(spChannel, buf);
	}
}

void CChannelDir::PMSAnnounceAgent(CChannel* pChannel, const GBuf& buf)
{
	LPXBUF pBuf = ::XbufCreate(buf.GetXBuf());
	ASSERT(pBuf);
	if(!pBuf)
	{
		LOG(INF_UK, "CHS_CChannelDir"_LK, "+++++ ASSERT :PMSAnnounceAgent +++++");
		return;
	}

	::XsigQueueSignal(::GetChannelThreadPool(), pChannel, 0,  (WPARAM)CHANNEL_ANNOUNCEMSG, (LPARAM)pBuf);
}

BOOL CChannelDir::GetChannel(ChannelID channelID, CChannel ** ppChannel)
{
	//ASSERT(ppChannel && !*ppChannel);
	if(!(ppChannel && !*ppChannel))
	{
		LOG(INF_UK, "CHS_CChannelDir"_LK, "+++++ ASSERT : +++++");
		return FALSE;
	}

	TLock lo(this);
	{
		TChannelDir::iterator it = m_mapChannelDir.find(channelID);
		if(it == m_mapChannelDir.end()) 
		{
			LOG(INF_UK, "CHS_CChannelDir"_LK, "Not found channel : CChannelDir::GetChannel() ");
			return FALSE;
		}
		*ppChannel = it->second;
		(*ppChannel)->AddRef();
	}
	return TRUE;
}

BOOL CChannelDir::ParseChannelFromString(string sBuf, ChannelBaseInfoList& lstChannelInfo, char cDelimeter)
{
	ChannelBaseInfo channel;
	ChannelRegistInfo channelForLB;

	int nSize = sBuf.size();

	char* sTempBuf = new char[nSize + 1];

	list<string> lstData;

	memset(sTempBuf, '\0', nSize + 1);

	sprintf(sTempBuf, "%s", sBuf.c_str());

	if (sTempBuf[0] != 'S' || sTempBuf[1] != cDelimeter || sTempBuf[2] != '0' || sTempBuf[3] != cDelimeter || sTempBuf[4] == cDelimeter)
		return FALSE;

	string sTemp;

	for (int nCursor = 4; nCursor < nSize + 1; nCursor++)
	{
		if (sTempBuf[nCursor] == cDelimeter)
		{
			lstData.push_back(sTemp);

			sTemp.erase();
		}
		else if (sTempBuf[nCursor] == '\\' && sTempBuf[nCursor+1] == cDelimeter)
			sTemp += sTempBuf[++nCursor];
		else if (sTempBuf[nCursor] == '\\' && sTempBuf[nCursor+1] == '\\')
			sTemp += sTempBuf[++nCursor];
		else
			sTemp += sTempBuf[nCursor];
	}

	LONG lChannelSize = lstData.size() / MAX_FIELD_COUNT;

	list<string>::iterator itr = lstData.begin();
	while (lChannelSize > 0)
	{
		channel.m_channelID.m_lSSN = channelForLB.m_channelID.m_lSSN = atoi((*itr).c_str());
		itr++;
		channel.m_channelID.m_dwCategory = channelForLB.m_channelID.m_dwCategory = atoi((*itr).c_str());
		itr++;
		channel.m_channelID.m_dwGCIID = channelForLB.m_channelID.m_dwGCIID = atoi((*itr).c_str());
		itr++;
		channel.m_lCACode = channelForLB.m_lACCode = atoi((*itr).c_str());
		itr++;
		channel.m_lMaxCount = channelForLB.m_lMaxCount = atoi((*itr).c_str());
		itr++;
		channel.m_lLimitCount = channelForLB.m_lLimitCount = atoi((*itr).c_str());
		itr++;
		channel.m_sCategoryName = channelForLB.m_sCategoryName = *itr;
		itr++;
		channel.m_sTitle = channelForLB.m_sTitle = *itr;
		itr++;
		// channel 등록이 변경되면서 .. 08/20
		channelForLB.m_lChannelType = atoi((*itr).c_str());
		itr++;
		channelForLB.m_lChannelOpenState = atoi((*itr).c_str());
		itr++;

		channel.m_nsapCHS = theChannelDir.GetCHSNsap();
		lstChannelInfo.push_back(channel);
		m_lstChannelInfoForLBReg.push_back(channelForLB);
		channel.Clear();
		channelForLB.Clear();
		if (itr == lstData.end())
			break;
		lChannelSize--;
	}

	return TRUE;
}

BOOL CChannelDir::ParseChannelFromStringW(wstring sBuf, ChannelBaseInfoList& lstChannelInfo, WCHAR cDelimeter)
{
	ChannelBaseInfo channel;
	ChannelRegistInfo channelForLB;

	int nSize = sBuf.size();

	WCHAR* sTempBuf = new WCHAR[nSize + 1];

	list<wstring> lstData;

	memset(sTempBuf, L'\0', nSize + 1);

	swprintf(sTempBuf, L"%s", sBuf.c_str());

	if (sTempBuf[0] != L'S' || sTempBuf[1] != cDelimeter || sTempBuf[2] != L'0' || sTempBuf[3] != cDelimeter || sTempBuf[4] == cDelimeter)
	{
		delete[] sTempBuf;
		return FALSE;
	}

	wstring sTemp;

	for (int nCursor = 4; nCursor < nSize + 1; nCursor++)
	{
		if (sTempBuf[nCursor] == cDelimeter)
		{
			lstData.push_back(sTemp);

			sTemp = L"";
		}
		else if ( sTempBuf[nCursor] == L'\\' && sTempBuf[nCursor+1] == cDelimeter)
			sTemp += sTempBuf[++nCursor];
		else if (sTempBuf[nCursor] == L'\\' && sTempBuf[nCursor+1] == L'\\')
			sTemp += sTempBuf[++nCursor];
		else
			sTemp += sTempBuf[nCursor];
	}

	LONG lChannelSize = lstData.size() / MAX_FIELD_COUNT;

	list<wstring>::iterator itr = lstData.begin();
	while (lChannelSize > 0)
	{
		channel.m_channelID.m_lSSN = channelForLB.m_channelID.m_lSSN = _wtoi((*itr).c_str());
		itr++;
		channel.m_channelID.m_dwCategory = channelForLB.m_channelID.m_dwCategory = _wtoi((*itr).c_str());
		itr++;
		channel.m_channelID.m_dwGCIID = channelForLB.m_channelID.m_dwGCIID = _wtoi((*itr).c_str());
		itr++;
		channel.m_lCACode = channelForLB.m_lACCode = _wtoi((*itr).c_str());
		itr++;
		channel.m_lMaxCount = channelForLB.m_lMaxCount = _wtoi((*itr).c_str());
		itr++;
		channel.m_lLimitCount = channelForLB.m_lLimitCount = _wtoi((*itr).c_str());
		itr++;
		string strCtg;
		GUtil::UTF16ToAnsi(*itr, strCtg);
		channel.m_sCategoryName = channelForLB.m_sCategoryName = strCtg;
		itr++;
		string strTitle = "";
		GUtil::UTF16ToAnsi(*itr, strTitle);
		channel.m_sTitle = channelForLB.m_sTitle = strTitle;
		itr++;
		// channel 등록이 변경되면서 .. 08/20
		channelForLB.m_lChannelType = _wtoi((*itr).c_str());
		itr++;
		channelForLB.m_lChannelOpenState = _wtoi((*itr).c_str());
		itr++;

		channel.m_nsapCHS = theChannelDir.GetCHSNsap();
		lstChannelInfo.push_back(channel);
		m_lstChannelInfoForLBReg.push_back(channelForLB);
		channel.Clear();
		channelForLB.Clear();
		if (itr == lstData.end())
			break;
		lChannelSize--;
	}

	delete[] sTempBuf;
	return TRUE;
}

void CChannelDir::Init()
{
}

LONG CChannelDir::CreateDefinedChannel()
{
	TLock lo(this);
	
	char buff[1024] = {0, };
	string sCHSIP;
	m_nsapCHS.GetIP(sCHSIP);
	//
	//	change Tabel name  : 2002/03/15 (channel --> info_cahnnel)
	//
	DBGW_String strResult;
	int nResult = 0;
	
	sprintf(buff, "AdminDB|Q|select SSN,CATEGORYNUM,GCID,ACCODE,MAXCOUNT,LIMITCOUNT,CATEGORYNAME,TITLE,CHANNELTYPE,OPENSTATE from info_channel where CHSIP = ?|%s", sCHSIP.c_str());
	//sprintf(buff, "AdminDB|Q|select SSN,CATEGORYNUM,GCID,ACCODE,MAXCOUNT,LIMITCOUNT,CATEGORYNAME,TITLE,CHANNELTYPE,OPENSTATE from info_channel where CHSIP = '10.22.242.131'");
	BOOL _bRet = ExecuteQuery(1, buff, &strResult, &nResult);
	if(!_bRet)
	{
		LOG(INF_UK, "CHS_CD_CreateDefinedChannel"_LK, "+++++ Fail Query from AdminDB, Can not get channel Info +++++");
		return 0;
	}
	
	string sBuf = strResult.GetData();	
	ChannelBaseInfoList lstChannelInfo;	
	_bRet = ParseChannelFromString(sBuf, lstChannelInfo, QUERY_DELIMETER);
	if(!_bRet)
	{
		LOG(INF_UK, "CHS_CD_CreateDefinedChannel"_LK, "+++++ Fail to parsing from db-query-result, Can not get channel Info +++++");
		return 0;
	}

	bool bIsTestChannel = false;

	// 실서버는 ChannelInfo가 항상 0보다 크다는 조건하에 아래 조건문을 비교한다. (if 문으로 진입하는 건 Test 할때만~)
	if (lstChannelInfo.size() <= 0)
	{
		LOG(ERR_UK, "CHS_CD_CreateDefinedChannel"_LK, "CHS_IP : ", sCHSIP.c_str());

		char sGetData[1024] = {0x00};
		::GetPrivateProfileStringA("CHS_TEST", "FLAG", "0" , sGetData, sizeof(sGetData)/sizeof(char), theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/);

		if (atoi(sGetData) == 1)
		{
			CTestChannelMgr testChannelMgr;

			if (!testChannelMgr.GetTestChannel(lstChannelInfo) )
			{
				LOG(INF_UK, "CHS_CD_CreateDefineChannel"_LK, "+++++ GetTestChannel() GetModuleNameA, fopen Function Call Error!!! +++++" );
				return 0;
			}

			bIsTestChannel = true;
		}
		else
		{
			LOG(INF_UK, "CHS_CD_CreateDefineChannel"_LK, "+++++ CHS.INI [CHS_TEST] [FLAG != 1] +++++" );
			return 0;
		}
	}

	if (bIsTestChannel) // Test Channel
		LOG(INF_UK, "CHS_CD_CreateDefinedChannel"_LK, "+++++++++++++++++ Test Channel +++++++++++++++++++++");
	else // AdminDB Access
		LOG(INF_UK, "CHS_CD_CreateDefinedChannel"_LK, "+++++++++++++++++ AdminDB Access Channel +++++++++++++++++++++");
		
	
	LONG channel_size = 0L;
	ForEachElmt(ChannelBaseInfoList, lstChannelInfo, im, jm)
	{
		ChannelBaseInfo & cinfo = *im;
		if(!CreateChannel(cinfo.m_channelID, cinfo, CHANTYPE_STATIC))
		{
			string logstr = format("+++++ Failed Create channel [%d:%d:%d]ChannelID ++++++", cinfo.m_channelID.m_lSSN, cinfo.m_channelID.m_dwCategory, cinfo.m_channelID.m_dwGCIID);
			LOG(ERR_UK, "CHS_CD_CreateDefinedChannel"_LK, logstr);
		}
		else
		{
			string logstr = format("+++++ [%d:%d:%d] ++++++", cinfo.m_channelID.m_lSSN, cinfo.m_channelID.m_dwCategory, cinfo.m_channelID.m_dwGCIID);
			LOG(ERR_UK, "CHS_CD_CreateDefinedChannel"_LK, logstr);
		}
		channel_size++;

		m_ssnTable[cinfo.m_channelID.m_lSSN] = "SSN";	/// 뒤에 스트링은 의미 없음.
	}
	LOG(INF_UK, "CHS_CChannelDir"_LK, "<================= Created Channel Size : ", channel_size, " =================>");

	Init();

	return channel_size;
}

BOOL CChannelDir::InitPreCreatedRoom()
{
	// 미리 만들어진 게임 방 관련된 SSN들을 처리 하기 위한 INI 파일 설정 작업 
	// 1. 일본 빠찡코 - 더 추가 될지도 모름(2006.03.09)
	char szIndex[256] = {0x00};
	char szSSN[512] = {0x00};
	DWORD dwRet = ::GetPrivateProfileStringA("PRE_ROOM", NULL, NULL , szIndex, sizeof(szIndex)/sizeof(char), theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/);
	DWORD dwRead = 0;
	
	char* pszIndex = szIndex;
	while(dwRead < dwRet)
	{
		GetPrivateProfileStringA("PRE_ROOM", pszIndex, "0" , szSSN, sizeof(szSSN)/sizeof(char), theControl.m_confPath.GetConfPath()/*CONFIG_FILENAME*/);
		
		LONG lAddSSN = atol(szSSN);
		// SSN을 리스트로 쫙 읽어온다.

		if (lAddSSN > 0)
		{
            m_mapPreRoom[lAddSSN] = false;
			LOG(INF_UK, "CHS_CD_InitPreCreatedRoom"_LK, "InitPreCreatedRoom ADD SSN : [", lAddSSN, "]" );
		}
		else
		{
			LOG(WAR_UK, "CHS_CD_InitPreCreatedRoom"_LK, "InitPreCreatedRoom SSN Ivalid : [", lAddSSN, "]" );
			return FALSE;
		}

		LONG lLength = strlen(pszIndex) + 1;
		dwRead += lLength;
		pszIndex += lLength;
	}

	return TRUE;
}

BOOL CChannelDir::SettingPreCreatedSSN()
{
	ForEachElmt(MapPreRoomSSN, m_mapPreRoom, it, ij)
	{
		LONG lSSN = (*it).first;
		SSNTable::iterator iter = m_ssnTable.find(lSSN);
		if (iter != m_ssnTable.end())
		{
			m_mapPreRoom[lSSN] = true;
			m_bSetPreCreateSSN = true;
		}
	}

	return TRUE;
}

BOOL CChannelDir::ParseChannel(ChannelBaseInfoList & lstChannelInfo, DB_QueryResult & channelInfo)
{
	ChannelBaseInfo channel;
	ChannelRegistInfo channelForLB;
//	int field_idx = 0;
	LONG item_size = channelInfo.m_QueryResultList.size();
//	if(item_size % MAX_FIELD_COUNT)	// 뭔가 하나 더 따라오는군..
//		return FALSE;
	LONG channel_size = item_size / MAX_FIELD_COUNT;


	QueryResultList::iterator itr = channelInfo.m_QueryResultList.begin();
	while(channel_size > 0)
	{
		channel.m_channelID.m_lSSN = channelForLB.m_channelID.m_lSSN = atoi((*itr).c_str());
		itr++;
		channel.m_channelID.m_dwCategory = channelForLB.m_channelID.m_dwCategory = atoi((*itr).c_str());
		itr++;
		channel.m_channelID.m_dwGCIID = channelForLB.m_channelID.m_dwGCIID = atoi((*itr).c_str());
		itr++;
		channel.m_lCACode = channelForLB.m_lACCode = atoi((*itr).c_str());
		itr++;
		channel.m_lMaxCount = channelForLB.m_lMaxCount = atoi((*itr).c_str());
		itr++;
		channel.m_lLimitCount = channelForLB.m_lLimitCount = atoi((*itr).c_str());
		itr++;
		channel.m_sCategoryName = channelForLB.m_sCategoryName = *itr;
		itr++;
		channel.m_sTitle = channelForLB.m_sTitle = *itr;
		itr++;
		// channel 등록이 변경되면서 .. 08/20
		channelForLB.m_lChannelType = atoi((*itr).c_str());
		itr++;
		channelForLB.m_lChannelOpenState = atoi((*itr).c_str());
		itr++;

		channel.m_nsapCHS = theChannelDir.GetCHSNsap();
		lstChannelInfo.push_back(channel);
		m_lstChannelInfoForLBReg.push_back(channelForLB);
		channel.Clear();
		channelForLB.Clear();
		if(itr == channelInfo.m_QueryResultList.end())
			break;
		channel_size--;
	}

	return TRUE;
}

CChannel* CChannelDir::CreateChannel(ChannelID channelID, const ChannelBaseInfo& binfo, long lChannelType)
{
//	TLock lo(this); //protected function으로 변경.
	
	CChannel * pChannel = NULL;
	{
		TChannelDir::iterator it = m_mapChannelDir.find(channelID);
		if(it == m_mapChannelDir.end()) 
		{
			pChannel = new CChannel(channelID, binfo, lChannelType);
			VALIDATE(pChannel);
			if(!pChannel)
				return NULL;
			BOOL bRet = pChannel->Run();
			VALIDATE(bRet);
			if(!bRet)
			{
				pChannel->Stop();
				pChannel->Release();
				return NULL;
			}
			m_mapChannelDir[channelID] = pChannel;
		}
	}

	return pChannel;
}

//void CChannelDir::SetLRBIP(TCHAR * pIP) 
//{
//	::ZeroMemory(m_cLRBIP, sizeof(m_cLRBIP)); 
//	memcpy(m_cLRBIP, pIP, strlen(pIP));
//}
//void CChannelDir::GetLRBIP(TCHAR * pIP) 
//{
//	memcpy(pIP, m_cLRBIP, strlen(m_cLRBIP));
//}

///////////////////////////////////////////////////////////////////////////////
// Game channel Instance Control

void CChannelDir::RemNGS(NSAP &nsap)	//neo1
{
	CChannelPtrTable tbl;
	{
		TLock lo(this);
		ForEachElmt(TChannelDir, m_mapChannelDir, it, jt)
		{
			CChannel* p = it->second;
			tbl.push_back(p);
		}
	}

	ForEachElmt(CChannelPtrTable, tbl, it, jt)
	{
		CChannelPtr& sp = *it;
		sp->PostRemNGS(nsap);
	}

}

void CChannelDir::ResetUserMsgCnt()
{
	TLock lo(this);
	for(TChannelDir::iterator itr = m_mapChannelDir.begin() ; itr != m_mapChannelDir.end() ; itr++)
	{
		CChannel* pChannel = itr->second;
		::XsigQueueSignal(::GetChannelThreadPool(), pChannel, CHANNEL_RESET_USERMSGCNT, (WPARAM)NULL, (LPARAM)NULL);
	}
}

BOOL CChannelDir::GetWaitRoomList(ChannelID cid, GetWaitRoomInfoList& lstRoom, unsigned int nGetCnt, LONG lType)
{
	if(nGetCnt > 100) return FALSE;
	lstRoom.clear();

	NFRoomInfoInChannelList lstGetRoom;

	unsigned int nGetRoomCnt = 0;
	for(TChannelDir::iterator itr = m_mapChannelDir.begin() ; itr != m_mapChannelDir.end() ; itr++) 
	{
		CChannel* pChannel = itr->second;
		if(pChannel->GetChannelID() != cid || pChannel->GetChannelID().m_lSSN != cid.m_lSSN || pChannel->GetChannelID().m_dwCategory != cid.m_dwCategory) 
			continue;

		pChannel->GetWaitRoomList(cid, lstGetRoom, nGetCnt, lType);

		if (lstGetRoom.size() > 0)
		{
			if(GetTickCount()%2 == 0)
				lstRoom.push_back(CGetWaitRoomInfo(pChannel->GetChannelID(), lstGetRoom));
			else
				lstRoom.push_front(CGetWaitRoomInfo(pChannel->GetChannelID(), lstGetRoom));
			
			if((nGetRoomCnt += lstGetRoom.size()) >= nGetCnt) 
				break;
			lstGetRoom.clear();
		}
	}
	return TRUE;
}

BOOL CChannelDir::OnChannelDListReq(LONG lSSN, MsgCHSNGS_ChannelIDList & msg)
{
	TLock lo(this);

	// INI파일에서 읽은 SSN으로 ChannelIDList를 검색하여 ChannelList 생성
	if (lSSN < 0)
	{
		ForEachElmt(MapPreRoomSSN, m_mapPreRoom, it, ij)
		{
			LONG lIniSSN = 0;
			bool bCheckSSN = (*it).second;
			
			if (bCheckSSN)
				lIniSSN = (*it).first;
			lSSN = lIniSSN;
			break;
		}
	}
	
	ForEachElmt(TChannelDir, m_mapChannelDir, it, ij)
	{
		ChannelID cid = (*it).first;
		if (cid.m_lSSN == lSSN)
		{
			msg.m_lstChannelID.push_back(cid);
		}
	}

	return TRUE;
}