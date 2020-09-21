#include "Util.h"
#include <execinfo.h>
#include <cxxabi.h>

namespace Toy
{
static const size_t BUF_SIZE = 100;

std::string demangle(const char *symbol)
{
    size_t size = 0;
    int status = 0;
    char temp[128] = {0};
    char *demangled = NULL;
    //first, try to demangle a c++ name
    if (sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp) == 1) {
        if ((demangled = abi::__cxa_demangle(temp, NULL, &size,
                        &status)) != NULL) {
            std::string result(demangled);
            free(demangled);
            return result;
        }
    }
    //if that didn't work, try to get a regular c symbol
    if (sscanf(symbol, "%127s", temp) == 1) {
        return temp;
    }
    //if all else fails, just return the symbol
    return symbol;

}

std::string Backtrace(int skip_layer, bool & flag)
{
    void ** buffer = static_cast<void **>(malloc(sizeof(void *) * BUF_SIZE));
    char ** infos;
    int n = backtrace(buffer, BUF_SIZE);
    // backtrace_symbols的实现需要符号名称的支持，在gcc编译过程中需要加入-rdynamic参数
    // Note that names of "static" functions are not exposed, and won't be available in the backtrace.
    infos = backtrace_symbols(buffer, n);
    if(infos == nullptr)
    {
        flag = false;
        return std::string("Backtrace Symbol Error: Can not generate backtrace info.");
    }
    std::string ret;
    for(int i = skip_layer; i < n; ++i)
    {
        ret.append(demangle(infos[i]));
        //ret.append(infos[i]);
        ret.append("\n");
    }
    flag = true;
    free(infos);
    return ret;
}

void Assert(const char * file, int line)
{
    bool flag = false;
    std::string info = Toy::Backtrace(2, flag);
    if (flag)
    {
        printf("-------------------Assert-----------------------\n");
        printf("@File = %s.\n@Line = %d.\n", file ,line);
        printf("%s", info.c_str());
        printf("-------------------Assert-----------------------\n");
    }
}



} // namespace Toy