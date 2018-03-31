#pragma once
#include <string>
#include <vector>
#include "ItemsAndEntities.h"
#include "Inventory.h"

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

	int m_object_typeid;

public:
	NPC(NPCType npc_type, const std::string& name, int object_typeid)
		: m_npc_type{ npc_type }, m_name{ name }, m_object_typeid{ object_typeid }
	{}

	const std::string& getName() const { return m_name; }
	NPCType getNPCType() const { return m_npc_type; }
	int getXCoord() const { return m_xcoord; }
	int getYCoord() const { return m_ycoord; }
	int getObjectTypeID() const { return m_object_typeid; }
	void setCoords(int xcoord, int ycoord) { m_xcoord = xcoord; m_ycoord = ycoord; }
};

class Merchant final : public NPC
{
private:
	bool m_can_be_bought_from;
	bool m_can_be_sold_to;

	int m_buy_price_percent;	//Buy price as percent of value
	int m_sell_price_percent;	//Sell price as percent of value

public:
	Merchant(const std::string& name, bool can_be_bought_from, bool can_be_sold_to, int buy_price_percent, int sell_price_percent, int object_typeid)
		: NPC(NPCType::MERCHANT, name, object_typeid), m_can_be_bought_from{ can_be_bought_from }, m_can_be_sold_to{ can_be_sold_to },
		m_buy_price_percent{ buy_price_percent }, m_sell_price_percent{ sell_price_percent }
	{}

	Inventory* m_inventory;

	//Should be passed size of Game::item_data when loading game data
	//Should be passed size of Game::items when loading saved game
	bool valid();

	bool canBeBoughtFrom() const { return m_can_be_bought_from; }
	bool CanBeSoldTo() const { return m_can_be_sold_to; }
	int getBuyPricePercent() const { return m_buy_price_percent; }
	int getSellPricePercent() const { return m_sell_price_percent; }
	int getItemBuyPrice(int item_value) const { return static_cast<int>(item_value * m_buy_price_percent/100.0); }
	int getItemSellPrice(int item_value) const { return static_cast<int>(item_value * m_sell_price_percent / 100.0); }

};