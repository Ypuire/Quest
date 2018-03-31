#include "Player.h"

void Player::gainExp(int exp_to_add)
{
	m_exp += exp_to_add;
	if (m_exp >= 5)
	{
		while (m_exp >= 5) //Currently, 5 exp to level up
		{
			m_exp -= 5;
			levelUp();
		}
	}
	else
	{
		return;
	}
}

//Level up, atk/def/mindmg/maxdmg/maxhp will increase
void Player::levelUp()
{
	//For now, atk and def = level
	++m_level;
	m_atk = m_level;
	m_def = m_level;

	if (m_level < 6) //For levels 1 - 5
	{
		m_max_hp += static_cast<int>((10.0 - m_level) / 100.0 * m_max_hp);
		m_min_dmg += static_cast<int>((10.0 - m_level) / 100.0 * m_min_dmg);
		m_max_dmg += static_cast<int>((10.0 - m_level) / 100.0 * m_max_dmg);
	}
	else
	{
		m_max_hp += static_cast<int>(5.0 / 100.0 * m_max_hp);
		m_min_dmg += static_cast<int>(5.0 / 100.0 * m_min_dmg);
		m_max_dmg += static_cast<int>(5.0 / 100.0 * m_max_dmg);
	}
}

//Future use scaleStatsToLevel()

//Checks if the data inside is valid or not (e.g. no negative hp values)
//Should be passed size of Game::item_data when loading game data
//Should be passed size of Game::items when loading saved game
bool Player::valid()
{
	if (m_max_hp <= 0 || m_hp > m_max_hp)
		return false;
	if (m_atk < 1)
		return false;
	if (m_def < 1)
		return false;
	if (m_min_dmg < 0 || m_max_dmg < m_min_dmg)
		return false;
	if (m_exp < 0)
		return false;
	if (m_level < 0)
		return false;
	if (m_gold < 0)
		return false;
	return true;
}

bool Player::runFrom(Mob& mob)
{
	//Run_chance has a limit of up to 5 decimal places, any more are ignored
	//Success rate will be counted as run_chance / 10'000'000
	int run_chance = static_cast<int>(mob.getRunChance() * 100'000);
	int random_number = getRandomInt(0, 10'000'000);
	if (random_number < run_chance) //If random number is within success rate, successful at running
		return true;
	//else
	return false;
}

bool Player::runFrom(Threat& threat)
{
	//Run_chance has a limit of up to 5 decimal places, any more are ignored
	//Success rate will be counted as run_chance / 10'000'000
	int run_chance = static_cast<int>(threat.getRunChance() * 100'000);
	int random_number = getRandomInt(0, 10'000'000);
	if (random_number < run_chance) //If random number is within success rate, successful at running
		return true;
	//else
	return false;
}

//void Player::scaleStatsToLevel()
//{
//	m_max_hp = 100 + (m_level - 1) * 10;
//	m_hp = m_max_hp;
//
//	m_def = m_level;
//}