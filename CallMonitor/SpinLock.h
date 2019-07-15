#pragma once
#include <atomic>
#include "ISync.h"

class SpinLock : public ISync
{
	std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
	virtual void Enter() override;
	virtual void Leave() override;

private:
	inline void lock() {
		while (locked.test_and_set(std::memory_order_acquire)) { ; }
	}
	inline void unlock() {
		locked.clear(std::memory_order_release);
	}
};