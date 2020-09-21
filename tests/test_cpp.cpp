#include <iostream>
#include <memory> // shared_ptr
#include <mutex>
#include "Util.h"

class Base
{
public:
    virtual ~Base(){}
    virtual void print() { printf("Base.\n"); }

};
template<typename T>
class Derived : public Base
{
public:
    virtual void print() { printf("Derived.\n"); }
    T getVal() 
    {
        std::unique_lock<std::mutex> lock(mutex);
        return val; 
    }

private:
    std::mutex mutex;
    T val;
};

template<typename T>
static void cast(Base * p)
{
    dynamic_cast<Derived<T> *>(p)->print();
    std::cout << dynamic_cast<Derived<T> *>(p)->getVal() << std::endl;
}

// template<typename T>
// static void cast(std::shared_ptr<Base> p)
// {
//     std::dynamic_pointer_cast<Derived<T>>(p)->print();

// }

void test_toy_assert()
{
    int i = 1;
    int j = 2;
  
    TOY_ASSERT(i == j);

}
void test_dynamic_typeid()
{
    Base * dp = new Derived<double>();
    dp->print();
    cast<double>(dp);
    test_toy_assert();

}



int main()
{
    test_dynamic_typeid();
    //test_toy_assert();
    return 0;
}