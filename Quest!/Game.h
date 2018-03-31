#pragma once

#include <vector>
#include "MiscFunctions.h"
#include "DataLoader.h"
#include "EventMsgHandler.h"
#include "ItemsAndEntities.h"
#include "Events.h"
#include "Map.h"

class Game final
{
private:

	Map map;
	std::vector<int> item_number_to_place, mob_number_to_place, threat_number_to_place; //Stores data for number to place for each item/mob/threat
	std::vector<Player> player_data;					//Stores data for player base stats
	std::vector<Item> item_data;		//Stores data for all item assets
	std::vector<Mob> mob_data;			//Stores data for all entity assets
	std::vector<Threat> threat_data;	//Stores data for all threat assets

	std::vector<Player> player;		//Stores the actual player objects in the game
	std::vector<Item> items;		//Stores the actual item objects in the game
	std::vector<Mob> mobs;			//Stores the actual mob objects in the game
	std::vector<Threat> threats;	//Stores the actual threat objects in the game

	std::vector<int> used_items_id;	//Keeps track of the IDs of items that have been completely used up
	std::vector<int> dead_mobs_id;	//Keeps track of the IDs of dead mobs

	char user_input;
	bool need_update_map;
	bool first_time_in_encounter_mob;
	GameState game_state;
	//bool exit{ false };

	double time_left, max_time, current_time;

	EventMsgHandler event_message_handler; //Holds messages from previous events (Should be cleared after being printed)

	void loadItemsAndEntities();		//Throws if error loading/validating items and entites
	void getName(DataLoader& data_loader, std::string& name);
	void checkDataLoaderStatus(const DataLoader& data_loader) const;	//Throws if the filereader failed or is at eof
	void placeItemsAndEntities();		//Throws if not enough tiles to place items/entities

	//Map "refreshing" functions
	void updateMapTileCharacter(int x, int y);	//Updates the character to be printed on a tile
	void updateEntireMapTileCharacter();		//Updates the character to be printed on every tile

	//Printing functions
	void printMap() const;								//Shows the game board
	void printTimeLeft() const;							//Prints the time left to complete the game
	void printPlayerDetails() const;					//Prints the hero's hp,	exp, level, equipped weapon, inventory slots
	void printPlayerPosition() const;
	void printVictoryMessage() const;					//Congratulates player on winning
	void printGameOverMessage() const;					//Game over message
	void printObjectsOnPlayerTileDetails() const;			//Prints what(Item/entity) is on the same tile as the player
	void printAvailablePlayerActions() const;			//Print available options the player can take
	//void printNoSavePresent(char save_number) const;

	void evaluatePlayerAction();
	void evaluatePossibleEncounter();
	void useHealingItemSlot(int inventory_slot_number);
	void useWeaponItemSlot(int inventory_slot_number);
	bool isPlayerActionValid();							//Note: Changes the action of the player if valid
	bool isSwapItemCaseValid();							//Note: Changes the action of the player if valid (Specific case of isPlayerActionValid()

	void advanceTime(double time_to_pass);
	bool noTimeLeft() const;
	bool playerDied() const;
	bool playerHasMagicalPotion() const;

	void logUsedItem(int item_id) { used_items_id.push_back(item_id); }		//Note: This logs to the game for it to note which items are used, not for player
	void logDeadMob(int entity_id) { dead_mobs_id.push_back(entity_id); }	//Note: This logs to the game for it to note which mobs are dead, not for player

	void playerMove(Player& player); //Not only moves the player but also sets visibility and explored elements of maptiles
	void playerMoveRandomDirection(Player& player);
	void setVisibilityAround(const Entity& entity, bool new_visibility);
	void checkSurroundings(const Entity& entity);

	void evaluateEvents();

	bool canMoveUp(const Entity& entity) const;
	bool canMoveLeft(const Entity& entity) const;
	bool canMoveRight(const Entity& entity) const;
	bool canMoveDown(const Entity& entity) const;

	void start(); //Start Game
	void cleanUpGame();
public:
	Game(int xsize = 10, int ysize = 10) 
	{
		map.initializeNewMap(xsize, ysize);
		loadItemsAndEntities();				//Load Data.dat for entities and items
	}

	void startNewGame()
	{
		if (!Game::map.initialized)
		{
			Game::map.initializeNewMap(10, 10);
		}
		Game::placeItemsAndEntities();
		Game::updateEntireMapTileCharacter();
		//initializeDefaultValues();
		Game::need_update_map = false;
		Game::game_state = GameState::ONGOING;
		Game::time_left = 100;
		Game::max_time = 100;
		Game::current_time = 0;
		Game::start();
		Game::cleanUpGame();
	}

	//void startSavedGame()

};