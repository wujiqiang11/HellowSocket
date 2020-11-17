#include"TcpServer.hpp"
using namespace std;
int main()
{
	MyServer server(NULL, 4567, 0);
	server.WaitReq();

	getchar();
	return 0;

}



