
#ifndef _CHAR_LOBBY_CONTEXT_
#define _CHAR_LOBBY_CONTEXT_

#include <NF/ADL/MsgNCSCli.h>
#include <NF/ADL/MsgCHSNCS.h>
#include <NF/ADL/MsgNCSNGS.h>


#include <ACHV/AchvEvent.h>

class CUser;
class CCharLobby;

class CCharLobbyContext
{
public:
	typedef PayloadNCSCli TMsg;

	CCharLobbyContext(CCharLobby* pCharLobby);
	virtual ~CCharLobbyContext();

	void OnUserMsg(CUser* pUser, PayloadCliNCS& pld);
	void PostUserDisconnect(LONG lKey);
	void OnDisconnectUser(LONG lKey);

protected:
	BOOL GetAllNFCharInfoExt(NFCharInfoExt* pNFCharInfo, TlstInvenSlot& lstRemovedItem, LONG lLastPlayCSN, LONG lGSN, LONG& lUserError);

	// Msg Handler
public:
	void OnReqLogOut(CUser* pUser, MsgCliNCS_ReqLogOut* pMsg);
	void OnReqExitNFGame(CUser* pUser, MsgCliNCS_ReqExitNFGame* pMsg);
	void OnReqNFCharInfoExt(CUser* pUser, MsgCliNCS_ReqNFCharInfoExt* pMsg);

	// NSAP
	void OnReqGCNSAP(CUser* pUser, MsgCliNCS_ReqGCNSAP* pMsg);
	void OnReqGRJoin(CUser* pUser, MsgCliNCS_ReqGRJoin* pMsg);
	void OnReqCategoryList(CUser* pUser, MsgCliNCS_ReqCategoryList* pMsg);
	void OnReqChannelList(CUser* pUser, MsgCliNCS_ReqChannelList* pMsg);
	
	// Menu
	void OnReqProductList(CUser* pUser, MsgCliNCS_ReqProductList* pMsg);
	void OnReqAchvInfo(CUser* pUser, MsgCliNCS_ReqAchvInfo* pMsg);
	void OnReqChangeParts(CUser* pUser, MsgCliNCS_ReqChangeParts* pMsg);
	void OnReqBuyItem(CUser* pUser, MsgCliNCS_ReqBuyItem* pMsg);
	void OnReqRemoveItem(CUser* pUser, MsgCliNCS_ReqRemoveItem* pMsg);
	void OnReqOpenCardPack(CUser* pUser, MsgCliNCS_ReqOpenCardPack* pMsg);
	void OnReqChangeCardSlot(CUser* pUser, MsgCliNCS_ReqChangeCardSlot* pMsg);
	void OnReqExchangeCards(CUser* pUser, MsgCliNCS_ReqExchangeCards* pMsg);
	void OnReqChangeQuickSlot(CUser* pUser, MsgCliNCS_ReqChangeQuickSlot* pMsg);
	void OnReqRepairEnduranceItem(CUser* pUser, MsgCliNCS_ReqRepairEnduranceItem* pMsg);
	void OnReqNextEnchantInfo(CUser* pUser, MsgCliNCS_ReqNextEnchantInfo* pMsg);
	void OnReqItemEnchant(CUser* pUser, MsgCliNCS_ReqItemEnchant* pMsg);
	void OnReqAquaFish(CUser* pUser, MsgCliNCS_ReqAquaFish* pMsg);
	void OnReqRewardItem(CUser* pUser, MsgCliNCS_ReqRewardItem* pMsg);
	void OnReqLockedNote(CUser* pUser, MsgCliNCS_ReqLockedNote* pMsg);

	// Community
	void OnReqNFLetterList(CUser* pUser, MsgCliNCS_ReqNFLetterList* pMsg);
	void OnReqNFLetterContent(CUser* pUser, MsgCliNCS_ReqNFLetterContent* pMsg);
	void OnReqNFLetterReceiverCheck(CUser* pUser, MsgCliNCS_ReqNFLetterReceiverCheck* pMsg);
	void OnReqNFLetterSend(CUser* pUser, MsgCliNCS_ReqNFLetterSend* pMsg);
	void OnReqNFLetterDelete(CUser* pUser, MsgCliNCS_ReqNFLetterDelete* pMsg);
	void OnReqNFFriendInfo(CUser* pUser, MsgCliNCS_ReqNFFriendInfo* pMsg);
	void OnReqNFFriendAdd(CUser* pUser, MsgCliNCS_ReqNFFriendAdd* pMsg);
	void OnReqNFFriendAccept(CUser* pUser, MsgCliNCS_ReqNFFriendAccept* pMsg);
	void OnReqNFFriendReject(CUser* pUser, MsgCliNCS_ReqNFFriendReject* pMsg);
	void OnReqNFFriendDelete(CUser* pUser, MsgCliNCS_ReqNFFriendDelete* pMsg);
	void OnReqNFBlockList(CUser* pUser, MsgCliNCS_ReqNFBlockList* pMsg);
	void OnReqNFFriendApplicantList(CUser* pUser, MsgCliNCS_ReqNFFriendApplicantList* pMsg);
	void OnReqFollowUser(CUser* pUser, MsgCliNCS_ReqFollowUser* pMsg);
	void OnReqNFBlockOrUnBlock(CUser* pUser, MsgCliNCS_ReqNFBlockOrUnBlock* pMsg);

	// Recv_From_NLS
	void ProcessNFFriendList(MsgNLSCLI_AnsLocation& msg);
	void ProcessNFFriendApplication(const MsgNLSCLI_AnsLocation& msg);		// 친구요청이 왔다.
	void ProcessNFFriendAccept(const MsgNLSCLI_AnsLocation& msg);			// 친구요청을 수락했다.
	void ProcessNFFriendAutoAccept(const MsgNLSCLI_AnsLocation& msg);
	void ProcessFollowUser(const MsgNLSCLI_AnsLocation& msg);
	void ProcessNFLetterNew(const MsgNLSCLI_AnsLocation& msg);

	// Recv_From_CHS
	void ProcessAcceptFriendFromCHS(const MsgCHSNCS_NtfNFFriendAccept& msg);

	// Recv_From_NGS
	void ProcessNewLetterFromNGS(const MsgNGSNCS_NtfNFLetterReceive& msg);
	void ProcessAcceptFriendFromNGS(const MsgNGSNCS_NtfNFFriendAccept& msg);
	void ProcessAddFriendFromNGS(const MsgNGSNCS_NtfNFFriendAdd& msg);

public:
	CCharLobby*		m_pCharLobby;

	// achv
	static void AchvReportCallback(LONG lGSN, LONG CSN, int achv_ID, const achv::EventItem_T *pEvtItem);

friend class CCharLobby;
};

#endif //_CHAR_LOBBY_CONTEXT_