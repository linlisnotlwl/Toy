#include "Timer.h"

namespace Toy
{

Timer::Timer() : _begin_timepoint(std::chrono::high_resolution_clock::now())
{
}


Timer::~Timer()
{
}

void Timer::update()
{
	_begin_timepoint = std::chrono::high_resolution_clock::now();
}

double Timer::getElapsedSecond()
{
	return getElapsedMicroSecond() * 0.000001;
}

double Timer::getElapsedMillSecond()
{
	return getElapsedMicroSecond() * 0.001;
}

long long Timer::getElapsedMicroSecond()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin_timepoint).count();
}

} // namespace Toy


