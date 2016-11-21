// 
// Category.h
// 
/**********************************************************
		Service Infomation Table
									-ss
***********************************************************/

#ifndef _CATEGORY_H__
#define _CATEGORY_H__

#include <ADL/MsgAdminStruct.h>
#include <NF/ADL/MsgCHSNCS.h>
#include <atlbase.h>
#include <ADL/MsgAdminDB.h>

#include "GChannel.h"   // We need this to sort pset<GameChannel>.


class LRBConnector;

////////////////////////////////////////////////////////////////////////
// Category 

class CHSInfo;

// Compare channel IDs
struct GameChannelPtrComp : public binary_function<GameChannel*, GameChannel*, bool>
{
	bool operator()(const GameChannel* lgch, const GameChannel* rgch) const
	{
		return (lgch->GetGameChannelID() < rgch->GetGameChannelID());
	}
};

class Category 
{
	friend class ServiceTable;
public:
	Category();
	Category(LONG lSSN, DWORD dwCagetoryNum);
	Category(const ChannelPrefix& channelPrefix);
	~Category();

public:
	bool operator==(const Category& category) 
	{
		if(m_channelPrefix.m_dwCategory != category.m_channelPrefix.m_dwCategory)
			return FALSE;
		return TRUE;
	};

public:
	void Clear();
	BOOL AddGameChannel(GameChannel* pGameChannel);
	BOOL GetAllChannelList(ChannelInfoList& channelInfoList, LONG& lUserCount, LONG& lChannelCount);
	BOOL GetBalancedChannelList(ChannelInfoList& channelInfoList, LONG lRequiredChannelCount, LONG& lUserCount, LONG& lChannelCount); // 기존의 GetChannelList
	GameChannel* FindMinGameChannel(long& lErr);
	GameChannel* FindGameChannel(DWORD dwGCIID);

	BOOL GetChannelBaseInfo(ChannelBaseInfo& channelBaseInfo);
	BOOL GetAllChannelList(ChannelBaseInfoList& channelBaseInfoList, LONG& lUserCount, LONG& lChannelCount);

public:
	const ChannelPrefix& GetCategoryInfo() const { return m_channelPrefix; }
	ChannelPrefix& GetCategoryInfo() { return m_channelPrefix; }
public:
	void GetGCList(list<GameChannel*>& lstGC);
private:
	ChannelPrefix m_channelPrefix;
protected:
	typedef pset<GameChannel, GameChannelPtrComp> ChannelSet_T;
	ChannelSet_T m_set;
};
	
/////////////////////////////////////////////////////////////////////////
// Category Table
class CategoryTable
{
public:
	typedef pmap<DWORD, Category> TMap;
public:
	CategoryTable(long lSSN);
	~CategoryTable();
public:
	void Clear();
	BOOL AddCategory(Category* pCategory);	//수정할것.
	Category* FindCategory(DWORD dwCategoryNum);
	Category* FindCategory(ChannelPrefix& channelPrefix);
	BOOL GetAllCategory(ChannelBaseInfoList& lstChannelBaseInfo);
	BOOL GetAllCategory(ChannelInfoList& lstChannelInfo);

public:
	BOOL GetOpenState() { return m_bOpenState; }
	void SetOpenState(BOOL bOpenState) { m_bOpenState = bOpenState; }
private:
	long m_lSSN;
	TMap m_mapCategory;
	BOOL m_bOpenState;
	friend class ServiceTable;
};

///////////////////////////////////////////////////////////////////////
//Service Table
class ServiceTable 
{
	IMPLEMENT_TISAFE(ServiceTable)
public:
	typedef pmap<LONG, CategoryTable> TMap;
public:
	ServiceTable();
	~ServiceTable();
protected:
	CategoryTable* SearchCategoryTable(LONG ServiceNum);
	BOOL AddService(LONG lServiceNum, CategoryTable* pCategoryTable);
	void Clear();
public:
	BOOL Init();
	//DB 에 없었던 channel sverver가 추가 되었다. 따라서 service table update
private:
	BOOL AddChannelsInSvcTb(const NSAP& nsap, LRBAddress lrbAddr, ChannelRegInfoList lstChRegInfo);
public:
	void AllServiceInfo(AMS_CategoryList& rlstCategory);
	BOOL DeleteAllService();
	BOOL OpenService(long lSSN);
	BOOL CloseService(long lSSN);

	void CreateGameChannel(const ChannelInfo& info, long lType, BOOL bOpenState, BOOL IsNewChSvr = FALSE);

	BOOL OnGameChannelCreate(const ChannelID& channelID, long lPort);
	BOOL UpdateChannelInfoList(const ChannelUpdateInfoList& lst);

	BOOL OnGameChannelDestroy(const ChannelID& channelID);
	BOOL OpenChannel(const ChannelID& channelID);
	void OpenChannel(const NSAP& nsap);
	BOOL CloseChannel(const ChannelID& channelID);
	void CloseChannel(const NSAP& nsap);

	long RecommendGameChannel(const ChannelPrefix& prefix, ChannelInfo& cinfo);
	long VerifyGameChannel(const ChannelID& channelID, ChannelInfo& cinfo);

	BOOL GetAllCategoryList(ChannelBaseInfoList& listChannelBaseInfo, LONG lSSN);
	BOOL GetAllCategoryList(ChannelInfoList& listChannelInfo, LONG lSSN);

	long FindGameChannel(const ChannelID& channelID, ChannelInfo& cinfo);
	void GetChannelList(const ChannelID& channelID, ChannelInfoList& lst, LONG& lUserCount, LONG& lChannelCount);
	void EnableChannelInChSvr(const NSAP& nsap, LRBAddress lrbAddr, ChannelRegInfoList lstChRegInfo);
	BOOL GetParseData(string& sBuf, DB_ChannelInfoList& lstchannelInfo, char cDelimeter);
private:
	TMap m_mapService;
};

//////////////////////////////////////////////////////////////////////////
// Global Variable
extern ServiceTable	theServiceTable;

#endif //_CATEGORY_H__