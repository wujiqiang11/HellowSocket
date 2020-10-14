#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include<Windows.h>
#include<WinSock2.h>
#include <iostream>

using namespace std;

enum CMD
{
	CMD_LOGIN,  // ��¼��Ϣ
	CMD_LOGINRE,  // ��½������Ϣ
	CMD_LOGOUT,  // �ǳ���Ϣ
	CMD_LOGOUTRE,  // �ǳ�������Ϣ
	CMD_ERROR,  //������Ϣ
	CMD_LOGIN_BRO,  // ��¼�㲥
	CMD_LOGOUT_BRO,  // �ǳ��㲥
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
	char recBuf[1024];  //���ջ�����
	pkgHeader* recHeader;
	int recBufLen = recv(_sock, recBuf, sizeof(pkgHeader), 0);  //������Ϣ��ͷ
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		recHeader = (pkgHeader*)recBuf;
		printf("���յ���ͷ: ���ȣ�%d,���ͣ�%d\n", recHeader->pkgLen, recHeader->cmd);
	}
	switch (recHeader->cmd)  //�鿴��ͷ����������
	{
		case(CMD_LOGINRE):
		{
			
			int recBufLen = recv(_sock, recBuf+sizeof(pkgHeader), sizeof(LoginResult)-sizeof(pkgHeader), 0);
			LoginResult* loginResult=(LoginResult*)recBuf;
			if (loginResult->result == 202)
			{
				printf("��½�ɹ�!\n");
			}
		}
			break;
		case(CMD_LOGOUTRE):
		{
			int recBufLen = recv(_sock, recBuf + sizeof(pkgHeader), sizeof(LogOutResult) - sizeof(pkgHeader), 0);
			LogOutResult* logoutResult = (LogOutResult*)recBuf;
			if (logoutResult->result == 202)
			{
				printf("�ǳ��ɹ�!\n");
			}
		}
			break;
		case(CMD_LOGIN_BRO):
		{
			int recBufLen = recv(_sock, recBuf + sizeof(pkgHeader), sizeof(LoginBro) - sizeof(pkgHeader), 0);
			LoginBro* loginBro=(LoginBro*)recBuf;
			printf("IDΪ %s ���û���¼������!\n", loginBro->userID);
		}
			break;
		case(CMD_LOGOUT_BRO):
		{
			int recBufLen = recv(_sock, recBuf + sizeof(pkgHeader), sizeof(LogoutBro) - sizeof(pkgHeader), 0);
			LogoutBro* logoutBro = (LogoutBro*)recBuf;
			printf("IDΪ %s ���û��ǳ�������!\n", logoutBro->userID);
		}
			break;
		default:
			break;
	}
	return 0;
}

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
	
	fd_set fdRead;

	while (true)
	{
		
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);


		//nfds  �Ǽ���fd_set�����һ��socket������+1, ���Ա�ʾ���Ϸ�Χ
		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdRead,NULL,NULL,&t);  //select�������������,�����в��ɶ�(����д)���׽���ȥ�� 
		if (ret < 0)
		{
			printf("select �������.\n");
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			if(process(_sock)==-1)
				printf("�������Ͽ�����\n");
		}
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
			if (process(_sock) == -1)
				printf("�������Ͽ�����\n");
			
		}
		else if (strcmp(client_cmd, "logout") == 0)
		{
			LogOutData logoutMse;
			printf("��������û���: ");
			scanf("%s", logoutMse.userName);
			send(_sock, (const char*)&logoutMse, sizeof(LogOutData), 0);
			if (process(_sock) == -1)
				printf("�������Ͽ�����\n");
			
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