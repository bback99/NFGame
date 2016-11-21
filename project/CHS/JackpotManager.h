#pragma once

#ifdef _JACKPOT_INDICATION_
class CJackpotManager : public IXObject
{
	IMPLEMENT_TISAFEREFCNT(CJackpotManager)

	LONG	m_lPeriod;
	BOOL	m_bServiceFlagByGLS;

	typedef map<LONG, string> SSN_JACKPOTMONEY_MAP;
	SSN_JACKPOTMONEY_MAP m_mapSSNJackpotMoney;

	LRBAddress m_addrAODBGW;
	GXSigTimer m_jackpotMoneyUpdateTimer;
public :
	CJackpotManager();
	~CJackpotManager();
	BOOL	Init();
	void	SetJackpotMoneyBySSN(LONG lSSN, string strJackpotMoney);
	string	GetJackpotMoneyBySSN(LONG lSSN);

	BOOL	IsServiceOn(LONG lSSN) const;
	BOOL	IsServiceOn() const;

	void	NotifyIndicationBySSN(LONG lSSN);

	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);

	void Stop();
private :
	BOOL RegisterAddressFromConfigFile(const char* szSectionName, const char* szKeyName, const char* szDefault);
};

extern CJackpotManager theJackpotManager;
#endif
