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
	bool valid(int item_size);

	bool canBeBoughtFrom() const { return m_can_be_bought_from; }
	bool CanBeSoldTo() const { return m_can_be_sold_to; }
	int getItemBuyPrice(int item_value) const { return static_cast<int>(item_value * m_buy_price_percent/100.0); }
	int getItemSellPrice(int item_value) const { return static_cast<int>(item_value * m_sell_price_percent / 100.0); }

	//Inventory management
	size_t getInventorySize() { return m_inventory_item_ids.size(); }
	int getInventorySlotItemID(int inventory_slot) const { return m_inventory_item_ids[inventory_slot - 1]; }
	ItemType getInventorySlotItemType(int inventory_slot) const { return m_inventory_item_type[inventory_slot - 1]; }
	const std::vector<int>& getInventoryItemIDs() const { return m_inventory_item_ids; } //Not used yet
	void addNewItemSlot(ItemType item_type, int item_id);
	void clearItemSlot(int inventory_slot); //Note: Does not decrease slot count
	void setItemSlot(int inventory_slot, ItemType new_item_type, int new_item_id);
};