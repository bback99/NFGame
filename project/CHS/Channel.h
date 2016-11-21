//
// Channel.h
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include "User.h"
#include "ChatHistory.h"
#include "RCReporter.h"
#include "StatisticsTable.h"
#include <NF/ADL/MsgCHSNGS.h>


class CLink;
class CChannel;
class CChannelContext;


// signal object Handle
// from GLS
#define CHANNELSIGNAL_ROOMCREATE ((HSIGNAL)0xffffffff)
#define CHANNELSIGNAL_ROOMDELETE ((HSIGNAL)0xfffffffe)
#define CHANNELSIGNAL_ROOMSTATUSCHANGE ((HSIGNAL)0xfffffffd)
#define CHANNELSIGNAL_USERROOMJOIN ((HSIGNAL)0xfffffffc)
#define CHANNELSIGNAL_USERROOMLEAVE ((HSIGNAL)0xfffffffb)
#define CHANNELSIGNAL_USERROOMCHANGE ((HSIGNAL)0xfffffffa)
#define CHANNELSIGNAL_USERROOMAVATAR ((HSIGNAL)0xfffffff9)
#define CHANNELSIGNAL_USERROOMNICKNAME ((HSIGNAL)0xfffffff8)
#define CHANNELSIGNAL_CHANGEROOMOPTION ((HSIGNAL)0xfffffff7)
#define CHANNELSIGNAL_CHANGEGAMEOPTION ((HSIGNAL)0xfffffff6)
#define CHANNELSIGNAL_INVITEREQ ((HSIGNAL)0xfffffff5)
#define CHANNELSIGNAL_REMGLS ((HSIGNAL)0xfffffff4)
#define CHANNELSIGNAL_CHANGELOGINSTATE ((HSIGNAL)0xfffffff3)

#define CHANNELSIGNAL_FROMGLS ((HSIGNAL)0xfffffff0)
#define CHANNELSIGNAL_NLSANSWER	((HSIGNAL)0xffffffef)

#define LISTENER_JOINCHANNELANS		((HSIGNAL)0xffffffee)
#define LISTENER_INVITECHANNELANS	((HSIGNAL)0xffffffed)
#define LISTENER_DJOINCHANNELANS    ((HSIGNAL)0xffffffec)
#define LISTENER_DUPLICATEJOINCHANNELANS    ((HSIGNAL)0xffffffeb)	// for 채널 재접속...


// from ELB
#define CHANNELSIGNAL_CHANNELLISTANS ((HSIGNAL)0xffffffe8)
#define CHANNELSIGNAL_GLSINFOANS ((HSIGNAL)0xffffffe7)
#define CHANNELSIGNAL_GRDIRECTCREATEANS ((HSIGNAL)0xffffffe6)

#define CHANNELSIGNAL_IBBANS			((HSIGNAL)0xffffffe5)
// from general
#define CHANNEL_USERJOIN		((HSIGNAL)0xffffffdf)
#define CHANNEL_DIRECTCREATE	((HSIGNAL)0xffffffde) 

#define CHANNEL_USERLINKCUT_SIG		((HSIGNAL)0xffffffdd) 
#define CHANNEL_USERLINKCUT_SIG_FROM_NLS ((HSIGNAL)0xfffffddd) 
#define CHANNEL_PROCESS_NF_FRIEND_ACCEPT ((HSIGNAL)0xffffdddd) 
#define CHANNEL_PROCESS_NF_FRIEND_ADD ((HSIGNAL)0xfffddddd) 
#define CHANNEL_PROCESS_FOLLOW_USER ((HSIGNAL)0xffdddddd)

enum {
	CHANNEL_USERLINKCUT = 0, 
	CHANNEL_LRBMSG,
	CHANNEL_ALIVECHK,
	CHANNEL_SMGWMSG,
	CHANNEL_SMGWCMD,
	CHANNEL_ANNOUNCEMSG,
	CHANNEL_INVITEUSERJOIN
};


//// Using in accuse
#define ACCUSESIGNAL_AGENTANSWER		((HSIGNAL)0xff000001)

//// for Reset UserMsgCnt
#define CHANNEL_RESET_USERMSGCNT		((HSIGNAL)0xff000004)
#define LISTENER_RESET_USERMSGCNT		((HSIGNAL)0xff000005)
//////////////////////////////////////////////////////////////////////
// Safe Lock
class CALock
{
public:
	CALock() { m_gcs = NULL; }
	CALock(GCriticalSection* gcs) { m_gcs = gcs; m_gcs->Lock(); }
	~CALock() { if(m_gcs) m_gcs->Unlock(); }
private:
	GCriticalSection* m_gcs;
};
#define AUTO_LOCK(DATA)	CALock alock(DATA);


/////////////////////////////////////////////////////////////////////////////////// 
// CChannelContext

class CChannelContext
{
public:
	typedef PayloadCHSCli TMsg;
	CChannelContext(CChannel* pChannel, const ChannelBaseInfo& binfo);

	// from AMS signaling...
public:
	void OnRemNGS(const NSAP& nsap);

	// from GLS signaling...
public:
	void OnRoomDelete(DWORD dwRSN);
	void OnRoomStatusChange(DWORD dwRSN, LONG lRoomState);
	void OnUserRoomLeave(DWORD dwRSN, long lUSN);
	void OnUserRoomChange(DWORD dwRSN, ChangeUserGameDataList & lstGUD);
	void OnRoomCreate(DWORD dwRSN, const NFRoomBaseInfo& info);
	void OnUserRoomJoin(DWORD dwRSN, const NFUserBaseInfo& info);
	void ChangeUserInfo(DWORD dwRSN, NFRoomInfoInChannel& rinfo, ChangeUserGameData & ugd);
	void OnChangeRoomOption(DWORD dwRSN, const NFRoomOption& option);
	void OnReqFreeRoomInfo(LONG lCSN, const RoomID& roomID, const LRBAddress& addrNGS);
	void OnUserRoomAvatar(DWORD dwRSN, LONG lUSN, const string & sAvatar, const string & sAvatarFace, LONG lMood);
	void OnUserRoomNickName(DWORD dwRSN, LONG lUSN, const xstring sNickName);
	void OnChangeGameOption(DWORD dwRSN, const string& option);
	void OnChangeLoginState(DWORD dwRSN, LONG lUSN, LONG lLoginState);

	// from RTRKS signaling
public:
	void OnAddRankData(long lUSN, string& strRank);

public:
	void AddUser(CUser* pUser);
	void RemoveUser(CUser* pUser);
	CUser* FindUser(long lCSN);
	CUser* FindUser(xstring sUID);

public:
//	static void AchvReportCallback(LONG GSN, LONG CSN, int achv_ID, const achv::EventItem_T *pEvtItem);

	void OnUserDisconnect(long lCSN);
	void OnUserDisconnectFromNLS(long lCSN, CUser* pNewUser);
	void OnUserChangeChannel(long lCSN);
	void OnUserChatPart(xstring sUID);
	void OnUserMsg(CUser * pUser, PayloadCliCHS& pld);
	void ChannelJoinFromNLS(CUser * pUser, LONG lMsgType);
	void OnChannelListReq(LONG lCSN, LONG lCategory);

	void OnNGSInfoReq(MsgCliCHS_NGSInfoReq * pMsg, LONG lCSN, LONG lMsgType) ;
	LONG OnUserJoinReq(MsgCliCHS_JoinChannelReq * pMsg, CUser & user);	
	LONG OnUserJoinReq_FreeMode(CUser& user);
	void OnDirectCreateReq(MsgCliCHS_GRDirectCreateReq * pMsg, LONG lCSN);
	BOOL OnDirectCreateReq_LCS(CUser & user);	
	void OnDirectInviteReq(MsgCliCHS_DirectInviteReq * pMsg, xstring sUserID);
	void OnChatMsgTo(MsgCliCHS_ChatMsgTo * pMsg, LONG lCSN);
	void OnChatMsgAll(MsgCliCHS_ChatMsgAll * pMsg, LONG lCSN);
	void OnLoginStateChangeNtf(MsgCliCHS_LoginStateChangeNtf * pMsg, LONG lCSN);

	void OnAccuseReq(MsgCliCHS_AccuseReq * pMsg, CUser & user);
	void UserAccuseReq(MsgCliCHS_AccuseReq * pMsg, CUser & user);
	void RoomTitleAccuseReq(MsgCliCHS_AccuseReq * pMsg, CUser & user);
	void PrearrangeGameAccuseReq(MsgCliCHS_AccuseReq * pMsg, CUser & user);

	void OnGetUserGameInfoReq(MsgCliCHS_GetUserGameInfoReq * pMsg, CUser & user);
	void OnGetRankInfoReq(MsgCliCHS_GetUserRankInfoReq * pMsg, CUser & user);
	void OnReqAchieveInfo(MsgCliCHS_ReqAchvInfo * pMsg, CUser & user);

	void OnReqNFFriendAdd(MsgCliCHS_ReqNFFriendAdd* pMsg, CUser& user);
	void OnReqNFFriendAccept(MsgCliCHS_ReqNFFriendAccept* pMsg, CUser& user);
	void OnReqAwaiterList(MsgCliCHS_ReqAwaiterList* pMsg, CUser& user);
	void OnReqNFBlockOrUnBlock(MsgCliCHS_ReqNFBlockOrUnBlock* pMsg, CUser& user);
	void OnReqFollowUser(MsgCliCHS_ReqFollowUser* pMsg, CUser& user);	

	void OnReqRoomList			(MsgCliCHS_ReqRoomList* pMsg,		CUser & user);
	void OnReqChangeParts		(MsgCliCHS_ReqChangeParts* pMsg,	CUser & user);
	void OnReqChangeCardSlot	(MsgCliCHS_ReqChangeCardSlot* pMsg, CUser & user);
	void OnReqExchangeCards		(MsgCliCHS_ReqExchangeCards* pMsg,	CUser & user);
	void OnReqBuyItem			(MsgCliCHS_ReqBuyItem* pMsg,		CUser & user);
	void OnReqRemoveItem		(MsgCliCHS_ReqRemoveItem* pMsg,		CUser & user);
	void OnReqOpenCardPack		(MsgCliCHS_ReqOpenCardPack* pMsg,	CUser & user);
	void OnReqChangeQuickSlot	(MsgCliCHS_ReqChangeQuickSlot* pMsg,	CUser & user);
	void MakeContentHeader(AccuseBus & bus, string & game_name);

	void OnReqNextEnchantInfo(MsgCliCHS_ReqNextEnchantInfo* pReq, CUser& user);
	void OnReqItemEnchant(MsgCliCHS_ReqItemEnchant* pReq, CUser& user);

	void OnReqAquaFish(MsgCliCHS_ReqAquaFish* pReq, CUser& user);

	void SendToUser(long lCSN, const PayloadCHSCli & pld);
	void SendToUser(long lCSN, const GBuf & buf);
	void SendToUser(CUser* pUser, const PayloadCHSCli & pld);
	void SendToUser(CUser* pUser, const GBuf & buf);
	void SendToAll(const PayloadCHSCli& pld, long lCSN = INVALID_CSN);
	void SendToAll(const GBuf& buf);

	void SendToInviteAll(PayloadInvitation& pld);
	LONG OnInvitationReq(LONG lCSN, MsgCHSCli_GRInvitationReq& rMsg);

	void GetUserList(NFUserInfoList& userBaseInfoList);
	BOOL GetWaitRoomList(ChannelID cid, NFRoomInfoInChannelList& lstRoom, UINT nGetCnt = 10, LONG lType = 1);
	BOOL GetRoomWithRoomID(DWORD dwGRIID, NFRoomInfoInChannel* rinfo);
	void OnAddRoomList(ChannelID cid, NFRoomInfoInChannelList& lstRoom);
	BOOL FindRoom(DWORD dwGRIID);

	LONG GetMaxUserCount() {return m_ChannelInfo.m_lMaxCount;}
	BOOL IsUserConnected(LONG lCSN);
	void ResetRcvMsgCnt();
	DWORD AllocFreeRoomIndex()
	{
		AUTO_LOCK(&m_gcs);
		DWORD dwRoomIndex = 0;
		if (m_lstRoomIndexFree.size() <= 0)
			return dwRoomIndex;
		else
		{
			ForEachElmt(list<LONG>, m_lstRoomIndexFree, it, ij)
			{
				if ((*it) > 0) {
					dwRoomIndex = (DWORD)(*it);
					break;
				}
			}
			m_lstRoomIndexFree.erase(it);
		}
		return dwRoomIndex;
	}
	tstring GetCurrentDate();
protected:
	CChannel* m_pChannel;
	
	//ChannelInfo m_ChannelInfo;
	//PC방 UserCount를 추가하기 위해서 상속받은 ChannelInfoPCRoom Class로 교체
	//단, 클라이언트와 통신하기 위해선 ChannelInfoPCRoom 클래스의 ChannelInfo의 데이터만 넘겨주면 된다.
	ChannelInfoPCRoom	m_ChannelInfo;

	NFRoomInfoInChannelList m_roomlist;

	// 테스트 용
	NFRoomInfoInChannelList m_roomlist_Test;
	std::list<LONG>			m_lstRoomIndexFree;

	////////// Lock for RoomList(m_roomlist) /////////
	GCriticalSection m_gcs;
	//////////////////////////////////////////////////

	CUserMap m_UsersMap;

	ChatHistory m_ChatHistory;
	//Reomot Channel List Req/Ans Control	//neo CRC

#ifdef USE_CRCREPORTER
	CRCReporter m_RCReporter;
#endif

friend class CChannel;
};

///////////////////////////////////////////////////////////////////////////////////
// CChannel

class ChannelLinkGroup : public XLinkAdlManagerT<CLink, PayloadCliCHS>
{
public:
	typedef XLinkAdlManagerT<CLink, PayloadCliCHS> TBase;
public:
	ChannelLinkGroup(CChannel* pChannel) : m_pChannel(pChannel) { m_hSignal = NULL; }
protected:
	virtual BOOL OnError(TLink* pSocket, long lEvent, int nErrorCode);
	virtual BOOL OnRcvMsg(CLink* pSocket, PayloadCliCHS& rPld);
public:
	BOOL IsActive() { return (m_hSignal != NULL); }
	BOOL IsHandle(HSIGNAL hSig) { return ((m_hSignal) && (m_hSignal == hSig)); }
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD_(void, OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
	void Lock();
	void Unlock();
	BOOL Deactivate();
public:
	void UserLinkCut(DWORD dwIndex);
protected:
	void DestroyLink(CLink* pLink);
protected:
	CChannel* m_pChannel;
	HSIGNAL m_hSignal;
	friend CChannel;
};

class InviteLinkGroup : public XLinkAdlManagerT<CInviteLink, PayloadInvitation>  
{
public:
	typedef XLinkAdlManagerT<CInviteLink, PayloadInvitation>  TBase;
public:
	InviteLinkGroup(CChannel* pChannel) : m_pChannel(pChannel) { m_hSignal = NULL; }
protected:
	virtual BOOL OnError(TLink* pSocket, long lEvent, int nErrorCode);
	virtual BOOL OnRcvMsg(CInviteLink* pSocket, PayloadInvitation& rPld);
	BOOL OnRcvInvitationReq(CInviteLink* pLink, MsgInvitationReq* pMsg);
public:
	BOOL IsActive() { return (m_hSignal != NULL); }
	BOOL IsHandle(HSIGNAL hSig) { return ((m_hSignal) && (m_hSignal == hSig)); }
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD_(void, OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
	void Lock();
	void Unlock();

	BOOL Deactivate();
public:
	void OnAddLink(CInviteLink* pLink);
	void SendToAll(const GBuf& buf);
	void SendMsg(CInviteLink* pLink, PayloadInvitation& pld);
protected:
	void DestroyLink(CInviteLink* pLink);
protected:
	CChannel* m_pChannel;
	HSIGNAL m_hSignal;
	friend CChannel;
};

class CChannel : public GXSigHandler
{
	IMPLEMENT_TISAFEREFCNT(CChannel)
public:
	typedef GXSigHandler TBase;
	typedef map<LONG, LONG> TMapBanishUser;			// USN - RegTime
private:
	CChannel(const CChannel&);
public:
	virtual BOOL Run();
	virtual BOOL Stop();
protected:
	void SendToLink(CLink* pLink, const GBuf& buf);
	void SendToInviteAll(const GBuf& buf);
protected: 
	void OnSignal_UserJoin(WPARAM wParam, LPARAM lParam);
	void OnSignal_DirectCreate(WPARAM wParam, LPARAM lParam);
	CUser* FindUser(LONG lCSN) { return m_pChannelContext->FindUser(lCSN); }

public: // for LRB signal...
	void PostRemNGS(const NSAP& nsap);

public:
	void OnSignal_RemNGS(WPARAM, LPARAM);
	void OnSignal_ForwardIBBMsg(WPARAM wParam, LPARAM lParam);
	void PushNGSMessage(GBuf & msg);

protected:
	BOOL PopGLSMessage(GBuf & msg);

	void OnRoomCreateFromNGS(MsgNGSCHS_RoomCreateNtf & msg);
	void OnRoomDeleteFromNGS(MsgNGSCHS_RoomDeleteNtf & msg);
	void OnRoomStatusChangeFromNGS(MsgNGSCHS_RoomStatusChangeNtf & msg);
	void OnUserRoomJoinFromNGS(MsgNGSCHS_UserRoomJoinNtf & msg);
	void OnUserRoomLeaveFromNGS(MsgNGSCHS_UserRoomLeaveNtf & msg);
	void OnUserRoomChangeFromNGS(MsgNGSCHS_UserInfoInRoomChangeNtf & msg);
	void OnChangeRoomOptionFromNGS(MsgNGSCHS_ChangeRoomOptionNtf & msg);
	void OnChangeGameOptionFromNGS(MsgNGSCHS_ChangeGameOptionNtf & msg);
	void OnChangeLoginStateFromNGS(MsgNGSCHS_ChangeLoginStateNtf & msg);
	void OnGameRoomListAnsFromNGS(MsgNGSCHS_GameRoomListAns & msg);
	void OnReqFreeRoomInfoFromNGS(MsgNGSCHS_ReqFreeRoomInfo & msg);
	BOOL GetWaitRoomList(ChannelID cid, NFRoomInfoInChannelList& lstRoom, UINT nGetCnt = 10, LONG lType = 1);

protected:
	BOOL AddSocket(CLink* pLink);
	void RemoveSocket(CLink* pLink);
public: // for Listener signal...
	void PostInviteReq(CLink* pLink, const DWORD dwGRIID);

protected:
	void OnSignal_InviteReq(WPARAM, LPARAM);

public: // for LB Signal ..
	void PostChannelListAns(long lCSN, GBuf& buf);
	void PostGLSInfoAns(long lCSN, GBuf& buf);
	void PostGRDirectCreateAns(long lCSN, GBuf& buf);
	void PostNtfNFFriendAdd(long lCSN, GBuf& buf);
	void PostNtfNFFriendAccept(long lCSN, GBuf& buf);
	void PostNtfNFLetterReceive(long lCSN);
protected:
	void OnSignal_ChannelListAns(WPARAM wParam, LPARAM lParam);
	void OnSignal_GLSInfoAns(WPARAM wParam, LPARAM lParam);
	void OnSignal_GRDirectCreateAns(WPARAM wParam, LPARAM lParam);

public: // for PMS signal
	void OnAnnounceMsgFromPMS(const GBuf& buf);

protected:
	CChannel(const ChannelID& channnelID, const ChannelBaseInfo& binfo, long lChannelType);
	virtual ~CChannel();

	void OnGRDirectCreateReq(CUser * pUser);

	LONG IncRoomID();
	LONG DecRoomID();
public:
	DWORD GetRoomID();
	DWORD GetRoomIDbyFreeRoom();
	LONG GetMaxUserCount() {return m_pChannelContext->GetMaxUserCount();}

	ChannelID & GetChannelID() { return m_ChannelID; }

	BOOL IsUserConnected(LONG lUSN) { return m_pChannelContext->IsUserConnected(lUSN);}
	BOOL IsLinkRegistered(CLink *pLink) { return m_apChannelLinkGroup->FindLink(pLink->GetHandle()) == pLink;}

	void OnSendCyclicUserCount();
	
protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hObj, WPARAM wParam, LPARAM lParam);
	void OnSignalEx(HSIGNAL hObj, WPARAM wP, LPARAM lP);	// GLS 메세지의 분리위해.
	void PostUserMsg(long lUSN);
	void OnKickOutBanishUser();

	BOOL OnRcvInvitationReq(const NSAP& nsap, const RoomID& roomID, const UserBaseInfo& userInfo, const xstring& sPasswd, const xstring& sMsg, const USNList& usnList, ArcListT<LONG>& lErrorList);
protected:
	ChannelID m_ChannelID;
	DWORD m_lRoomID;
	CChannelContextPtr m_pChannelContext;
	LONG m_lChannelType;

	AutoPtrT<ChannelLinkGroup> m_apChannelLinkGroup;
	AutoPtrT<InviteLinkGroup> m_apInviteLinkGroup;

	GSafeBufQ m_Queue;

	GXSigTimer		m_timerBanishUser;
	TMapBanishUser	m_mapBanishUser;

friend class CChannelContext;
friend class CChannelDir;
friend class ChannelLinkGroup;
friend class InviteLinkGroup;

	friend long GetNFCharInfo(CUser * pUser, CChannel * pChannel);
};

typedef RCPtrT<CChannel> CChannelPtr;
typedef vector< CChannelPtr > CChannelPtrTable;

#endif //!CHANNEL_H
