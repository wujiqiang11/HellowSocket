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
using namespace std;

class TcpClient
{
public:
	TcpClient();
	virtual ~TcpClient();
	void InitSocket();  //��ʼ��Socket
	int Connect(char* ip, unsigned short port);  //���ӷ�����
	void CloseSocket();  //�ر�socket
	//��������
	//��������
	//����ҵ��
private:
	SOCKET _sock;
};

TcpClient::TcpClient()
{
	_sock = INVALID_SOCKET;
}

TcpClient::~TcpClient()
{
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
	//  1.����һ��socket
	_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
		printf("���� Socket ʧ��!\n");
	else
	{
		printf("���� Socket �ɹ�!\n");
	}
}

int TcpClient::Connect(char* ip, unsigned short port)
{
	//  2.���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr("ip");
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
	return ret;
}

void TcpClient::CloseSocket()
{
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

#endif // !_TcpClient_hpp_
