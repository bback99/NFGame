// User.cpp: implementation of the CUser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "User.h"
#include <NFVariant/NFGameData.h>

//#include "Common.h"

//////////////////////////////////////////////////////////////////////
// for LCS 
STDMETHODIMP_(void) CUser::NLSGetKeyValue(TKey& key)
{
	key.m_lMainKey = GetUSN();
	key.m_lSubKey = GetCSN();
}
STDMETHODIMP_(BOOL) CUser::NLSGetRoomID(RoomID & roomID)
{
	ChannelID cid = GetChannelID();
	roomID.m_lSSN = cid.m_lSSN;
	roomID.m_dwCategory = cid.m_dwCategory;
	roomID.m_dwGCIID = cid.m_dwGCIID;
	roomID.m_dwGRIID = 0UL;

	return TRUE;
}
STDMETHODIMP_(void) CUser::NLSSetErrorCode(LONG lECode)
{
	SetErrorCode(lECode);
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUser::CUser(const NFUser& ud)
{
	m_UserData.BCopy(ud);
	m_pLink = NULL;
	m_lBLSStatFlag = -1L;
	m_lRcvMsgCnt = 0;
	m_sLastChangeNickDate = _T(DEFAULT_LAST_CHANGENICK_DATE);
	m_kContNFFriend.clear();
	m_kContNFBlock.clear();
}

CUser::CUser(const NFUserBaseInfo& ud)
{
	m_UserData.m_nfUserBaseInfo.BCopy(ud);
	m_UserData.m_nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN = ud.m_lLastPlayCSN;
	m_pLink = NULL;
	m_lBLSStatFlag = -1L;
	m_lRcvMsgCnt = 0;
	m_sLastChangeNickDate = _T(DEFAULT_LAST_CHANGENICK_DATE);
	m_kContNFFriend.clear();
	m_kContNFBlock.clear();
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

void CUser::SetLink(CLink* pLink)
{
	m_pLink = pLink;
	if(pLink)
		m_UserData.m_nfUserBaseInfo.m_lClientIp = 0;//m_pLink->GetIP(); // Poker 어뷰징 때문에 0으로 설정하게 함...
}

void CUser::UpdateAvatar(const string& sAvatar, const string& sAvatarFace, LONG lMood)
{
	m_UserData.m_nfUserBaseInfo.m_sAvatar.assign(sAvatar.begin(), sAvatar.end());
	m_UserData.m_nfUserBaseInfo.m_sAvatarFace.assign(sAvatarFace.begin(), sAvatarFace.end());
	m_UserData.m_nfUserBaseInfo.m_lMood = lMood;
}
void CUser::UpdateNickName(xstring sNickName)
{
	::BCopy(m_UserData.m_nfUserBaseInfo.m_sNickName, sNickName);
}
void CUser::UpdateItem(string sItem)
{
//	if(sItem.length() > 0)
//		m_UserData.m_sItem = sItem;
//#pragma oMSG("UserBaseInfo 에 없는 정보임..추가는 추후 결정.")
}

void CUser::UpdateLoginState(LONG lLoginState)
{
	m_UserData.m_nfUserBaseInfo.m_lLoginState = lLoginState;
}

LONG CUser::ChangeQuickSlot(vector<LONG>& vecQuickSlot)
{
	NFCharInfoExt& nfCharInfoExt = m_UserData.m_nfCharInfoExt;
	nfCharInfoExt.m_nfQuickSlot.clear();

	ForEachElmt(vector<LONG>, vecQuickSlot, it, ij)
		nfCharInfoExt.m_nfQuickSlot.push_back( (*it) );

	return 1;
}


BOOL CUser::FindNFFriend(CONT_NF_FRIEND& rkContNFFriend)
{
	if( m_kContNFFriend.empty() )
	{
		return FALSE;
	}

	rkContNFFriend = m_kContNFFriend;
	return TRUE;
}

void CUser::SetNFFriend(const CONT_NF_FRIEND& rkContNFFriend)
{
	m_kContNFFriend = rkContNFFriend;
}

void CUser::AddNFFriend(const TKey& rAddKey, const CNFFriend& rAddFriend)
{
	m_kContNFFriend.insert( std::make_pair( rAddKey, rAddFriend) );
}

void CUser::SetNFBlock(const CONT_NF_FRIEND_NICK& rkContBlock)
{
	if( !rkContBlock.empty() )
	{
		m_kContNFBlock = rkContBlock;
	}
}

void CUser::AddNFBlock(const string& rstrAddBlockCharName)
{
	m_kContNFBlock.push_back(rstrAddBlockCharName);
}

void CUser::DeleteNFBlock(const string& rstrDeleteBlockCharName)
{
	CONT_NF_FRIEND_NICK::iterator iter = std::find( m_kContNFBlock.begin(), m_kContNFBlock.end(), rstrDeleteBlockCharName );
	if( iter != m_kContNFBlock.end() )
	{
		m_kContNFBlock.erase(iter);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	CUserMap  - plist

void CUserMap::Add(CUser* pUser) 
{
	++m_dwLockDebug;
	if (m_dwLockDebug > 1)
		LOG(ERR_UK, "CHS_CUserMapErr,++++DEBUG : m_dwLockDebug : ", m_dwLockDebug);

	ASSERT(pUser);
	if(!pUser)
	{
		LOG(INF_UK, "CHS_CUserMap"_LK, "+++++ ASSERT : UserMap Add pUser : pUser is NULL +++++");
		return;
	}
	long lCSN = pUser->GetCSN();
	ASSERT(ISVALID_USN(lCSN));//lUSN != -1);
	if(!ISVALID_USN(lCSN))
	{
		LOG(INF_UK, "CHS_CUserMap"_LK, "+++++ ASSERT : UserMap Add CSN : Invalid CSN +++++");
		return;
	}

	push_back(pUser);

	--m_dwLockDebug;
}

void CUserMap::Remove(CUser* pUser) 
{
	++m_dwLockDebug;
	if (m_dwLockDebug > 1)
		LOG(ERR_UK, "CHS_CUserMapErr,++++DEBUG : m_dwLockDebug : ", m_dwLockDebug);

	ASSERT(pUser);
	if(!pUser)
	{
		LOG(INF_UK, "CHS_CUserMap"_LK, "+++++ ASSERT : UserMap Remove : pUser is NULL +++++");
		return;
	}
	iterator it = find(pUser);
	if(it != end())
		erase(it);
	--m_dwLockDebug;
}

CUser* CUserMap::Find(long lCSN) const
{
	ForEachCElmt(CUserMap, *this, it, it2)
	{
		CUser * pUser = *it;
		if(pUser->GetCSN() == lCSN)
		{
			return pUser;
		}
	}

	return NULL;
}

CUser* CUserMap::Find(xstring sLUserID) const
{
	ForEachCElmt(CUserMap, *this, it, it2)
	{
		CUser* pUser = *it;
		if(!pUser->GetUserID().compare(sLUserID))
		{
			return pUser;
		}
	}
	return NULL;
}

