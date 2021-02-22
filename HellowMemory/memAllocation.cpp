#include"memAllocation.h"
#include"memoryAdmin.hpp"
void* operator new(size_t nSize)
{
	void* pReturn = memoryAdmin::getInstance()->allocMem(nSize);
	return pReturn;
}

void operator delete(void* pTemp)
{
	memoryAdmin::getInstance()->freeMem(pTemp);
}

void* operator new[](size_t nSize)
{
	void* pReturn = memoryAdmin::getInstance()->allocMem(nSize);
	return pReturn;
}

void operator delete[](void* pTemp)
{
	memoryAdmin::getInstance()->freeMem(pTemp);
}