#include "stdafx.h"

#include "BSManager.h"
#include "LRBHandler.h"

#include "ChannelDir.h"
#include "ODBGWManager.h"


CBSManager theBSManager;

//////////////////////////////////////////////////////////////////////////////////
// UTIL 함수.
TCUID RoomID2TCUID(RoomID roomID)
{
	TCUID cuid;
	cuid.m_id1 = roomID.m_lSSN;
	cuid.m_id2 = roomID.m_dwCategory;
	cuid.m_id3 = roomID.m_dwGCIID;
	cuid.m_id4 = roomID.m_dwGRIID;

	return cuid;
}
RoomID TCUID2RoomID(TCUID cuid)
{
	RoomID roomID;
	roomID.m_lSSN		=	cuid.m_id1;
	roomID.m_dwCategory	=	cuid.m_id2;
	roomID.m_dwGCIID	=	cuid.m_id3;
	roomID.m_dwGRIID	=	cuid.m_id4;

	return roomID;
}

//////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(void) CBSManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)		
{
}

CBSManager::CBSManager()
{
	m_aryAddrBMS = NULL;
	m_numBMS = 0;
	m_dwRefCnt = 0;
}
CBSManager::~CBSManager()
{
	if (m_aryAddrBMS != NULL)
		delete [] m_aryAddrBMS;
}


//////////////////////////////////////////////////////////////////////////////////

void CBSManager::SendRegisterBMSInfoReqToBMS()
{
	MsgCHSBMS_RegisterBMSReq		req;

	PayloadCHSBS	pld(PayloadCHSBS::msgRegisterBMSReq_Tag, req);


	GBuf buf;
	BOOL bRet = BStore(buf, pld);
	if (!bRet)
	{
		return ;
	}
	
	LRBAddress addrMBMS;
	addrMBMS.SetAddress("MBMS");

	theLRBHandler.ConnectorSendTo(buf, PROTOCOL_LDTP, theLRBHandler.GetMyAddress(), addrMBMS);	
}

//////////////////////////////////////////////////////////////////////////////////
void CBSManager::OnRcvRegisterAnsFromBMS(const LRBAddress &addr, MsgBMSCHS_RegisterBMSAns *pAns)
{
	if (pAns->m_lErrorCode != BE_SUCCESS)
	{
		LOG(INF_UK, "CHS_CBSManager, RegisterAns Error!!. pAns->m_lErrorCode:", pAns->m_lErrorCode);
		return;
	}

	if (pAns->m_lBMSID <= 0 ||  100 < pAns->m_lBMSTotalCount || pAns->m_lBMSTotalCount < pAns->m_lBMSID)
	{
		LOG(ERR_UK, "CHS_CBSManager, RegisterAns Error!!. pAns->lBMSID:", pAns->m_lBMSID, ", pAns->lBMSTotalCount:", pAns->m_lBMSTotalCount);
		return;
	}

	{//----------------------------------------------------------	
		TLock lo(this);

		if (m_numBMS == 0)
		{
			m_aryAddrBMS = new LRBAddress[pAns->m_lBMSTotalCount];
			m_numBMS = pAns->m_lBMSTotalCount;

			LOG(ERR_UK, "CHS_CBSManager, OnRcvRegisterAnsFromBMS // m_numBMS : ", m_numBMS, "Total CNT : ", pAns->m_lBMSTotalCount);
		}
		else
		{
			if (m_numBMS != pAns->m_lBMSTotalCount)
			{
				LOG(ERR_UK, "CHS_CBSManager, RegisterAns Error!!. m_numBMS:", m_numBMS, ", pAns->lBMSTotalCount:", pAns->m_lBMSTotalCount);
				return;
			}
		}

		m_aryAddrBMS[pAns->m_lBMSID - 1] = addr;
	}//----------------------------------------------------------	
}

void CBSManager::OnRcvRegisterNtfFromBMS(const LRBAddress &addr, MsgBMSCHS_RegisterBMSNtf *pNtf)
{
	if (pNtf->m_lBMSID <= 0 ||  100 < pNtf->m_lBMSTotalCount || pNtf->m_lBMSTotalCount < pNtf->m_lBMSID)
	{
		LOG(ERR_UK, "CHS_CBSManager, RegisterBMSNtf Error!!. pNtf->lBMSID:", pNtf->m_lBMSID, ", pNtf->lBMSTotalCount:", pNtf->m_lBMSTotalCount);
		return;
	}

	{//----------------------------------------------------------	
		TLock lo(this);

		if (m_numBMS == 0)
		{
			m_aryAddrBMS = new LRBAddress[pNtf->m_lBMSTotalCount];
			m_numBMS = pNtf->m_lBMSTotalCount;

			LOG(ERR_UK, "CHS_CBSManager, OnRcvRegisterNtfFromBMS // m_numBMS : ", m_numBMS, "Total CNT : ", pNtf->m_lBMSTotalCount);
		}
		else
		{
			if (m_numBMS != pNtf->m_lBMSTotalCount)
			{
				LOG(ERR_UK, "CHS_CBSManager, RegisterBMSNtf Error!!. m_numBMS:", m_numBMS, ", pNtf->lBMSTotalCount:", pNtf->m_lBMSTotalCount);
				return;
			}
		}

		m_aryAddrBMS[pNtf->m_lBMSID - 1] = addr;
	}//----------------------------------------------------------
}


//////////////////////////////////////////////////////////////////////////////////

//BOOL CBSManager::OnRcvStartPeepReqFromCli(RoomID &roomID, LONG lUSN, ChannelID& CID, LONG lMSN)
BOOL CBSManager::OnRcvStartPeepReqFromCli(RoomID &roomID, LONG lUSN, ChannelID& CID, LONG lSSN, BOOL bIsVIPRoom)
{
	if (m_numBMS == 0) // Need Lock? No. Locking Don't be needed.
	{
		LOG(WAR_UK, "CHS_BSManager, OnRcvStartPeepReqFromCli Error!!. No BMS Regitered ");
		return FALSE; // 아웃 패러미터로 에러코드를 리턴할 수 있도록 하자.
	}

	MsgCHSBMS_StartPeepReq		req;		
	req.m_strCliInfo = ::format("%d|%u|%u", CID.m_lSSN, CID.m_dwCategory, CID.m_dwGCIID);
	req.m_CUID			= RoomID2TCUID(roomID);
	req.m_lAudienceUSN	= lUSN;


	BOOL bFindRoomLRBAddr;
	//if (lMSN >= 0)
	if (bIsVIPRoom)
	{
		// VIP 룸인 경우 테이블에서 gls 의 LRB Address 를 찾아온다.
		//bFindRoomLRBAddr = theODBGWMgr.GetRoomLRBAddr(lMSN, roomID, req.m_addrGLS);
		bFindRoomLRBAddr = theODBGWMgr.GetRoomLRBAddr(lSSN, roomID, req.m_addrGLS);
	}
	else // 일반 룸인 경우 RoomID 로 GLS 주소를 찾아온다.
		bFindRoomLRBAddr = GetRoomLRBAddr(roomID, req.m_addrGLS);

	if (bFindRoomLRBAddr == FALSE)
	{
		string sid;
		roomID.GetInstanceID(sid);

		if (bIsVIPRoom)
			sid += " (VIP Room)";
		else
			sid += "";

//		sid +=  (lMSN >= 0) ? " (VIP Room)" : "" ; 

		LOG(WAR_UK, "CHS_BSManager, OnRcvStartPeepReqFromCli Error!!. Not Founded RoomID:",sid );
		return FALSE;
	}

	PayloadCHSBS pld(PayloadCHSBS::msgStartPeepReq_Tag, req);
	GBuf buf;
	BOOL bRet = BStore(buf, pld);
	if (!bRet)
	{
		LOG(WAR_UK, "CHS_BCSManager, OnRcvStartPeepReqFromCli Error!!. BStore Failed!! ");
		return FALSE;
	}

	LRBAddress addrBMS;
	int index = 0;
	{//----------------------------------------------------------
		TLock lo(this);
		index = roomID.m_dwGRIID%m_numBMS;
		addrBMS = m_aryAddrBMS[index];
	}//----------------------------------------------------------

	// 보낼려고 하는 BMS의 주소가 NULL이면,
	if (addrBMS.IsNULL())
	{
		LOG(WAR_UK, "BMS Address is NULL... index :", index);
		return FALSE;
	}

	theLRBHandler.SendToBMS(addrBMS, buf);	
	theLog.Put(DEV_UK, "Send StartPeepReq For", roomID.m_lSSN, "-", roomID.m_dwCategory, "-", roomID.m_dwGCIID, "-", roomID.m_dwGRIID, " To", addrBMS.GetString(), ", BufSize:", buf.GetLength());
	return TRUE;
}


void CBSManager::OnRcvStartPeepAnsFromBCS(MsgBCSCHS_StartPeepAns *pAns, ChannelID &cid, LONG &lUSN, MsgCHSCli_StartPeepAns* pToCli)
{
	lUSN = pAns->m_lAudienceUSN;
	::sscanf(pAns->m_strCliInfo.c_str(), "%d|%u|%u", &cid.m_lSSN, &cid.m_dwCategory, &cid.m_dwGCIID);

	pToCli->m_lErrCode = 0x00;// 에러처리하시오.
	pToCli->m_lAuthKey = pAns->m_lAuthKey;
	pToCli->m_nsapBCS  = pAns->m_nsapBCS;
	pToCli->m_roomID   = ::TCUID2RoomID(pAns->m_CUID);

}

//////////////////////////////////////////////////////////////////////////////////
void CBSManager::AddRoomLRBAddr(const RoomID & roomID, const LRBAddress &addr)
{
	AutoLock lo(&m_csRoomAddr);
	m_mapRoomAddr[roomID] = addr;
}
void CBSManager::RemoveRoomLRBAddr(const RoomID & roomID)
{
	AutoLock lo(&m_csRoomAddr);
	m_mapRoomAddr.erase(roomID);
}
BOOL CBSManager::GetRoomLRBAddr(const RoomID & roomID,LRBAddress &addr)
{
	AutoLock lo(&m_csRoomAddr);
	TRoomIDAddrMap::iterator i = m_mapRoomAddr.find(roomID);
	if (i == m_mapRoomAddr.end())
		return FALSE;

	addr = i->second;
	return TRUE;
}

void CBSManager::DeleteBMSAddr(const LRBAddress& addrBMS)
{
	TLock lo(this);
	
	for(int i=0; i<m_numBMS; i++)
	{
		LRBAddress		delBMSAddr;
		delBMSAddr = m_aryAddrBMS[i];

		if (delBMSAddr == addrBMS)
		{
			m_aryAddrBMS[i].Clear();
			break;
		}
	}
}
