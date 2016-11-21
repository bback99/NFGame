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
	const char* GetConfDir() { return (const char*)m_sConfigDir;}

private:
	BOOL SetConfPath(const char* strfilename);	

private:
	char m_sConfigPath[MAX_PATH];
	char m_sConfigDir[MAX_PATH];
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
	BOOL Run();
	BOOL Stop();
	string GetParseData(string& sTarget);

public:
	CConfPath m_confPath;

public:
	//int GetQueryIndex(string &strQuery)
	//{
	//	map<string, int>::iterator i = m_mapRCQuery.find(strQuery);
	//	if (i == m_mapRCQuery.end())
	//		return -1;
	//	else
	//		return i->second;
	//}

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
//protected:
//	map<string, int> m_mapRCQuery;	

};

extern CControl theControl;



#endif //!Control_H
