#ifndef _TcpServer_
#define _TcpServer_

#ifdef _WIN32
#define FD_SETSIZE      6000
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
#include"High_resolution_timer.hpp"
#include <iostream>
#include<thread>
#include<vector>
#include<mutex>
#include"Message.hpp"
#ifndef RECV_BUF_SIZE
#define RECV_BUF_SIZE 10240  //第一接收缓存区的大小
#endif // !RECV_BUF_SIZE
using namespace std;
const int Cell_server_num = 4;  //服务子进程的数量
class g_client  //服务端中客户端管理类
{
public:
	g_client(SOCKET _sock);
	int getLastPos();
	void copyMse(const char* src, int len);  //将第一缓存区的数据拷贝进第二缓存区
	void moveMse(int proLen);  //将第二缓存区的数据前移
	void closeSock();
	const char* getMseBuf();
	SOCKET sock;
private:
	char recMesBuf[RECV_BUF_SIZE * 10];  //第二接收缓存区
	int LastPos;  //第二接收缓存区最后数据尾部位置
};

g_client::g_client(SOCKET _sock = INVALID_SOCKET)
{
	sock = _sock;
	memset(recMesBuf, 0, sizeof(recMesBuf));
	LastPos = 0;
}

int g_client::getLastPos()
{
	return LastPos;
}

void g_client::copyMse(const char* src, int len)
{
	if (LastPos + len < RECV_BUF_SIZE * 10)
	{
		memcpy(recMesBuf + LastPos, src, len);
		LastPos += len;
	}
	else
	{
		printf("copyMse 内存越界\n");
	}

}

void g_client::moveMse(int proLen)
{
	int unproLen = LastPos - proLen;
	if (LastPos <= RECV_BUF_SIZE * 10)
	{
		memcpy(recMesBuf, recMesBuf + proLen, unproLen);
		LastPos -= proLen;
	}
	else
	{
		printf("moveMse 内存越界!\n");
	}

}

void g_client::closeSock()
{
#ifdef WIN32
	closesocket(sock);
#else
	close(sock);
#endif
}

const char* g_client::getMseBuf()
{
	return recMesBuf;
}


class CellServer
{
public:
	CellServer(int seconds);
	virtual ~CellServer();
	void CloseSocket();  //关闭SOCKET
	void SendData(SOCKET _csock, pkgHeader* sendHeader);  //发送数据包
	int RecvData(g_client* _client);  //接受数据包(拆包)
	int RecvTestData(g_client* _client);  //接受粘包测试数据包(拆包)
	void ProcessReq(SOCKET _csock, pkgHeader* recHeader);  //处理客户端请求
	void WaitReq();  //等待客户端请求
	int recPkgNum;  //每秒接收数据包数量	
	char recBuf[RECV_BUF_SIZE] = {};  //接收缓存区
	void client_buf_import(g_client* g);
	void client_buf_export();
	void keep_server();
	void start();
	int get_clients_sum();  //取得当前服务的客户端数量
private:
	std::vector<g_client*> clients_pro;  //正在处理的客户端类的vector
	std::vector<g_client*> clients_buf;  //缓冲区客户端类的vector
	SOCKET _sock;
	fd_set fdRead;
	TestPkg testpkg;
	mutex m;
	bool keepRunning;
	std::thread* t;
	int select_second;  //select查询时最大时间
};

CellServer::CellServer(int second)
{
	select_second = second;
	keepRunning = true;
	recPkgNum = 0;
}

CellServer::~CellServer()
{
	CloseSocket();
}

void CellServer::CloseSocket()
{
	if (clients_buf.size() > 0)
	{
		client_buf_export();  //将客户端由中间缓冲区输出到处理队列
	}
	for (size_t n = 0; n < clients_pro.size(); n++)
	{
		clients_pro[n]->closeSock();
		delete[] clients_pro[n];
	}
}

void CellServer::start()
{
	t = new std::thread(&CellServer::keep_server, this);
	t->detach();
}

void CellServer::keep_server()
{
	while (keepRunning)
	{
		WaitReq();
	}
}

void CellServer::client_buf_import(g_client* g)
{
	lock_guard<mutex> lg(m);
	clients_buf.push_back(g);
}

void CellServer::client_buf_export()
{
	lock_guard<mutex> lg(m);
	for (auto iter = clients_buf.begin(); iter != clients_buf.end(); iter++)
	{
		clients_pro.push_back(*iter);
	}
	clients_buf.clear();
}

void CellServer::SendData(SOCKET _csock, pkgHeader* sendHeader)
{
	if (_sock != INVALID_SOCKET)
	{
		send(_csock, (const char*)sendHeader, sendHeader->pkgLen, 0);
	}
}

int CellServer::RecvData(g_client* _client)
{
	int recBufLen = recv(_client->sock, recBuf, RECV_BUF_SIZE, 0);  //接收消息包头
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		_client->copyMse(recBuf, recBufLen);
		while (_client->getLastPos() >= sizeof(pkgHeader))
		{
			pkgHeader* recHeader = (pkgHeader*)_client->getMseBuf();
			//printf("接收到包头: 长度：%d,类型：%d\n", recHeader->pkgLen, recHeader->cmd);
			//若第二缓冲区数据长度大于接收包的长度，则处理这个包，否则说明接收到的数据包不完整
			if (_client->getLastPos() >= recHeader->pkgLen)
			{
				ProcessReq(_client->sock, recHeader);
				_client->moveMse(recHeader->pkgLen);
			}
			else
			{
				break;
			}
		}
	}
	return 0;
}

int CellServer::RecvTestData(g_client* _client)
{
	int recBufLen = recv(_client->sock, recBuf, RECV_BUF_SIZE, 0);  //接收消息包头
	if (recBufLen <= 0)
	{
		return -1;
	}
	else
	{
		printf("接收到 %d 字节数据\n", recBufLen);
		SendData(_client->sock, &testpkg);
		return 0;
	}


}

void CellServer::ProcessReq(SOCKET _csock, pkgHeader* recHeader)
{
	recPkgNum++;
	switch (recHeader->cmd)  //查看包头的命令类型
	{
	case(CMD_LOGIN):
	{

		LoginData* loginMse = (LoginData*)recHeader;
		printf("接收到登录信息,用户名：%s, 密码：%s\n", loginMse->userName, loginMse->userWord);
		LoginResult loginReMse;
		loginReMse.result = 202;
		SendData(_csock, &loginReMse);
		//printf("发送登录返回\n");

		LoginBro loginBro;  //新用户登录广播消息
#ifdef _WIN32
		strcpy_s(loginBro.userID, loginMse->userName);
#else
		strcpy(loginBro.userID, loginMse->userName);
#endif
		for (auto iter = clients_pro.begin(); iter != clients_pro.end(); iter++)
		{
			if ((*iter)->sock == _csock)
				continue;
			else
			{
				//printf("发送登录广播\n");
				SendData((*iter)->sock, &loginBro);
			}
		}
		break;
	}

	case(CMD_LOGOUT):
	{

		LogOutData* logoutMse = (LogOutData*)recHeader;
		printf("接收到登出信息,用户名：%s\n", logoutMse->userName);
		LogOutResult logoutReMse;
		logoutReMse.result = 202;
		SendData(_csock, &logoutReMse);
		//printf("发送登出返回\n");


		LogoutBro logoutBro;  //新用户登出广播消息
#ifdef _WIN32
		strcpy_s(logoutBro.userID, logoutMse->userName);
#else
		strcpy(logoutBro.userID, logoutMse->userName);
#endif
		for (auto iter = clients_pro.begin(); iter != clients_pro.end(); iter++)
		{
			if ((*iter)->sock == _csock)
				continue;
			else
			{
				SendData((*iter)->sock, &logoutBro);
				printf("发送登出广播\n");
			}
		}
		break;
	}
	case(CMD_TEST):  //测试粘包
	{
		//printf("返回测试数据包\n");
		//SendData(_csock, &testpkg);
		break;
	}
	default:
	{
		ErrorPkg errorPkg;
		errorPkg.error_code = 404;
		SendData(_csock, &errorPkg);
		printf("发送错误信息\n");
		break;
	}
	}
}

int CellServer::get_clients_sum()
{
	return  clients_buf.size() + clients_pro.size();
}

void CellServer::WaitReq()
{
	FD_ZERO(&fdRead);

	if (get_clients_sum() <= 0) //没有待处理的客户端
		return;
	if (clients_buf.size() > 0)
	{
		client_buf_export();  //将客户端由中间缓冲区输出到处理队列
	}
	SOCKET max_sock = clients_pro[0]->sock;

	for (int n = (int)clients_pro.size() - 1; n >= 0; n--)
	{
		FD_SET(clients_pro[n]->sock, &fdRead);
		if (clients_pro[n]->sock > max_sock)
		{
			max_sock = clients_pro[n]->sock;
		}
	}

	//nfds  是集合fd_set中最后一个socket描述符+1, 用以表示集合范围
	timeval t = { select_second,0 };
	int ret = select(max_sock + 1, &fdRead, NULL, NULL, &t);  //select将更新这个集合,把其中不可读(不可写)的套节字去掉 
	if (ret < 0)
	{
		printf("select 任务结束.\n");
		keepRunning = false;
	}

	for (size_t n = 0; n < clients_pro.size(); n++)
	{
		if (FD_ISSET(clients_pro[n]->sock, &fdRead))
		{
			if (RecvData(clients_pro[n]) == -1)
				//if (RecvTestData(g_clients[n]) == -1)
			{
				//printf("客户端退出.\n");
				auto iter = clients_pro.begin() + n;
				delete[](*iter);
				clients_pro.erase(iter);

			}
		}
	}
}


class MyServer
{
public:
	MyServer(const char* ip, unsigned short port, int select_s);
	virtual ~MyServer();
	void InitServer(const char* ip, unsigned short port);  //初始化服务器(1. 建立SOCKET 2.绑定端口 3.开始监听)
	void CloseSocket();  //关闭SOCKET
	void WaitReq();  //等待客户端请求
	void otherServer();
	bool keepRunning;
	TestPkg testpkg;
	MyTimer timer;  //高精度计时器
private:
	SOCKET _sock;
	std::vector<CellServer*> CellServerList;
	void CellServerInit();  //启动服务线程
	void add_ClientToCellServer(g_client* g);  //将客户端分配给服务线程
	fd_set fdRead;
	int select_seconds;
	int getClientsSum();
	int getPkgSum();  //返回所有服务子线程每秒接收数据包的数量和
};

MyServer::MyServer(const char* ip, unsigned short port, int select_s)
{
	_sock = INVALID_SOCKET;
	InitServer(ip, port);
	CellServerInit();
	keepRunning = true;
	select_seconds = select_s;
}

MyServer::~MyServer()
{
	CloseSocket();
}

int MyServer::getPkgSum()
{
	int sum = 0;
	for (auto iter = CellServerList.begin(); iter != CellServerList.end(); iter++)
	{
		sum += (*iter)->recPkgNum;
		(*iter)->recPkgNum=0;
	}
	return sum;

}

int MyServer::getClientsSum()
{
	int sum = 0;
	for (auto iter = CellServerList.begin(); iter != CellServerList.end(); iter++)
	{
		sum += (*iter)->get_clients_sum();
	}
	return sum;
}

void MyServer::CellServerInit()
{
	for (int i = 0; i < Cell_server_num; i++)
	{
		CellServer* cell_server = new CellServer(select_seconds);
		CellServerList.push_back(cell_server);
		cell_server->start();
	}
}

void MyServer::add_ClientToCellServer(g_client* g)
{
	auto min_cell_ser = CellServerList.begin();  //用于保存当前服务最少客户端的服务子线程
	for (auto iter = CellServerList.begin(); iter != CellServerList.end(); iter++)
	{
		if ((*min_cell_ser)->get_clients_sum() > (*iter)->get_clients_sum())
		{
			min_cell_ser = iter;
		}
	}
	(*min_cell_ser)->client_buf_import(g);
}

void MyServer::InitServer(const char* ip, unsigned short port)
{
#ifdef _WIN32
	//启动windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);  // 算出版本号
	WSADATA dat;
	WSAStartup(ver, &dat);

#endif
	//----------------
	//--- 使用socket API建立简易TCP服务端
	//  1.建立一个socket
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//  2.bind 绑定用于接受客户端连接的网络端口
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
		printf("ERROR, 服务端绑定端口失败！\n");
	}
	else
	{
		printf("服务端绑定端口成功！\n");
	}

	//  3.listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("错误，监听网络端口失败..\n");
	}
	else
	{
		printf("监听网络端口成功\n");
	}
}

void MyServer::CloseSocket()
{

#ifdef _WIN32
	//  6.关闭socket closesocket
	closesocket(_sock);
	//------------------
	//清除windows socket环境
	WSACleanup();
#else
	//  6.关闭socket closesocket
	close(_sock);
#endif
}


void MyServer::WaitReq()
{
	if (timer.GetSeconds() >= 1)
	{
		printf("seconds:%d, 当前客户端数量:%d, 每秒接收包数:%d\n", timer.GetSeconds(), getClientsSum(), getPkgSum());
		timer.update();
	}

	FD_SET(_sock, &fdRead);

	SOCKET max_sock = _sock;
	//nfds  是集合fd_set中最后一个socket描述符+1, 用以表示集合范围
	timeval t = {select_seconds,0 };
	int ret = select(max_sock + 1, &fdRead, NULL, NULL, &t);  //select将更新这个集合,把其中不可读(不可写)的套节字去掉 
	if (ret < 0)
	{
		printf("select 任务结束.\n");
		keepRunning = false;
	}
	if (FD_ISSET(_sock, &fdRead))  //查看_sock是否还有可读请求
	{
		FD_CLR(_sock, &fdRead);
		//  4.accept 等待接受客户端连接
		sockaddr_in clientAdd = {};
		int AddLen = sizeof(sockaddr_in);
		SOCKET csock = INVALID_SOCKET;
#ifdef _WIN32
		csock = accept(_sock, (sockaddr*)&clientAdd, &AddLen);
#else
		csock = accept(_sock, (sockaddr*)&clientAdd, (socklen_t*)&AddLen);
#endif
		if (csock == INVALID_SOCKET)
		{
			printf("接收到无效 socket\n");
		}
		g_client* new_client = new g_client(csock);
		add_ClientToCellServer(new_client);
		//printf("新的客户端加入, IP = %s ,SOCKET = %d ,当前客户端数量: %d\n", inet_ntoa(clientAdd.sin_addr), (int)csock, getClientsSum());
	}

}

void MyServer::otherServer()
{

#ifdef _WIN32
	Sleep(500);
#else
	sleep(0.5);
#endif
	printf("其他服务进行中\n");
}
#endif // _TcpServer_