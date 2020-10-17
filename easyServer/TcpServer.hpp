#ifndef _TcpServer_
#define _TcpServer_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
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
#include<vector>
#include"Message.hpp"
using namespace std;

class MyServer
{
public:
	MyServer(const char* ip, unsigned short port);
	virtual ~MyServer();
	void InitServer(const char* ip, unsigned short port);  //��ʼ��������(1. ����SOCKET 2.�󶨶˿� 3.��ʼ����)
	void CloseSocket();  //�ر�SOCKET
	void SendData(SOCKET _csock, pkgHeader* sendHeader);  //�������ݰ�
	int RecvData(SOCKET _csock);  //�������ݰ�(���)
	void ProcessReq(SOCKET _csock, pkgHeader* recHeader);  //����ͻ�������
	void WaitReq(int seconds);  //�ȴ��ͻ�������
	void otherServer();
	bool keepRunning;
private:
	std::vector<SOCKET> g_clients;
	SOCKET _sock;
	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExp;
};

MyServer::MyServer(const char* ip, unsigned short port)
{
	_sock = INVALID_SOCKET;
	InitServer(ip, port);
	keepRunning = true;
}

MyServer::~MyServer()
{
}

void MyServer::InitServer(const char* ip, unsigned short port)
{
#ifdef _WIN32
	//����windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);  // ����汾��
	WSADATA dat;
	WSAStartup(ver, &dat);

#endif
	//----------------
	//--- ʹ��socket API��������TCP�����
	//  1.����һ��socket
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//  2.bind �����ڽ��ܿͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;  //socket protocol
	_sin.sin_port = htons(port);  //host to net unsighned short
	if (ip)
	{
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);	// inet_addr("127.0.0.1");
#else
		_sin.sin_addr.s_addr = inet_addr(ip);	// inet_addr("127.0.0.1");
#endif
	}
	else
	{
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;	// inet_addr("127.0.0.1");
#else
		_sin.sin_addr.s_addr = INADDR_ANY;	// inet_addr("127.0.0.1");
#endif
	}

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
}

void MyServer::CloseSocket()
{
#ifdef _WIN32
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		closesocket(g_clients[n]);
	}
	//  6.�ر�socket closesocket
	closesocket(_sock);
	//------------------
	//���windows socket����
	WSACleanup();
#else
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		close(g_clients[n]);
	}
	//  6.�ر�socket closesocket
	close(_sock);
#endif
}

void MyServer::SendData(SOCKET _csock, pkgHeader* sendHeader)
{
	if (_sock != INVALID_SOCKET)
	{
		send(_csock, (const char*)sendHeader, sendHeader->pkgLen, 0);
	}
}

int MyServer::RecvData(SOCKET _csock)
{
	char recBuf[4096];  //���ջ�����
	int recBufLen = recv(_csock, recBuf, sizeof(pkgHeader), 0);  //������Ϣ��ͷ
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		pkgHeader* recHeader = (pkgHeader*)recBuf;
		printf("���յ���ͷ: ���ȣ�%d,���ͣ�%d\n", recHeader->pkgLen, recHeader->cmd);
		recv(_csock, recBuf + sizeof(pkgHeader), recHeader->pkgLen - sizeof(pkgHeader), 0);
		ProcessReq(_csock, recHeader);
	}
	return 0;
}

void MyServer::ProcessReq(SOCKET _csock, pkgHeader* recHeader)
{
	switch (recHeader->cmd)  //�鿴��ͷ����������
	{
	case(CMD_LOGIN):
	{

		LoginData* loginMse = (LoginData*)recHeader;
		printf("���յ���¼��Ϣ,�û�����%s, ���룺%s\n", loginMse->userName, loginMse->userWord);
		LoginResult loginReMse;
		loginReMse.result = 202;
		SendData(_csock, &loginReMse);
		//printf("���͵�¼����\n");

		LoginBro loginBro;  //���û���¼�㲥��Ϣ
#ifdef _WIN32
		strcpy_s(loginBro.userID, loginMse->userName);
#else
		strcpy(loginBro.userID, loginMse->userName);
#endif
		for (auto iter = g_clients.begin(); iter != g_clients.end(); iter++)
		{
			if (*iter == _csock)
				continue;
			else
			{
				//printf("���͵�¼�㲥\n");
				SendData(*iter, &loginBro);
			}
		}
	}
	break;
	case(CMD_LOGOUT):
	{
		
		LogOutData* logoutMse = (LogOutData*)recHeader;
		printf("���յ��ǳ���Ϣ,�û�����%s\n", logoutMse->userName);
		LogOutResult logoutReMse;
		logoutReMse.result = 202;
		SendData(_csock, &logoutReMse);
		//printf("���͵ǳ�����\n");


		LogoutBro logoutBro;  //���û��ǳ��㲥��Ϣ
#ifdef _WIN32
		strcpy_s(logoutBro.userID, logoutMse->userName);
#else
		strcpy(logoutBro.userID, logoutMse->userName);
#endif
		for (auto iter = g_clients.begin(); iter != g_clients.end(); iter++)
		{
			if (*iter == _csock)
				continue;
			else
			{
				SendData(*iter, &logoutBro);
				printf("���͵ǳ��㲥\n");
			}
		}
	}
	break;
	default:
		ErrorPkg errorPkg;
		errorPkg.error_code = 404;
		SendData(_csock, &errorPkg);
		printf("���ʹ�����Ϣ\n");
		break;
	}
}

void MyServer::WaitReq(int seconds)
{
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdExp);

	FD_SET(_sock, &fdRead);
	FD_SET(_sock, &fdWrite);
	FD_SET(_sock, &fdExp);

	SOCKET max_sock = _sock;

	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		FD_SET(g_clients[n], &fdRead);
		if (g_clients[n] > max_sock)
		{
			max_sock = g_clients[n];
		}
	}

	//nfds  �Ǽ���fd_set�����һ��socket������+1, ���Ա�ʾ���Ϸ�Χ
	timeval t = { seconds,0 };
	int ret = select(max_sock + 1, &fdRead, &fdWrite, &fdExp, &t);  //select�������������,�����в��ɶ�(����д)���׽���ȥ�� 
	if (ret < 0)
	{
		printf("select �������.\n");
		keepRunning = false;
	}
	if (FD_ISSET(_sock, &fdRead))  //�鿴_sock�Ƿ��пɶ�����
	{
		FD_CLR(_sock, &fdRead);
		//  4.accept �ȴ����ܿͻ�������
		sockaddr_in clientAdd = {};
		int AddLen = sizeof(sockaddr_in);
		SOCKET _csock = INVALID_SOCKET;
#ifdef _WIN32
		_csock = accept(_sock, (sockaddr*)&clientAdd, &AddLen);
#else
		_csock = accept(_sock, (sockaddr*)&clientAdd, (socklen_t*)&AddLen);
#endif
		if (_csock == INVALID_SOCKET)
		{
			printf("���յ���Ч socket\n");
		}
		g_clients.push_back(_csock);
		printf("�µĿͻ��˼���, IP = %s \n", inet_ntoa(clientAdd.sin_addr));
	}
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		if (FD_ISSET(g_clients[n], &fdRead))
		{
			if (RecvData(g_clients[n]) == -1)
			{
				auto iter = g_clients.begin() + n;
				g_clients.erase(iter);
				printf("�ͻ����˳�.\n");
			}
		}
	}
}

void MyServer::otherServer()
{
	Sleep(0.5 * 1000);
	printf("�������������\n");
}
#endif // _TcpServer_
