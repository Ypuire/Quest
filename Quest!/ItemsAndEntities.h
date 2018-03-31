#pragma once

#include <string>
#include "MiscFunctions.h"
#include "Events.h"

enum class ItemType
{
	PLACEHOLDER = -2,
	NOTHING = -1,
	BASE, //Has no use other than to sell it
	HEALING,
	WEAPON,
	MAGICALPOTION
};

enum class EntityType
{
	NOTHING = -1,
	PLAYER,
	MOB,
	THREAT,
	MERCHANT,
};

class BaseItem
{
protected:
	ItemType m_item_type;

	std::string m_name;

	int m_value;
	int m_xcoord; //So far items havent had to have their coordinates used though, since all maptiles hold itemtype and their ids
	int m_ycoord;

public:
	BaseItem(ItemType item_type, const std::string& name, int value)
		: m_item_type{ item_type }, m_name{ name }, m_value{ value }
	{}

	bool valid();

	ItemType getItemType() const { return m_item_type; }
	const std::string& getName() const { return m_name; }
	int getXCoord() const { return m_xcoord; }
	int getYCoord() const { return m_ycoord; }
	int getCoords(int& xcoord, int& ycoord) const { xcoord = m_xcoord; ycoord = m_ycoord; }
	int getValue() const { return m_value; }

	void setXCoord(int new_xcoord) { m_xcoord = new_xcoord; }
	void setYCoord(int new_ycoord) { m_ycoord = new_ycoord; }
	void setCoords(int new_xcoord, int new_ycoord) { m_xcoord = new_xcoord; m_ycoord = new_ycoord; }
};

class Item final : public BaseItem //Usable item
{
private:

	ItemType m_item_type;

	std::string m_name;

	int m_min_hp_change;
	int m_max_hp_change;
	int m_uses;

	double m_success_rate;

public:

	Item(ItemType item_type, const std::string& name, int min_hp_change, int max_hp_change, int uses, double success_rate, int value)
		: BaseItem(item_type, name, value), m_min_hp_change{ min_hp_change }, m_max_hp_change{ max_hp_change }, m_uses{ uses },
		m_success_rate{ success_rate }
	{}

	bool valid();

	int getMinHpChange() const { return m_min_hp_change; }
	int getMaxHpChange() const { return m_max_hp_change; }
	int getUses() const { return m_uses; }
	double getSuccessRate() const { return m_success_rate; }

	void decrementUses() { --m_uses; }
};

class Entity
{
protected:
	EntityType m_entity_type;
	std::string m_name;

	int m_max_hp;
	int m_hp;
	int m_atk;
	int m_def;

	int m_min_dmg;
	int m_max_dmg;

	int m_exp;
	int m_level;

	int m_xcoord;
	int m_ycoord;

	bool m_is_dead;

	int m_gold;

	int m_object_typeid;

public:

	Entity(EntityType entity_type, const std::string& name, int max_hp, int hp,int atk, int def, int min_dmg, int max_dmg, int exp, int level,
		bool is_dead, int gold, int object_typeid)
		: m_entity_type{ entity_type }, m_name{ name }, m_max_hp{ max_hp }, m_hp{ hp }, m_atk{ atk }, m_def{ def }, m_min_dmg{ min_dmg }, m_max_dmg{ max_dmg },
		m_exp{ exp }, m_level{ level }, m_is_dead{ is_dead }, m_gold{ gold }, m_object_typeid{ object_typeid }
	{}

	//void attack(Entity& other_entity);
	EntityType getEntityType() const { return m_entity_type; }
	const std::string& getName() const { return m_name; }
	int getHealth() const { return m_hp; }
	int getMaxHealth() const { return m_max_hp; }
	int getAtk() const { return m_atk; }
	int getDef() const { return m_def; }
	int getMinDmg() const { return m_min_dmg; }
	int getMaxDmg() const { return m_max_dmg; }
	int getExp() const { return m_exp; }
	int getLevel() const { return m_level; }
	bool isDead() const { return m_is_dead; }
	int getGold() const { return m_gold; }
	int getXCoord() const { return m_xcoord; }
	int getYCoord() const { return m_ycoord; }
	void getCoords(int& xcoord, int& ycoord) const { xcoord = m_xcoord; ycoord = m_ycoord; }

	void setXCoord(int new_xcoord) { m_xcoord = new_xcoord; }
	void setYCoord(int new_ycoord) { m_ycoord = new_ycoord; }
	void setCoords(int new_xcoord, int new_ycoord) { m_xcoord = new_xcoord; m_ycoord = new_ycoord; }
	void heal(int heal_amount);
	void takeDamage(int damage_amount);
	void gainGold(int gold_to_add) { m_gold += gold_to_add; }
	void loseGold(int gold_to_subtract) { m_gold -= gold_to_subtract; if (m_gold < 0) m_gold = 0; }
};

class Mob final : public Entity
{
private:
	double m_run_chance;
	//Level not implemented
public:
	Mob(const std::string& name, int max_hp, int hp, int atk, int def, int min_dmg, int max_dmg, int exp, int level,
		double run_chance, int gold, int object_typeid, bool is_dead = false)
		:Entity(EntityType::MOB, name, max_hp, hp, atk, def, min_dmg, max_dmg, exp, level, is_dead, gold, object_typeid), m_run_chance{ run_chance }
	{}

	bool valid();

	double getRunChance() const { return m_run_chance; }
};

class Threat final
{
private:
	std::string m_name;

	int m_min_dmg;
	int m_max_dmg;

	double m_run_chance;

public:
	Threat(const std::string& name, int min_dmg, int max_dmg, double run_chance)
		: m_name{ name }, m_min_dmg{ min_dmg }, m_max_dmg{ max_dmg }, m_run_chance{ run_chance }
	{}

	bool valid();

	const std::string& getName() const { return m_name; }
	int getMinDmg() const { return m_min_dmg; }
	int getMaxDmg() const { return m_max_dmg; }
	double getRunChance() const { return m_run_chance; }
};

class Player final : public Entity
{
private:
	ItemType m_inventory_item_type[4];
	int m_inventory_id[4];
	//Add equipped slots in the future

	Action m_action;

public:
	Player(const std::string& name, int max_hp, int hp, int atk, int def, int exp, int level, 
		int inventory1_id, int inventory2_id, int inventory3_id, int inventory4_id, int gold, int object_typeid = 0, bool is_dead = false,
		ItemType inventory1_item_type = ItemType::PLACEHOLDER, ItemType inventory2_item_type = ItemType::PLACEHOLDER,
		ItemType inventory3_item_type = ItemType::PLACEHOLDER, ItemType inventory4_item_type = ItemType::PLACEHOLDER)
		: Entity(EntityType::PLAYER, name, max_hp, hp, atk, def, 0, 0, exp, level, is_dead, gold, object_typeid), //Initially 0 min dmg and 0 max dmg, only one type of player
		m_inventory_item_type{ inventory1_item_type ,inventory2_item_type ,inventory3_item_type ,inventory4_item_type },
		m_inventory_id{ inventory1_id ,inventory2_id, inventory3_id, inventory4_id }
	{
		//Scale level
	}

	bool valid(int item_size);

	bool isInventoryFull();
	int getInventorySlotItemID(int inventory_slot_number) const { return m_inventory_id[inventory_slot_number - 1]; } //Insert inventory slot number, not index
	ItemType getInventorySlotItemType(int inventory_slot_number) const { return m_inventory_item_type[inventory_slot_number - 1]; }
	int getEmptyInventorySlot();
	Action getAction() const { return m_action; }

	void setAction(Action action) { m_action = action; }
	void setInventorySlotItem(int inventory_slot_number, ItemType new_item_type, int new_item_id);
	bool runFrom(Mob& mob);
	bool runFrom(Threat& threat);
	void gainExp(int exp_to_add);
	void levelUp();

};
