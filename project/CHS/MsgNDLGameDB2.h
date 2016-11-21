//
// MsgNDLGameDB2.h
//

#ifndef NDL_MsgNDLGameDB2_h
#define NDL_MsgNDLGameDB2_h



// ERROR_INSUFFICIENT_BUFFER, ERROR_INVALID_DATA, ERROR_MORE_DATA


//==============================
//		NDLGameDB2.ndl
//============================== 

#ifndef _VERSION_JAPAN_
	typedef string			xstring;
	#define BSize_xstring	BSize_string
	#define BEncode_xstring BEncode_string
	#define BDecode_xstring BDecode_string
#else
	typedef wstring			xstring;
	#define BSize_xstring	BSize_wstring
	#define BEncode_xstring BEncode_wstring
	#define BDecode_xstring BDecode_wstring
#endif

enum CHANNEL_ROOM_STATE
{
	RSTATE_INVALID = -1,	 // 어떤(?) 이유에서 방에 들어오면 안된다..(초기값...)
	RSTATE_PLAYER_READY = 0, // 게임이 시작되지 않았으며, 방에 플레이어로 참여할 수 있는 상태
	RSTATE_PLAYER_PLAY,      // 게임 플레이 중이며, 방에 플레이어로 참여할 수 있는 상태
	RSTATE_OBSERVER, 	     // 방에 관전자로만 참여 가능한 상태, 게임 시작 여부는 상관없음
	RSTATE_USER_MAX,		 // 방에 참여할 수 없는 상태(= 제한 인원이 찬 상태), 게임 시작 여부는 상관없음
	RSTATE_USER_MAX_MAN,	 // 방에 남자는 참여할수 없는 상태(= 남자 제한인원 Full). 게임 시작 여부는 상관없음. 쿵쿵팅 게임에서 사용
	RSTATE_USER_MAX_WOMAN	 // 방에 여자는 참여할수 없는 상태(= 여자 제한인원 Full). 게임 시작 여부는 상관없음. 쿵쿵팅 게임에서 사용
};

enum DBQUERY_TYPE{
	DB_QUERYTYPE_NOTDEFINED = 0,
	DB_GLS_GETUSERDATA = 1, // 사용자 데이타를 달라 (GLS)
	DB_GETUSERDATA = DB_GLS_GETUSERDATA,
	DB_SETUSERDATA = 2,	//사용자 데이타를 세팅하라
	DB_CHS_GETUSERDATA = 3, // 사용자 데이타를 달라 (CHS)
	DB_SETEXTMONEY,	// 초과머니를 설정하라
	DB_QUERY = 101,	//string query
	DB_QUERY_LIST,	//실행 결과를 List로 Return
	DB_MULTI_QUERY,
	DB_LOG_QUERY,
	DB_SCH_QUERY = 200,
	DB_SCH_QUERY_LIST
};


class DB_GetUserInData {
public:
	LONG m_lUSN;
	xstring m_sUID;
	LONG m_lSex;
	LONG m_lJumin;
	LONG m_lRegion;
	LONG m_lFirstUSN;
public:
#ifndef DB_GetUserInData_Ctor // to override default ctor, define this symbol
	DB_GetUserInData() { }
#endif
	DB_GetUserInData(const DB_GetUserInData& obj) { operator=(obj); }
	explicit DB_GetUserInData(const LONG& lUSN, const xstring& sUID = xstring(), const LONG& lSex = LONG(), const LONG& lJumin = LONG(), const LONG& lRegion = LONG(), const LONG& lFirstUSN = LONG()) 
		 : m_lUSN(lUSN), m_sUID(sUID), m_lSex(lSex), m_lJumin(lJumin), m_lRegion(lRegion), m_lFirstUSN(lFirstUSN) { }
	DB_GetUserInData& operator=(const DB_GetUserInData& obj) {
		m_lUSN = obj.m_lUSN;
		m_sUID = obj.m_sUID;
		m_lSex = obj.m_lSex;
		m_lJumin = obj.m_lJumin;
		m_lRegion = obj.m_lRegion;
		m_lFirstUSN = obj.m_lFirstUSN;
		return *this;
	}
public:


	void Clear()
	{
		m_lUSN = 0L;
		m_sUID.erase();
		m_lSex = 0L;
		m_lJumin = 0L;
		m_lRegion = 0L;
		m_lFirstUSN = 0L;
	}
};

inline DWORD BSize_DB_GetUserInData(const DB_GetUserInData& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_LONG(obj.m_lUSN);
	dwSize += BSize_xstring(obj.m_sUID);
	dwSize += BSize_LONG(obj.m_lSex);
	dwSize += BSize_LONG(obj.m_lJumin);
	dwSize += BSize_LONG(obj.m_lRegion);
	dwSize += BSize_LONG(obj.m_lFirstUSN);
	return dwSize;
}
inline LPBUF BEncode_DB_GetUserInData(LPBUF pBuf, LPBUFLEN pdwLen, const DB_GetUserInData& obj) {
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lUSN))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sUID))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lSex))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lJumin))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lRegion))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lFirstUSN))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_DB_GetUserInData(LPCBUF pBuf, LPBUFLEN pdwLen, DB_GetUserInData* pObj) {
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lUSN))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sUID))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lSex))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lJumin))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lRegion))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lFirstUSN))) return NULL;
	return pBuf;
}

class DB_SetUserInData {
public:
	LONG m_lUSN;
	LONG m_lFirstUSN;
	LONG m_lLXP;
	LONGLONG m_lCMoney;
	LONG m_lExtMoney;
	LONG m_lCLevel;
	LONG m_lPersonalWin;
	LONG m_lPersonalLose;
	LONG m_lPersonalDraw;
	LONG m_lDisCnt;
	LONG m_lPlayTime;
	LONG m_lGameType;
	LONG m_lRoomID;
	string m_sGameDate;
	LONG m_lReservedInt1;
	LONG m_lReservedInt2;
	LONG m_lReservedInt3;
	LONG m_lReservedInt4;
	LONG m_lReservedInt5;
	LONG m_lReservedInt6;
	LONG m_lReservedInt7;
	LONG m_lReservedInt8;
	LONG m_lReservedInt9;
	LONG m_lReservedInt10;
	LONG m_lReservedInt11;
	LONG m_lReservedInt12;
	LONG m_lReservedInt13;
	LONG m_lReservedInt14;
	LONG m_lReservedInt15;
	LONG m_lReservedInt16;
	LONG m_lReservedInt17;
	LONG m_lReservedInt18;
	LONG m_lReservedInt19;
	LONG m_lReservedInt20;
	LONGLONG m_lReservedLL1;
	LONGLONG m_lReservedLL2;
	LONGLONG m_lReservedLL3;
	LONGLONG m_lReservedLL4;
	LONGLONG m_lReservedLL5;
	LONGLONG m_lReservedLL6;
	LONGLONG m_lReservedLL7;
	LONGLONG m_lReservedLL8;
	LONGLONG m_lReservedLL9;
	LONGLONG m_lReservedLL10;
	xstring m_sReservedString1;
	xstring m_sReservedString2;
	xstring m_sReservedString3;
	xstring m_sReservedString4;
	xstring m_sReservedString5;
	xstring m_sReservedString6;
	xstring m_sReservedString7;
	xstring m_sReservedString8;
	xstring m_sReservedString9;
	xstring m_sReservedString10;
	xstring m_sReservedString11;
	xstring m_sReservedString12;
	xstring m_sReservedString13;
	xstring m_sReservedString14;
	xstring m_sReservedString15;
public:
#ifndef DB_SetUserInData_Ctor // to override default ctor, define this symbol
	DB_SetUserInData() { }
#endif
	DB_SetUserInData(const DB_SetUserInData& obj) { operator=(obj); }
	explicit DB_SetUserInData(const LONG& lUSN, const LONG& lFirstUSN = LONG(), const LONG& lLXP = LONG(), const LONGLONG& lCMoney = LONGLONG(), const LONG& lExtMoney = LONG(), const LONG& lCLevel = LONG(), const LONG& lPersonalWin = LONG(), const LONG& lPersonalLose = LONG(), const LONG& lPersonalDraw = LONG(), const LONG& lDisCnt = LONG(), const LONG& lPlayTime = LONG(), const LONG& lGameType = LONG(), const LONG& lRoomID = LONG(), const string& sGameDate = string(), const LONG& lReservedInt1 = LONG(), const LONG& lReservedInt2 = LONG(), const LONG& lReservedInt3 = LONG(), const LONG& lReservedInt4 = LONG(), const LONG& lReservedInt5 = LONG(), const LONG& lReservedInt6 = LONG(), const LONG& lReservedInt7 = LONG(), const LONG& lReservedInt8 = LONG(), const LONG& lReservedInt9 = LONG(), const LONG& lReservedInt10 = LONG(), const LONG& lReservedInt11 = LONG(), const LONG& lReservedInt12 = LONG(), const LONG& lReservedInt13 = LONG(), const LONG& lReservedInt14 = LONG(), const LONG& lReservedInt15 = LONG(), const LONG& lReservedInt16 = LONG(), const LONG& lReservedInt17 = LONG(), const LONG& lReservedInt18 = LONG(), const LONG& lReservedInt19 = LONG(), const LONG& lReservedInt20 = LONG(), const LONGLONG& lReservedLL1 = LONGLONG(), const LONGLONG& lReservedLL2 = LONGLONG(), const LONGLONG& lReservedLL3 = LONGLONG(), const LONGLONG& lReservedLL4 = LONGLONG(), const LONGLONG& lReservedLL5 = LONGLONG(), const LONGLONG& lReservedLL6 = LONGLONG(), const LONGLONG& lReservedLL7 = LONGLONG(), const LONGLONG& lReservedLL8 = LONGLONG(), const LONGLONG& lReservedLL9 = LONGLONG(), const LONGLONG& lReservedLL10 = LONGLONG(), const xstring& sReservedString1 = xstring(), const xstring& sReservedString2 = xstring(), const xstring& sReservedString3 = xstring(), const xstring& sReservedString4 = xstring(), const xstring& sReservedString5 = xstring(), const xstring& sReservedString6 = xstring(), const xstring& sReservedString7 = xstring(), const xstring& sReservedString8 = xstring(), const xstring& sReservedString9 = xstring(), const xstring& sReservedString10 = xstring(), const xstring& sReservedString11 = xstring(), const xstring& sReservedString12 = xstring(), const xstring& sReservedString13 = xstring(), const xstring& sReservedString14 = xstring(), const xstring& sReservedString15 = xstring()) 
		 : m_lUSN(lUSN), m_lFirstUSN(lFirstUSN), m_lLXP(lLXP), m_lCMoney(lCMoney), m_lExtMoney(lExtMoney), m_lCLevel(lCLevel), m_lPersonalWin(lPersonalWin), m_lPersonalLose(lPersonalLose), m_lPersonalDraw(lPersonalDraw), m_lDisCnt(lDisCnt), m_lPlayTime(lPlayTime), m_lGameType(lGameType), m_lRoomID(lRoomID), m_sGameDate(sGameDate), m_lReservedInt1(lReservedInt1), m_lReservedInt2(lReservedInt2), m_lReservedInt3(lReservedInt3), m_lReservedInt4(lReservedInt4), m_lReservedInt5(lReservedInt5), m_lReservedInt6(lReservedInt6), m_lReservedInt7(lReservedInt7), m_lReservedInt8(lReservedInt8), m_lReservedInt9(lReservedInt9), m_lReservedInt10(lReservedInt10), m_lReservedInt11(lReservedInt11), m_lReservedInt12(lReservedInt12), m_lReservedInt13(lReservedInt13), m_lReservedInt14(lReservedInt14), m_lReservedInt15(lReservedInt15), m_lReservedInt16(lReservedInt16), m_lReservedInt17(lReservedInt17), m_lReservedInt18(lReservedInt18), m_lReservedInt19(lReservedInt19), m_lReservedInt20(lReservedInt20), m_lReservedLL1(lReservedLL1), m_lReservedLL2(lReservedLL2), m_lReservedLL3(lReservedLL3), m_lReservedLL4(lReservedLL4), m_lReservedLL5(lReservedLL5), m_lReservedLL6(lReservedLL6), m_lReservedLL7(lReservedLL7), m_lReservedLL8(lReservedLL8), m_lReservedLL9(lReservedLL9), m_lReservedLL10(lReservedLL10), m_sReservedString1(sReservedString1), m_sReservedString2(sReservedString2), m_sReservedString3(sReservedString3), m_sReservedString4(sReservedString4), m_sReservedString5(sReservedString5), m_sReservedString6(sReservedString6), m_sReservedString7(sReservedString7), m_sReservedString8(sReservedString8), m_sReservedString9(sReservedString9), m_sReservedString10(sReservedString10), m_sReservedString11(sReservedString11), m_sReservedString12(sReservedString12), m_sReservedString13(sReservedString13), m_sReservedString14(sReservedString14), m_sReservedString15(sReservedString15) { }
	DB_SetUserInData& operator=(const DB_SetUserInData& obj) {
		m_lUSN = obj.m_lUSN;
		m_lFirstUSN = obj.m_lFirstUSN;
		m_lLXP = obj.m_lLXP;
		m_lCMoney = obj.m_lCMoney;
		m_lExtMoney = obj.m_lExtMoney;
		m_lCLevel = obj.m_lCLevel;
		m_lPersonalWin = obj.m_lPersonalWin;
		m_lPersonalLose = obj.m_lPersonalLose;
		m_lPersonalDraw = obj.m_lPersonalDraw;
		m_lDisCnt = obj.m_lDisCnt;
		m_lPlayTime = obj.m_lPlayTime;
		m_lGameType = obj.m_lGameType;
		m_lRoomID = obj.m_lRoomID;
		m_sGameDate = obj.m_sGameDate;
		m_lReservedInt1 = obj.m_lReservedInt1;
		m_lReservedInt2 = obj.m_lReservedInt2;
		m_lReservedInt3 = obj.m_lReservedInt3;
		m_lReservedInt4 = obj.m_lReservedInt4;
		m_lReservedInt5 = obj.m_lReservedInt5;
		m_lReservedInt6 = obj.m_lReservedInt6;
		m_lReservedInt7 = obj.m_lReservedInt7;
		m_lReservedInt8 = obj.m_lReservedInt8;
		m_lReservedInt9 = obj.m_lReservedInt9;
		m_lReservedInt10 = obj.m_lReservedInt10;
		m_lReservedInt11 = obj.m_lReservedInt11;
		m_lReservedInt12 = obj.m_lReservedInt12;
		m_lReservedInt13 = obj.m_lReservedInt13;
		m_lReservedInt14 = obj.m_lReservedInt14;
		m_lReservedInt15 = obj.m_lReservedInt15;
		m_lReservedInt16 = obj.m_lReservedInt16;
		m_lReservedInt17 = obj.m_lReservedInt17;
		m_lReservedInt18 = obj.m_lReservedInt18;
		m_lReservedInt19 = obj.m_lReservedInt19;
		m_lReservedInt20 = obj.m_lReservedInt20;
		m_lReservedLL1 = obj.m_lReservedLL1;
		m_lReservedLL2 = obj.m_lReservedLL2;
		m_lReservedLL3 = obj.m_lReservedLL3;
		m_lReservedLL4 = obj.m_lReservedLL4;
		m_lReservedLL5 = obj.m_lReservedLL5;
		m_lReservedLL6 = obj.m_lReservedLL6;
		m_lReservedLL7 = obj.m_lReservedLL7;
		m_lReservedLL8 = obj.m_lReservedLL8;
		m_lReservedLL9 = obj.m_lReservedLL9;
		m_lReservedLL10 = obj.m_lReservedLL10;
		m_sReservedString1 = obj.m_sReservedString1;
		m_sReservedString2 = obj.m_sReservedString2;
		m_sReservedString3 = obj.m_sReservedString3;
		m_sReservedString4 = obj.m_sReservedString4;
		m_sReservedString5 = obj.m_sReservedString5;
		m_sReservedString6 = obj.m_sReservedString6;
		m_sReservedString7 = obj.m_sReservedString7;
		m_sReservedString8 = obj.m_sReservedString8;
		m_sReservedString9 = obj.m_sReservedString9;
		m_sReservedString10 = obj.m_sReservedString10;
		m_sReservedString11 = obj.m_sReservedString11;
		m_sReservedString12 = obj.m_sReservedString12;
		m_sReservedString13 = obj.m_sReservedString13;
		m_sReservedString14 = obj.m_sReservedString14;
		m_sReservedString15 = obj.m_sReservedString15;
		return *this;
	}
public:


	void Clear()
	{
		m_lUSN = 0L;
		m_lFirstUSN = 0L;
		m_lLXP = 0L;           
		m_lCMoney = 0L; 
		m_lExtMoney = 0L;       
		m_lCLevel = 0L;        
		m_lPersonalWin = 0L;   
		m_lPersonalLose = 0L;  
		m_lPersonalDraw = 0L;  
		m_lDisCnt = 0L;
		m_lPlayTime = 0L;
		m_lGameType = 0L;
		m_lRoomID = 0L;
		m_sGameDate.erase();       
		m_lReservedInt1 = 0L;  
		m_lReservedInt2 = 0L;  
		m_lReservedInt3 = 0L;  
		m_lReservedInt4 = 0L;  
		m_lReservedInt5 = 0L;  
		m_lReservedInt6 = 0L;  
		m_lReservedInt7 = 0L;  
		m_lReservedInt8 = 0L;  
		m_lReservedInt9 = 0L;  
		m_lReservedInt10= 0L;
		m_lReservedInt11= 0L;
		m_lReservedInt12= 0L;
		m_lReservedInt13= 0L;
		m_lReservedInt14= 0L;
		m_lReservedInt15= 0L;
		m_lReservedInt16= 0L;
		m_lReservedInt17= 0L;
		m_lReservedInt18= 0L;
		m_lReservedInt19= 0L;
		m_lReservedInt20= 0L;
		m_lReservedLL1 = 0L;  
		m_lReservedLL2 = 0L;  
		m_lReservedLL3 = 0L;
		m_lReservedLL4 = 0L;
		m_lReservedLL5 = 0L;
		m_lReservedLL6 = 0L;
		m_lReservedLL7 = 0L;
		m_lReservedLL8 = 0L;
		m_lReservedLL9 = 0L;
		m_lReservedLL10 = 0L;
		m_sReservedString1.erase();
		m_sReservedString2.erase();
		m_sReservedString3.erase();
		m_sReservedString4.erase();
		m_sReservedString5.erase();
		m_sReservedString6.erase();
		m_sReservedString7.erase();
		m_sReservedString8.erase();
		m_sReservedString9.erase();
		m_sReservedString10.erase();
		m_sReservedString11.erase();
		m_sReservedString12.erase();
		m_sReservedString13.erase();
		m_sReservedString14.erase();
		m_sReservedString15.erase();
	}
};

inline DWORD BSize_DB_SetUserInData(const DB_SetUserInData& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_LONG(obj.m_lUSN);
	dwSize += BSize_LONG(obj.m_lFirstUSN);
	dwSize += BSize_LONG(obj.m_lLXP);
	dwSize += BSize_LONGLONG(obj.m_lCMoney);
	dwSize += BSize_LONG(obj.m_lExtMoney);
	dwSize += BSize_LONG(obj.m_lCLevel);
	dwSize += BSize_LONG(obj.m_lPersonalWin);
	dwSize += BSize_LONG(obj.m_lPersonalLose);
	dwSize += BSize_LONG(obj.m_lPersonalDraw);
	dwSize += BSize_LONG(obj.m_lDisCnt);
	dwSize += BSize_LONG(obj.m_lPlayTime);
	dwSize += BSize_LONG(obj.m_lGameType);
	dwSize += BSize_LONG(obj.m_lRoomID);
	dwSize += BSize_string(obj.m_sGameDate);
	dwSize += BSize_LONG(obj.m_lReservedInt1);
	dwSize += BSize_LONG(obj.m_lReservedInt2);
	dwSize += BSize_LONG(obj.m_lReservedInt3);
	dwSize += BSize_LONG(obj.m_lReservedInt4);
	dwSize += BSize_LONG(obj.m_lReservedInt5);
	dwSize += BSize_LONG(obj.m_lReservedInt6);
	dwSize += BSize_LONG(obj.m_lReservedInt7);
	dwSize += BSize_LONG(obj.m_lReservedInt8);
	dwSize += BSize_LONG(obj.m_lReservedInt9);
	dwSize += BSize_LONG(obj.m_lReservedInt10);
	dwSize += BSize_LONG(obj.m_lReservedInt11);
	dwSize += BSize_LONG(obj.m_lReservedInt12);
	dwSize += BSize_LONG(obj.m_lReservedInt13);
	dwSize += BSize_LONG(obj.m_lReservedInt14);
	dwSize += BSize_LONG(obj.m_lReservedInt15);
	dwSize += BSize_LONG(obj.m_lReservedInt16);
	dwSize += BSize_LONG(obj.m_lReservedInt17);
	dwSize += BSize_LONG(obj.m_lReservedInt18);
	dwSize += BSize_LONG(obj.m_lReservedInt19);
	dwSize += BSize_LONG(obj.m_lReservedInt20);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL1);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL2);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL3);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL4);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL5);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL6);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL7);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL8);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL9);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL10);
	dwSize += BSize_xstring(obj.m_sReservedString1);
	dwSize += BSize_xstring(obj.m_sReservedString2);
	dwSize += BSize_xstring(obj.m_sReservedString3);
	dwSize += BSize_xstring(obj.m_sReservedString4);
	dwSize += BSize_xstring(obj.m_sReservedString5);
	dwSize += BSize_xstring(obj.m_sReservedString6);
	dwSize += BSize_xstring(obj.m_sReservedString7);
	dwSize += BSize_xstring(obj.m_sReservedString8);
	dwSize += BSize_xstring(obj.m_sReservedString9);
	dwSize += BSize_xstring(obj.m_sReservedString10);
	dwSize += BSize_xstring(obj.m_sReservedString11);
	dwSize += BSize_xstring(obj.m_sReservedString12);
	dwSize += BSize_xstring(obj.m_sReservedString13);
	dwSize += BSize_xstring(obj.m_sReservedString14);
	dwSize += BSize_xstring(obj.m_sReservedString15);
	return dwSize;
}
inline LPBUF BEncode_DB_SetUserInData(LPBUF pBuf, LPBUFLEN pdwLen, const DB_SetUserInData& obj) {
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lUSN))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lFirstUSN))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lLXP))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lCMoney))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lExtMoney))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lCLevel))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalWin))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalLose))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalDraw))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lDisCnt))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPlayTime))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lGameType))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lRoomID))) return NULL;
	if(!(pBuf = BEncode_string(pBuf, pdwLen, obj.m_sGameDate))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt1))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt2))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt3))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt4))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt5))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt6))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt7))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt8))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt9))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt10))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt11))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt12))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt13))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt14))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt15))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt16))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt17))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt18))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt19))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt20))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL1))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL2))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL3))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL4))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL5))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL6))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL7))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL8))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL9))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL10))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString1))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString2))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString3))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString4))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString5))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString6))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString7))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString8))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString9))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString10))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString11))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString12))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString13))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString14))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString15))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_DB_SetUserInData(LPCBUF pBuf, LPBUFLEN pdwLen, DB_SetUserInData* pObj) {
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lUSN))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lFirstUSN))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lLXP))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lCMoney))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lExtMoney))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lCLevel))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalWin))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalLose))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalDraw))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lDisCnt))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPlayTime))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lGameType))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lRoomID))) return NULL;
	if(!(pBuf = BDecode_string(pBuf, pdwLen, &pObj->m_sGameDate))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt1))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt2))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt3))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt4))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt5))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt6))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt7))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt8))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt9))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt10))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt11))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt12))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt13))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt14))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt15))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt16))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt17))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt18))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt19))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt20))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL1))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL2))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL3))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL4))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL5))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL6))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL7))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL8))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL9))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL10))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString1))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString2))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString3))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString4))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString5))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString6))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString7))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString8))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString9))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString10))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString11))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString12))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString13))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString14))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString15))) return NULL;
	return pBuf;
}

class DB_SetUserOutData {
public:
	LONG m_lUSN;
	LONG m_lLXP;
	LONGLONG m_lCMoney;
	LONG m_lExtMoney;
	LONG m_lCLevel;
	LONG m_lPersonalWin;
	LONG m_lPersonalLose;
	LONG m_lPersonalDraw;
	LONG m_lDisCnt;
	LONG m_lGuildID1;
	LONG m_lReservedInt1;
	LONG m_lReservedInt2;
	LONG m_lReservedInt3;
	LONG m_lReservedInt4;
	LONG m_lReservedInt5;
	LONG m_lReservedInt6;
	LONG m_lReservedInt7;
	LONG m_lReservedInt8;
	LONG m_lReservedInt9;
	LONG m_lReservedInt10;
	LONG m_lReservedInt11;
	LONG m_lReservedInt12;
	LONG m_lReservedInt13;
	LONG m_lReservedInt14;
	LONG m_lReservedInt15;
	LONG m_lReservedInt16;
	LONG m_lReservedInt17;
	LONG m_lReservedInt18;
	LONG m_lReservedInt19;
	LONG m_lReservedInt20;
	LONGLONG m_lReservedLL1;
	LONGLONG m_lReservedLL2;
	LONGLONG m_lReservedLL3;
	LONGLONG m_lReservedLL4;
	LONGLONG m_lReservedLL5;
	LONGLONG m_lReservedLL6;
	LONGLONG m_lReservedLL7;
	LONGLONG m_lReservedLL8;
	LONGLONG m_lReservedLL9;
	LONGLONG m_lReservedLL10;
	xstring m_sReservedString1;
	xstring m_sReservedString2;
	xstring m_sReservedString3;
	xstring m_sReservedString4;
	xstring m_sReservedString5;
	xstring m_sReservedString6;
	xstring m_sReservedString7;
	xstring m_sReservedString8;
	xstring m_sReservedString9;
	xstring m_sReservedString10;
	xstring m_sReservedString11;
	xstring m_sReservedString12;
	xstring m_sReservedString13;
	xstring m_sReservedString14;
	xstring m_sReservedString15;
	LONG m_lResult;
public:
#ifndef DB_SetUserOutData_Ctor // to override default ctor, define this symbol
	DB_SetUserOutData() { }
#endif
	DB_SetUserOutData(const DB_SetUserOutData& obj) { operator=(obj); }
	explicit DB_SetUserOutData(const LONG& lUSN, const LONG& lLXP = LONG(), const LONGLONG& lCMoney = LONGLONG(), const LONG& lExtMoney = LONG(), const LONG& lCLevel = LONG(), const LONG& lPersonalWin = LONG(), const LONG& lPersonalLose = LONG(), const LONG& lPersonalDraw = LONG(), const LONG& lDisCnt = LONG(), const LONG& lGuildID1 = LONG(), const LONG& lReservedInt1 = LONG(), const LONG& lReservedInt2 = LONG(), const LONG& lReservedInt3 = LONG(), const LONG& lReservedInt4 = LONG(), const LONG& lReservedInt5 = LONG(), const LONG& lReservedInt6 = LONG(), const LONG& lReservedInt7 = LONG(), const LONG& lReservedInt8 = LONG(), const LONG& lReservedInt9 = LONG(), const LONG& lReservedInt10 = LONG(), const LONG& lReservedInt11 = LONG(), const LONG& lReservedInt12 = LONG(), const LONG& lReservedInt13 = LONG(), const LONG& lReservedInt14 = LONG(), const LONG& lReservedInt15 = LONG(), const LONG& lReservedInt16 = LONG(), const LONG& lReservedInt17 = LONG(), const LONG& lReservedInt18 = LONG(), const LONG& lReservedInt19 = LONG(), const LONG& lReservedInt20 = LONG(), const LONGLONG& lReservedLL1 = LONGLONG(), const LONGLONG& lReservedLL2 = LONGLONG(), const LONGLONG& lReservedLL3 = LONGLONG(), const LONGLONG& lReservedLL4 = LONGLONG(), const LONGLONG& lReservedLL5 = LONGLONG(), const LONGLONG& lReservedLL6 = LONGLONG(), const LONGLONG& lReservedLL7 = LONGLONG(), const LONGLONG& lReservedLL8 = LONGLONG(), const LONGLONG& lReservedLL9 = LONGLONG(), const LONGLONG& lReservedLL10 = LONGLONG(), const xstring& sReservedString1 = xstring(), const xstring& sReservedString2 = xstring(), const xstring& sReservedString3 = xstring(), const xstring& sReservedString4 = xstring(), const xstring& sReservedString5 = xstring(), const xstring& sReservedString6 = xstring(), const xstring& sReservedString7 = xstring(), const xstring& sReservedString8 = xstring(), const xstring& sReservedString9 = xstring(), const xstring& sReservedString10 = xstring(), const xstring& sReservedString11 = xstring(), const xstring& sReservedString12 = xstring(), const xstring& sReservedString13 = xstring(), const xstring& sReservedString14 = xstring(), const xstring& sReservedString15 = xstring(), const LONG& lResult = LONG()) 
		 : m_lUSN(lUSN), m_lLXP(lLXP), m_lCMoney(lCMoney), m_lExtMoney(lExtMoney), m_lCLevel(lCLevel), m_lPersonalWin(lPersonalWin), m_lPersonalLose(lPersonalLose), m_lPersonalDraw(lPersonalDraw), m_lDisCnt(lDisCnt), m_lGuildID1(lGuildID1), m_lReservedInt1(lReservedInt1), m_lReservedInt2(lReservedInt2), m_lReservedInt3(lReservedInt3), m_lReservedInt4(lReservedInt4), m_lReservedInt5(lReservedInt5), m_lReservedInt6(lReservedInt6), m_lReservedInt7(lReservedInt7), m_lReservedInt8(lReservedInt8), m_lReservedInt9(lReservedInt9), m_lReservedInt10(lReservedInt10), m_lReservedInt11(lReservedInt11), m_lReservedInt12(lReservedInt12), m_lReservedInt13(lReservedInt13), m_lReservedInt14(lReservedInt14), m_lReservedInt15(lReservedInt15), m_lReservedInt16(lReservedInt16), m_lReservedInt17(lReservedInt17), m_lReservedInt18(lReservedInt18), m_lReservedInt19(lReservedInt19), m_lReservedInt20(lReservedInt20), m_lReservedLL1(lReservedLL1), m_lReservedLL2(lReservedLL2), m_lReservedLL3(lReservedLL3), m_lReservedLL4(lReservedLL4), m_lReservedLL5(lReservedLL5), m_lReservedLL6(lReservedLL6), m_lReservedLL7(lReservedLL7), m_lReservedLL8(lReservedLL8), m_lReservedLL9(lReservedLL9), m_lReservedLL10(lReservedLL10), m_sReservedString1(sReservedString1), m_sReservedString2(sReservedString2), m_sReservedString3(sReservedString3), m_sReservedString4(sReservedString4), m_sReservedString5(sReservedString5), m_sReservedString6(sReservedString6), m_sReservedString7(sReservedString7), m_sReservedString8(sReservedString8), m_sReservedString9(sReservedString9), m_sReservedString10(sReservedString10), m_sReservedString11(sReservedString11), m_sReservedString12(sReservedString12), m_sReservedString13(sReservedString13), m_sReservedString14(sReservedString14), m_sReservedString15(sReservedString15), m_lResult(lResult) { }
	DB_SetUserOutData& operator=(const DB_SetUserOutData& obj) {
		m_lUSN = obj.m_lUSN;
		m_lLXP = obj.m_lLXP;
		m_lCMoney = obj.m_lCMoney;
		m_lExtMoney = obj.m_lExtMoney;
		m_lCLevel = obj.m_lCLevel;
		m_lPersonalWin = obj.m_lPersonalWin;
		m_lPersonalLose = obj.m_lPersonalLose;
		m_lPersonalDraw = obj.m_lPersonalDraw;
		m_lDisCnt = obj.m_lDisCnt;
		m_lGuildID1 = obj.m_lGuildID1;
		m_lReservedInt1 = obj.m_lReservedInt1;
		m_lReservedInt2 = obj.m_lReservedInt2;
		m_lReservedInt3 = obj.m_lReservedInt3;
		m_lReservedInt4 = obj.m_lReservedInt4;
		m_lReservedInt5 = obj.m_lReservedInt5;
		m_lReservedInt6 = obj.m_lReservedInt6;
		m_lReservedInt7 = obj.m_lReservedInt7;
		m_lReservedInt8 = obj.m_lReservedInt8;
		m_lReservedInt9 = obj.m_lReservedInt9;
		m_lReservedInt10 = obj.m_lReservedInt10;
		m_lReservedInt11 = obj.m_lReservedInt11;
		m_lReservedInt12 = obj.m_lReservedInt12;
		m_lReservedInt13 = obj.m_lReservedInt13;
		m_lReservedInt14 = obj.m_lReservedInt14;
		m_lReservedInt15 = obj.m_lReservedInt15;
		m_lReservedInt16 = obj.m_lReservedInt16;
		m_lReservedInt17 = obj.m_lReservedInt17;
		m_lReservedInt18 = obj.m_lReservedInt18;
		m_lReservedInt19 = obj.m_lReservedInt19;
		m_lReservedInt20 = obj.m_lReservedInt20;
		m_lReservedLL1 = obj.m_lReservedLL1;
		m_lReservedLL2 = obj.m_lReservedLL2;
		m_lReservedLL3 = obj.m_lReservedLL3;
		m_lReservedLL4 = obj.m_lReservedLL4;
		m_lReservedLL5 = obj.m_lReservedLL5;
		m_lReservedLL6 = obj.m_lReservedLL6;
		m_lReservedLL7 = obj.m_lReservedLL7;
		m_lReservedLL8 = obj.m_lReservedLL8;
		m_lReservedLL9 = obj.m_lReservedLL9;
		m_lReservedLL10 = obj.m_lReservedLL10;
		m_sReservedString1 = obj.m_sReservedString1;
		m_sReservedString2 = obj.m_sReservedString2;
		m_sReservedString3 = obj.m_sReservedString3;
		m_sReservedString4 = obj.m_sReservedString4;
		m_sReservedString5 = obj.m_sReservedString5;
		m_sReservedString6 = obj.m_sReservedString6;
		m_sReservedString7 = obj.m_sReservedString7;
		m_sReservedString8 = obj.m_sReservedString8;
		m_sReservedString9 = obj.m_sReservedString9;
		m_sReservedString10 = obj.m_sReservedString10;
		m_sReservedString11 = obj.m_sReservedString11;
		m_sReservedString12 = obj.m_sReservedString12;
		m_sReservedString13 = obj.m_sReservedString13;
		m_sReservedString14 = obj.m_sReservedString14;
		m_sReservedString15 = obj.m_sReservedString15;
		m_lResult = obj.m_lResult;
		return *this;
	}
public:


	void Clear()
	{
		m_lUSN = 0L;
		m_lLXP = 0L;           
		m_lCMoney = 0L; 
		m_lExtMoney = 0L;    
		m_lCLevel = 0L;        
		m_lPersonalWin = 0L;   
		m_lPersonalLose = 0L;  
		m_lPersonalDraw = 0L;  
		m_lDisCnt = 0L;        
		m_lGuildID1 = 0L;      
		m_lReservedInt1 = 0L;  
		m_lReservedInt2 = 0L;  
		m_lReservedInt3 = 0L;  
		m_lReservedInt4 = 0L;  
		m_lReservedInt5 = 0L;  
		m_lReservedInt6 = 0L;  
		m_lReservedInt7 = 0L; 
		m_lReservedInt8 = 0L;  
		m_lReservedInt9 = 0L;  
		m_lReservedInt10= 0L;
		m_lReservedInt11= 0L;
		m_lReservedInt12= 0L;
		m_lReservedInt13= 0L;
		m_lReservedInt14= 0L;
		m_lReservedInt15= 0L;
		m_lReservedInt16= 0L;
		m_lReservedInt17= 0L;
		m_lReservedInt18= 0L;
		m_lReservedInt19= 0L;
		m_lReservedInt20= 0L;
		m_lReservedLL1 = 0L;  
		m_lReservedLL2 = 0L;  
		m_lReservedLL3 = 0L;
		m_lReservedLL4 = 0L;
		m_lReservedLL5 = 0L;
		m_lReservedLL6 = 0L;
		m_lReservedLL7 = 0L;
		m_lReservedLL8 = 0L;
		m_lReservedLL9 = 0L;
		m_lReservedLL10 = 0L;
		m_sReservedString1.erase();
		m_sReservedString2.erase();
		m_sReservedString3.erase();
		m_sReservedString4.erase();
		m_sReservedString5.erase();
		m_sReservedString6.erase();
		m_sReservedString7.erase();
		m_sReservedString8.erase();
		m_sReservedString9.erase();
		m_sReservedString10.erase();
		m_sReservedString11.erase();
		m_sReservedString12.erase();
		m_sReservedString13.erase();
		m_sReservedString14.erase();
		m_sReservedString15.erase();
		m_lResult = 0L;
	}
};

inline DWORD BSize_DB_SetUserOutData(const DB_SetUserOutData& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_LONG(obj.m_lUSN);
	dwSize += BSize_LONG(obj.m_lLXP);
	dwSize += BSize_LONGLONG(obj.m_lCMoney);
	dwSize += BSize_LONG(obj.m_lExtMoney);
	dwSize += BSize_LONG(obj.m_lCLevel);
	dwSize += BSize_LONG(obj.m_lPersonalWin);
	dwSize += BSize_LONG(obj.m_lPersonalLose);
	dwSize += BSize_LONG(obj.m_lPersonalDraw);
	dwSize += BSize_LONG(obj.m_lDisCnt);
	dwSize += BSize_LONG(obj.m_lGuildID1);
	dwSize += BSize_LONG(obj.m_lReservedInt1);
	dwSize += BSize_LONG(obj.m_lReservedInt2);
	dwSize += BSize_LONG(obj.m_lReservedInt3);
	dwSize += BSize_LONG(obj.m_lReservedInt4);
	dwSize += BSize_LONG(obj.m_lReservedInt5);
	dwSize += BSize_LONG(obj.m_lReservedInt6);
	dwSize += BSize_LONG(obj.m_lReservedInt7);
	dwSize += BSize_LONG(obj.m_lReservedInt8);
	dwSize += BSize_LONG(obj.m_lReservedInt9);
	dwSize += BSize_LONG(obj.m_lReservedInt10);
	dwSize += BSize_LONG(obj.m_lReservedInt11);
	dwSize += BSize_LONG(obj.m_lReservedInt12);
	dwSize += BSize_LONG(obj.m_lReservedInt13);
	dwSize += BSize_LONG(obj.m_lReservedInt14);
	dwSize += BSize_LONG(obj.m_lReservedInt15);
	dwSize += BSize_LONG(obj.m_lReservedInt16);
	dwSize += BSize_LONG(obj.m_lReservedInt17);
	dwSize += BSize_LONG(obj.m_lReservedInt18);
	dwSize += BSize_LONG(obj.m_lReservedInt19);
	dwSize += BSize_LONG(obj.m_lReservedInt20);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL1);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL2);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL3);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL4);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL5);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL6);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL7);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL8);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL9);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL10);
	dwSize += BSize_xstring(obj.m_sReservedString1);
	dwSize += BSize_xstring(obj.m_sReservedString2);
	dwSize += BSize_xstring(obj.m_sReservedString3);
	dwSize += BSize_xstring(obj.m_sReservedString4);
	dwSize += BSize_xstring(obj.m_sReservedString5);
	dwSize += BSize_xstring(obj.m_sReservedString6);
	dwSize += BSize_xstring(obj.m_sReservedString7);
	dwSize += BSize_xstring(obj.m_sReservedString8);
	dwSize += BSize_xstring(obj.m_sReservedString9);
	dwSize += BSize_xstring(obj.m_sReservedString10);
	dwSize += BSize_xstring(obj.m_sReservedString11);
	dwSize += BSize_xstring(obj.m_sReservedString12);
	dwSize += BSize_xstring(obj.m_sReservedString13);
	dwSize += BSize_xstring(obj.m_sReservedString14);
	dwSize += BSize_xstring(obj.m_sReservedString15);
	dwSize += BSize_LONG(obj.m_lResult);
	return dwSize;
}
inline LPBUF BEncode_DB_SetUserOutData(LPBUF pBuf, LPBUFLEN pdwLen, const DB_SetUserOutData& obj) {
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lUSN))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lLXP))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lCMoney))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lExtMoney))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lCLevel))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalWin))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalLose))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalDraw))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lDisCnt))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lGuildID1))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt1))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt2))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt3))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt4))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt5))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt6))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt7))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt8))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt9))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt10))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt11))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt12))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt13))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt14))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt15))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt16))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt17))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt18))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt19))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt20))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL1))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL2))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL3))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL4))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL5))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL6))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL7))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL8))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL9))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL10))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString1))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString2))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString3))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString4))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString5))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString6))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString7))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString8))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString9))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString10))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString11))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString12))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString13))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString14))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString15))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lResult))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_DB_SetUserOutData(LPCBUF pBuf, LPBUFLEN pdwLen, DB_SetUserOutData* pObj) {
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lUSN))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lLXP))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lCMoney))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lExtMoney))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lCLevel))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalWin))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalLose))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalDraw))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lDisCnt))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lGuildID1))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt1))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt2))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt3))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt4))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt5))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt6))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt7))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt8))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt9))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt10))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt11))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt12))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt13))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt14))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt15))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt16))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt17))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt18))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt19))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt20))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL1))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL2))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL3))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL4))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL5))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL6))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL7))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL8))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL9))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL10))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString1))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString2))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString3))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString4))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString5))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString6))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString7))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString8))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString9))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString10))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString11))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString12))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString13))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString14))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString15))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lResult))) return NULL;
	return pBuf;
}

class DB_GetUserOutData {
public:
	LONG m_lUSN;
	LONG m_lLXP;
	LONGLONG m_lCMoney;
	LONG m_lExtMoney;
	LONG m_lCLevel;
	LONG m_lPersonalWin;
	LONG m_lPersonalLose;
	LONG m_lPersonalDraw;
	LONG m_lDisCnt;
	LONG m_lGuildID1;
	LONG m_lReservedInt1;
	LONG m_lReservedInt2;
	LONG m_lReservedInt3;
	LONG m_lReservedInt4;
	LONG m_lReservedInt5;
	LONG m_lReservedInt6;
	LONG m_lReservedInt7;
	LONG m_lReservedInt8;
	LONG m_lReservedInt9;
	LONG m_lReservedInt10;
	LONG m_lReservedInt11;
	LONG m_lReservedInt12;
	LONG m_lReservedInt13;
	LONG m_lReservedInt14;
	LONG m_lReservedInt15;
	LONG m_lReservedInt16;
	LONG m_lReservedInt17;
	LONG m_lReservedInt18;
	LONG m_lReservedInt19;
	LONG m_lReservedInt20;
	LONGLONG m_lReservedLL1;
	LONGLONG m_lReservedLL2;
	LONGLONG m_lReservedLL3;
	LONGLONG m_lReservedLL4;
	LONGLONG m_lReservedLL5;
	LONGLONG m_lReservedLL6;
	LONGLONG m_lReservedLL7;
	LONGLONG m_lReservedLL8;
	LONGLONG m_lReservedLL9;
	LONGLONG m_lReservedLL10;
	xstring m_sReservedString1;
	xstring m_sReservedString2;
	xstring m_sReservedString3;
	xstring m_sReservedString4;
	xstring m_sReservedString5;
	xstring m_sReservedString6;
	xstring m_sReservedString7;
	xstring m_sReservedString8;
	xstring m_sReservedString9;
	xstring m_sReservedString10;
	xstring m_sReservedString11;
	xstring m_sReservedString12;
	xstring m_sReservedString13;
	xstring m_sReservedString14;
	xstring m_sReservedString15;
public:
#ifndef DB_GetUserOutData_Ctor // to override default ctor, define this symbol
	DB_GetUserOutData() { }
#endif
	DB_GetUserOutData(const DB_GetUserOutData& obj) { operator=(obj); }
	explicit DB_GetUserOutData(const LONG& lUSN, const LONG& lLXP = LONG(), const LONGLONG& lCMoney = LONGLONG(), const LONG& lExtMoney = LONG(), const LONG& lCLevel = LONG(), const LONG& lPersonalWin = LONG(), const LONG& lPersonalLose = LONG(), const LONG& lPersonalDraw = LONG(), const LONG& lDisCnt = LONG(), const LONG& lGuildID1 = LONG(), const LONG& lReservedInt1 = LONG(), const LONG& lReservedInt2 = LONG(), const LONG& lReservedInt3 = LONG(), const LONG& lReservedInt4 = LONG(), const LONG& lReservedInt5 = LONG(), const LONG& lReservedInt6 = LONG(), const LONG& lReservedInt7 = LONG(), const LONG& lReservedInt8 = LONG(), const LONG& lReservedInt9 = LONG(), const LONG& lReservedInt10 = LONG(), const LONG& lReservedInt11 = LONG(), const LONG& lReservedInt12 = LONG(), const LONG& lReservedInt13 = LONG(), const LONG& lReservedInt14 = LONG(), const LONG& lReservedInt15 = LONG(), const LONG& lReservedInt16 = LONG(), const LONG& lReservedInt17 = LONG(), const LONG& lReservedInt18 = LONG(), const LONG& lReservedInt19 = LONG(), const LONG& lReservedInt20 = LONG(), const LONGLONG& lReservedLL1 = LONGLONG(), const LONGLONG& lReservedLL2 = LONGLONG(), const LONGLONG& lReservedLL3 = LONGLONG(), const LONGLONG& lReservedLL4 = LONGLONG(), const LONGLONG& lReservedLL5 = LONGLONG(), const LONGLONG& lReservedLL6 = LONGLONG(), const LONGLONG& lReservedLL7 = LONGLONG(), const LONGLONG& lReservedLL8 = LONGLONG(), const LONGLONG& lReservedLL9 = LONGLONG(), const LONGLONG& lReservedLL10 = LONGLONG(), const xstring& sReservedString1 = xstring(), const xstring& sReservedString2 = xstring(), const xstring& sReservedString3 = xstring(), const xstring& sReservedString4 = xstring(), const xstring& sReservedString5 = xstring(), const xstring& sReservedString6 = xstring(), const xstring& sReservedString7 = xstring(), const xstring& sReservedString8 = xstring(), const xstring& sReservedString9 = xstring(), const xstring& sReservedString10 = xstring(), const xstring& sReservedString11 = xstring(), const xstring& sReservedString12 = xstring(), const xstring& sReservedString13 = xstring(), const xstring& sReservedString14 = xstring(), const xstring& sReservedString15 = xstring()) 
		 : m_lUSN(lUSN), m_lLXP(lLXP), m_lCMoney(lCMoney), m_lExtMoney(lExtMoney), m_lCLevel(lCLevel), m_lPersonalWin(lPersonalWin), m_lPersonalLose(lPersonalLose), m_lPersonalDraw(lPersonalDraw), m_lDisCnt(lDisCnt), m_lGuildID1(lGuildID1), m_lReservedInt1(lReservedInt1), m_lReservedInt2(lReservedInt2), m_lReservedInt3(lReservedInt3), m_lReservedInt4(lReservedInt4), m_lReservedInt5(lReservedInt5), m_lReservedInt6(lReservedInt6), m_lReservedInt7(lReservedInt7), m_lReservedInt8(lReservedInt8), m_lReservedInt9(lReservedInt9), m_lReservedInt10(lReservedInt10), m_lReservedInt11(lReservedInt11), m_lReservedInt12(lReservedInt12), m_lReservedInt13(lReservedInt13), m_lReservedInt14(lReservedInt14), m_lReservedInt15(lReservedInt15), m_lReservedInt16(lReservedInt16), m_lReservedInt17(lReservedInt17), m_lReservedInt18(lReservedInt18), m_lReservedInt19(lReservedInt19), m_lReservedInt20(lReservedInt20), m_lReservedLL1(lReservedLL1), m_lReservedLL2(lReservedLL2), m_lReservedLL3(lReservedLL3), m_lReservedLL4(lReservedLL4), m_lReservedLL5(lReservedLL5), m_lReservedLL6(lReservedLL6), m_lReservedLL7(lReservedLL7), m_lReservedLL8(lReservedLL8), m_lReservedLL9(lReservedLL9), m_lReservedLL10(lReservedLL10), m_sReservedString1(sReservedString1), m_sReservedString2(sReservedString2), m_sReservedString3(sReservedString3), m_sReservedString4(sReservedString4), m_sReservedString5(sReservedString5), m_sReservedString6(sReservedString6), m_sReservedString7(sReservedString7), m_sReservedString8(sReservedString8), m_sReservedString9(sReservedString9), m_sReservedString10(sReservedString10), m_sReservedString11(sReservedString11), m_sReservedString12(sReservedString12), m_sReservedString13(sReservedString13), m_sReservedString14(sReservedString14), m_sReservedString15(sReservedString15) { }
	DB_GetUserOutData& operator=(const DB_GetUserOutData& obj) {
		m_lUSN = obj.m_lUSN;
		m_lLXP = obj.m_lLXP;
		m_lCMoney = obj.m_lCMoney;
		m_lExtMoney = obj.m_lExtMoney;
		m_lCLevel = obj.m_lCLevel;
		m_lPersonalWin = obj.m_lPersonalWin;
		m_lPersonalLose = obj.m_lPersonalLose;
		m_lPersonalDraw = obj.m_lPersonalDraw;
		m_lDisCnt = obj.m_lDisCnt;
		m_lGuildID1 = obj.m_lGuildID1;
		m_lReservedInt1 = obj.m_lReservedInt1;
		m_lReservedInt2 = obj.m_lReservedInt2;
		m_lReservedInt3 = obj.m_lReservedInt3;
		m_lReservedInt4 = obj.m_lReservedInt4;
		m_lReservedInt5 = obj.m_lReservedInt5;
		m_lReservedInt6 = obj.m_lReservedInt6;
		m_lReservedInt7 = obj.m_lReservedInt7;
		m_lReservedInt8 = obj.m_lReservedInt8;
		m_lReservedInt9 = obj.m_lReservedInt9;
		m_lReservedInt10 = obj.m_lReservedInt10;
		m_lReservedInt11 = obj.m_lReservedInt11;
		m_lReservedInt12 = obj.m_lReservedInt12;
		m_lReservedInt13 = obj.m_lReservedInt13;
		m_lReservedInt14 = obj.m_lReservedInt14;
		m_lReservedInt15 = obj.m_lReservedInt15;
		m_lReservedInt16 = obj.m_lReservedInt16;
		m_lReservedInt17 = obj.m_lReservedInt17;
		m_lReservedInt18 = obj.m_lReservedInt18;
		m_lReservedInt19 = obj.m_lReservedInt19;
		m_lReservedInt20 = obj.m_lReservedInt20;
		m_lReservedLL1 = obj.m_lReservedLL1;
		m_lReservedLL2 = obj.m_lReservedLL2;
		m_lReservedLL3 = obj.m_lReservedLL3;
		m_lReservedLL4 = obj.m_lReservedLL4;
		m_lReservedLL5 = obj.m_lReservedLL5;
		m_lReservedLL6 = obj.m_lReservedLL6;
		m_lReservedLL7 = obj.m_lReservedLL7;
		m_lReservedLL8 = obj.m_lReservedLL8;
		m_lReservedLL9 = obj.m_lReservedLL9;
		m_lReservedLL10 = obj.m_lReservedLL10;
		m_sReservedString1 = obj.m_sReservedString1;
		m_sReservedString2 = obj.m_sReservedString2;
		m_sReservedString3 = obj.m_sReservedString3;
		m_sReservedString4 = obj.m_sReservedString4;
		m_sReservedString5 = obj.m_sReservedString5;
		m_sReservedString6 = obj.m_sReservedString6;
		m_sReservedString7 = obj.m_sReservedString7;
		m_sReservedString8 = obj.m_sReservedString8;
		m_sReservedString9 = obj.m_sReservedString9;
		m_sReservedString10 = obj.m_sReservedString10;
		m_sReservedString11 = obj.m_sReservedString11;
		m_sReservedString12 = obj.m_sReservedString12;
		m_sReservedString13 = obj.m_sReservedString13;
		m_sReservedString14 = obj.m_sReservedString14;
		m_sReservedString15 = obj.m_sReservedString15;
		return *this;
	}
public:


	void Clear()
	{
		m_lUSN = 0L;
		m_lLXP = 0L;           
		m_lCMoney = 0L;     
		m_lExtMoney = 0L;
		m_lCLevel = 0L;        
		m_lPersonalWin = 0L;   
		m_lPersonalLose = 0L;  
		m_lPersonalDraw = 0L;  
		m_lDisCnt = 0L;        
		m_lGuildID1 = 0L;      
		m_lReservedInt1 = 0L;  
		m_lReservedInt2 = 0L;  
		m_lReservedInt3 = 0L;  
		m_lReservedInt4 = 0L;  
		m_lReservedInt5 = 0L;  
		m_lReservedInt6 = 0L;  
		m_lReservedInt7 = 0L;  
		m_lReservedInt8 = 0L;  
		m_lReservedInt9 = 0L;  
		m_lReservedInt10= 0L;
		m_lReservedInt11= 0L;
		m_lReservedInt12= 0L;
		m_lReservedInt13= 0L;
		m_lReservedInt14= 0L;
		m_lReservedInt15= 0L;
		m_lReservedInt16= 0L;
		m_lReservedInt17= 0L;
		m_lReservedInt18= 0L;
		m_lReservedInt19= 0L;
		m_lReservedInt20= 0L;
		m_lReservedLL1 = 0L;  
		m_lReservedLL2 = 0L;  
		m_lReservedLL3 = 0L;
		m_lReservedLL4 = 0L;
		m_lReservedLL5 = 0L;
		m_lReservedLL6 = 0L;
		m_lReservedLL7 = 0L;
		m_lReservedLL8 = 0L;
		m_lReservedLL9 = 0L;
		m_lReservedLL10 = 0L;
		m_sReservedString1.erase();
		m_sReservedString2.erase();
		m_sReservedString3.erase();
		m_sReservedString4.erase();
		m_sReservedString5.erase();
		m_sReservedString6.erase();
		m_sReservedString7.erase();
		m_sReservedString8.erase();
		m_sReservedString9.erase();
		m_sReservedString10.erase();
		m_sReservedString11.erase();
		m_sReservedString12.erase();
		m_sReservedString13.erase();
		m_sReservedString14.erase();
		m_sReservedString15.erase();
	}
};

inline DWORD BSize_DB_GetUserOutData(const DB_GetUserOutData& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_LONG(obj.m_lUSN);
	dwSize += BSize_LONG(obj.m_lLXP);
	dwSize += BSize_LONGLONG(obj.m_lCMoney);
	dwSize += BSize_LONG(obj.m_lExtMoney);
	dwSize += BSize_LONG(obj.m_lCLevel);
	dwSize += BSize_LONG(obj.m_lPersonalWin);
	dwSize += BSize_LONG(obj.m_lPersonalLose);
	dwSize += BSize_LONG(obj.m_lPersonalDraw);
	dwSize += BSize_LONG(obj.m_lDisCnt);
	dwSize += BSize_LONG(obj.m_lGuildID1);
	dwSize += BSize_LONG(obj.m_lReservedInt1);
	dwSize += BSize_LONG(obj.m_lReservedInt2);
	dwSize += BSize_LONG(obj.m_lReservedInt3);
	dwSize += BSize_LONG(obj.m_lReservedInt4);
	dwSize += BSize_LONG(obj.m_lReservedInt5);
	dwSize += BSize_LONG(obj.m_lReservedInt6);
	dwSize += BSize_LONG(obj.m_lReservedInt7);
	dwSize += BSize_LONG(obj.m_lReservedInt8);
	dwSize += BSize_LONG(obj.m_lReservedInt9);
	dwSize += BSize_LONG(obj.m_lReservedInt10);
	dwSize += BSize_LONG(obj.m_lReservedInt11);
	dwSize += BSize_LONG(obj.m_lReservedInt12);
	dwSize += BSize_LONG(obj.m_lReservedInt13);
	dwSize += BSize_LONG(obj.m_lReservedInt14);
	dwSize += BSize_LONG(obj.m_lReservedInt15);
	dwSize += BSize_LONG(obj.m_lReservedInt16);
	dwSize += BSize_LONG(obj.m_lReservedInt17);
	dwSize += BSize_LONG(obj.m_lReservedInt18);
	dwSize += BSize_LONG(obj.m_lReservedInt19);
	dwSize += BSize_LONG(obj.m_lReservedInt20);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL1);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL2);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL3);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL4);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL5);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL6);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL7);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL8);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL9);
	dwSize += BSize_LONGLONG(obj.m_lReservedLL10);
	dwSize += BSize_xstring(obj.m_sReservedString1);
	dwSize += BSize_xstring(obj.m_sReservedString2);
	dwSize += BSize_xstring(obj.m_sReservedString3);
	dwSize += BSize_xstring(obj.m_sReservedString4);
	dwSize += BSize_xstring(obj.m_sReservedString5);
	dwSize += BSize_xstring(obj.m_sReservedString6);
	dwSize += BSize_xstring(obj.m_sReservedString7);
	dwSize += BSize_xstring(obj.m_sReservedString8);
	dwSize += BSize_xstring(obj.m_sReservedString9);
	dwSize += BSize_xstring(obj.m_sReservedString10);
	dwSize += BSize_xstring(obj.m_sReservedString11);
	dwSize += BSize_xstring(obj.m_sReservedString12);
	dwSize += BSize_xstring(obj.m_sReservedString13);
	dwSize += BSize_xstring(obj.m_sReservedString14);
	dwSize += BSize_xstring(obj.m_sReservedString15);
	return dwSize;
}
inline LPBUF BEncode_DB_GetUserOutData(LPBUF pBuf, LPBUFLEN pdwLen, const DB_GetUserOutData& obj) {
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lUSN))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lLXP))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lCMoney))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lExtMoney))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lCLevel))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalWin))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalLose))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lPersonalDraw))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lDisCnt))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lGuildID1))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt1))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt2))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt3))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt4))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt5))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt6))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt7))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt8))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt9))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt10))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt11))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt12))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt13))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt14))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt15))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt16))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt17))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt18))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt19))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lReservedInt20))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL1))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL2))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL3))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL4))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL5))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL6))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL7))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL8))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL9))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_lReservedLL10))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString1))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString2))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString3))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString4))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString5))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString6))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString7))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString8))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString9))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString10))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString11))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString12))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString13))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString14))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sReservedString15))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_DB_GetUserOutData(LPCBUF pBuf, LPBUFLEN pdwLen, DB_GetUserOutData* pObj) {
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lUSN))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lLXP))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lCMoney))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lExtMoney))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lCLevel))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalWin))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalLose))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lPersonalDraw))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lDisCnt))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lGuildID1))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt1))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt2))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt3))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt4))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt5))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt6))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt7))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt8))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt9))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt10))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt11))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt12))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt13))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt14))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt15))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt16))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt17))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt18))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt19))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lReservedInt20))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL1))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL2))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL3))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL4))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL5))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL6))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL7))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL8))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL9))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_lReservedLL10))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString1))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString2))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString3))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString4))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString5))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString6))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString7))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString8))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString9))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString10))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString11))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString12))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString13))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString14))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sReservedString15))) return NULL;
	return pBuf;
}

class DB_SetExtMoneyInData {
public:
	LONG m_lUSN;
	LONG m_lMSN;
public:
#ifndef DB_SetExtMoneyInData_Ctor // to override default ctor, define this symbol
	DB_SetExtMoneyInData() { }
#endif
	DB_SetExtMoneyInData(const DB_SetExtMoneyInData& obj) { operator=(obj); }
	explicit DB_SetExtMoneyInData(const LONG& lUSN, const LONG& lMSN = LONG()) 
		 : m_lUSN(lUSN), m_lMSN(lMSN) { }
	DB_SetExtMoneyInData& operator=(const DB_SetExtMoneyInData& obj) {
		m_lUSN = obj.m_lUSN;
		m_lMSN = obj.m_lMSN;
		return *this;
	}
public:


	void Clear()
	{
		m_lUSN = 0L;
		m_lMSN = 0L;
	}
};

inline DWORD BSize_DB_SetExtMoneyInData(const DB_SetExtMoneyInData& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_LONG(obj.m_lUSN);
	dwSize += BSize_LONG(obj.m_lMSN);
	return dwSize;
}
inline LPBUF BEncode_DB_SetExtMoneyInData(LPBUF pBuf, LPBUFLEN pdwLen, const DB_SetExtMoneyInData& obj) {
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lUSN))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lMSN))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_DB_SetExtMoneyInData(LPCBUF pBuf, LPBUFLEN pdwLen, DB_SetExtMoneyInData* pObj) {
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lUSN))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lMSN))) return NULL;
	return pBuf;
}

class DB_SetExtMoneyOutData {
public:
	LONG m_lUSN;
	LONG m_lMSN;
	LONGLONG m_llDExtMoney;
	LONGLONG m_llCExtMoney;
	xstring m_sExtMoneyEndDate;
	LONGLONG m_llCMoney;
	LONGLONG m_llCSafeMoney;
public:
#ifndef DB_SetExtMoneyOutData_Ctor // to override default ctor, define this symbol
	DB_SetExtMoneyOutData() { }
#endif
	DB_SetExtMoneyOutData(const DB_SetExtMoneyOutData& obj) { operator=(obj); }
	explicit DB_SetExtMoneyOutData(const LONG& lUSN, const LONG& lMSN = LONG(), const LONGLONG& llDExtMoney = LONGLONG(), const LONGLONG& llCExtMoney = LONGLONG(), const xstring& sExtMoneyEndDate = xstring(), const LONGLONG& llCMoney = LONGLONG(), const LONGLONG& llCSafeMoney = LONGLONG()) 
		 : m_lUSN(lUSN), m_lMSN(lMSN), m_llDExtMoney(llDExtMoney), m_llCExtMoney(llCExtMoney), m_sExtMoneyEndDate(sExtMoneyEndDate), m_llCMoney(llCMoney), m_llCSafeMoney(llCSafeMoney) { }
	DB_SetExtMoneyOutData& operator=(const DB_SetExtMoneyOutData& obj) {
		m_lUSN = obj.m_lUSN;
		m_lMSN = obj.m_lMSN;
		m_llDExtMoney = obj.m_llDExtMoney;
		m_llCExtMoney = obj.m_llCExtMoney;
		m_sExtMoneyEndDate = obj.m_sExtMoneyEndDate;
		m_llCMoney = obj.m_llCMoney;
		m_llCSafeMoney = obj.m_llCSafeMoney;
		return *this;
	}
public:


	void Clear()
	{
		m_lUSN = 0L;
		m_lMSN = 0L;
		
		m_llDExtMoney = 0L;
		m_llCExtMoney = 0L;
		m_sExtMoneyEndDate.erase();
		m_llCMoney = 0L;
		m_llCSafeMoney = 0L;
	}
};

inline DWORD BSize_DB_SetExtMoneyOutData(const DB_SetExtMoneyOutData& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_LONG(obj.m_lUSN);
	dwSize += BSize_LONG(obj.m_lMSN);
	dwSize += BSize_LONGLONG(obj.m_llDExtMoney);
	dwSize += BSize_LONGLONG(obj.m_llCExtMoney);
	dwSize += BSize_xstring(obj.m_sExtMoneyEndDate);
	dwSize += BSize_LONGLONG(obj.m_llCMoney);
	dwSize += BSize_LONGLONG(obj.m_llCSafeMoney);
	return dwSize;
}
inline LPBUF BEncode_DB_SetExtMoneyOutData(LPBUF pBuf, LPBUFLEN pdwLen, const DB_SetExtMoneyOutData& obj) {
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lUSN))) return NULL;
	if(!(pBuf = BEncode_LONG(pBuf, pdwLen, obj.m_lMSN))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_llDExtMoney))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_llCExtMoney))) return NULL;
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sExtMoneyEndDate))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_llCMoney))) return NULL;
	if(!(pBuf = BEncode_LONGLONG(pBuf, pdwLen, obj.m_llCSafeMoney))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_DB_SetExtMoneyOutData(LPCBUF pBuf, LPBUFLEN pdwLen, DB_SetExtMoneyOutData* pObj) {
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lUSN))) return NULL;
	if(!(pBuf = BDecode_LONG(pBuf, pdwLen, &pObj->m_lMSN))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_llDExtMoney))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_llCExtMoney))) return NULL;
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sExtMoneyEndDate))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_llCMoney))) return NULL;
	if(!(pBuf = BDecode_LONGLONG(pBuf, pdwLen, &pObj->m_llCSafeMoney))) return NULL;
	return pBuf;
}

typedef list<DB_SetUserInData> SetUserInDataList;
inline DWORD BSize_SetUserInDataList(const SetUserInDataList& obj) {
	DWORD dwSize = BSize_DWORD(dwSize);
	for(list<DB_SetUserInData>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		dwSize += BSize_DB_SetUserInData(*it);
	return dwSize;
}
inline LPBUF BEncode_SetUserInDataList(LPBUF pBuf, LPBUFLEN pdwLen, const SetUserInDataList& obj) {
	if(!(pBuf = BEncode_DWORD(pBuf, pdwLen, obj.size()))) return NULL;
	for(list<DB_SetUserInData>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		if(!(pBuf = BEncode_DB_SetUserInData(pBuf, pdwLen, *it))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_SetUserInDataList(LPCBUF pBuf, LPBUFLEN pdwLen, SetUserInDataList* pObj) {
	DWORD dwSize = 0;
	if(!(pBuf = BDecode_DWORD(pBuf, pdwLen, &dwSize))) return NULL;
	pObj->clear();
	for(DWORD dwCount = 0; dwCount < dwSize; ++dwCount) {
		pObj->push_back(DB_SetUserInData());
		if(!(pBuf = BDecode_DB_SetUserInData(pBuf, pdwLen, &pObj->back()))) { pObj->pop_back(); return NULL; }
	}
	return pBuf;
}

typedef list<DB_SetUserOutData> SetUserOutDataList;
inline DWORD BSize_SetUserOutDataList(const SetUserOutDataList& obj) {
	DWORD dwSize = BSize_DWORD(dwSize);
	for(list<DB_SetUserOutData>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		dwSize += BSize_DB_SetUserOutData(*it);
	return dwSize;
}
inline LPBUF BEncode_SetUserOutDataList(LPBUF pBuf, LPBUFLEN pdwLen, const SetUserOutDataList& obj) {
	if(!(pBuf = BEncode_DWORD(pBuf, pdwLen, obj.size()))) return NULL;
	for(list<DB_SetUserOutData>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		if(!(pBuf = BEncode_DB_SetUserOutData(pBuf, pdwLen, *it))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_SetUserOutDataList(LPCBUF pBuf, LPBUFLEN pdwLen, SetUserOutDataList* pObj) {
	DWORD dwSize = 0;
	if(!(pBuf = BDecode_DWORD(pBuf, pdwLen, &dwSize))) return NULL;
	pObj->clear();
	for(DWORD dwCount = 0; dwCount < dwSize; ++dwCount) {
		pObj->push_back(DB_SetUserOutData());
		if(!(pBuf = BDecode_DB_SetUserOutData(pBuf, pdwLen, &pObj->back()))) { pObj->pop_back(); return NULL; }
	}
	return pBuf;
}

class NDL_SetUserInDataInfo {
public:
	SetUserInDataList m_lstInData;
public:
#ifndef NDL_SetUserInDataInfo_Ctor // to override default ctor, define this symbol
	NDL_SetUserInDataInfo() { }
#endif
	NDL_SetUserInDataInfo(const NDL_SetUserInDataInfo& obj) { operator=(obj); }
	explicit NDL_SetUserInDataInfo(const SetUserInDataList& lstInData) 
		 : m_lstInData(lstInData) { }
	NDL_SetUserInDataInfo& operator=(const NDL_SetUserInDataInfo& obj) {
		m_lstInData = obj.m_lstInData;
		return *this;
	}
public:


	void Clear()
	{
		m_lstInData.clear();
	}
};

inline DWORD BSize_NDL_SetUserInDataInfo(const NDL_SetUserInDataInfo& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_SetUserInDataList(obj.m_lstInData);
	return dwSize;
}
inline LPBUF BEncode_NDL_SetUserInDataInfo(LPBUF pBuf, LPBUFLEN pdwLen, const NDL_SetUserInDataInfo& obj) {
	if(!(pBuf = BEncode_SetUserInDataList(pBuf, pdwLen, obj.m_lstInData))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_NDL_SetUserInDataInfo(LPCBUF pBuf, LPBUFLEN pdwLen, NDL_SetUserInDataInfo* pObj) {
	if(!(pBuf = BDecode_SetUserInDataList(pBuf, pdwLen, &pObj->m_lstInData))) return NULL;
	return pBuf;
}

class NDL_SetUserOutDataInfo {
public:
	SetUserOutDataList m_lstOutData;
public:
#ifndef NDL_SetUserOutDataInfo_Ctor // to override default ctor, define this symbol
	NDL_SetUserOutDataInfo() { }
#endif
	NDL_SetUserOutDataInfo(const NDL_SetUserOutDataInfo& obj) { operator=(obj); }
	explicit NDL_SetUserOutDataInfo(const SetUserOutDataList& lstOutData) 
		 : m_lstOutData(lstOutData) { }
	NDL_SetUserOutDataInfo& operator=(const NDL_SetUserOutDataInfo& obj) {
		m_lstOutData = obj.m_lstOutData;
		return *this;
	}
public:


	void Clear()
	{
		m_lstOutData.clear();
	}
};

inline DWORD BSize_NDL_SetUserOutDataInfo(const NDL_SetUserOutDataInfo& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_SetUserOutDataList(obj.m_lstOutData);
	return dwSize;
}
inline LPBUF BEncode_NDL_SetUserOutDataInfo(LPBUF pBuf, LPBUFLEN pdwLen, const NDL_SetUserOutDataInfo& obj) {
	if(!(pBuf = BEncode_SetUserOutDataList(pBuf, pdwLen, obj.m_lstOutData))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_NDL_SetUserOutDataInfo(LPCBUF pBuf, LPBUFLEN pdwLen, NDL_SetUserOutDataInfo* pObj) {
	if(!(pBuf = BDecode_SetUserOutDataList(pBuf, pdwLen, &pObj->m_lstOutData))) return NULL;
	return pBuf;
}

class NDL_QueryString {
public:
	xstring m_sQuery;
public:
#ifndef NDL_QueryString_Ctor // to override default ctor, define this symbol
	NDL_QueryString() { }
#endif
	NDL_QueryString(const NDL_QueryString& obj) { operator=(obj); }
	explicit NDL_QueryString(const xstring& sQuery) 
		 : m_sQuery(sQuery) { }
	NDL_QueryString& operator=(const NDL_QueryString& obj) {
		m_sQuery = obj.m_sQuery;
		return *this;
	}
public:


	void Clear()
	{
		m_sQuery.erase();
	}
};

inline DWORD BSize_NDL_QueryString(const NDL_QueryString& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_xstring(obj.m_sQuery);
	return dwSize;
}
inline LPBUF BEncode_NDL_QueryString(LPBUF pBuf, LPBUFLEN pdwLen, const NDL_QueryString& obj) {
	if(!(pBuf = BEncode_xstring(pBuf, pdwLen, obj.m_sQuery))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_NDL_QueryString(LPCBUF pBuf, LPBUFLEN pdwLen, NDL_QueryString* pObj) {
	if(!(pBuf = BDecode_xstring(pBuf, pdwLen, &pObj->m_sQuery))) return NULL;
	return pBuf;
}

class NDL_ResultStr {
public:
	string m_sResult;
public:
#ifndef NDL_ResultStr_Ctor // to override default ctor, define this symbol
	NDL_ResultStr() { }
#endif
	NDL_ResultStr(const NDL_ResultStr& obj) { operator=(obj); }
	explicit NDL_ResultStr(const string& sResult) 
		 : m_sResult(sResult) { }
	NDL_ResultStr& operator=(const NDL_ResultStr& obj) {
		m_sResult = obj.m_sResult;
		return *this;
	}
public:


	void Clear()
	{
		m_sResult.erase();
	}
};

inline DWORD BSize_NDL_ResultStr(const NDL_ResultStr& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_string(obj.m_sResult);
	return dwSize;
}
inline LPBUF BEncode_NDL_ResultStr(LPBUF pBuf, LPBUFLEN pdwLen, const NDL_ResultStr& obj) {
	if(!(pBuf = BEncode_string(pBuf, pdwLen, obj.m_sResult))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_NDL_ResultStr(LPCBUF pBuf, LPBUFLEN pdwLen, NDL_ResultStr* pObj) {
	if(!(pBuf = BDecode_string(pBuf, pdwLen, &pObj->m_sResult))) return NULL;
	return pBuf;
}

class NDL_ResultStrW {
public:
	wstring m_sResult;
public:
#ifndef NDL_ResultStrW_Ctor // to override default ctor, define this symbol
	NDL_ResultStrW() { }
#endif
	NDL_ResultStrW(const NDL_ResultStrW& obj) { operator=(obj); }
	explicit NDL_ResultStrW(const wstring& sResult) 
		 : m_sResult(sResult) { }
	NDL_ResultStrW& operator=(const NDL_ResultStrW& obj) {
		m_sResult = obj.m_sResult;
		return *this;
	}
public:


	void Clear()
	{
		m_sResult.erase();
	}
};

inline DWORD BSize_NDL_ResultStrW(const NDL_ResultStrW& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_wstring(obj.m_sResult);
	return dwSize;
}
inline LPBUF BEncode_NDL_ResultStrW(LPBUF pBuf, LPBUFLEN pdwLen, const NDL_ResultStrW& obj) {
	if(!(pBuf = BEncode_wstring(pBuf, pdwLen, obj.m_sResult))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_NDL_ResultStrW(LPCBUF pBuf, LPBUFLEN pdwLen, NDL_ResultStrW* pObj) {
	if(!(pBuf = BDecode_wstring(pBuf, pdwLen, &pObj->m_sResult))) return NULL;
	return pBuf;
}

typedef list<string> StrList;
inline DWORD BSize_StrList(const StrList& obj) {
	DWORD dwSize = BSize_DWORD(dwSize);
	for(list<string>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		dwSize += BSize_string(*it);
	return dwSize;
}
inline LPBUF BEncode_StrList(LPBUF pBuf, LPBUFLEN pdwLen, const StrList& obj) {
	if(!(pBuf = BEncode_DWORD(pBuf, pdwLen, obj.size()))) return NULL;
	for(list<string>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		if(!(pBuf = BEncode_string(pBuf, pdwLen, *it))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_StrList(LPCBUF pBuf, LPBUFLEN pdwLen, StrList* pObj) {
	DWORD dwSize = 0;
	if(!(pBuf = BDecode_DWORD(pBuf, pdwLen, &dwSize))) return NULL;
	pObj->clear();
	for(DWORD dwCount = 0; dwCount < dwSize; ++dwCount) {
		pObj->push_back(string());
		if(!(pBuf = BDecode_string(pBuf, pdwLen, &pObj->back()))) { pObj->pop_back(); return NULL; }
	}
	return pBuf;
}

class NDL_ResultList {
public:
	StrList m_lstResultString;
public:
#ifndef NDL_ResultList_Ctor // to override default ctor, define this symbol
	NDL_ResultList() { }
#endif
	NDL_ResultList(const NDL_ResultList& obj) { operator=(obj); }
	explicit NDL_ResultList(const StrList& lstResultString) 
		 : m_lstResultString(lstResultString) { }
	NDL_ResultList& operator=(const NDL_ResultList& obj) {
		m_lstResultString = obj.m_lstResultString;
		return *this;
	}
public:


	void Clear()
	{
		m_lstResultString.clear();
	}
};

inline DWORD BSize_NDL_ResultList(const NDL_ResultList& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_StrList(obj.m_lstResultString);
	return dwSize;
}
inline LPBUF BEncode_NDL_ResultList(LPBUF pBuf, LPBUFLEN pdwLen, const NDL_ResultList& obj) {
	if(!(pBuf = BEncode_StrList(pBuf, pdwLen, obj.m_lstResultString))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_NDL_ResultList(LPCBUF pBuf, LPBUFLEN pdwLen, NDL_ResultList* pObj) {
	if(!(pBuf = BDecode_StrList(pBuf, pdwLen, &pObj->m_lstResultString))) return NULL;
	return pBuf;
}

typedef list<wstring> StrListW;
inline DWORD BSize_StrListW(const StrListW& obj) {
	DWORD dwSize = BSize_DWORD(dwSize);
	for(list<wstring>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		dwSize += BSize_wstring(*it);
	return dwSize;
}
inline LPBUF BEncode_StrListW(LPBUF pBuf, LPBUFLEN pdwLen, const StrListW& obj) {
	if(!(pBuf = BEncode_DWORD(pBuf, pdwLen, obj.size()))) return NULL;
	for(list<wstring>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		if(!(pBuf = BEncode_wstring(pBuf, pdwLen, *it))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_StrListW(LPCBUF pBuf, LPBUFLEN pdwLen, StrListW* pObj) {
	DWORD dwSize = 0;
	if(!(pBuf = BDecode_DWORD(pBuf, pdwLen, &dwSize))) return NULL;
	pObj->clear();
	for(DWORD dwCount = 0; dwCount < dwSize; ++dwCount) {
		pObj->push_back(wstring());
		if(!(pBuf = BDecode_wstring(pBuf, pdwLen, &pObj->back()))) { pObj->pop_back(); return NULL; }
	}
	return pBuf;
}

class NDL_ResultListW {
public:
	StrListW m_lstResultString;
public:
#ifndef NDL_ResultListW_Ctor // to override default ctor, define this symbol
	NDL_ResultListW() { }
#endif
	NDL_ResultListW(const NDL_ResultListW& obj) { operator=(obj); }
	explicit NDL_ResultListW(const StrListW& lstResultString) 
		 : m_lstResultString(lstResultString) { }
	NDL_ResultListW& operator=(const NDL_ResultListW& obj) {
		m_lstResultString = obj.m_lstResultString;
		return *this;
	}
public:


	void Clear()
	{
		m_lstResultString.clear();
	}
};

inline DWORD BSize_NDL_ResultListW(const NDL_ResultListW& obj) {
	DWORD dwSize = 0;
	dwSize += BSize_StrListW(obj.m_lstResultString);
	return dwSize;
}
inline LPBUF BEncode_NDL_ResultListW(LPBUF pBuf, LPBUFLEN pdwLen, const NDL_ResultListW& obj) {
	if(!(pBuf = BEncode_StrListW(pBuf, pdwLen, obj.m_lstResultString))) return NULL;
	return pBuf;
}
inline LPCBUF BDecode_NDL_ResultListW(LPCBUF pBuf, LPBUFLEN pdwLen, NDL_ResultListW* pObj) {
	if(!(pBuf = BDecode_StrListW(pBuf, pdwLen, &pObj->m_lstResultString))) return NULL;
	return pBuf;
}
#endif //!NDL_MsgNDLGameDB2_h
