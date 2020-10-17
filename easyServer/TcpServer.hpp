#ifndef _TcpServer_
#define _TcpServer_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
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
#include<vector>
#include"Message.hpp"
using namespace std;

class MyServer
{
public:
	MyServer(const char* ip, unsigned short port);
	virtual ~MyServer();
	void InitServer(const char* ip, unsigned short port);  //初始化服务器(1. 建立SOCKET 2.绑定端口 3.开始监听)
	void CloseSocket();  //关闭SOCKET
	void SendData(SOCKET _csock, pkgHeader* sendHeader);  //发送数据包
	int RecvData(SOCKET _csock);  //接受数据包(拆包)
	void ProcessReq(SOCKET _csock, pkgHeader* recHeader);  //处理客户端请求
	void WaitReq(int seconds);  //等待客户端请求
	void otherServer();
	bool keepRunning;
private:
	std::vector<SOCKET> g_clients;
	SOCKET _sock;
	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExp;
};

MyServer::MyServer(const char* ip, unsigned short port)
{
	_sock = INVALID_SOCKET;
	InitServer(ip, port);
	keepRunning = true;
}

MyServer::~MyServer()
{
}

void MyServer::InitServer(const char* ip, unsigned short port)
{
#ifdef _WIN32
	//启动windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);  // 算出版本号
	WSADATA dat;
	WSAStartup(ver, &dat);

#endif
	//----------------
	//--- 使用socket API建立简易TCP服务端
	//  1.建立一个socket
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//  2.bind 绑定用于接受客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;  //socket protocol
	_sin.sin_port = htons(port);  //host to net unsighned short
	if (ip)
	{
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);	// inet_addr("127.0.0.1");
#else
		_sin.sin_addr.s_addr = inet_addr(ip);	// inet_addr("127.0.0.1");
#endif
	}
	else
	{
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;	// inet_addr("127.0.0.1");
#else
		_sin.sin_addr.s_addr = INADDR_ANY;	// inet_addr("127.0.0.1");
#endif
	}

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
}

void MyServer::CloseSocket()
{
#ifdef _WIN32
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		closesocket(g_clients[n]);
	}
	//  6.关闭socket closesocket
	closesocket(_sock);
	//------------------
	//清除windows socket环境
	WSACleanup();
#else
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		close(g_clients[n]);
	}
	//  6.关闭socket closesocket
	close(_sock);
#endif
}

void MyServer::SendData(SOCKET _csock, pkgHeader* sendHeader)
{
	if (_sock != INVALID_SOCKET)
	{
		send(_csock, (const char*)sendHeader, sendHeader->pkgLen, 0);
	}
}

int MyServer::RecvData(SOCKET _csock)
{
	char recBuf[4096];  //接收缓存区
	int recBufLen = recv(_csock, recBuf, sizeof(pkgHeader), 0);  //接收消息包头
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		pkgHeader* recHeader = (pkgHeader*)recBuf;
		printf("接收到包头: 长度：%d,类型：%d\n", recHeader->pkgLen, recHeader->cmd);
		recv(_csock, recBuf + sizeof(pkgHeader), recHeader->pkgLen - sizeof(pkgHeader), 0);
		ProcessReq(_csock, recHeader);
	}
	return 0;
}

void MyServer::ProcessReq(SOCKET _csock, pkgHeader* recHeader)
{
	switch (recHeader->cmd)  //查看包头的命令类型
	{
	case(CMD_LOGIN):
	{

		LoginData* loginMse = (LoginData*)recHeader;
		printf("接收到登录信息,用户名：%s, 密码：%s\n", loginMse->userName, loginMse->userWord);
		LoginResult loginReMse;
		loginReMse.result = 202;
		SendData(_csock, &loginReMse);
		//printf("发送登录返回\n");

		LoginBro loginBro;  //新用户登录广播消息
#ifdef _WIN32
		strcpy_s(loginBro.userID, loginMse->userName);
#else
		strcpy(loginBro.userID, loginMse->userName);
#endif
		for (auto iter = g_clients.begin(); iter != g_clients.end(); iter++)
		{
			if (*iter == _csock)
				continue;
			else
			{
				//printf("发送登录广播\n");
				SendData(*iter, &loginBro);
			}
		}
	}
	break;
	case(CMD_LOGOUT):
	{
		
		LogOutData* logoutMse = (LogOutData*)recHeader;
		printf("接收到登出信息,用户名：%s\n", logoutMse->userName);
		LogOutResult logoutReMse;
		logoutReMse.result = 202;
		SendData(_csock, &logoutReMse);
		//printf("发送登出返回\n");


		LogoutBro logoutBro;  //新用户登出广播消息
#ifdef _WIN32
		strcpy_s(logoutBro.userID, logoutMse->userName);
#else
		strcpy(logoutBro.userID, logoutMse->userName);
#endif
		for (auto iter = g_clients.begin(); iter != g_clients.end(); iter++)
		{
			if (*iter == _csock)
				continue;
			else
			{
				SendData(*iter, &logoutBro);
				printf("发送登出广播\n");
			}
		}
	}
	break;
	default:
		ErrorPkg errorPkg;
		errorPkg.error_code = 404;
		SendData(_csock, &errorPkg);
		printf("发送错误信息\n");
		break;
	}
}

void MyServer::WaitReq(int seconds)
{
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdExp);

	FD_SET(_sock, &fdRead);
	FD_SET(_sock, &fdWrite);
	FD_SET(_sock, &fdExp);

	SOCKET max_sock = _sock;

	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		FD_SET(g_clients[n], &fdRead);
		if (g_clients[n] > max_sock)
		{
			max_sock = g_clients[n];
		}
	}

	//nfds  是集合fd_set中最后一个socket描述符+1, 用以表示集合范围
	timeval t = { seconds,0 };
	int ret = select(max_sock + 1, &fdRead, &fdWrite, &fdExp, &t);  //select将更新这个集合,把其中不可读(不可写)的套节字去掉 
	if (ret < 0)
	{
		printf("select 任务结束.\n");
		keepRunning = false;
	}
	if (FD_ISSET(_sock, &fdRead))  //查看_sock是否还有可读请求
	{
		FD_CLR(_sock, &fdRead);
		//  4.accept 等待接受客户端连接
		sockaddr_in clientAdd = {};
		int AddLen = sizeof(sockaddr_in);
		SOCKET _csock = INVALID_SOCKET;
#ifdef _WIN32
		_csock = accept(_sock, (sockaddr*)&clientAdd, &AddLen);
#else
		_csock = accept(_sock, (sockaddr*)&clientAdd, (socklen_t*)&AddLen);
#endif
		if (_csock == INVALID_SOCKET)
		{
			printf("接收到无效 socket\n");
		}
		g_clients.push_back(_csock);
		printf("新的客户端加入, IP = %s \n", inet_ntoa(clientAdd.sin_addr));
	}
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		if (FD_ISSET(g_clients[n], &fdRead))
		{
			if (RecvData(g_clients[n]) == -1)
			{
				auto iter = g_clients.begin() + n;
				g_clients.erase(iter);
				printf("客户端退出.\n");
			}
		}
	}
}

void MyServer::otherServer()
{
	Sleep(0.5 * 1000);
	printf("其他服务进行中\n");
}
#endif // _TcpServer_
