#pragma once
#include <vector>
#include "ItemsAndEntities.h"

class Inventory final
{
private:
	std::vector<ItemType> m_item_types;
	std::vector<int> m_item_ids;


public:
	Inventory(int size);

	size_t size() const { return m_item_types.size(); }
	ItemType getItemType(int inventory_slot_index) const { return m_item_types[inventory_slot_index]; }
	int getItemID(int inventory_slot_index) const { return m_item_ids[inventory_slot_index]; }

	void setItemID(int inventory_slot_index, int new_item_id) { m_item_ids[inventory_slot_index] = new_item_id; }
	void setItem(int inventory_slot_index, ItemType new_item_type, int new_item_id);
	void clearItem(int inventory_slot_index); //Does not erase the item slot

	bool valid(int item_size) const; //For use during loading inventory IDs
	bool isFull() const;
	int getEmptyIndex() const; //Returns the first available inventory slot (-1 if no free inventory slot)
};
