#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include<Windows.h>
#include<WinSock2.h>
#include <iostream>

using namespace std;

enum CMD
{
	CMD_LOGIN,  // 登录消息
	CMD_LOGINRE,  // 登陆返回消息
	CMD_LOGOUT,  // 登出消息
	CMD_LOGOUTRE,  // 登出返回消息
	CMD_ERROR,  //错误消息
	CMD_LOGIN_BRO,  // 登录广播
	CMD_LOGOUT_BRO,  // 登出广播
};
struct pkgHeader
{
	short pkgLen;
	CMD cmd;
};

struct LoginData :public pkgHeader
{
	LoginData()
	{
		pkgLen = sizeof(LoginData);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char userWord[32];
};
struct LoginResult :public pkgHeader
{
	LoginResult()
	{
		pkgLen = sizeof(LoginResult);
		cmd = CMD_LOGINRE;
	}
	int result;
};
struct LogOutData :public pkgHeader
{
	LogOutData()
	{
		pkgLen = sizeof(LogOutData);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};
struct LogOutResult :public pkgHeader
{
	LogOutResult()
	{
		pkgLen = sizeof(LogOutResult);
		cmd = CMD_LOGOUTRE;
	}
	int result;
};
struct LoginBro :public pkgHeader
{
	LoginBro()
	{
		pkgLen = sizeof(LoginBro);
		cmd = CMD_LOGIN_BRO;
	}
	char userID[32];
};
struct LogoutBro :public pkgHeader
{
	LogoutBro()
	{
		pkgLen = sizeof(LogoutBro);
		cmd == CMD_LOGOUT_BRO;
	}
	char userID[32];
};

int process(SOCKET  _sock)
{
	char recBuf[1024];  //接收缓存区
	pkgHeader* recHeader;
	int recBufLen = recv(_sock, recBuf, sizeof(pkgHeader), 0);  //接收消息包头
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		recHeader = (pkgHeader*)recBuf;
		printf("接收到包头: 长度：%d,类型：%d\n", recHeader->pkgLen, recHeader->cmd);
	}
	switch (recHeader->cmd)  //查看包头的命令类型
	{
		case(CMD_LOGINRE):
		{
			
			int recBufLen = recv(_sock, recBuf+sizeof(pkgHeader), sizeof(LoginResult)-sizeof(pkgHeader), 0);
			LoginResult* loginResult=(LoginResult*)recBuf;
			if (loginResult->result == 202)
			{
				printf("登陆成功!\n");
			}
		}
			break;
		case(CMD_LOGOUTRE):
		{
			int recBufLen = recv(_sock, recBuf + sizeof(pkgHeader), sizeof(LogOutResult) - sizeof(pkgHeader), 0);
			LogOutResult* logoutResult = (LogOutResult*)recBuf;
			if (logoutResult->result == 202)
			{
				printf("登出成功!\n");
			}
		}
			break;
		case(CMD_LOGIN_BRO):
		{
			int recBufLen = recv(_sock, recBuf + sizeof(pkgHeader), sizeof(LoginBro) - sizeof(pkgHeader), 0);
			LoginBro* loginBro=(LoginBro*)recBuf;
			printf("ID为 %s 的用户登录服务器!\n", loginBro->userID);
		}
			break;
		case(CMD_LOGOUT_BRO):
		{
			int recBufLen = recv(_sock, recBuf + sizeof(pkgHeader), sizeof(LogoutBro) - sizeof(pkgHeader), 0);
			LogoutBro* logoutBro = (LogoutBro*)recBuf;
			printf("ID为 %s 的用户登出服务器!\n", logoutBro->userID);
		}
			break;
		default:
			break;
	}
	return 0;
}

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
	
	fd_set fdRead;

	while (true)
	{
		
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);


		//nfds  是集合fd_set中最后一个socket描述符+1, 用以表示集合范围
		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdRead,NULL,NULL,&t);  //select将更新这个集合,把其中不可读(不可写)的套节字去掉 
		if (ret < 0)
		{
			printf("select 任务结束.\n");
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			if(process(_sock)==-1)
				printf("服务器断开连接\n");
		}
		char client_cmd[32] = {};
		scanf("%s", client_cmd);
		if (strcmp(client_cmd, "exit") == 0)
			break;
		else if (strcmp(client_cmd, "login") == 0)
		{
			LoginData loginMse;
			printf("输入你的用户名: ");
			scanf("%s", loginMse.userName);
			printf("输入你的密码:");
			scanf("%s", loginMse.userWord);
			send(_sock, (const char*)&loginMse, sizeof(LoginData), 0);
			if (process(_sock) == -1)
				printf("服务器断开连接\n");
			
		}
		else if (strcmp(client_cmd, "logout") == 0)
		{
			LogOutData logoutMse;
			printf("输入你的用户名: ");
			scanf("%s", logoutMse.userName);
			send(_sock, (const char*)&logoutMse, sizeof(LogOutData), 0);
			if (process(_sock) == -1)
				printf("服务器断开连接\n");
			
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