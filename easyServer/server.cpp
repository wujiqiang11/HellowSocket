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
		pkgHeader recHeader = {};
		int recBufLen = recv(_csock, (char*)&recHeader, sizeof(pkgHeader), 0);  //������Ϣ��ͷ
		if (recBufLen <= 0)
		{
			printf("�ͻ����˳�\n");
			break;
		}
		else
		{
			printf("���յ���ͷ: ���ȣ�%d,���ͣ�%d\n", recHeader.pkgLen,recHeader.cmd);
		}
		switch (recHeader.cmd)  //�鿴��ͷ����������
		{
			case(CMD_LOGIN):
			{
				LoginData recLoginData = {};
				recBufLen = recv(_csock, (char*)&recLoginData, sizeof(LoginData), 0);
				if (recBufLen > 0)
				{
					printf("���յ���¼��Ϣ,�û�����%s, ���룺%s\n", recLoginData.userName, recLoginData.userWord);
					
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
					printf("���յ��˳���¼��Ϣ���û���:  %s\n", recLogOutData.userName);
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
	//  6.�ر�socket closesocket
	closesocket(_sock);
	//------------------
	//���windows socket����
	WSACleanup();
	getchar();
	return 0;

}



