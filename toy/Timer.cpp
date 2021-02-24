#include "Timer.h"
#include <algorithm>
#include "Log.h"
#include "Util.h"
namespace Toy
{

TinyTimer::TinyTimer() : _begin_timepoint(std::chrono::high_resolution_clock::now())
{
}


TinyTimer::~TinyTimer()
{
}

void TinyTimer::update()
{
	_begin_timepoint = std::chrono::high_resolution_clock::now();
}

double TinyTimer::getElapsedSecond()
{
	return getElapsedMicroSecond() * 0.000001;
}

double TinyTimer::getElapsedMillSecond()
{
	return getElapsedMicroSecond() * 0.001;
}

long long TinyTimer::getElapsedMicroSecond()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin_timepoint).count();
}

// Tick implementation start:
Tick::Tick()
{
	//TOY_LOG_DEBUG << "Tick constructing: " << this;
}

Tick::~Tick()
{
	//TOY_LOG_DEBUG << "Destructing Tick : " << this;
	//printf("Tick::~Tick(), from %llu.\n", this);
}

Tick::Tick(uint64_t expired_tick, CallbackFun cb, int32_t cycle, uint64_t interval)
	: m_cb(cb), m_cycle(cycle), m_interval(interval), m_expired_tick(expired_tick)
{
	//TOY_LOG_DEBUG << "Tick constructing: " << this;
}

void Tick::setAll(uint64_t expired_tick, CallbackFun cb, int32_t cycle, uint64_t interval)
{
	m_expired_tick = expired_tick;
	m_cb = cb;
	m_cycle = cycle < 0 ? -1 : cycle;
	m_interval = interval == 0 ? 1 : interval;
}

void Tick::nextCycle(uint64_t cur_tick)
{
	m_expired_tick = cur_tick + m_interval;
}

void Tick::runTask()
{
	m_cb();
}

bool Tick::isFinished()
{
	if(m_cycle < 0)
		return false;
	else
		return m_expired_count >= static_cast<uint32_t>(m_cycle);
}

void Tick::incExpiredCount()
{
	++m_expired_count;
	//printf("    Tick::incExpiredCount(), %x\n", this);
}

// Tick implementation end.-------------------------------------------<Tick>

// Wheel implementation start:

Wheel::Wheel(uint64_t slot_num, uint64_t slot_interval)
	: m_slot_num(slot_num), m_slot_interval(slot_interval), 
	m_slots(std::vector<SlotType>(slot_num, SlotType())), m_cur_slot(0), 
	prev(nullptr), next(nullptr)
{
	TOY_LOG_DEBUG << "Wheel constructing : " << this;
}

Wheel::~Wheel()
{
	for (auto &s : m_slots)
		s.clear();
	TOY_LOG_DEBUG << "Wheel::~Wheel finished : " << this;
}

void Wheel::tick(uint64_t tick)
{
	if(m_cur_slot >= m_slot_num)
	{
		m_cur_slot = 0;
		// 超出最大表示范围，因为Max_tick很大，所以将这种情况视为不可能出现的状态
		TOY_ASSERT(next != nullptr);
		next->tick(tick);
		return;
	}
	
	if(!m_slots[m_cur_slot].empty())
	{
		for(auto it = m_slots[m_cur_slot].begin(); it != m_slots[m_cur_slot].end(); )
		{
			auto p = *it;

			// printf("-->Cur_level = %u. Cur_slot = %u. Cur_tick = %u. ExpiredTick = %u\n", 
			// 	getSlotInterval(), m_cur_slot, tick, p->getExpiredTick());
			if(p->getExpiredTick() <= tick)
			{
				p->runTask();
				//printf("run tick !!!\n");
				p->incExpiredCount();

				// circle
				if(!p->isFinished())
				{

					// NOTE: if TimerWheel::update is infrequent, 
					//		 the expired_tick sometimes will be smaller than cur_tick;
					//		 So the next cycle will not be run.
					//		 So here run the next cycle from cur_tick rather than last expired_tick;
					if(p->getExpiredTick() + p->getInterval() <= tick)
						p->nextCycle(tick);
					else
						p->nextCycle(p->getExpiredTick());
					// FIXME: tick bigger than MAX_TICK
					// TODO: make clearly
					//uint64_t more = p->getExpiredTick();
					uint64_t left = p->getExpiredTick() - tick;
					if(left >= getSlotInterval())
					{
						Wheel::Ptr insert_wheel = this;
						while(insert_wheel != nullptr && 
							left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot() + 1 >= insert_wheel->getSlotNum())
							insert_wheel = insert_wheel->getNextWheel();
						if(insert_wheel != nullptr)
						{
							//printf("Left = %u, slot_interval = %u, cur_slot = %u\n", left, insert_wheel->getSlotInterval(), insert_wheel->getCurSlot());
							insert_wheel->addInSlot(left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot(), p);
							// printf("  Readd In whell(interval = %u), slot = %u.\n",
							//  	insert_wheel->getSlotInterval(), left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot());															
						}	
					}
					else
					{
						// find the insert wheel
						
						Wheel::Ptr insert_wheel = this;
						while (insert_wheel != nullptr && left < insert_wheel->getSlotInterval())
						{
							insert_wheel = insert_wheel->getPrevWheel();
						}
						if (insert_wheel != nullptr)
						{
							insert_wheel->addInSlot(left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot(), p);
		
							// printf("\tsmaller%%% Readd In whell(interval = %u), slot = %u.\n",
							// 	   insert_wheel->getSlotInterval(), left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot());
						}
					}
				}
				// else
				// {
				// 	printf("(-_-)TickTask Finish : %x\n", p.get());
				// }
				
			}
			else //添加时，没有更新会导致此种状况出现 // will it happend?? yes // 也行可以在add中加个update试试？？
			{
				// printf("ExpiredTick bigger than cur tick !!!\n");
				// find the insert wheel
				uint64_t left = (p->getExpiredTick() - tick);
				Wheel::Ptr insert_wheel = prev;
				while(insert_wheel != nullptr && left < insert_wheel->getSlotInterval())
				{
					insert_wheel = insert_wheel->getPrevWheel();
				}
				if(insert_wheel != nullptr)
				{
					//printf("Left = %u, slot_interval = %u, cur_slot = %u\n", left, insert_wheel->getSlotInterval(), insert_wheel->getCurSlot());
					insert_wheel->addInSlot(left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot(), p);
					// printf("smaller@@@@ Readd In whell(interval = %u), slot = %u.\n", 
					//  	insert_wheel->getSlotInterval(), left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot());
				}
			}
			it = m_slots[m_cur_slot].erase(it);
		}

	}
	m_cur_slot++;
	
}

bool Wheel::addInSlot(uint64_t slot, Tick::Ptr tick_p)
{
	if(slot >= m_slot_num || tick_p->isFinished())
		return false;
	std::unique_lock<std::mutex> lock(m_slots_mutex);
	m_slots[slot].push_front(tick_p);
	return true;
}

bool Wheel::delInSlot(uint64_t slot, Tick::Ptr tick_p)
{
	if(slot >= m_slot_num)
		return false;
	std::unique_lock<std::mutex> lock(m_slots_mutex);
	auto it = std::find(m_slots[slot].begin(), m_slots[slot].end(), tick_p);
	if(it != m_slots[slot].end())
	{
		TOY_LOG_DEBUG << "Delete Tick::Ptr in Wheel Success.";
		m_slots[slot].erase(it);
		return true;
	}
	else
		return false;
}

uint64_t Wheel::getSlotInterval()
{
	return m_slot_interval;
}

uint64_t Wheel::getSlotNum()
{
	return m_slot_num;
}

void Wheel::setPrevWheel(Wheel::Ptr wheel_p)
{
	prev = wheel_p;
}

void Wheel::setNextWheel(Wheel::Ptr wheel_p)
{
	next = wheel_p;
}

Wheel::Ptr Wheel::getPrevWheel()
{
	return prev;
}

Wheel::Ptr Wheel::getNextWheel()
{
	return next;
}

uint64_t Wheel::getCurSlot()
{
	return m_cur_slot;
}

// Wheel implementation end.-------------------------------------------<Wheel>

// TimerWheel implementation start:

TimerWheel::TimerWheel(int group_num, uint64_t base_interval) 
	: m_last_tick(0),  is_running(false),//m_closest_tick(0),
	m_base_interval(base_interval), m_quit(true)
{
	TOY_LOG_DEBUG << "TimerWheel contructing, from " << this;
	if(0 >= group_num || MAX_WHEEL_NUM < group_num)
		group_num = DEFAULT_WHEEL_NUM;
	//m_wheel_group = WheelGroup(group_num, nullptr);
	uint64_t cur_interval = base_interval;
	for(int i = 0; i < group_num; ++i)
	{
		m_wheel_group.push_back(new Wheel(DEFAULT_SLOT_NUM, cur_interval));
		cur_interval *= DEFAULT_SLOT_NUM;
	}
	// building connection with each wheel
	m_wheel_group[0]->setNextWheel(m_wheel_group[1]);
	m_wheel_group[group_num - 1]->setPrevWheel(m_wheel_group[group_num - 2]);
	for(size_t i = 1; i < m_wheel_group.size() - 1; ++i)
	{
		m_wheel_group[i]->setNextWheel(m_wheel_group[i + 1]);
		m_wheel_group[i]->setPrevWheel(m_wheel_group[i - 1]);		
	}
	// for debug
	// for(auto & p : m_wheel_group)
	// {
	// 	printf("%x\n", p);
	// }
}

TimerWheel::~TimerWheel()
{
	//is_running = false;
	close();
	for(auto & p : m_wheel_group)
		delete p;
	//printf("TimerWheel::~TimerWheel, from %llu.\n", this);
	TOY_LOG_DEBUG << "TimerWheel::~TimerWheel, from " << this;
}

void TimerWheel::start()
{
	m_last_tick = 0;
	is_running = true;
	m_time.update();
}

bool TimerWheel::add(Tick::Ptr empty_tick, uint64_t start_after_the_time, 
	Tick::CallbackFun cb, int32_t cycle, uint64_t interval)
{
	//update(); 是否需要这个update？？
	//std::unique_lock<std::mutex> lock(m_update_mutex);
	uint64_t expired_time = getCurTick() + start_after_the_time;
	empty_tick->setAll(expired_time, cb, cycle, interval);
	
	std::pair<size_t, uint64_t> location = getTickLocation(expired_time);
	//if(location.first >= m_wheel_group.size() || location.second >= m_wheel_group[location.first]->getSlotNum())
	//	return false;

	//printf("Add Tick %u in location(%u,%u).\n", expired_time, location.first, location.second);
	//printf("cycle = %d, trigger time = %d.\n", empty_tick->getCycleNum(), empty_tick->getExeTime());
	return m_wheel_group[location.first]->addInSlot(location.second, empty_tick);
}

bool TimerWheel::add(uint64_t start_after_the_time, Tick::CallbackFun cb, 
	int32_t cycle, uint64_t interval)
{
	return add(std::make_shared<Tick>(), start_after_the_time, cb, cycle, interval);
	// make_shared 调用的构造函数未初始化 expired_count,导致出现添加定时任务失败的bug，现已改正。
}

bool TimerWheel::del(Tick::Ptr tick_p)
{
	update();
	auto location = getTickLocation(tick_p->getExpiredTick());
	return m_wheel_group[location.first]->delInSlot(location.second, tick_p);
}

void TimerWheel::update()
{
	// 如果当前没有任务，则将起始时间更新为当前时间，但还是没解决时间溢出问题
	// 解决办法：把MAX_TICK设置成很大的数(UINT64_MAX)，一百年哈哈
	std::unique_lock<std::mutex> lock(m_update_mutex);
	if(is_running)
	{
		uint64_t cur_tick = getCurTick();
		//printf("Update Now, cur_tick = %u.\n", cur_tick);
		// if (cur_tick < m_closest_tick)
		// {
		// 	m_last_tick = cur_tick;
		// 	return;
		// }

		if(cur_tick > m_last_tick)
		{
			uint64_t diff = cur_tick - m_last_tick;
			uint64_t temp = m_last_tick;
			while(diff-- > 0)
				tick(temp++);
		}
		m_last_tick = cur_tick;
	}
}

void TimerWheel::autoUpdate()
{
	std::unique_lock<std::mutex> lock(m_quit_mutex);
	m_quit = false;
	std::thread t( [this](){ this->TimerWheel::autoUpdateThreadFun(); } );
	t.detach();
}

void TimerWheel::autoUpdateThreadFun()
{
	while(is_running)
	{
		// why 300, test from avg latency
		std::this_thread::sleep_for(std::chrono::microseconds(300)); // milliseconds(1)
        update();
	}
	// 这里用信号会导致，先notify后wait
	//m_quit_sema.notify();
	std::unique_lock<std::mutex> lock(m_quit_mutex);
	m_quit = true;
	m_cv.notify_all();

}

void TimerWheel::close()
{
	is_running = false;
	std::unique_lock<std::mutex> lock(m_quit_mutex);
	if(!m_quit)
		m_cv.wait(lock, [this](){ return this->m_quit; });
	//m_quit_sema.wait();
	m_last_tick = 0;
}

uint64_t TimerWheel::getCurTick()
{
	uint64_t elapsed_time = static_cast<uint64_t>(m_time.getElapsedMillSecond());
	return elapsed_time / m_base_interval;
}

void TimerWheel::tick(uint64_t tick)
{
	if(is_running)
	{
		m_wheel_group[0]->tick(tick);
	}
}


std::pair<size_t, uint64_t> TimerWheel::getTickLocation(uint64_t expired_time)
{
	size_t wheel_index = 0;
	uint64_t slot_index = 0;
	uint64_t left = expired_time - m_last_tick;
	for(size_t i = 0; i < m_wheel_group.size(); ++i)
	{
		slot_index = left / m_wheel_group[i]->getSlotInterval() + m_wheel_group[i]->getCurSlot();
		if(slot_index < m_wheel_group[i]->getSlotNum())
		{
			wheel_index = i;
			break;
		}
	}
	return std::pair<size_t, uint64_t>(wheel_index, slot_index);
}


// TimerWheel implementation end.-------------------------------------------<TimerWheel>

} // namespace Toy


