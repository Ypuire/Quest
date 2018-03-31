#pragma once
#include <vector>
#include "ItemsAndEntities.h"
#include "MiscFunctions.h"

class MapTile final
{
private:
	bool m_explored;
	bool m_visible;

	EntityType m_entity_type; //
	int m_entity_id;
	ItemType m_item_type;
	int m_item_id;

	char m_tile_character; //Character to display on screen

public:

	//Constructor with default initialization intended
	MapTile(bool explored = false, bool visible = false, EntityType entity_type = EntityType::NOTHING, int entity_id = -1,
		ItemType item_type = ItemType::NOTHING, int item_id = -1)
		: m_explored{ explored }, m_visible{ visible }, m_entity_type{ entity_type }, m_entity_id{ entity_id }, m_item_type{ item_type }, m_item_id{ item_id }
	{}

	bool getIsExplored() const { return m_explored; }
	bool getIsVisible() const { return m_visible; }
	EntityType getEntityType() const { return m_entity_type; }
	int getEntityID() const { return m_entity_id; }
	ItemType getItemType() const { return m_item_type; }
	int getItemID() const { return m_item_id; }
	char getCharacter() const { return m_tile_character; }
	void setExplored() { m_explored = true; }
	void setVisible(bool now_visible) { m_visible = now_visible; }
	void setEntity(EntityType new_entity_type, int new_entity_id) { m_entity_type = new_entity_type; m_entity_id = new_entity_id; }
	void setItem(ItemType new_item_type, int new_item_id) { m_item_type = new_item_type; m_item_id = new_item_id; }
	void setCharacter(char new_tile_character) { m_tile_character = new_tile_character; }
};

class Map final
{
private:
	int m_xsize;
	int m_ysize;
	std::vector<std::vector<MapTile> > maptile;

	void initializeMap();

public:

	//Clears all maptiles held by this map object, setting size to 0 and deallocating all maptiles
	void clear();

	//Note: Does not clear any pre-existing tiles. Does nothing if there are existing tiles uncleared
	void initializeNewMap(int xsize, int ysize);


	int getXSize() const { return m_xsize; }
	int getYSize() const { return m_ysize; }

	void getRandomTileWithoutItemCoords(int& xcoord, int& ycoord);

	void getRandomTileWithoutEntityCoords(int& xcoord, int& ycoord);

	MapTile& operator()(const int x, const int y) { return maptile[x][y]; }
	const MapTile& operator()(const int x, const int y) const { return maptile[x][y]; }
};