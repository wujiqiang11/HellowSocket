#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include<Windows.h>
#include<WinSock2.h>
#include <iostream>
#include<vector>

using namespace std;
enum CMD
{
	CMD_LOGIN,
	CMD_LOGINRE,
	CMD_LOGOUT,
	CMD_LOGOUTRE,
	CMD_ERROR
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

std::vector<SOCKET> g_clients;

int process(SOCKET  _csock)
{
	char recBuf[1024];  //接收缓存区
	pkgHeader* recHeader;
	int recBufLen = recv(_csock, recBuf, sizeof(pkgHeader), 0);  //接收消息包头
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
	case(CMD_LOGIN):
	{
		recBufLen = recv(_csock, recBuf + sizeof(pkgHeader), sizeof(LoginData) - sizeof(pkgHeader), 0);
		LoginData* loginMse = (LoginData*)recBuf;
		if (recBufLen > 0)
		{
			printf("接收到登录信息,用户名：%s, 密码：%s\n", loginMse->userName, loginMse->userWord);
			LoginResult loginReMse;
			loginReMse.result = 202;
			send(_csock, (const char*)&loginReMse, sizeof(LoginResult), 0);
		}
	}
	break;
	case(CMD_LOGOUT):
	{
		recBufLen = recv(_csock, recBuf + sizeof(pkgHeader), sizeof(LogOutData) - sizeof(pkgHeader), 0);
		LogOutData* logoutMse = (LogOutData*)recBuf;
		if (recBufLen > 0)
		{
			printf("接收到登出信息,用户名：%s\n", logoutMse->userName);
			LogOutResult logoutReMse;
			logoutReMse.result = 202;
			send(_csock, (const char*)&logoutReMse, sizeof(LogOutResult), 0);
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
	
	

	while (true)
	{
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		for (int n = (int)g_clients.size()-1; n >= 0; n--)
		{
			cout << n << endl;
			FD_SET(g_clients[n], &fdRead);
		}

		//nfds  是集合fd_set中最后一个socket描述符+1, 用以表示集合范围
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, NULL);  //select将更新这个集合,把其中不可读(不可写)的套节字去掉 
		if (ret < 0)
		{
			printf("select 任务结束.\n");
			break;
		}
		if (FD_ISSET(_sock, &fdRead))  //查看_sock是否还有可读请求
		{
			FD_CLR(_sock, &fdRead);
			//  4.accept 等待接受客户端连接
			sockaddr_in clientAdd = {};
			int AddLen = sizeof(sockaddr_in);
			SOCKET _csock = INVALID_SOCKET;
			_csock = accept(_sock, (sockaddr*)&clientAdd, &AddLen);
			if (_csock == INVALID_SOCKET)
			{
				printf("接收到无效 socket\n");
			}
			g_clients.push_back(_csock);
			printf("新的客户端加入, IP = %s \n", inet_ntoa(clientAdd.sin_addr));

			for (int i = 0; i < fdRead.fd_count; i++)
			{
				if (process(fdRead.fd_array[i]) == -1)
				{
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
					printf("客户端退出.\n");
					
				}
			}

		}

	}

	for (size_t n = 0; n < g_clients.size(); n++)
	{
		closesocket(g_clients[n]);
	}

	//  6.关闭socket closesocket
	closesocket(_sock);
	//------------------
	//清除windows socket环境
	WSACleanup();
	getchar();
	return 0;

}



