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
#include"Message.hpp"
#define RECV_BUF_SIZE 10240  //第一接收缓存区的大小
using namespace std;

class TcpClient
{
public:
	TcpClient();
	virtual ~TcpClient();
	void InitSocket();  //初始化Socket
	int Connect(const char* ip, unsigned short port);  //连接服务器
	void CloseClientSock();  //关闭socket
	//发送数据包
	void SendData(pkgHeader* sendHeader);
	//接受数据包
	int RecvData();
	//接受测试数据包
	int RecvTestData();
	//select 接受网络请求消息
	void KeepRun();
	//业务层面接收网络消息
	void ResMse(pkgHeader* recHeader);
	//业务层面处理用户命令
	void ProcessCMD();
	//发送测试数据包
	void SendTest();
	std::thread RecvCMD();  //接收用户输入
	bool keep_running;
	TestPkg testpkg;
	char recBuf[RECV_BUF_SIZE] = {};  //第一接收缓存区
	char recMesBuf[RECV_BUF_SIZE * 10] = {};  //第二接收缓存区
	int LastPos_MesBuf;  //第二接收缓存区数据的尾部位置
private:
	SOCKET _sock;
	
};

TcpClient::TcpClient()
{
	_sock = INVALID_SOCKET;
	keep_running = true;
	LastPos_MesBuf = 0;
	InitSocket();
}

TcpClient::~TcpClient()
{
	CloseClientSock();
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
	if (_sock != INVALID_SOCKET)
	{
		printf("关闭旧链接\n");
		CloseClientSock();
	}
	//  1.建立一个socket
	_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
		printf("建立 Socket 失败!\n");
	else
	{
		printf("建立 Socket 成功!\n");
	}
}

int TcpClient::Connect(const char* ip, unsigned short port)
{
	//  2.连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
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
	return net;
}

void TcpClient::CloseClientSock()
{
	if (_sock != INVALID_SOCKET)
	{
		printf("关闭socket");
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
}

void TcpClient::KeepRun()
{
	if (_sock != INVALID_SOCKET)
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);
		//nfds  是集合fd_set中最后一个socket描述符+1, 用以表示集合范围
		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdRead, NULL, NULL, &t);  //select将更新这个集合,把其中不可读(不可写)的套节字去掉  
		if (ret < 0)
		{
			printf("select function end.\n");
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			if (RecvData() == -1)
			//if (RecvTestData() == -1)
			{
				printf("Disconnect to server.\n");
				keep_running = false;
			}
		}
	}
}

int TcpClient::RecvData()
{

	int recBufLen = (int)recv(_sock, recBuf, RECV_BUF_SIZE, 0);  //将接收到的数据一次装入第一缓冲区
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		memcpy(recMesBuf + LastPos_MesBuf, recBuf, recBufLen);
		LastPos_MesBuf += recBufLen;
		while (LastPos_MesBuf >= sizeof(pkgHeader))  //若第二缓冲区的数据长度大于一个包头部长度，则接收该头部
		{
			pkgHeader* recHeader = (pkgHeader*)recMesBuf;
			//printf("接收到包头: 长度：%d,类型：%d\n", recHeader->pkgLen, recHeader->cmd);
			//若第二缓冲区数据长度大于接收包的长度，则处理这个包，否则说明接收到的数据包不完整
			if (LastPos_MesBuf >= recHeader->pkgLen)
			{
				ResMse(recHeader);
				int _unproLen = LastPos_MesBuf - recHeader->pkgLen;  //未处理的数据长度
				int _proLen = recHeader->pkgLen;  //当前正在处理的数据长度
				memcpy(recMesBuf, recMesBuf + _proLen, _unproLen);
				LastPos_MesBuf = LastPos_MesBuf - recHeader->pkgLen;
			}
			else
			{
				break;
				printf("break.\n");
			}
		}
		return 0;
	}
	/*
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		pkgHeader* recHeader = (pkgHeader*)recBuf;
		printf("接收到包头: 长度：%d,类型：%d\n", recHeader->pkgLen, recHeader->cmd);
		recv(_sock, recBuf + sizeof(pkgHeader), recHeader->pkgLen - sizeof(pkgHeader), 0);
		ResMse(recHeader);
	}
	return 0;
	*/
}


int TcpClient::RecvTestData()
{
	int recBufLen = recv(_sock, recBuf, 409600, 0);  //接收消息包头
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		printf("接收到 %d 字节数据\n", recBufLen);
		SendData(&testpkg);
		return 0;
	}

}

void TcpClient::ResMse(pkgHeader* recHeader)
{
	switch (recHeader->cmd)  //查看包头的命令类型
	{
	case(CMD_LOGINRE):
	{

		LoginResult* loginResult = (LoginResult*)recHeader;
		if (loginResult->result == 202)
		{
			printf("登陆成功!\n");
		}
	}
	break;
	case(CMD_LOGOUTRE):
	{

		LogOutResult* logoutResult = (LogOutResult*)recHeader;
		if (logoutResult->result == 202)
		{
			printf("登出成功!\n");
		}
	}
	break;
	case(CMD_LOGIN_BRO):
	{
		LoginBro* loginBro = (LoginBro*)recHeader;
		printf("ID为 %s 的用户登录服务器!\n", loginBro->userID);
	}
	break;
	case(CMD_LOGOUT_BRO):
	{
		LogoutBro* logoutBro = (LogoutBro*)recHeader;
		printf("ID为 %s 的用户登出服务器!\n", logoutBro->userID);
	}
	break;
	case(CMD_TEST):
	{
		//printf("接收到测试数据包!\n");
		break;
	}
	default:
		printf("接收到无效消息\n");
		break;
	}
}

void TcpClient::SendData(pkgHeader* sendHeader)
{
	if (_sock != INVALID_SOCKET)
	{
		send(_sock, (const char*)sendHeader, sendHeader->pkgLen, 0);
	}
}

void TcpClient::ProcessCMD()
{
	while (true)
	{
		char client_cmd[32] = {};
		scanf("%s", client_cmd);
		if (strcmp(client_cmd, "exit") == 0)
		{
			keep_running = false;
			break;
		}
		else if (strcmp(client_cmd, "login") == 0)
		{
			LoginData loginMse;
			printf("请输入你的用户名: ");
			scanf("%s", loginMse.userName);
			printf("请输入你的密码:");
			scanf("%s", loginMse.userWord);
			SendData(&loginMse);
			//send(_sock, (const char*)&loginMse, sizeof(LoginData), 0);

		}
		else if (strcmp(client_cmd, "logout") == 0)
		{
			LogOutData logoutMse;
			printf("请输入你的用户名: ");
			scanf("%s", logoutMse.userName);
			SendData(&logoutMse);
			//send(_sock, (const char*)&logoutMse, sizeof(LogOutData), 0);
		}
		else if (strcmp(client_cmd, "test") == 0)
		{	
			int sum = 0;
			while (true)
			{
				SendData(&testpkg);
				//printf("发送测试数据包 %d\n",++sum);
			}
			//send(_sock, (const char*)&logoutMse, sizeof(LogOutData), 0);
		}

		else
			printf("无效输入!\n");
	}
}

std::thread TcpClient::RecvCMD()
{
	std::thread t(&TcpClient::ProcessCMD, this);  //开一个输入线程
	return t;
}

void TcpClient::SendTest()
{
	SendData(&testpkg);
}
#endif // !_TcpClient_hpp_
