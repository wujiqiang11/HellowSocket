#include"memAllocation.h"
int main()
{
	char* data1 = new char[128];
	delete[] data1;
	char* data2 = new char;
	delete data2;
	char* data3 = new char[64];
	delete[] data3;
	char* data4 = new char[55];
	delete[] data4;

	return 0;
}