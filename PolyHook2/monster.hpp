#pragma once
#include <iostream>
using namespace std;

namespace powerworld
{
	class monster
	{
	public:
		monster(int level);
		int attack();
		bool defence(int hit);

	private:
		int getCombat();
		unsigned long long getHealth();

		unsigned long long m_health;
		int combat;
	};

	monster::monster(int level) : m_health(10 * level + 100), combat(10 + level)
	{
	}

	int monster::attack()
	{
		int power = getCombat();
		if (getHealth() < 10)
			power *= 2;
		return power;
	}

	bool monster::defence(int hit)
	{
		m_health -= hit;
		//if (getHealth() <= 0)
		//{
		//	std::cout << "monster die!" << std::endl;
		//	return false;
		//}

		return true;
	}

	int monster::getCombat()
	{
		return combat;
	}

	unsigned long long monster::getHealth()
	{
		return m_health;
	}
}