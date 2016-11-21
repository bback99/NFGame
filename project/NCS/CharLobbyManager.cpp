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

// 이 함수는 CharLobbyContext에 구현하지 않고, Manager에서 구현 하는 이유는 Listener에서 Link를 옮겨와서 유저 정보가 엄는 상태에서
// CharLobbyContext로 옮겨갈 수 없기 때문에(USN이 아니고 GSN으로 CharLobby를 접속시켜야 하므로) 여기서 구현
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
	// SiteUserID 셋팅 :USN이 Valid하면, PMangUser -> NF_USER테이블의 SiteUserID 필드가 USN이 된다.
	if (ISVALID_USN(nfUBI.m_lUSN))						
	{
	 	char szTemp[255] = {0,};
	 	sprintf(szTemp, "%d", nfUBI.m_lUSN);
	 	strSiteUserID = szTemp;
	}
	// Invalid 하면, 클라이언트가 넘겨준 SiteUserID로 NF_USER테이블의 SiteUserID 필드로 저장한다.
	else				
	 	strSiteUserID = pUser->m_strSiteUserID;
	
	theLog.Put(DEV, "JoinNCSUser - SiteUserID : ", strSiteUserID, "\n"); 

	//////////////////////////////////////////////////////////////////////////
	LONG lErrCode = NF::G_NF_ERR_SUCCESS;

	// NFUser 테이블에서 GSN을 가져온다...
	if (!theNFDBMgr.SelectNFCharGSN(strSiteUserID, nfUBI.m_lGSN))
	{
	 	theLog.Put(ERR, "JoinNCSUser - SelectNFCharGSN Failed, SiteUserID : ", strSiteUserID, "\n");
		lErrCode = NF::G_NF_ERR_DB_SELECT_GSN;
	}
	else
	{
		if (nfUBI.m_lGSN <= 0)
		{
			// 없으면, Insert NFUser : nf_user에 regist 하고, 클라이언트로 캐릭터 생성하라고 알려준다. 
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
				// 한번이라도 요청한적 있으면... User정보에 셋팅한다..
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

	// 채널에서 들어온 놈이라, Char가 선택 되서 들어오는 놈이므로 강제로 Login시켜준다...
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
		m_pUserManager->KickOutUser(pPrevUser->GetGSN());				// 밀어내기
		m_pUserManager->RemoveUser(pPrevUser, pPrevUser->GetGSN());		// Add해주기 위해서 remove 먼저 한다...
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

	// NLS로 보낸다...
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
	/// @@@ 이 함수 호출시 위에서 lock 걸고 들어와야 함....
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

	// 캐릭터 생성하면, Test용으로 Inven에 넣는다...
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

		if (((*it).second).m_lItemCode == G_DEFAULT_ITEM_CODE_USABLE)	// 물약은 using을 키지 않는다...
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
	RemoveLink(pUser);										// 링크만 끊는다..(유저 초기화 안 함)
	m_pUserManager->RemoveUser(pUser, pUser->GetGSN());		// 유저리스트에서 삭제

	pUser->SetCSN(lLoginCSN);

	CCharLobby* pCharLobby = FindCharLobby(pUser->GetCSN());
	if (pCharLobby)
		::XsigQueueSignal(GetThreadPool(), pCharLobby, (HSIGNAL)CCharLobby::CHARLOBBY_LOGIN, (WPARAM)pUser, (LPARAM)0);

}