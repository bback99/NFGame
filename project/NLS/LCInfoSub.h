// LCInfoSub.h: interface for the LCInfoSub class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LCINFOSUB_H__436DA44E_FD80_4E77_A3BB_41B950730266__INCLUDED_)
#define AFX_LCINFOSUB_H__436DA44E_FD80_4E77_A3BB_41B950730266__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDataObjectPool  
{
public:
	IMPLEMENT_TISAFEREFCNT(CDataObjectPool)

	typedef vector<void *> vecPoolT;

public:
	CDataObjectPool();
	CDataObjectPool(LONG lSize, DWORD dwPoolCnt = 1000UL, DWORD dwSlip = 4UL);
	virtual ~CDataObjectPool();

	// lCount 즉 원하는 갯수 만큼의 heap block 할당을 보장하지 않는다.
	// return 값으로 확인가능.
	LONG CreatePool(LONG lSize, DWORD dwPoolCnt = 1000UL, DWORD dwSlip = 4UL);	
	void DestroyPool();	
	// Pool에서 제공.. 없을 경우 Pool을 일정 단위만큼 생성..
	// if fail return NULL 
	void * Pop();

	void Push(void * pV);

	LONG GetIdelCnt() { return m_dwIdleBlockCnt; }
	DWORD GetPoolSize() { return m_dwLimitBlockCnt; }
	DWORD GetBlockSize() { return m_dwBlockSize; }

private:
	LONG IncIdleCnt() { ::InterlockedIncrement((LPLONG)&m_dwIdleBlockCnt); return m_dwIdleBlockCnt; }
	LONG DecIdleCnt() { ::InterlockedDecrement((LPLONG)&m_dwIdleBlockCnt); return m_dwIdleBlockCnt; }

	// for control Heap 
	bool CreateHeap(size_t alloc_size = 0, size_t max_size = 0);
	void DestroyHeap();
	void * _AllocHeap(size_t block_size);
	void _FreeHeap(void * pv);
	void _Lock() {::HeapLock(m_hHeap);}
	void _Unlock() {::HeapUnlock(m_hHeap);}

protected:
	// heap의 할당및 해제 수행.. Default값(0 )이면, 적당한 갯수를 채워넣는다.
	void _AllocMemory(DWORD dwAppendCnt = 0UL);
	void _FreeMemory(DWORD dwRemoveCnt = 0UL);

	DWORD _GetCapacity() { return m_dwLimitBlockCnt - m_dwIdleBlockCnt; }

private:
	vecPoolT	m_vecObj;

	DWORD		m_dwBlockSize;
	DWORD		m_dwLimitBlockCnt;
	DWORD		m_dwIdleBlockCnt;

	// for control Heap 
	HANDLE		m_hHeap;
	DWORD		m_dwSlipLevel;

};

extern CDataObjectPool theDataObjectPool;


#endif // !defined(AFX_LCINFOSUB_H__436DA44E_FD80_4E77_A3BB_41B950730266__INCLUDED_)
