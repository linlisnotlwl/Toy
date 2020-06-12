#include "Config.h"

namespace Toy
{

// xx implementation start:
// xx implementation end.-------------------------------------------<xx>


// ConfigVarBase implementation start:

// ConfigVarBase implementation end.-------------------------------------------<ConfigVarBase>

// Config implementation start:
Config::Config()
{
}

Config::~Config()
{
}

std::mutex Config::s_mutex;
typename Config::ConfigVarMap Config::s_config_vars;
// Config implementation end.-------------------------------------------<Config>


} // namespace Toy