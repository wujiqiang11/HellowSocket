#ifndef _CELLTASK_h_
#include<list>
#include<mutex>
class CellTask
{
public:
	CellTask() {}
	virtual ~CellTask() {}
	virtual void dotask() {}
private:

};

class CellTaskServer
{
public:
	void start();
	void addTask(CellTask*);
	CellTaskServer() {}
	~CellTaskServer() {}
private:
	std::list<CellTask*> _TaskList;
	std::list<CellTask*> _TaskListBuf;
	void KeepRun();
	std::mutex _mutex;
	std::thread* t;
};

void CellTaskServer::start()
{
	t = new std::thread(&CellTaskServer::KeepRun, this);
	t->detach();
}

void CellTaskServer::addTask(CellTask* _celltask)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_TaskListBuf.push_back(_celltask);
}

void CellTaskServer::KeepRun()
{
	while (true)
	{
		if (!_TaskListBuf.empty())
		{
			std::lock_guard<std::mutex> lock(_mutex);
			for (auto it = _TaskListBuf.begin(); it != _TaskListBuf.end(); it++)
			{
				_TaskList.push_back(*it);
			}
			_TaskListBuf.clear();
		}
		if (_TaskList.empty())
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		}
		else
		{
			for (auto pTask : _TaskList)
			{
				pTask->dotask();
				delete pTask;
			}
			_TaskList.clear();
		}
	}
}

#endif