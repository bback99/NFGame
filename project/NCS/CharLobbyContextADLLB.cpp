#include "stdafx.h"
#include "User.h"
#include "CharLobby.h"
#include "CharLobbyContext.h"
#include "Category.h"


// LB
void CCharLobbyContext::OnReqGCNSAP(CUser* pUser, MsgCliNCS_ReqGCNSAP* pMsg)
{
	BOOL bRet = TRUE;

	ChannelPrefix chPrefix;
	chPrefix.m_lSSN = pMsg->m_lSSN;
	chPrefix.m_dwCategory = pMsg->m_dwCategory;

	ChannelInfo chInfo;

	LONG lErr = 0;

	lErr = theServiceTable.RecommendGameChannel(chPrefix, chInfo);
	if (lErr != 0)
	{
		bRet = FALSE;
	}

	MsgNCSCli_AnsGCNSAP msg;

	PayloadNCSCli pld(PayloadNCSCli::msgAnsGCNSAP_Tag, msg);

	pld.un.m_msgAnsGCNSAP->m_lErrorCode = lErr;
	pld.un.m_msgAnsGCNSAP->m_dwGCIID = chInfo.m_channelID.m_dwGCIID;
	pld.un.m_msgAnsGCNSAP->m_sCHSIP = chInfo.m_nsapCHS.GetIP();
	pld.un.m_msgAnsGCNSAP->m_dwPort = chInfo.m_nsapCHS.m_dwPort;

	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqGRJoin(CUser* pUser, MsgCliNCS_ReqGRJoin* pMsg)
{
	ChannelID chID;

	chID.m_lSSN = pMsg->m_lSSN;
	chID.m_dwCategory = pMsg->m_dwCategory;
	chID.m_dwGCIID = pMsg->m_dwGCIID;

	ChannelInfo chInfo;

	LONG lErr = theServiceTable.VerifyGameChannel(chID, chInfo);
	if (lErr != 0)
	{
		LOG(ERR, "Link : --OnGRJoinReq NsapReq Err [", lErr, "]--");
		LOG(ERR, "Link : --Error Info : UID[", pMsg->m_sUserID, "] SSN[", chID.m_lSSN, "] Category[", chID.m_dwCategory, "]--");
		return;
	}

	MsgNCSCli_AnsGRJoin msg;
	PayloadNCSCli pld(PayloadNCSCli::msgAnsGRJoin_Tag, msg);
	pld.un.m_msgAnsGRJoin->m_lErrorCode = lErr;
	pld.un.m_msgAnsGRJoin->m_sCHSIP = chInfo.m_nsapCHS.GetIP();
	pld.un.m_msgAnsGRJoin->m_dwPort = chInfo.m_nsapCHS.m_dwPort;

	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqCategoryList(CUser* pUser, MsgCliNCS_ReqCategoryList* pMsg)
{
	MsgNCSCli_AnsCategoryList	ans;

	ans.m_lErrorCode = theServiceTable.GetAllCategoryList(ans.m_lstChannelBaseInfo, pMsg->m_lSSN);

	PayloadNCSCli	pld(PayloadNCSCli::msgAnsCategoryList_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqChannelList(CUser* pUser, MsgCliNCS_ReqChannelList* pMsg)
{
	MsgNCSCli_AnsChannelList	ans;

	LONG lTotalUserCount, lTotalChannelCount;

	ChannelID	findChannelID(pMsg->m_lSSN, pMsg->m_dwCategory, 0);

	theServiceTable.GetChannelList(findChannelID, ans.m_lstChannelInfo, lTotalUserCount, lTotalChannelCount);

	PayloadNCSCli	pld(PayloadNCSCli::msgAnsChannelList_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}