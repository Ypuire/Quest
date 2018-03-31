#pragma once

#include <vector>
#include "MiscFunctions.h"
#include "DataLoader.h"
#include "EventMsgHandler.h"
#include "ItemsAndEntities.h"
#include "Player.h"
#include "Merchant.h"
#include "Events.h"
#include "Map.h"

class Game final
{
private:

	Map map;
	std::vector<int> base_item_number_to_place, item_number_to_place, mob_number_to_place, threat_number_to_place; //Stores data for number to place for each item/mob/threat
	Player player_data;	//Stores data for player base stats
	std::vector<BaseItem> base_item_data;//For BaseItem assets loaded at runtime to be copy constructed from
	std::vector<Item> item_data;		//For Item assets
	std::vector<Mob> mob_data;			//For Mob assets
	std::vector<Threat> threat_data;	//For Threat assets
	std::vector<Merchant> merchant_data;//For Merchant assets

	std::vector<Inventory> inventory_data;

	Player player;		//Stores the actual player objects in the game
	std::vector<BaseItem> base_items;//For actual baseitem objects
	std::vector<Item> items;		//For actual item objects
	std::vector<Mob> mobs;			//For actual mob objects
	std::vector<Merchant> merchants;

	std::vector<Inventory> inventories;

	EventList event_list;

	//Not actually done anything with for the moment
	std::vector<int> used_items_id;	//Keeps track of the IDs of items that have been completely used up
	std::vector<int> dead_mobs_id;	//Keeps track of the IDs of dead mobs

	//Default options variables (Not to be changed while in game, like the _data vectors
	int map_xsize, map_ysize;
	int magical_potion_xcoord, magical_potion_ycoord;

	char user_input;
	bool need_update_map;
	bool first_time_in_encounter_mob;
	GameState game_state;

	double time_left, max_time, current_time;

	EventMsgHandler event_message_handler; //Holds messages from previous events (Should be cleared after being printed)

	void loadData();					//Loads items/entities data. Throws if error loading/validating items and entites
	int getName(DataLoader& data_loader, std::string& name);
	void loadNPCs();
	void loadOptions();					//Loads default options. Throws if error loading/validating default options values
	bool coordsOutOfBounds(int xcoord, int ycoord);
	void verifyEvents() const;
	void placeItemsAndEntities();		//Throws if not enough tiles to place items/entities

	//Map "refreshing" functions
	void updateMapTileCharacter(int x, int y);	//Updates the character to be printed on a tile
	void updateEntireMapTileCharacter();		//Updates the character to be printed on every tile

	//Printing functions
	void printMap() const;								//Shows the game board
	void printTimeLeft() const;							//Prints the time left to complete the game
	void printPlayerDetails(Player& player) const;		//Prints the hero's hp,	exp, level, equipped weapon, inventory slots
	void printInventoryLeftRight(Inventory* inventory) const; //Prints the inventory slots and their items w/ no. of uses from left to right, then top to down
	void printInventoryTopDown(Inventory* inventory) const; //Prints the inventory slots and their items w/ no. of uses from top to down, then left to right
	void printPlayerPosition(Player& player) const;
	void printVictoryMessage() const;					//Congratulates player on winning
	void printGameOverMessage() const;					//Game over message
	void printObjectsOnTileDetails(int xcoord, int ycoord) const;	//Prints what(Item/entity) is on the same tile as the player
	void printAvailablePlayerActions() const;			//Print available options the player can take

	//Functions that deal with player action
	void evaluatePlayerAction();
	void evaluatePossibleEncounter();
	bool isPlayerActionValid();							//Note: Changes the action of the player if valid

	//Sub-menu functions
	bool playerMoveMenu();							//Note: Changes the action of the player if valid (Specific case of isPlayerActionValid()
	bool useItemMenu();
	bool swapItemMenu();							//Note: Changes the action of the player if valid (Specific case of isPlayerActionValid()
	void inspectMenu(const Inventory* const inventory, int xcoord, int ycoord);		//Note: Not an actual player action, only tells the player details about items
	void inspectMenu(const Inventory* const inventory); //Version of inspectMenu that has inspect item/entity on tile disabled
	void printItemDetails(BaseItem& base_item) const;
	void printItemDetails(Item& item) const;
	void printEntityDetails(Mob& mob) const;
	void printEntityDetails(Threat& threat) const;
	void printEntityDetails(Merchant& merchant) const;
	void talkToNPCMenu();
	void merchantTalkMenu(Merchant& merchant);
	void npcBuyMenu(Merchant& merchant);
	void npcSellMenu(Merchant& merchant);

	void playerMove(Player& player); //Not only moves the player but also sets visibility and explored elements of maptiles
	void playerMoveRandomDirection(Player& player);
	void setVisibilityAround(const Entity& entity, bool new_visibility);
	void useHealingItem(int inventory_index);
	void useWeaponItem(int inventory_index);
	int evaluateActualDamage(int expected_damage, int attacker_atk, int defender_def);
	void logUsedItem(int item_id) { used_items_id.push_back(item_id); }		//Note: This logs to the game for it to note which items are used, not for player
	void logDeadMob(int entity_id) { dead_mobs_id.push_back(entity_id); }	//Note: This logs to the game for it to note which mobs are dead, not for player
	void swapItems(Player& player, int inventory_index);
	void checkSurroundings(const Entity& entity);

	//Non-player event functions
	void evaluateEvents();
	void advanceTime(double time_to_pass);
	bool noTimeLeft() const;
	bool playerDied() const;
	bool playerHasMagicalPotion() const;

	//Generic Entity functions
	bool canMoveUp(const Entity& entity) const;
	bool canMoveLeft(const Entity& entity) const;
	bool canMoveRight(const Entity& entity) const;
	bool canMoveDown(const Entity& entity) const;

	void initializeDefaultValues();
	void start(); //Start Game
	void cleanUpGame();
public:
	Game() 
	{
		Game::loadData();				//Load Data.dat for entities and items
		Game::loadOptions();			//Load Options.dat for default options
		Game::loadNPCs();				//Load NPCs.dat for NPCs
		Game::event_list.loadEvents("Events.dat");
		Game::verifyEvents();
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