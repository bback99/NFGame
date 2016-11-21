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
		if(bDeleteFlag) delete (TData*)iter->second; /// �����͸� ������ delete ��Ű�� �ؾ� �Ǵµ�?
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
		if(bDeleteFlag) delete (TData*)iter->second; /// �����͸� ������ delete ��Ű�� �ؾ� �Ǵµ�?
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
	// �� function�� ������ ��� Key group������ �۾��� �Ϸ��ؾ� �Ѵ�.
	LONG AddItem(TKey key, NLSBaseInfo & newBaseInfo, NLSBaseInfo & tempBaseInfo);
	BOOL _AddItem(TKey key, NLSBaseInfo & info);
	LONG UpdateItem(TKey key, NLSBaseInfo & baseInfo);
	void RemItem(TKey key);
	void _RemItem(NLSBaseInfo * pbaseInfo);
	void RemValue(TKey key, NLSBaseInfo & baseInfo);
	void RemValue(TKey key, NSAP & nsap);
	void RemoveValue(const LRBAddress& addr);

	// ��� �� Ư�� ��Ȳ�� ���� ó�� 
	void ClearLCInfo();
	void ClearAll() { ClearLCInfo(); }
	
	// �Ϲ����� �䱸 ó��..
	BOOL FindKey(TKey key);
	BOOL FindKeyEx(TKey key, LONG lIP);
	BOOL FindValue(TKey key, const NLSBaseInfo & baseInfo, LONG& lIndex);	
	BOOL GetItem(TKey key, NLSBaseInfo & lstBaseInfo);
	BOOL GetItem(TKey key, NLSBaseInfoList & lstBaseInfo);
	BOOL GetItem(TKey key, LONG lIndex, NLSBaseInfo & info);	// lIndex�� �ش��ϴ� user ����.
	BOOL AddRegionStat(char regionID);
	BOOL SubstractRegionStat(char regionID);
	LONG GetRegionStat(char regionID);
	BOOL SetItemOptionData(TKey key, LONG lIndex, string strData);	// �ɼ� �����ϱ�.
	LONG GetItemCnt();						// map ��ü�� ����� Item ��
	LONG GetValueCnt(char Key);			// Key�� �ش��ϴ� value�� ����

	// Ư�� ������ ���� ��ġ ���� ��û ó�� 

private:
	CCommonMapData<LONGLONG, LIST_UINFO> m_mapUSN;
	CCommonMapData<char, LONG> m_mapRegionStat;
//	CCommonMapData<RoomID, LIST_UINFO> m_mapRoomID;

	long m_lAddCount;
};

extern CLocationMainDB theLocationMainDB;

#endif // !defined(AFX_LCINFOMAP_H__1A353F28_DC3F_4AC9_8F07_3AABCF48CCEB__INCLUDED_)
