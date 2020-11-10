#ifndef _HIGH_RESOLUTION_TIME_
#define _HIGH_RESOLUTION_TIME_
#include<chrono>
using namespace std::chrono;
class MyTimer
{
public:
	MyTimer();
	~MyTimer();
	void update();
	long long GetSeconds();
	long long GetMicroseconds();
	long long GetMilliseconds();
private:
	time_point<high_resolution_clock> _begin;
};

MyTimer::MyTimer()
{
	this->update();
}

MyTimer::~MyTimer()
{

}

void MyTimer::update()
{
	_begin = high_resolution_clock::now();
}

long long MyTimer::GetSeconds()
{
	return this->GetMilliseconds() * 0.000001;
}

long long MyTimer::GetMicroseconds()
{
	return this->GetMilliseconds() * 0.001;
}

long long MyTimer::GetMilliseconds()
{
	return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
}
#endif // !_HIGH_RESOLUTION_TIME_
