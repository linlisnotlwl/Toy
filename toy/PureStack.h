#pragma once
#include <cstdlib>  // for malloc free
#include <new>  // for placement new
#include <cassert>

namespace Toy
{
#if 0
    static constexpr int INIT_STACK_SIZE = 10; // test resize
#else
    static constexpr int INIT_STACK_SIZE = 128; // normal
#endif
/**
 * @brief   Container Stack : store any type in same stack. 
 * @warning No safety guarantee. Users should use it correctly.
 * @ref     参考代码：https://github.com/Tencent/rapidjson/blob/v1.0.1/include/rapidjson/internal/stack.h
 */
class PureStack
{
public:
    PureStack() {}
    ~PureStack()
    {
        if(nullptr != data)
            free(data);
        data = nullptr;
        free_space = nullptr;
        memory_size = 0;
    }
    PureStack(const PureStack &) = delete;
    const PureStack & operator=(const PureStack &) = delete;
    // template<typename T>
    // void push(T push_data)
    // {

    // }
    template<typename T>
    T * push(size_t count = 1)
    {
        assert(count >= 0);
        resize(getSize() + sizeof(T) * count);
        T * ret = reinterpret_cast<T *>(free_space);
        free_space += sizeof(T) * count;
        return ret;
    }
    template<typename T, typename... U>
    T * emplace_back(U... args)
    {
        resize(getSize() + sizeof(T));
        T * ret = new(free_space) T(args...);
        free_space += sizeof(T);
        //data_size += sizeof(T);
        return ret;
    }
    template<typename T>
    void pop(size_t count = 1)
    {
        assert(count >= 0);
        assert(getSize() >= sizeof(T) * count); 
        free_space -= sizeof(T) * count;
        //data_size -= sizeof(T) * count;
    }
    template<typename T>
    void destruct_pop(size_t count = 1)
    {
        assert(count > 0);
        assert(getSize() >= sizeof(T) * count);
        for(size_t i = 1; i <= count; ++i)
        {
            reinterpret_cast<T *>(free_space - sizeof(T) * i)->~T();
        } 
        free_space -= sizeof(T) * count;
        //data_size -= sizeof(T) * count;
    }
    template<typename T>
    T* top()
    {
        assert(getSize() >= sizeof(T)); 
        return  reinterpret_cast<T *>(free_space - sizeof(T));
    }
    template<typename T>
    void releaseAll() 
    {   
        size_t num = getSize() / sizeof(T);
        destruct_pop<T>(num);
    }
    inline size_t getSize() const { return static_cast<size_t>(free_space - data); }
    void * getData() const { return reinterpret_cast<void *>(data); }
    void clear() { free_space = data; }
    
private:
    void resize(size_t new_size)
    {
        if(new_size > memory_size)
        {
            if(memory_size == 0)
            {
                memory_size = INIT_STACK_SIZE < new_size ? new_size : INIT_STACK_SIZE; // bug fixed:look history
                data = static_cast<char *>(malloc(memory_size));
                if(data == nullptr)
                {
                    memory_size = 0;
                    throw std::bad_alloc();
                }   
                free_space = data;    
            }
            else
            {
                size_t temp = memory_size;
                size_t data_size = getSize();
                while(new_size > temp)
                    temp += temp >> 1;  // temp = 1.5 * temp;
                data = static_cast<char *>(realloc(data, temp));
                if(data == nullptr)
                {
                    memory_size = 0;
                    throw std::bad_alloc();
                }
                free_space = data + data_size;
                memory_size = temp;
            }   
        }
    }
    /// save data
    char * data = nullptr;
    /// point to space that can be used
    char * free_space = nullptr;
    /// bytes of data
    //size_t data_size = 0;
    size_t memory_size = 0;

};
} // namespace Toy