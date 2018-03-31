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
		//printAvailablePlayerActions();

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
						Game::printMap();
						Game::printTimeLeft();	//Reprint
						Game::printPlayerDetails();
						Game::printObjectsOnPlayerTileDetails();
						Game::printAvailablePlayerActions();
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
	Game::mobs.clear();
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
		std::cout << "You will constantly take damage from a " << Game::threat_data[Game::map(xcoord, ycoord).getEntityID()].getName() << "if you remain here!\n";
	}
	if (Game::map(xcoord, ycoord).getItemType() == ItemType::MAGICALPOTION)
	{
		std::cout << "There is a magical potion here! You reaaaaally should want to take this.\n";
	}
	else if (Game::map(xcoord, ycoord).getItemType() != ItemType::NOTHING)
	{
		std::cout << "There is a " << Game::items[Game::map(xcoord, ycoord).getItemID()].getName() 
			<< "(No. of uses left: "<< Game::items[Game::map(xcoord, ycoord).getItemID()].getUses() << ") that you may wish to pick up.\n";
	}
}

//Loads data for items, mobs and threats from Data.dat into a vector (For new entities to be copy constructed from)
//Throws std::runtime_error (Should be caught by std::exception handler)
void Game::loadItemsAndEntities()
{
	DataLoader data_loader{ "Data.dat" }; //rename
	if (!data_loader.is_open())
	{
		throw std::runtime_error("Unable to load game data, please make sure Data.dat is in the same folder.");
	}
	
	int code{ -1 }, current_code{ 0 };
	int object_typeid{ 0 };

	//For loading every entity (For assets, set object_id to be -1 as they are not intended to be used as actual in-game objects, but as assets)
	int object_id{ -1 };
	int number_to_place;
	//For loading Item
	int item_type;
	std::string name;
	int min_hp_change, max_hp_change, uses;
	double success_rate;
	//For loading Entity
	int max_hp, hp, def, min_dmg, max_dmg, exp, level;
	double run_chance;
	//For loading Threat, no new unique variable needed

	while (data_loader())
	{
		data_loader >> code;
		if (code < current_code || (code - 1) > current_code)
			throw std::runtime_error("Data.dat: Incorrect code number or order read, game data cannot be loaded.");
		if (code > current_code)
		{
			/*if (current_code == 0)
				Game::last_itemID = identificationID - 1;
			if (current_code == 1)
				Game::last_mobID = identificationID - 1;
			if (current_code == 2)
				Game::last_threatID = identificationID - 1;*/
			object_typeid = 0;
			current_code = code;
		}
		switch (current_code)
		{
		case 0:
		{
			//Load item, code 0
			//Get a name from within double quotes
			getName(data_loader, name);

			data_loader >> item_type >> min_hp_change >> max_hp_change >> uses >> success_rate >> number_to_place;

			checkDataLoaderStatus(data_loader);
			Item item(name, static_cast<ItemType>(item_type), min_hp_change, max_hp_change, uses, success_rate, object_typeid, object_id);
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
			getName(data_loader, name);

			int inventory_id[4];
			data_loader >> max_hp >> hp >> def >> exp >> level >> inventory_id[0] >> inventory_id[1] >> inventory_id[2] >> inventory_id[3];

			checkDataLoaderStatus(data_loader);
			Player player(name, max_hp, hp, def, exp, level, inventory_id[0], inventory_id[1], inventory_id[2], inventory_id[3]);
			if (!player.valid(Game::item_data.size()))//validate data, if invalid, throw
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
			getName(data_loader, name);

			data_loader >> max_hp >> hp >> def >> min_dmg >> max_dmg >> exp >> level >> run_chance >> number_to_place;

			checkDataLoaderStatus(data_loader);
			Mob mob(name, max_hp, hp, def, min_dmg, max_dmg, exp, level, run_chance, object_typeid, object_id);
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
			getName(data_loader, name);

			data_loader >> min_dmg >> max_dmg >> run_chance >> number_to_place;
			
			checkDataLoaderStatus(data_loader);
			Threat threat(name, min_dmg, max_dmg, run_chance, object_typeid, object_id);
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
		{
			int total_items_to_spawn = 1; //Start with magical potion counted
			int total_entities_to_spawn = 1; //Start with player counted
			int mapsize = Game::map.getXSize() * Game::map.getYSize();
			for (int i : item_number_to_place)
			{
				total_items_to_spawn += i;
			}
			if (total_items_to_spawn > mapsize)
			{
				throw std::runtime_error("Data.dat: Total number of items to be placed is greater than total number of tiles, game cannot be instantiated.");
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
				throw std::runtime_error("Data.dat: Total number of entities to be placed is greater than total number of tiles, game cannot be instantiated.");
			}

			//End of file, success load
			data_loader.close();
			return;
		}
		default:
		{
			throw std::runtime_error("Data.dat: Invalid code read, game data cannot be loaded.");
		}
		}
		++object_typeid;
	}
	throw std::runtime_error("Data.dat: Data loader has failed, game data cannot be loaded.");
}

//Gets a name defined withint double quotes from a data file
void Game::getName(DataLoader& data_loader, std::string& name)
{
	if (name.size() != 0)
	{
		name.clear();
	}
	if (data_loader.getWithinQuotes(name) < 0)
	{ //Error occurred
		throw std::runtime_error("Data.dat: " + data_loader.getErrorMsg());
	}
}

void Game::checkDataLoaderStatus(const DataLoader& data_loader) const
{
	if (data_loader.eof())
	{
		throw std::runtime_error("Data.dat: Data loader unexpectedly reached end of file. File is incomplete/Syntax is wrong.");
	}
	if (data_loader.fail())
	{
		throw std::runtime_error("Data.dat: Data loader encountered unexpected input. Data is defined wrongly/Syntax is wrong.");
	}
}

//Places the items and entities on the map
//Should only be called ONCE, at the start of the game
void Game::placeItemsAndEntities()
{
	//Place player
	Game::map(0, 0).setEntity(EntityType::PLAYER, 0); //entity_id = 0
	Game::player.push_back(Game::player_data[0]);
	Game::player[0].setCoords(0, 0);
	Game::map(0, 0).setExplored();
	Game::setVisibilityAround(Game::player[0], true);

	//Place magical potion
	Game::map(9, 5).setItem(ItemType::MAGICALPOTION, 0);

	int xcoord, ycoord;
	int item_id = 0;

	//Place items
	for (size_t i{ 0 }; i < Game::item_number_to_place.size(); ++i) //i is an alias for an object_typeid, which refers to the ith element in object_data
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
			Game::items.push_back(Game::item_data[Game::player[0].getInventorySlotItemID(i)]);
			Game::player[0].setInventorySlotItem(i, item_id, Game::item_data[Game::player[0].getInventorySlotItemID(i)].getItemType());
			Game::items[item_id].setCoords(-1, -1);
			++item_id;
		}
		else
		{
			Game::player[0].setInventorySlotItem(i, -1, ItemType::NOTHING);
		}
	}

	//Reset object_id, this should be unique per type of object (ITEM, MOB, THREAT)
	int entity_id = 0;
	//Place Mobs
	for (size_t i{ 0 }; i < Game::mob_number_to_place.size(); ++i) //i is an alias for an object_typeid, which refers to the ith element in object_data
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
	//Reset object_id, this should be unique per type of object (ITEM, MOB, THREAT)
	entity_id = 0;
	//Place Threats
	for (size_t i{ 0 }; i < Game::threat_number_to_place.size(); ++i) //i is an alias for an object_typeid, which refers to the ith element in object_data
	{
		int number_to_place = Game::threat_number_to_place[i];
		for (; number_to_place > 0; --number_to_place, ++entity_id) //Generate and place all required number of objects of that specific threat object
		{
			Game::map.getRandomTileWithoutEntityCoords(xcoord, ycoord);
			Game::map(xcoord, ycoord).setEntity(EntityType::THREAT, i); //Threats are a special case, refer directly to object_typeid
			//Game::threats.push_back(Game::threat_data[i]); threats data never change
			//Game::threats[object_id].setCoords(xcoord, ycoord); Threats dont store their own position
		}
	}
}

//Updates the character that will be printed to the screen when the tile is printed
//Should be used when a change to the EntityType/ObjectType on that tile is made
//If entity present, set to entity character, else if no entity, but object present, set to object character, else nothing
void Game::updateMapTileCharacter(int x, int y)
{
	
	if (Game::map(x,y).getEntityType() == EntityType::MOB)
	{
		Game::map(x, y).setCharacter('M');
	}
	else if (Game::map(x, y).getEntityType() == EntityType::THREAT)
	{
		Game::map(x, y).setCharacter('T');
	}
	else if(Game::map(x, y).getEntityType() == EntityType::PLAYER)
	{
		Game::map(x, y).setCharacter('P');
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
	std::cout << "Time passed: " << Game::current_time << "days\t" << "Amount of time left: " << Game::time_left << std::endl;
}

//Prints the amount of health the player has, and the items in his four inventory slots
void Game::printPlayerDetails() const
{
	std::cout << "Health: " << Game::player[0].getHealth() << std::endl;
	for (int i{ 1 }; i < 5; ++i) //Print details of inventory slots 1 - 4
	{
		if (Game::player[0].getInventorySlotItemType(i) != ItemType::NOTHING)
		{
			std::cout << "Inventory slot " << i << ": " << Game::items[Game::player[0].getInventorySlotItemID(i)].getName()
				<< "(No. of uses left: " << Game::items[Game::player[0].getInventorySlotItemID(i)].getUses() << ")\t";
		}
		else
		{
			std::cout << "Inventory slot " << i << ": NOTHING\t\t\t";
		}
		if (i == 2 || i == 4)
		{
			std::cout << '\n';
		}
	}
	//update with object id (item only) (??? 14/1)
}

//Prints the position of the player
void Game::printPlayerPosition() const
{
	std::cout << Game::player[0].getName() << " is currently at (" << Game::player[0].getXCoord() << ", " << Game::player[0].getYCoord() << ")\n";
}

//Prints all the options available to the player to take in that situation
void Game::printAvailablePlayerActions() const  
{
	if (Game::game_state == GameState::ENCOUNTER_MOB) //Use inventory slot 1-4, swap item, run
	{
		std::cout << "1)Use inventory slot 1\n2)Use inventory slot 2\n3)Use inventory slot 3\n4)Use inventory slot 4\n5)Pick up/swap/drop items\n";
		int xcoord, ycoord;
		Game::player[0].getCoords(xcoord, ycoord);
		std::cout << "6)Run! (Chance of failure: " << Game::mobs[Game::map(xcoord, ycoord).getEntityID()].getRunChance() << "%)\ns)Save\ne)Exit\n";
	}
	//if Game::game_state == GameState::ENCOUNTER_SHOP);
	else if (Game::game_state == GameState::ENCOUNTER_THREAT)
	{
		std::cout << "1)Use inventory slot 1\n2)Use inventory slot 2\n3)Use inventory slot 3\n4)Use inventory slot 4\n5)Pick up/swap/drop items\n";
		int xcoord, ycoord;
		Game::player[0].getCoords(xcoord, ycoord);
		std::cout << "6)Run! (Chance of failure: " << Game::threat_data[Game::map(xcoord, ycoord).getEntityID()].getRunChance() << "%)\ns)Save\ne)Exit\n" ;
	}
	else //Game::game_state == GameState::ONGOING
	{
		std::cout << "1)Move up\n" << "2)Move left\n" << "3)Move right\n" << "4)Move down\n";
		std::cout << "5)Use inventory slot 1\n" << "6)Use inventory slot 2\n" << "7)Use inventory slot 3\n" << "8)Use inventory slot 4\n";
		std::cout << "9)Check surroundings\n" << "0)Pick up/swap/drop items\n" << "s)Save\n" << "e)Exit\n";
	}
}

//Checks if the option entered by the player is a valid option or not
//Outputs a message if the action is invalid. Message details why option is invalid
//Note: Sets the player's action if valid
bool Game::isPlayerActionValid()
{
	if(Game::game_state == GameState::ONGOING) //Map movement, inventory, check surroundings, save, exit
	{
		switch (Game::user_input)
		{
		case '1': //Move up
			if (Game::canMoveUp(Game::player[0]))
			{
				Game::player[0].setAction(Action::MOVE_UP);
				return true;
			}
			else
			{
				std::cout << "Invalid option! You cannot move up, you are at the edge of the map.\n";
				return false;
			}
		case '2': //Move left
			if (Game::canMoveLeft(Game::player[0]))
			{
				Game::player[0].setAction(Action::MOVE_LEFT);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move left, you are at the edge of the map.\n";
				return false;
			}
		case '3': //Move right
			if (Game::canMoveRight(Game::player[0]))
			{
				Game::player[0].setAction(Action::MOVE_RIGHT);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move right, you are at the edge of the map.\n";
				return false;
			}
		case '4': //Move down
			if (Game::canMoveDown(Game::player[0]))
			{
				Game::player[0].setAction(Action::MOVE_DOWN);
				return true;
			}
			{
				std::cout << "Invalid option! You cannot move down, you are at the edge of the map.\n";
				return false;
			}
		case '5': //Use inventory slot 1
		case '6': //Use inventory slot 2
		case '7': //Use inventory slot 3
		case '8': //Use inventory slot 4
			if (Game::player[0].getInventorySlotItemType(user_input - 52) == ItemType::HEALING) //Accept only healing items
			{
				Game::player[0].setAction(static_cast<Action>(static_cast<int>(Action::INVENTORY1) + Game::user_input - 53));
				return true;
			}
			else if (Game::player[0].getInventorySlotItemType(user_input - 52) == ItemType::WEAPON)
			{
				std::cout << "Invalid option! You can't use a weapon when there is nothing to use it against.\n";
				return false;
			}
			else //Nothing in that inventory slot
			{
				std::cout << "Invalid option! There is nothing in that inventory slot to use!\n";
				return false;
			}
		case '9': //Check surroundings (Always possible when not in encounter)
			Game::player[0].setAction(Action::CHECK_SURROUNDINGS);
			return true;
		case '0': //Go to swap item menu
			return Game::isSwapItemCaseValid(); //Will output its own invalid option message
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
		case '1': //Use inventory1 (Weapon or healing item)
		case '2':
		case '3':
		case '4':
			if (Game::player[0].getInventorySlotItemType(user_input - 48) != ItemType::NOTHING) //-48 turns '1' to 1
			{
				Game::player[0].setAction(static_cast<Action>(static_cast<int>(Action::INVENTORY1) + Game::user_input - 49));
				return true;
			}
			else
			{
				std::cout << "Invalid option! There is nothing in that inventory slot to use!\n";
				return false;
			}
		case '5': //Go to swap item menu
			return Game::isSwapItemCaseValid(); //Will output its own invalid option message
		case '6': //Run
			Game::player[0].setAction(Action::RUN);
			return true;
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
	else //if (Game::game_state == GameState::ENCOUNTER_THREAT)
	{
		switch (Game::user_input)
		{
		case '1': //Use inventory1
		case '2':
		case '3':
		case '4':
			if (Game::player[0].getInventorySlotItemType(user_input - 48) == ItemType::HEALING) //Accept only healing items
			{
				Game::player[0].setAction(static_cast<Action>(static_cast<int>(Action::INVENTORY1) + Game::user_input - 49));
				return true;
			}
			else if (Game::player[0].getInventorySlotItemType(user_input - 52) == ItemType::WEAPON)
			{
				std::cout << "Invalid option! You can't use a weapon against a threat.\n";
				return false;
			}
			else //Nothing in that inventory slot
			{
				std::cout << "Invalid option! There is nothing in that inventory slot to use!\n";
				return false;
			}
		case '5': //Go to swap item menu
			return Game::isSwapItemCaseValid(); //Will output its own invalid option message
		case '6': //Run
			Game::player[0].setAction(Action::RUN);
			return true;
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

//Prompts the player which inventory slot's item the player would want to use to swap items
//Swapping also applies to if either the tile or the inventory slot has no item in it (Is empty)
//Returns true if valid option for chosen slot
//Returns false if not, or if user wants to go back to previous options
bool Game::isSwapItemCaseValid()
{
	for (;;)
	{
		std::cout << "Pick up/swap/drop item using inventory slot: 1, 2, 3 or 4? (Enter 'b' to go back)\n";
		std::cin >> Game::user_input;
		switch (Game::user_input)
		{
		case '1': //Pick up/swap/drop item with inventory slot 1
		case '2': //Pick up/swap/drop item with inventory slot 2
		case '3': //Pick up/swap/drop item with inventory slot 3
		case '4': //Pick up/swap/drop item with inventory slot 4
		{
			if (Game::map(player[0].getXCoord(), player[0].getYCoord()).getItemType() == ItemType::NOTHING && 
				Game::player[0].getInventorySlotItemType(user_input - 48) == ItemType::NOTHING) //No item on both the tile nor in the chosen inventory slot
			{
				std::cout << "Invalid option! There is nothing that can be picked up/swapped/dropped.\n";
				return false;
			}
			else
			{
				Game::player[0].setAction(static_cast<Action>(static_cast<int>(Action::SWAP_ITEM1) + (Game::user_input - 49))); //;) reducing repeated code
				return true;
			}
		}
		case 'b':
		case 'B':
			return false; //But will not print invalid option (Backed by caller)
		default:
			std::cout << "Invalid option! Option is unrecognised.\n";
			return false;
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
		Game::player[0].setInventorySlotItem(inventory_slot_number, Game::map(xcoord, ycoord).getItemID(), Game::map(xcoord, ycoord).getItemType());
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
	else //(entity_on_tile_type == EntityType::NOTHING)
	{
		Game::game_state = GameState::ONGOING;
		Game::need_update_map = true;
	}
}


//Uses item (Confirmed ObjectType::HEALING) in the inventory slot to heal player
void Game::useHealingItemSlot(int inventory_slot_number)
{
	int item_id = Game::player[0].getInventorySlotItemID(inventory_slot_number);
	int heal_amount = getRandomInt(Game::items[item_id].getMinHpChange(), Game::items[item_id].getMaxHpChange());
	Game::player[0].heal(heal_amount);
	Game::event_message_handler.addEventMsg("You heal for " + std::to_string(heal_amount) + " points of health");
	Game::items[item_id].decrementUses();
	if (Game::items[item_id].getUses() == 0)
	{
		//Manage used up items
		Game::player[0].setInventorySlotItem(inventory_slot_number, -1, ItemType::NOTHING);
		logUsedItem(item_id);
		Game::event_message_handler.addEventMsg("Your item has been used up.");
	}
	Game::need_update_map = false;
}

void Game::useWeaponItemSlot(int inventory_slot_number)
{
	int item_id = Game::player[0].getInventorySlotItemID(inventory_slot_number);
	int damage_amount = getRandomInt(Game::items[item_id].getMinHpChange(), Game::items[item_id].getMaxHpChange());
	int xcoord, ycoord;
	Game::player[0].getCoords(xcoord, ycoord);
	int entity_id = Game::map(xcoord, ycoord).getEntityID();
	Game::mobs[entity_id].takeDamage(damage_amount);
	Game::event_message_handler.addEventMsg("You attack the " + Game::mobs[entity_id].getName() + " for " + std::to_string(damage_amount) + " points of damage!");
	Game::items[item_id].decrementUses();
	if (Game::items[item_id].getUses() == 0)
	{
		//Manage used up items
		Game::player[0].setInventorySlotItem(inventory_slot_number, -1, ItemType::NOTHING);
		logUsedItem(item_id);
		Game::event_message_handler.addEventMsg("Your item has been used up.");
	}
	if (Game::mobs[entity_id].isDead())
	{
		Game::game_state = GameState::ONGOING;
		Game::map(xcoord, ycoord).setEntity(EntityType::PLAYER, 0);
		logDeadMob(entity_id);
		Game::event_message_handler.addEventMsg("The " + Game::mobs[entity_id].getName() + " dies!");
		Game::need_update_map = true;
		return;
	}
	Game::need_update_map = false;
}

//Moves the player as well as update surrounding tiles visibility and whether they are explored by the player
void Game::playerMove(Player& player)
{
	setVisibilityAround(player, false);
	int xcoord, ycoord;
	player.getCoords(xcoord, ycoord);
	if (Game::map(xcoord, ycoord).getEntityType() == EntityType::PLAYER) //Tile player is on is not occupied by any other entity (Player isn't running)
		Game::map(xcoord, ycoord).setEntity(EntityType::NOTHING, -1); //Can safely set tile being moved away from to hold no entity

	updateMapTileCharacter(xcoord, ycoord);
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
	player.getCoords(xcoord, ycoord);
	if (Game::map(xcoord, ycoord).getEntityType() == EntityType::NOTHING) //Will not have encounter
		Game::map(xcoord, ycoord).setEntity(EntityType::PLAYER, 0);

	updateMapTileCharacter(xcoord, ycoord);
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
	if (entity.getXCoord() != Game::map.getXSize())
		return true;
	return false;
}

//Checks if an entity can move down
bool Game::canMoveDown(const Entity& entity) const
{
	if (entity.getYCoord() != Game::map.getYSize())
		return true;
	return false;
}