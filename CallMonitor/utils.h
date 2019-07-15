#pragma once
#include "stdafx.h"
#include <iostream>
#include <chrono>

static std::chrono::system_clock::time_point start;

static void enter_time()
{
	start = std::chrono::system_clock::now();
}

static void leave_time()
{
	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
	std::cout << "[global] : " << sec.count() << " seconds" << std::endl;
}