#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include<Windows.h>
#include<WinSock2.h>
#include <iostream>

using namespace std;

struct DataPkg
{
	int age;
	char name[32];
};

int main()
{
	//启动windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);  // 算出版本号
	WSADATA dat;
	WSAStartup(ver, &dat);
	//----------------
	//--- 使用socket API建立一个简易的TCP客户端
	//  1.建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
		printf("建立Socket失败\n");
	else
	{
		printf("建立Socket成功..\n");
	}
	//  2.连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int net = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (net == INVALID_SOCKET)
	{
		printf("ERROR 连接失败！\n");
	}
	else
	{
		printf("连接成功\n");
	}
	
	while (true)
	{
		char sendBuf[256] = {};
		scanf("%s", sendBuf);
		if (strcmp(sendBuf, "exit") == 0)
			break;
		else
		{
			send(_sock, sendBuf, strlen(sendBuf) + 1, 0);
		}
		//  3.接受服务器信息 revc
		char recBuf[256] = {};
		int nlen = recv(_sock, recBuf, 256, 0);
		if (nlen > 0)
		{
			DataPkg* rec = (DataPkg*)recBuf;
			printf("接收到数据: 姓名：%s, 年龄：%d\n", rec->name,rec->age);
		}
	}
	//  4.关闭socket closesocket
	closesocket(_sock);
	//----------------
	//清除windows socket环境
	WSACleanup();
	getchar();
	return 0;
}