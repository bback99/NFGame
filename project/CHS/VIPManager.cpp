#include "stdafx.h"

#ifdef _BUTTERFLY_APPLY_

#include "ADL/MsgCHSCli_BF.h"
#include "VIPManager.h"
#include "LRBHandler.h"
#include "Control.h"

CVIPManager theVIPMgr;


#define ODBGWQUERY_VIPROOMLIST	0x01


void CVIPManager::OnLRBRegister(const LRBAddress &addr)
{
	LONG lMSN;
	sscanf(addr.GetString().c_str(), "MCHSVIP%d", &lMSN);

	string strQueryID = ::format("SELECT_VIP_ROOM_INFO_%d", lMSN);
	
	int nQueryID = theControl.GetQueryIndex(strQueryID);
	if (nQueryID < 0)
		return;
	

	//AODBVIPMSN 로 쿼리를 전송한다.
	MsgCliODBGW_ExecuteBindQueryReq msg;
	msg.m_needAnswer	= TRUE;
	msg.m_bufKey		= ::format("%u|%u", (LONG)ODBGWQUERY_VIPROOMLIST,(LONG)lMSN);
	msg.m_nQueryID		= nQueryID;
	msg.m_vecParams.clear();

	PayloadCliODBGW pld(PayloadCliODBGW::msgBindReq_Tag, msg);
	string strODBGW = ::format("AODBVIP%d",lMSN);
	LRBAddress addrAODBGW;
	addrAODBGW.SetAddress(strODBGW.c_str());

	GBuf buf;	
	if (pld.BStore(buf) == FALSE)
	{
		LOG(INF_UK, "CHS_BStoreError,CVIPManager::OnLRBRegister(). addrAODBGW=", strODBGW);
		 return;
	}

	theLRBHandler.SendToODBGW(addrAODBGW, buf);

	return;
}
void CVIPManager::OnQueryAns(const LRBAddress &src, const LRBAddress &dst, MsgODBGWCli_QueryAns *pAns)
{
	if (pAns->m_lErrCode != 0) // 에러 로그 추가하시오.
		return ;

	// Type, MSN 을 알아낸다.
	LONG lType = -1, lMSN = -1;
	::sscanf(pAns->m_bufKey.c_str(), "%d|%d", &lType, &lMSN);
	
	switch(lType)
	{
	case ODBGWQUERY_VIPROOMLIST:
		{	
			UpdateVIPList(lMSN, pAns->m_vecResult);			
		}
		break;
	default: // 에러로그 남기시오.	
		break;
	}

	return;
}
void CVIPManager::OnChangeNtf(const LRBAddress &src, const LRBAddress &dst, MsgODBGWCli_ChangedNtf *pNtf)
{
	switch (pNtf->m_NotiID)
	{
	case ODBGWNOTI_VIPLIST:
		{			
			// MSN을 알아낸다.
			LONG lMSN;
			//sscanf(src.GetString().c_str(), "AODBVIP%d", &lMSN);
			sscanf(dst.GetString().c_str(), "MCHSVIP%d", &lMSN);
			theLog.Put(DEV_UK, "ButterFlyDebug. CVIPManager::OnChangeNtf(), src=", src.GetString(), ", dst=", dst.GetString());

			UpdateVIPList(lMSN, pNtf->m_vecResult);
			return;
		}
		break;
	default:	// 에러 로그 남기시오.
		break;
	}	
}

/**
mysql> desc VIP_ROOM_INFO_2
+---------------+---------------------+------+-----+---------+-------+
| Field         | Type                | Null | Key | Default | Extra |
+---------------+---------------------+------+-----+---------+-------+
| ROOMID        | bigint(20) unsigned |      | PRI | 0       |       |
| GLSIP         | varchar(15)         |      |     | 0       |       |
| CHSIP         | varchar(15)         |      |     | 0       |       |
| GLSLRBADDRESS | varchar(12)         |      | MUL |         |       |
| REGPASSWD     | char(1)             |      |     |         |       |
| ENTRANCEMONEY | bigint(20) unsigned |      | MUL | 0       |       |
| PPINGMONEY    | bigint(20) unsigned |      |     | 0       |       |
| CURUSERCOUNT  | tinyint(3) unsigned |      |     | 0       |       |
| ROOMTITLE     | varchar(46)         |      |     |         |       |
+---------------+---------------------+------+-----+---------+-------+
*/

//SELECT ROOMID,GLSIP,CHSIP,GLSPORT,CHSPORT,GLSLRBADDRESS,CHSLRBADDRESS,ENTRANCEMONEY,ROOMTITLE from VIP_ROOM_INFO_MSN

RoomID Str64ToRoomID(string str) // DB 에 64비트만 들어간다고 가정하고 일단 이렇게.
{	
	INT64 rid64;
	rid64 = _atoi64(str.c_str());

	RoomID rid;

	rid.m_dwGRIID = (DWORD)(rid64 & 0x00000000FFFFFFFF);
	rid64 >>= 32;
	rid.m_dwGCIID = (DWORD)(rid64 & 0x00000000FFFFFFFF);

	rid.m_lSSN = rid.m_dwGCIID/1000000;
	rid.m_dwCategory = (rid.m_dwGCIID - rid.m_lSSN*1000000)/1000;

	return rid;
}

void CVIPManager::UpdateVIPList(LONG lMSN, vector<string> &queryAns)
{
	TLock lo(this);
	VIPRoomList *pVIPList = NULL;
	VIPMap::iterator i = m_mapVIP.find(lMSN);

	if (i == m_mapVIP.end())
	{
		pVIPList = new VIPRoomList();
		m_mapVIP[lMSN] =  pVIPList;
	}
	else
		pVIPList = i->second;
	
	pVIPList->m_sizeList = queryAns.size() / 9;
	theLog.Put(DEV_UK, "ButterFlyDebug, Received VIPRoomListUpdateMsg. RoomNum=", pVIPList->m_sizeList);
	{
		pVIPList->m_vecRID.clear();
		pVIPList->m_vecGLSIP.clear();
		pVIPList->m_vecCHSIP.clear();
		pVIPList->m_vecGLSPort.clear();
		pVIPList->m_vecCHSPort.clear();
		pVIPList->m_vecGLSAddr.clear();
		pVIPList->m_vecCHSAddr.clear();
		pVIPList->m_vecEnterMoney.clear();
		pVIPList->m_vecTitle.clear();
	}

	for (int i = 0; i < pVIPList->m_sizeList ; i++)
	{
		{// For Debug Log		
			string strRoomID;
			Str64ToRoomID(queryAns[i*9 + 0].c_str()).GetInstanceID(strRoomID);
			theLog.Put(DEV_UK, "VIPRoom[",i+1,"] :", queryAns[i*9 + 0], "RoomID:", strRoomID);
		}
		pVIPList->m_vecRID.push_back(queryAns[i*9 + 0]);
		pVIPList->m_vecGLSIP.push_back(queryAns[i*9 + 1]);
		pVIPList->m_vecCHSIP.push_back(queryAns[i*9 + 2]);	
		pVIPList->m_vecGLSPort.push_back(queryAns[i*9 + 3]);
		pVIPList->m_vecCHSPort.push_back(queryAns[i*9 + 4]);
		pVIPList->m_vecGLSAddr.push_back(queryAns[i*9 + 5]);
		pVIPList->m_vecCHSAddr.push_back(queryAns[i*9 + 6]);
		pVIPList->m_vecEnterMoney.push_back(queryAns[i*9 + 7]);
		pVIPList->m_vecTitle.push_back(queryAns[i*9 + 8]);
	}
	return;
}



void CVIPManager::GetVIPRoomList(LONG lSSN, MsgCHSCli_VIPRoomListAns &ans)
{

	ans.m_lSSN = lSSN;
	VIPRoomList *pVIPList = NULL;

	ans.m_lErrCode = 0x00;

	TLock lo(this);

	LONG lMSN = theControl.SSN2MSN(lSSN);
	VIPMap::iterator i = m_mapVIP.find(lMSN);

	if (i != m_mapVIP.end())
		pVIPList = i->second;

	if (pVIPList == NULL || pVIPList->m_sizeList == 0)
	{
		 ans.m_lRoomCount = 0x00;
		return;
	}
	
	ans.m_lRoomCount = pVIPList->m_sizeList;
	for (int i = 0 ; i < pVIPList->m_sizeList; i++)
	{		
		ans.m_vecRID.push_back(Str64ToRoomID(pVIPList->m_vecRID[i]));

		NSAP nsap;
		nsap.m_dwIP			= ::inet_addr(pVIPList->m_vecCHSIP[i].c_str());
		nsap.m_dwPort		= atoi(pVIPList->m_vecCHSPort[i].c_str());
		ans.m_vecNsapCHS.push_back(nsap);

		nsap.m_dwIP			= ::inet_addr(pVIPList->m_vecGLSIP[i].c_str());
		nsap.m_dwPort		= atoi(pVIPList->m_vecGLSPort[i].c_str());
		ans.m_vecNsapGLS.push_back(nsap);
		
		ans.m_vecTitle.push_back(pVIPList->m_vecTitle[i]);
		
		string str;
		str = pVIPList->m_vecEnterMoney[i];
		ans.m_vecOtherInfo.push_back(str);
	}
	return;
}



string RoomIDToStr64(RoomID &rid)
{
	INT64 rid64;
	rid64 = rid.m_dwGCIID;
	rid64 <<=32;
	rid64 += rid.m_dwGRIID;

	char szRid[30] = "";
	_i64toa(rid64, szRid, 10);

	string str(szRid);
	return str;
}

BOOL CVIPManager::GetRoomLRBAddr(LONG lMSN, RoomID &rid, LRBAddress &addrGLS)
{
	VIPRoomList *pVIPList = NULL;

	TLock lo(this);
	VIPMap::iterator i = m_mapVIP.find(lMSN);

	if (i != m_mapVIP.end())
		pVIPList = i->second;

	if (pVIPList == NULL || pVIPList->m_sizeList == 0)
		return FALSE;

	string str64Rid = RoomIDToStr64(rid);
	for (int i = 0 ; i < pVIPList->m_sizeList; i++)
	{
		if (pVIPList->m_vecRID[i].compare(str64Rid))
		{
			addrGLS.SetAddress(pVIPList->m_vecGLSAddr[i].c_str());
			return TRUE;
		}
	}
	return FALSE;
}

STDMETHODIMP_(void) CVIPManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)	
{
	return;
}

#endif // _BUTTERFLY_APPLY_