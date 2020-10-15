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
		//printf("���յ���ͷ: ���ȣ�%d,���ͣ�%d\n", recHeader->pkgLen, recHeader->cmd);
	}
	switch (recHeader->cmd)  //�鿴��ͷ����������
	{
	case(CMD_LOGINRE):
	{

		int recBufLen = recv(_sock, recBuf + sizeof(pkgHeader), sizeof(LoginResult) - sizeof(pkgHeader), 0);
		LoginResult* loginResult = (LoginResult*)recBuf;
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
		LoginBro* loginBro = (LoginBro*)recBuf;
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

bool keep_running = true;

void cmd_thread(SOCKET _sock)  //�����߳�
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
			printf("����������û���: ");
			scanf("%s", loginMse.userName);
			printf("�������������:");
			scanf("%s", loginMse.userWord);
			send(_sock, (const char*)&loginMse, sizeof(LoginData), 0);

		}
		else if (strcmp(client_cmd, "logout") == 0)
		{
			LogOutData logoutMse;
			printf("����������û���: ");
			scanf("%s", logoutMse.userName);
			send(_sock, (const char*)&logoutMse, sizeof(LogOutData), 0);
		}
		else
			printf("��Ч����!\n");
	}
}
int main()
{
#ifdef _WIN32
	//����windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);  // ����汾��
	WSADATA dat;
	WSAStartup(ver, &dat);

#endif
	//----------------
	//--- ʹ��socket API����һ�����׵�TCP�ͻ���
	//  1.����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
		printf("���� Socket ʧ��!\n");
	else
	{
		printf("���� Socket �ɹ�!\n");
	}
	//  2.���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
	_sin.sin_addr.s_addr = inet_addr("192.168.77.1");
#endif
	int net = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (net == INVALID_SOCKET)
	{
		printf("���Ӵ���!\n");
	}
	else
	{
		printf("���ӷ������ɹ�!\n");
	}

	fd_set fdRead;
	std::thread t(cmd_thread, _sock);  //��һ�������߳�
	t.detach();

	while (keep_running)
	{
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);
		//nfds  �Ǽ���fd_set�����һ��socket������+1, ���Ա�ʾ���Ϸ�Χ
		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdRead, NULL, NULL, &t);  //select�������������,�����в��ɶ�(����д)���׽���ȥ��  
		if (ret < 0)
		{
			printf("select function end.\n");
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			if (process(_sock) == -1)
			{
				printf("Disconnect to server.\n");
				break;
			}
		}

	}
	//  4.�ر�socket closesocket
#ifdef _WIN32	
	closesocket(_sock);
	//----------------
	//���windows socket����
	WSACleanup();
#else
	close(_sock);
#endif
	getchar();
	return 0;
}