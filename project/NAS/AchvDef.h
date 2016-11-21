#ifndef __ACHVDEF_H_
#define __ACHVDEF_H_

#include <NF/ADL/MsgNFCommonStruct.h>

//--------------------------------------------------------------------
// Name: class CAchvMgr
// Dec: ���� �Ŵ���
//		���� Meta Data�� ����, ���� �޼� üũ
//--------------------------------------------------------------------
class CAchvMgr
{
	typedef multimap< LONG, SAchvDef > MAP_DEF_ACHV;
	
	//typedef map< LONG, SAchvState >			MAP_ACHV_STATE; // <������ȣ, ������������>
	//typedef map< LONG, MAP_ACHV_STATE >		MAP_ACHV_STATE_ALL_CHAR; // < CSN, MAP_ACHV_STATE >

	enum EFACTOR_COMPARE_TYPE
	{
		ECT_NIL = 0,

		ECT_EQUAL,		// ���� ���ƾ� ����
		ECT_GREATER,	// �ش� �� �̻��̸� ����
		ECT_LESS,		// �ش� �� ���ϸ� ����
		ECT_RANGE,		// MIN �̻� && MAX ���ϸ� ����
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
	MAP_ACHV_STATE_ALL_CHAR		m_kMapAchvStateAllChar; // ��� �¶��� ĳ������ �������� ��Ȳ�� ��� ����.
};

// ! �׻� ������ g_kAchvDef�� �Ѵ�.
#define g_kAchvDef CAchvMgr::Instance()

#endif // __ACHVDEF_H_