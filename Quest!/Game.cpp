#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <exception>
#include "Game.h"


#define EPSILON 0.001 //Given 0.001 since days are only counted up to 0.01 precision
#define FLT_EQUAL(x, y) (((x - EPSILON) < y) && ((x + EPSILON) > y))

//Default values to be initialized (Options are loaded from Options.dat)
void Game::initializeDefaultValues()
{
	Game::need_update_map = true;
	Game::game_state = GameState::ONGOING;
	Game::time_left = max_time;
	Game::current_time = 0;
	Game::player_next_turn_time = 0;
	Game::computer_next_turn_time = -1;
	std::cout << std::fixed << std::setprecision(2);
}

//Main body loop
void Game::start()
{
	while (true)
	{
		Game::evaluateEvents();		//Check victory/loss conditions
		Game::evaluateEncounters(); //Attacked by mob/etc, check player death

		if (FLT_EQUAL(Game::current_time, Game::player_next_turn_time)) //Execute all this only if its the player's turn (Print details and events and executes player action)
		{
			if (Game::need_update_map)
			{
				Game::printMap();
			}
			Game::printTimeLeft();
			Game::printPlayerDetails(Game::player);
			if (game_state == GameState::WON)
			{
				Game::event_message_handler.printMsgs();
				Game::printVictoryMessage();
				return;
			}
			if (game_state == GameState::LOST)
			{
				Game::event_message_handler.printMsgs();
				Game::printGameOverMessage();
				return;
			}

			Game::printPlayerPosition(Game::player);	//Prints the position of the player
			Game::event_message_handler.printMsgs();	//Prints all the event messages then clears the event message cache
			Game::printObjectsOnTileDetails(Game::player.getXCoord(), Game::player.getYCoord());	//Print what is on the same tile at the player (Item/entity)
			Game::printAvailablePlayerActions();	//Move, inventory, check surroundings, save, exit

			bool input_valid{ true };
			do {	//Handle user input and decisions here (Loops while invalid user input received)
				std::cin >> Game::user_input;
				if (!validInput() || !Game::isPlayerActionValid())
				{
					input_valid = false;
					continue; //Loop
				}
			} while (!input_valid);

			if (Game::user_input == 'b' || Game::user_input == 'B') //Just came back from a sub-menu without any action
			{
				continue; //Go back to the start of the main loop to reprint everything (Will not repeat any mob/threat attacks, etc, just kinda inefficient (May relook in future)
			}

			Game::evaluatePlayerAction(); //Move, attack, use inventory
			if (Game::game_state == GameState::EXITING)
			{
				return;
			}
			else if (Game::game_state == GameState::SAVING)
			{
				//Will currently never reach here
				return;
			}
		}	//End of code to execute on player's turn
		Game::advanceTime(Game::time_interval);	//Advance time to evaluate events
	}//End of main loop
}

//Clears all variables defined at the start of the game to get ready for the next time its run
void Game::cleanUpGame()
{
	Game::base_items.clear();
	Game::items.clear();
	Game::mobs.clear();
	Game::merchants.clear();
	Game::inventories.clear();
	Game::map.clear();
	Game::used_items_id.clear();
	Game::dead_mobs_id.clear();
}

//Loads the default options from Options file
//May throw std::runtime_error (Should be caught by std::exception handler)
void Game::loadOptions()
{
	DataLoader data_loader{ "Options.dat" };
	if (!data_loader.is_open())
	{
		throw std::runtime_error("Unable to load default options, please make sure Options.dat is in the same folder.");
	}

	int player_start_xcoord, player_start_ycoord;
	data_loader >> Game::map_xsize >> Game::map_ysize >> Game::max_time >> Game::time_interval >> Game::first_encounter_reaction_time
		>> Game::time_taken_to_move >> Game::time_taken_to_use_inv >> Game::time_taken_to_check_surroundings >> Game::time_taken_to_run
		>> player_start_xcoord >> player_start_ycoord >> Game::magical_potion_xcoord >> Game::magical_potion_ycoord;

	//Expected eof() only, if fail, something was read wrongly
	if (data_loader.fail())
	{
		throw std::runtime_error("Options.dat: Data loader encountered unexpected input. Data is defined wrongly/Syntax is wrong.");
	}

	//Check values validity here
	if (Game::map_xsize <= 0)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. X size of the map cannot be negative or zero");
	}
	if (Game::map_ysize < 0)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. Y size of the map cannot be negative or zero");
	}
	if (Game::map_xsize > 10000)
	{
		throw std::runtime_error("Options.dat: You probably didn't want to define a map of horizontal size greater than 10000");
	}
	if (Game::map_ysize > 10000)
	{
		throw std::runtime_error("Options.dat: You probably didn't want to define a map of vertical size greater than 10000");
	}
	if (Game::max_time <= 0)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. Max time must be at least 1 day long");
	}
	if (Game::time_interval > 0.25 || Game::time_interval < 0.01)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. Time interval must not be greater than 0.25 or smaller than 0.01");
	}
	if (Game::first_encounter_reaction_time < 0)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. First encounter reaction time cannot be negative");
	}
	if (Game::time_taken_to_move < 0 || Game::time_taken_to_use_inv < 0 || Game::time_taken_to_check_surroundings < 0 || Game::time_taken_to_run < 0)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. Time taken for an action cannot be negative");
	}
	if (player_start_xcoord == -1 && player_start_ycoord == -1) {} //Ignore this case, this tells the item/entity generator to place the player randomly
	else
	{
		if (player_start_xcoord < 0)
		{
			throw std::runtime_error("Options.dat: Data is defined wrongly. X coordinate of player cannot be negative");
		}
		if (player_start_ycoord < 0)
		{
			throw std::runtime_error("Options.dat: Data is defined wrongly. Y coordinate of player cannot be negative");
		}
		if (player_start_xcoord >= Game::map_xsize)
		{
			throw std::runtime_error("Options.dat: Data is defined wrongly. X coordinate of player cannot be greater than or equal to size of map (0 to (xsize - 1))");
		}
		if (player_start_ycoord >= Game::map_ysize)
		{
			throw std::runtime_error("Options.dat: Data is defined wrongly. Y coordinate of player cannot be greater than or equal to size of map (0 to (ysize - 1))");
		}
	}
	if (Game::magical_potion_xcoord == -1 && Game::magical_potion_ycoord == -1) {} //Ignore this case, this tells the item/entity generator to place the potion randomly
	else
	{
		if (Game::magical_potion_xcoord < 0)
		{
			throw std::runtime_error("Options.dat: Data is defined wrongly. X coordinate of magical potion cannot be negative");
		}
		if (Game::magical_potion_ycoord < 0)
		{
			throw std::runtime_error("Options.dat: Data is defined wrongly. Y coordinate of magical potion cannot be negative");
		}
		if (Game::magical_potion_xcoord >= Game::map_xsize)
		{
			throw std::runtime_error("Options.dat: Data is defined wrongly. X coordinate of magical potion cannot be greater than or equal to size of map (0 to (xsize - 1))");
		}
		if (Game::magical_potion_ycoord >= Game::map_ysize)
		{
			throw std::runtime_error("Options.dat: Data is defined wrongly. Y coordinate of magical potion cannot be greater than or equal to size of map (0 to (ysize - 1))");
		}
	}
	Game::player_data.setCoords(player_start_xcoord, player_start_ycoord);
}

//Loads data for items, mobs and threats from Data file into vectors (For new entities to be copy constructed from)
//May throw std::runtime_error (Should be caught by std::exception handler)
void Game::loadData()
{
	DataLoader data_loader{ "Data.dat" };
	if (!data_loader.is_open())
	{
		throw std::runtime_error("Unable to load game data, please make sure Data.dat is in the same folder.");
	}

	int code{ -1 }, current_code{ 0 };
	int object_typeid{ 0 };

	//For loading any object other than the player
	int number_to_place;
	//For loading Item
	int item_type;
	std::string name;
	int min_hp_change, max_hp_change, uses, value;
	double success_rate;
	//For loading Entity
	int max_hp, hp, atk, def, min_dmg, max_dmg, exp, level, gold;
	double run_chance, attack_frequency;
	//For loading Threat, no new unique variable needed

	while (data_loader())
	{
		data_loader >> code;
		data_loader.clearWhitespace();
		if (code < current_code || (code - 1) > current_code)
		{
			throw std::runtime_error("Data.dat: Incorrect code number or code order read, game data cannot be loaded.");
		}
		if (code > current_code)
		{
			object_typeid = 0;
			current_code = code;
		}
		switch (current_code) //Note: Break and not continue, there's code after switch case to execute
		{
		case 0:
		{
			//Load item, code 0
			data_loader >> item_type;
			if (static_cast<ItemType>(item_type) == ItemType::BASE)
			{
				//Get a name from within double quotes
				if (Game::getName(data_loader, name) < 0) //Returns -1 if error
				{
					throw std::runtime_error("Data.dat: " + data_loader.getErrorMsg());
				}
				data_loader >> value >> number_to_place;

				data_loader.checkStatus(); //Throws if error
				BaseItem base_item(static_cast<ItemType>(item_type), name, value, object_typeid);
				if (!base_item.valid())
				{
					throw std::runtime_error("Data.dat: Item data with object_typeid " + std::to_string(object_typeid) + " is invalid. A member variable value has been defined wrongly.");
				}

				if (number_to_place >= 0)
				{
					Game::base_item_number_to_place.push_back(number_to_place);
				}
				else
				{
					throw std::runtime_error("Data.dat: Item data with object_typeid " + std::to_string(object_typeid) + " has an invalid number of instances to place.");
				}

				Game::base_item_data.push_back(base_item);
				break;
			}
			else //Load a usable item
			{
				int item_object_typeid = object_typeid - Game::base_item_data.size();
				//Get a name from within double quotes
				if (Game::getName(data_loader, name) < 0) //Returns -1 if error
				{
					throw std::runtime_error("Data.dat: " + data_loader.getErrorMsg());
				}

				data_loader >> min_hp_change >> max_hp_change >> uses >> success_rate >> value >> number_to_place;

				data_loader.checkStatus(); //Throws if error
				Item item(static_cast<ItemType>(item_type), name, min_hp_change, max_hp_change, uses, success_rate, value, item_object_typeid);
				if (!item.valid()) //Validate data, if invalid, throw (Passes size of item_data for validating inventory ids)
				{
					throw std::runtime_error("Data.dat: Item data with object_typeid " + std::to_string(item_object_typeid) + " is invalid. A member variable value has been defined wrongly.");
				}

				if (number_to_place >= 0)
				{
					Game::item_number_to_place.push_back(number_to_place);
				}
				else
				{
					throw std::runtime_error("Data.dat: Item data with object_typeid " + std::to_string(item_object_typeid) + " has an invalid number of instances to place.");
				}

				Game::item_data.push_back(item);
			}
			break;
		}
		case 1:
		{
			//Load player, code 1
			if (Game::player_data.getMaxHealth() != 0) //Already loaded a player asset
			{
				throw std::runtime_error("Data.dat: More than one player type not allowed.");
			}

			//Get a name from within double quotes
			if (Game::getName(data_loader, name) < 0)
			{
				throw std::runtime_error("Data.dat: " + data_loader.getErrorMsg());
			}

			int inventory_size;
			data_loader >> max_hp >> hp >> atk >> def >> exp >> level >> gold >> inventory_size;

			data_loader.checkStatus(); //Throws if error
			Inventory inventory(inventory_size);
			int inventory_id;
			for (int i{ 0 }; i < inventory_size; ++i) //Load inventory
			{
				data_loader >> inventory_id;
				inventory.setItemID(i, inventory_id);
			}
			if (!inventory.valid(Game::base_item_data.size() + Game::item_data.size()))
			{
				throw std::runtime_error("Data.dat: Player inventory data is invalid. Please check the typeids, and if you entered the correct number of ids.");
			}
			Game::inventory_data.push_back(inventory);

			Game::player_data = Player(name, max_hp, hp, atk, def, exp, level, gold);
			data_loader.checkStatus(); //Throws if error
			if (!Game::player_data.valid())//validate data, if invalid, throw
			{
				throw std::runtime_error("Data.dat: Player data is invalid. A member variable value has been defined wrongly.");
			}
			break;
		}
		case 2:
		{
			//Load mob, code 2
			//Get a name from within double quotes
			if (Game::getName(data_loader, name) < 0)
			{
				throw std::runtime_error("Data.dat: " + data_loader.getErrorMsg());
			}

			data_loader >> max_hp >> hp >> atk >> def >> min_dmg >> max_dmg >> attack_frequency >> exp >> level >> run_chance >> gold >> number_to_place;

			data_loader.checkStatus(); //Throws if error
			Mob mob(name, max_hp, hp, atk, def, min_dmg, max_dmg, attack_frequency, exp, level, run_chance, gold, object_typeid);
			if (!mob.valid()) //Validate data, if invalid, throw
			{
				throw std::runtime_error("Data.dat: Mob data with object_typeid " + std::to_string(object_typeid) + " is invalid. Data.dat is in a wrong format.");
			}

			if (number_to_place >= 0)
			{
				Game::mob_number_to_place.push_back(number_to_place);
			}
			else
			{
				throw std::runtime_error("Data.dat: Mob data with object_typeid " + std::to_string(object_typeid) + " has an invalid number of instances to place.");
			}

			Game::mob_data.push_back(mob);
			break;
		}
		case 3:
		{
			//Load threat, code 3
			//Get a name from within double quotes
			if (Game::getName(data_loader, name) < 0)
			{
				throw std::runtime_error("Data.dat: " + data_loader.getErrorMsg());
			}

			data_loader >> atk >> min_dmg >> max_dmg >> attack_frequency >> run_chance >> number_to_place;

			data_loader.checkStatus(); //Throws if error
			Threat threat(name, atk, min_dmg, max_dmg, attack_frequency, run_chance, object_typeid);
			if (!threat.valid()) //Validate data, if invalid, throw
			{
				throw std::runtime_error("Data.dat: Threat data with object_typeid " + std::to_string(object_typeid) + " is invalid. Data.dat is in a wrong format.");
			}

			if (number_to_place >= 0)
			{
				Game::threat_number_to_place.push_back(number_to_place);
			}
			else
			{
				throw std::runtime_error("Data.dat: Threat data with object_typeid " + std::to_string(object_typeid) + " has an invalid number of instances to place.");
			}

			Game::threat_data.push_back(threat);
			break;
		}
		case 4:
			//Check for if total number of items/entities exceeded mapsize moved to load NPCs
			//End of file, success load
			data_loader.close();
			return;
		default:
			throw std::runtime_error("Data.dat: Unknown code read, game data cannot be loaded.");
		}
		++object_typeid;
	}
	throw std::runtime_error("Data.dat: Data loader has failed, game data cannot be loaded."); //Should return from function from loop if successful
}

//Gets a name defined within double quotes from a data file
//Returns 0 if successful, negative 1 if not
int Game::getName(DataLoader& data_loader, std::string& name)
{
	if (name.size() != 0)
	{
		name.clear();
	}
	if (data_loader.getWithinQuotes(name) < 0)
	{ //Error occurred
		return -1;
	}
	else
	{
		return 0;
	}
}

//Loads all the NPCs to be generated in the game
//May throw std::runtime_error (Should be caught by std::exception handler)
void Game::loadNPCs()
{
	DataLoader data_loader{ "NPCs.dat" };
	if (!data_loader.is_open())
	{
		throw std::runtime_error("Unable to load NPCs, please make sure NPCs.dat is in the same folder.");
	}

	int code{ -1 }, current_code{ 0 };
	int npc_typeid{ 0 };

	//For loading merchant
	std::string name;
	bool can_be_bought_from, can_be_sold_to;
	int buy_price_percent, sell_price_percent;
	int inventory_size;
	int xcoord, ycoord;

	while (data_loader())
	{
		data_loader >> code;
		if (code < current_code || (code - 1) > current_code)
		{
			throw std::runtime_error("NPCs.dat: Incorrect code number or code order read, game data cannot be loaded.");
		}
		if (code > current_code)
		{
			npc_typeid = 0;
			current_code = code;
		}
		switch (current_code)
		{
		case 0:
		{
			//Load Merchant, code 0
			if (Game::getName(data_loader, name) < 0) //Returns -1 if error
			{
				throw std::runtime_error("NPCs.dat: " + data_loader.getErrorMsg());
			}

			data_loader >> can_be_bought_from >> can_be_sold_to >> buy_price_percent >> sell_price_percent >> inventory_size;

			data_loader.checkStatus(); //Throws if error

			Inventory inventory(inventory_size);
			int inventory_id;
			for (int i{ 0 }; i < inventory_size; ++i) //Load inventory
			{
				data_loader >> inventory_id;
				inventory.setItemID(i, inventory_id);
			}
			if (!inventory.valid(Game::base_item_data.size() + Game::item_data.size()))
			{
				throw std::runtime_error("NPCs.dat: Merchant inventory data is invalid. Please check the typeids, and if you entered the correct number of ids.");
			}
			Game::inventory_data.push_back(inventory);

			data_loader.checkStatus(); //Throws if error
			Merchant merchant(name, can_be_bought_from, can_be_sold_to, buy_price_percent, sell_price_percent, npc_typeid);

			if (!merchant.valid())
			{
				throw std::runtime_error("NPCs.dat: Merchant with npc_typeid " + std::to_string(npc_typeid) + " is invalid. A member variable has been defined wrongly.");
			}

			data_loader >> xcoord >> ycoord;
			data_loader.checkStatus(); //Throws if error
			if (xcoord == -1 && ycoord == -1) {} //Ignore this case, this tells the item/entity generator to place this merchant randomly
			else if (Game::coordsOutOfBounds(xcoord, ycoord))
			{
				throw std::runtime_error("NPCs.dat: The coordinates of merchant with npc_typeid " + std::to_string(npc_typeid) + " are out of bounds. Please check again.");
			}
			merchant.setCoords(xcoord, ycoord);

			Game::merchant_data.push_back(merchant);
			break;
		}
		case 1:
		{
			//End of file. Since this is the last file to be loaded, all data has been loaded and we can check for any
			//data errors such as the options resulting in too many entities being spawned
			for (int i{ 0 }; i < static_cast<int>(merchant_data.size()); ++i)
			{
				for (int j{ i + 1 }; j < static_cast<int>(merchant_data.size()); ++j)
				{
					if (Game::merchant_data[i].getXCoord() == Game::merchant_data[j].getXCoord() && Game::merchant_data[i].getYCoord() == Game::merchant_data[j].getYCoord())
					{
						throw std::runtime_error("NPCs.dat: Coordinates collision between NPCs with merchant_id " + std::to_string(i) + " and " + std::to_string(j));
					}
				}
				if (Game::merchant_data[i].getXCoord() == Game::player_data.getXCoord() && Game::merchant_data[i].getYCoord() == Game::player_data.getYCoord())
				{
					throw std::runtime_error("NPCs.dat: Coordinates collision between NPC with merchant_id " + std::to_string(i) + " and player");
				}
			}

			int total_items_to_spawn_on_map = 1; //Start with magical potion counted
			int total_entities_to_spawn_on_map = 1; //Start with player counted
			int mapsize = Game::map_xsize * Game::map_ysize;
			
			for (int i : item_number_to_place)
			{
				total_items_to_spawn_on_map += i;
			}
			if (total_items_to_spawn_on_map > mapsize)
			{
				throw std::runtime_error("Total number of items to be placed is greater than total number of tiles, game cannot be instantiated.");
			}

			total_entities_to_spawn_on_map += merchant_data.size(); //Only one merchant allowed per type for now so merchant_data.size() == no. of merchants to spawn
			for (int i : mob_number_to_place)
			{
				total_entities_to_spawn_on_map += i;
			}
			for (int i : threat_number_to_place)
			{
				total_entities_to_spawn_on_map += i;
			}
			if (total_entities_to_spawn_on_map > mapsize)
			{
				throw std::runtime_error("Total number of entities to be placed is greater than total number of tiles, game cannot be instantiated.");
			}

			data_loader.close();
			return;
		}
		default:
			throw std::runtime_error("NPCs.dat: Invalid code read, game data cannot be loaded.");
		}
		++npc_typeid;
	}
	throw std::runtime_error("NPCs.dat: Data loader has failed, game data cannot be loaded."); //Should return from function from loop if successful
}

//Checks if a coordinate is outside of the boundaries of the map
//Will not throw
bool Game::coordsOutOfBounds(int xcoord, int ycoord) const
{
	if (xcoord < 0)
		return true;
	if (ycoord < 0)
		return true;
	if (xcoord >= Game::map_xsize)
		return true;
	if (ycoord >= Game::map_ysize)
		return true;
	return false;
}

//Verifies if the number of events loaded is correct
//May throw std::runtime_error (Should be caught by std::exception handler)
void Game::verifyEvents() const
{
	if (Game::base_item_data.size() != Game::event_list.getNumberOfObjectTypesLoaded(ObjectType::BASEITEM))
	{
		throw std::runtime_error("Too many or too few sets of events defined for base items. Did you mistype an object code?");
	}
	if (Game::item_data.size() != Game::event_list.getNumberOfObjectTypesLoaded(ObjectType::ITEM))
	{
		throw std::runtime_error("Too many or too few sets of events defined for items. Did you mistype an object code?");
	}
	if (Game::mob_data.size() != Game::event_list.getNumberOfObjectTypesLoaded(ObjectType::MOB))
	{
		throw std::runtime_error("Too many or too few sets of events defined for mobs. Did you mistype an object code?");
	}
	if (Game::threat_data.size() != Game::event_list.getNumberOfObjectTypesLoaded(ObjectType::THREAT))
	{
		throw std::runtime_error("Too many or too few sets of events defined for threats. Did you mistype an object code?");
	}
	if (Game::merchant_data.size() != Game::event_list.getNumberOfObjectTypesLoaded(ObjectType::MERCHANT))
	{
		throw std::runtime_error("Too many or too few sets of events defined for merchants. Did you mistype an object code?");
	}
}

//Places the items and entities on the map
//Should only be called ONCE, at the start of the game
void Game::placeItemsAndEntities()
{
	int xcoord, ycoord;

	//Place player
	Game::player = Game::player_data;
	if (Game::player.getXCoord() == -1 && Game::player.getYCoord() == -1) 
	{
		Game::map.getRandomTileWithoutEntityCoords(xcoord, ycoord);
		Game::player.setCoords(xcoord, ycoord); //Place on a random tile
	}
	Game::map(Game::player.getXCoord(), Game::player.getYCoord()).setEntity(EntityType::PLAYER, 0); //entity_id = 0
	Game::map(Game::player.getXCoord(), Game::player.getYCoord()).setExplored();
	Game::setVisibilityAround(Game::player, true);

	//Place magical potion
	if (Game::magical_potion_xcoord == -1 && Game::magical_potion_ycoord == -1)
	{
		Game::map.getRandomTileWithoutItemCoords(xcoord, ycoord);
		Game::map(xcoord, ycoord).setItem(ItemType::MAGICALPOTION, 0); //Place on random tile
	}
	else
	{
		Game::map(Game::magical_potion_xcoord, Game::magical_potion_ycoord).setItem(ItemType::MAGICALPOTION, 0);
	}

	int baseitem_id = 0;
	//Place baseitems
	for (size_t i{ 0 }; i < Game::base_item_number_to_place.size(); ++i)
	{
		int number_to_place = Game::base_item_number_to_place[i];
		for (; number_to_place > 0; --number_to_place, ++baseitem_id)
		{
			Game::map.getRandomTileWithoutItemCoords(xcoord, ycoord);
			Game::map(xcoord, ycoord).setItem(Game::base_item_data[i].getItemType(), baseitem_id);
			Game::base_items.push_back(Game::base_item_data[i]);
			Game::base_items[baseitem_id].setCoords(xcoord, ycoord);
		}
	}

	int item_id = 0;

	//Place items
	for (size_t i{ 0 }; i < Game::item_number_to_place.size(); ++i) //i is an alias for an item_typeid, which refers to the ith element in items_data
	{
		int number_to_place = Game::item_number_to_place[i];
		for (; number_to_place > 0; --number_to_place, ++item_id) //Generate and place all required number of objects of that specific item object
		{
			Game::map.getRandomTileWithoutItemCoords(xcoord, ycoord);
			Game::map(xcoord, ycoord).setItem(Game::item_data[i].getItemType(), item_id);
			Game::items.push_back(Game::item_data[i]);
			Game::items[item_id].setCoords(xcoord, ycoord);
		}
	}

	//Generate inventories
	for (const auto& inventory : Game::inventory_data)
	{
		Game::inventories.push_back(inventory);
	}

	//Generate the items in player's starting inventory
	Game::player.m_inventory = &Game::inventories[0];
	for (size_t i{ 0 }; i < Game::player.m_inventory->size(); ++i) //Inventory slot 1 to 4
	{
		if (Game::player.m_inventory->getItemID(i) != -1)
		{
			if (Game::player.m_inventory->getItemID(i) < static_cast<int>(Game::base_item_data.size())) //Is an ID for base item
			{
				Game::base_items.push_back(Game::base_item_data[Game::player.m_inventory->getItemID(i)]);
				Game::player.m_inventory->setItem(i, Game::base_items[baseitem_id].getItemType(), baseitem_id);
				++baseitem_id;
			}
			else //Is an ID for usable item
			{
				int item_typeid = Game::player.m_inventory->getItemID(i) - Game::base_item_data.size();
				Game::items.push_back(Game::item_data[item_typeid]);
				Game::player.m_inventory->setItem(i, Game::items[item_id].getItemType(), item_id);
				//Game::items[item_id].setCoords(-1, -1);
				++item_id;
			}
		}
		else //Nothing to generate in that slot
		{
			Game::player.m_inventory->clearItem(i);
		}
	}

	int merchant_id = 0;
	//Handle merchant generation
	for (auto& i : Game::merchant_data)
	{
		Game::merchants.push_back(i);
		merchants[merchant_id].m_inventory = &Game::inventories[merchant_id + 1]; //Merchants' inventories indexes start at 1 onwards (0 is held by player)
		//Generate items in merchant's inventory
		for (size_t j{ 0 }; j < merchants[merchant_id].m_inventory->size(); ++j) //j is the inventory slot number (Not index)
		{
			if (merchants[merchant_id].m_inventory->getItemID(j) != -1) //If there is an item to be generated
			{
				if (merchants[merchant_id].m_inventory->getItemID(j) < static_cast<int>(Game::base_item_data.size())) //Is an ID for base item (Intentional despite unsigned/signed warning)
				{
					Game::base_items.push_back(Game::base_item_data[merchants[merchant_id].m_inventory->getItemID(j)]);
					merchants[merchant_id].m_inventory->setItem(j, Game::base_items[baseitem_id].getItemType(), baseitem_id);
					++baseitem_id;
				}
				else //Is an ID for usable item
				{
					int item_typeid = merchants[merchant_id].m_inventory->getItemID(j) - Game::base_item_data.size();
					Game::items.push_back(Game::item_data[item_typeid]);
					merchants[merchant_id].m_inventory->setItem(j, Game::item_data[item_typeid].getItemType(), item_id);
					++item_id;
				}
			}
			else //Nothing to generate in that slot
			{
				merchants[merchant_id].m_inventory->clearItem(j);
			}
		}
		if (merchants[merchant_id].getXCoord() == -1 && merchants[merchant_id].getYCoord() == -1)
		{
			Game::map.getRandomTileWithoutEntityCoords(xcoord, ycoord);
			Game::merchants[merchant_id].setCoords(xcoord, ycoord); //Place on random tile
		}
		Game::map(merchants[merchant_id].getXCoord(), merchants[merchant_id].getYCoord()).setEntity(EntityType::MERCHANT, merchant_id); //Do for all NPCs
		++merchant_id;
	}

	//This should be unique per type of entity (MOB, THREAT) Note: Threat is not derived from Entity class despite this wording
	int entity_id = 0;
	//Place Mobs
	for (size_t i{ 0 }; i < Game::mob_number_to_place.size(); ++i) //i is an alias for an entity_typeid, which refers to the ith element in entity_data
	{
		int number_to_place = Game::mob_number_to_place[i];
		for (; number_to_place > 0; --number_to_place, ++entity_id) //Generate and place all required number of objects of that specific mob object
		{
			Game::map.getRandomTileWithoutEntityCoords(xcoord, ycoord);
			Game::map(xcoord, ycoord).setEntity(EntityType::MOB, entity_id);
			Game::mobs.push_back(Game::mob_data[i]);
			Game::mobs[entity_id].setCoords(xcoord, ycoord);
		}
	}
	//Reset entity_id, this should be unique per type of entity (MOB, THREAT)
	entity_id = 0;
	//Place Threats
	for (size_t i{ 0 }; i < Game::threat_number_to_place.size(); ++i) //i is an alias for an entity_typeid, which refers to the ith element in entity_data
	{
		int number_to_place = Game::threat_number_to_place[i];
		for (; number_to_place > 0; --number_to_place, ++entity_id) //Generate and place all required number of objects of that specific threat object
		{
			do {
				Game::map.getRandomTileWithoutEntityCoords(xcoord, ycoord);
			} while (Game::map(xcoord, ycoord).getItemType() == ItemType::MAGICALPOTION); //Get a new random tile without an entity if the tile has a magical potion
			Game::map(xcoord, ycoord).setEntity(EntityType::THREAT, i); //Threats are a special case, refer directly to entity_typeid
																		//Game::threats.push_back(Game::threat_data[i]); threats data never change
																		//Game::threats[entity_id].setCoords(xcoord, ycoord); Threats dont store their own position
		}
	}
}

//Updates the character that will be printed to the screen when the tile is printed
//Should be used when a change to the EntityType/ObjectType on that tile is made
//If entity present, set to entity character, else if no entity, but item present, set to item character, else nothing
void Game::updateMapTileCharacter(int x, int y)
{
	if (Game::player.getXCoord() == x && Game::player.getYCoord() == y)
	{
		Game::map(x, y).setCharacter('P');
	}
	else if (Game::map(x,y).getEntityType() == EntityType::MOB)
	{
		Game::map(x, y).setCharacter('M');
	}
	else if (Game::map(x, y).getEntityType() == EntityType::THREAT)
	{
		Game::map(x, y).setCharacter('T');
	}
	else if (Game::map(x, y).getEntityType() == EntityType::MERCHANT)
	{
		Game::map(x, y).setCharacter('N');
	}
	else if (Game::map(x, y).getItemType() != ItemType::NOTHING)
	{
		Game::map(x, y).setCharacter('I');
	}
	else //if (Game::map(x, y).getObjectType() == ObjectType::Nothing)
	{
		Game::map(x, y).setCharacter('O');
	}
}

//Updates all the characters that will be printed to the screen when the map is printed
//Should be used only when starting a new game or loading a saved game
void Game::updateEntireMapTileCharacter()
{
	for (int xcoord{ 0 }, ycoord{ 0 }; ycoord < Game::map.getYSize();)
	{
		Game::updateMapTileCharacter(xcoord, ycoord);
		++xcoord;
		if (xcoord == Game::map.getXSize())
		{
			xcoord = 0;
			++ycoord;
		}
	}
}

//Print the entire map on the screen
void Game::printMap() const
{
	for (int xcoord{ 0 }, ycoord{ 0 }; ycoord < Game::map.getYSize();)
	{
		if (Game::map(xcoord, ycoord).getIsExplored() && !Game::map(xcoord, ycoord).getIsVisible()) //If explored but not visible, dont show actual item/entity
		{
			std::cout << "E ";
		}
		else if (Game::map(xcoord, ycoord).getIsExplored() && Game::map(xcoord, ycoord).getIsVisible()) //If explored and visible, show actual item/entity
		{
			std::cout << Game::map(xcoord, ycoord).getCharacter() << ' ';
		}
		else //Not explored (Even if visible/not visible)
		{
			std::cout << "U "; //Unexplored
		}

		++xcoord;
		if (xcoord == Game::map.getXSize())
		{
			std::cout << '\n';
			xcoord = 0;
			++ycoord;
		}
	}
}

//Prints the amount of time passed since the game started and how much time is left to complete the game
void Game::printTimeLeft() const
{
	std::cout << "Time passed: " << Game::current_time << "days\t\tAmount of time left: " << Game::time_left << std::endl;
}

//Prints the player's level, atk, exp, health, max health, def and gold, and the items in his four inventory slots
void Game::printPlayerDetails(Player& player) const
{
	std::cout << "Level: " << player.getLevel() << "\t\t\tAtk: " << player.getAtk() << "\t\tExp: " << player.getExp() << '\n';
	std::cout << "Health: " << player.getHealth() << '/' << player.getMaxHealth() << "\t\t\tDef: " << player.getDef();
	std::cout << "\t\tGold: " << player.getGold() << '\n';
	Game::printInventoryTopDown(player.m_inventory);
}

//Prints all the items in the player's inventory
//When inventory is a class of its own, this function should be changed to accept any inventory object
void Game::printInventoryLeftRight(Inventory* inventory) const
{
	int inventory_index = 0;
	for (size_t i{ 1 }; i < (inventory->size() + 1); ++i, ++inventory_index) //Print details of inventory slots 1 - 4
	{
		if (inventory->getItemType(inventory_index) == ItemType::MAGICALPOTION)
		{
			std::cout << "Inventory slot " << i << ":Magical Potion\t\t";
		}
		else if (inventory->getItemType(inventory_index) != ItemType::NOTHING) //Has an item other than magical potion in that slot
		{
			if (inventory->getItemType(inventory_index) == ItemType::BASE)
			{
				std::cout << "Inventory slot " << i << ':' << Game::base_items[inventory->getItemID(inventory_index)].getName() << "\t\t";
			}
			else
			{
				std::cout << "Inventory slot " << i << ':' << Game::items[inventory->getItemID(inventory_index)].getName()
					<< "(Uses left: " << Game::items[inventory->getItemID(inventory_index)].getUses() << ")\t";
			}
		}
		else
		{
			std::cout << "Inventory slot " << i << ":NOTHING\t\t";
		}
		if (i % 2 == 0) //Is even number, newline
		{
			std::cout << '\n';
		}
	}
}

void Game::printInventoryTopDown(Inventory* inventory) const
{
	int half_of_inventory_size_ceil = static_cast<int>(std::ceil(inventory->size() / 2.0));
	int inventory_index = 0;
	for (size_t i{ 0 }; i < inventory->size(); ++i)
	{
		if (inventory->getItemType(inventory_index) == ItemType::MAGICALPOTION)
		{
			std::cout << "Inventory slot " << (inventory_index + 1) << ":Magical Potion\t\t";
		}
		else if (inventory->getItemType(inventory_index) != ItemType::NOTHING) //Has an item other than magical potion in that slot
		{
			if (inventory->getItemType(inventory_index) == ItemType::BASE)
			{
				std::cout << "Inventory slot " << (inventory_index + 1) << ':' << Game::base_items[inventory->getItemID(inventory_index)].getName() << "\t\t";
			}
			else
			{
				std::cout << "Inventory slot " << (inventory_index + 1) << ':' << Game::items[inventory->getItemID(inventory_index)].getName()
					<< "(Uses left: " << Game::items[inventory->getItemID(inventory_index)].getUses() << ")\t";
			}
		}
		else
		{
			std::cout << "Inventory slot " << (inventory_index + 1) << ":NOTHING\t\t";
		}
		if (inventory_index < half_of_inventory_size_ceil) //Just printed the left side. Next, print the right side
		{
			inventory_index += half_of_inventory_size_ceil;
		}
		else //Just printed right side. Next, print left side
		{
			std::cout << '\n';
			inventory_index -= (half_of_inventory_size_ceil - 1);
		}
	}
	if (inventory->size() % 2 == 1)
	{
		std::cout << '\n';
	}
}

//Prints the position of the player
void Game::printPlayerPosition(Player& player) const
{
	std::cout << player.getName() << " is currently at (" << player.getXCoord() << ", " << player.getYCoord() << ")\n";
}

//Prints a congratulatory message to the player
//Should be called when victory condition met (Finding magical potion in time)
void Game::printVictoryMessage() const
{
	std::cout << "Congratulations! You have found the magical potion before time was up!\n";
	std::cout << "You return home to give your grandfather the magical potion, and save his life!\n" << std::endl;
}

//Prints a game over message
//Should be called when game over condition met (Not finding magical potion in time or dying)
void Game::printGameOverMessage() const
{
	std::cout << "Too bad! Game over! ";
	if (Game::time_left == 0)
	{
		std::cout << "You couldn't find the magical potion in time to save your grandfather!\n";
		std::cout << "Maybe you will be lucky in an alternate universe...\n" << std::endl;
	}
	else //Player hp = 0
	{
		std::cout << "You lost all your strength and can no longer carry on the search for the magical potion!\n";
		std::cout << "Maybe you will be lucky in an alternate universe...\n" << std::endl;
	}
}

//Prints the name of the objects on the specified tile and its coordinates (Including uses left for an item if there is any on the tile)
void Game::printObjectsOnTileDetails(int xcoord, int ycoord) const
{
	//If a mob or threat present
	if (Game::map(xcoord, ycoord).getEntityType() == EntityType::MOB)
	{
		std::cout << '(' << xcoord << ", " << ycoord << ") ";
		std::cout << "There is an enemy " << Game::mobs[Game::map(xcoord, ycoord).getEntityID()].getName()
			<< "(Health: " << Game::mobs[Game::map(xcoord, ycoord).getEntityID()].getHealth() << ") here!\n";
	}
	else if (Game::map(xcoord, ycoord).getEntityType() == EntityType::THREAT)
	{
		std::cout << '(' << xcoord << ", " << ycoord << ") ";
		std::cout << "You will constantly take damage from the " << Game::threat_data[Game::map(xcoord, ycoord).getEntityID()].getName() << " if you remain here!\n";
	}
	else if (Game::map(xcoord, ycoord).getEntityType() == EntityType::MERCHANT)
	{
		std::cout << '(' << xcoord << ", " << ycoord << ") ";
		std::cout << "There is a friendly " << Game::merchants[Game::map(xcoord, ycoord).getEntityID()].getName() << " here that you can talk to.\n";
	}
	

	if (Game::map(xcoord, ycoord).getItemType() == ItemType::MAGICALPOTION)
	{
		std::cout << '(' << xcoord << ", " << ycoord << ") ";
		std::cout << "There is a magical potion here! You reaaaaally should want to take this.\n";
	}
	else if (Game::map(xcoord, ycoord).getItemType() != ItemType::NOTHING)
	{
		if (Game::map(xcoord, ycoord).getItemType() == ItemType::BASE)
		{
			std::cout << '(' << xcoord << ", " << ycoord << ") ";
			std::cout << "There is a " << Game::base_items[Game::map(xcoord, ycoord).getItemID()].getName() << " here that you may wish to pick up.\n";
		}
		else //Usable item
		{
			std::cout << '(' << xcoord << ", " << ycoord << ") ";
			std::cout << "There is a " << Game::items[Game::map(xcoord, ycoord).getItemID()].getName()
				<< "(Uses left: " << Game::items[Game::map(xcoord, ycoord).getItemID()].getUses() << ") here that you may wish to pick up.\n";
		}
	}
	
	if (Game::map(xcoord, ycoord).getEntityType() == EntityType::PLAYER && Game::map(xcoord, ycoord).getItemType() == ItemType::NOTHING) //Nothing other than player
	{
		std::cout << '(' << xcoord << ", " << ycoord << ") ";
		std::cout << "There is nothing here on this tile.\n";
	}
}

//Prints all the options available to the player to take in that situation
//If multiple players get implemented (Probably not for the console application, maybe when on SDL)
//Change gamestate to a playerstate unique to each player object and this function shld accept a player object as a parameter
void Game::printAvailablePlayerActions() const  
{
	if (Game::game_state == GameState::ONGOING)
	{
		//m) Move	u)Use inventory(Healing items)	p)Pick up/etc	c)Check surroundings	i)Inspect	s)Save	e)Exit
		std::cout << "m)Move\nu)Use inventory\np)Pick up/swap/drop items\nc)Check surroundings\ni)Inspect\ns)Save\ne)Exit\n";
	}
		
	else if(Game::game_state == GameState::ENCOUNTER_MOB) //Use inventory slot 1-4, swap item, run
	{
		//u)Use inventory(Healing/weapon)	p)Pick up/etc	r)Run	i)Inspect	s)Save	e)Exit
		std::cout << "u)Use inventory\np)Pick up/swap/drop items\n";
		int xcoord, ycoord;
		Game::player.getCoords(xcoord, ycoord);
		std::cout << "r)Run! (Chance of failure: " << Game::mobs[Game::map(xcoord, ycoord).getEntityID()].getRunChance() 
			<< "%)\ni)Inspect\ns)Save\ne)Exit\n";
	}
	
	else if (Game::game_state == GameState::ENCOUNTER_THREAT)
	{
		//u)Use inventory(Healing)	p)Pick up/etc	r)Run	i)Inspect	s)Save	e)Exit
		std::cout << "u)Use inventory\np)Pick up/swap/drop\n";
		int xcoord, ycoord;
		Game::player.getCoords(xcoord, ycoord);
		std::cout << "r)Run! (Chance of failure: " << Game::threat_data[Game::map(xcoord, ycoord).getEntityID()].getRunChance() 
			<< "%)\ni)Inspect\ns)Save\ne)Exit\n" ;
	}
	else//if Game::game_state == GameState::ENCOUNTER_MERCHANT);
	{
		//m)Move	u)Use inventory(Healing)	p)Pick up/etc	c)Check surroundings	i)Inspect	t)Talk	s)Save	e)Exit
		std::cout << "m)Move\nu)Use inventory\np)Pick up/swap/drop items\nc)Check surroundings\ni)Inspect\nt)Talk\ns)Save\ne)Exit\n";
	}
}

//Checks if the option entered by the player is a valid option or not and launches the appropriate sub menus if necessary
//Outputs a message if the action is invalid. Message details why option is invalid
//Note: Sets the player's action if valid
//Refer to printAvailablePlayerActions(), this function shld also accept a player object as a parameter when ported to SDL
bool Game::isPlayerActionValid()
{
	if(Game::game_state == GameState::ONGOING) //Map movement, inventory, check surroundings, save, exit
	{
		switch (Game::user_input)
		{
		case 'm':
		case 'M':
			return Game::playerMoveMenu();
		case 'u':
		case 'U':
			return Game::useItemMenu();
		case 'p': //Go to swap item menu
		case 'P':
			return Game::swapItemMenu(); //Will output its own invalid option message
		case 'c': //Check surroundings (Always possible when not in encounter)
		case 'C':
			Game::player.setAction(Action::CHECK_SURROUNDINGS);
			return true;
		case 'i':
		case 'I':
			Game::inspectMenu(Game::player.m_inventory, Game::player.getXCoord(), Game::player.getYCoord()); //Menu to choose which item from inventory or tile to inspect (Get details of)
			return true; //Return to caller, reprint everything (return to start of main loop)
		case 's':
		case 'S':
			std::cout << "Sorry, this has not been implemented yet\n";
			return false;
		case 'e':
		case 'E':
			Game::player.setAction(Action::EXIT);
			return true;
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			return false;
		}
	}
	else if (Game::game_state == GameState::ENCOUNTER_MOB)	//Use inventory (Weapon/Healing), swap item or run
	{
		switch (Game::user_input)
		{
		case 'u':
		case 'U':
			return Game::useItemMenu();
		case 'p': //Go to swap item menu
		case 'P':
			return Game::swapItemMenu(); //Will output its own invalid option message
		case 'r': //Run
		case 'R':
			Game::player.setAction(Action::RUN);
			return true;
		case 'i':
		case 'I':
			Game::inspectMenu(Game::player.m_inventory, Game::player.getXCoord(), Game::player.getYCoord()); //Menu to choose which item from inventory or tile to inspect (Get details of)
			return true; //Return to caller, reprint everything (return to start of main loop)
		case 's':
		case 'S':
			std::cout << "Sorry, this has not been implemented yet\n";
			return false;
		case 'e':
		case 'E':
			Game::player.setAction(Action::EXIT);
			return true;
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			return false;
		}
	}
	else if (Game::game_state == GameState::ENCOUNTER_THREAT)
	{
		switch (Game::user_input)
		{
		case 'u':
		case 'U':
			return Game::useItemMenu();
		case 'p': //Go to swap item menu
		case 'P':
			return Game::swapItemMenu(); //Will output its own invalid option message
		case 'r': //Run
		case 'R':
			Game::player.setAction(Action::RUN);
			return true;
		case 'i':
		case 'I':
			Game::inspectMenu(Game::player.m_inventory, Game::player.getXCoord(), Game::player.getYCoord()); //Menu to choose which item from inventory or tile to inspect (Get details of)
			return true; //Return to caller, reprint everything (return to start of main loop)
		case 's':
		case 'S':
			std::cout << "Sorry, this has not been implemented yet\n";
			return false;
		case 'e':
		case 'E':
			Game::player.setAction(Action::EXIT);
			return true;
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			return false;
		}
	}
	else //if (Game::game_state == GameState::ENCOUNTER_NPC)
	{
		switch (Game::user_input)
		{
		case 'm':
		case 'M':
			return Game::playerMoveMenu();
		case 'u':
		case 'U':
			return Game::useItemMenu();
		case 'p': //Go to swap item menu
		case 'P':
			return Game::swapItemMenu(); //Will output its own invalid option message
		case 'c': //Check surroundings (Always possible when not in encounter)
		case 'C':
			Game::player.setAction(Action::CHECK_SURROUNDINGS);
			return true;
		case 'i':
		case 'I':
			Game::inspectMenu(Game::player.m_inventory, Game::player.getXCoord(), Game::player.getYCoord()); //Menu to choose which item from inventory or tile to inspect (Get details of)
			return true; //Return to caller, reprint everything (return to start of main loop)
		case 't':
		case 'T':
			Game::talkToNPCMenu();
			return true; //Return to caller, reprint everything (return to start of main loop)
		case 's':
		case 'S':
			std::cout << "Sorry, this has not been implemented yet\n";
			return false;
		case 'e':
		case 'E':
			Game::player.setAction(Action::EXIT);
			return true;
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			return false;
		}
	}
}

//The sub-menu that pops up when user decides to move the player character
//Also accept player object as parameter in future
bool Game::playerMoveMenu()
{
	//1/a for left, 2/w for up, 3/s for down, 4/d for right
	std::cout << "In which direction would you like to move?\n"
		<< "Enter W, A, S or D or 1, 2, 3, or 4 for up, left, right or down respectively (Enter 'b' to go back)\n";
	while (true) //Does not exit until valid option chosen or player decides to return from this menu
	{
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case '1': //Move up
		case 'w':
		case 'W':
			if (Game::canMoveUp(Game::player))
			{
				Game::player.setAction(Action::MOVE_UP);
				return true;
			}
			else
			{
				std::cout << "Invalid option! You cannot move up, you are at the edge of the map.\n";
				continue; //Loop this menu again
			}
		case '2': //Move left
		case 'a':
		case 'A':
			if (Game::canMoveLeft(Game::player))
			{
				Game::player.setAction(Action::MOVE_LEFT);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move left, you are at the edge of the map.\n";
				continue; //Loop this menu again
			}
		case '3': //Move right
		case 'd':
		case 'D':
			if (Game::canMoveRight(Game::player))
			{
				Game::player.setAction(Action::MOVE_RIGHT);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move right, you are at the edge of the map.\n";
				continue; //Loop this menu again
			}
		case '4': //Move down
		case 's':
		case 'S':
			if (Game::canMoveDown(Game::player))
			{
				Game::player.setAction(Action::MOVE_DOWN);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move down, you are at the edge of the map.\n";
				continue; //Loop this menu again
			}
		case 'b':
		case 'B':
			return true; //Return to caller, reprint everything (return to start of main loop)
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//The sub-menu that pops up when the user decides to have the player character use an item from the inventory
//Also accept player object as parameter in future
bool Game::useItemMenu()
{
	std::cout << "Which inventory slot's item would you like to use? (Enter 'b' to go back)\n";
	while (true) //Does not exit until valid option chosen or player decides to return from this menu
	{
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case '1': //Use inventory slot 1
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			int inventory_index = Game::user_input - 49;
			if (inventory_index >= static_cast<int>(Game::player.m_inventory->size()))
			{
				std::cout << "Invalid option! Option is unrecognised.\n";
				continue; //Loop this menu again
			}
			//Base items have no use associated with them (Only used for selling for gold)
			if (Game::player.m_inventory->getItemType(inventory_index) == ItemType::BASE)
			{
				std::cout << "Invalid option! This item has no use.\n";
				continue;
			}
			//Healing items can always be used. Weapons can only be used against mob
			else if (Game::player.m_inventory->getItemType(inventory_index) == ItemType::HEALING) //Healing items can always be used
			{
				Game::player.setAction(static_cast<Action>(static_cast<int>(Action::INVENTORY1) + Game::user_input - 49));
				return true;
			}
			else if (Game::player.m_inventory->getItemType(inventory_index) == ItemType::WEAPON) //Weapons can only be used during encounter with mob
			{
				if (Game::game_state == GameState::ONGOING) //Invalid
				{
					std::cout << "Invalid option! You can't use a weapon when there is nothing to use it against.\n";
				}
				else if (Game::game_state == GameState::ENCOUNTER_MOB) //Weapon is valid to use only during encounter with mob
				{
					Game::player.setAction(static_cast<Action>(static_cast<int>(Action::INVENTORY1) + Game::user_input - 49));
					return true;
				}
				else if (Game::game_state == GameState::ENCOUNTER_THREAT) //Invalid
				{
					std::cout << "Invalid option! You can't use a weapon against a threat.\n";
				}
				else if (Game::game_state == GameState::ENCOUNTER_NPC) //Invalid
				{
					std::cout << "Invalid option! Don't use a weapon against the poor NPC!\n";
				}
				continue; //Was invalid (Not encounter with mob)
			}
			else //Nothing in that inventory slot
			{
				std::cout << "Invalid option! There is nothing in that inventory slot to use!\n";
				continue;
			}
		}
		case 'b':
		case 'B':
			return true; //Return to caller, reprint everything (return to start of main loop)
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//The sub-menu that pops up when user decides to pick up/swap or drop an item using an inventory slot
//Also accept player object as parameter in future
bool Game::swapItemMenu()
{
	std::cout << "Pick up/swap/drop item using which inventory slot? (Enter 'b' to go back)\n";
	while(true) //Does not exit until valid option chosen or player decides to return from this menu
	{
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case '1': //Pick up/swap/drop item with inventory slot 1
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			size_t inventory_index = Game::user_input - 49;
			if (inventory_index >= Game::player.m_inventory->size())
			{
				std::cout << "Invalid option! Option is unrecognised.\n";
				continue; //Loop this menu again
			}
			if (Game::map(Game::player.getXCoord(), Game::player.getYCoord()).getItemType() != ItemType::NOTHING ||
				Game::player.m_inventory->getItemType(inventory_index) != ItemType::NOTHING) //An item either on the tile or in player's inventory, allow action
			{
				Game::player.setAction(static_cast<Action>(static_cast<int>(Action::SWAP_ITEM1) + (Game::user_input - 49))); //;) reducing repeated code
				return true;
			}
			else
			{
				std::cout << "Invalid option! There is nothing that can be picked up/swapped/dropped.\n";
				continue; //Loop this menu again
			}
		}
		case 'b':
		case 'B':
			return true; //Return to caller, reprint everything (return to start of main loop)
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//The sub-menu that pops up when user decides to inspect something (Can inspect own inventory, and the tile the player is on)
//Three parameters, first for the inventory, second and third for x/ycoords for the tile
//Don't pass in xcoord and ycoord to get the version with inspecting item/entity on tile disabled
void Game::inspectMenu(const Inventory* const inventory, int xcoord, int ycoord)
{
	while (true)
	{
		//Note: This menu is not a menu to choose a action, this is deliberately reprinted
		std::cout << "What would you like to inspect? Enter the number of the inventory slot to inspect the item in that slot,\n" 
			<< "'t' for the item on the current tile, or 'e' for the entity on the current tile (Enter 'b' to go back)\n";
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			size_t inventory_index = Game::user_input - 49;
			if (inventory_index >= inventory->size())
			{
				std::cout << "Invalid option! Option is unrecognised.\n";
				continue; //Loop this menu again
			}
			if (inventory->getItemType(inventory_index) != ItemType::NOTHING)
			{
				if (inventory->getItemType(inventory_index) == ItemType::BASE) //Is a non-usable item
				{
					printItemDetails(Game::base_items[inventory->getItemID(inventory_index)]);
				}
				else //Is a usable item
				{
					printItemDetails(Game::items[inventory->getItemID(inventory_index)]);
				}
			}
			else
			{
				std::cout << "Invalid option! There is no item in that inventory slot to inspect.\n\n";
			}
			continue; //Loop in case player wants to read inspect another item/entity
		}
		case 't':
		case 'T':
			if (Game::map(xcoord, ycoord).getItemType() != ItemType::NOTHING)
			{
				if (Game::map(xcoord, ycoord).getItemType() == ItemType::MAGICALPOTION)
				{
					std::cout << "\nMagical Potion details:\n"
						<< "The ultimate potion! The best! The most wondrous! Perfection!\nJust take it already.\nEnd of details.\n\n";
				}
				else if (Game::map(xcoord, ycoord).getItemType() == ItemType::BASE) //Is a non-usable item
				{
					printItemDetails(Game::base_items[Game::map(xcoord, ycoord).getItemID()]);
				}
				else //If a usable item
				{
					printItemDetails(Game::items[Game::map(xcoord, ycoord).getItemID()]);
				}
			}
			else
			{
				std::cout << "Invalid option! There is no item in on this tile to inspect.\n\n";
			}
			continue; //Loop in case player wants to read inspect another item/entity
		case 'e':
		case 'E':
			if (Game::map(xcoord, ycoord).getEntityType() != EntityType::PLAYER) //Tile that player is on cannot be EntityType::NOTHING
			{
				if (Game::map(xcoord, ycoord).getEntityType() == EntityType::MOB)
				{
					printEntityDetails(Game::mobs[Game::map(xcoord, ycoord).getEntityID()]);
				}
				else if (Game::map(xcoord, ycoord).getEntityType() == EntityType::THREAT)
				{
					printEntityDetails(Game::threat_data[Game::map(xcoord, ycoord).getEntityID()]);
				}
				else //if (Game::map(xcoord, ycoord).getEntityType == EntityType::MERCHANT)
				{
					printEntityDetails(Game::merchants[Game::map(xcoord, ycoord).getEntityID()]);
				}
			}
			else
			{
				std::cout << "Invalid option! There is no entity other than yourself on this tile to inspect.\n\n";
			}
			continue; //Loop in case player wants to read inspect another item/entity
		case 'b':
		case 'B':
			return; //Return to caller
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//Specialized sub-menu for inspecting only an inventory (Like the merchant's inventory, with inspecting item/entity on tile disabled)
void Game::inspectMenu(const Inventory* const inventory)
{
	while (true)
	{
		//Note: This menu is not a menu to choose a action, this is deliberately reprinted
		std::cout << "What would you like to inspect? Enter the number of the inventory slot to inspect the item in that slot,\n";
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			size_t inventory_index = Game::user_input - 49;
			if (inventory_index >= inventory->size())
			{
				std::cout << "Invalid option! Option is unrecognised.\n";
				continue; //Loop this menu again
			}
			if (inventory->getItemType(inventory_index) != ItemType::NOTHING)
			{
				if (inventory->getItemType(inventory_index) == ItemType::BASE) //Is a non-usable item
				{
					printItemDetails(Game::base_items[inventory->getItemID(inventory_index)]);
				}
				else //Is a usable item
				{
					printItemDetails(Game::items[inventory->getItemID(inventory_index)]);
				}
			}
			else
			{
				std::cout << "Invalid option! There is no item in that inventory slot to inspect.\n\n";
			}
			continue; //Loop in case player wants to read inspect another item/entity
		}
		case 'b':
		case 'B':
			return; //Return to caller
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//Prints the details of an un-usable item
void Game::printItemDetails(BaseItem& base_item) const
{
	std::cout << '\n' << base_item.getName() << " details:\n"
		<< Game::event_list.getMessage(ObjectType::BASEITEM, base_item.getObjectTypeID(), BaseItemEvent::INSPECT_DESCRIPTION)
		<< "\nType: Trading\nValue: " << base_item.getValue() 
		<< "\nEnd of details.\n\n";
}

//Prints the details of a usable item
void Game::printItemDetails(Item& item) const
{
	std::cout << '\n' << item.getName() << " details:\n"
		<< Game::event_list.getMessage(ObjectType::ITEM, item.getObjectTypeID(), ItemEvent::INSPECT_DESCRIPTION);
	if (item.getItemType() == ItemType::HEALING)
	{
		std::cout << "\nType: Healing\n";
	}
	else if (item.getItemType() == ItemType::WEAPON)
	{
		std::cout << "\nType: Weapon\n";
	}
	std::cout << "Min HP change: " << item.getMinHpChange()
		<< "\nMax HP change: " << item.getMaxHpChange()
		<< "\nUses: " << item.getUses()
		<< "\nSuccess rate: " << item.getSuccessRate()
		<< "%\nValue: " << item.getValue()
		<< "\nEnd of details.\n\n";
}

//Prints details of a mob
void Game::printEntityDetails(Mob& mob) const
{
	std::cout << '\n' << mob.getName() << " details:\n"
		<< Game::event_list.getMessage(ObjectType::MOB, mob.getObjectTypeID(), MobEvent::INSPECT_DESCRIPTION)
		<< "\nLevel: " << mob.getLevel()
		<< "\nMax Health: " << mob.getMaxHealth()
		<< "\nHealth: " << mob.getHealth()
		<< "\nAtk: " << mob.getAtk()
		<< "\nDef: " << mob.getDef()
		<< "\nMin damage: " << mob.getMinDmg()
		<< "\nMax damage: " << mob.getMaxDmg()
		<< "\nAttack frequency: " << mob.getAtkFrequency()
		<< "\nExp drop: " << mob.getExp()
		<< "\nGold drop: " << mob.getGold()
		<< "\nRun chance from mob: " << mob.getRunChance()
		<< "\nEnd of details.\n\n";
}

//Prints details of a threat
void Game::printEntityDetails(Threat& threat) const
{
	std::cout << '\n' << threat.getName() << " details:\n"
		<< Game::event_list.getMessage(ObjectType::THREAT, threat.getObjectTypeID(), ThreatEvent::INSPECT_DESCRIPTION)
		<< "\nAtk: " << threat.getAtk()
		<< "\nMin damage: " << threat.getMinDmg()
		<< "\nMax damage: " << threat.getMaxDmg()
		<< "\nAttack frequency: " << threat.getAtkFrequency()
		<< "\nRun chance from threat: " << threat.getRunChance()
		<< "\nEnd of details.\n\n";
}

//Prints details of a merchant NPC
void Game::printEntityDetails(Merchant& merchant) const
{
	std::cout << '\n' << merchant.getName() << " details:\n"
		<< Game::event_list.getMessage(ObjectType::MERCHANT, merchant.getObjectTypeID(), MerchantEvent::INSPECT_DESCRIPTION)
		<< "\nCan be bought from: ";
	if (merchant.canBeBoughtFrom())
	{
		std::cout << "Yes";
	}
	else
	{
		std::cout << "No";
	}
	std::cout << "\nCan be sold to: ";
	if (merchant.CanBeSoldTo())
	{
		std::cout << "Yes";
	}
	else
	{
		std::cout << "No";
	}
	if (merchant.canBeBoughtFrom())
	{
		std::cout << "\nBuy price as percent of item value: " << merchant.getBuyPricePercent() << '%';
	}
	if (merchant.CanBeSoldTo())
	{
		std::cout << "\nSell price as percent of item value: " << merchant.getSellPricePercent() << '%';
	}
	std::cout << "\nEnd of details.\n\n";
}

//Selects the appropriate NPC talk menu based on what NPC the player is talking to
void Game::talkToNPCMenu()
{
	int npc_id = Game::map(Game::player.getXCoord(), Game::player.getYCoord()).getEntityID();
	switch (Game::map(Game::player.getXCoord(), Game::player.getYCoord()).getEntityType())
	{
	case EntityType::MERCHANT:
		Game::merchantTalkMenu(Game::merchants[npc_id]);
		return;
	//No other types of NPCs, add further cases in the future (Also, shouldn't need a default case)
	}
}

//The sub-menu that pops up when user decides to talk to an NPC
void Game::merchantTalkMenu(Merchant& merchant)
{
	while (true)
	{
		//Note: This menu is not a menu to choose an action, this is deliberately reprinted
		std::cout << merchant.getName() << ": Welcome to my shop! What can I do for you? (You have " << Game::player.getGold() << " gold)\n";
		if (merchant.canBeBoughtFrom())
		{
			std::cout << "p)Purchase items\n";
		}
		else
		{
			std::cout << "This merchant does not sell any items.\n";
		}
		if (merchant.CanBeSoldTo())
		{
			std::cout << "s)Sell items\n";
		}
		else
		{
			std::cout << "This merchant does not buy any items.\n";
		}
		std::cout << "b)Back\n";
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case 'p':
		case 'P':
			if (merchant.canBeBoughtFrom())
			{
				Game::npcBuyMenu(merchant);
			}
			else
			{
				std::cout << "Invalid option! This merchant does not sell any items.\n";
			}
			continue;
		case 's':
		case 'S':
			if (merchant.CanBeSoldTo())
			{
				Game::npcSellMenu(merchant);
			}
			else
			{
				std::cout << "Invalid option! This mechant does not buy items.\n";
			}
			continue;
		case 'b':
		case 'B':
			return; //Return to caller
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//The sub-menu that pops up when the user decides to buy something from a merchant
//Accept player object as second parameter in future
void Game::npcBuyMenu(Merchant& merchant)
{
	while (true)
	{
		std::cout << "What would you like to buy? (You have " << Game::player.getGold() << " gold)\n";
		Game::printInventoryTopDown(Game::player.m_inventory);
		for (size_t i{ 0 }; i < merchant.m_inventory->size(); ++i)//Prints all the items for sale
		{
			int item_id = merchant.m_inventory->getItemID(i); //change to index
			if (item_id == -1)
			{
				std::cout << (i + 1) << ")NOTHING\n";
			}
			else
			{
				if (merchant.m_inventory->getItemType(i) == ItemType::BASE)
				{
					std::cout << (i + 1) << ")" << Game::base_items[item_id].getName() << " (Cost: " << merchant.getItemBuyPrice(Game::base_items[item_id].getValue()) << " gold)\n";
				}
				else
				{
					std::cout << (i + 1) << ")" << Game::items[item_id].getName() << " (Cost: " << merchant.getItemBuyPrice(Game::items[item_id].getValue()) << " gold)\n";
				}
			}
		}
		std::cout << "i)Inspect\nb)Back\n";
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (user_input)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			size_t merchant_inventory_index = Game::user_input - 49;
			if (merchant_inventory_index >= merchant.m_inventory->size()) //If merchant has no such chosen inventory slot
			{
				std::cout << "Invalid option! Option is unrecognised.\n";
				continue;
			}
			if (merchant.m_inventory->getItemType(merchant_inventory_index) != ItemType::NOTHING) //If merchant has no item in that slot
			{
				if (Game::player.m_inventory->isFull()) //Player has no free inventory slot
				{
					std::cout << "Invalid option! Your inventory is full.\n";
					continue;
				}

				ItemType item_type = merchant.m_inventory->getItemType(merchant_inventory_index); //ItemType of item being bought
				int item_id = merchant.m_inventory->getItemID(merchant_inventory_index); //ID of the item being bought
				if (item_type == ItemType::BASE)
				{
					int item_buy_price = merchant.getItemBuyPrice(Game::base_items[item_id].getValue()); //Price of the item being bought
					if (Game::player.getGold() < item_buy_price) //Player has not enough gold to afford item
					{
						std::cout << "Invalid option! You cannot afford that item.\n";
						continue;
					}
					Game::player.loseGold(item_buy_price);
					int player_inventory_index = Game::player.m_inventory->getEmptyIndex();
					Game::player.m_inventory->setItem(player_inventory_index, Game::base_items[item_id].getItemType(), item_id);
					merchant.m_inventory->clearItem(merchant_inventory_index);
					std::cout << "You buy the " << Game::base_items[item_id].getName() << " for " << item_buy_price << " gold.\n";
				}
				else //Usable item
				{
					int item_buy_price = merchant.getItemBuyPrice(Game::items[item_id].getValue()); //Price of the item being bought
					if (Game::player.getGold() < item_buy_price) //Player has not enough gold to afford item
					{
						std::cout << "Invalid option! You cannot afford that item.\n";
						continue;
					}
					Game::player.loseGold(item_buy_price);
					int player_inventory_index = Game::player.m_inventory->getEmptyIndex();
					Game::player.m_inventory->setItem(player_inventory_index, Game::items[item_id].getItemType(), item_id);
					merchant.m_inventory->clearItem(merchant_inventory_index);
					std::cout << "You buy the " << Game::items[item_id].getName() << " for " << item_buy_price << " gold.\n";
				}
			}
			else
			{
				std::cout << "Invalid option! There is nothing in that slot to buy.\n";
				continue;
			}
			
			continue;
		}
		case 'i':
		case 'I':
			inspectMenu(merchant.m_inventory);
			continue;
		case 'b':
		case 'B':
			return; //Return to caller
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//The sub-menu that pops up when user decides to sell something to a merchant
//Accept player object as second parameter in future
void Game::npcSellMenu(Merchant& merchant)
{
	while (true)
	{
		std::cout << "What would you like to sell? (You have " << Game::player.getGold() << " gold)\n";
		for (size_t i{ 0 }; i < Game::player.m_inventory->size(); ++i) //Print details of inventory slots 1 - 4
		{
			if (Game::player.m_inventory->getItemType(i) != ItemType::NOTHING) //Has an item in that slot
			{
				if (Game::player.m_inventory->getItemType(i) == ItemType::BASE) //If is a non-usable item
				{
					std::cout << (i + 1) << ')' << Game::base_items[Game::player.m_inventory->getItemID(i)].getName() << "\tValue: "
						<< merchant.getItemSellPrice(Game::base_items[Game::player.m_inventory->getItemID(i)].getValue()) << " gold\n";
				}
				else
				{
					std::cout << (i + 1) << ')' << Game::items[Game::player.m_inventory->getItemID(i)].getName()
						<< "(Uses left: " << Game::items[Game::player.m_inventory->getItemID(i)].getUses() << ")\tValue: "
						<< merchant.getItemSellPrice(Game::items[Game::player.m_inventory->getItemID(i)].getValue()) << " gold\n";
				}
			}
			else
			{
				std::cout << (i + 1) << ")NOTHING\n";
			}
		}
		std::cout << "b)Back\n";
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			size_t player_inventory_index = Game::user_input - 49;
			if (player_inventory_index >= static_cast<int>(Game::player.m_inventory->size())) //If player has no such inventory slot
			{
				std::cout << "Invalid option! Option is unrecognised.\n";
				continue; //Loop this menu again
			}
			ItemType item_type = Game::player.m_inventory->getItemType(player_inventory_index);
			if (item_type != ItemType::NOTHING) //Player has an item in that slot
			{
				if (item_type == ItemType::BASE) //If non-usable item
				{
					int item_sell_price = merchant.getItemSellPrice(Game::base_items[Game::player.m_inventory->getItemID(player_inventory_index)].getValue());
					Game::player.gainGold(item_sell_price);
					std::cout << "You sell the " << Game::base_items[Game::player.m_inventory->getItemID(player_inventory_index)].getName()
						<< " for " << item_sell_price << " gold.\n";
					Game::player.m_inventory->clearItem(player_inventory_index);
					continue;
				}
				else
				{
					int item_sell_price = merchant.getItemSellPrice(Game::items[Game::player.m_inventory->getItemID(player_inventory_index)].getValue());
					Game::player.gainGold(item_sell_price);
					std::cout << "You sell the " << Game::items[Game::player.m_inventory->getItemID(player_inventory_index)].getName()
						<< " for " << item_sell_price << " gold.\n";
					Game::player.m_inventory->clearItem(player_inventory_index);
					continue;
				}
			}
			else
			{
				std::cout << "Invalid option! There is no item in that inventory slot to sell.\n";
				continue;
			}
		}
		case 'b':
		case 'B':
			return; //Return to caller
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//Evaluates the player's action (Moves player, then check for encounter, use inventory, etc)
void Game::evaluatePlayerAction()
{
	//During encounter with mob, use inventory option is implemented together with that of an encounter with nothing
	switch (Game::player.getAction())
	{
	case Action::MOVE_UP:
	case Action::MOVE_LEFT:
	case Action::MOVE_RIGHT:
	case Action::MOVE_DOWN:
	{
		Game::playerPassedTime(Game::time_taken_to_move);
		Game::playerMove(Game::player);
		Game::evaluatePossibleEncounter(); //Handles need_update_map
		break;
	}
	case Action::INVENTORY1: //No need to consider Base item
	case Action::INVENTORY2:
	case Action::INVENTORY3:
	case Action::INVENTORY4:
	case Action::INVENTORY5:
	case Action::INVENTORY6:
	case Action::INVENTORY7:
	case Action::INVENTORY8:
	case Action::INVENTORY9:
	{
		Game::playerPassedTime(Game::time_taken_to_use_inv);
		//Converts INVENTORY1 to 0, INVENTORY4 to 3
		int inventory_index = static_cast<int>(Game::player.getAction()) - static_cast<int>(Action::INVENTORY1);
		if (Game::player.m_inventory->getItemType(inventory_index) == ItemType::HEALING)
		{
			Game::useHealingItem(inventory_index); //Handles need_update_map
		}
		else //if( Game::player.getInventorySlotItemType(inventory_index) == ItemType::WEAPON)
		{	//If is weapon, and execution reached here, that means player is definitely in encounter with mob and not in any other game state
			Game::useWeaponItem(inventory_index); //Handles need_update_map
		}
		break;
	}
	case Action::SWAP_ITEM1:
	case Action::SWAP_ITEM2:
	case Action::SWAP_ITEM3:
	case Action::SWAP_ITEM4:
	case Action::SWAP_ITEM5:
	case Action::SWAP_ITEM6:
	case Action::SWAP_ITEM7:
	case Action::SWAP_ITEM8:
	case Action::SWAP_ITEM9:
	{
		Game::playerPassedTime(Game::time_taken_to_use_inv);
		//Converts SWAP_ITEM1 to 0, SWAP_ITEM4 to 3
		int inventory_index = static_cast<int>(Game::player.getAction()) - static_cast<int>(Action::SWAP_ITEM1);
		Game::swapItems(Game::player, inventory_index);
		break;
	}
	case Action::CHECK_SURROUNDINGS:
		Game::playerPassedTime(Game::time_taken_to_check_surroundings);
		Game::checkSurroundings(Game::player);
		Game::event_message_handler.addEventMsg("You checked your surroundings carefully...");
		Game::need_update_map = true;
		break;
	case Action::SAVE:
		break;
	case Action::EXIT:
		Game::game_state = GameState::EXITING;
		break;
	case Action::RUN:
	{
		playerPassedTime(time_taken_to_run);
		int xcoord, ycoord;
		Game::player.getCoords(xcoord, ycoord);
		int entity_id = Game::map(xcoord, ycoord).getEntityID();
		if (Game::map(xcoord, ycoord).getEntityType() == EntityType::MOB)
		{
			if (Game::player.runFrom(Game::mobs[entity_id])) //If run successful
			{
				Game::event_message_handler.addEventMsg("You ran away successfully.");
				playerMoveRandomDirection(Game::player);
				evaluatePossibleEncounter(); //Handles need_update_map
			}
			else
			{
				Game::event_message_handler.addEventMsg("You failed to run away!");
				Game::need_update_map = false;
			}
		}
		else if (Game::map(xcoord, ycoord).getEntityType() == EntityType::THREAT)
		{
			if (Game::player.runFrom(Game::threat_data[entity_id])) //If run successful
			{
				Game::event_message_handler.addEventMsg("You ran away successfully.");
				playerMoveRandomDirection(Game::player);
				evaluatePossibleEncounter(); //Handles need_update_map
			}
			else
			{
				Game::event_message_handler.addEventMsg("You failed to run away!");
				Game::need_update_map = false;
			}
		}
		break;
	}
	}
}

//Evaluates if there is a mob or threat on the tile the player has moved to and sets encounter (GameState) appropriately if true
//Also accept player object as parameter in future
//Will set if the game needs to update the map
void Game::evaluatePossibleEncounter()
{
	EntityType entity_on_tile_type = Game::map(Game::player.getXCoord(), Game::player.getYCoord()).getEntityType();
	int entity_id = Game::map(Game::player.getXCoord(), Game::player.getYCoord()).getEntityID();
	if (entity_on_tile_type == EntityType::MOB)
	{
		Game::game_state = GameState::ENCOUNTER_MOB;
		Game::computer_next_turn_time = Game::player_next_turn_time + Game::first_encounter_reaction_time; //Bonus reaction time when encountering mob
		Game::event_message_handler.addEventMsg("You have encountered a " + Game::mobs[entity_id].getName() + "! Prepare yourself!");
		//encounter msg
		Game::need_update_map = false;
	}
	else if (entity_on_tile_type == EntityType::THREAT)
	{
		Game::game_state = GameState::ENCOUNTER_THREAT;
		Game::computer_next_turn_time = Game::player_next_turn_time; //Threat should immediately attack the player
		Game::event_message_handler.addEventMsg("You have encountered a " + Game::threat_data[entity_id].getName() + "! Run!");
		Game::need_update_map = false;
	}
	else if (entity_on_tile_type == EntityType::MERCHANT)
	{
		//For now, only npc is merchant
		Game::game_state = GameState::ENCOUNTER_NPC;
		if (Game::merchants[entity_id].getNPCType() == NPCType::MERCHANT)
		{
			Game::event_message_handler.addEventMsg("You have encountered a friendly " + Game::merchants[entity_id].getName() + ". You can buy/sell items!");
		}
		Game::need_update_map = true;
	}
	else //(entity_on_tile_type == EntityType::NOTHING)
	{
		Game::game_state = GameState::ONGOING;
		Game::need_update_map = true;
	}
}

//Moves the player as well as update surrounding tiles visibility and whether they are explored by the player
void Game::playerMove(Player& player)
{
	setVisibilityAround(player, false);
	int xcoord, ycoord;
	player.getCoords(xcoord, ycoord);
	if (Game::map(xcoord, ycoord).getEntityType() == EntityType::PLAYER) //Tile player is on is not occupied by any other entity (Player isn't running)
		Game::map(xcoord, ycoord).setEntity(EntityType::NOTHING, -1); //Can safely set tile being moved away from to hold no entity

	switch (player.getAction())
	{
	case Action::MOVE_UP:
		player.setYCoord(ycoord - 1);
		Game::event_message_handler.addEventMsg("You moved up.");
		break;
	case Action::MOVE_LEFT:
		player.setXCoord(xcoord - 1);
		Game::event_message_handler.addEventMsg("You moved left.");
		break;
	case Action::MOVE_RIGHT:
		player.setXCoord(xcoord + 1);
		Game::event_message_handler.addEventMsg("You moved right.");
		break;
	case Action::MOVE_DOWN:
		player.setYCoord(ycoord + 1);
		Game::event_message_handler.addEventMsg("You moved down.");
		break;
	}
	updateMapTileCharacter(xcoord, ycoord); //Update old tile
	player.getCoords(xcoord, ycoord);
	if (Game::map(xcoord, ycoord).getEntityType() == EntityType::NOTHING) //Will not have encounter
		Game::map(xcoord, ycoord).setEntity(EntityType::PLAYER, 0);

	updateMapTileCharacter(xcoord, ycoord); //Update new tile
	setVisibilityAround(player, true);
	Game::map(xcoord, ycoord).setExplored();
}

//Moves the player in a random direction
//Checks if that random direction is a valid move and loops until it gets a valid random move
void Game::playerMoveRandomDirection(Player& player)
{
	bool valid_move = false;
	int rand;
	do
	{
		rand = getRandomInt(1, 4);
		switch (rand)
		{
		case 1:
			valid_move = canMoveUp(Game::player);
			if(valid_move)
				Game::player.setAction(Action::MOVE_UP);
			break;
		case 2:
			valid_move = canMoveLeft(Game::player);
			if (valid_move)
				Game::player.setAction(Action::MOVE_LEFT);
			break;
		case 3:
			valid_move = canMoveRight(Game::player);
			if (valid_move)
				Game::player.setAction(Action::MOVE_RIGHT);
			break;
		case 4:
			valid_move = canMoveDown(Game::player);
			if (valid_move)
				Game::player.setAction(Action::MOVE_DOWN);
			break;
		}
	} while (valid_move == false);
	playerMove(Game::player);
}

//Sets the visibility of all 8 surrounding tiles and the tile that the entity is on to new_visibility
void Game::setVisibilityAround(const Entity& entity, bool new_visibility)
{
	int xcoord, ycoord;
	entity.getCoords(xcoord, ycoord);
	int map_xsize = Game::map.getXSize(), map_ysize = Game::map.getYSize();

	Game::map(xcoord, ycoord).setVisible(new_visibility); //Set for the tile that the entity is on
	if (xcoord < (map_xsize - 1) && ycoord < (map_ysize - 1)) //Not on bottom right side of the map
		Game::map(xcoord + 1, ycoord + 1).setVisible(new_visibility);
	if (ycoord < (map_ysize - 1)) //Not on bottom most side of the map
		Game::map(xcoord, ycoord + 1).setVisible(new_visibility);
	if (xcoord > 0 && (ycoord < map_ysize - 1)) //Not on bottom left corner of map
		Game::map(xcoord - 1, ycoord + 1).setVisible(new_visibility);
	if (xcoord > 0)	//Not on left most side of the map
		Game::map(xcoord - 1, ycoord).setVisible(new_visibility);
	if (xcoord > 0 && ycoord > 0) //Not on top left corner of the map
		Game::map(xcoord - 1, ycoord - 1).setVisible(new_visibility);
	if (ycoord > 0) //Not on top most side of the map
		Game::map(xcoord, ycoord - 1).setVisible(new_visibility);
	if (xcoord < (map_xsize - 1) && ycoord > 0) //Not on top right corner of the map
		Game::map(xcoord + 1, ycoord - 1).setVisible(new_visibility);
	if (xcoord < (map_xsize - 1)) //Not on right most side of the map
		Game::map(xcoord + 1, ycoord).setVisible(new_visibility);
}

//Uses item (Confirmed ItemType::HEALING) in the inventory slot to heal player
void Game::useHealingItem(int inventory_index)
{
	int item_id = Game::player.m_inventory->getItemID(inventory_index);

	double success_rate = static_cast<int>(Game::items[item_id].getSuccessRate() * 100'000);
	int random_number = getRandomInt(0, 10'000'000);
	Game::event_message_handler.addEventMsg("You rolled " + std::to_string(random_number) + " out of 10'000'000");
	if (random_number < success_rate) //If random number is within success rate, successful at using item
	{
		int heal_amount = getRandomInt(Game::items[item_id].getMinHpChange(), Game::items[item_id].getMaxHpChange());
		Game::player.heal(heal_amount);
		Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::ITEM, Game::items[item_id].getObjectTypeID(), ItemEvent::USE_SUCCESS));
		Game::event_message_handler.addEventMsg("You heal for " + std::to_string(heal_amount) + " points of health");
	}
	else //Use item failed
	{
		Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::ITEM, Game::items[item_id].getObjectTypeID(), ItemEvent::USE_FAILURE));
	}
	Game::items[item_id].decrementUses();
	if (Game::items[item_id].getUses() == 0)
	{
		//Manage used up items
		Game::player.m_inventory->clearItem(inventory_index);
		logUsedItem(item_id);
		Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::ITEM, Game::items[item_id].getObjectTypeID(), ItemEvent::ITEM_USED_UP));
	}
	
	if (Game::game_state == GameState::ONGOING)
	{
		Game::need_update_map = true;
	}
	else //In any encounter
	{
		Game::need_update_map = false;
	}
}

//Uses item (Confirmed ItemType::WEAPON) in the inventory slot to attack mob
void Game::useWeaponItem(int inventory_index)
{
	int item_id = Game::player.m_inventory->getItemID(inventory_index);

	double success_rate = static_cast<int>(Game::items[item_id].getSuccessRate() * 100'000);
	int random_number = getRandomInt(0, 10'000'000);
	Game::event_message_handler.addEventMsg("You rolled " + std::to_string(random_number) + " out of 10'000'000");
	if (random_number < success_rate) //If random number is within success rate, successful at using item
	{
		int expected_damage = getRandomInt(Game::items[item_id].getMinHpChange(), Game::items[item_id].getMaxHpChange());
		int xcoord, ycoord;
		Game::player.getCoords(xcoord, ycoord);
		int entity_id = Game::map(xcoord, ycoord).getEntityID();

		int actual_damage = Game::evaluateActualDamage(expected_damage, player.getAtk(), Game::mobs[entity_id].getDef());

		int damage_discrepency = actual_damage - expected_damage;

		Game::mobs[entity_id].takeDamage(actual_damage);
		Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::ITEM, Game::items[item_id].getObjectTypeID(), ItemEvent::USE_SUCCESS));
		Game::event_message_handler.addEventMsg("You attack the " + Game::mobs[entity_id].getName() + " for " + std::to_string(actual_damage) + " points of damage!");
		if (damage_discrepency < 0) //Took less damage than expected
		{
			Game::event_message_handler.addEventMsg(Game::mobs[entity_id].getName() + "\'s greater defense allowed it to block " + std::to_string(-damage_discrepency) + " points of damage.");
		}
		else if (damage_discrepency > 0)
		{
			Game::event_message_handler.addEventMsg("Your greater attack allowed you to deal " + std::to_string(damage_discrepency) + " more points of damage.");
		}

		if (Game::mobs[entity_id].isDead())
		{
			Game::game_state = GameState::ONGOING;
			Game::map(xcoord, ycoord).setEntity(EntityType::PLAYER, 0);
			updateMapTileCharacter(xcoord, ycoord);
			logDeadMob(entity_id);
			Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::MOB, Game::mobs[entity_id].getObjectTypeID(), MobEvent::DEATH));
			int player_initial_level = Game::player.getLevel();
			Game::player.gainExp(Game::mobs[entity_id].getExp()); //Note: Levelling implementation has to be relooked in the future
			Game::player.gainGold(Game::mobs[entity_id].getGold());
			Game::event_message_handler.addEventMsg("You gain " + std::to_string(Game::mobs[entity_id].getExp()) + " EXP and " 
				+ std::to_string(Game::mobs[entity_id].getGold()) + " gold");
			if (Game::player.getLevel() > player_initial_level)
			{
				Game::event_message_handler.addEventMsg("You feel additional strength surging forth from gaining experience... You levelled up!"); //Has to be reworked in the future
			}
			Game::need_update_map = true;
		}
		//else Game::need_update_map = false; Same as not updating the variable
	}
	else //Use item failed
	{
		Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::ITEM, Game::items[item_id].getObjectTypeID(), ItemEvent::USE_FAILURE));
		Game::need_update_map = false;
	}

	Game::items[item_id].decrementUses(); //Success or not, item's number of uses left will be decremented
	if (Game::items[item_id].getUses() == 0)
	{
		//Manage used up items
		Game::player.m_inventory->clearItem(inventory_index);
		logUsedItem(item_id);
		Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::ITEM, Game::items[item_id].getObjectTypeID(), ItemEvent::ITEM_USED_UP));
	}
}

int Game::evaluateActualDamage(int expected_damage, int attacker_atk, int defender_def)
{
	if (attacker_atk > defender_def)
	{
		return static_cast<int>(((std::cbrt(attacker_atk - defender_def) + 0.35) * expected_damage));
	}
	else if (attacker_atk == defender_def)
	{
		return expected_damage;
	}
	else //if(attacker_atk < defender_def)
	{
		int actual_damage = static_cast<int>((0.83 - ((std::cbrt(defender_def - attacker_atk) - 1) / 2.0)) * expected_damage);
		if (actual_damage < 0) return 0;
		else return actual_damage;
	}
}

//Allows player to pick up/drop/swap items with the item on the tile that the player is on using the specified inventory slot
//Note: Does not check if inventory slot is out of bounds
void Game::swapItems(Player& player, int inventory_index)
{
	//Store item held by player temporarily
	int inventory_item_id = player.m_inventory->getItemID(inventory_index);
	ItemType inventory_item_type = player.m_inventory->getItemType(inventory_index);
	int xcoord, ycoord;
	player.getCoords(xcoord, ycoord);

	if (player.m_inventory->getItemType(inventory_index) == ItemType::NOTHING) //Nothing in inventory, i.e. picking up item
	{
		Game::event_message_handler.addEventMsg("Successfully picked up item.");
	}
	else if (Game::map(xcoord, ycoord).getItemType() == ItemType::NOTHING) //Nothing on tile, i.e. dropping an item
	{
		Game::event_message_handler.addEventMsg("Successfully dropped item.");
	}
	else //Something in both inventory and on tile, i.e. swapping items
	{
		Game::event_message_handler.addEventMsg("Successfully swapped items.");
	}

	player.m_inventory->setItem(inventory_index, Game::map(xcoord, ycoord).getItemType(), Game::map(xcoord, ycoord).getItemID());
	Game::map(xcoord, ycoord).setItem(inventory_item_type, inventory_item_id);


	if (Game::game_state == GameState::ONGOING)
	{
		Game::need_update_map = true;
	}
	else //In any encounter
	{
		Game::need_update_map = false;
	}
}

//Sets the all 8 tiles surrounding the entity to be explored
void Game::checkSurroundings(const Entity& entity)
{
	int xcoord, ycoord;
	entity.getCoords(xcoord, ycoord);
	int map_xsize = Game::map.getXSize(), map_ysize = Game::map.getYSize();

	//Game::map(xcoord, ycoord).setExplored; //Set for the tile that the entity is on (Not needed as it is already set when moved to that tile)
	if (xcoord < (map_xsize - 1) && ycoord < (map_ysize - 1)) //Not on bottom right side of the map
		Game::map(xcoord + 1, ycoord + 1).setExplored();
	if (ycoord < (map_ysize - 1)) //Not on bottom most side of the map
		Game::map(xcoord, ycoord + 1).setExplored();
	if (xcoord > 0 && (ycoord < map_ysize - 1)) //Not on bottom left corner of map
		Game::map(xcoord - 1, ycoord + 1).setExplored();
	if (xcoord > 0)	//Not on left most side of the map
		Game::map(xcoord - 1, ycoord).setExplored();
	if (xcoord > 0 && ycoord > 0) //Not on top left corner of the map
		Game::map(xcoord - 1, ycoord - 1).setExplored();
	if (ycoord > 0) //Not on top most side of the map
		Game::map(xcoord, ycoord - 1).setExplored();
	if (xcoord < (map_xsize - 1) && ycoord > 0) //Not on top right corner of the map
		Game::map(xcoord + 1, ycoord - 1).setExplored();
	if (xcoord < (map_xsize - 1)) //Not on right most side of the map
		Game::map(xcoord + 1, ycoord).setExplored();

}

//Checks if victory condition of finding potion met, and if loss condition of running out of time met
//In future, evaluates natural events unrelated to player and entities
void Game::evaluateEvents()
{
	if (noTimeLeft())
	{
		Game::game_state = GameState::LOST;
		return;
	}
	//else
	if (playerHasMagicalPotion())
	{
		Game::game_state = GameState::WON;
		return;
	}
}

//Checks if it is time for mobs/threats to attack(/move in the future) and check for loss condition of player death
void Game::evaluateEncounters()
{
	if (Game::game_state == GameState::ENCOUNTER_MOB)
	{
		while (Game::computer_next_turn_time <= Game::current_time) //Repeat until its next timing to attack has not passed yet
		{
			int entity_id = Game::map(Game::player.getXCoord(), Game::player.getYCoord()).getEntityID();
			int expected_damage = getRandomInt(Game::mobs[entity_id].getMinDmg(), Game::mobs[entity_id].getMaxDmg());
			int actual_damage = Game::evaluateActualDamage(expected_damage, Game::mobs[entity_id].getAtk(), player.getDef());
			int damage_discrepency = actual_damage - expected_damage;

			Game::player.takeDamage(actual_damage);
			Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::MOB, Game::mobs[entity_id].getObjectTypeID(), MobEvent::ATTACK));
			Game::event_message_handler.addEventMsg("You take " + std::to_string(actual_damage) + " points of damage!");
			if (damage_discrepency < 0) //Took less damage than expected
			{
				Game::event_message_handler.addEventMsg("Your greater defense allowed you to block " + std::to_string(-damage_discrepency) + " points of damage.");
			}
			else if (damage_discrepency > 0)
			{
				Game::event_message_handler.addEventMsg(Game::mobs[entity_id].getName() + "\'s greater attack allowed it to deal you " + std::to_string(damage_discrepency) + " more points of damage.");
			}
			Game::computer_next_turn_time += Game::mobs[entity_id].getAtkFrequency();
		}
	}
	else if (Game::game_state == GameState::ENCOUNTER_THREAT) //Note: Threat will always attack
	{
		while (Game::computer_next_turn_time <= Game::current_time) //Repeat until its next timing to attack has not passed yet
		{
			int entity_id = Game::map(Game::player.getXCoord(), Game::player.getYCoord()).getEntityID();
			int expected_damage = getRandomInt(Game::threat_data[entity_id].getMinDmg(), Game::threat_data[entity_id].getMaxDmg());
			int actual_damage = Game::evaluateActualDamage(expected_damage, Game::threat_data[entity_id].getAtk(), player.getDef());
			int damage_discrepency = actual_damage - expected_damage;

			Game::player.takeDamage(actual_damage);
			Game::event_message_handler.addEventMsg(Game::event_list.getMessage(ObjectType::THREAT, Game::threat_data[entity_id].getObjectTypeID(), ThreatEvent::ATTACK));
			Game::event_message_handler.addEventMsg("You take " + std::to_string(actual_damage) + " points of damage!");
			if (damage_discrepency < 0) //Took less damage than expected
			{
				Game::event_message_handler.addEventMsg("Your greater defense allowed you to block " + std::to_string(-damage_discrepency) + " points of damage.");
			}
			else if (damage_discrepency > 0)
			{
				Game::event_message_handler.addEventMsg(Game::threat_data[entity_id].getName() + "\'s greater attack allowed it to deal you " + std::to_string(damage_discrepency) + " more points of damage.");
			}
			Game::computer_next_turn_time += Game::threat_data[entity_id].getAtkFrequency();
		}
	}

	if (playerDied())
	{
		Game::game_state = GameState::LOST;
	}
}

void Game::playerPassedTime(double time_passed)
{
	Game::player_next_turn_time = Game::current_time + time_passed;

	int whole_time_to_pass = static_cast<int>(time_passed); //Hold the whole number part of time_to_pass
	int decimal_time_to_pass = static_cast<int>(time_passed * 100) - whole_time_to_pass * 100; //Hold the decimal part of time_to_pass

	Game::event_message_handler.addEventMsg(std::to_string(whole_time_to_pass) + '.' + std::to_string(decimal_time_to_pass) + " Days have passed...");
}

//Adjusts time_left and the current_time accordingly
//Note: Changes time_left to 0 if time_to_pass exceeds time_left
void Game::advanceTime(double time_to_pass)
{
	Game::time_left -= time_to_pass; 
	Game::current_time += time_to_pass;
	if (Game::time_left < 0.0)
	{
		Game::time_left = 0.0;
	}
}

//Checks if there is no time left to find the magical potion
bool Game::noTimeLeft() const
{
	if (Game::time_left == 0)
	{
		return true;
	}
	return false;
}

//Checks if the player has died
bool Game::playerDied() const
{
	if (Game::player.isDead())
		return true;
	//else
	return false;
}

//Checks if the player has the magical potion in any of his four inventory slots
bool Game::playerHasMagicalPotion() const
{
	for (int i{ 0 }; i < 4; ++i)
	{
		if (Game::player.m_inventory->getItemType(i) == ItemType::MAGICALPOTION)
			return true;
	}
	//if all checks above false,
	return false;
}

//Checks if an entity can move up
bool Game::canMoveUp(const Entity& entity) const
{
	if (entity.getYCoord() != 0)
		return true;
	return false;
}

//Checks if an entity can move left
bool Game::canMoveLeft(const Entity& entity) const
{
	if (entity.getXCoord() != 0)
		return true;
	return false;
}

//Checks if an entity can move right
bool Game::canMoveRight(const Entity& entity) const
{
	if (entity.getXCoord() != (Game::map.getXSize() - 1))
		return true;
	return false;
}

//Checks if an entity can move down
bool Game::canMoveDown(const Entity& entity) const
{
	if (entity.getYCoord() != (Game::map.getYSize() - 1))
		return true;
	return false;
}