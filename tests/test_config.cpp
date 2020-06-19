#include "../toy/Config.h"
#include "../toy/Log.h"


static void test_config()
{
    if(!Toy::Config::loadConfig("./config.json"))
        printf("Can not load config file.\n");
    Toy::Config::viewAll();
    Toy::ConfigVar<std::string>::Ptr p = Toy::Config::lookup<std::string>("project_name");
    
    p->registerCallback(1, [](const std::string & old_val, const std::string & new_val)
    { 
        std::cout << "Calling Callback[1]. Old_val = " << old_val << "  New_val = " << new_val << std::endl;
    });
    p->setVal("new Project Name");
}

static void test_log_config()
{
    TOY_LOG_DEBUG << "test_log_config" ;
    TOY_LOG_WARN << "test_log_config::warning";
    Toy::Config::viewAll();
}

int main()
{
    test_config();
    test_log_config();
    return 0;
}