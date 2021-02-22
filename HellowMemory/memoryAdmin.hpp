#include<assert.h>
#include<stdlib.h>

class memoryBuf;

class memoryBlock  //内存块头部
{
public:
	memoryBlock() {}
	~memoryBlock() {}
	memoryBlock* nextblock;  //指向下一个可用内存块头部的指针
	memoryBuf* thisBuf;  //管理本内存块头部的内存池
	int cite_num;  //该内存块被引用的数量
	bool ifInBuf;  //该内存块是否在内存池中
	size_t memSize;  //该内存块所指向的内存单元大小
};

class memoryBuf  //内存池
{
public:
	memoryBuf() 
	{ 
		_blockSize = sizeof(memoryBlock); 
		_memNum = 0;
		_memSize = 0;
		_PHeader = nullptr;
		_Buf_P = nullptr;
	}
	~memoryBuf() 
	{
		if (_Buf_P)
			free(_Buf_P);
	}
	void initBuf();
	memoryBlock* allocMem(size_t nSize);  //申请内存
	void freeMem(memoryBlock* pTemp);  //释放内存
protected:
	size_t _memSize;  //该内存池单个内存单元的大小
	size_t _memNum;  //该内存池内存单元的数量
	size_t _blockSize;  //内存块头部的大小
	memoryBlock* _PHeader;  //该内存池的头部指针，指向第一块可以使用的内存单元
	char* _Buf_P;  //指向该内存池所管理的内存空间
};

void memoryBuf::initBuf()
{
	_Buf_P = (char* )malloc((_memSize + _blockSize) * _memNum);
	_PHeader = (memoryBlock* )_Buf_P;
	_PHeader->thisBuf = this;
	_PHeader->cite_num = 0;
	_PHeader->ifInBuf = true;
	_PHeader->memSize = _memSize;
	memoryBlock* last_P = _PHeader;
	memoryBlock* next_P;
	for (int i = 1; i < _memNum; i++)
	{
		next_P = last_P + (_memSize + _blockSize);
		last_P->nextblock = next_P;
		next_P->thisBuf = this;
		next_P->cite_num = 0;
		next_P->ifInBuf = true;
		next_P->memSize = _memSize;
		last_P = next_P;
	}
	last_P->nextblock = nullptr;
}

memoryBlock* memoryBuf::allocMem(size_t nSize)
{
	if (!_Buf_P)  //内存池还没有分配内存，则先对内存池进行初始化
	{
		initBuf();
	}
	memoryBlock* pReturn;
	if (!_PHeader)  //内存池已经分配满了,需要跟操作系统重新申请内存
	{
		pReturn = (memoryBlock*)malloc(nSize + _blockSize);
		pReturn->nextblock = nullptr;
		pReturn->cite_num = 0;
		pReturn->ifInBuf = false;
		pReturn->memSize = nSize;
		pReturn->thisBuf = this;
	}
	else  //内存池还有可分配的内存块，直接将该块分配出去
	{
		pReturn = _PHeader;
		_PHeader = _PHeader->nextblock;
		assert(pReturn->cite_num == 0);
		pReturn->cite_num++;
	}
	return pReturn;
}

void memoryBuf::freeMem(memoryBlock* pTemp)
{
	assert(pTemp->cite_num == 1);
	if (--pTemp->cite_num > 0)
	{
		return;  //为共享内存保留的接口
	}
	if (pTemp->ifInBuf)  //该内存块属于本内存池
	{
		pTemp->nextblock = _PHeader;
		_PHeader = pTemp;
	}
	else  //该内存块是向系统申请的
	{
		free(pTemp);
	}
}

template<size_t memSize, size_t memNum>
class memoryBuffer:public memoryBuf  //内存池模板类，可以更加直观的初始化内存池
{
public:
	memoryBuffer()
	{
		_memSize = memSize;
		_memNum = memNum;
		initBuf();
	}
	//~memoryBuffer() {}
};


class memoryAdmin  //内存池管理工具
{
public:
	static memoryAdmin* getInstance()
	{
		return p;
	}
	void* allocMem(size_t memSize);
	void freeMem(void* p);

private:
	memoryAdmin();
	~memoryAdmin() {}
	memoryBuffer<64, 10> memBuf64;  //单元大小为64字节的内存池
	memoryBuffer<128, 10> memBuf128;  //单元大小为128字节的内存池
	size_t maxSize;  //内存池支持分配的最大内存单元大小
	static memoryAdmin* p;
	memoryBuf* Size2Buf[128 + 1];  //由内存块大小映射到对应内存池的映射数组
	void initMapping(int begin, int end, memoryBuf* PBuf);  //初始化Size2Buf数组
};
memoryAdmin* memoryAdmin::p = new memoryAdmin();
memoryAdmin::memoryAdmin()
{
	initMapping(0, 64, &memBuf64);
	initMapping(65, 128, &memBuf128);
	maxSize = 128;
}

void memoryAdmin::initMapping(int begin, int end, memoryBuf* PBuf)
{
	for (int i = begin; i <= end; i++)
	{
		Size2Buf[i] = PBuf;
	}
}

void* memoryAdmin::allocMem(size_t memSize)
{
	memoryBlock* pReturn;
	if (memSize > maxSize)  //需要分配的内存超过内存池的大小，需要直接向系统申请
	{
		pReturn = (memoryBlock*)malloc(memSize+sizeof(memoryBlock));
		pReturn->nextblock = nullptr;
		pReturn->cite_num = 0;
		pReturn->ifInBuf = false;
		pReturn->memSize = memSize;
		pReturn->thisBuf = nullptr;
	}
	else
	{
		pReturn = Size2Buf[memSize]->allocMem(memSize);
	}
	return (char*)pReturn + sizeof(memoryBlock);
}

void memoryAdmin::freeMem(void* p)
{
	memoryBlock* pTemp = (memoryBlock*)((char*)p - sizeof(memoryBlock));
	if (!pTemp->thisBuf)  //该内存块不属于任何一个内存池，则由内存管理单元将该内存块释放
	{
		free(pTemp);
	}
	else  //将该内存块交由其内存池释放
	{
		pTemp->thisBuf->freeMem(pTemp);
	}
}

