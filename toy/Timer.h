#pragma once
#include <chrono>

namespace Toy
{

class Timer
{
public:
	Timer();
	~Timer();
	void update();	// update start time point
	
	// get Elapsed Time from update() or init
	double getElapsedSecond(); 
	double getElapsedMillSecond(); 
	long long getElapsedMicroSecond(); 
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin_timepoint;	// start time point
};

}


