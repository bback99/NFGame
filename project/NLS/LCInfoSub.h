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

	// lCount �� ���ϴ� ���� ��ŭ�� heap block �Ҵ��� �������� �ʴ´�.
	// return ������ Ȯ�ΰ���.
	LONG CreatePool(LONG lSize, DWORD dwPoolCnt = 1000UL, DWORD dwSlip = 4UL);	
	void DestroyPool();	
	// Pool���� ����.. ���� ��� Pool�� ���� ������ŭ ����..
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
	// heap�� �Ҵ�� ���� ����.. Default��(0 )�̸�, ������ ������ ä���ִ´�.
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
