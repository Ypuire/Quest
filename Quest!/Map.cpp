#include "Map.h"

void Map::initializeMap()
{
	maptile.resize(m_xsize);
	for (int i = 0; i < m_xsize; ++i)
	{
		maptile.at(i).resize(m_ysize);
	}
}

void Map::clear()
{
	m_xsize = 0;
	m_ysize = 0;
	maptile.clear();
}

void Map::initializeNewMap(int xsize, int ysize)
{
	if (maptile.size() == 0) //Initialize only if maptiles non-existent
	{
		m_xsize = xsize;
		m_ysize = ysize;
		initializeMap();	//Default initializes all MapTiles
	}
}

void Map::getRandomTileWithoutItemCoords(int& xcoord, int& ycoord)
{
	do {
		xcoord = getRandomInt(0, m_xsize - 1);
		ycoord = getRandomInt(0, m_ysize - 1);
	} while ((maptile[xcoord][ycoord].getItemType() != ItemType::NOTHING));
}

void Map::getRandomTileWithoutEntityCoords(int& xcoord, int& ycoord)
{
	do {
		xcoord = getRandomInt(0, m_xsize - 1);
		ycoord = getRandomInt(0, m_ysize - 1);
	} while ((maptile[xcoord][ycoord].getEntityType() != EntityType::NOTHING));
}