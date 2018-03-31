#include "ItemsAndEntities.h"

bool Item::valid() //Checks if the data inside is valid or not (e.g. no negative hp values)
{
	if (m_min_hp_change < 0 || m_max_hp_change < m_min_hp_change)
		return false;
	if (m_uses < 0)
		return false;
	if (m_success_rate < 0.0 || m_success_rate > 100.0)
		return false;
	return true;
}

//May use this instead in the future, when equipped slots become a thing
//void Entity::attack(Entity& other_entity)
//{
//	int change_in_hp = getRandomNumber(m_min_dmg, m_max_dmg);
//	other_entity.takeDamage(change_in_hp);
//}

void Entity::heal(int heal_amount)
{
	m_hp += heal_amount;
	if (m_hp > m_max_hp)
		m_hp = m_max_hp;
}

void Entity::takeDamage(int damage_amount)
{
	m_hp -= damage_amount;
	if (m_hp <= 0)
	{
		m_hp = 0;
		m_is_dead = true;
	}
}

//Checks if the data inside is valid or not (e.g. no negative hp values)
bool Player::valid(int item_data_size)
{
	if (m_max_hp <= 0 || m_hp > m_max_hp)
		return false;
	if (m_def < 0)
		return false;
	if (m_min_dmg < 0 || m_max_dmg < m_min_dmg)
		return false;
	if (m_exp < 0)
		return false;
	if (m_level < 0)
		return false;
	for (int i{ 0 }; i < 4; ++i)
	{
		//NOTHING is defined to be the first possible item type with an int value of -1, while magical potion should be the last possible type
		if (static_cast<int>(m_inventory_item_type[i]) < static_cast<int>(ItemType::NOTHING) || 
			static_cast<int>(m_inventory_item_type[i]) >= static_cast<int>(ItemType::MAGICALPOTION))
			return false;
	}
	int max_item_type = item_data_size - 1;
	for (int i{ 0 }; i < 4; ++i)
	{
		//ID is defined to be -1 if there is nothing in the inventory
		if (m_inventory_item_type[i] == ItemType::NOTHING && m_inventory_id[i] != -1)
			return false;	
		//If there is an item
		if (m_inventory_item_type[i] != ItemType::NOTHING && (m_inventory_id[i] < 0 || m_inventory_id[i] > max_item_type))
			return false;
	}
	return true;
}

void Player::setInventorySlotItem(int inventory_slot_number, int new_item_id, ItemType new_item_type) 
{
	m_inventory_id[inventory_slot_number - 1] = new_item_id;
	m_inventory_item_type[inventory_slot_number - 1] = new_item_type;
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

//Checks if the data inside is valid or not (e.g. no negative hp values)
bool Mob::valid()
{
	if (m_max_hp <= 0 || m_hp > m_max_hp)
		return false;
	if (m_def < 0)
		return false;
	if (m_min_dmg < 0 || m_max_dmg < m_min_dmg)
		return false;
	if (m_exp < 0)
		return false;
	if (m_level < 0)
		return false;
	if (m_run_chance < 0.0 || m_run_chance > 100.0)
		return false;
	return true;
}

//Checks if the data inside is valid or not (e.g. no negative dmg values)
bool Threat::valid()
{
	if (m_min_dmg < 0 || m_max_dmg < m_min_dmg)
		return false;
	if (m_run_chance < 0.0 || m_run_chance > 100.0)
		return false;
	return true;
}