#pragma once

namespace Toy
{
class Noncopyable
{
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;

public:
	// Note: Scott Meyers mentions in his Effective Modern
	//       C++ book, that deleted functions should generally
	//       be public as it results in better error messages
	//       due to the compilers behavior to check accessibility
	//       before deleted status
    Noncopyable(const Noncopyable &) = delete;
    const Noncopyable &operator=(const Noncopyable &) = delete;
};
}// namespace Toy
