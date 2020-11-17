#ifndef _TcpClient_hpp_
#define _TcpClient_hpp_

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
#include"Message.hpp"
#define RECV_BUF_SIZE 10240  //��һ���ջ������Ĵ�С
using namespace std;

class TcpClient
{
public:
	TcpClient();
	virtual ~TcpClient();
	void InitSocket();  //��ʼ��Socket
	int Connect(const char* ip, unsigned short port);  //���ӷ�����
	void CloseClientSock();  //�ر�socket
	//�������ݰ�
	void SendData(pkgHeader* sendHeader);
	//�������ݰ�
	int RecvData();
	//���ܲ������ݰ�
	int RecvTestData();
	//select ��������������Ϣ
	void KeepRun();
	//ҵ��������������Ϣ
	void ResMse(pkgHeader* recHeader);
	//ҵ����洦���û�����
	void ProcessCMD();
	//���Ͳ������ݰ�
	void SendTest();
	std::thread RecvCMD();  //�����û�����
	bool keep_running;
	TestPkg testpkg;
	char recBuf[RECV_BUF_SIZE] = {};  //��һ���ջ�����
	char recMesBuf[RECV_BUF_SIZE * 10] = {};  //�ڶ����ջ�����
	int LastPos_MesBuf;  //�ڶ����ջ��������ݵ�β��λ��
private:
	SOCKET _sock;
	
};

TcpClient::TcpClient()
{
	_sock = INVALID_SOCKET;
	keep_running = true;
	LastPos_MesBuf = 0;
	InitSocket();
}

TcpClient::~TcpClient()
{
	CloseClientSock();
}

void TcpClient::InitSocket()
{
#ifdef _WIN32
	//����windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);  // ����汾��
	WSADATA dat;
	WSAStartup(ver, &dat);

#endif
	//----------------
	//--- ʹ��socket API����һ�����׵�TCP�ͻ���
	if (_sock != INVALID_SOCKET)
	{
		printf("�رվ�����\n");
		CloseClientSock();
	}
	//  1.����һ��socket
	_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
		printf("���� Socket ʧ��!\n");
	else
	{
		printf("���� Socket �ɹ�!\n");
	}
}

int TcpClient::Connect(const char* ip, unsigned short port)
{
	//  2.���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
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
	return net;
}

void TcpClient::CloseClientSock()
{
	if (_sock != INVALID_SOCKET)
	{
		printf("�ر�socket");
		//  4.�ر�socket closesocket
#ifdef _WIN32	
		closesocket(_sock);
		//----------------
		//���windows socket����
		WSACleanup();
#else
		close(_sock);
#endif
	}
}

void TcpClient::KeepRun()
{
	if (_sock != INVALID_SOCKET)
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);
		//nfds  �Ǽ���fd_set�����һ��socket������+1, ���Ա�ʾ���Ϸ�Χ
		timeval t = { 0,0 };
		int ret = select(_sock + 1, &fdRead, NULL, NULL, &t);  //select�������������,�����в��ɶ�(����д)���׽���ȥ��  
		if (ret < 0)
		{
			printf("select function end.\n");
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			if (RecvData() == -1)
			//if (RecvTestData() == -1)
			{
				printf("Disconnect to server.\n");
				keep_running = false;
			}
		}
	}
}

int TcpClient::RecvData()
{

	int recBufLen = (int)recv(_sock, recBuf, RECV_BUF_SIZE, 0);  //�����յ�������һ��װ���һ������
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		memcpy(recMesBuf + LastPos_MesBuf, recBuf, recBufLen);
		LastPos_MesBuf += recBufLen;
		while (LastPos_MesBuf >= sizeof(pkgHeader))  //���ڶ������������ݳ��ȴ���һ����ͷ�����ȣ�����ո�ͷ��
		{
			pkgHeader* recHeader = (pkgHeader*)recMesBuf;
			//printf("���յ���ͷ: ���ȣ�%d,���ͣ�%d\n", recHeader->pkgLen, recHeader->cmd);
			//���ڶ����������ݳ��ȴ��ڽ��հ��ĳ��ȣ����������������˵�����յ������ݰ�������
			if (LastPos_MesBuf >= recHeader->pkgLen)
			{
				ResMse(recHeader);
				int _unproLen = LastPos_MesBuf - recHeader->pkgLen;  //δ��������ݳ���
				int _proLen = recHeader->pkgLen;  //��ǰ���ڴ�������ݳ���
				memcpy(recMesBuf, recMesBuf + _proLen, _unproLen);
				LastPos_MesBuf = LastPos_MesBuf - recHeader->pkgLen;
			}
			else
			{
				break;
				printf("break.\n");
			}
		}
		return 0;
	}
	/*
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		pkgHeader* recHeader = (pkgHeader*)recBuf;
		printf("���յ���ͷ: ���ȣ�%d,���ͣ�%d\n", recHeader->pkgLen, recHeader->cmd);
		recv(_sock, recBuf + sizeof(pkgHeader), recHeader->pkgLen - sizeof(pkgHeader), 0);
		ResMse(recHeader);
	}
	return 0;
	*/
}


int TcpClient::RecvTestData()
{
	int recBufLen = recv(_sock, recBuf, 409600, 0);  //������Ϣ��ͷ
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		printf("���յ� %d �ֽ�����\n", recBufLen);
		SendData(&testpkg);
		return 0;
	}

}

void TcpClient::ResMse(pkgHeader* recHeader)
{
	switch (recHeader->cmd)  //�鿴��ͷ����������
	{
	case(CMD_LOGINRE):
	{

		LoginResult* loginResult = (LoginResult*)recHeader;
		if (loginResult->result == 202)
		{
			printf("��½�ɹ�!\n");
		}
	}
	break;
	case(CMD_LOGOUTRE):
	{

		LogOutResult* logoutResult = (LogOutResult*)recHeader;
		if (logoutResult->result == 202)
		{
			printf("�ǳ��ɹ�!\n");
		}
	}
	break;
	case(CMD_LOGIN_BRO):
	{
		LoginBro* loginBro = (LoginBro*)recHeader;
		printf("IDΪ %s ���û���¼������!\n", loginBro->userID);
	}
	break;
	case(CMD_LOGOUT_BRO):
	{
		LogoutBro* logoutBro = (LogoutBro*)recHeader;
		printf("IDΪ %s ���û��ǳ�������!\n", logoutBro->userID);
	}
	break;
	case(CMD_TEST):
	{
		//printf("���յ��������ݰ�!\n");
		break;
	}
	default:
		printf("���յ���Ч��Ϣ\n");
		break;
	}
}

void TcpClient::SendData(pkgHeader* sendHeader)
{
	if (_sock != INVALID_SOCKET)
	{
		send(_sock, (const char*)sendHeader, sendHeader->pkgLen, 0);
	}
}

void TcpClient::ProcessCMD()
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
			SendData(&loginMse);
			//send(_sock, (const char*)&loginMse, sizeof(LoginData), 0);

		}
		else if (strcmp(client_cmd, "logout") == 0)
		{
			LogOutData logoutMse;
			printf("����������û���: ");
			scanf("%s", logoutMse.userName);
			SendData(&logoutMse);
			//send(_sock, (const char*)&logoutMse, sizeof(LogOutData), 0);
		}
		else if (strcmp(client_cmd, "test") == 0)
		{	
			int sum = 0;
			while (true)
			{
				SendData(&testpkg);
				//printf("���Ͳ������ݰ� %d\n",++sum);
			}
			//send(_sock, (const char*)&logoutMse, sizeof(LogOutData), 0);
		}

		else
			printf("��Ч����!\n");
	}
}

std::thread TcpClient::RecvCMD()
{
	std::thread t(&TcpClient::ProcessCMD, this);  //��һ�������߳�
	return t;
}

void TcpClient::SendTest()
{
	SendData(&testpkg);
}
#endif // !_TcpClient_hpp_
