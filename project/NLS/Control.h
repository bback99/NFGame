//
// Control.h
//

#ifndef CONTROL_H
#define CONTROL_H

///////////////////////////////////////////////////////////////////////  
//
//enum LRB_STATUS
//{
//	STATE_STOP = 0,
//	STATE_RECOVER,	// 복구 작업 진행 중, find or Addmsg의 quing and redirect.
//	STATE_RUN
//};

enum
{
	CONTROL_RECOVER = 1,
	CONTROL_RECOVER_EX, 
	CONTROL_STOP,
};

enum
{
	SEND_LCSMSG = 1, 
	QUE_LCSMSG,
	STOP_LCSMSG
};

class CConfPath
{
public:
	CConfPath();
	const char* GetConfPath() { return (const char*)m_sConfigPath;}

private:
	BOOL SetConfPath(const char* strfilename);	

private:
	char m_sConfigPath[MAX_PATH];
};

///////////////////////////////////////////////////////////////////////////////////
// CControl

class CControl : public GXSigHandler
{
	IMPLEMENT_TISAFEREFCNT(CControl)
public:
	CControl();
	~CControl();
protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);
public:	// Main/Sub Mode
	BOOL Stop();

//	// LCS version 1.0
//	BOOL SelectMain(LONG lTime, string sName);
//	LONG GetMode();
//	void SetMode(LONG lMode);

public:	
	BOOL RunEx();
	BOOL RecoverEx();
	BOOL RecoverUserListEx();

	BOOL GetComputerName(string & comp_name);

private:

public:
	int m_nReconnectCnt;
//	// LCS version 1.0
//	LONG m_SVRMODE;
	CConfPath m_confPath;

};

extern CControl theControl;

#endif //!CONTROL_H
