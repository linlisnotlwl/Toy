#pragma once
#include "Singleton.h"
#include "Scheduler.h"





#define TOY_GLOBAL_SCHEDULER Toy::Singleton<Toy::Scheduler>::getInstance()
#define TOY_CO_START TOY_GLOBAL_SCHEDULER.start()
#define TOY_CO_END TOY_GLOBAL_SCHEDULER.stop()
#define TOY_CO_YEILD do { Toy::Cohandler::yeild(); } while (0)
//#define TOY_CO(function) do{ TOY_GLOBAL_SCHEDULER.createCoroutine(function); } while (0)
#define TOY_CO_CREATE(function) do{ TOY_GLOBAL_SCHEDULER.createCoroutine(function); } while (0)
//#define TOY_CO_CREATE(function, stack_size) do{ TOY_GLOBAL_SCHEDULER.createCoroutine(function, stack_size); } while (0)

namespace Toy
{



} // namespace Toy

