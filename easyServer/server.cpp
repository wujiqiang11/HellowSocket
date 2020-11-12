#include"TcpServer.hpp"
using namespace std;
int main()
{
	MyServer server(NULL, 4567, 0);
	while (server.keepRunning)
	{
		server.WaitReq();
		//server.otherServer();
	}

	getchar();
	return 0;

}



