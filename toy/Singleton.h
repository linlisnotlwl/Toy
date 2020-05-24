#pragma once
#include "Noncopyable.h"
#include <mutex>    // for std::call_once
#include <memory>	// for smart pointer
#include <cassert>
#include <atomic>

namespace Toy
{
// The discussion of thread-safe singleton:
// (1) using static local member (thread-safe after C++11):
//     https://stackoverflow.com/questions/1661529/is-meyers-implementation-of-the-singleton-pattern-thread-safe
//template<typename T>
//class Singleton : Noncopyable      // default private: set base class's public & protected members as it's private members
//{
//public:
//    static T & getInstance()
//    {
//        static T instance;     // without using new
//        return instance;
//    }
//private:
//	Singleton() = default;
//	~Singleton() = default;
//};


// (2) using call_once (C++11 support)	 // same as pthread_once()
template<typename T>
class Singleton : Noncopyable
{
public:
	typedef std::unique_ptr<T> Ptr;
    static T & getInstance()
    {
        static std::once_flag flag;
		//static Ptr m_instance_ptr;
		// it can only use default construct of class T.
		// if we want to use other construct,
		// we can add another level of indirection with template specialization.
		std::call_once(flag, []() { m_instance_ptr.reset(new T()); });
        assert(m_instance_ptr != nullptr);
        return *m_instance_ptr;
    }
private:
    static Ptr m_instance_ptr;
    Singleton() = default;
    ~Singleton() = default;
};
template<typename T>
typename Singleton<T>::Ptr Singleton<T>::m_instance_ptr = nullptr;
//std::unique_ptr<T> Singleton<T>::m_instance_ptr = nullptr;

//// (3) eager initialize : initialize at the very first beginning
//template<typename T>
//class Singleton : Noncopyable
//{
//public:
//	typedef std::unique_ptr<T> Ptr;
//	static T & getInstance()
//	{
//		return *m_instance_ptr;
//	}
//private:
//	static Ptr m_instance_ptr;
//	Singleton() = default;
//	~Singleton() = default;
//};
//template<typename T>
//typename Singleton<T>::Ptr Singleton<T>::m_instance_ptr = std::unique_ptr<T>(new T());

// (4) using C++11 memory model : memory fence(Killing a chicken with ox kinfe. ^-^)
//// Normal DCLP : It's not thread-safe in some mutil-core platform.
//// http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf
//template<typename T>
//class Singleton : Noncopyable
//{
//public:
//	typedef std::unique_ptr<T> Ptr;
//	static T & getInstance()
//	{
//		if (m_instance_ptr == nullptr)
//		{
//			std::unique_lock<std::mutex> lock(m_mutex);
//			if (m_instance_ptr == nullptr)
//			{
//				m_instance_ptr.reset(new T());
//			}
//		}
//		assert(m_instance_ptr != nullptr);
//		return *m_instance_ptr;
//	}
//private:
//	Singleton() = default;
//	~Singleton() = default;
//	static Ptr m_instance_ptr;
//	static std::mutex m_mutex;
//};
//template<typename T>
//typename Singleton<T>::Ptr Singleton<T>::m_instance_ptr = nullptr;
//template<typename T>
//std::mutex Singleton<T>::m_mutex;

// Improve DCLP with C++ memory barrier.
//template<typename T>
//class Singleton : Noncopyable
//{
//public:
//	//typedef std::shared_ptr<T> Ptr;
//	static T & getInstance()
//	{
//		T * temp = m_instance_ptr.load(std::memory_order_relaxed);
//		std::atomic_thread_fence(std::memory_order_acquire);
//
//			if (temp == nullptr)
//			{
//				std::unique_lock<std::mutex> lock(m_mutex);
//				temp = m_instance_ptr.load(std::memory_order_relaxed);
//				if (temp == nullptr)
//				{
//					temp = new T();
//					std::_Atomic_thread_fence(std::memory_order_release);
//					m_instance_ptr.store(temp, std::memory_order_relaxed);
//				}
//			}
//		assert(temp != nullptr);
//		return *temp;
//	}
//private:
//	Singleton() = default;
//	~Singleton() = default;
//	static std::atomic<T *> m_instance_ptr;
//	static std::mutex m_mutex;
//};
//template<typename T>
//std::atomic<T *> Singleton<T>::m_instance_ptr = nullptr;
//template<typename T>
//std::mutex Singleton<T>::m_mutex;

// Newest version(only support by C++20) using shared_ptr: 
//	Becasue atomic<T> requires T to be trivially copyable;
//  So shared_ptr<T> should be trivially copyable. 
//  But shared_ptr<T> is not(test by std::is_trivially_copyable_v<std::shared_ptr<T>> in C++11). 
//	In C++20,  atomic<shared_ptr<T>> has been supported.
//  A trivially copyable class is a class (defined with class, struct or union) that:
//		1.Every copy constructor is trivial or deleted
//		2.Every move constructor is trivial or deleted
//		3.Every copy assignment operator is trivial or deleted
//		4.Every move assignment operator is trivial or deleted
//		5.at least one copy constructor, move constructor, copy assignment operator, or move assignment operator is non - deleted
//		6.Trivial non - deleted destructor
//		This implies that the class has no virtual functions or virtual base classes.
//		Scalar types and arrays of TriviallyCopyable objects are TriviallyCopyable as well.

//template<typename T>
//class Singleton : Noncopyable
//{
//public:
//	typedef std::shared_ptr<T> Ptr;
//	static T & getInstance()
//	{
//		Ptr temp = m_instance_ptr.load(std::memory_order_relaxed);
//		std::atomic_thread_fence(std::memory_order_acquire);
//
//		if (temp == nullptr)
//		{
//			std::unique_lock<std::mutex> lock(m_mutex);
//			temp = m_instance_ptr.load(std::memory_order_relaxed);
//			if (temp == nullptr)
//			{
//				temp = Ptr(new T());
//				std::_Atomic_thread_fence(std::memory_order_release);
//				m_instance_ptr.store(temp, std::memory_order_relaxed);
//			}
//		}
//		assert(temp != nullptr);
//		return *temp;
//	}
//private:
//	Singleton() = default;
//	~Singleton() = default;
//	static std::atomic<Ptr> m_instance_ptr;
//	static std::mutex m_mutex;
//};
//template<typename T>
//std::atomic<std::shared_ptr<T>> Singleton<T>::m_instance_ptr = nullptr;
//template<typename T>
//std::mutex Singleton<T>::m_mutex;

}// namespace Toy