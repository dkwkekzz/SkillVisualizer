#pragma once
#include <iostream>
using namespace std;

namespace powerworld
{
	class blademaster
	{
	public:
		blademaster();
		~blademaster();
		int attack(int power);
		void defence();
		static void skillmoonblade(int power);

	private:
		unsigned long long m_health;
	};

	blademaster::blademaster() : m_health(999999)
	{
		m_health += 9999;
		m_health += 111111;
	}


	blademaster::~blademaster()
	{
	}

	int blademaster::attack(int power)
	{
		return power;
	}

	void blademaster::defence()
	{
		m_health <<= 1;
	}

	void blademaster::skillmoonblade(int power)
	{
		int ret = 0;
		for (int i = 0; i != 2000; i++)
		{
			ret += power;
		}
		cout << "forceattack -> " << ret << endl;
	}
}
