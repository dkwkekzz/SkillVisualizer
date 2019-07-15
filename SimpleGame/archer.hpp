#pragma once
#include "pch.h"

namespace powerworld
{
	class archer
	{
	public:
		archer();
		~archer();
		int attack();
		bool defence(int hit);
		void heal(int level);
		void addExp(int val);
		void none();
		static int skillIncredibleShot(int power);

	private:
		int getCombat();
		unsigned long long getHealth();

		unsigned long long m_health;
		int combat;
	};

	archer::archer() : m_health(1000), combat(10)
	{
	}


	archer::~archer()
	{
	}

	int archer::attack()
	{
		int power = getCombat();
		if (getHealth() < 10)
			power *= 2;
		return power;
	}

	bool archer::defence(int hit)
	{
		m_health -= hit;
		if (getHealth() <= 0)
		{
			std::cout << "you die!" << std::endl;
			return false;
		}

		return true;
	}

	void archer::heal(int level)
	{
		m_health += getCombat() * 10;
	}

	int archer::skillIncredibleShot(int level)
	{
		return level * 10000;
	}

	void archer::addExp(int val)
	{
		m_health += val * 10;
		combat += val;
	}

	void archer::none()
	{
	}

	int archer::getCombat()
	{
		return combat;
	}

	unsigned long long archer::getHealth()
	{
		return m_health;
	}
}
