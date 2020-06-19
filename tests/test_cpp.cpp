#include <iostream>
#include <memory> // shared_ptr
#include <mutex>


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
static void test_dynamic_typeid()
{
    Base * dp = new Derived<double>();
    dp->print();
    cast<double>(dp);

}

int main()
{
    test_dynamic_typeid();
    return 0;
}