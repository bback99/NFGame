#include "stdafx.h"
#include <NFVariant/NFDBManager.h>
#include <NLSManager.h>
#include <NFVariant/NFItem.h>
#include "User.h"
#include "CharLobby.h"
#include "CharLobbyManager.h"
#include "Category.h"
#include "CharLobbyManagerUserManager.h"

//// ACHV BEGIN
#include <ACHV/AchvDef.h>
static achv::CAchvMgr& g_achv = achv::CAchvMgr::Instance();
//// ACHV END

void CCharLobbyManager::OnUserMsg(CUser* pUser, PayloadCliNCS& pld)
{
	if (NULL != pUser)
	{
		switch(pld.mTagID)
		{
		case PayloadCliNCS::msgReqNFCharInfoList_Tag:
			return OnReqNFCharInfoList(pUser, pld.un.m_msgReqNFCharInfoList);
		case PayloadCliNCS::msgReqExistNick_Tag:
			return OnReqCheckExistNick(pUser, pld.un.m_msgReqExistNick);
		case PayloadCliNCS::msgReqCreateNFChar_Tag:
			return OnReqCreateNFChar(pUser, pld.un.m_msgReqCreateNFChar);
		case PayloadCliNCS::msgReqDeleteNFChar_Tag:
			return OnReqDeleteNFChar(pUser, pld.un.m_msgReqDeleteNFChar);
		case PayloadCliNCS::msgReqCharShopList_Tag:
			return OnReqCharShopList(pUser, pld.un.m_msgReqCharShopList);
		case PayloadCliNCS::msgReqLogIn_Tag:
			return OnReqLogin(pUser, pld.un.m_msgReqLogIn);
		case PayloadCliNCS::msgReqTutorial_Tag:
			return OnReqTutorial(pUser, pld.un.m_msgReqTutorial);
		default:
			{
				theLog.Put(ERR, "CharLobbyManager::OnRcvMsg - Unknown message(Tag:", pld.mTagID, ")");
				break;
			}
		}
	}
	m_pUserManager->KickOutUser(pUser->GetGSN());
}

void CCharLobbyManager::OnReqLogOut(LPARAM lParam)
{
	TLock lo(this);
	CUser* pUser = (CUser*)lParam;
	if (!pUser) return;

	if (!AddUser(pUser))
	{
		theLog.Put(ERR, "JoinNCSUser - OnReqLogOut Failed, GSN : ", pUser->GetGSN(), "\n"); 
		return;
	}

	if (!g_achv.logout(pUser->GetCSN()))
		theLog.Put(ERR, "JoinNCSUser - OnReqLogOut Failed, GSN : ", pUser->GetGSN(), "\n"); 

	MsgNCSCli_AnsLogOut ans;

	PayloadNCSCli pld(PayloadNCSCli::msgAnsLogOut_Tag, ans);
	m_pUserManager->SendToUser(pUser->GetGSN(), pld);
}

void CCharLobbyManager::OnReqNFCharInfoList(CUser* pUser, MsgCliNCS_ReqNFCharInfoList* pMsg)
{
	TLock lo(this);

	MsgNCSCli_AnsNFCharInfoList	ans;
	ans.Clear();

	if (pUser->GetAllNFCharInfoList()) {
		ans.m_mapCSNCharInfoExt = pUser->GetTMapNFCharInfoExt();
		ans.m_lErrorCode = NF::EC_JNF_SUCCESS;
	}
	else
	{
		TMapNFCharInfoExt& nfCharInfoExt = pUser->GetTMapNFCharInfoExt();
		if (theCharLobbyManager.GetNFCharInfoFromDB(pUser, ans.m_lErrorCode))
		{
			ans.m_mapCSNCharInfoExt = nfCharInfoExt;
			pUser->SetAllNFCharInfoList();
		}
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFCharInfoList_Tag, ans);
	m_pUserManager->SendToUser(pUser->GetGSN(), pld);
}

void CCharLobbyManager::OnReqCheckExistNick(CUser* pUser, MsgCliNCS_ReqExistNick* pMsg)
{
	MsgNCSCli_AnsExistNick	ans;
	ans.m_lErrorCode = NF::G_NF_ERR_SUCCESS;

	if (!theNFDBMgr.SelectNFCharGSN(pMsg->m_strNick, ans.m_lErrorCode))
		theLog.Put(ERR, "SelectNFCharGSN DB Fail!!!, Char GSN: ", pUser->GetGSN());

	if (ans.m_lErrorCode != NF::G_NF_ERR_SUCCESS)
		theLog.Put(DEV, "SelectNFCharGSN Exist Nick!!!, Char GSN: ", pUser->GetGSN(), ", NICK : ", pMsg->m_strNick.c_str());

	PayloadNCSCli pld(PayloadNCSCli::msgAnsJoinNCS_Tag, ans);
	m_pUserManager->SendToUser(pUser->GetGSN(), pld);
}

void CCharLobbyManager::OnReqCharShopList(CUser* pUser, MsgCliNCS_ReqCharShopList* pMsg)
{

}

void CCharLobbyManager::OnReqLogin(CUser* pUser, MsgCliNCS_ReqLogIn* pMsg)
{
	if (NULL == pMsg) return;

	ProcessReqLogin(pUser, pMsg->m_lLoginCSN);
}

void CCharLobbyManager::OnReqCreateNFChar(CUser* pUser, MsgCliNCS_ReqCreateNFChar* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsCreateNFChar	ans;
	TMapNFCharInfoExt& mapNFCharInfoExt = pUser->GetTMapNFCharInfoExt();
	NFCharInfoExt	nfCharInfoExt;
	nfCharInfoExt.Clear();

	nfCharInfoExt.m_nfCharBaseInfo.m_strCharName = pMsg->m_strCharName;
	nfCharInfoExt.m_nfCharExteriorInfo.m_lBasicCharSRL = pMsg->m_lDefaultChar;
	nfCharInfoExt.m_nfCharExteriorInfo.m_lHairColor = pMsg->m_lDefaultHairColor;
	nfCharInfoExt.m_nfCharExteriorInfo.m_lHairStyle = pMsg->m_lDefaultHairStyle;
	nfCharInfoExt.m_nfCharExteriorInfo.m_lDefaultJacket = pMsg->m_lDefaultJaket;
	nfCharInfoExt.m_nfCharExteriorInfo.m_lDefaultPants = pMsg->m_lDefaultPants;

	int nErrorCode = NF::EC_JNF_SUCCESS;

	// Parameter 체크
	if (pMsg->m_strCharName.size() <= 0 ||
		pUser->GetGSN() <= 0 ||
		pMsg->m_lDefaultChar <= 0 ||
		pMsg->m_lDefaultHairColor < 0 ||
		pMsg->m_lDefaultHairStyle <= 0 ||
		pMsg->m_lDefaultJaket <= 0 ||
		pMsg->m_lDefaultPants <= 0)
	{
		ans.m_lCSN = 0;
		nErrorCode = NF::EC_CNC_DEFAULT_VALUE_INVALID;
	}
	else
	{
		// 캐릭터 보유 갯수 5개 넘으면 안 됨..
		if (mapNFCharInfoExt.size() >= G_MAX_NF_CHAR_CNT)
			nErrorCode = NF::EC_CNC_MAX_CHAR_CNT;
		else
		{
			// 캐릭터 생성 성공
			BOOL bRet = theNFDBMgr.InsertNFChar(pUser->GetGSN(), nfCharInfoExt, nErrorCode);
			if (!bRet)
				theLog.Put(ERR, "GetNFCharBaseInfo - CreateNFCharInfo Failed, GSN, ", pUser->GetGSN(), ", Errorcode :", nErrorCode, ", NickName :", nfCharInfoExt.m_nfCharBaseInfo.m_strCharName.c_str());
			else
			{
				nErrorCode = NF::EC_JNF_SUCCESS;

				// Add CreateNFChar 
				TMapNFCharInfoExt::iterator iter = mapNFCharInfoExt.find(nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
				if (iter != mapNFCharInfoExt.end())
				{
					nErrorCode = NF::EC_CNC_NICKNAME_ALREADY;

					theLog.Put(ERR, "GetNFCharBaseInfo - CreateNFCharInfo Exist Add Failed, CSN :  ", nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN, ", NickName :", nfCharInfoExt.m_nfCharBaseInfo.m_strCharName.c_str());
				}
				else {
					// 지급 아이템 체크
					if (!InsertDefaultCharItem(pUser->GetGSN(), nfCharInfoExt))	
					{
						theLog.Put(ERR, "GetNFCharBaseInfo - InsertDefaultCharItem Failed, CSN : ", nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN, "\n");
						nErrorCode = NF::G_NF_ERR_DB_INSERT_DEFAULT_CHAR_ITEM;

						int nErr = NF::G_NF_ERR_SUCCESS;

						// 아이템 지급실패하면, 걍 리턴해버리고... 생성했던 캐릭터도 삭제 하자....
						if (!theNFDBMgr.DeleteNFChar(pUser->GetGSN(), nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN, nErr))
							theLog.Put(ERR, "GetNFCharBaseInfo - DeleteNFChar Failed, CSN : ", nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN, "\n");

						if (nErr != NF::G_NF_ERR_SUCCESS)
							nErrorCode = nErr;
					}
					else 
					{
						theLog.Put(DEV, "GetNFCharBaseInfo - CreateNFCharInfo Success, CSN : ", nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN, "\n");

						// 초기화
						for(int i=0; i<10; i++)
							nfCharInfoExt.m_nfQuickSlot.push_back(0);

						// 게임 머니도 db에서 Default로 지급 (100원), DB를 다시 읽지 않기 때문에 나중에 수정해야 할듯...고려해봐야함!!! 2011/7/20
						nfCharInfoExt.m_nfCharBaseInfo.m_llMoney	= G_DEFAULT_GIVE_GAMEMONEY;
						nfCharInfoExt.m_nfCharBaseInfo.m_lLevel		= G_DEFAULT_CHAR_LEVEL;
						theNFDataItemMgr.GetNFExp(1, nfCharInfoExt.m_nfCharBaseInfo.m_lExpMax);
						theNFDataItemMgr.GetNFAbility(nfCharInfoExt.m_nfCharExteriorInfo.m_lBasicCharSRL, nfCharInfoExt.m_nfAbility);

						mapNFCharInfoExt[nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN] = nfCharInfoExt;

						// 캐릭터 생성하면 최선의 Detail 정보이므로..
						pUser->AddDetailCSN(nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN);
					}
				}
			}
			ans.m_lCSN = nfCharInfoExt.m_nfCharBaseInfo.m_lNFCSN;
		}
	}

	ans.m_lErrorCode	= (long)nErrorCode;

	PayloadNCSCli pld(PayloadNCSCli::msgAnsCreateNFChar_Tag, ans);
	m_pUserManager->SendToUser(pUser->GetGSN(), pld);
}

void CCharLobbyManager::OnReqDeleteNFChar(CUser* pUser, MsgCliNCS_ReqDeleteNFChar* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsDeleteNFChar	ans;

	TMapNFCharInfoExt& mapNFCharInfoExt = pUser->GetTMapNFCharInfoExt();

	int nErrorCode = 0;

	theNFDBMgr.DeleteNFChar(pUser->GetGSN(), pMsg->m_lDeleteCSN, nErrorCode);
	if (nErrorCode >= 0)
	{
		// Delete CreateNFChar 
		TMapNFCharInfoExt::iterator iter = mapNFCharInfoExt.find(pMsg->m_lDeleteCSN);
		if (iter != mapNFCharInfoExt.end())
			mapNFCharInfoExt.erase(iter);

		ans.m_lErrorCode = NF::EC_DNF_SUCCESS;

		// 캐릭터 삭제하면 여기서도 날린다.
		pUser->RemoveDetailCSN(pMsg->m_lDeleteCSN);

		theLog.Put(DEV, "######### Result Size : ",  mapNFCharInfoExt.size(), "\n");
	}
	else {
		theLog.Put(ERR, "GetNFCharBaseInfo - DropNFCharInfo Failed, GSN:", pUser->GetGSN(), ", CSN:", pMsg->m_lDeleteCSN, ", ErrorCode:", nErrorCode);
		ans.m_lErrorCode = nErrorCode;
	}

	ans.m_mapCSNCharInfoExt = mapNFCharInfoExt;

	PayloadNCSCli pld(PayloadNCSCli::msgAnsDeleteNFChar_Tag, ans);
	m_pUserManager->SendToUser(pUser->GetGSN(), pld);
}

// 튜토리얼로 진입시 클라에서 서버로 GCNSap 정보를 요청하고 있으므로
// tutorial req/ans ADL을 만들고, 내용은 GCNSape 정보를 되돌려준다.
void CCharLobbyManager::OnReqTutorial(CUser* pUser, MsgCliNCS_ReqTutorial* pMsg)
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

	MsgNCSCli_AnsTutorial msg;

	PayloadNCSCli pld(PayloadNCSCli::msgAnsTutorial_Tag, msg);

	pld.un.m_msgAnsGCNSAP->m_lErrorCode = lErr;
	pld.un.m_msgAnsGCNSAP->m_dwGCIID = chInfo.m_channelID.m_dwGCIID;
	pld.un.m_msgAnsGCNSAP->m_sCHSIP = chInfo.m_nsapCHS.GetIP();
	pld.un.m_msgAnsGCNSAP->m_dwPort = chInfo.m_nsapCHS.m_dwPort;

	m_pUserManager->SendToUser(pUser->GetGSN(), pld);
}
