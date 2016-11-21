
//
// UserMap.cpp
//

#include "stdafx.h"
#include "User.h"


///////////////////////////////////////////////////////////////////////////////////
// CNFUser
CNFUser::CNFUser()
{
}

///////////////////////////////////////////////////////////////////////////////////
// CUser

CUser::CUser(const LONG lUSN, const LONG lCSN, const string& strSiteCode, const string& strSiteUserID, const string& strPWD, const LONG lAdminLEV)
{
	m_nfUserBaseInfo.Clear();
	m_mapCharInfoExt.clear();

	m_nfUserBaseInfo.m_lUSN = lUSN;
	m_nfUserBaseInfo.m_lLastPlayCSN = lCSN;

	m_lTutorialCSN = 0;

	m_bAllNFCharInfoList = FALSE;

	m_kContNFFriendMgr.clear();
	m_kContNFBlockMgr.clear();
	m_kContNFFriendApplicantMgr.clear();
}

CUser::~CUser()
{
	if(m_pLink)
	{
		m_pLink->SetUser(NULL);
		delete(m_pLink);
		m_pLink = NULL;
	}
}

// for LCS 
STDMETHODIMP_(void) CUser::NLSGetKeyValue(TKey& key)
{
	key.m_lMainKey = GetGSN();
	key.m_lSubKey = GetCSN();
}
STDMETHODIMP_(BOOL) CUser::NLSGetRoomID(RoomID & roomID)
{
	roomID.Clear();
	roomID.m_lSSN = 55;
	return TRUE;
}
STDMETHODIMP_(void) CUser::NLSSetErrorCode(LONG lECode)
{
	SetErrorCode(lECode);
}

void CUser::UnpopMsg()
{
	TLock lo(this);
	mRcvMsgQ.push_front(mLastMsg); 
}

void CUser::UnpopMsg(const TMsg& msg) 
{
	TLock lo(this);
	mRcvMsgQ.push_front(msg); 
}

void CUser::PushMsg(const TMsg& msg) 
{
	TLock lo(this);
	mRcvMsgQ.push_back(TMsg());
	mRcvMsgQ.back().BCopy(msg);
}

BOOL CUser::PopMsg(CUser::TMsg& msg) 
{
	TLock lo(this);
	if(mRcvMsgQ.empty()) return FALSE;
	mLastMsg.BCopy(mRcvMsgQ.front());
	msg.BCopy(mLastMsg);
	mRcvMsgQ.pop_front();
	return TRUE;
}


void CUser::PushGameData(MsgCliNCS_GameData& data)
{
	TLock lo(this);

	TGameData dataNew;
	dataNew.m_lSize = data.m_sData.length();
	dataNew.m_GameData.m_lIndex = data.m_lIndex;
	dataNew.m_GameData.m_sData.assign(data.m_sData.c_str(), dataNew.m_lSize);
	mGameDataQ.push_back(dataNew);
}

DWORD CUser::GetGameData(LPLONG plIndex)
{
	TLock lo(this);

	if (mGameDataQ.size() == 0) return 0;
	TGameData& data = *mGameDataQ.begin();
	*plIndex = data.m_GameData.m_lIndex;
	return data.m_lSize;
}

BOOL CUser::GetGameData(LPLONG plIndex, LPSTR szData, BOOL bErase)
{
	TLock lo(this);

	if (mGameDataQ.size() == 0) return FALSE;

	TGameData& data = *mGameDataQ.begin();
	*plIndex = data.m_GameData.m_lIndex;
	::memcpy(szData, data.m_GameData.m_sData.c_str(), data.m_lSize);

	if (bErase) mGameDataQ.pop_front();

	return TRUE;
}

BOOL CUser::FindNFCharInfoExt(LONG lCSN, NFCharInfoExt& nfCharInfoExt)
{
	TLock lo(this);
	TMapNFCharInfoExt::iterator iter = m_mapCharInfoExt.find(lCSN);
	if (iter != m_mapCharInfoExt.end())
	{
		nfCharInfoExt = (*iter).second;
		return TRUE;
	}
	return FALSE;
}

BOOL CUser::FindNFFriend(LONG lCSN, CONT_NF_FRIEND& rkContNFFriend)
{
	TLock lo(this);
	CONT_NF_FRIEND_MGR::iterator iter = m_kContNFFriendMgr.find(lCSN);
	if( iter != m_kContNFFriendMgr.end() )
	{
		rkContNFFriend = (*iter).second;
		return TRUE;
	}

	return FALSE;
}

void CUser::SetNFFriend(LONG lCSN, const CONT_NF_FRIEND& rkContNFFriend)
{
	TLock lo(this);
	if(!rkContNFFriend.empty())
	{
		m_kContNFFriendMgr[lCSN] = rkContNFFriend;
	}
}

void CUser::AddNFFriend(const LONG lCSN, const TKey& rAddKey, const CNFFriend& rAddFriend)
{
	TLock lo(this);
	CONT_NF_FRIEND_MGR::iterator find_iter = m_kContNFFriendMgr.find(lCSN);
	if( m_kContNFFriendMgr.end() != find_iter )
	{
		CONT_NF_FRIEND_MGR::mapped_type& kContNFFriend = find_iter->second;
		kContNFFriend.insert( std::make_pair( rAddKey, rAddFriend ) );
	}
}

void CUser::DeleteNFFriend(const LONG lCSN, const string& rstrDeleteCharName)
{
	TLock lo(this);
	CONT_NF_FRIEND_MGR::iterator find_iter = m_kContNFFriendMgr.find(lCSN);
	if( m_kContNFFriendMgr.end() != find_iter )
	{
		CONT_NF_FRIEND_MGR::mapped_type& kContNFFriend = find_iter->second;
		CONT_NF_FRIEND::iterator iter = kContNFFriend.begin();
		while( kContNFFriend.end() != iter )
		{
			if( rstrDeleteCharName == iter->second.m_strCharName )
			{
				kContNFFriend.erase(iter);
				break;
			}

			++iter;
		}
	}
}

BOOL CUser::FindNFBlockList(LONG lCSN, CONT_NF_FRIEND_NICK& rkContNFBlock )
{
	TLock lo(this);
	CONT_NF_FRIEND_NICK_MGR::iterator iter = m_kContNFBlockMgr.find(lCSN);
	if( iter != m_kContNFBlockMgr.end() )
	{
		rkContNFBlock = (*iter).second;
		return TRUE;
	}

	return FALSE;
}

void CUser::SetNFBlock(LONG lCSN, const CONT_NF_FRIEND_NICK& rkContNFBlock )
{
	TLock lo(this);
	if( !rkContNFBlock.empty() )
	{
		m_kContNFBlockMgr[lCSN] = rkContNFBlock;
	}
}

// Block
void CUser::AddNFBlock(const LONG lCSN, const string& rstrAddBlockCharName)
{
	TLock lo(this);
	CONT_NF_FRIEND_NICK_MGR::iterator find_iter = m_kContNFBlockMgr.find(lCSN);
	if( m_kContNFBlockMgr.end() != find_iter )
	{
		CONT_NF_FRIEND_NICK_MGR::mapped_type& kContNFBlock = find_iter->second;
		kContNFBlock.push_back(rstrAddBlockCharName);
	}
	else
	{	
		CONT_NF_FRIEND_NICK kContNFBlock;
		kContNFBlock.push_back(rstrAddBlockCharName);
		m_kContNFBlockMgr.insert( std::make_pair( lCSN, kContNFBlock ));
	}
}


// UnBlock
void CUser::DeleteNFBlock(const LONG lCSN, const string& rstrDeleteBlockCharName)
{
	TLock lo(this);
	CONT_NF_FRIEND_NICK_MGR::iterator find_iter = m_kContNFBlockMgr.find(lCSN);
	if( m_kContNFBlockMgr.end() != find_iter )
	{
		CONT_NF_FRIEND_NICK_MGR::mapped_type& kContNFBlock = find_iter->second;
		CONT_NF_FRIEND_NICK::iterator find_iter2 = std::find( kContNFBlock.begin(), kContNFBlock.end(), rstrDeleteBlockCharName );
		if( find_iter2 != kContNFBlock.end() )
		{
			kContNFBlock.erase(find_iter2);
		}
	}
}

BOOL CUser::FindNFFriendApplicant(LONG lCSN, CONT_NF_FRIEND_NICK& rkContNFFriendApplicant )
{
	TLock lo(this);
	CONT_NF_FRIEND_NICK_MGR::iterator iter = m_kContNFFriendApplicantMgr.find(lCSN);
	if( iter != m_kContNFFriendApplicantMgr.end() )
	{
		rkContNFFriendApplicant = (*iter).second;
		return TRUE;
	}

	return FALSE;
}
void CUser::SetNFFriendAplicant(LONG lCSN, const CONT_NF_FRIEND_NICK& rkContNFFriendApplicant )
{
	TLock lo(this);
	if( !rkContNFFriendApplicant.empty() )
	{
		m_kContNFFriendApplicantMgr[lCSN] = rkContNFFriendApplicant;
	}
}
void CUser::AddNFFriendApplicant(const LONG lCSN, const string& rstrAddApplicantCharName)
{
	TLock lo(this);
	CONT_NF_FRIEND_NICK_MGR::iterator find_iter = m_kContNFFriendApplicantMgr.find(lCSN);
	if( m_kContNFFriendApplicantMgr.end() != find_iter )
	{
		CONT_NF_FRIEND_NICK_MGR::mapped_type& kContNFFriendApplicant = find_iter->second;
		kContNFFriendApplicant.push_back(rstrAddApplicantCharName);
	}
}
void CUser::DeleteNFFriendApplicant(const LONG lCSN, const string& rstrDeleteApplicantCharName)
{
	TLock lo(this);
	CONT_NF_FRIEND_NICK_MGR::iterator find_iter = m_kContNFFriendApplicantMgr.find(lCSN);
	if( m_kContNFFriendApplicantMgr.end() != find_iter )
	{
		CONT_NF_FRIEND_NICK_MGR::mapped_type& kContNFFriendApplicant = find_iter->second;
		CONT_NF_FRIEND_NICK::iterator find_iter2 = std::find( kContNFFriendApplicant.begin(), kContNFFriendApplicant.end(), rstrDeleteApplicantCharName );
		if( find_iter2 != kContNFFriendApplicant.end() )
		{
			kContNFFriendApplicant.erase(find_iter2);
		}
	}
}

void CUser::AddNFCharInfoExt(LONG lCSN, const NFCharInfoExt& nfCharInfoExt)
{
	TLock lo(this);
	TMapNFCharInfoExt::iterator it = m_mapCharInfoExt.find(lCSN);
	if (it != m_mapCharInfoExt.end())
		(*it).second = nfCharInfoExt;
	else
		m_mapCharInfoExt.insert(make_pair(lCSN, nfCharInfoExt));
}