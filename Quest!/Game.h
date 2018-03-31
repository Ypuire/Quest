#pragma once

#include <vector>
#include <memory>
#include "MiscFunctions.h"
#include "DataLoader.h"
#include "EventMsgHandler.h"
#include "ItemsAndEntities.h"
#include "NPCs.h"
#include "Events.h"
#include "Map.h"

class Game final
{
private:

	Map map;
	std::vector<int> base_item_number_to_place, item_number_to_place, mob_number_to_place, threat_number_to_place; //Stores data for number to place for each item/mob/threat
	std::vector<Player> player_data;	//Stores data for player base stats
	std::vector<BaseItem> base_item_data;//For BaseItem assets loaded at runtime to be copy constructed from
	std::vector<Item> item_data;		//For Item assets
	std::vector<Mob> mob_data;			//For Mob assets
	std::vector<Threat> threat_data;	//For Threat assets
	std::vector<std::unique_ptr<NPC>> npc_data; //For NPC assets Note: This was experimentally attempting polymorphism, but the reasoning for doing so has been
	//determined to be ill-formed and not best done using polymorphism. Up to be changed to a normal object-based vector in the future to be more like Item and BaseItem
	//instead of storying all derived NPC classes in one vector of unique pointers

	std::vector<Player> player;		//Stores the actual player objects in the game
	std::vector<BaseItem> base_items;//For actual baseitem objects
	std::vector<Item> items;		//For actual item objects
	std::vector<Mob> mobs;			//For actual mob objects
	std::vector<Threat> threats;	//For actual threat objects
	std::vector<std::unique_ptr<NPC>> npcs;

	//Not actually done anything with for the moment
	std::vector<int> used_items_id;	//Keeps track of the IDs of items that have been completely used up
	std::vector<int> dead_mobs_id;	//Keeps track of the IDs of dead mobs

	//Default options variables (Not to be changed while in game, like the _data vectors
	int map_xsize, map_ysize;
	int player_start_xcoord, player_start_ycoord;
	int magical_potion_xcoord, magical_potion_ycoord;

	char user_input;
	bool need_update_map;
	bool first_time_in_encounter_mob;
	GameState game_state;

	double time_left, max_time, current_time;

	EventMsgHandler event_message_handler; //Holds messages from previous events (Should be cleared after being printed)

	void loadOptions();					//Loads default options. Throws if error loading/validating default options values
	void loadData();					//Loads items/entities data. Throws if error loading/validating items and entites
	int getName(DataLoader& data_loader, std::string& name);
	void checkDataLoaderStatus(const DataLoader& data_loader, const std::string& filename = "Unknown file") const;	//Throws if the filereader failed or is at eof
	void loadNPCs();
	void placeItemsAndEntities();		//Throws if not enough tiles to place items/entities

	//Map "refreshing" functions
	void updateMapTileCharacter(int x, int y);	//Updates the character to be printed on a tile
	void updateEntireMapTileCharacter();		//Updates the character to be printed on every tile

	//Printing functions
	void printMap() const;								//Shows the game board
	void printTimeLeft() const;							//Prints the time left to complete the game
	void printPlayerDetails() const;					//Prints the hero's hp,	exp, level, equipped weapon, inventory slots
	void printInventory() const;
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
	bool playerMoveMenu();						//Note: Changes the action of the player if valid (Specific case of isPlayerActionValid()
	bool useItemMenu();
	bool swapItemMenu();							//Note: Changes the action of the player if valid (Specific case of isPlayerActionValid()
	void inspectMenu();								//Note: Not an actual player action, only tells the player details about items
	void printItemDetails(Item& item) const;
	void printItemDetails(BaseItem& base_item) const;
	void talkToNPCMenu();
	void npcBuyMenu(Merchant* merchant);
	void npcSellMenu(Merchant* merchant);

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

	void initializeDefaultValues()
	{
		Game::need_update_map = false;
		Game::game_state = GameState::ONGOING;
		Game::time_left = max_time;
		Game::current_time = 0;
	}
	void start(); //Start Game
	void cleanUpGame();
public:
	Game() 
	{
		Game::loadOptions();			//Load Options.dat for default options
		Game::loadData();				//Load Data.dat for entities and items
		Game::loadNPCs();				//Load NPCs.dat for NPCs
	}

	void startNewGame()
	{
		Game::map.initializeNewMap(Game::map_xsize, Game::map_ysize);
		Game::placeItemsAndEntities();
		Game::updateEntireMapTileCharacter();
		Game::initializeDefaultValues();
		Game::start();
		Game::cleanUpGame(); //Calls all required clear functions
	}

	//void startSavedGame()

};