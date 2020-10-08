#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include<Windows.h>
#include<WinSock2.h>
#include <iostream>

using namespace std;
enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};
struct pkgHeader
{
	short pkgLen;
	CMD cmd;
};

struct LoginData
{
	char userName[32];
	char userWord[32];
};
struct LoginResult
{
	int result;
};
struct LogOutData
{
	char userName[32];
};
struct LogOutResult
{
	int result;
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
		pkgHeader recHeader = {};
		int recBufLen = recv(_csock, (char*)&recHeader, sizeof(pkgHeader), 0);  //接收消息包头
		if (recBufLen <= 0)
		{
			printf("客户端退出\n");
			break;
		}
		else
		{
			printf("接收到包头: 长度：%d,类型：%d\n", recHeader.pkgLen,recHeader.cmd);
		}
		switch (recHeader.cmd)  //查看包头的命令类型
		{
			case(CMD_LOGIN):
			{
				LoginData recLoginData = {};
				recBufLen = recv(_csock, (char*)&recLoginData, sizeof(LoginData), 0);
				if (recBufLen > 0)
				{
					printf("接收到登录信息,用户名：%s, 密码：%s\n", recLoginData.userName, recLoginData.userWord);
					
					LoginResult sendLoginRe = { 202 };
					pkgHeader header = {};
					header.cmd = CMD_LOGIN;
					header.pkgLen = 0;
					send(_csock, (const char*)&header, sizeof(pkgHeader), 0);
					send(_csock, (const char*)&sendLoginRe, sizeof(LoginResult), 0);
				}
			}
				break;
			case(CMD_LOGOUT):
			{
				LogOutData recLogOutData = {};
				recBufLen = recv(_csock, (char*)&recLogOutData, sizeof(LogOutData), 0);
				if (recBufLen > 0)
				{
					printf("接收到退出登录信息，用户名:  %s\n", recLogOutData.userName);
					pkgHeader header = {};
					header.cmd = CMD_LOGOUT;
					header.pkgLen = 0;
					LogOutResult  sendLoginOutRe = { 202 };
					send(_csock, (const char*)&header, sizeof(pkgHeader), 0);
					send(_csock, (const  char*)&sendLoginOutRe, sizeof(LogOutResult), 0);
				}
			}
				break;
			default:
				pkgHeader header = {};
				header.cmd = CMD_ERROR;
				header.pkgLen = 0;
				send(_csock, (const char*)&header, sizeof(pkgHeader), 0);
				break;
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



