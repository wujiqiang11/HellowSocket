#include"Message.hpp"
#include"TcpClient.hpp"
using namespace std;


int main()
{

	TcpClient client;
	client.Connect("192.168.77.130",4567);
	std::thread t=client.RecvCMD();  //��һ�������߳�
	t.detach();
	while (client.keep_running)
	{
		client.KeepRun();
	}
	getchar();
	return 0;
}