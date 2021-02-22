#include<assert.h>
#include<stdlib.h>

class memoryBuf;

class memoryBlock  //�ڴ��ͷ��
{
public:
	memoryBlock() {}
	~memoryBlock() {}
	memoryBlock* nextblock;  //ָ����һ�������ڴ��ͷ����ָ��
	memoryBuf* thisBuf;  //�����ڴ��ͷ�����ڴ��
	int cite_num;  //���ڴ�鱻���õ�����
	bool ifInBuf;  //���ڴ���Ƿ����ڴ����
	size_t memSize;  //���ڴ����ָ����ڴ浥Ԫ��С
};

class memoryBuf  //�ڴ��
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
	memoryBlock* allocMem(size_t nSize);  //�����ڴ�
	void freeMem(memoryBlock* pTemp);  //�ͷ��ڴ�
protected:
	size_t _memSize;  //���ڴ�ص����ڴ浥Ԫ�Ĵ�С
	size_t _memNum;  //���ڴ���ڴ浥Ԫ������
	size_t _blockSize;  //�ڴ��ͷ���Ĵ�С
	memoryBlock* _PHeader;  //���ڴ�ص�ͷ��ָ�룬ָ���һ�����ʹ�õ��ڴ浥Ԫ
	char* _Buf_P;  //ָ����ڴ����������ڴ�ռ�
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
	if (!_Buf_P)  //�ڴ�ػ�û�з����ڴ棬���ȶ��ڴ�ؽ��г�ʼ��
	{
		initBuf();
	}
	memoryBlock* pReturn;
	if (!_PHeader)  //�ڴ���Ѿ���������,��Ҫ������ϵͳ���������ڴ�
	{
		pReturn = (memoryBlock*)malloc(nSize + _blockSize);
		pReturn->nextblock = nullptr;
		pReturn->cite_num = 0;
		pReturn->ifInBuf = false;
		pReturn->memSize = nSize;
		pReturn->thisBuf = this;
	}
	else  //�ڴ�ػ��пɷ�����ڴ�飬ֱ�ӽ��ÿ�����ȥ
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
		return;  //Ϊ�����ڴ汣���Ľӿ�
	}
	if (pTemp->ifInBuf)  //���ڴ�����ڱ��ڴ��
	{
		pTemp->nextblock = _PHeader;
		_PHeader = pTemp;
	}
	else  //���ڴ������ϵͳ�����
	{
		free(pTemp);
	}
}

template<size_t memSize, size_t memNum>
class memoryBuffer:public memoryBuf  //�ڴ��ģ���࣬���Ը���ֱ�۵ĳ�ʼ���ڴ��
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


class memoryAdmin  //�ڴ�ع�����
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
	memoryBuffer<64, 10> memBuf64;  //��Ԫ��СΪ64�ֽڵ��ڴ��
	memoryBuffer<128, 10> memBuf128;  //��Ԫ��СΪ128�ֽڵ��ڴ��
	size_t maxSize;  //�ڴ��֧�ַ��������ڴ浥Ԫ��С
	static memoryAdmin* p;
	memoryBuf* Size2Buf[128 + 1];  //���ڴ���Сӳ�䵽��Ӧ�ڴ�ص�ӳ������
	void initMapping(int begin, int end, memoryBuf* PBuf);  //��ʼ��Size2Buf����
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
	if (memSize > maxSize)  //��Ҫ������ڴ泬���ڴ�صĴ�С����Ҫֱ����ϵͳ����
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
	if (!pTemp->thisBuf)  //���ڴ�鲻�����κ�һ���ڴ�أ������ڴ����Ԫ�����ڴ���ͷ�
	{
		free(pTemp);
	}
	else  //�����ڴ�齻�����ڴ���ͷ�
	{
		pTemp->thisBuf->freeMem(pTemp);
	}
}

