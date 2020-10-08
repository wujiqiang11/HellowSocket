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
	//--- 使用socket API建立简易TCP服务端
	//  1.建立一个socket
	SOCKET _sock =socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//  2.bind 绑定用于接受客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;  //socket protocol
	_sin.sin_port = htons(4567);  //host to net unsighned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("ERROR, 服务端绑定端口失败！\n");
	}
	printf("服务端绑定端口成功！\n");
	//  3.listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("错误，监听网络端口失败..\n");
	}
	else
	{
		printf("监听网络端口成功\n");
	}
	//  4.accept 等待接受客户端连接
	sockaddr_in clientAdd = {};
	int AddLen = sizeof(sockaddr_in);
	SOCKET _csock = INVALID_SOCKET;
	
	_csock = accept(_sock, (sockaddr*)&clientAdd, &AddLen);
	if (_csock == INVALID_SOCKET)
	{
		printf("接收到无效 socket\n");
	}
	printf("新的客户端加入, IP = %s \n", inet_ntoa(clientAdd.sin_addr));

	while (true)
	{
		char recBuf[256] = {};
		int recBufLen = recv(_csock, recBuf, 256, 0);
		if (recBufLen <= 0)
		{
			printf("客户端退出\n");
			break;
		}
		else
		{
			printf("接收到命令: %s\n", recBuf);
		}

		//  5.send 向客户端发送数据
		if (strcmp(recBuf, "getInfo") == 0)
		{
			DataPkg msgbuf =  { 18,"wujiqiang" };
			send(_csock, (const char*)&msgbuf, sizeof(DataPkg), 0);
		}
		else
		{
			char msgbuf[] = "?????";
			send(_csock, msgbuf, strlen(msgbuf) + 1, 0);
		}
		
	}
	//  6.关闭socket closesocket
	closesocket(_sock);
	//------------------
	//清除windows socket环境
	WSACleanup();
	getchar();
	return 0;

}



