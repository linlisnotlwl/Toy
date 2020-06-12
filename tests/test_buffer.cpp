#include "Buffer.h"

static void test()
{
    using namespace Toy;
    static constexpr size_t BUFFER_SIZE = 4000 * 1024;
    //init
    FixBuffer<BUFFER_SIZE> buffer;
    size_t size = 0;
    for(int i = 0; i < 100; ++i)
    {
        std::string temp("helloworld");
        temp += std::to_string(i);
        buffer.append(temp.c_str(), temp.length());
        size += temp.length();
    }
    printf("size = %lu, buffer size = %lu\n", size, buffer.getDataSize());
    printf("should left = %lu, actual left = %lu\n", BUFFER_SIZE - size, buffer.getFreeSize());
    auto i1 = buffer.getData();
    //auto i2 = buffer.getCurrentPos();
    auto i3 = buffer.empty();
    if(i3)
        printf("it is empty.\n");
    FixBuffer<BUFFER_SIZE> buffer2;
    swap(buffer, buffer2);
    if(i1 == buffer.getData())
        printf("swap success.\n");
    buffer2.setZero();
    printf("char = %c.\n", *buffer2.getData());

}

int main()
{
    test();
    return 0;
}