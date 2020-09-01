#include "Timer.h"
#include <algorithm>
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

// Tick implementation start:
Tick::Tick()
{

}

Tick::Tick(uint64_t expired_tick, CallbackFun cb, int32_t cycle, uint64_t interval)
	: m_expired_tick(expired_tick), m_cb(cb), m_cycle(cycle), m_interval(interval)
{

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
	if(m_cycle == -1)
		return true;
	else
		return m_expired_count >= m_cycle;
}

void Tick::incExpiredCount()
{
	m_expired_count++;
}

// Tick implementation end.-------------------------------------------<Tick>

// Wheel implementation start:

Wheel::Wheel(uint64_t slot_num, uint64_t slot_interval)
	: m_slot_num(slot_num), m_slot_interval(slot_interval), 
	m_cur_slot(0), m_slots(std::vector<SlotType>(slot_num, SlotType())),
	prev(nullptr), next(nullptr)
{

}

Wheel::~Wheel()
{
	// for (auto &s : m_slots)
	// 	for (auto &p : s)
	// 		delete p;
}

void Wheel::tick(uint64_t tick)
{
	if(m_cur_slot >= m_slot_num)
	{
		m_cur_slot = 0;
		// FIXME : next == nullptr
		next->tick(tick);
		return;
	}
	if(!m_slots[m_cur_slot].empty())
	{
		for(auto it = m_slots[m_cur_slot].begin(); it != m_slots[m_cur_slot].end(); )
		{
			auto p = *it;
			// if (p->getExpiredTick() < tick)
			// {
			// 	printf("Something happend, ExpiredTick = %ud, cur_tick = %ud.\n", p->getExpiredTick(), tick);
			// }
			printf("-->Cur_level = %u. Cur_slot = %u.\n", getSlotInterval(), m_cur_slot);
			if(p->getExpiredTick() <= tick)
			{
				p->runTask();
				p->incExpiredCount();
				it = m_slots[m_cur_slot].erase(it);
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
					// TODO: make clear
					uint64_t more = p->getExpiredTick();
					uint64_t left = p->getExpiredTick() - tick;
					if(left >= getSlotInterval())
					{
						Wheel::Ptr insert_wheel = this;
						while(insert_wheel != nullptr && 
							left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot() >= insert_wheel->getSlotNum())
							insert_wheel = insert_wheel->getNextWheel();
						if(insert_wheel != nullptr)
						{
							if (left / insert_wheel->getSlotInterval() == 0)
								insert_wheel->addInSlot(insert_wheel->getCurSlot() + 1, p);
							else
								insert_wheel->addInSlot(left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot(), p);

							printf("  left = %u \n", left);
							printf("  Readd In whell(interval = %u), slot = %u.\n",
								   insert_wheel->getSlotInterval(), left / insert_wheel->getSlotInterval() + insert_wheel->getCurSlot());															
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
							insert_wheel->addInSlot(left / insert_wheel->getSlotInterval(), p);
							//printf("left = %u, left / insert_wheel->getSlotInterval() = %u.\n", left,left / insert_wheel->getSlotInterval());
							printf("\tsmaller%%% Readd In whell(interval = %u), slot = %u.\n",
								   insert_wheel->getSlotInterval(), left / insert_wheel->getSlotInterval());
						}
					}
				}
				else
				{
					printf("(-_-)TickTask Finish : %x\n", p.get());
				}
				
			}
			else // TODO: will it happend??
			{
				// find the insert wheel
				uint64_t left = (p->getExpiredTick() - tick);
				Wheel::Ptr insert_wheel = prev;
				while(insert_wheel != nullptr && left < insert_wheel->getSlotInterval())
				{
					insert_wheel = insert_wheel->getPrevWheel();
				}
				if(insert_wheel != nullptr)
				{
					insert_wheel->addInSlot(left % insert_wheel->getSlotInterval(), p);
					printf("smaller@@@@ Readd In whell(interval = %u), slot = %u.\n", 
						insert_wheel->getSlotInterval(), left % insert_wheel->getSlotInterval());
				}
					
				it = m_slots[m_cur_slot].erase(it);
			}
		}
	}
	m_cur_slot++;		
}

void Wheel::addInSlot(uint64_t slot, Tick::Ptr tick_p)
{
	if(slot >= m_slot_num)
		return;
	m_slots[slot].push_front(tick_p);
}

void Wheel::delInSlot(uint64_t slot, Tick::Ptr tick_p)
{
	if(slot >= m_slot_interval)
		return;
	auto it = std::find(m_slots[slot].begin(), m_slots[slot].end(), tick_p);
	if(it != m_slots[slot].end())
		m_slots[slot].erase(it);
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
	: m_last_tick(0), m_closest_tick(0), is_running(false),
	m_base_interval(base_interval)
{
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
	for(auto & p : m_wheel_group)
	{
		printf("%x\n", p);
	}
}

TimerWheel::~TimerWheel()
{
	//is_running = false;
	close();
	for(auto & p : m_wheel_group)
		delete p;
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
	update();
	uint64_t expired_time = getCurTick() + start_after_the_time;
	empty_tick->setAll(expired_time , cb, cycle, interval);
	std::pair<size_t, uint64_t> location = getTickLocation(expired_time);
	//if(location.first >= m_wheel_group.size() || location.second >= m_wheel_group[location.first]->getSlotNum())
	//	return false;

	printf("Add Tick %u in location(%u,%u).\n", expired_time, location.first, location.second);
	m_wheel_group[location.first]->addInSlot(location.second, empty_tick);
	return true;
}	

bool TimerWheel::del(Tick::Ptr tick_p)
{
	auto location = getTickLocation(tick_p->getExpiredTick());
	m_wheel_group[location.first]->delInSlot(location.second, tick_p);
}

void TimerWheel::update()
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

void TimerWheel::close()
{
	
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

int TimerWheel::getIndex()
{
	
}

std::pair<size_t, uint64_t> TimerWheel::getTickLocation(uint64_t expired_time)
{
	size_t wheel_index = 0;
	uint64_t slot_index = expired_time;
	uint64_t temp = 0;
	for(int i = 0; i < m_wheel_group.size(); ++i)
	{
		temp = slot_index / DEFAULT_SLOT_NUM;
		if(temp == 0)
		{
			break;
		}
		wheel_index++;	
		slot_index = temp;
	}
	return std::pair<size_t, uint64_t>(wheel_index, slot_index);
}


// TimerWheel implementation end.-------------------------------------------<TimerWheel>




} // namespace Toy


