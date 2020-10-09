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
	//--- ʹ��socket API��������TCP�����
	//  1.����һ��socket
	SOCKET _sock =socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//  2.bind �����ڽ��ܿͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;  //socket protocol
	_sin.sin_port = htons(4567);  //host to net unsighned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("ERROR, ����˰󶨶˿�ʧ�ܣ�\n");
	}
	printf("����˰󶨶˿ڳɹ���\n");
	//  3.listen ��������˿�
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("���󣬼�������˿�ʧ��..\n");
	}
	else
	{
		printf("��������˿ڳɹ�\n");
	}
	//  4.accept �ȴ����ܿͻ�������
	sockaddr_in clientAdd = {};
	int AddLen = sizeof(sockaddr_in);
	SOCKET _csock = INVALID_SOCKET;
	
	_csock = accept(_sock, (sockaddr*)&clientAdd, &AddLen);
	if (_csock == INVALID_SOCKET)
	{
		printf("���յ���Ч socket\n");
	}
	printf("�µĿͻ��˼���, IP = %s \n", inet_ntoa(clientAdd.sin_addr));

	while (true)
	{
		char recBuf[1024];  //���ջ�����
		pkgHeader* recHeader;
		int recBufLen = recv(_csock, recBuf, sizeof(pkgHeader), 0);  //������Ϣ��ͷ
		if (recBufLen <= 0)
		{
			printf("�ͻ����˳�\n");
			break;
		}
		else
		{
			recHeader = (pkgHeader*)recBuf;
			printf("���յ���ͷ: ���ȣ�%d,���ͣ�%d\n", recHeader->pkgLen,recHeader->cmd);
		}
		switch (recHeader->cmd)  //�鿴��ͷ����������
		{
			case(CMD_LOGIN):
			{
				recBufLen = recv(_csock, recBuf+sizeof(pkgHeader), sizeof(LoginData)-sizeof(pkgHeader), 0);
				LoginData* loginMse = (LoginData*)recBuf;
				if (recBufLen > 0)
				{
					printf("���յ���¼��Ϣ,�û�����%s, ���룺%s\n", loginMse->userName, loginMse->userWord);
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
					printf("���յ��ǳ���Ϣ,�û�����%s\n", logoutMse->userName);
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
	//  6.�ر�socket closesocket
	closesocket(_sock);
	//------------------
	//���windows socket����
	WSACleanup();
	getchar();
	return 0;

}



