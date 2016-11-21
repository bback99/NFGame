
#ifdef _NFGAME_

#include <ADL/MsgCHSCli_NF.h>

class CChannelContext;
class CUser;

class CNFDefaultMagData
{
public:
	CNFDefaultMagData() : m_nRegion(0), m_nMsgID(0), m_nMsgType(0) {}
	CNFDefaultMagData(int nRegion, int nMsgID, int nMsgType) : m_nRegion(nRegion), m_nMsgID(nMsgID), m_nMsgType(nMsgType) {}
public:
	int m_nRegion;
	int m_nMsgID;
	int m_nMsgType;
};

class CChannelContextNFImpl
{
IMPLEMENT_TISAFEREFCNT(CChannelContextNFImpl)
typedef map<long, long> MAP_USERROOMID;
public:
	CChannelContextNFImpl();
	virtual ~CChannelContextNFImpl();

	BOOL ProcessMessage(const CNFDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, string& strMsg);
	BOOL OnGRListPageReq(const CNFDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, MsgCliCHS_GRListPageReq* pMsg);

	void InsertUserFromMap(LONG lUSN, LONG lGRIID);
	void DeleteUserFromMap(LONG lUSN);
	DWORD GetUserRoomIDFromMap(LONG lUSN);

private:
	MAP_USERROOMID m_mapUserRoomID;		/// User별 roomid를 저장해둔당...
};

extern CChannelContextNFImpl theChannelContextNFImpl;

#endif