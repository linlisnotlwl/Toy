#pragma once
#include <cstdio>
#include <string>
#include <cassert>

#ifdef NDEBUG
    #define TOY_ASSERT(expression) assert(expression)
#else
    #define TOY_ASSERT(expression) do { \
            if(!(expression)) \
                Toy::Assert(__FILE__, __LINE__);   \
            assert(expression);  \
        } while (0)   
#endif
    

namespace Toy
{

std::string demangle(const char *symbol);// get function name
std::string Backtrace(int skip_layer, bool & flag);
void Assert(const char * file, int line);


// struct ListNode
// {
//     ListNode * prev = nullptr;
//     ListNode * next = nullptr;
// };

// void setListNodeNext(ListNode * cur, ListNode * next_node)
// {
//     if(cur != nullptr)
//         cur->next = next_node;
// }

// void setListNodePrev(ListNode * cur, ListNode * prev_node)
// {
//     if(cur != nullptr)
//         cur->prev = prev_node;
// }




} // namespace Toy    

