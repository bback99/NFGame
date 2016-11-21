// LCInfoSub.cpp: implementation of the LCInfoSub class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LCInfoSub.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDataObjectPool theDataObjectPool;

CDataObjectPool::CDataObjectPool()
{
	m_hHeap = 0;

	m_vecObj.clear();
	m_dwBlockSize = 0UL;
	m_dwLimitBlockCnt = 0UL;
	m_dwIdleBlockCnt = 0UL;
	m_dwSlipLevel = 0UL;
}

CDataObjectPool::CDataObjectPool(LONG lSize, DWORD dwPoolCnt, DWORD dwSlip)
{
	m_hHeap = 0;
	m_vecObj.clear();
	m_dwBlockSize = 0UL;
	m_dwLimitBlockCnt = 0UL;
	m_dwIdleBlockCnt = 0UL;
	if(dwSlip < 2 || dwSlip > 4) 
		m_dwSlipLevel = 2UL;
	else 
		m_dwSlipLevel = dwSlip;

	CreatePool(lSize, dwPoolCnt);
}

CDataObjectPool::~CDataObjectPool()
{
	DestroyPool();
}

// 증감을 동시에 처리..실제 메모리를 heap에서 할당 받아, memory pool에 보관.
void CDataObjectPool::_AllocMemory(DWORD dwAppendCnt)
{
	TLock lo(this);

	if(dwAppendCnt > 0)	// 원하는 갯수 or full로 채운다.
	{
		DWORD _dwcnt = 0L;
		while(_GetCapacity() > 0 && _dwcnt != dwAppendCnt)
		{
			void * pv = _AllocHeap(m_dwBlockSize);
			if(pv)
			{
				m_vecObj.push_back(pv);
				m_dwIdleBlockCnt++;
				_dwcnt++;
			}
		}
	}
	else				// 적당히 채운다.
	{
		while(_GetCapacity() > (m_dwLimitBlockCnt / m_dwSlipLevel) )
		{
			void * pv = _AllocHeap(m_dwBlockSize);
			if(pv)
			{
				m_vecObj.push_back(pv);
				m_dwIdleBlockCnt++;
			}
		}
	}
}
void CDataObjectPool::_FreeMemory(DWORD dwRemoveCnt)
{
	TLock lo(this);
	if(dwRemoveCnt > 0)	// 원하는 갯수 또는 전체를 비운다.
	{
		DWORD _dwcnt = 0L;
		while(m_dwIdleBlockCnt > 0 && _dwcnt != dwRemoveCnt )
		{
			void * pv = m_vecObj[--m_dwIdleBlockCnt];
			if(pv)
				_FreeHeap(pv);
			m_vecObj.pop_back();
			_dwcnt++;
		}
	}
	else
	{
		while(_GetCapacity() < (m_dwLimitBlockCnt / m_dwSlipLevel) )
		{
			void * pv = m_vecObj[--m_dwIdleBlockCnt];
			if(pv)
				_FreeHeap(pv);
			m_vecObj.pop_back();
		}
	}

}

// Heap memory Pool 생성..
// 동일한 크기의 작은 메모리를 다수 생성.
LONG CDataObjectPool::CreatePool(LONG lSize, DWORD dwPoolCnt, DWORD dwSlip)
{
	TLock lo(this);
	if(dwPoolCnt < 0 || lSize < 0)
		return 0;
	m_dwBlockSize = lSize;
	m_dwLimitBlockCnt = dwPoolCnt;
	if(dwSlip < 2 || dwSlip > 4) 
		m_dwSlipLevel = 2UL;
	else 
		m_dwSlipLevel = dwSlip;

	// Pool size 만큼의 관리공간을 확보한다.
	m_vecObj.reserve(dwPoolCnt);

	// 한번 이상 call 할 수 없다.. 
	if(!CreateHeap( (lSize * dwPoolCnt) ))
		return FALSE;

	// Block 단위로 할당.. vector에 보관..
	_AllocMemory(dwPoolCnt);
	printf(" Create Pool : Created Pool size %d \n", dwPoolCnt);

	return m_dwIdleBlockCnt;
}

// 모든 메모리를 제거..
void CDataObjectPool::DestroyPool()
{
	TLock lo(this);

	DestroyHeap();
	m_hHeap = 0;
	m_vecObj.clear();
	m_dwBlockSize = 0UL;
	m_dwLimitBlockCnt = 0UL;
	m_dwIdleBlockCnt = 0UL;
}

// 
void * CDataObjectPool::Pop()
{
	TLock lo(this);
	if(m_dwIdleBlockCnt == 0)
	{
		_AllocMemory();
		printf(" Create Pool from Pop : Alloced Memory size %d \n", m_dwIdleBlockCnt);
	}
	void * pv = m_vecObj[--m_dwIdleBlockCnt];
	ASSERT(pv);
	memset(pv, 0, m_dwBlockSize);
	m_vecObj.pop_back();
	return pv;
}

void CDataObjectPool::Push(void * pV)
{
	TLock lo(this);
	if(_GetCapacity() == 0)
	{
		_FreeMemory();
		printf(" Create Pool Push: Alloced Memory size %d \n", m_dwIdleBlockCnt);
	}
	ASSERT(pV);
	m_vecObj.push_back(pV);
	m_dwIdleBlockCnt++;
// 아래 code의 문제점....... -.-
//	m_vecObj[m_dwIdleBlockCnt++] = pV;
}


////////////////////////////////////////////////////////////////////////////////////
// Control Heap .. 

bool CDataObjectPool::CreateHeap(size_t alloc_size, size_t max_size)
{
	if(m_hHeap)
		return FALSE;

	// max_size를 0으로 셋팅함으로써.. 필요시 heap이 증가 할 수 있도록 한다.
	m_hHeap = ::HeapCreate(HEAP_NO_SERIALIZE, alloc_size, max_size);

	return (m_hHeap == NULL) ? FALSE : TRUE;
}

void CDataObjectPool::DestroyHeap()
{
	if(!m_hHeap)
		return;
	::HeapDestroy(m_hHeap);
}

void * CDataObjectPool::_AllocHeap(size_t block_size)
{
	ASSERT(block_size);
	ASSERT(m_hHeap);

	
	void * _p = ::HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, block_size);
	DWORD dwRet = GetLastError();
	if(dwRet == STATUS_NO_MEMORY)
		printf("a lack of available memory or heap corruption \n");
	else if( dwRet == STATUS_ACCESS_VIOLATION)
		printf("heap corruption or improper function parameters \n");
	else 
		return _p;

	return NULL;
}

void CDataObjectPool::_FreeHeap(void * pv)
{
	::HeapFree(m_hHeap, HEAP_NO_SERIALIZE, pv);
}