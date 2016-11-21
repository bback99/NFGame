//
// User.h
//

#ifndef User_H
#define User_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "common.h"

class CUser
{
public:
	typedef PayloadCliCHS TMsg;

	CUser(const NFUserBaseInfo& ud);
	CUser(const NFUser& ud);	
	~CUser();
	void SetLink(CLink* pLink);
	CLink* GetLink() { return m_pLink; }

	NFUser& GetNFUser() { return m_UserData; }
	NFCharInfoExt* GetNFCharInfoExt() { return &m_UserData.m_nfCharInfoExt; }
	NFAbilityExt& GetAbilityExt() { return m_AbilityExt; }

	long GetUSN() const { return m_UserData.m_nfUserBaseInfo.m_lUSN; }
	long GetCSN() const { return m_UserData.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN; }
	long GetGSN() const { return m_UserData.m_nfUserBaseInfo.m_lGSN; }

	xstring GetUserID() { return m_UserData.m_nfUserBaseInfo.m_sUID; }
	xstring GetUserNick() { return m_UserData.m_nfCharInfoExt.m_nfCharBaseInfo.m_strCharName; }

	const ChannelID& GetChannelID() const { return m_ChannelID; }
	ChannelID& GetChannelID() { return m_ChannelID; }
	void SetChannelID(const ChannelID& cid) { m_ChannelID.BCopy(cid); }


	// for NLS
	STDMETHOD_(void, NLSGetKeyValue)(TKey& key);
	STDMETHOD_(BOOL, NLSGetRoomID) (RoomID & roomID);
	STDMETHOD_(void, NLSSetErrorCode)(LONG lECode);

	LONG GetSayUSN() { return m_UserData.m_nfUserBaseInfo.m_lUSN; }
	void SetErrorCode(long lErrorCode) { m_lErrorCode = lErrorCode; }
	long GetErrorCode() const { return m_lErrorCode; }

public:	
	const NFUser& GetUserData() const { return m_UserData; }
	NFUser& GetUserData() { return m_UserData; }
	const NFAbilityExt& GetUserDataExt() const { return m_AbilityExt; }
	NFAbilityExt& GetUserDataExt() { return m_AbilityExt; }

	void UpdateAvatar(const string& sAvatar, const string& sAvatarface, LONG lMood);
	void UpdateNickName(xstring sNickName);
	void UpdateItem(string sItem);
	void UpdateLoginState(LONG lLoginState);
	void ChangeUserState(LONG lUserState) { m_UserData.m_nfUserBaseInfo.m_lChannelJoinState = lUserState; }
	void UpdateGameData(string & sGameData) { m_UserData.m_nfUserBaseInfo.m_sUserGameData.assign(sGameData.begin(), sGameData.end()); }

	void SetBLSData(string & sBLS) { m_sBLSData.assign(sBLS.begin(), sBLS.end()); }
	string& GetBLSData() { return m_sBLSData; }
	LONG GetBLSStatFlag() { return m_lBLSStatFlag; }
	void SetBLSStatFlag(LONG lStat) { m_lBLSStatFlag = lStat; }
	void SetGameMode(LONG lGameMode) { m_lGameMode = lGameMode; }
	LONG GetGameMode() { return m_lGameMode; }

	void SetChangeNickDate(tstring date) { m_sLastChangeNickDate = date; }
	tstring GetLastChangeNickDate() const { return m_sLastChangeNickDate; }

	// NF
	LONG ChangeQuickSlot(vector<LONG>& vecQuickSlot);
	BOOL FindNFFriend(CONT_NF_FRIEND& rkContNFFriend);
	void SetNFFriend(const CONT_NF_FRIEND& rkContNFFriend);
	void AddNFFriend(const TKey& rAddKey, const CNFFriend& rAddFriend);

	const CONT_NF_FRIEND_NICK& GetNFBlockList() const { return m_kContNFBlock; }
	void SetNFBlock(const CONT_NF_FRIEND_NICK& rkContBlock);
	void AddNFBlock(const string& rstrAddBlockCharName);		// Block
	void DeleteNFBlock(const string& rstrDeleteBlockCharName);	// UnBlock

public:
	DWORD m_dwGRIID;
	LONG m_lRcvMsgCnt;

protected:
	//NFUserBaseInfo m_UserData;	
	NFUser			m_UserData;
	NFAbilityExt	m_AbilityExt;
	ChannelID m_ChannelID;
	CLink* m_pLink;

	LONG m_lErrorCode;
	LONG m_lGameMode;
	string m_sBLSData;
	LONG m_lBLSStatFlag;
	tstring m_sLastChangeNickDate;

	CONT_NF_FRIEND		m_kContNFFriend;
	CONT_NF_FRIEND_NICK m_kContNFBlock;
};

///////////////////////////////////////////////////////////////////////////////
// CUserList

typedef plist<CUser> CUserList;
class CUserMap : protected plist<CUser>
{
	typedef plist<CUser> TBase;
public:
	CUserMap() {m_dwLockDebug = 0;};
	typedef TBase::const_iterator const_iterator;
	typedef TBase::iterator iterator;

	void Add(CUser * pUser);
	void Remove(CUser* pUser);
	CUser* Find(long lCSN) const;
	CUser* Find(xstring sLUserID) const;
	const_iterator begin() const { return TBase::begin(); }
	iterator begin() { return TBase::begin(); }
	const_iterator end() const { return TBase::end(); }
	iterator end() { return TBase::end(); }
	LONG Size() { return TBase::size(); }
private:
	DWORD m_dwLockDebug;
};
/////////////////////////////////////////////////////////////////////////////

#endif // !defined(User_H)

