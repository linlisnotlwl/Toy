#pragma once
#include <chrono>
#include <list>
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "Noncopyable.h"
#include "Semaphore.h"

namespace Toy
{

class TinyTimer
{
public:
	TinyTimer();
	~TinyTimer();
	void update();	// update start time point
	
	// get Elapsed Time from update() or init
	double getElapsedSecond(); 	// s
	double getElapsedMillSecond(); // ms
	long long getElapsedMicroSecond(); // us

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin_timepoint;	// start time point
};

class Tick
{
public:
	// TODO: callback函数改成模板参数，从而可以实现不同的函数形式
	typedef std::function<void ()> CallbackFun;
	typedef std::shared_ptr<Tick> Ptr;

	Tick();
	Tick(uint64_t expired_tick, CallbackFun cb, 
		int32_t cycle = 1, uint64_t interval = 1);
	void setAll(uint64_t expired_tick, CallbackFun cb, 
		int32_t cycle, uint64_t interval);
	void nextCycle(uint64_t cur_tick); // set to next expired_time
	uint64_t getExpiredTick(){ return m_expired_tick; }
	uint64_t getInterval(){ return m_interval; }
	void runTask();	// run callback
	bool isFinished();
	void incExpiredCount();

private:
	CallbackFun m_cb;
	//void * m_cb_data;
	int32_t m_cycle;	// execute times, -1 means infinite
	uint32_t m_expired_count; //
	uint64_t m_interval; // 每执行一次的间隔时间
	//uint64_t m_id; // for delete
	uint64_t m_expired_tick;
};

static constexpr uint64_t DEFAULT_SLOT_NUM = 256;
class Wheel
{
public:
	typedef std::list<Tick::Ptr> SlotType;
	typedef Wheel * Ptr;
	Wheel(uint64_t slot_num, uint64_t slot_interval);
	~Wheel();
	void tick(uint64_t tick);
	// modify tick in slot
	void addInSlot(uint64_t slot, Tick::Ptr tick_p);
	bool delInSlot(uint64_t slot, Tick::Ptr tick_p);
	// get wheel attribute
	uint64_t getSlotInterval();
	uint64_t getSlotNum();
	// set & get connection of each wheel
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
	//int m_wheel_level;
};

static constexpr int MAX_WHEEL_NUM = 8;
static constexpr int DEFAULT_WHEEL_NUM = 8;
//static constexpr uint64_t MAX_TICK = 777600000; // 60^5 = 216hours = 9 days

/**
 * @brief 时间轮计时器
 * 
 */
class TimerWheel : public Noncopyable
{
public:
	typedef std::vector<Wheel::Ptr> WheelGroup;
	typedef uint64_t TimeDuration;
	explicit TimerWheel(int group_num, uint64_t base_interval);
	~TimerWheel();
	void start();
	/**
	 * @brief 添加定时任务,线程安全
	 * 
	 * @param empty_tick 空tick，用来保存该任务，可用于删除
	 * @param start_after_the_time 启动时间，在设定的时间后启动，单位ms
	 * @param cb 回调函数，执行任务
	 * @param cycle 循环次数
	 * @param interval 循环的间隔时间，单位ms
	 */
	bool add(Tick::Ptr empty_tick, uint64_t start_after_the_time, Tick::CallbackFun cb, 
		int32_t cycle = 1, uint64_t interval = UINT64_MAX);
	bool add(uint64_t start_after_the_time, Tick::CallbackFun cb, 
		int32_t cycle = 1, uint64_t interval = UINT64_MAX);
	bool del(Tick::Ptr);
	void update(); // update time to run tick task
	void autoUpdate(); // using another thread to update the timerwheel
	void close();
	uint64_t getCurTick();
private:
	std::pair<size_t, uint64_t> getTickLocation(uint64_t expired_time);
	void tick(uint64_t tick);

	void  autoUpdateThreadFun();

	WheelGroup m_wheel_group;
	uint64_t m_last_tick;
	//uint64_t m_cur_tick;
	//uint64_t m_closest_tick; // ??you bi yao ma
	bool is_running;
	uint64_t m_base_interval;
	TinyTimer m_time;

	//std::thread m_update_thread;
	Semaphore m_quit_sema;
	std::condition_variable m_cv;
	std::mutex m_quit_mutex;
	bool m_quit;

};

} // namespace Toy


