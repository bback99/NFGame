#pragma once


class CVIPManager : public IXObject
{
public:
	IMPLEMENT_TISAFE(CVIPManager);
public:	
	void OnLRBRegister(const LRBAddress &addr);
	void OnQueryAns(const LRBAddress &src, const LRBAddress &dst, MsgODBGWCli_QueryAns *pAns);
	void OnChangeNtf(const LRBAddress &src, const LRBAddress &dst, MsgODBGWCli_ChangedNtf *pNtf);
	void UpdateVIPList(LONG lMSN, vector<string> &queryAns);

	void GetVIPRoomList(LONG lMSN, MsgCHSCli_VIPRoomListAns &ans);

	BOOL GetRoomLRBAddr(LONG lMSN, RoomID &rid, LRBAddress &addrGLS);

protected:
	DWORD m_dwRefCnt;
	STDMETHOD_(ULONG, AddRef)() { return (DWORD)::InterlockedIncrement((LPLONG)&m_dwRefCnt);};
	STDMETHOD_(ULONG, Release)(){ return (DWORD)::InterlockedDecrement((LPLONG)&m_dwRefCnt);};
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

	

protected:

	class VIPRoomList
	{
	public:
		int m_sizeList;
		vector<string> m_vecRID;
		vector<string> m_vecCHSIP;
		vector<string> m_vecGLSIP;
		vector<string> m_vecCHSPort;
		vector<string> m_vecGLSPort;
		vector<string> m_vecTitle;
		vector<string> m_vecEnterMoney;
		vector<string> m_vecPPing;
		vector<string> m_vecUserCnt;
		vector<string> m_vecIsSecret;
		vector<string> m_vecGLSAddr;
		vector<string> m_vecCHSAddr;
	};

	typedef map<LONG, VIPRoomList*> VIPMap; // <MSN, UsnList*>

	VIPMap m_mapVIP;
};



extern CVIPManager theVIPMgr;