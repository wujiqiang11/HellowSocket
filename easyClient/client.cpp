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

int main()
{
	//����windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);  // ����汾��
	WSADATA dat;
	WSAStartup(ver, &dat);
	//----------------
	//--- ʹ��socket API����һ�����׵�TCP�ͻ���
	//  1.����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
		printf("����Socketʧ��\n");
	else
	{
		printf("����Socket�ɹ�..\n");
	}
	//  2.���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int net = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (net == INVALID_SOCKET)
	{
		printf("ERROR ����ʧ�ܣ�\n");
	}
	else
	{
		printf("���ӳɹ�\n");
	}
	
	while (true)
	{
		char client_cmd[32] = {};
		scanf("%s", client_cmd);
		if (strcmp(client_cmd, "exit") == 0)
			break;
		else if (strcmp(client_cmd, "login") == 0)
		{
			LoginData loginMse;
			printf("��������û���: ");
			scanf("%s", loginMse.userName);
			printf("�����������:");
			scanf("%s", loginMse.userWord);
			send(_sock, (const char*)&loginMse, sizeof(LoginData), 0);

			LoginResult result;
			int recBufLen = recv(_sock, (char*)&result, sizeof(LoginResult), 0);
			if (result.result == 202)
			{
				printf("��½�ɹ�!\n");
			}
		}
		else if (strcmp(client_cmd, "logout") == 0)
		{
			LogOutData logoutMse;
			printf("��������û���: ");
			scanf("%s", logoutMse.userName);
			send(_sock, (const char*)&logoutMse, sizeof(LogOutData), 0);

			LogOutResult result = {};
			int recBufLen = recv(_sock, (char*)&result, sizeof(LogOutResult), 0);

			if (result.result == 202)
			{
				printf("�ǳ��ɹ�!\n");
			}
		}
		else
			printf("��Ч������������룡\n");
	}
	//  4.�ر�socket closesocket
	closesocket(_sock);
	//----------------
	//���windows socket����
	WSACleanup();
	getchar();
	return 0;
}