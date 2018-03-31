#pragma once

#include <string>
#include "MiscFunctions.h"
#include "Events.h"

enum class ItemType
{
	PLACEHOLDER = -2,
	NOTHING = -1,
	HEALING,
	WEAPON,
	MAGICALPOTION
};

enum class EntityType	//Currently not used
{
	NOTHING = -1,
	PLAYER,
	MOB,
	THREAT
};

//enum class ObjectType Replace with itemtype
//{
//	NOTHING = -1,
//	ITEM,
//};

class Item final
{

	std::string m_name;

	ItemType m_item_type;

	int m_min_hp_change;
	int m_max_hp_change;
	int m_uses;

	double m_success_rate; //NOT USED YET
	int m_object_typeid;
	int m_object_id;

	int m_xcoord;
	int m_ycoord;

public:

	Item(const std::string& name, ItemType item_type, int min_hp_change, int max_hp_change, int uses, double success_rate, int object_typeid, int object_id)
		: m_name{ name }, m_item_type{ item_type }, m_min_hp_change{ min_hp_change }, m_max_hp_change{ max_hp_change }, m_uses{ uses },
		m_success_rate{ success_rate }, m_object_typeid{ object_typeid }, m_object_id{ object_id }
	{}

	bool valid();

	ItemType getItemType() const { return m_item_type; }
	const std::string& getName() const { return m_name; }
	int getMinHpChange() const { return m_min_hp_change; }
	int getMaxHpChange() const { return m_max_hp_change; }
	int getUses() const { return m_uses; }
	double getSuccessRate() const { return m_success_rate; }
	int getXCoord() const { return m_xcoord; }
	int getYCoord() const { return m_ycoord; }
	int getCoords(int& xcoord, int& ycoord) const { xcoord = m_xcoord; ycoord = m_ycoord; }
	void setXCoord(int new_xcoord) { m_xcoord = new_xcoord; }
	void setYCoord(int new_ycoord) { m_ycoord = new_ycoord; }
	void setCoords(int new_xcoord, int new_ycoord) { m_xcoord = new_xcoord; m_ycoord = new_ycoord; }
	void decrementUses() { --m_uses; }
};

class Entity
{
protected:
	EntityType m_entity_type;
	std::string m_name;

	int m_max_hp;
	int m_hp;
	int m_def;

	int m_min_dmg;
	int m_max_dmg;

	int m_exp;
	int m_level;

	int m_xcoord;
	int m_ycoord;

	bool m_is_dead;

	int m_object_id;

public:

	Entity(EntityType entity_type, const std::string& name, int max_hp, int hp, int def, int min_dmg, int max_dmg, int exp, int level, bool is_dead, int object_id)
		: m_entity_type{ entity_type }, m_name{ name }, m_max_hp{ max_hp }, m_hp{ hp }, m_def{ def }, m_min_dmg{ min_dmg }, m_max_dmg{ max_dmg },
		m_exp{ exp }, m_level{ level }, m_is_dead{ is_dead }, m_object_id{ object_id }
	{}

	//void attack(Entity& other_entity);
	//void heal(int gain_in_hp) { m_hp += gain_in_hp; }
	//void takeDamage(int loss_in_hp) { m_hp -= loss_in_hp; }
	EntityType getEntityType() const { return m_entity_type; }
	const std::string& getName() const { return m_name; }
	int getHealth() const { return m_hp; }
	int getDef() const { return m_def; }
	int getMinDmg() const { return m_min_dmg; }
	int getMaxDmg() const { return m_max_dmg; }
	//exp, level
	bool isDead() const { return m_is_dead; }
	int getXCoord() const { return m_xcoord; }
	int getYCoord() const { return m_ycoord; }
	void getCoords(int& xcoord, int& ycoord) const { xcoord = m_xcoord; ycoord = m_ycoord; }
	void setXCoord(int new_xcoord) { m_xcoord = new_xcoord; }
	void setYCoord(int new_ycoord) { m_ycoord = new_ycoord; }
	void setCoords(int new_xcoord, int new_ycoord) { m_xcoord = new_xcoord; m_ycoord = new_ycoord; }
	void heal(int heal_amount);
	void takeDamage(int damage_amount);
};

class Mob final : public Entity
{
	int m_object_typeid;

	double m_run_chance;
	//Level not implemented
public:
	Mob(const std::string& name, int max_hp, int hp, int def, int min_dmg, int max_dmg, int exp, int level, double run_chance, int object_typeid, int object_id, bool is_dead = false)
		:Entity(EntityType::MOB, name, max_hp, hp, def, min_dmg, max_dmg, exp, level, is_dead, object_id), m_run_chance{ run_chance }, m_object_typeid{ object_typeid }
	{}

	bool valid();
	double getRunChance() const { return m_run_chance; }
};

class Threat
{
private:
	std::string m_name;

	int m_min_dmg;
	int m_max_dmg;

	double m_run_chance;

	int m_object_id;
	int m_object_typeid;
public:
	Threat(const std::string& name, int min_dmg, int max_dmg, double run_chance, int object_typeid, int object_id)
		: m_name{ name }, m_min_dmg{ min_dmg }, m_max_dmg{ max_dmg }, m_run_chance{ run_chance }, m_object_typeid{ object_typeid }, m_object_id{ object_id }
	{}

	bool valid();

	const std::string& getName() const { return m_name; }
	int getMinDmg() const { return m_min_dmg; }
	int getMaxDmg() const { return m_max_dmg; }
	double getRunChance() const { return m_run_chance; }
	//int getObjectTypeID() const { return m_object_typeid; }
};

class Player final : public Entity
{
	ItemType m_inventory_item_type[4];
	int m_inventory_id[4];
	//int m_equipped_slot;

	//int m_encounterID; //NOT USED YET

	//void scaleStatsToLevel();

	Action m_action;

public:
	Player(const std::string& name, int max_hp, int hp, int def, int exp, int level, 
		int inventory1_id, int inventory2_id, int inventory3_id, int inventory4_id,
		ItemType inventory1_item_type = ItemType::PLACEHOLDER, ItemType inventory2_item_type = ItemType::PLACEHOLDER,
		ItemType inventory3_item_type = ItemType::PLACEHOLDER, ItemType inventory4_item_type = ItemType::PLACEHOLDER)
		: Entity(EntityType::PLAYER, name, max_hp, hp, def, 0, 0, exp, level, 0, false), //Initially 0 min dmg and 0 max dmg, identification ID for player is 0, is not dead
		m_inventory_item_type{ inventory1_item_type ,inventory2_item_type ,inventory3_item_type ,inventory4_item_type },
		m_inventory_id{ inventory1_id ,inventory2_id, inventory3_id, inventory4_id }
	{
		//if (m_level != 1)
		//{
		//	scaleStatsToLevel();//scale hp, max hp, def
		//}
		//equipped slot
	}

	bool valid(int item_size);

	//int getEncounterID() const { return m_encounterID; }
	int getInventorySlotItemID(int inventory_slot_number) const { return m_inventory_id[inventory_slot_number - 1]; } //Insert inventory slot number, not index
	ItemType getInventorySlotItemType(int inventory_slot_number) const { return m_inventory_item_type[inventory_slot_number - 1]; }
	Action getAction() const { return m_action; }
	void setAction(Action action) { m_action = action; }
	void setInventorySlotItem(int inventory_slot_number, int new_item_id, ItemType new_item_type);
	bool runFrom(Mob& mob);
	bool runFrom(Threat& threat);

};
