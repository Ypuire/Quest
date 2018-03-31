#pragma once

#include <string>
#include "MiscFunctions.h"

class Inventory;

enum class ItemType : int
{
	PLACEHOLDER = -2,
	NOTHING = -1,
	BASE, //Has no use other than to sell it
	HEALING,
	WEAPON,
	MAGICALPOTION
};

enum class EntityType : int
{
	NOTHING = -1,
	PLAYER,
	MOB,
	THREAT,
	MERCHANT,
};

enum class ObjectType : int //Collection of objects (Items/entities) with event messages tied to them
{
	BASEITEM = 0,
	ITEM,
	MOB,
	THREAT,
	MERCHANT,
	NUMBER_OF_OBJECTS
};

class BaseItem
{
protected:
	ItemType m_item_type;

	std::string m_name;

	int m_value;
	int m_xcoord; //So far items havent had to have their coordinates used though, since all maptiles hold itemtype and their ids
	int m_ycoord;

	int m_object_typeid;

public:
	BaseItem(ItemType item_type, const std::string& name, int value, int object_typeid)
		: m_item_type{ item_type }, m_name{ name }, m_value{ value }, m_object_typeid{ object_typeid }
	{}

	bool valid();

	ItemType getItemType() const { return m_item_type; }
	const std::string& getName() const { return m_name; }
	int getXCoord() const { return m_xcoord; }
	int getYCoord() const { return m_ycoord; }
	int getCoords(int& xcoord, int& ycoord) const { xcoord = m_xcoord; ycoord = m_ycoord; }
	int getValue() const { return m_value; }
	int getObjectTypeID() const { return m_object_typeid; }

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

	Item(ItemType item_type, const std::string& name, int min_hp_change, int max_hp_change, int uses, double success_rate, int value, int object_typeid)
		: BaseItem(item_type, name, value, object_typeid), m_min_hp_change{ min_hp_change }, m_max_hp_change{ max_hp_change }, m_uses{ uses }, 
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

	Entity() {}

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
	int getObjectTypeID() const { return m_object_typeid; }

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

	double m_atk_frequency;
	double m_run_chance;
	//Level not implemented
public:
	Mob(const std::string& name, int max_hp, int hp, int atk, int def, int min_dmg, int max_dmg, double atk_frequency, int exp, int level,
		double run_chance, int gold, int object_typeid, bool is_dead = false)
		:Entity(EntityType::MOB, name, max_hp, hp, atk, def, min_dmg, max_dmg, exp, level, is_dead, gold, object_typeid), 
		m_atk_frequency{ atk_frequency }, m_run_chance{ run_chance }
	{}

	bool valid();

	double getAtkFrequency() const { return m_atk_frequency; }
	double getRunChance() const { return m_run_chance; }
};

class Threat final
{
private:
	std::string m_name;

	int m_atk;
	int m_min_dmg;
	int m_max_dmg;

	double m_atk_frequency, m_run_chance;

	int m_object_typeid;

public:
	Threat(const std::string& name, int atk, int min_dmg, int max_dmg, double atk_frequency, double run_chance, int object_typeid)
		: m_name{ name }, m_atk{ atk }, m_min_dmg{ min_dmg }, m_max_dmg{ max_dmg }, m_atk_frequency{ atk_frequency }, m_run_chance{ run_chance }, m_object_typeid{ object_typeid }
	{}

	bool valid();

	const std::string& getName() const { return m_name; }
	int getAtk() const { return m_atk; }
	int getMinDmg() const { return m_min_dmg; }
	int getMaxDmg() const { return m_max_dmg; }
	double getAtkFrequency() const{ return m_atk_frequency; }
	double getRunChance() const { return m_run_chance; }
	int getObjectTypeID() const { return m_object_typeid; }
};


