// LCInfoMap.h: interface for the CLCInfoMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LCINFOMAP_H__1A353F28_DC3F_4AC9_8F07_3AABCF48CCEB__INCLUDED_)
#define AFX_LCINFOMAP_H__1A353F28_DC3F_4AC9_8F07_3AABCF48CCEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Config.h"
//#include "LCInfoSub.h"

template<class TKey, class TData>
class CCommonPtrMapData {
//	typedef map<TKey, TData*>::iterator COMPTR_MAP_ITERATOR;
public:
	CCommonPtrMapData() {}
	BOOL Insert(TKey key, TData* data) {
		map<TKey, TData>::iterator iter = m_mapData.find(key);		
		if(iter == m_mapData.end()) m_mapData[key] = data;
		else return FALSE;
		return TRUE;
	}
	BOOL Delete(TKey key, BOOL bDeleteFlag = FALSE) {
		map<TKey, TData>::iterator iter = m_mapData.find(key);		
		if(iter == m_mapData.end()) return FALSE;
		if(bDeleteFlag) delete (TData*)iter->second; /// 포인터면 기존거 delete 시키고 해야 되는디?
		m_mapData.erase(iter);
		return TRUE;
	}
	BOOL Update(TKey key, TData* data, BOOL bDeleteFlag = FALSE)	// TRUE : exist, FALSE : not exist
	{
		map<TKey, TData>::iterator iter = m_mapData.find(key);		
		if(iter == m_mapData.end()) {
			m_mapData[key] = data;
			return FALSE;
		}
		if(bDeleteFlag) delete (TData*)iter->second; /// 포인터면 기존거 delete 시키고 해야 되는디?
		m_mapData[key] = data;	
		return TRUE;
	}
	TData* Find(TKey key) 	{
		map<TKey, TData>::iterator iter = m_mapData.find(key);
		if(iter == m_mapData.end()) return NULL;
		else return (iter->second);
	}
	void Clear() { m_mapData.clear(); }
	long Size() { return m_mapData.size(); }
	map<TKey, TData*>* GetData() { return &m_mapData; }

private:
	map<TKey, TData*> m_mapData;
};

template<class TKey, class TData>
class CCommonMapData {	
public:
	CCommonMapData() {}
	BOOL Insert(TKey key, TData data) {
		map<TKey, TData>::iterator iter = m_mapData.find(key);		
		if(iter == m_mapData.end()) m_mapData[key] = data;
		else return FALSE;
		return TRUE;
	}
	BOOL Delete(TKey key) {
		map<TKey, TData>::iterator iter = m_mapData.find(key);		
		if(iter == m_mapData.end()) return FALSE;		
		m_mapData.erase(iter);
		return TRUE;
	}
	BOOL Update(TKey key, TData data) {
		map<TKey, TData>::iterator iter = m_mapData.find(key);		
		if(iter == m_mapData.end()) {
			m_mapData[key] = data;
			return FALSE;
		}		
		m_mapData[key] = data;
		return TRUE;
	}
	TData* Find(TKey key) {
		map<TKey, TData>::iterator iter = m_mapData.find(key);
		if(iter == m_mapData.end()) return NULL;
		else return &(iter->second);
	}
	void Clear() { m_mapData.clear(); }
	long Size() { return m_mapData.size(); }
	map<TKey, TData>* GetData() { return &m_mapData; }

private:
	typedef map<TKey, TData>	TMapData;
	TMapData m_mapData;
};



class CLocationMainDB
{
public:
	IMPLEMENT_TISAFEREFCNT(CLocationMainDB)

	typedef list<NLSBaseInfo*> LIST_UINFO;
	typedef list<NLSBaseInfo*>::iterator LIST_UINFO_ITER;

	typedef vector<DWORD> VecDWORDT;
	typedef vector<LONG> VecLONGT;

public:
	CLocationMainDB();
	virtual ~CLocationMainDB();

	// default function .. 
	// 각 function의 동작은 모든 Key group에대한 작업을 완료해야 한다.
	LONG AddItem(TKey key, NLSBaseInfo & newBaseInfo, NLSBaseInfo & tempBaseInfo);
	BOOL _AddItem(TKey key, NLSBaseInfo & info);
	LONG UpdateItem(TKey key, NLSBaseInfo & baseInfo);
	void RemItem(TKey key);
	void _RemItem(NLSBaseInfo * pbaseInfo);
	void RemValue(TKey key, NLSBaseInfo & baseInfo);
	void RemValue(TKey key, NSAP & nsap);
	void RemoveValue(const LRBAddress& addr);

	// 장애 및 특정 상황에 대한 처리 
	void ClearLCInfo();
	void ClearAll() { ClearLCInfo(); }
	
	// 일반적인 요구 처리..
	BOOL FindKey(TKey key);
	BOOL FindKeyEx(TKey key, LONG lIP);
	BOOL FindValue(TKey key, const NLSBaseInfo & baseInfo, LONG& lIndex);	
	BOOL GetItem(TKey key, NLSBaseInfo & lstBaseInfo);
	BOOL GetItem(TKey key, NLSBaseInfoList & lstBaseInfo);
	BOOL GetItem(TKey key, LONG lIndex, NLSBaseInfo & info);	// lIndex에 해당하는 user 정보.
	BOOL AddRegionStat(char regionID);
	BOOL SubstractRegionStat(char regionID);
	LONG GetRegionStat(char regionID);
	BOOL SetItemOptionData(TKey key, LONG lIndex, string strData);	// 옵션 변경하기.
	LONG GetItemCnt();						// map 전체에 저장된 Item 수
	LONG GetValueCnt(char Key);			// Key에 해당하는 value의 갯수

	// 특정 정보에 의한 위치 정보 요청 처리 

private:
	CCommonMapData<LONGLONG, LIST_UINFO> m_mapUSN;
	CCommonMapData<char, LONG> m_mapRegionStat;
//	CCommonMapData<RoomID, LIST_UINFO> m_mapRoomID;

	long m_lAddCount;
};

extern CLocationMainDB theLocationMainDB;

#endif // !defined(AFX_LCINFOMAP_H__1A353F28_DC3F_4AC9_8F07_3AABCF48CCEB__INCLUDED_)
