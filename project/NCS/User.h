

#ifndef USER_H
#define USER_H



#include "Link.h"
#include <NF/ADL/MsgNCSCli.h>
#include <NFVariant/NFGameData.h>
#include <NLSManager.h>

// enum
enum USER_STATE_INDEX {
	US_INVALID,
	US_ALIVE,
	US_NEEDCHECK,
	US_CHECK,
	US_DEAD,
	US_MAX
};

///////////////////////////////////////////////////////////////////////////////////
// CNFUser
class CNFUser
{
public:
	CNFUser();
	virtual ~CNFUser() { }

public:
	// Only NF
	NFUser& GetNFUserInfo() { return m_NFUser; }

private:
	NFUser		m_NFUser;
};


struct TGameData {
	LONG m_lSize;
	MsgCliNCS_GameData m_GameData;
};


///////////////////////////////////////////////////////////////////////////////////
// CUser

class CUser : public INLSObject
{
	IMPLEMENT_TISAFE(CUser)

public:
	typedef PayloadCliNCS	TMsg;
	typedef list<TMsg>		TMsgQ;
	typedef list<TGameData> GameDataQ;
public:
	CUser(const LONG lUSN, const LONG lCSN, const string& strSiteCode, const string& strSiteUserID, const string& strPWD, const LONG lAdminLEV);
	virtual ~CUser();

public:
	// for NLS
	STDMETHOD_(void, NLSGetKeyValue)(TKey& key);
	STDMETHOD_(BOOL, NLSGetRoomID) (RoomID & roomID);
	STDMETHOD_(void, NLSSetErrorCode)(LONG lECode);

public:
	CLink* GetLink() { return m_pLink; }
	void SetLink(CLink* pLink) { m_pLink = pLink; }

	long GetUSN() const { return m_nfUserBaseInfo.m_lUSN; }
	long GetGSN() const { return m_nfUserBaseInfo.m_lGSN; }
	long GetCSN() const { return m_nfUserBaseInfo.m_lLastPlayCSN; }

	void SetTutorialCSN(LONG lTutorialCSN) { m_lTutorialCSN = lTutorialCSN; }
	LONG GetTutorialCSN() const { return m_lTutorialCSN; }

	void SetCSN(LONG lCSN) { m_nfUserBaseInfo.m_lLastPlayCSN = lCSN; }
	void SetUserID(xstring sID) { m_nfUserBaseInfo.m_sUID.assign(sID.c_str(), sID.length()); }
	xstring& GetUserID() { return m_nfUserBaseInfo.m_sUID; }

	const LRBAddress& GetCHSAddr() { return m_CHSAddr; }
	void SetCHSAddr(const LRBAddress& addr) { m_CHSAddr = addr; }	

	NFRoomOption& GetRoomOption() { return m_roomOption; }
	const NFRoomOption& GetRoomOption() const { return m_roomOption; }
	const xstring& GetPassword() const { return m_roomOption.m_sPassword; }
	void SetPassword(xstring& pwd) { m_roomOption.m_sPassword.assign(pwd.c_str(), pwd.length()); }
	void UpdateUserGameData(const string& sUserData);
	void UpdateUserGameData(LPCSTR szUserData, DWORD dwSize);
	void UpdateUserInfo(BOOL bFromString);
	void SetUserState(LONG lState);
	void SetErrorCode(long lErrorCode) { m_lErrorCode = lErrorCode; }
	long GetErrorCode() const { return m_lErrorCode; }

	void UnpopMsg();
	void UnpopMsg(const TMsg& msg);
	void PushMsg(const TMsg& msg);
	BOOL PopMsg(TMsg& msg);

	void PushGameData(MsgCliNCS_GameData& data);
	BOOL GetGameData(LPLONG plIndex, LPSTR szData, BOOL bErase = TRUE);
	DWORD GetGameData(LPLONG plIndex);

	// NF External Function
	TMapNFCharInfoExt& GetTMapNFCharInfoExt() { return m_mapCharInfoExt; }
	NFUserBaseInfo& GetNFUserBaseInfo() { return m_nfUserBaseInfo; }
	BOOL GetNFCharInfoExt(NFCharInfoExt& nfCharInfo)
	{
		TMapNFCharInfoExt::iterator it = m_mapCharInfoExt.find(GetCSN());
		if (it == m_mapCharInfoExt.end())
			return FALSE;
		nfCharInfo = (*it).second;
		return TRUE;
	}
	NFCharInfoExt* GetNFCharInfoExt()
	{ 
		return GetNFCharInfoExt(GetCSN());
	}
	NFCharInfoExt* GetNFCharInfoExt(LONG lCSN)
	{ 
		TMapNFCharInfoExt::iterator it = m_mapCharInfoExt.find(lCSN);
		if (it == m_mapCharInfoExt.end())
			return NULL;
		return &((*it).second);
	}
	NFCharBaseInfo* GetNFCharBaseInfo()
	{ 
		return GetNFCharBaseInfo(GetCSN());
	}
	BOOL GetNFCharBaseInfo(NFCharBaseInfo& nfCharInfo)
	{
		TMapNFCharInfoExt::iterator it = m_mapCharInfoExt.find(GetCSN());
		if (it == m_mapCharInfoExt.end())
			return FALSE;
		nfCharInfo = (*it).second.m_nfCharBaseInfo;
		return TRUE;
	}	
	NFCharBaseInfo* GetNFCharBaseInfo(LONG lCSN)
	{ 
		TMapNFCharInfoExt::iterator it = m_mapCharInfoExt.find(lCSN);
		if (it == m_mapCharInfoExt.end())
			return NULL;
		return &((*it).second.m_nfCharBaseInfo);
	}
	LONG GetLevel()
	{
		NFCharBaseInfo* baseInfo = GetNFCharBaseInfo(GetCSN());
		if (NULL != baseInfo)
            return baseInfo->m_lLevel;
		return 0;
	}
	std::string& GetCharName()
	{
		NFCharBaseInfo* baseInfo = GetNFCharBaseInfo(GetCSN());
		if (NULL == baseInfo)
		{
			baseInfo = new NFCharBaseInfo;
			baseInfo->Clear();
		}
		return baseInfo->m_strCharName;
	}
	NFCharExteriorInfo& GetExteriorInfo()
	{
		NFCharInfoExt* charInfo = GetNFCharInfoExt(GetCSN());
		if (NULL == charInfo)
		{
			charInfo = new NFCharInfoExt;
			charInfo->Clear();
		}
		return charInfo->m_nfCharExteriorInfo;
	}
	ArcVectorT< LONG >& GetQuickSlot()
	{
		NFCharInfoExt* charInfo = GetNFCharInfoExt(GetCSN());
		if (NULL == charInfo)
		{
			charInfo = new NFCharInfoExt;
			charInfo->Clear();
		}
		return charInfo->m_nfQuickSlot;
	}
	BOOL FindNFCharInfoExt(LONG lCSN, NFCharInfoExt& nfCharInfoExt);
	void AddNFCharInfoExt(LONG lCSN, const NFCharInfoExt& nfCharInfoExt);

	CONT_NF_FRIEND_MGR& GetContNFFriendMgr()		{ return m_kContNFFriendMgr; }
	CONT_NF_FRIEND&		GetContNFFriend(LONG lCSN)	{ return m_kContNFFriendMgr[lCSN]; }
	BOOL FindNFFriend(LONG lCSN, CONT_NF_FRIEND& rkContNFFriend);
	void SetNFFriend(LONG lCSN, const CONT_NF_FRIEND& rkContNFFriend);
	void AddNFFriend(const LONG lCSN, const TKey& rAddKey, const CNFFriend& rAddFriend);
	void DeleteNFFriend(const LONG lCSN, const string& rstrDeleteCharName);
	
	BOOL FindNFBlockList(LONG lCSN, CONT_NF_FRIEND_NICK& rkContNFBlock );
	void SetNFBlock(LONG lCSN, const CONT_NF_FRIEND_NICK& rkContNFBlock );
	void AddNFBlock(const LONG lCSN, const string& rstrAddBlockCharName);		// Block
	void DeleteNFBlock(const LONG lCSN, const string& rstrDeleteBlockCharName);	// UnBlock

	BOOL FindNFFriendApplicant(LONG lCSN, CONT_NF_FRIEND_NICK& rkContNFFriendApplicant );
	void SetNFFriendAplicant(LONG lCSN, const CONT_NF_FRIEND_NICK& rkContNFFriendApplicant );
	void AddNFFriendApplicant(const LONG lCSN, const string& rstrAddApplicantCharName);
	void DeleteNFFriendApplicant(const LONG lCSN, const string& rstrDeleteApplicantCharName);

	// 
	void SetAllNFCharInfoList() { m_bAllNFCharInfoList = TRUE; }
	BOOL GetAllNFCharInfoList() { return m_bAllNFCharInfoList; }

	// DetailCSN function
	void AddDetailCSN(LONG lCSN)
	{
		if (lCSN <= 0)
			return;

		std::map<LONG, LONG>::iterator iter = m_mapDetailCSN.find(lCSN);
		if (iter != m_mapDetailCSN.end())
			return;

		m_mapDetailCSN[lCSN] = lCSN;
	}

	void RemoveDetailCSN(LONG lCSN)
	{
		if (lCSN <= 0)
			return;

		std::map<LONG, LONG>::iterator iter = m_mapDetailCSN.find(lCSN);
		if (iter == m_mapDetailCSN.end())
			return;

		m_mapDetailCSN.erase(iter);
	}

	BOOL FindDetailCSN(LONG lCSN)
	{
		if (lCSN <= 0)
			return FALSE;

		std::map<LONG, LONG>::iterator iter = m_mapDetailCSN.find(lCSN);
		if (iter == m_mapDetailCSN.end())
			return FALSE;

		return TRUE;
	}

protected:
	TMsg			mLastMsg;
	TMsgQ			mRcvMsgQ;

	LRBAddress		m_CHSAddr;
	GameDataQ		mGameDataQ;
	CLink*			m_pLink;

	long			m_lErrorCode;

	NFRoomOption		m_roomOption;
	NFUserBaseInfo		m_nfUserBaseInfo;
	TMapNFCharInfoExt	m_mapCharInfoExt;
	CONT_NF_FRIEND_MGR	m_kContNFFriendMgr;
	CONT_NF_FRIEND_NICK_MGR	m_kContNFBlockMgr;
	CONT_NF_FRIEND_NICK_MGR	m_kContNFFriendApplicantMgr;

	// 캐릭터 로비에서 필요한 simple한 NFCharInfo를 요청하고 나서 NFcharInfoExt 정보가 필요해서 요청한 경우 이 리스트에 등록해놓고 리스트에서 체크한다..
	// m_mapCharInfoExt 에 아래 정보를 넣으면 클라이언트도 영향이 있으니 일단 패스...
	// 그렇다고 처음부터 모든 캐릭터 정보(NFCharInfoExt)를 다 읽어들일순 없어서... 가능하다면 이게 편하지 -_-;; 2011/7/20
	std::map<LONG, LONG>		m_mapDetailCSN;		


public:
	// nf_user를 위한 변수
	string			m_strSiteCode;
	string			m_strSiteUserID;
	string			m_strPWD;
	LONG			m_lAdminLEV;

public:
	LONG			m_lRcvMsgCnt;
	BOOL			m_bAllNFCharInfoList;
	LONG			m_lTutorialCSN;
};

///////////////////////////////////////////////////////////////////////////////
// CUserList

typedef plist<CUser> CUserList;


#endif //!USER_H