#include "NPCs.h"

bool Merchant::valid(int item_size)
{
	if (m_buy_price_percent < 0)
		return false;
	if (m_sell_price_percent < 0)
		return false;
	if (m_inventory_item_ids.size() > 9) //Max 9 slots
		return false;

	if (m_inventory_item_type.size() != 0) //Then execute below
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

void Merchant::addNewItemSlot(ItemType item_type, int item_id)
{
	m_inventory_item_type.push_back(item_type);
	m_inventory_item_ids.push_back(item_id);
}

void Merchant::clearItemSlot(int inventory_slot) //Note: Does not decrease slot count
{
	m_inventory_item_type[inventory_slot - 1] = ItemType::NOTHING;
	m_inventory_item_ids[inventory_slot - 1] = -1;
}

void Merchant::setItemSlot(int inventory_slot, ItemType new_item_type, int new_item_id)
{
	m_inventory_item_type[inventory_slot - 1] = new_item_type;
	m_inventory_item_ids[inventory_slot - 1] = new_item_id;
}