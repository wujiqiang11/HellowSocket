#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include<Windows.h>
#include<WinSock2.h>
#include <iostream>

using namespace std;
int main()
{
	//����windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);  // ����汾��
	WSADATA dat;
	WSAStartup(ver, &dat);
	//----------------
	//--- ʹ��socket API����һ�����׵�TCP�ͻ���
	//  1.����һ��socket
	SOCKET _csock = socket(AF_INET, SOCK_STREAM, 0);
	if (_csock == INVALID_SOCKET)
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
	int net = connect(_csock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (net == INVALID_SOCKET)
	{
		printf("ERROR ����ʧ�ܣ�\n");
	}
	else
	{
		printf("���ӳɹ�\n");
	}
	//  3.���ܷ�������Ϣ revc
	char recBuf[256] = {};
	int nlen = recv(_csock, recBuf, 256, 0);
	if (nlen > 0)
	{
		printf("���յ�����:%s\n", recBuf);
	}
	//  4.�ر�socket closesocket
	closesocket(_csock);
	//----------------
	//���windows socket����
	WSACleanup();
	getchar();
	return 0;
}