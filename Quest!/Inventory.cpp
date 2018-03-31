#include "Inventory.h"

Inventory::Inventory(int size)
{
	m_item_types.resize(size);
	for (int i{ 0 }; i < size; ++i)
	{
		m_item_types[i] = ItemType::PLACEHOLDER;
	}
	m_item_ids.resize(size);
}

void Inventory::setItem(int inventory_slot_index, ItemType new_item_type, int new_item_id)
{
	m_item_types[inventory_slot_index] = new_item_type;
	m_item_ids[inventory_slot_index] = new_item_id;
}

void Inventory::clearItem(int inventory_slot_index)
{
	m_item_types[inventory_slot_index] = ItemType::NOTHING;
	m_item_ids[inventory_slot_index] = -1;
}

bool Inventory::valid(int item_size) const
{
	if (m_item_types.size() == 0) //Empty inventory
		return true;
	if (m_item_types.size() > 9)
		return false;
	if (m_item_types[0] != ItemType::PLACEHOLDER) //If not placeholder, we are loading a saved game, check validity of item type
	{
		for (int i{ 0 }; i < static_cast<int>(m_item_types.size()); ++i)
		{
			//NOTHING is defined to be the first possible item type with an int value of -1, while magical potion should be the last possible type
			if (static_cast<int>(m_item_types[i]) < static_cast<int>(ItemType::NOTHING) ||
				static_cast<int>(m_item_types[i]) >= static_cast<int>(ItemType::MAGICALPOTION))
			{
				return false;
			}
			//ID is defined to be -1 if there is nothing in the inventory
			if (m_item_types[i] == ItemType::NOTHING && m_item_ids[i] != -1)
			{
				return false;
			}
			//If there is an item, check if its item ID is correct (0 to Game::items.size() - 1)
			if (m_item_types[i] != ItemType::NOTHING && (m_item_ids[i] < 0 || m_item_ids[i] > (item_size - 1)))
			{
				return false;
			}
		}
		return true;
	}

	for (int item_id : m_item_ids) //Since item type is placeholder, we are only loading game data
	{
		//ID is valid if either nothing(-1) or within the indexes of item_data (0 to item_data.size() - 1)
		if (item_id < -1 || item_id >= item_size)
		{
			return false;
		}
	}
	return true;
}

bool Inventory::isFull() const
{
	for (int i : m_item_ids)
	{
		if (i == -1) //An inventory slot is empty
		{
			return false;
		}
	}
	return true; //All slots are not empty and thus full
}

int Inventory::getEmptyIndex() const //Returns the first available inventory slot (-1 if no free inventory slot)
{
	for (size_t i{ 0 }; i < m_item_ids.size(); ++i)
	{
		if (m_item_ids[i] == -1)
		{
			return i;
		}
	}
	return -1;
}