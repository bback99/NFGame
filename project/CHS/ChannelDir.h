//
// ChannelDir.h
//

#ifndef CHANNELDIR_H
#define CHANNELDIR_H

#include "Channel.h"
#include "ADL/MsgAdminDB.h"

#include "Control.h"

//////////////////////////////////////////////////////
extern BOOL GetParsingData(char* strParsing, vector<long>* vecData);
/////////////////////////////////////////////////////////////////////////////////// 
// CChannelDir

class CChannelDir
{
	IMPLEMENT_TISAFE(CChannelDir)
protected:
	typedef pmap<ChannelID, CChannel> TChannelDir;
	TChannelDir m_mapChannelDir;

	typedef map<LRBAddress, NSAP> TGLSTable;
	TGLSTable m_mapGLS;

public:
	CChannelDir() : m_lCSNTop(0), m_bSetPreCreateSSN(0) { m_ssnTable.clear(); } 
	~CChannelDir() {};

protected: 
	//void AMSAnnounceAgent(CChannel* pChannel, const GBuf& buf);
	void PMSAnnounceAgent(CChannel* pChannel, const GBuf& buf);
	CChannel* CreateChannel(ChannelID channelID, const ChannelBaseInfo& binfo, long lChannelType);
	BOOL ParseChannel(ChannelBaseInfoList & lstChannelInfo, DB_QueryResult & channelInfo);
	BOOL ParseChannelFromString(string sBuf, ChannelBaseInfoList& lstChannelInfo, char cDelimeter);
	BOOL ParseChannelFromStringW(wstring sBuf, ChannelBaseInfoList& lstChannelInfo, WCHAR cDelimeter);
	void Init();

public:
	void AddGLSLogicalAddr(const LRBAddress & lrbAddr, const NSAP & nsap, BOOL bMode = FALSE);
	void RemGLSLogicalAddr(LRBAddress & lrbAddr);
	BOOL GetGLSNsap(LRBAddress & lrbAddr, NSAP & nsap);

	LONG GetChannelSize() {return m_mapChannelDir.size();}
	void RegisterAllInstance();
	void DeregisterAllInstance();
//	void SetLRBIP(TCHAR * pIP) ;
//	void GetLRBIP(TCHAR * pIP) ;
	void SetCHSNsap(NSAP & nsap) {m_nsapCHS = nsap;}
	NSAP& GetCHSNsap() {return m_nsapCHS;}
	const NSAP& GetCHSNsap() const {return m_nsapCHS;}

	LONG CreateDefinedChannel();
	LONG CreateDefinedChannelW();
	BOOL InitPreCreatedRoom();
	BOOL SettingPreCreatedSSN();
	BOOL GetChannel(ChannelID channelID, CChannel** ppChannel);

	//////// PMS announce msg send.
	void PMSAnnounceReqInSystem(xstring & sMsg);
	void PMSAnnounceReqInCategory(long lSSN, DWORD lCat, xstring & sMsg);
	void PMSAnnounceReqInSSN(long lSSN, xstring & sMsg);
	void ResetUserMsgCnt();
	BOOL GetWaitRoomList(ChannelID cid, GetWaitRoomInfoList& lstRoom, unsigned int nGetCnt = 10, LONG lType = 1);

	// Channel Instance control..
	void RemNGS(NSAP &nsap);

	void GetChannelCategoryDataList();
	BOOL OnChannelDListReq(LONG lSSN, MsgCHSNGS_ChannelIDList & msg);

public:
	ChannelRegInfoList m_lstChannelInfoForLBReg;

	typedef map<long, string>	SSNTable;
	SSNTable			m_ssnTable;		/// CHS 서버에 있는 게임 종류...

	typedef map<long, bool>		MapPreRoomSSN;
	MapPreRoomSSN		m_mapPreRoom;

	typedef vector<long>		VecCHSGLSConSSN;
	VecCHSGLSConSSN		m_vecCHSGLSConSSN;

	bool	m_bSetPreCreateSSN;
protected: 
	long m_lCSNTop;
	NSAP m_nsapCHS;
	TCHAR m_cLRBIP[30];
};

extern CChannelDir theChannelDir;

#endif //!CHANNELDIR_H
