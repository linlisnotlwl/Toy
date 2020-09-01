#pragma once
#include <chrono>
#include <list>
#include <vector>
#include <functional>
#include <memory>
#include "Noncopyable.h"

namespace Toy
{

class Timer
{
public:
	Timer();
	~Timer();
	void update();	// update start time point
	
	// get Elapsed Time from update() or init
	double getElapsedSecond(); 	// s
	double getElapsedMillSecond(); // ms
	long long getElapsedMicroSecond(); // us


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
	typedef std::function<void ()> CallbackFun;
	typedef std::shared_ptr<Tick> Ptr;

	Tick();
	Tick(uint64_t expired_tick, CallbackFun cb, 
		int32_t cycle = 1, uint64_t interval = 1);
	void setAll(uint64_t expired_tick, CallbackFun cb, 
		int32_t cycle, uint64_t interval);
	void nextCycle(uint64_t cur_tick);
	uint64_t getExpiredTick(){ return m_expired_tick; }
	uint64_t getInterval(){ return m_interval; }
	void runTask();
	bool isFinished();
	void incExpiredCount();

private:
	CallbackFun m_cb;
	void * m_cb_data;
	int32_t m_cycle;	// execute times, -1 means infinite
	uint32_t m_expired_count; //
	uint64_t m_interval; // 每执行一次的间隔时间
	uint64_t m_id; // for delete
	uint64_t m_expired_tick;
};

static constexpr uint64_t DEFAULT_SLOT_NUM = 60;
class Wheel
{
public:
	typedef std::list<Tick::Ptr> SlotType;
	typedef Wheel * Ptr;
	Wheel(uint64_t slot_num, uint64_t slot_interval);
	~Wheel();
	void tick(uint64_t tick);
	void addInSlot(uint64_t slot, Tick::Ptr tick_p);
	void delInSlot(uint64_t slot, Tick::Ptr tick_p);
	uint64_t getSlotInterval();
	uint64_t getSlotNum();
	void setPrevWheel(Wheel::Ptr);
	void setNextWheel(Wheel::Ptr);
	Wheel::Ptr getPrevWheel();
	Wheel::Ptr getNextWheel();
	uint64_t getCurSlot();
private:
	uint64_t m_slot_num;
	uint64_t m_slot_interval; // tick值，表示一个tick代表的毫秒数
	std::vector<SlotType> m_slots; // every slot contain a list of TickNode
	uint64_t m_cur_slot;
	Wheel::Ptr prev;
	Wheel::Ptr next;
	int m_wheel_level;
};

static constexpr int MAX_WHEEL_NUM = 5;
static constexpr int DEFAULT_WHEEL_NUM = 5;
static constexpr uint64_t MAX_TICK = 777600000; // 60^5 216hour
class TimerWheel : public Noncopyable
{
public:
	typedef std::vector<Wheel::Ptr> WheelGroup;
	explicit TimerWheel(int group_num, uint64_t base_interval);
	~TimerWheel();
	void start();
	bool add(Tick::Ptr, uint64_t, Tick::CallbackFun, 
		int32_t cycle = 1, uint64_t interval = -1);
	bool del(Tick::Ptr);
	void update();
	//void autoUpdate();
	void close();
	uint64_t getCurTick();
	std::pair<size_t, uint64_t> getTickLocation(uint64_t expired_time);

private:
	void tick(uint64_t tick);
	int getIndex();
	WheelGroup m_wheel_group;
	uint64_t m_last_tick;
	//uint64_t m_cur_tick;
	uint64_t m_closest_tick; // ??you bi yao ma
	bool is_running;
	uint64_t m_base_interval;
	Timer m_time;

};


}


