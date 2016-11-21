#ifndef __ACHVDEF_H_
#define __ACHVDEF_H_

#include <NF/ADL/MsgNFCommonStruct.h>

//--------------------------------------------------------------------
// Name: class CAchvMgr
// Dec: 업적 매니저
//		업적 Meta Data를 갖고, 업적 달성 체크
//--------------------------------------------------------------------
class CAchvMgr
{
	typedef multimap< LONG, SAchvDef > MAP_DEF_ACHV;
	
	//typedef map< LONG, SAchvState >			MAP_ACHV_STATE; // <업적번호, 업적진행정보>
	//typedef map< LONG, MAP_ACHV_STATE >		MAP_ACHV_STATE_ALL_CHAR; // < CSN, MAP_ACHV_STATE >

	enum EFACTOR_COMPARE_TYPE
	{
		ECT_NIL = 0,

		ECT_EQUAL,		// 값이 같아야 성공
		ECT_GREATER,	// 해당 값 이상이면 성공
		ECT_LESS,		// 해당 값 이하면 성공
		ECT_RANGE,		// MIN 이상 && MAX 이하면 성공
	};

private:
	CAchvMgr()
		:m_kMapDefAchv(), m_kMapAchvStateAllChar() {}
	~CAchvMgr(){}

public:
	static CAchvMgr& Instance()
	{
		static CAchvMgr instance;
		return instance;
	}

private:
	BOOL IsComplete(const LONG lCSN, const LONG lAchvID);
	double GetProgress(const LONG lCSN, const LONG lAchvID);
	void UpdateAchvState(const LONG lGSN, const LONG lCSN, const LONG lAchvID, const double dProgress, const bool isCompleted);
	BOOL CheckRelation(const LONG lCSN, const LONG lParentID);

public:
	BOOL LoadAchvXML();
	void CheckAchv(const LONG lGSN, const LONG lCSN, const LONG lEvent, const map< string, LONG >& mapFactorVal);
	
	BOOL LoadCharAchvState(const LONG lGSN, const LONG lCSN);

private:
	MAP_DEF_ACHV				m_kMapDefAchv;
	MAP_ACHV_STATE_ALL_CHAR		m_kMapAchvStateAllChar; // 모든 온라인 캐릭터의 업적진행 상황을 담고 있음.
};

// ! 항상 접근은 g_kAchvDef로 한다.
#define g_kAchvDef CAchvMgr::Instance()

#endif // __ACHVDEF_H_