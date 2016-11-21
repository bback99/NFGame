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

// ������ ���ÿ� ó��..���� �޸𸮸� heap���� �Ҵ� �޾�, memory pool�� ����.
void CDataObjectPool::_AllocMemory(DWORD dwAppendCnt)
{
	TLock lo(this);

	if(dwAppendCnt > 0)	// ���ϴ� ���� or full�� ä���.
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
	else				// ������ ä���.
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
	if(dwRemoveCnt > 0)	// ���ϴ� ���� �Ǵ� ��ü�� ����.
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

// Heap memory Pool ����..
// ������ ũ���� ���� �޸𸮸� �ټ� ����.
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

	// Pool size ��ŭ�� ���������� Ȯ���Ѵ�.
	m_vecObj.reserve(dwPoolCnt);

	// �ѹ� �̻� call �� �� ����.. 
	if(!CreateHeap( (lSize * dwPoolCnt) ))
		return FALSE;

	// Block ������ �Ҵ�.. vector�� ����..
	_AllocMemory(dwPoolCnt);
	printf(" Create Pool : Created Pool size %d \n", dwPoolCnt);

	return m_dwIdleBlockCnt;
}

// ��� �޸𸮸� ����..
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
// �Ʒ� code�� ������....... -.-
//	m_vecObj[m_dwIdleBlockCnt++] = pV;
}


////////////////////////////////////////////////////////////////////////////////////
// Control Heap .. 

bool CDataObjectPool::CreateHeap(size_t alloc_size, size_t max_size)
{
	if(m_hHeap)
		return FALSE;

	// max_size�� 0���� ���������ν�.. �ʿ�� heap�� ���� �� �� �ֵ��� �Ѵ�.
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