#pragma once
#include "Control.h"
#include <ADL/MsgODBGWCli.h>

class CODBGWManager 
	// kukuta : public IXObject
{
public:
	IMPLEMENT_TISAFE(CODBGWManager);
public:	
	CODBGWManager();
	virtual ~CODBGWManager();

	BOOL RegisterServiceReq();
#ifdef _JACKPOT_INDICATION_
	BOOL SendSelectJackpotMoney();
#else
	BOOL SendSelectJackpotMoney(LONG lSSN);
#endif
#ifdef _HOPE_JACKPOT_
	BOOL SendHopeJackpotSelectInfoReq();
#endif	// _HOPE_JACKPOT_
private :
	BOOL SendRegisterServiceReq(const LRBAddress& addrODBGW);

public :
	void OnLRBRegister(const LRBAddress &addr);
	void UpdateVIPList(LONG lSSN, vector<string> &queryAns);
	void GetVIPRoomList(LONG lSSN, MsgCHSCli_VIPRoomListAns &ans);

	BOOL GetRoomLRBAddr(LONG lSSN, RoomID &rid, LRBAddress &addrGLS);

	BOOL IsRMMServiceOn(LONG lSSN)
	{
		TLock lo(this);
		return m_setRMMSSN.find(lSSN) != m_setRMMSSN.end();
	}

	BOOL IsVIPServiceOn(LONG lSSN)
	{
		TLock lo(this);
		return m_setVIPSSN.find(lSSN) != m_setVIPSSN.end();
	}											 

	BOOL IsJackpotServiceOn() 
	{
		TLock lo(this);
		return m_bJackpotFlag;
	}

	void DeleteServiceNumber(const LRBAddress& addr)
	{
		TLock lo(this);
		// NACK이 올 경우에 해당 주소에 관련된 SSN을 Set에서 삭제한다.
		if (strstr(addr.GetString().c_str(), "AODBRMM"))
		{
			LONG lSSN;
			sscanf(addr.GetString().c_str(), "AODBRMM%d", &lSSN);
			m_setRMMSSN.erase(lSSN);
		}
		else if (strstr(addr.GetString().c_str(), "AODBVIP"))
		{
			LONG lSSN;
			sscanf(addr.GetString().c_str(), "AODBVIP%d", &lSSN);
			m_setVIPSSN.erase(lSSN);
		}
		else if(strstr(addr.GetString().c_str(), "AODBJACKPOT"))
		{
			SetJackpotServiceOn(FALSE);
		}
	}

	void OnRcvMsg(const LRBAddress& src, const LRBAddress& dest, GBuf& buf, WORD wMessageType, WORD wProtocol);

private :
	BOOL RegisterRMMServiceReq(LONG lSSN);
	BOOL RegisterVIPServiceReq(LONG lSSN);
	BOOL RegisterJackpotServiceReq();

	void SetJackpotServiceOn(BOOL bFlag) 
	{
		TLock lo(this);
		m_bJackpotFlag = bFlag;
	}
protected :
	void OnQueryAns(const LRBAddress &src, const LRBAddress &dst, MsgODBGWCli_QueryAns *pAns);
	void OnRegisterServiceNtf(const LRBAddress &src, const LRBAddress &dst);
	void OnRegisterServiceAns(const LRBAddress &src, const LRBAddress &dst);	
	void OnChangedNtf(const LRBAddress &src, const LRBAddress &dst, MsgODBGWCli_ChangedNtf *pNtf);

private :
#ifdef _JACKPOT_INDICATION_
	void OnRcvSelectJackpotTotalMoneyAns(MsgODBGWCli_QueryAns* pAns);
#endif

protected:
	DWORD m_dwRefCnt;
	STDMETHOD_(ULONG, AddRef)() { return (DWORD)::InterlockedIncrement((LPLONG)&m_dwRefCnt);};
	STDMETHOD_(ULONG, Release)(){ return (DWORD)::InterlockedDecrement((LPLONG)&m_dwRefCnt);};
protected:
	set<LONG> m_setRMMSSN;
	set<LONG> m_setVIPSSN;
	BOOL m_bJackpotFlag;	
//	map<string, int> m_mapQueryIndex;	

	class VIPRoomList
	{
	public:
		int m_sizeList;
		vector<string> m_vecRoomID;		
		vector<string> m_vecGLSIP;		
		vector<string> m_vecGLSPort;		
		vector<string> m_vecGLSAddr;		
		vector<string> m_vecRegPasswd;		
		vector<string> m_vecEntranceMoney;		
		vector<string> m_vecDisplayInfo;
	};

	typedef map<LONG, VIPRoomList*> VIPMap; // <SSN, UsnList*>

	VIPMap m_mapVIP;
};

extern CODBGWManager theODBGWMgr;

#ifdef _HOPE_JACKPOT_
class CHopeJackpotManager
{
	BOOL m_bHopeJackpotFlag;
	BOOL m_bHopeJackpotGroupFlag;
	LRBAddress m_addrMulticastJackpot;
	LRBAddress m_addrAnycastODBGW;
	LRBAddress m_addrMulticastCHS;

	set<LONG> m_setSSN;
	set<LONG> m_setSSNGroup;
	int m_nDisplayType;
	LONG m_lTotalWinningMoney;
	LONG m_lTotalJackpotMoney;
	int m_nTodayRemainWinner;

public:
	CHopeJackpotManager();
	~CHopeJackpotManager();

	BOOL Init();

	BOOL IsServiceOn();
	BOOL IsServiceOn(LONG lSSN);
	BOOL IsServiceGroup();
	BOOL IsServiceGroup(LONG lSSN);
	LRBAddress GetODBGWAddress();
	void GetJackpotInfo(int& nDisplayType, LONG& lTotalWinningMoney, LONG& lTotalJackpotMoney, int& nTodayRemainWinner);
	void SetJackpotInfo(int nDisplayType, LONG lTotalWinningMoney, LONG lTotalJackpotMoney, int nTodayRemainWinner);
	BOOL RegisterAddress();
};

extern CHopeJackpotManager theHopeJackpotManager;
#endif	// _HOPE_JACKPOT_
