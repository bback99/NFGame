// 
// ChannelSvrTable.h
// 

/**********************************************************
		Channel Server Infomation Table
									-ss
***********************************************************/


#ifndef _CHANNELSVRTABLE_H__
#define _CHANNELSVRTABLE_H__

#include <NF/ADL/MsgCHSNCS.h>
//#include <ADL/MsgAMSLB.h>


class ChannelSvrInfo : public ServerRegistration
{
public:
	typedef ServerRegistration TBase;
public:
	ChannelSvrInfo();
	ChannelSvrInfo(NSAP& nsap, LRBAddress lrbSvrAddr);
	virtual ~ChannelSvrInfo();
public:
	virtual void Clear();

public:
	void AddChannelInfo(const ChannelInfo& cinfo);
	void UpdateChannelList(ChannelUpdateInfoList& lstChannelInfo);
	BOOL GetChannelList(ChannelInfoList& channelInfoList);

	void CloseCHS();
	void OpenCHS();
	BOOL GetChsOpenState() { return m_bIsChsOpen; }

	friend class ChannelSvrTable;
private:
	ChannelInfoList m_lstChannelInfo;
private:
	BOOL m_bIsChsOpen;

	//CGS인지 구분하기 위해서.
	LONG m_lSvrCatType;
};

//lID is LogicAddress
class ChannelSvrTable 
{
	IMPLEMENT_TISAFE(ChannelSvrTable)
public:
	typedef pmap<LRBAddress, ChannelSvrInfo> TMap;

public:
	ChannelSvrTable();
	virtual ~ChannelSvrTable();
public:
	void Clear();
public:
	BOOL AddChannelSvr(LRBAddress lrbAddr, NSAP& nsap, LONG lSvrCatType = SVCCAT_CHS);
	BOOL DeleteChannelSvr(LRBAddress lrbAddr);
	BOOL UpdateChannelSvr(LRBAddress lrbAddr, ChannelUpdateInfoList& lstChannelInfo);
public:
	BOOL CloseChannelServer(LRBAddress lrbAddr);
	BOOL CloseChannelServer(LRBAddress lrbAddr, NSAP& nsap);
	BOOL StartChannelServer(LRBAddress lrbAddr, NSAP& nsap);

	BOOL GetGameChannelList(LRBAddress lrbAddr, ChannelInfoList& lst);
	BOOL AddChannelInfoInChannelSvr(LRBAddress lrbAddr, const ChannelInfo& ChInfo);

	LONG AvailableChannelSvrCount();
private:
	TMap m_channelsvrTable;
};

extern ChannelSvrTable theChannelSvrTable;

#endif 