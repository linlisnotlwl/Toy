#include "ToyCo.h"



void fun_one()
{
    printf("1\n");
    TOY_CO_YEILD;
    printf("3\n");
    int k = 0;
    for(int i = 0; i < 1000000; ++i)
    {
        TOY_CO_YEILD;
        k++;
    }
    printf("k = %d.\n", k);
}

void fun_two()
{
    printf("2\n");
    TOY_CO_YEILD;
    printf("4\n");
    int j = 0;
    for(int i = 0; i < 1000000; ++i)
    {
        TOY_CO_YEILD;
        j++;
    }
    printf("j = %d.\n", j);
    TOY_CO_END;
}


int main()
{
    TOY_CO_CREATE(fun_one);
    TOY_CO_CREATE(fun_two);

    TOY_CO_START;
    return 0;
}