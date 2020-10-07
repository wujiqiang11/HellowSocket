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
	char msgbuf[] = "Helow, I`m server!";
	while (true)
	{
		_csock = accept(_sock, (sockaddr*)&clientAdd, &AddLen);
		if (_csock == INVALID_SOCKET)
		{
			printf("���յ���Ч socket\n");
		}
		printf("�µĿͻ��˼���, IP = %s \n", inet_ntoa(clientAdd.sin_addr));
		//  5.send ��ͻ��˷���һ������
		send(_csock, msgbuf, strlen(msgbuf) + 1, 0);
	}


	//  6.�ر�socket closesocket
	closesocket(_sock);
	//------------------
	//���windows socket����
	WSACleanup();
	return 0;

}


