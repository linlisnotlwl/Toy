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

void Config::transformJV2CV(const JsonVar &jv, std::string str)
{
    if (str.length() == 0) // avoid those json value that can not be transform
    {
        if (jv.getType() == JsonVar::JsonType::OBJECT)
        {
            const JsonVar::Object *objects = jv.getObject();
            for (auto it = objects->begin(); it != objects->end(); ++it)
                transformJV2CV(it->second, it->first);
        }
    }
    else
    {
        switch (jv.getType())
        {
        case JsonVar::JsonType::OBJECT:
        {
            const JsonVar::Object *object = jv.getObject();
            for (auto it = object->begin(); it != object->end(); ++it)
                transformJV2CV(it->second, str + "." + it->first);
            break;
        }
        case JsonVar::JsonType::TRUE:
        case JsonVar::JsonType::FALSE:
            lookup<bool>(str, jv.getBoolVal());
            break;
        case JsonVar::JsonType::NUMBER:
            lookup<double>(str, jv.getNumberVal());
            break;
        case JsonVar::JsonType::STRING:
            lookup<std::string>(str, jv.getCStr());
            break;
        case JsonVar::JsonType::ARRAY:
        {
            const JsonVar::Array *array = jv.getArray();
            for (size_t i = 0; i < array->size(); ++i)
                transformJV2CV((*array)[i], str + "[" + std::to_string(i) + "]");
            break;
        }
        default:
            break;
        }
    }
}

std::mutex Config::s_mutex;
typename Config::ConfigVarMap Config::s_config_vars;
// Config implementation end.-------------------------------------------<Config>


} // namespace Toy