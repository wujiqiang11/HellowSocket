#include"TcpServer.hpp"
using namespace std;
int main()
{
	MyServer server(NULL, 4567);
	while (server.keepRunning)
	{
		server.WaitReq(0);
		//server.otherServer();
	}

	getchar();
	return 0;

}



