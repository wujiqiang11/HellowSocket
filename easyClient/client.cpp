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
		char client_cmd[32] = {};
		scanf("%s", client_cmd);
		if (strcmp(client_cmd, "exit") == 0)
			break;
		else if (strcmp(client_cmd, "login") == 0)
		{
			LoginData login = {};
			printf("输入你的用户名: ");
			scanf("%s", login.userName);
			printf("输入你的密码:");
			scanf("%s", login.userWord);

			pkgHeader header = {};
			header.cmd = CMD_LOGIN;
			header.pkgLen = sizeof(LoginData);

			send(_sock, (const char*)&header, sizeof(pkgHeader), 0);
			send(_sock, (const char*)&login, sizeof(LoginData), 0);

			LoginResult result = {};
			int recBufLen = recv(_sock, (char*)&header, sizeof(pkgHeader), 0);
			recBufLen = recv(_sock, (char*)&result, sizeof(LoginResult), 0);

			if (result.result == 202)
			{
				printf("登陆成功!\n");
			}
		}
		else if (strcmp(client_cmd, "logout") == 0)
		{
			LogOutData logout = {};
			printf("输入你的用户名: ");
			scanf("%s", logout.userName);


			pkgHeader header = {};
			header.cmd = CMD_LOGOUT;
			header.pkgLen = sizeof(LogOutData);

			send(_sock, (const char*)&header, sizeof(pkgHeader), 0);
			send(_sock, (const char*)&logout, sizeof(LogOutData), 0);

			LogOutResult result = {};
			int recBufLen = recv(_sock, (char*)&header, sizeof(pkgHeader), 0);
			recBufLen = recv(_sock, (char*)&result, sizeof(LogOutResult), 0);

			if (result.result == 202)
			{
				printf("登出成功!\n");
			}
		}
		else
			printf("无效命令，请重新输入！\n");
	}
	//  4.关闭socket closesocket
	closesocket(_sock);
	//----------------
	//清除windows socket环境
	WSACleanup();
	getchar();
	return 0;
}