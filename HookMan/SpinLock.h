#pragma once
#include <atomic>

class SpinLock
{
	std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
	inline void lock() {
		while (locked.test_and_set(std::memory_order_acquire)) { ; }
	}
	inline void unlock() {
		locked.clear(std::memory_order_release);
	}
};