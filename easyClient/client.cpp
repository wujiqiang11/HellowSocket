#include"Message.hpp"
#include"TcpClient.hpp"
using namespace std;

bool keep_running = true;
void inputCMD()
{
	while (true)
	{
		char client_cmd[32] = {};
		scanf("%s", client_cmd);
		if (strcmp(client_cmd, "exit") == 0)
		{
			printf("exit!\n");
			keep_running = false;
			break;
		}
		else
			printf("无效输入!\n");
	}
}

int main()
{
	//const int client_num = FD_SETSIZE - 1;
	const int client_num = 1000;
	TcpClient* client[client_num];
	for (int i = 0; i < client_num; i++)
	{
		client[i] = new TcpClient();
		client[i]->Connect("172.18.166.60", 4567);
		//client[i]->Connect("127.0.0.1", 4567);
	}
	std::thread t(inputCMD);
	t.detach();
	while (keep_running)
	{
		for (int i = 0; i < client_num; i++)
		{
			client[i]->SendTest();
			client[i]->KeepRun();
		}
	}
	for (int i = 0; i < client_num; i++)
	{
		delete [] client[i];
	}
	//client.Connect("127.0.0.1", 4567);
	/*
	std::thread t=client.RecvCMD();  //开一个输入线程
	t.detach();
	while (client.keep_running)
	{
		client.KeepRun();
	}
	*/
	getchar();
	return 0;
}