#pragma once
#include "Events.h"
#include "ItemsAndEntities.h"
#include "Inventory.h"

class Player final : public Entity
{
private:
	//Add equipped slots in the future

	Action m_action;

public:
	Player() {}

	Player(const std::string& name, int max_hp, int hp, int atk, int def, int exp, int level, int gold, int object_typeid = 0, bool is_dead = false)
		: Entity(EntityType::PLAYER, name, max_hp, hp, atk, def, 0, 0, exp, level, is_dead, gold, object_typeid)
		//Initially 0 min dmg and 0 max dmg, only one type of player
	{
		gainExp(0);
		m_hp = m_max_hp;
	}

	Inventory* m_inventory; //Public member variable with its own public functions and private members

	bool valid();

	Action getAction() const { return m_action; }

	void setAction(Action action) { m_action = action; }
	bool runFrom(Mob& mob);
	bool runFrom(Threat& threat);
	void gainExp(int exp_to_add);
	void levelUp();

};