#include <iostream>
#include <iomanip>
#include <string>
#include "Game.h"

void Game::start()
{
	std::cout << std::fixed << std::setprecision(2);

	while (true)
	{
		printMap();

		do {
			Game::printTimeLeft();
			Game::printPlayerDetails();
			if (game_state == GameState::WON)
			{
				Game::event_message_handler.printMsgs();
				printVictoryMessage();
				return;
			}
			if (game_state == GameState::LOST)
			{
				Game::event_message_handler.printMsgs();
				printGameOverMessage();
				return;
			}

			bool input_valid;
			Game::printPlayerPosition();
			Game::event_message_handler.printMsgs();
			Game::printObjectsOnPlayerTileDetails();	//Print what is on the same tile at the player (Item/entity)
			Game::printAvailablePlayerActions();	//Move, inventory, check surroundings, save, exit
			do {

				std::cin >> Game::user_input;
				if (!validInput() || !Game::isPlayerActionValid())
				{
					if (Game::user_input == 'b' || Game::user_input == 'B')
					{	//If here as a result of user choosing to swap item, but decided not to and went back, don't print invalid option
						if (Game::need_update_map)
						{
							Game::printMap();	//Reprint Note: This portion of code is up for revision
						}
						Game::printTimeLeft();
						Game::printPlayerDetails();
						Game::printPlayerPosition();
						Game::event_message_handler.printMsgs();
						Game::printObjectsOnPlayerTileDetails();	//Print what is on the same tile at the player (Item/entity)
						Game::printAvailablePlayerActions();	//Move, inventory, check surroundings, save, exit
					}
					//Else, the message telling the player that he/she chose an invalid option and why will be printed by isPlayerActionValid
					input_valid = false;
				}
				else 
					input_valid = true;
			} while (!input_valid);

			Game::evaluatePlayerAction(); //Move, attack, use inventory
			if (Game::game_state == GameState::EXITING)
				return;
			else if (Game::game_state == GameState::SAVING)
			{
				std::cout << "Sorry, this has not been implemented yet\n";
			}
			else
				Game::evaluateEvents();		//Attacked by mob/update shop/etc, update time(Non-player events)
		} while (Game::need_update_map == false);	//Includes shop, encounter with mob
	}
}

//Clears all variables defined at the start of the game to get ready for the next time its run
void Game::cleanUpGame()
{
	Game::player.clear();
	Game::items.clear();
	Game::base_items.clear();
	Game::mobs.clear();
	Game::npcs.clear();
	Game::map.clear();
	Game::used_items_id.clear();
	Game::dead_mobs_id.clear();
	//Game::threats.erase(); currently threats not used
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
	std::cout << "Too bad! Game over!";
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

//Prints the name of the objects on the tile that the player is on (Including uses left for an item if there is any on the tile)
void Game::printObjectsOnPlayerTileDetails() const
{
	int xcoord, ycoord;
	Game::player[0].getCoords(xcoord, ycoord);
	//If a mob or threat present
	if (Game::map(xcoord, ycoord).getEntityType() == EntityType::MOB)
	{
		std::cout << "You are in an encounter with a " << Game::mobs[Game::map(xcoord, ycoord).getEntityID()].getName()
			<< "(Health: " << Game::mobs[Game::map(xcoord, ycoord).getEntityID()].getHealth() << ")!\n";
	}
	else if (Game::map(xcoord, ycoord).getEntityType() == EntityType::THREAT)
	{
		std::cout << "You will constantly take damage from the " << Game::threat_data[Game::map(xcoord, ycoord).getEntityID()].getName() << " if you remain here!\n";
	}
	else if (Game::map(xcoord, ycoord).getEntityType() == EntityType::NPC)
	{
		std::cout << "There is a friendly " << Game::npcs[Game::map(xcoord, ycoord).getEntityID()].get()->getName() << " that you can talk to.\n";
	}
	//Else, no entity, don't print anything
	if (Game::map(xcoord, ycoord).getItemType() == ItemType::MAGICALPOTION)
	{
		std::cout << "There is a magical potion here! You reaaaaally should want to take this.\n";
	}
	else if (Game::map(xcoord, ycoord).getItemType() != ItemType::NOTHING)
	{
		if (Game::map(xcoord, ycoord).getItemType() == ItemType::BASE)
		{
			std::cout << "There is a " << Game::base_items[Game::map(xcoord, ycoord).getItemID()].getName() << " that you may wish to pick up.\n";
		}
		else //Usable item
		{
			std::cout << "There is a " << Game::items[Game::map(xcoord, ycoord).getItemID()].getName() 
				<< "(Uses left: "<< Game::items[Game::map(xcoord, ycoord).getItemID()].getUses() << ") that you may wish to pick up.\n";
		}
	}
	//else nothing, and print nothing
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

	data_loader >> Game::map_xsize >> Game::map_ysize >> Game::max_time;
	data_loader >> Game::player_start_xcoord >> Game::player_start_ycoord >> Game::magical_potion_xcoord >> Game::magical_potion_ycoord;

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
	if (Game::player_start_xcoord < 0)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. X coordinate of player cannot be negative");
	}
	if (Game::player_start_ycoord < 0)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. Y coordinate of player cannot be negative");
	}
	if (Game::player_start_xcoord >= Game::map_xsize)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. X coordinate of player cannot be greater than or equal to size of map (0 to (xsize - 1))");
	}
	if (Game::player_start_ycoord >= Game::map_ysize)
	{
		throw std::runtime_error("Options.dat: Data is defined wrongly. Y coordinate of player cannot be greater than or equal to size of map (0 to (ysize - 1))");
	}
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
	double run_chance;
	//For loading Threat, no new unique variable needed

	while (data_loader())
	{
		data_loader >> code;
		if (code < current_code || (code - 1) > current_code)
			throw std::runtime_error("Data.dat: Incorrect code number or code order read, game data cannot be loaded.");
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

				checkDataLoaderStatus(data_loader, "Data.dat");
				BaseItem base_item(static_cast<ItemType>(item_type), name, value);
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
				//Get a name from within double quotes
				if (Game::getName(data_loader, name) < 0) //Returns -1 if error
				{
					throw std::runtime_error("Data.dat: " + data_loader.getErrorMsg());
				}

				data_loader >> min_hp_change >> max_hp_change >> uses >> success_rate >> value >> number_to_place;

				checkDataLoaderStatus(data_loader, "Data.dat");
				Item item(static_cast<ItemType>(item_type), name, min_hp_change, max_hp_change, uses, success_rate, value);
				if (!item.valid()) //Validate data, if invalid, throw (Passes size of item_data for validating inventory ids)
				{
					throw std::runtime_error("Data.dat: Item data with object_typeid " + std::to_string(object_typeid) + " is invalid. A member variable value has been defined wrongly.");
				}

				if (number_to_place >= 0)
				{
					Game::item_number_to_place.push_back(number_to_place);
				}
				else
				{
					throw std::runtime_error("Data.dat: Item data with object_typeid " + std::to_string(object_typeid) + " has an invalid number of instances to place.");
				}

				Game::item_data.push_back(item);
			}
			break;
		}
		case 1:
		{
			//Load player, code 1
			if (Game::player_data.size() > 0)
			{
				throw std::runtime_error("Data.dat: More than one player type not allowed.");
			}

			//Get a name from within double quotes
			if (Game::getName(data_loader, name) < 0)
			{
				throw std::runtime_error("Data.dat: " + data_loader.getErrorMsg());
			}

			int inventory_id[4];
			data_loader >> max_hp >> hp >> atk >> def >> exp >> level >> inventory_id[0] >> inventory_id[1] >> inventory_id[2] >> inventory_id[3] >> gold;

			checkDataLoaderStatus(data_loader, "Data.dat");
			Player player(name, max_hp, hp, atk, def, exp, level, inventory_id[0], inventory_id[1], inventory_id[2], inventory_id[3], gold);
			if (!player.valid(Game::base_item_data.size() + Game::item_data.size()))//validate data, if invalid, throw
			{
				throw std::runtime_error("Data.dat: Player data is invalid. A member variable value has been defined wrongly.");
			}

			Game::player_data.push_back(player);
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

			data_loader >> max_hp >> hp >> atk >> def >> min_dmg >> max_dmg >> exp >> level >> run_chance >> gold >> number_to_place;

			checkDataLoaderStatus(data_loader, "Data.dat");
			Mob mob(name, max_hp, hp, atk, def, min_dmg, max_dmg, exp, level, run_chance, gold, object_typeid);
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

			data_loader >> min_dmg >> max_dmg >> run_chance >> number_to_place;

			checkDataLoaderStatus(data_loader, "Data.dat");
			Threat threat(name, min_dmg, max_dmg, run_chance);
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
			throw std::runtime_error("Data.dat: Invalid code read, game data cannot be loaded.");
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

//Throws if there is an error with the data_loader
void Game::checkDataLoaderStatus(const DataLoader& data_loader, const std::string& filename) const
{
	if (data_loader.eof())
	{
		throw std::runtime_error(filename + "Data loader unexpectedly reached end of file. File is incomplete/Syntax is wrong.");
	}
	if (data_loader.fail())
	{
		throw std::runtime_error(filename + "Data loader encountered unexpected input. Data is defined wrongly/Syntax is wrong.");
	}
}

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
	int number_of_inventory_slots;
	int xcoord, ycoord;

	while (data_loader())
	{
		data_loader >> code;
		if (code < current_code || (code - 1) > current_code)
			throw std::runtime_error("NPCs.dat: Incorrect code number or code order read, game data cannot be loaded.");
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

			data_loader >> can_be_bought_from >> can_be_sold_to >> buy_price_percent >> sell_price_percent >> number_of_inventory_slots;

			Game::checkDataLoaderStatus(data_loader, "NPCs.dat");
			Merchant merchant(name, can_be_bought_from, can_be_sold_to, buy_price_percent, sell_price_percent);
			for (int i{ 0 }; i < number_of_inventory_slots; ++i)
			{
				int item_typeid;
				data_loader >> item_typeid;
				Game::checkDataLoaderStatus(data_loader, "NPCs.dat");
				merchant.addNewItemSlot(ItemType::PLACEHOLDER, item_typeid);
			}

			if (!merchant.valid(Game::base_item_data.size() + Game::item_data.size()))
			{
				throw std::runtime_error("NPCs.dat: Merchant with npc_typeid " + std::to_string(npc_typeid) + " is invalid. A member variable has been defined wrongly.");
			}

			data_loader >> xcoord >> ycoord;
			Game::checkDataLoaderStatus(data_loader, "NPCs.dat");

			if (xcoord < 0)
			{
				throw std::runtime_error("NPCs.dat: Merchant with npc_typeid " + std::to_string(npc_typeid) + " is invalid. X coordinate of NPC cannot be negative");
			}
			if (ycoord < 0)
			{
				throw std::runtime_error("NPCs.dat: Merchant with npc_typeid " + std::to_string(npc_typeid) + " is invalid. Y coordinate of NPC cannot be negative");
			}
			if (xcoord >= Game::map_xsize)
			{
				throw std::runtime_error("NPCs.dat: Merchant with npc_typeid " + std::to_string(npc_typeid) + " is invalid. X coordinate of NPC cannot be greater than or equal to size of map (0 to (xsize - 1))");
			}
			if (ycoord >= Game::map_ysize)
			{
				throw std::runtime_error("NPCs.dat: Merchant with npc_typeid " + std::to_string(npc_typeid) + " is invalid. Y coordinate of NPC cannot be greater than or equal to size of map (0 to (ysize - 1))");
			}
			merchant.setCoords(xcoord, ycoord);

			Game::npc_data.push_back(std::make_unique<Merchant>(merchant));
			break;
		}
		case 1:
		{
			//End of file

			int total_items_to_spawn = 1; //Start with magical potion counted
			int total_entities_to_spawn = 1; //Start with player counted
			int mapsize = Game::map_xsize * Game::map_ysize;
			for (auto& i : npc_data)
			{
				switch (i.get()->getNPCType())
				{
				case NPCType::MERCHANT:
				{
					Merchant* merchant = static_cast<Merchant*>(i.get());
					total_entities_to_spawn += 1;
					total_items_to_spawn += merchant->getInventorySize();
					break;
				}
				default:
				//No other NPC types implemented yet
					break;
				}
			}
			for (int i : item_number_to_place)
			{
				total_items_to_spawn += i;
			}
			if (total_items_to_spawn > mapsize)
			{
				throw std::runtime_error("Total number of items to be placed is greater than total number of tiles, game cannot be instantiated.");
			}
			for (int i : mob_number_to_place)
			{
				total_entities_to_spawn += i;
			}
			for (int i : threat_number_to_place)
			{
				total_entities_to_spawn += i;
			}
			if (total_entities_to_spawn > mapsize)
			{
				throw std::runtime_error("Total number of entities to be placed is greater than total number of tiles, game cannot be instantiated.");
			}

			//May move number of items/entities vs number of tiles check to here
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

//Places the items and entities on the map
//Should only be called ONCE, at the start of the game
void Game::placeItemsAndEntities()
{
	//Place player
	Game::map(Game::player_start_xcoord, Game::player_start_ycoord).setEntity(EntityType::PLAYER, 0); //entity_id = 0
	Game::player.push_back(Game::player_data[0]);
	Game::player[0].setCoords(Game::player_start_xcoord, Game::player_start_ycoord);
	Game::map(Game::player_start_xcoord, Game::player_start_ycoord).setExplored();
	Game::setVisibilityAround(Game::player[0], true);

	//Place magical potion
	Game::map(Game::magical_potion_xcoord, Game::magical_potion_ycoord).setItem(ItemType::MAGICALPOTION, 0);

	int xcoord, ycoord;

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

	//Generate the items in player's starting inventory
	for (int i{ 1 }; i < 5; ++i) //Inventory slot 1 to 4
	{
		if (Game::player[0].getInventorySlotItemID(i) != -1)
		{
			if (Game::player[0].getInventorySlotItemID(i) < Game::base_item_data.size()) //Is an ID for base item
			{
				Game::base_items.push_back(Game::base_item_data[Game::player[0].getInventorySlotItemID(i)]);
				Game::player[0].setInventorySlotItem(i,Game::base_items[baseitem_id].getItemType(), baseitem_id);
				++baseitem_id;
			}
			else //Is an ID for usable item
			{
				int item_typeid = Game::player[0].getInventorySlotItemID(i) - Game::base_item_data.size();
				Game::items.push_back(Game::item_data[item_typeid]);
				Game::player[0].setInventorySlotItem(i, Game::items[item_id].getItemType(), item_id);
				//Game::items[item_id].setCoords(-1, -1);
				++item_id;
			}
		}
		else //Nothing to generate in that slot
		{
			Game::player[0].setInventorySlotItem(i, ItemType::NOTHING, -1);
		}
	}

	int npc_id = 0;
	//Handle NPCs generation
	for (auto& i : Game::npc_data)
	{
		switch (i.get()->getNPCType())
		{
		case NPCType::MERCHANT:
		{
			Merchant* merchant = static_cast<Merchant*>(i.get());
			//Place Merchant and generate their items
			Game::npcs.push_back(std::make_unique<Merchant>(*merchant));
			merchant = static_cast<Merchant*>(Game::npcs[Game::npcs.size() - 1].get());
			//Generate items in merchant's inventory
			for (size_t j{ 0 }; j < merchant->getInventorySize(); ++j) //j is the inventory slot index 
			{
				if (merchant->getInventorySlotItemID(j + 1) != -1) //If item to be generated
				{
					if (merchant->getInventorySlotItemID(j + 1) < Game::base_item_data.size()) //Is an ID for base item
					{
						Game::base_items.push_back(Game::base_item_data[merchant->getInventorySlotItemID(j + 1)]);
						merchant->setItemSlot(j + 1, Game::base_items[baseitem_id].getItemType(), baseitem_id);
						++baseitem_id;
					}
					else //Is an ID for usable item
					{
						int item_typeid = merchant->getInventorySlotItemID(j + 1) - Game::base_item_data.size();
						Game::items.push_back(Game::item_data[item_typeid]);
						merchant->setItemSlot(j + 1, Game::item_data[item_typeid].getItemType(), item_id);
						//Game::items[item_id].setCoords(-1, -1);
						++item_id;
					}
				}
				else //Nothing to generate in that slot
				{
					merchant->setItemSlot(j + 1, ItemType::NOTHING, -1);
				}
			}
			Game::map(merchant->getXCoord(), merchant->getYCoord()).setEntity(EntityType::NPC, npc_id); //Do for all NPCs
			++npc_id;
		}
		default: //No other NPCs yet
			break;
		}
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
//If entity present, set to entity character, else if no entity, but object present, set to object character, else nothing
void Game::updateMapTileCharacter(int x, int y)
{
	if (Game::player[0].getXCoord() == x && Game::player[0].getYCoord() == y)
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
	else if (Game::map(x, y).getEntityType() == EntityType::NPC)
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

//Prints the amount of health the player has, and the items in his four inventory slots
void Game::printPlayerDetails() const
{
	std::cout << "Level: " << Game::player[0].getLevel() << "\t\t\tAtk: " << Game::player[0].getAtk() << "\t\tExp: " << Game::player[0].getExp() << '\n';
	std::cout << "Health: " << Game::player[0].getHealth() << '/' << Game::player[0].getMaxHealth() << "\t\t\tDef: " << Game::player[0].getDef();
	std::cout << "\t\tGold: " << Game::player[0].getGold() << '\n';
	Game::printInventory();
}

void Game::printInventory() const
{
	for (int i{ 1 }; i < 5; ++i) //Print details of inventory slots 1 - 4
	{
		if (Game::player[0].getInventorySlotItemType(i) == ItemType::MAGICALPOTION)
		{
			std::cout << "Inventory slot " << i << ":Magical Potion\t\t";
		}
		else if (Game::player[0].getInventorySlotItemType(i) != ItemType::NOTHING) //Has an item other than magical potion in that slot
		{
			if (Game::player[0].getInventorySlotItemType(i) == ItemType::BASE)
			{
				std::cout << "Inventory slot " << i << ':' << Game::base_items[Game::player[0].getInventorySlotItemID(i)].getName() << "\t\t";
			}
			else
			{
				std::cout << "Inventory slot " << i << ':' << Game::items[Game::player[0].getInventorySlotItemID(i)].getName()
					<< "(Uses left: " << Game::items[Game::player[0].getInventorySlotItemID(i)].getUses() << ")\t";
			}
		}
		else
		{
			std::cout << "Inventory slot " << i << ":NOTHING\t\t";
		}
		if (i == 2 || i == 4)
		{
			std::cout << '\n';
		}
	}
}

//Prints the position of the player
void Game::printPlayerPosition() const
{
	std::cout << Game::player[0].getName() << " is currently at (" << Game::player[0].getXCoord() << ", " << Game::player[0].getYCoord() << ")\n";
}

//Prints all the options available to the player to take in that situation
void Game::printAvailablePlayerActions() const  
{
	if (Game::game_state == GameState::ONGOING)
	{
		//m) Move	u)Use inventory(Healing items)	p)Pick up/etc	c)Check surroundings	i)Inspect	s)Save	e)Exit
		std::cout << "m)Move\nu)Use inventory\np)Pick up/swap/drop items\nc)Check surroundings\ni)Inspect items\ns)Save\ne)Exit\n";
	}
		
	else if(Game::game_state == GameState::ENCOUNTER_MOB) //Use inventory slot 1-4, swap item, run
	{
		//u)Use inventory(Healing/weapon)	p)Pick up/etc	r)Run	i)Inspect	s)Save	e)Exit
		std::cout << "u)Use inventory\np)Pick up/swap/drop items\n";
		int xcoord, ycoord;
		Game::player[0].getCoords(xcoord, ycoord);
		std::cout << "r)Run! (Chance of failure: " << Game::mobs[Game::map(xcoord, ycoord).getEntityID()].getRunChance() 
			<< "%)\ni)Inspect items\ns)Save\ne)Exit\n";
	}
	
	else if (Game::game_state == GameState::ENCOUNTER_THREAT)
	{
		//u)Use inventory(Healing)	p)Pick up/etc	r)Run	i)Inspect	s)Save	e)Exit
		std::cout << "1)Use inventory slot 1\n2)Use inventory slot 2\n3)Use inventory slot 3\n4)Use inventory slot 4\n5)Pick up/swap/drop items\n";
		int xcoord, ycoord;
		Game::player[0].getCoords(xcoord, ycoord);
		std::cout << "6)Run! (Chance of failure: " << Game::threat_data[Game::map(xcoord, ycoord).getEntityID()].getRunChance() 
			<< "%)\ni)Inspect items\ns)Save\ne)Exit\n" ;
	}
	else//if Game::game_state == GameState::ENCOUNTER_MERCHANT);
	{
		//m)Move	u)Use inventory(Healing)	p)Pick up/etc	c)Check surroundings	i)Inspect	t)Talk	s)Save	e)Exit
		std::cout << "m)Move\nu)Use inventory\np)Pick up/swap/drop items\nc)Check surroundings\ni)Inspect items\nt)Talk\ns)Save\ne)Exit\n";
	}
}

//Checks if the option entered by the player is a valid option or not and launches the appropriate sub menus if necessary
//Outputs a message if the action is invalid. Message details why option is invalid
//Note: Sets the player's action if valid
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
			Game::player[0].setAction(Action::CHECK_SURROUNDINGS);
			return true;
		case 'i':
		case 'I':
			Game::inspectMenu(); //Menu to choose which item from inventory or tile to inspect (Get details of)
			return false; //Not an actual action, prompt for one again
		case 's':
		case 'S':
			Game::player[0].setAction(Action::SAVE);
			return true;
		case 'e':
		case 'E':
			Game::player[0].setAction(Action::EXIT);
			return true;
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			return false;
		}
	}
	//else if (Game::hero.encounter == EncounterType::SHOP);
	else if (Game::game_state == GameState::ENCOUNTER_MOB)	//Use inventory slot 1, 2, 3, 4 (Weapon/Healing), swap item or run
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
			Game::player[0].setAction(Action::RUN);
			return true;
		case 'i':
		case 'I':
			Game::inspectMenu(); //Menu to choose which item from inventory or tile to inspect (Get details of)
			return false; //Not an actual action, prompt for one again
		case 's':
		case 'S':
			Game::player[0].setAction(Action::SAVE);
			return true;
		case 'e':
		case 'E':
			Game::player[0].setAction(Action::EXIT);
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
			Game::player[0].setAction(Action::RUN);
			return true;
		case 'i':
		case 'I':
			Game::inspectMenu(); //Menu to choose which item from inventory or tile to inspect (Get details of)
			return false; //Not an actual action, prompt for one again
		case 's':
		case 'S':
			Game::player[0].setAction(Action::SAVE);
			return true;
		case 'e':
		case 'E':
			Game::player[0].setAction(Action::EXIT);
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
			Game::player[0].setAction(Action::CHECK_SURROUNDINGS);
			return true;
		case 'i':
		case 'I':
			Game::inspectMenu(); //Menu to choose which item from inventory or tile to inspect (Get details of)
			return false; //Not an actual action, prompt for one again
		case 't':
		case 'T':
			Game::talkToNPCMenu();
			return false; //Not an actual action, prompt for one again
		case 's':
		case 'S':
			Game::player[0].setAction(Action::SAVE);
			return true;
		case 'e':
		case 'E':
			Game::player[0].setAction(Action::EXIT);
			return true;
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			return false;
		}
	}
}

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
			if (Game::canMoveUp(Game::player[0]))
			{
				Game::player[0].setAction(Action::MOVE_UP);
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
			if (Game::canMoveLeft(Game::player[0]))
			{
				Game::player[0].setAction(Action::MOVE_LEFT);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move left, you are at the edge of the map.\n";
				continue; //Loop this menu again
			}
		case '3': //Move right
		case 'd':
		case 'D':
			if (Game::canMoveRight(Game::player[0]))
			{
				Game::player[0].setAction(Action::MOVE_RIGHT);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move right, you are at the edge of the map.\n";
				continue; //Loop this menu again
			}
		case '4': //Move down
		case 's':
		case 'S':
			if (Game::canMoveDown(Game::player[0]))
			{
				Game::player[0].setAction(Action::MOVE_DOWN);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move down, you are at the edge of the map.\n";
				continue; //Loop this menu again
			}
		case 'b':
		case 'B':
			return false; //Return to caller, will not print invalid option (Backed by caller)
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

bool Game::useItemMenu()
{
	std::cout << "Which inventory slot's item would you like to use: 1, 2, 3 or 4? (Enter 'b' to go back)\n";
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
			//Base items have no use associated with them (Only used for selling for gold)
			if (Game::player[0].getInventorySlotItemType(user_input - 48) == ItemType::BASE)
			{
				std::cout << "Invalid option! This item has no use.\n";
				continue;
			}
			//Healing items can always be used. Weapons can only be used against mob
			else if (Game::player[0].getInventorySlotItemType(user_input - 48) == ItemType::HEALING) //Healing items can always be used
			{
				Game::player[0].setAction(static_cast<Action>(static_cast<int>(Action::INVENTORY1) + Game::user_input - 49));
				return true;
			}
			else if (Game::player[0].getInventorySlotItemType(user_input - 48) == ItemType::WEAPON) //Weapons can only be used during encounter with mob
			{
				if (Game::game_state == GameState::ONGOING) //Invalid
				{
					std::cout << "Invalid option! You can't use a weapon when there is nothing to use it against.\n";
				}
				else if (Game::game_state == GameState::ENCOUNTER_MOB) //Weapon is valid to use only during encounter with mob
				{
					Game::player[0].setAction(static_cast<Action>(static_cast<int>(Action::INVENTORY1) + Game::user_input - 49));
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
		case 'b':
		case 'B':
			return false; //Return to caller, will not print invalid option (Backed by caller)
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

//Prompts the player which inventory slot's item the player would want to use to swap items
//Swapping also applies to if either the tile or the inventory slot has no item in it (Is empty)
//Returns true if valid option for chosen slot
//Returns false if not, or if user wants to go back to previous options
bool Game::swapItemMenu()
{
	std::cout << "Pick up/swap/drop item using inventory slot: 1, 2, 3 or 4? (Enter 'b' to go back)\n";
	while(true) //Does not exit until valid option chosen or player decides to return from this menu
	{
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case '1': //Pick up/swap/drop item with inventory slot 1
		case '2': //Pick up/swap/drop item with inventory slot 2
		case '3': //Pick up/swap/drop item with inventory slot 3
		case '4': //Pick up/swap/drop item with inventory slot 4
			if (Game::map(player[0].getXCoord(), player[0].getYCoord()).getItemType() != ItemType::NOTHING ||
				Game::player[0].getInventorySlotItemType(user_input - 48) != ItemType::NOTHING) //An item either on the tile or in player's inventory, allow action
			{
				Game::player[0].setAction(static_cast<Action>(static_cast<int>(Action::SWAP_ITEM1) + (Game::user_input - 49))); //;) reducing repeated code
				return true;
			}
			else
			{
				std::cout << "Invalid option! There is nothing that can be picked up/swapped/dropped.\n";
				continue; //Loop this menu again
			}
		case 'b':
		case 'B':
			return false; //Return to caller, will not print invalid option (Backed by caller)
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

void Game::inspectMenu()
{
	while (true)
	{
		//Note: This menu is not a menu to choose a action, this is deliberately reprinted
		std::cout << "Which item would you like to inspect? The item in inventory slot 1, 2, 3, or 4?\n" 
			<< "Enter 't' to inspect the item on the current tile (Enter 'b' to go back)\n";
		std::cin >> Game::user_input;
		if (!validInput()) continue; //Loop this menu again if invalid input
		switch (Game::user_input)
		{
		case '1':
		case '2':
		case '3':
		case '4':
			if (Game::player[0].getInventorySlotItemType(user_input - 48) != ItemType::NOTHING)
			{
				if (Game::player[0].getInventorySlotItemType(user_input - 48) == ItemType::BASE) //Is a non-usable item
				{
					printItemDetails(Game::base_items[Game::player[0].getInventorySlotItemID(user_input - 48)]);
				}
				else //Is a usable item
				{
					printItemDetails(Game::items[Game::player[0].getInventorySlotItemID(user_input - 48)]);
				}
			}
			else
			{
				std::cout << "Invalid option! There is no item in that inventory slot to inspect.\n";
			}
			continue; //Loop in case player wants to read details of another item
		case 't':
		case 'T':
			if (Game::map(Game::player[0].getXCoord(), Game::player[0].getYCoord()).getItemType() != ItemType::NOTHING)
			{
				if (Game::map(Game::player[0].getXCoord(), Game::player[0].getYCoord()).getItemType() == ItemType::MAGICALPOTION)
				{
					std::cout << "\nMagical Potion details:\n"
						<< "The ultimate potion! The best! The most wondrous! Perfection!\nJust take it already.\nEnd of details.\n\n";
				}
				else if (Game::map(Game::player[0].getXCoord(), Game::player[0].getYCoord()).getItemType() == ItemType::BASE) //Is a non-usable item
				{
					printItemDetails(Game::base_items[Game::map(Game::player[0].getXCoord(), Game::player[0].getYCoord()).getItemID()]);
				}
				else //If a usable item
				{
					printItemDetails(Game::items[Game::map(Game::player[0].getXCoord(), Game::player[0].getYCoord()).getItemID()]);
				}
			}
			else
			{
				std::cout << "Invalid option! There is no item in that inventory slot to inspect.\n";
			}
			continue; //Loop in case player wants to read details of another item
		case 'b':
		case 'B':
			return; //Return to caller
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

void Game::printItemDetails(Item& item) const
{
	std::cout << "\n" << item.getName() << " details:\nType: ";
	if (item.getItemType() == ItemType::HEALING)
	{
		std::cout << "Healing\n";
	}
	else if (item.getItemType() == ItemType::WEAPON)
	{
		std::cout << "Weapon\n";
	}
	std::cout << "Min HP change: " << item.getMinHpChange()
		<< "\nMax HP change: " << item.getMaxHpChange()
		<< "\nUses: " << item.getUses()
		<< "\nSuccess rate: " << item.getSuccessRate() << '%'
		<< "\nValue: " << item.getValue()
		<< "\nEnd of details.\n\n";
}

void Game::printItemDetails(BaseItem& base_item) const
{
	std::cout << "\n" << base_item.getName() << " details:\nType: ";
	std::cout << "Trading\nValue: " << base_item.getValue() << "\nEnd of details.\n\n";
}

void Game::talkToNPCMenu()
{
	int npc_id = Game::map(Game::player[0].getXCoord(), Game::player[0].getYCoord()).getEntityID();
	switch (Game::npcs[npc_id].get()->getNPCType())
	{
	case NPCType::MERCHANT:
	{
		Merchant* merchant = static_cast<Merchant*>(Game::npcs[npc_id].get());
		while (true)
		{
			//Note: This menu is not a menu to choose an action, this is deliberately reprinted
			std::cout << merchant->getName() << ": Welcome to my shop! What can I do for you? (You have " << Game::player[0].getGold() << " gold)\n";
			if (merchant->canBeBoughtFrom())
			{
				std::cout << "p)Purchase items\n";
			}
			else
			{
				std::cout << "This merchant does not sell any items.\n";
			}
			if (merchant->CanBeSoldTo())
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
				if (merchant->canBeBoughtFrom())
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
				if (merchant->CanBeSoldTo())
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
		break;
	}
	default:
		break;
	}
}

void Game::npcBuyMenu(Merchant* merchant)
{
	while (true)
	{
		std::cout << "What would you like to buy? (You have " << Game::player[0].getGold() << " gold)\n";
		for (size_t i{ 1 }; i < (merchant->getInventorySize() + 1); ++i)//Prints all the items for sale
		{
			int item_id = merchant->getInventorySlotItemID(i);
			if (item_id == -1)
			{
				std::cout << i << ")NOTHING\n";
			}
			else
			{
				if (merchant->getInventorySlotItemType(i) == ItemType::BASE)
				{
					std::cout << i << ")" << Game::base_items[item_id].getName() << " (Cost: " << merchant->getItemBuyPrice(Game::base_items[item_id].getValue()) << " gold)\n";
				}
				else
				{
					std::cout << i << ")" << Game::items[item_id].getName() << " (Cost: " << merchant->getItemBuyPrice(Game::items[item_id].getValue()) << " gold)\n";
				}
			}
		}
		std::cout << "b)Back\n";
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
			size_t merchant_inventory_slot = Game::user_input - 48;
			if (merchant_inventory_slot > merchant->getInventorySize()) //If merchant has no such chosen inventory slot
			{
				std::cout << "Invalid option! Option is unrecognised.\n";
				continue;
			}
			if (merchant->getInventorySlotItemType(merchant_inventory_slot) != ItemType::NOTHING) //If merchant has no item in that slot
			{
				if (Game::player[0].isInventoryFull()) //Player has no free inventory slot
				{
					std::cout << "Invalid option! Your inventory is full.\n";
					continue;
				}

				ItemType item_type = merchant->getInventorySlotItemType(merchant_inventory_slot); //ItemType of item being bought
				int item_id = merchant->getInventorySlotItemID(merchant_inventory_slot); //ID of the item being bought
				if (item_type == ItemType::BASE)
				{
					int item_buy_price = merchant->getItemBuyPrice(Game::base_items[item_id].getValue()); //Price of the item being bought
					if (Game::player[0].getGold() < item_buy_price) //Player has not enough gold to afford item
					{
						std::cout << "Invalid option! You cannot afford that item.\n";
						continue;
					}
					Game::player[0].loseGold(item_buy_price);
					int player_inventory_slot = Game::player[0].getEmptyInventorySlot();
					Game::player[0].setInventorySlotItem(player_inventory_slot, Game::base_items[item_id].getItemType(), item_id);
					merchant->clearItemSlot(merchant_inventory_slot);
					std::cout << "You buy the " << Game::base_items[item_id].getName() << " for " << item_buy_price << " gold.\n";
				}
				else //Usable item
				{
					int item_buy_price = merchant->getItemBuyPrice(Game::items[item_id].getValue()); //Price of the item being bought
					if (Game::player[0].getGold() < item_buy_price) //Player has not enough gold to afford item
					{
						std::cout << "Invalid option! You cannot afford that item.\n";
						continue;
					}
					Game::player[0].loseGold(item_buy_price);
					int player_inventory_slot = Game::player[0].getEmptyInventorySlot();
					Game::player[0].setInventorySlotItem(player_inventory_slot, Game::items[item_id].getItemType(), item_id);
					merchant->clearItemSlot(merchant_inventory_slot);
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
		case 'b':
		case 'B':
			return; //Return to caller
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			continue; //Loop this menu again
		}
	}
}

void Game::npcSellMenu(Merchant* merchant)
{
	while (true)
	{
		std::cout << "What would you like to sell? (You have " << Game::player[0].getGold() << " gold)\n";
		for (int i{ 1 }; i < 5; ++i) //Print details of inventory slots 1 - 4
		{
			if (Game::player[0].getInventorySlotItemType(i) != ItemType::NOTHING) //Has an item in that slot
			{
				if (Game::player[0].getInventorySlotItemType(i) == ItemType::BASE) //If is a non-usable item
				{
					std::cout << i << ')' << Game::base_items[Game::player[0].getInventorySlotItemID(i)].getName() << "\tValue: " 
						<< merchant->getItemSellPrice(Game::base_items[Game::player[0].getInventorySlotItemID(i)].getValue()) << " gold\n";
				}
				else
				{
					std::cout << i << ')' << Game::items[Game::player[0].getInventorySlotItemID(i)].getName()
						<< "(Uses left: " << Game::items[Game::player[0].getInventorySlotItemID(i)].getUses() << ")\tValue: " 
						<< merchant->getItemSellPrice(Game::items[Game::player[0].getInventorySlotItemID(i)].getValue()) << " gold\n";
				}
			}
			else
			{
				std::cout << i << ")NOTHING\n";
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
		{
			size_t player_inventory_slot = Game::user_input - 48;
			ItemType item_type = Game::player[0].getInventorySlotItemType(player_inventory_slot);
			if (item_type != ItemType::NOTHING) //Player has an item in that slot
			{
				if (item_type == ItemType::BASE) //If non-usable item
				{
					int item_sell_price = merchant->getItemSellPrice(Game::base_items[Game::player[0].getInventorySlotItemID(player_inventory_slot)].getValue());
					Game::player[0].gainGold(item_sell_price);
					std::cout << "You sell the " << Game::base_items[Game::player[0].getInventorySlotItemID(player_inventory_slot)].getName() 
						<< " for " << item_sell_price << " gold.\n";
					Game::player[0].setInventorySlotItem(player_inventory_slot, ItemType::NOTHING, -1);
					continue;
				}
				else
				{
					int item_sell_price = merchant->getItemSellPrice(Game::items[Game::player[0].getInventorySlotItemID(player_inventory_slot)].getValue());
					Game::player[0].gainGold(item_sell_price);
					std::cout << "You sell the " << Game::items[Game::player[0].getInventorySlotItemID(player_inventory_slot)].getName() 
						<< " for " << item_sell_price << " gold.\n";
					Game::player[0].setInventorySlotItem(player_inventory_slot, ItemType::NOTHING, -1);
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
	switch (Game::player[0].getAction())
	{
	case Action::MOVE_UP:
	case Action::MOVE_LEFT:
	case Action::MOVE_RIGHT:
	case Action::MOVE_DOWN:
	{
		Game::playerMove(Game::player[0]);
		Game::evaluatePossibleEncounter(); //Handles need_update_map
		Game::advanceTime(1);
		break;
	}
	case Action::INVENTORY1:
	case Action::INVENTORY2:
	case Action::INVENTORY3:
	case Action::INVENTORY4:
	{
		//Converts INVENTORY1 to 1, INVENTORY4 to 4
		int inventory_slot_number = static_cast<int>(Game::player[0].getAction()) - static_cast<int>(Action::INVENTORY1) + 1;
		if (Game::player[0].getInventorySlotItemType(inventory_slot_number) == ItemType::HEALING)
		{
			Game::useHealingItemSlot(inventory_slot_number); //Handles need_update_map
			Game::advanceTime(0.25);
		}
		else //if( Game::player[0].getInventorySlotItemType(inventory_slot_number) == ItemType::WEAPON)
		{	//If is weapon, and execution reached here, that means player is definitely in encounter with mob and not in any other game state
			Game::useWeaponItemSlot(inventory_slot_number); //Handles need_update_map
			Game::advanceTime(0.25);
			//use weapon against mob
		}
		break;
	}
	case Action::SWAP_ITEM1: //Need cases for other 3
	case Action::SWAP_ITEM2:
	case Action::SWAP_ITEM3:
	case Action::SWAP_ITEM4:
	{
		//Converts SWAP_ITEM1 to 1, SWAP_ITEM4 to 4
		int inventory_slot_number = static_cast<int>(Game::player[0].getAction()) - static_cast<int>(Action::SWAP_ITEM1) + 1;
		//Store item held by player temporarily
		int inventory_item_id = Game::player[0].getInventorySlotItemID(inventory_slot_number);
		ItemType inventory_item_type = Game::player[0].getInventorySlotItemType(inventory_slot_number);
		int xcoord, ycoord;
		Game::player[0].getCoords(xcoord, ycoord);
		Game::player[0].setInventorySlotItem(inventory_slot_number, Game::map(xcoord, ycoord).getItemType(), Game::map(xcoord, ycoord).getItemID());
		Game::map(xcoord, ycoord).setItem(inventory_item_type, inventory_item_id);
		Game::event_message_handler.addEventMsg("Successfully picked up/swapped/dropped items.");
		if (Game::game_state == GameState::ONGOING)
		{ 
			Game::need_update_map = true;
		}
		else //In any encounter
		{
			Game::need_update_map = false;
		}
		Game::advanceTime(0.5);
		break;
	}
	case Action::CHECK_SURROUNDINGS:
		Game::checkSurroundings(Game::player[0]);
		Game::event_message_handler.addEventMsg("You checked your surroundings carefully...");
		Game::need_update_map = true;
		Game::advanceTime(3);
		break;
	case Action::SAVE:
		Game::game_state = GameState::SAVING;
		break;
	case Action::EXIT:
		Game::game_state = GameState::EXITING;
		break;
	case Action::RUN:
	{
		int xcoord, ycoord;
		Game::player[0].getCoords(xcoord, ycoord);
		int entity_id = Game::map(xcoord, ycoord).getEntityID();
		if (Game::map(xcoord, ycoord).getEntityType() == EntityType::MOB)
		{
			if (Game::player[0].runFrom(Game::mobs[entity_id])) //If run successful
			{
				Game::event_message_handler.addEventMsg("You ran away successfully.");
				playerMoveRandomDirection(Game::player[0]);
				evaluatePossibleEncounter(); //Handles need_update_map
				advanceTime(1);
			}
			else
			{
				Game::event_message_handler.addEventMsg("You failed to run away!");
				Game::need_update_map = false;
			}
		}
		else if (Game::map(xcoord, ycoord).getEntityType() == EntityType::THREAT)
		{
			if (Game::player[0].runFrom(Game::threat_data[entity_id])) //If run successful
			{
				Game::event_message_handler.addEventMsg("You ran away successfully.");
				playerMoveRandomDirection(Game::player[0]);
				evaluatePossibleEncounter(); //Handles need_update_map
				advanceTime(1);
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

//checked for now
//Evaluates if there is a mob or threat on the tile the player has moved to and sets encounter appropriately if true
//Will set if the game needs to update the map
void Game::evaluatePossibleEncounter()
{
	EntityType entity_on_tile_type = Game::map(player[0].getXCoord(), player[0].getYCoord()).getEntityType();
	int entity_id = Game::map(player[0].getXCoord(), player[0].getYCoord()).getEntityID();
	if (entity_on_tile_type == EntityType::MOB)
	{
		Game::game_state = GameState::ENCOUNTER_MOB;
		Game::first_time_in_encounter_mob = true;
		Game::event_message_handler.addEventMsg("You have encountered a " + Game::mobs[entity_id].getName() + "! Prepare yourself!");
		//encounter msg
		Game::need_update_map = false;
	}
	else if (entity_on_tile_type == EntityType::THREAT)
	{
		Game::game_state = GameState::ENCOUNTER_THREAT;
		Game::event_message_handler.addEventMsg("You have encountered a " + Game::threat_data[entity_id].getName() + "! Run!");
		Game::need_update_map = false;
	}
	else if (entity_on_tile_type == EntityType::NPC)
	{
		//For now, only npc is merchant
		Game::game_state = GameState::ENCOUNTER_NPC;
		if (Game::npcs[entity_id].get()->getNPCType() == NPCType::MERCHANT)
		{
			Game::event_message_handler.addEventMsg("You have encountered a friendly " + Game::npcs[entity_id].get()->getName() + ". You can buy/sell items!");
		}
		Game::need_update_map = true;
	}
	else //(entity_on_tile_type == EntityType::NOTHING)
	{
		Game::game_state = GameState::ONGOING;
		Game::need_update_map = true;
	}
}


//Uses item (Confirmed ItemType::HEALING) in the inventory slot to heal player
void Game::useHealingItemSlot(int inventory_slot_number)
{
	int item_id = Game::player[0].getInventorySlotItemID(inventory_slot_number);

	double success_rate = static_cast<int>(Game::items[item_id].getSuccessRate() * 100'000);
	int random_number = getRandomInt(0, 10'000'000);
	Game::event_message_handler.addEventMsg("You rolled " + std::to_string(random_number) + " out of 10'000'000");
	if (random_number < success_rate) //If random number is within success rate, successful at using item
	{
		int heal_amount = getRandomInt(Game::items[item_id].getMinHpChange(), Game::items[item_id].getMaxHpChange());
		Game::player[0].heal(heal_amount);
		Game::event_message_handler.addEventMsg("You heal for " + std::to_string(heal_amount) + " points of health");
	}
	else
	{
		Game::event_message_handler.addEventMsg("You tried to use the " + Game::items[item_id].getName() + " but you failed!");
	}
	Game::items[item_id].decrementUses();
	if (Game::items[item_id].getUses() == 0)
	{
		//Manage used up items
		Game::player[0].setInventorySlotItem(inventory_slot_number, ItemType::NOTHING, -1);
		logUsedItem(item_id);
		Game::event_message_handler.addEventMsg("The " + Game::items[item_id].getName() + " has been used up.");
	}
	Game::need_update_map = false;
}


//Uses item (Confirmed ItemType::WEAPON) in the inventory slot to attack mob
void Game::useWeaponItemSlot(int inventory_slot_number)
{
	int item_id = Game::player[0].getInventorySlotItemID(inventory_slot_number);

	double success_rate = static_cast<int>(Game::items[item_id].getSuccessRate() * 100'000);
	int random_number = getRandomInt(0, 10'000'000);
	Game::event_message_handler.addEventMsg("You rolled " + std::to_string(random_number) + " out of 10'000'000");
	if (random_number < success_rate) //If random number is within success rate, successful at using item
	{
		int expected_damage = getRandomInt(Game::items[item_id].getMinHpChange(), Game::items[item_id].getMaxHpChange());
		int xcoord, ycoord;
		Game::player[0].getCoords(xcoord, ycoord);
		int entity_id = Game::map(xcoord, ycoord).getEntityID();

		int actual_damage;
		if (Game::player[0].getAtk() > Game::mobs[entity_id].getDef() || Game::player[0].getAtk() < Game::mobs[entity_id].getDef())
		{
			actual_damage = static_cast<int>(static_cast<double>(Game::player[0].getAtk()) / Game::mobs[entity_id].getDef() * expected_damage);
		}
		else //if (Game::player[0].getAtk() == Game::mobs[entity_id].getDef())
		{
			actual_damage = expected_damage; //Damage amount does not change
		}

		int damage_discrepency = actual_damage - expected_damage;

		Game::mobs[entity_id].takeDamage(actual_damage);
		Game::event_message_handler.addEventMsg("You attack the " + Game::mobs[entity_id].getName() + " for " + std::to_string(actual_damage) + " points of damage!");
		if (damage_discrepency < 0) //Took less damage than expected
		{
			Game::event_message_handler.addEventMsg(Game::mobs[entity_id].getName() + " blocked " + std::to_string(damage_discrepency) + " points of damage.");
		}
		else if (damage_discrepency > 0)
		{
			Game::event_message_handler.addEventMsg("You greater attack allowed you to deal " + std::to_string(damage_discrepency) + " more points of damage.");
		}

		if (Game::mobs[entity_id].isDead())
		{
			Game::game_state = GameState::ONGOING;
			Game::map(xcoord, ycoord).setEntity(EntityType::PLAYER, 0);
			updateMapTileCharacter(xcoord, ycoord);
			logDeadMob(entity_id);
			Game::event_message_handler.addEventMsg("The " + Game::mobs[entity_id].getName() + " dies!");
			int player_initial_level = Game::player[0].getLevel();
			Game::player[0].gainExp(Game::mobs[entity_id].getExp()); //Note: Levelling implementation has to be relooked in the future
			Game::player[0].gainGold(Game::mobs[entity_id].getGold());
			Game::event_message_handler.addEventMsg("You gain " + std::to_string(Game::mobs[entity_id].getExp()) + " EXP and " 
				+ std::to_string(Game::mobs[entity_id].getGold()) + " gold");
			if (Game::player[0].getLevel() > player_initial_level)
			{
				Game::event_message_handler.addEventMsg("You levelled up!"); //Has to be reworked in the future
			}
			Game::need_update_map = true;
		}
	}
	else //Use item failed
	{
		Game::event_message_handler.addEventMsg("You tried to use the " + Game::items[item_id].getName() + " but you failed!");
		Game::need_update_map = false;
	}

	Game::items[item_id].decrementUses(); //Success or not, item's number of uses left will be decremented
	if (Game::items[item_id].getUses() == 0)
	{
		//Manage used up items
		Game::player[0].setInventorySlotItem(inventory_slot_number, ItemType::NOTHING, -1);
		logUsedItem(item_id);
		Game::event_message_handler.addEventMsg("The " + Game::items[item_id].getName() + " has been used up.");
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
			valid_move = canMoveUp(Game::player[0]);
			if(valid_move)
				Game::player[0].setAction(Action::MOVE_UP);
			break;
		case 2:
			valid_move = canMoveLeft(Game::player[0]);
			if (valid_move)
				Game::player[0].setAction(Action::MOVE_LEFT);
			break;
		case 3:
			valid_move = canMoveRight(Game::player[0]);
			if (valid_move)
				Game::player[0].setAction(Action::MOVE_RIGHT);
			break;
		case 4:
			valid_move = canMoveDown(Game::player[0]);
			if (valid_move)
				Game::player[0].setAction(Action::MOVE_DOWN);
			break;
		}
	} while (valid_move == false);
	playerMove(Game::player[0]);
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
	//else
	if (Game::game_state == GameState::ENCOUNTER_MOB)
	{
		if (!Game::first_time_in_encounter_mob) //Note: If it is actually the first time, we want to do nothing as the mob should not attack yet
		{
			int entity_id = Game::map(Game::player[0].getXCoord(), Game::player[0].getYCoord()).getEntityID();
			int damage_amount = getRandomInt(Game::mobs[entity_id].getMinDmg(), Game::mobs[entity_id].getMaxDmg());
			Game::player[0].takeDamage(damage_amount);
			Game::event_message_handler.addEventMsg("The " + Game::mobs[entity_id].getName() + " attacks you for " + std::to_string(damage_amount) + " points of damage!");
		}
		Game::first_time_in_encounter_mob = false;
	}
	else if (Game::game_state == GameState::ENCOUNTER_THREAT) //Note: Threat will always attack
	{
		int entity_id = Game::map(Game::player[0].getXCoord(), Game::player[0].getYCoord()).getEntityID();
		int damage_amount = getRandomInt(Game::threat_data[entity_id].getMinDmg(), Game::threat_data[entity_id].getMaxDmg());
		Game::player[0].takeDamage(damage_amount);
		Game::event_message_handler.addEventMsg("The " + Game::threat_data[entity_id].getName() + " causes you to lose " + std::to_string(damage_amount) + " points of health!");
	}

	if (playerDied())
		Game::game_state = GameState::LOST;
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
	int whole_time_to_pass = static_cast<int>(time_to_pass); //Hold the whole number part of time_to_pass
	int decimal_time_to_pass = static_cast<int>(time_to_pass * 100) - whole_time_to_pass * 100; //Hold the decimal part of time_to_pass

	Game::event_message_handler.addEventMsg(std::to_string(whole_time_to_pass) + '.' + std::to_string(decimal_time_to_pass) + " Days have passed...");
}

//Checks if there is no time left to find the magical potion
//Returns true if no time left
//Returns false if there is still time
bool Game::noTimeLeft() const
{
	if (Game::time_left == 0)
	{
		return true;
	}
	return false;
}

//Checks if the player has died
//Returns true if so
//Returns false if not
bool Game::playerDied() const
{
	if (Game::player[0].isDead())
		return true;
	//else
	return false;
}

//Checks if the player has the magical potion in any of his four inventory slots
//Returns true if so
//Returns false if not
bool Game::playerHasMagicalPotion() const
{
	for (int i{ 0 }; i < 4; ++i)
	{
		if (Game::player[0].getInventorySlotItemType(i) == ItemType::MAGICALPOTION)
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

//Checks if an enttiy can move right
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