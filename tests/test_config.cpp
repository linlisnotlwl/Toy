#include "../toy/Config.h"


static void test()
{
    if(!Toy::Config::loadConfig("./config.json"))
        printf("Can not load config file.\n");
    Toy::Config::viewAll();
}

int main()
{
    test();
    return 0;
}