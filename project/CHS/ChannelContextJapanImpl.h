/**
#ifdef _GLOBALIZATION_

#include <ADL/MsgCHSCli_japan.h>

class CChannelContext;
class CUser;

class CJapanDefaultMagData
{
public:
	CJapanDefaultMagData() : m_nRegion(0), m_nMsgID(0), m_nMsgType(0) {}
	CJapanDefaultMagData(int nRegion, int nMsgID, int nMsgType) : m_nRegion(nRegion), m_nMsgID(nMsgID), m_nMsgType(nMsgType) {}
	// virtual ~CJapanDefaultMagData() {}
public:
	int m_nRegion;
	int m_nMsgID;
	int m_nMsgType;
};

class CChannelContextJapanImpl
{
	IMPLEMENT_TISAFEREFCNT(CChannelContextJapanImpl)
	typedef map<long, long> MAP_USERROOMID;
public:
	CChannelContextJapanImpl();
	virtual ~CChannelContextJapanImpl();

	BOOL ProcessMessage(const CJapanDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, string& strMsg);
	BOOL OnPckRoomJoinReq(const CJapanDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, MsgCliCHSJapan_PckRoomJoinReq* pMsg);
	BOOL OnPckRoomOutReq(const CJapanDefaultMagData& jdmd, CChannelContext& channelCtx, const CUser& user, MsgCliCHSJapan_PckRoomOutReq* pMsg);

	void InsertUserFromMap(LONG lUSN, LONG lGRIID);
	void DeleteUserFromMap(LONG lUSN);
	DWORD GetUserRoomIDFromMap(LONG lUSN);

private:
	MAP_USERROOMID m_mapUserRoomID;		/// User별 roomid를 저장해둔당...
};

extern CChannelContextJapanImpl theChannelContextJapanImpl;

#endif
*/