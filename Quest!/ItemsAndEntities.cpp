#include "ItemsAndEntities.h"

bool BaseItem::valid()
{
	if (m_value < 0)
		return false;
	return true;
}

bool Item::valid() //Checks if the data inside is valid or not (e.g. no negative hp values)
{
	if (m_min_hp_change < 0 || m_max_hp_change < m_min_hp_change)
		return false;
	if (m_uses < 0)
		return false;
	if (m_success_rate < 0.0 || m_success_rate > 100.0)
		return false;
	if (m_value < 0)
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
bool Mob::valid()
{
	if (m_max_hp <= 0 || m_hp > m_max_hp)
		return false;
	if (m_def < 1)
		return false;
	if (m_atk < 1)
		return false;
	if (m_min_dmg < 0 || m_max_dmg < m_min_dmg)
		return false;
	if (m_exp < 0)
		return false;
	if (m_level < 0)
		return false;
	if (m_atk_frequency < 0)
		return false;
	if (m_run_chance < 0.0 || m_run_chance > 100.0)
		return false;
	if (m_gold < 0)
		return false;
	return true;
}

//Checks if the data inside is valid or not (e.g. no negative dmg values)
bool Threat::valid()
{
	if (m_min_dmg < 0 || m_max_dmg < m_min_dmg)
		return false;
	if (m_atk_frequency < 0)
		return false;
	if (m_run_chance < 0.0 || m_run_chance > 100.0)
		return false;
	return true;
}