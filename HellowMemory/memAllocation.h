#pragma once
#ifndef _MEM_ALLOCA_
#define _MEM_ALLOCA_
void* operator new(size_t);
void operator delete(void*);
void* operator new[](size_t);
void operator delete[](void*);

#endif // !_MEM_ALLOCA_
