#include"Message.hpp"
#include"TcpClient.hpp"
#include"High_resolution_timer.hpp"
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

const int client_num = 1;
TcpClient* client[client_num];
const int Thread_num = 1;  //客户端开启的发送线程数

void SendThread(int n, int num, MyTimer* timer)
{
	for (int i = n*num; i < (n+1)*num; i++)
	{
		client[i] = new TcpClient();
		//client[i]->Connect("172.18.166.60", 4567);
		client[i]->Connect("127.0.0.1", 4567);
	}
	int sendSum = 0;
	while (keep_running)
	{
		for (int i = n * num; i < (n + 1) * num; i++)
		{
			client[i]->SendTest();
			client[i]->KeepRun();
			sendSum += client[i]->getSendNum();
		}
		if (timer->GetSeconds() >= 1)
		{
			printf("seconds:%d, 每秒send:%d\n", timer->GetSeconds(), sendSum);
			timer->update();
			sendSum = 0;
		}
	}
	for (int i = n * num; i < (n + 1) * num; i++)
	{
		delete[] client[i];
	}
}



int main()
{
	std::thread t(inputCMD);
	t.detach();
	MyTimer* timer[Thread_num];
	for (int i = 0; i < Thread_num; i++)
	{
		timer[i] = new MyTimer();
	}

	for (int i = 0; i < Thread_num; i++)
	{
		std::thread t(SendThread, i, client_num / Thread_num, timer[i]);
		t.detach();
	}
	for (int i = 0; i < Thread_num; i++)
	{
		delete[] timer[i];
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