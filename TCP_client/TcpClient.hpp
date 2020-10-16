#ifndef _TcpClient_hpp_
#define _TcpClient_hpp_

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#ifdef _WIN32
#include<Windows.h>
#include<WinSock2.h>
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include <iostream>
#include<thread>
using namespace std;

class TcpClient
{
public:
	TcpClient();
	virtual ~TcpClient();
	void InitSocket();  //初始化Socket
	int Connect(char* ip, unsigned short port);  //连接服务器
	void CloseSocket();  //关闭socket
	//发送数据
	//接受数据
	//处理业务
private:
	SOCKET _sock;
};

TcpClient::TcpClient()
{
	_sock = INVALID_SOCKET;
}

TcpClient::~TcpClient()
{
}

void TcpClient::InitSocket()
{
#ifdef _WIN32
	//启动windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);  // 算出版本号
	WSADATA dat;
	WSAStartup(ver, &dat);

#endif
	//----------------
	//--- 使用socket API建立一个简易的TCP客户端
	//  1.建立一个socket
	_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
		printf("建立 Socket 失败!\n");
	else
	{
		printf("建立 Socket 成功!\n");
	}
}

int TcpClient::Connect(char* ip, unsigned short port)
{
	//  2.连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr("ip");
#endif
	int net = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (net == INVALID_SOCKET)
	{
		printf("连接错误!\n");
	}
	else
	{
		printf("连接服务器成功!\n");
	}
	return ret;
}

void TcpClient::CloseSocket()
{
	//  4.关闭socket closesocket
#ifdef _WIN32	
	closesocket(_sock);
	//----------------
	//清除windows socket环境
	WSACleanup();
#else
	close(_sock);
#endif
}

#endif // !_TcpClient_hpp_
