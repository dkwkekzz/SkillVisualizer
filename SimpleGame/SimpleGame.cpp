// SimpleGame.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "archer.hpp"
#include "monster.hpp"
#include "Static.h"
#include "Alarm.h"
#include "masm.h"
#include <mutex>
#include <thread>

using namespace powerworld;
Suite::Module::Infra::NTime::NAlarm::Alarm alarm;

Suite::Game::Function::NTimer::BuffPackage inst;

mutex m;

class updater
{
public:
	void doing()
	{
		//monster m1(2);
		//archer a1;
		//m1.defence(a1.attack());
		//a1.defence(m1.attack());
		//a1.heal(1);
		//a1.none();
		alarm.Regist(&inst, nullptr, 0);
		//alarm.Regist(&inst, nullptr, 0, nullptr);

	}
};

void test()
{
	using namespace std;

	bool universal = true;
	std::thread world1([&]() {
		while (universal)
		{
			PushVal(111);
			//std::this_thread::sleep_for(100ms);
			auto a = PopVal();
			m.lock();
			std::cout << "thread1: " << a << std::endl;
			m.unlock();
		}
	});

	std::thread world2([&]() {
		while (universal)
		{
			PushVal(222);
			//std::this_thread::sleep_for(100ms);
			auto a = PopVal();
			m.lock();
			std::cout << "thread2: " << a << std::endl;
			m.unlock();
		}
	});

	world1.join();
	world2.join();
}

void testwork1()
{
	int universal = true;
	while (universal)
	{
		alarm.OnTick();
		this_thread::sleep_for(10ms);
	}
}

void testwork2()
{
	updater upd;
	//monster m1(2);
	//archer a1;
	//a1.none();

	int universal = true;
	while (universal)
	{
		//alarm.Regist(&inst, nullptr, 0);

		//m1.defence(a1.attack());
		//a1.defence(m1.attack());
		//a1.heal(1);

		upd.doing();
		this_thread::sleep_for(100ms);
	}
}

int main()
{
	//test();

	HMODULE hdll = LoadLibraryA("HookMan.dll");
	//HMODULE hdll2 = LoadLibraryA("CallMonitor.dll");

    std::cout << "Archer World!\n";

	archer player;
	queue<monster> monque;


	//alarm.Regist(&inst, nullptr, 0);

	//inst.OnAlarm(999);

	std::thread world1([&]() {
		testwork1();
	});

	testwork2();
	//std::thread world2([&]() {
	//	testwork2();
	//});
	//
	//int level = 1;
	//int turn = 0;
	//bool live = true;
	//while (live)
	//{
	//	if (turn++ == 5)
	//	{
	//		level++;
	//		turn = 0;
	//	}
	//
	//	if (turn & 1)
	//	{
	//		monque.emplace(monster(level));
	//		std::cout << "create monster..." << std::endl;
	//	}
	//
	//	monster * pMon = nullptr;
	//	if (!monque.empty())
	//	{
	//		pMon = &monque.front();
	//	}
	//
	//	int in = 0;
	//	std::cin >> in;
	//	if (in == 0)
	//	{	// attack
	//		if (!pMon)
	//		{
	//			std::cout << "no monster..." << std::endl;
	//			continue;
	//		}
	//
	//		if (!pMon->defence(player.attack()))
	//		{
	//			player.addExp(level * 2);
	//			monque.pop();
	//			continue;
	//		}
	//	}
	//	else if (in == 1)
	//	{
	//		player.heal(level);
	//	}
	//	else if (in == 2)
	//	{
	//		if (!pMon)
	//		{
	//			std::cout << "no monster..." << std::endl;
	//			continue;
	//		}
	//
	//		if (!pMon->defence(archer::skillIncredibleShot(level)))
	//		{
	//			player.addExp(level);
	//			monque.pop();
	//			continue;
	//		}
	//	}
	//	else if (in == 9)
	//	{
	//		break;
	//	}
	//
	//	if (pMon)
	//	{
	//		live = player.defence(pMon->attack());
	//	}
	//
	//	if (monque.size() > 100)
	//	{
	//		std::cout << "lose...\n";
	//		live = false;
	//	}
	//}
	//
	////universal = false;
	//world1.join();
	//world2.join();

	std::cin.get();
}