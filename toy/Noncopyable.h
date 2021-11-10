#pragma once

namespace Toy
{
// boost 表示它被用作私有继承
// noncopyable不声明虚析构函数，即不被设计为公共继承链的基础。永远从它私有继承。
// 一些讨论：https://stackoverflow.com/questions/5654330/privately-or-publicly-inherit-from-boostnon-copyable
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
    Noncopyable & operator=(const Noncopyable &) = delete;
};
}// namespace Toy
