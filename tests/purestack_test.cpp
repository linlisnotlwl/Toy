#include <cstdio>
#include <climits> // for INT_MAX INT_MIN
#include "../PureStack.h"

class Test
{
public:
    Test(int f) : test(new char[10]) , flag(f)
    {
        *test = 't'; 
        *(test+1) = 'o';
        *(test+2) = 'y';
        *(test+4) = '\0';
    }
    ~Test() { delete[] test; }
    void print() { printf("class Test%x : %s\n", this, test); }
private:
    char * test;
    int flag;
};

static void test_bulit_in()
{
    printf("........test_bulit_in start\t........\n");
    Toy::PureStack stack;
    *stack.push<char>() = 't';
    *stack.push<char>() = 'o';
    *stack.push<char>() = 'y';
    *stack.push<char>() = '\0';
    printf("%s\n", reinterpret_cast<char *>(stack.getData()));
    stack.pop<char>();
    *stack.push<int>() = 1;
    *stack.push<int>() = -100;
    *stack.push<int>() = INT_MIN;
    *stack.push<int>() = INT_MAX;
    printf("%d\n", *stack.top<int>());
    stack.pop<int>();
    printf("%d\n", *stack.top<int>());
    stack.pop<int>();
    printf("%d\n", *stack.top<int>());
    stack.pop<int>();
    printf("%d\n", *stack.top<int>());
    stack.pop<int>();
    stack.pop<char>(3);
    printf("size : %d\n", stack.getSize());
    // TODO : test resize()
    
    printf("........test_bulit_in end\t........\n");
}
static void test_trivially_copyable()
{
    // TODO
}
static void test_destruct()
{
    printf("........test_destruct start\t........\n");
    Toy::PureStack stack;
    
    Test * tp1 = new(stack.push<Test>()) Test(1);   //placememt new
    tp1->print();

    Test * tp2 = stack.emplace_back<Test>(1);
    tp2->print();
    stack.destruct_pop<Test>();

    Test * tp3 = stack.top<Test>();
    tp3->print();
    tp3->~Test();
    stack.pop<Test>();

    printf("size : %d\n", stack.getSize());
    printf("........test_destruct end\t........\n");
}
static void test_resize()
{
    // TODO
}
static void test()
{
    test_bulit_in();
    test_trivially_copyable();
    test_destruct();
}
int main()
{
    test();
    return 0;
}