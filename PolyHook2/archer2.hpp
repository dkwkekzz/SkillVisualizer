#pragma once
#include <iostream>
using namespace std;

namespace powerworld
{
	class archer2
	{
	public:
		archer2();
		~archer2();
		int attack(int power);
		void defence();
		static void forceattack(int power);

	private:
		unsigned long long m_health;
	};

	archer2::archer2() : m_health(9999)
	{
	}


	archer2::~archer2()
	{
	}

	int archer2::attack(int power)
	{
		m_health -= 2;
		cout << "attack -> " << power << endl;
		return power * 99;
	}

	void archer2::defence()
	{
		cout << "defence -> " << (m_health << 1) << endl;
	}

	void archer2::forceattack(int power)
	{
		int ret = 0;
		for (int i = 0; i != 1000; i++)
		{
			ret += power;
		}
		cout << "forceattack -> " << ret << endl;
	}
}
