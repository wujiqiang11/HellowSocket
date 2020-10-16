enum CMD
{
	CMD_LOGIN,  // ��¼��Ϣ
	CMD_LOGINRE,  // ��½������Ϣ
	CMD_LOGOUT,  // �ǳ���Ϣ
	CMD_LOGOUTRE,  // �ǳ�������Ϣ
	CMD_ERROR,  //������Ϣ
	CMD_LOGIN_BRO,  // ��¼�㲥
	CMD_LOGOUT_BRO  // �ǳ��㲥
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
