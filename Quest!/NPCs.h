#pragma once
#include <string>
#include <vector>
#include "ItemsAndEntities.h"

//NOTE: Check const std::string& name or std::string&&

enum class NPCType
{
	MERCHANT = 0
};

class NPC
{
protected:

	NPCType m_npc_type;

	std::string m_name;

	int m_xcoord;
	int m_ycoord;

public:
	NPC(NPCType npc_type, const std::string& name)
		: m_npc_type{ npc_type }, m_name{ name }
	{}
	virtual ~NPC() {};

	const std::string& getName() const { return m_name; }
	NPCType getNPCType() const { return m_npc_type; }
	int getXCoord() const { return m_xcoord; }
	int getYCoord() const { return m_ycoord; }
	void setCoords(int xcoord, int ycoord) { m_xcoord = xcoord; m_ycoord = ycoord; }
};

class Merchant final : public NPC
{
private:
	bool m_can_be_bought_from;
	bool m_can_be_sold_to;

	int m_buy_price_percent;	//Buy price as percent of value
	int m_sell_price_percent;	//Sell price as percent of value

	std::vector<ItemType> m_inventory_item_type;
	std::vector<int> m_inventory_item_ids;//Size of this vector tracks how many item slots the merchant has

public:
	Merchant(const std::string& name, bool can_be_bought_from, bool can_be_sold_to, int buy_price_percent, int sell_price_percent)
		: NPC(NPCType::MERCHANT, name), m_can_be_bought_from{ can_be_bought_from }, m_can_be_sold_to{ can_be_sold_to },
		m_buy_price_percent{ buy_price_percent }, m_sell_price_percent{ sell_price_percent }
	{}

	//Should be passed size of Game::item_data when loading game data
	//Should be passed size of Game::items when loading saved game
	bool valid(int item_size)
	{
		if (m_buy_price_percent < 0)
			return false;
		if (m_sell_price_percent < 0)
			return false;
		if (m_inventory_item_ids.size() > 9) //Max 9 slots
			return false;

		if(m_inventory_item_type.size() != 0) //Then execute below
		if (m_inventory_item_type[1] != ItemType::PLACEHOLDER) //If not placeholder, we are loading a saved game, check validity of item type
		{
			for (size_t i{ 0 }; i < m_inventory_item_ids.size(); ++i)
			{
				//NOTHING is defined to be the first possible item type with an int value of -1, while magical potion should be the last possible type
				if (static_cast<int>(m_inventory_item_type[i]) < static_cast<int>(ItemType::NOTHING) ||
					static_cast<int>(m_inventory_item_type[i]) >= static_cast<int>(ItemType::MAGICALPOTION))
				{
					return false;
				}
				//ID is defined to be -1 if there is nothing in the inventory
				if (m_inventory_item_type[i] == ItemType::NOTHING && m_inventory_item_ids[i] != -1)
				{
					return false;
				}
				//If there is an item, check if its item ID is correct (0 to Game::items.size() - 1)
				if (m_inventory_item_type[i] != ItemType::NOTHING && (m_inventory_item_ids[i] < 0 || m_inventory_item_ids[i] > (item_size - 1)))
				{
					return false;
				}
			}
			return true;
		}

		for (int i : m_inventory_item_ids)
		{
			//ID is valid if either nothing(-1) or within the indexes of item_data (0 to item_data.size() - 1)
			if (i < -1 || i >= item_size)
			{
				return false;
			}
		}
		return true;
	}

	bool canBeBoughtFrom() const { return m_can_be_bought_from; }
	bool CanBeSoldTo() const { return m_can_be_sold_to; }
	int getInventorySlotItemID(int inventory_slot) const { return m_inventory_item_ids[inventory_slot - 1]; }
	ItemType getInventorySlotItemType(int inventory_slot) const { return m_inventory_item_type[inventory_slot - 1]; }
	const std::vector<int>& getInventoryItemIDs() const { return m_inventory_item_ids; } //Not used yet
	size_t getInventorySize() { return m_inventory_item_ids.size(); }
	int getItemBuyPrice(int item_value) const { return static_cast<int>(item_value * m_buy_price_percent/100.0); }
	int getItemSellPrice(int item_value) const { return static_cast<int>(item_value * m_sell_price_percent / 100.0); }
	void addNewItemSlot(ItemType item_type, int item_id)
	{ 
		m_inventory_item_type.push_back(item_type); 
		m_inventory_item_ids.push_back(item_id); 
	}
	void clearItemSlot(int inventory_slot) //Note: Does not decrease slot count
	{
		m_inventory_item_type[inventory_slot - 1] = ItemType::NOTHING; 
		m_inventory_item_ids[inventory_slot - 1] = -1; 
	} 
	void setItemSlot(int inventory_slot, ItemType new_item_type, int new_item_id) 
	{ 
		m_inventory_item_type[inventory_slot - 1] = new_item_type; 
		m_inventory_item_ids[inventory_slot - 1] = new_item_id; 
	}
};