
#include "stdafx.h"
#include "User.h"
#include "CharLobby.h"
#include "CharLobbyContext.h"
#include <NFVariant/NFDBManager.h>
#include <NFVariant/NFMenu.h>
#include <NFVariant/NFItem.h>

CCharLobbyContext::CCharLobbyContext(CCharLobby* pCharLobby)
{
	m_pCharLobby = pCharLobby;
}

CCharLobbyContext::~CCharLobbyContext()
{
}

// SimpleInven에서 BaseInfo와 Inven정보를 읽어갔기 때문에.. 그 나머지만 읽어오면 된다.
BOOL CCharLobbyContext::GetAllNFCharInfoExt(NFCharInfoExt* pNFCharInfo, TlstInvenSlot& lstRemovedItem, LONG lLastPlayCSN, LONG lGSN, LONG& lUserError)
{
	BOOL bRet = TRUE;

	// 1. Detail한 inven 요청
	if (!theNFDBMgr.SelectNFCharInven(lstRemovedItem, pNFCharInfo->m_nfCharBaseInfo.m_strLastestLogOutDate, pNFCharInfo->m_nfCharInven, lGSN, lLastPlayCSN, lUserError))
	{
		lUserError = ERR_CRF_NOT_EXIST_INVEN;
		theLog.Put(ERR, "GetNFCharInfo SelectNFCharInvenByCSN is Fail!!!, Char GSN: ", lGSN, ", CSN : ", lLastPlayCSN); 
	}
	else
	{
		if (NF::G_NF_ERR_SUCCESS != lUserError)
			theLog.Put(ERR, "GetNFCharInfo SelectNFCharInvenByCSN is Fail!!!, Char GSN: ", lGSN, ", CSN : ", lLastPlayCSN, ", Err : ", lUserError);
	}

	// 수족관 물고기 정보
	if (!theNFMenu.GetAquaFish(lLastPlayCSN, pNFCharInfo->m_nfAquaFish))
	{
		lUserError = EC_NA_NOT_FOUND_AQUA_FISH;
		theLog.Put(ERR, "theNFMenu.GetAquaFish is Fail!!!, Char GSN: ", lGSN, ", CSN : ", lLastPlayCSN);
	}

	// WORKING(acepm83@neowiz.com) 능력치를 셋팅하기전에 수족관 정보를 읽어야 한다.(Beacause 수족관 버프)
	
	LONG lElapsedClearHour = 0; // 청소 경과시간
	LONG lElapsedFeedHour = 0;	// 밥준 경과시간
	
	if (!theNFDBMgr.SelectNFCharAqua(lGSN, lLastPlayCSN, pNFCharInfo->m_nfAqua, lElapsedClearHour, lElapsedFeedHour))
	{
		lUserError = NF::EC_NA_NOT_FOUND_AQUA;
		theLog.Put(ERR, "SelectNFCharAqua is Fail!!!, Char GSN: ", lGSN, ", CSN : ", lLastPlayCSN);
	}
	else
	{
		theNFMenu.CalcNFAquaGauge(pNFCharInfo, lElapsedClearHour, lElapsedFeedHour);
	}

	// 2. 능력치 셋팅
	lUserError = theNFItem.GetCharAbility(pNFCharInfo);
	if (NF::G_NF_ERR_SUCCESS != lUserError)
		theLog.Put(ERR, "GetNFCharInfo EquipedItemTotalAbility Error!!!, Char GSN: ", lGSN, ", CSN : ", lLastPlayCSN, ", Err : ", lUserError);

// 	// 3. LockedNote(+Main) 정보 읽기
// 	if (!theNFDBMgr.SelectNFLockedNote(lGSN, lLastPlayCSN, pNFCharInfo->m_nfLockedNote))
// 		theLog.Put(ERR, "GetNFCharInfo SelectNFLockedNote Error!!!, Char GSN: ", lGSN, ", CSN : ", lLastPlayCSN, ", Err : ", lUserError);

	// 4. 업적 정보 읽어오기
	if (!theNFDBMgr.SelectNFAchvList(lGSN, lLastPlayCSN, pNFCharInfo->m_nfCharAchievement.m_nfCharAchieve, lUserError))
		theLog.Put(ERR, "GetNFCharInfo SelectNFAchvList Error!!!, Char GSN: ", lGSN, ", CSN : ", lLastPlayCSN, ", Err : ", lUserError);

	return bRet;
}

void CCharLobbyContext::OnUserMsg(CUser* pUser, PayloadCliNCS& pld)
{
	if (NULL != pUser)
	{
		switch(pld.mTagID)
		{
		case PayloadCliNCS::msgReqLogOut_Tag:
			return OnReqLogOut(pUser, pld.un.m_msgReqLogOut);
		case PayloadCliNCS::msgReqNFCharInfoExt_Tag:
			return OnReqNFCharInfoExt(pUser, pld.un.m_msgReqNFCharInfoExt);
		case PayloadCliNCS::msgReqExitNFGame_Tag:
			return OnReqExitNFGame(pUser, pld.un.m_msgReqExitNFGame);
		case PayloadCliNCS::msgReqGCNSAP_Tag:
			return OnReqGCNSAP(pUser, pld.un.m_msgReqGCNSAP);
		case PayloadCliNCS::msgReqGRJoin_Tag:
			return OnReqGRJoin(pUser, pld.un.m_msgReqGRJoin);
		case PayloadCliNCS::msgReqCategoryList_Tag:
			return OnReqCategoryList(pUser, pld.un.m_msgReqCategoryList);
		case PayloadCliNCS::msgReqChannelList_Tag:
			return OnReqChannelList(pUser, pld.un.m_msgReqChannelList);
		case PayloadCliNCS::msgReqProductList_Tag:
			return OnReqProductList(pUser, pld.un.m_msgReqProductList);
		case PayloadCliNCS::msgReqChangeParts_Tag:
			return OnReqChangeParts(pUser, pld.un.m_msgReqChangeParts);
		case PayloadCliNCS::msgReqBuyItem_Tag:
			return OnReqBuyItem(pUser, pld.un.m_msgReqBuyItem);
		case PayloadCliNCS::msgReqRemoveItem_Tag:
			return OnReqRemoveItem(pUser, pld.un.m_msgReqRemoveItem);
		case PayloadCliNCS::msgReqOpenCardPack_Tag:
			return OnReqOpenCardPack(pUser, pld.un.m_msgReqOpenCardPack);
		case PayloadCliNCS::msgReqChangeCardSlot_Tag:
			return OnReqChangeCardSlot(pUser, pld.un.m_msgReqChangeCardSlot);
		case PayloadCliNCS::msgReqExchangeCards_Tag:
			return OnReqExchangeCards(pUser, pld.un.m_msgReqExchangeCards);
		case PayloadCliNCS::msgReqChangeQuickSlot_Tag:
			return OnReqChangeQuickSlot(pUser, pld.un.m_msgReqChangeQuickSlot);
		case PayloadCliNCS::msgReqRepairEnduranceItem_Tag:
			return OnReqRepairEnduranceItem(pUser, pld.un.m_msgReqRepairEnduranceItem);
		case PayloadCliNCS::msgReqNextEnchantInfo_Tag:
			return OnReqNextEnchantInfo(pUser, pld.un.m_msgReqNextEnchantInfo);
		case PayloadCliNCS::msgReqItemEnchant_Tag:
			return OnReqItemEnchant(pUser, pld.un.m_msgReqItemEnchant);
		case PayloadCliNCS::msgReqAquaFish_Tag:
			return OnReqAquaFish(pUser, pld.un.m_msgReqAquaFish);
		case PayloadCliNCS::msgReqRewardItem_Tag:
			return OnReqRewardItem(pUser, pld.un.m_msgReqRewardItem);
		case PayloadCliNCS::msgReqLockedNote_Tag:
			return OnReqLockedNote(pUser, pld.un.m_msgReqLockedNote);
		case PayloadCliNCS::msgReqNFLetterList_Tag:
			return OnReqNFLetterList(pUser, pld.un.m_msgReqNFLetterList);
		case PayloadCliNCS::msgReqNFLetterContent_Tag:
			return OnReqNFLetterContent(pUser, pld.un.m_msgReqNFLetterContent);
		case PayloadCliNCS::msgReqNFLetterReceiverCheck_Tag:
			return OnReqNFLetterReceiverCheck(pUser, pld.un.m_msgReqNFLetterReceiverCheck);
		case PayloadCliNCS::msgReqNFLetterSend_Tag:
			return OnReqNFLetterSend(pUser, pld.un.m_msgReqNFLetterSend);
		case PayloadCliNCS::msgReqNFLetterDelete_Tag:
			return OnReqNFLetterDelete(pUser, pld.un.m_msgReqNFLetterDelete);
		case PayloadCliNCS::msgReqNFFriendInfo_Tag:
			return OnReqNFFriendInfo(pUser, pld.un.m_msgReqNFFriendInfo);
		case PayloadCliNCS::msgReqNFFriendAdd_Tag:
			return OnReqNFFriendAdd(pUser, pld.un.m_msgReqNFFriendAdd);
		case PayloadCliNCS::msgReqNFFriendAccept_Tag:
			return OnReqNFFriendAccept(pUser, pld.un.m_msgReqNFFriendAccept);
		case PayloadCliNCS::msgReqNFFriendReject_Tag:
			return OnReqNFFriendReject(pUser, pld.un.m_msgReqNFFriendReject);
		case PayloadCliNCS::msgReqNFFriendDelete_Tag:
			return OnReqNFFriendDelete(pUser, pld.un.m_msgReqNFFriendDelete);
		case PayloadCliNCS::msgReqNFBlockList_Tag:
			return OnReqNFBlockList(pUser, pld.un.m_msgReqNFBlockList);
		case PayloadCliNCS::msgReqNFFriendApplicantList_Tag:
			return OnReqNFFriendApplicantList(pUser, pld.un.m_msgReqNFFriendApplicantList);
		case PayloadCliNCS::msgReqNFBlockOrUnBlock_Tag:
			return OnReqNFBlockOrUnBlock(pUser, pld.un.m_msgReqNFBlockOrUnBlock);
		case PayloadCliNCS::msgReqFollowUser_Tag:
			return OnReqFollowUser(pUser, pld.un.m_msgReqFollowUser);
		default:
			{
				theLog.Put(ERR, "CCharLobbyContext::OnRcvMsg - Unknown message(Tag:", pld.mTagID, ")");
				break;
			}
		}
	}
	m_pCharLobby->GetUserManager()->KickOutUser(pUser->GetCSN());
}

void CCharLobbyContext::PostUserDisconnect(LONG lKey)	/*CSN*/
{
	::XsigQueueSignal(GetThreadPool(), m_pCharLobby, (HSIGNAL)CCharLobby::CHARLOBBY_USERDISCONNECT, (WPARAM)lKey, (LPARAM)0);
}

void CCharLobbyContext::OnDisconnectUser(LONG lKey)
{
	CUser* pUser = m_pCharLobby->GetUserManager()->FindUser(lKey);
	if(NULL == pUser) return;

	RoomID roomID;
	pUser->NLSGetRoomID(roomID);
	TKey key(pUser->GetGSN(), pUser->GetCSN());

	LONG lLevel = pUser->GetLevel();
	if (lLevel < 0)
		lLevel = 0;
//	theNLSManager.UpdateUserToNLS(key, NLSCLISTATUS_DISCONNECT, roomID, lLevel);

	m_pCharLobby->GetUserManager()->DestroyUser(lKey);
}