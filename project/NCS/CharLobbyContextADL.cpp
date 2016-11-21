#include "stdafx.h"
#include <NFVariant/NFDBManager.h>
#include <NLSManager.h>
#include "User.h"
#include "CharLobby.h"
#include "CharLobbyContext.h"
#include "CharLobbyManager.h"
#include <NFVariant/NFItem.h>

// 1. CSN�� �ش��ϴ� NF_Char���̺��� ������ �ܾ�´�.
void CCharLobbyContext::OnReqNFCharInfoExt(CUser* pUser, MsgCliNCS_ReqNFCharInfoExt* pMsg)
{
	if (NULL == pMsg) return;

	MsgNCSCli_AnsNFCharInfoExt	ans;
	ans.Clear();
	ans.m_lErrorCode = NF::EC_JNF_SUCCESS;

	RoomID roomID;
	roomID.Clear();

	// 
	if (!pUser->FindDetailCSN(pUser->GetCSN()))
	{
		// JoinNCS���� �⺻���� ������ ���������Ƿ�... 
		pUser->GetNFCharBaseInfo(ans.m_nfCharInfoExt.m_nfCharBaseInfo);
		ans.m_nfCharInfoExt.m_nfCharExteriorInfo = pUser->GetExteriorInfo();
		ans.m_nfCharInfoExt.m_nfQuickSlot = pUser->GetQuickSlot();

		// inven�� �˴�.. DB�κ��� �о�´�...
		if (GetAllNFCharInfoExt(&ans.m_nfCharInfoExt, ans.m_lstRemovedItem, pUser->GetCSN(), pUser->GetGSN(), ans.m_lErrorCode))
		{
			// ������ ��ȿ��(�Ⱓ��, Ƚ����) üũ
			ans.m_lErrorCode = theNFItem.AutoChange_DefaultItem(pUser->GetGSN(), roomID, &ans.m_nfCharInfoExt, ans.m_lstRemovedItem, ans.m_lstChangedItem);
			if (NF::G_NF_ERR_SUCCESS == ans.m_lErrorCode)
			{
				pUser->AddNFCharInfoExt(pUser->GetCSN(), ans.m_nfCharInfoExt);
				pUser->AddDetailCSN(pUser->GetCSN());
			}
		}
		else
			theLog.Put(ERR, "GetNFCharBaseInfo - OnReqNFCharInfoExt Error, GSN, ", pUser->GetGSN(), ", Errorcode :", ans.m_lErrorCode);

		pUser->SetErrorCode(ans.m_lErrorCode);
	}
	else
	{
		TMapNFCharInfoExt::iterator iter = pUser->GetTMapNFCharInfoExt().find(pUser->GetCSN());
		if (iter != pUser->GetTMapNFCharInfoExt().end())
		{
			 pUser->GetNFCharInfoExt(ans.m_nfCharInfoExt);	// �����ͼ�...

			ans.m_lErrorCode = theNFItem.Check_TotalItemValid(pUser->GetGSN(), ans.m_nfCharInfoExt.m_nfCharInven, ans.m_lstRemovedItem);
			if (NF::G_NF_ERR_SUCCESS == ans.m_lErrorCode)
				// ������ ��ȿ�� üũ�� �Ѵ�...
				ans.m_lErrorCode = theNFItem.AutoChange_DefaultItem(pUser->GetGSN(), roomID, &ans.m_nfCharInfoExt, ans.m_lstRemovedItem, ans.m_lstChangedItem);
		}
		else
			ans.m_lErrorCode = EC_JNF_NOT_EXSIT_NF_CHAR;
	}

	ForEachElmt(TlstInvenSlot, ans.m_lstChangedItem, it, ij)
		theLog.Put(DEV, "GetNFCharBaseInfo - ########## changed_Inven Error, GSN, ", pUser->GetCSN(), ", Inven_Srl :", (*it).m_lInvenSRL);

	ForEachElmt(TlstInvenSlot, ans.m_lstRemovedItem, it2, ij2)
		theLog.Put(DEV, "GetNFCharBaseInfo - ########## removed_inven Error, GSN, ", pUser->GetCSN(), ", Inven_Srl :", (*it2).m_lInvenSRL);

	// LastPlayCSN�� �ٲ۴�...
	if (ans.m_lErrorCode == NF::EC_JNF_SUCCESS) 
	{
		pUser->SetCSN(pUser->GetCSN());
		theLog.Put(DEV, "OnReqNFCharInfoExt - Join WorldMap, GSN, ", pUser->GetGSN(), ", CSN : ", pUser->GetCSN());
	}

	PayloadNCSCli pld(PayloadNCSCli::msgAnsNFCharInfoExt_Tag, ans);
	m_pCharLobby->GetUserManager()->SendToUser(pUser->GetCSN(), pld);
}

void CCharLobbyContext::OnReqLogOut(CUser* pUser, MsgCliNCS_ReqLogOut* pMsg)
{
	if (NULL == pMsg) return;

	m_pCharLobby->GetUserManager()->RemoveLink(pUser);		// ��ũ�� ���´�..(���� �ʱ�ȭ �� ��)
	m_pCharLobby->GetUserManager()->RemoveUser(pUser, pUser->GetCSN());

	// �ٸ� LinkGroup���� �ѱ� ���� CUser�� �Ѱܾ� �Ѵ�.
	::XsigQueueSignal(GetThreadPool(), &theCharLobbyManager, (HSIGNAL)CCharLobbyManager::CHARLOBBYMGR_LOGOUT, (WPARAM)0, (LPARAM)pUser);
}

void CCharLobbyContext::OnReqExitNFGame(CUser* pUser, MsgCliNCS_ReqExitNFGame* pMsg)
{
	// LocationDB���� ����
	RoomID roomID;
	//theNLSManager.DeleteUserToNLS(pNewUser->GetGSN(), -1, NLSCLISTATUS_NFCHARLOBBY, roomID);
}