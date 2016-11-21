// 
// Link.h
// 
/**********************************************************
Base Socket for Listener
-bback99
***********************************************************/

#ifndef _LINK_H__
#define _LINK_H__

class CUser;


class CLink : public GXLink
{
public:
	typedef GXLink TBase;

	static const DWORD INVALID_INDEX = DWORD_MAX;

	CLink();
	virtual ~CLink();
	void SetUser(CUser* pUser);
	CUser* GetUser() { return m_pUser; }
	_inline void SetIP(LONG lIP) { m_lClientIP = lIP; }
	_inline LONG GetIP() const { return m_lClientIP; }
	_inline DWORD GetIndex() const { return m_dwIndex; }
	void SetGatewayIP(LONG lIP)	{ m_lGatewayIP = lIP; }
	LONG GetGatewayIP()	{ return m_lGatewayIP;}
	void SetMacAddr(string sMacAddr) { m_sClientMacAddr = sMacAddr;}
	string GetMacAddr()	{ return m_sClientMacAddr; }

public:
	template<class T>
		BOOL DoSendMsg(const T& obj) {
		GBuf ro;
		if(!::LStore(ro, obj)) return FALSE;
		return DoSend(ro);
	}

protected:
	CUser* m_pUser;
	DWORD m_dwIndex;
	LONG m_lClientIP;					// Client IP Information (2002.10.15)
	LONG m_lGatewayIP;					// Gateway IP Information (2004.8.27)
	string m_sClientMacAddr;
};



#endif //_LINK_H__

