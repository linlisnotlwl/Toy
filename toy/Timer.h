#pragma once
#include <chrono>
#include <list>
#include <vector>
#include <functional>
#include <memory>

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

class TickNode
{
public:
	TickNode();

private:
	TickNode * prev;
	TickNode * next;
};
class Tick
{
public:
	typedef std::function<void (void *)> CallBack;
	typedef std::shared_ptr<Tick> Ptr;
private:
	CallBack m_cb;
	void * m_cb_data;
	uint32_t m_cycle;
	uint32_t m_expired_count;
	uint64_t m_interval; // 每执行一次的间隔时间
	uint64_t m_id; // for delete

};

class Wheel
{
public:
	typedef std::list<Tick::Ptr> SlotType;
	typedef Wheel * Ptr;
private:
	uint64_t m_slot_num;
	uint32_t m_tick; // tick值，表示一个tick代表的毫秒数
	std::vector<SlotType> m_slots; // every slot contain a list of TickNode
};

class TimerWheel
{
public:
	explicit TimerWheel(int group_num, uint32_t base_interval, );
	void start(uint64_t absolute_ticks);
	bool add();
	bool del();
	void update();
	void close();

private:
	void tick();
	std::vector<Wheel::Ptr> m_Wheel_group;
	uint64_t m_last_time;
	uint64_t m_cur_tick;
	uint64_t m_closest_tick;
	bool is_running;
};


}


