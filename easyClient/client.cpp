#include"Message.hpp"
#include"TcpClient.hpp"
using namespace std;


int main()
{

	TcpClient client;
	client.Connect("172.18.166.60",4567);
	std::thread t=client.RecvCMD();  //��һ�������߳�
	t.detach();
	while (client.keep_running)
	{
		client.KeepRun();
	}
	getchar();
	return 0;
}