#include "stdafx.h"
#include <NFVariant/NFDBManager.h>
#include <NFVariant/NFItem.h>
#include "User.h"
#include "CharLobby.h"
#include "CharLobbyManager.h"
#include "CharLobbyManagerUserManager.h"
#include <NLSManager.h>
using namespace NF;

CCharLobbyManager	theCharLobbyManager;

CCharLobbyManager::CCharLobbyManager()
{
	m_pUserManager = new CCharLobbyManagerUserManager(this);
}

CCharLobbyManager::~CCharLobbyManager()
{

}

BOOL CCharLobbyManager::CreateCharLobby()
{
	for(int i=0; i<NF_CHAR_LOBBY_CNT; i++)
	{
		CCharLobby* pLobby = new CCharLobby(i);
		if (pLobby)
			m_mapCharLobby.insert(make_pair(i, pLobby));
		else
			return FALSE;
	}

	return TRUE;
}

STDMETHODIMP_(ULONG) CCharLobbyManager::AddRef() 
{
	DWORD dwRefCnt = ::InterlockedIncrement((LPLONG)&m_dwRefCnt);
	return dwRefCnt;
}

STDMETHODIMP_(ULONG) CCharLobbyManager::Release() 
{
	DWORD dwRefCnt = ::InterlockedDecrement((LPLONG)&m_dwRefCnt);
	if(dwRefCnt == 0)
	{
	}
	return dwRefCnt;
}

void CCharLobbyManager::OnSignal(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam)
{
	switch((LONG)hSignal)
	{
	case CHARLOBBYMGR_NCSADDUSERANS:
		{
			JoinNCSUser(lParam);
			break;
		}
	case CHARLOBBYMGR_LOGOUT:
		{
			OnReqLogOut(lParam);
			break;
		}
	case CHARLOBBYMGR_USERDISCONNECT:
		{
			OnUserDisconnect((LONG)wParam);
			break;
		}
	default:
		{
			TLock lo(this);
			TBase::OnSignal( hSignal, wParam, lParam );
		}
	}
}

// �� �Լ��� CharLobbyContext�� �������� �ʰ�, Manager���� ���� �ϴ� ������ Listener���� Link�� �Űܿͼ� ���� ������ ���� ���¿���
// CharLobbyContext�� �Űܰ� �� ���� ������(USN�� �ƴϰ� GSN���� CharLobby�� ���ӽ��Ѿ� �ϹǷ�) ���⼭ ����
void CCharLobbyManager::JoinNCSUser(LPARAM lParam)
{
	TLock lo(this);
	CUser* pUser = (CUser*)lParam;
	if (!pUser) return;

	BOOL bIsAutoLogIn = FALSE;
	if (pUser->GetCSN() > 0)
		bIsAutoLogIn = TRUE;

	NFUserBaseInfo& nfUBI = pUser->GetNFUserBaseInfo();

	//////////////////////////////////////////////////////////////////////////
	string strSiteUserID;
	// SiteUserID ���� :USN�� Valid�ϸ�, PMangUser -> NF_USER���̺��� SiteUserID �ʵ尡 USN�� �ȴ�.
	if (ISVALID_USN(nfUBI.m_lUSN))						
	{
	 	char szTemp[255] = {0,};
	 	sprintf(szTemp, "%d", nfUBI.m_lUSN);
	 	strSiteUserID = szTemp;
	}
	// Invalid �ϸ�, Ŭ���̾�Ʈ�� �Ѱ��� SiteUserID�� NF_USER���̺��� SiteUserID �ʵ�� �����Ѵ�.
	else				
	 	strSiteUserID = pUser->m_strSiteUserID;
	
	theLog.Put(DEV, "JoinNCSUser - SiteUserID : ", strSiteUserID, "\n"); 

	//////////////////////////////////////////////////////////////////////////
	LONG lErrCode = NF::G_NF_ERR_SUCCESS;

	// NFUser ���̺��� GSN�� �����´�...
	if (!theNFDBMgr.SelectNFCharGSN(strSiteUserID, nfUBI.m_lGSN))
	{
	 	theLog.Put(ERR, "JoinNCSUser - SelectNFCharGSN Failed, SiteUserID : ", strSiteUserID, "\n");
		lErrCode = NF::G_NF_ERR_DB_SELECT_GSN;
	}
	else
	{
		if (nfUBI.m_lGSN <= 0)
		{
			// ������, Insert NFUser : nf_user�� regist �ϰ�, Ŭ���̾�Ʈ�� ĳ���� �����϶�� �˷��ش�. 
			//if (!theNFDBMgr.InsertNFUser(pUser->m_strSiteCode, strSiteUserID, pUser->m_strPWD, pUser->m_lAdminLEV, ans.m_lGSN))
			if (!theNFDBMgr.InsertNFUser("PMANG", strSiteUserID, "PWD", 0, nfUBI.m_lGSN))
			{
				theLog.Put(ERR, "GetNFCharBaseInfo - InsertNFUser Failed : ", strSiteUserID, "\n");
				lErrCode = NF::G_NF_ERR_DB_INSERT_NF_USER;
			}
		}
		else
		{
			theLog.Put(DEV, "JoinNCSUser SelectNFCharGSN Success, SiteUserID(USN) : ", strSiteUserID, ", GSN :", nfUBI.m_lGSN, "\n");

			if (!GetNFCharInfoFromDB(pUser, lErrCode))
				theLog.Put(ERR, "JoinNCSUser - GetNFCharInfoFromDB Failed, GSN, ", nfUBI.m_lGSN, ", Err :", lErrCode, "\n");
			else 
				// �ѹ��̶� ��û���� ������... User������ �����Ѵ�..
				pUser->SetAllNFCharInfoList();
		}
	}

	if (!AddUser(pUser))
	{
		theLog.Put(DEV, "JoinNCSUser - AddUser Failed, GSN : ", pUser->GetGSN(), "\n"); 
		return;
	}

 	MsgNCSCli_AnsJoinNCS	ans;
 	ans.Clear();
 
 	ans.m_lErrorCode = lErrCode;
 	if (ans.m_lErrorCode == NF::G_NF_ERR_SUCCESS) {
 		ans.m_lGSN = pUser->GetGSN();
 		ans.m_nfUserBaseInfo = pUser->GetNFUserBaseInfo();
 		ans.m_mapCSNCharInfoExt = pUser->GetTMapNFCharInfoExt();
 	}
 
 	PayloadNCSCli pld(PayloadNCSCli::msgAnsJoinNCS_Tag, ans);
 	m_pUserManager->SendToUser(pUser->GetGSN(), pld);

	// ä�ο��� ���� ���̶�, Char�� ���� �Ǽ� ������ ���̹Ƿ� ������ Login�����ش�...
	if (TRUE == bIsAutoLogIn)
		ProcessReqLogin(pUser, pUser->GetCSN());
}

BOOL CCharLobbyManager::AddUser(CUser* pUser)
{
	if (NULL == pUser) {
		theLog.Put(ERR, "AddUser pUser = NULL \n"); 
		return FALSE;
	}

	LONG lGSN = pUser->GetGSN();
	CUser* pPrevUser = m_pUserManager->FindUser(lGSN);
	if (pPrevUser) {
		theLog.Put(ERR, "AddUser"_COMMA, " Already GSN :", pUser->GetGSN());
		m_pUserManager->KickOutUser(pPrevUser->GetGSN());				// �о��
		m_pUserManager->RemoveUser(pPrevUser, pPrevUser->GetGSN());		// Add���ֱ� ���ؼ� remove ���� �Ѵ�...
	}

	if (!m_pUserManager->AddUser(pUser, lGSN)) {
		theLog.Put(ERR, "AddUser"_COMMA, " AddUser GSN: ", lGSN);
		return FALSE;
	}
	return TRUE;
}

void CCharLobbyManager::RemoveLink(CUser* pUser)
{
	m_pUserManager->RemoveLink(pUser);
}

BOOL CCharLobbyManager::GetNFCharInfoFromDB(CUser* pUser, LONG& lErr)
{
	TMapNFCharInfoExt& mapCharInfoExt = pUser->GetTMapNFCharInfoExt();

	std::list<LONG> lstCSN;
	if (!theNFDBMgr.SelectAllNFCharCSNByGSN(lstCSN, pUser->GetGSN(), lErr))
	{
 		theLog.Put(ERR, "GetNFCharInfoFromDB SelectAllNFCharCSNByGSN is Fail!!!, Char GSN: ", pUser->GetGSN(), ", Error : ", lErr, "\n");
 		return TRUE;
	}

	// nfcharinfos by each csn 
	ForEachElmt(std::list<LONG>, lstCSN, it, ij)
	{
 		TMapNFCharInfoExt::iterator iter = mapCharInfoExt.find(*it);
 		if (iter != mapCharInfoExt.end())
 			continue;

 		NFCharInfoExt		nfCharInfoExt;
 		nfCharInfoExt.Clear();

 		if (!theNFDBMgr.SelectNFCharBaseNExteriorInfo(nfCharInfoExt, pUser->GetGSN(), (*it))) {
 			lErr = NF::G_NF_ERR_DB_SELECT_CHAR;
 			return FALSE;
 		}

		theNFDataItemMgr.GetNFExp(nfCharInfoExt.m_nfCharBaseInfo.m_lLevel, nfCharInfoExt.m_nfCharBaseInfo.m_lExpMax);

 		if (!theNFDBMgr.SelectNFSimpleInven(nfCharInfoExt.m_nfCharInven, pUser->GetGSN(), (*it), lErr)) {
 			lErr = NF::G_NF_ERR_DB_SELECT_CHAR_INVEN;
 			return FALSE;
 		}

 		mapCharInfoExt[(*it)] = nfCharInfoExt;
	}

	lErr = NF::G_NF_ERR_SUCCESS;
	return TRUE;
}

CCharLobby* CCharLobbyManager::FindCharLobby(LONG lKey)
{
	DWORD dwCharLobbyID = lKey % NF_CHAR_LOBBY_CNT;
	
	TMapCharLobby::iterator iter = m_mapCharLobby.find(dwCharLobbyID);
	if (iter == m_mapCharLobby.end())
	{
		theLog.Put(ERR, "FindCharLobby - NotFound CharLobby, Key, ", lKey, "\n");
		return FALSE;
	}

	return (*iter).second;
}

void CCharLobbyManager::OnUserDisconnect(LONG lKey)
{
	TLock lo(this);
	CUser* pUser = m_pUserManager->FindUser(lKey);
	if (!pUser) return;

	// NLS�� ������...
	RoomID roomId;
	roomId.Clear();
	TKey key(pUser->GetUSN(), pUser->GetCSN());
	theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, roomId, 0);

	m_pUserManager->DestroyUser(pUser, pUser->GetGSN());
}

void CCharLobbyManager::PostUserDisconnect(LONG lKey)	/*GSN*/
{
	::XsigQueueSignal(GetThreadPool(), this, (HSIGNAL)CCharLobbyManager::CHARLOBBYMGR_USERDISCONNECT, (WPARAM)lKey, (LPARAM)0);
}

BOOL CCharLobbyManager::InsertDefaultCharItem(const LONG lGSN, NFCharInfo& nfCharInfo)
{
	/// @@@ �� �Լ� ȣ��� ������ lock �ɰ� ���;� ��....
	int nErrorCode = NF::EC_JNF_SUCCESS;

	SYSTEMTIME sys_time;
	::GetSystemTime(&sys_time);
	string strCurTime = ::format("%04d%02d%02d%02d", sys_time.wYear, sys_time.wMonth, sys_time.wDay, sys_time.wHour);

	NFBasicChar* pChar = theNFDataItemMgr.GetNFBasicChar(nfCharInfo.m_nfCharExteriorInfo.m_lBasicCharSRL);
	if (NULL == pChar)
	{
		theLog.Put(ERR, "GetNFCharBaseInfo - theNFDataItemMgr.GetNFBasicChar Faild, NotFound!!! CSN :  ", nfCharInfo.m_nfCharBaseInfo.m_lNFCSN, ", BasicCharIndex :", nfCharInfo.m_nfCharExteriorInfo.m_lBasicCharSRL, "\n");
		return FALSE;
	}

	// ĳ���� �����ϸ�, Test������ Inven�� �ִ´�...
	// Bags
	TMapInven		mapInven;

	// Usable Item
	Product usable_item;
	usable_item.Clear();
	TMapItemCodeProduct& mapProduct = theNFDataItemMgr.GetProduct();
	TMapItemCodeProduct::iterator itProduct = mapProduct.find(G_DEFAULT_ITEM_CODE_USABLE);
	if (itProduct != mapProduct.end())
	{
		ForEachElmt(TlstProduct, (*itProduct).second, itr2, ijr2)
		{
			if ((*itr2).m_lItemID == G_DEFAULT_ITEM_ID_USABLE) {
				usable_item = (*itr2);
				break;
			}
		}
	}
	else {
		theLog.Put(ERR, "GetNFCharBaseInfo - theNFDataItemMgr.GetProduct NotFound!!! ItemCode :", G_DEFAULT_ITEM_CODE_USABLE, "\n");
		return FALSE;
	}

	NFInvenSlot	inven100(0, G_DEFAULT_ITEM_CODE_USABLE, FALSE, "U", eItemType_UsableItem, strCurTime, G_MAX_DATE, usable_item.m_lItemCNT);
	mapInven[inven100.m_lItemCode] = inven100;

	// Default Equip Item
	NFInvenSlot	inven8(0, pChar->m_lRodItemCode, FALSE, "E", eItemType_Rod, strCurTime, G_MAX_DATE, 1);
	mapInven[inven8.m_lItemCode] = inven8;
	NFInvenSlot	inven11(0, pChar->m_lReelItemCode, FALSE, "E", eItemType_Reel, strCurTime, G_MAX_DATE, 1);
	mapInven[inven11.m_lItemCode] = inven11;
	NFInvenSlot	inven14(0, pChar->m_lLineItemCode, FALSE, "E", eItemType_Line, strCurTime, G_MAX_DATE, 1);
	mapInven[inven14.m_lItemCode] = inven14;
	NFInvenSlot	inven2(0, pChar->m_lLureItemCode, FALSE, "E", eItemType_Lure, strCurTime, G_MAX_DATE, 1);
	mapInven[inven2.m_lItemCode] = inven2;

	LONG	lErrorCode = 0;
	NFInvenSlot	OldInven(0, 0, TRUE, "E", 0, strCurTime, G_MAX_DATE, 1);

	ForEachElmt(TMapInven, mapInven, it, ij)
	{
		LONG lItemID = (*it).second.m_lItemCode*G_VALUE_CONVERT_ITEMCODE;
		if ((*it).second.m_strItemCategory == "U")
			lItemID = G_DEFAULT_ITEM_ID_USABLE;

		if (!theNFDBMgr.InsertBuyItem(1, lItemID, lGSN, nfCharInfo.m_nfCharBaseInfo.m_lNFCSN, (*it).second, nErrorCode))
		{
			theLog.Put(ERR, "GetNFCharBaseInfo - InsertBuyItem Faild, CSN :  ", nfCharInfo.m_nfCharBaseInfo.m_lNFCSN, "\n");
			return FALSE;
		}

		if (((*it).second).m_lItemCode != G_DEFAULT_ITEM_CODE_USABLE)
			(*it).second.m_bIsUsing = TRUE;

		lErrorCode = theNFItem.AddInvenSlotItem(nfCharInfo.m_nfCharInven, (*it).second);
		if (NF::G_NF_ERR_SUCCESS != lErrorCode) {
			theLog.Put(ERR, "GetNFCharBaseInfo - AddInvenSlotItem Faild, CSN :  ", nfCharInfo.m_nfCharBaseInfo.m_lNFCSN, ", Err : ", lErrorCode, "\n");
			return FALSE;
		}

		if (((*it).second).m_lItemCode == G_DEFAULT_ITEM_CODE_USABLE)	// ������ using�� Ű�� �ʴ´�...
			continue;

		theNFItem.AddUsingInven(nfCharInfo.m_nfCharInven.m_mapUsingItem, (*it).second);

		if (!theNFDBMgr.UpdatePartsByCSN(OldInven, (*it).second, lGSN, nfCharInfo.m_nfCharBaseInfo.m_lNFCSN, lErrorCode))
		{
			theLog.Put(ERR, "GetNFCharBaseInfo - UpdatePartsByCSN Faild, CSN :  ", nfCharInfo.m_nfCharBaseInfo.m_lNFCSN, "\n");
			return FALSE;
		}
	}
	return TRUE;
}

void CCharLobbyManager::ProcessReqLogin(CUser* pUser, LONG lLoginCSN)
{
	RemoveLink(pUser);										// ��ũ�� ���´�..(���� �ʱ�ȭ �� ��)
	m_pUserManager->RemoveUser(pUser, pUser->GetGSN());		// ��������Ʈ���� ����

	pUser->SetCSN(lLoginCSN);

	CCharLobby* pCharLobby = FindCharLobby(pUser->GetCSN());
	if (pCharLobby)
		::XsigQueueSignal(GetThreadPool(), pCharLobby, (HSIGNAL)CCharLobby::CHARLOBBY_LOGIN, (WPARAM)pUser, (LPARAM)0);

}