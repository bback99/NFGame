//
// Control.h
//

#ifndef Control_H
#define Control_H

#include "Common.h"

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

class CControl
{
	IMPLEMENT_TISAFE(CControl)
public:
	CControl();
	~CControl();
public:
	BOOL RunControl();
	BOOL StopControl();
	BOOL GetIpAddress(string &str);
	BOOL GetComputerName(string & comp_name);

	void SetBootState(LONG lState);
	void SetBootStateOut(LONG lState);
	LONG GetBootState();
	long m_lCHSID;	

	BOOL InitChannel();
	
	void SetBLSRunFlag() { m_bFlagRunBLS = TRUE; }
	void ResetBLSRunFlag() { m_bFlagRunBLS = FALSE; }
	BOOL IsRunBLS() { return m_bFlagRunBLS; }

	////////// for IBB
	void SetIBBRunFlag() { m_bFlagRunIBB = TRUE; }
	void ResetIBBRunFlag() { m_bFlagRunIBB = FALSE; }
	BOOL IsRunIBB() { return m_bFlagRunIBB; }	

public:
	// DB등에서 읽어올 수 있도록 변경
	inline static LONG SSN2MSN(LONG lSSN)
	{
		switch (lSSN)
		{
		case 2:
		case 3:
		case 17:
		case 18:
		case 23:
		case 40:
			return 2;
		case 1:
		case 14:
		case 19:
		case 24:
		case 25:
		case 26:
			return 1;
		default:
			return -1;
		}
	}
protected:
	map<string, int> m_mapVIPQuery;	

protected:
	LONG m_lInitCount;
	LONG m_lRunLRBCount;
	LONG m_lRegInstCnt;
	LONG m_lRegLBCnt;

	LONG m_lBootState;

protected:
	BOOL m_bFlagRunBLS;
	BOOL m_bFlagRunIBB;

public:
    CConfPath m_confPath;
};

extern CControl theControl;

#endif //!Control_H
