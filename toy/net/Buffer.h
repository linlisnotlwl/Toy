#pragma once


namespace Toy
{
namespace net
{

class Buffer
{
public:
    explicit Buffer(size_t size);
    ~Buffer();

    void swap(Buffer & rhs);
    size_t readableSize() const;
    size_t writableSize() const;
    void append(const void * data, size_t len);
    char * begin();
    const char * begin() const;
    

private:

};
} // namespace net
} // namespace Toy