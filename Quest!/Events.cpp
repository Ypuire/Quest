#include "Events.h"

void EventList::loadEvents(const char* filename)
{
	DataLoader data_loader{ filename };

	if (!data_loader.is_open())
	{
		throw std::runtime_error(std::string(filename) + ": Unable to load events, please make sure that the event file is in the same folder as the executable");
	}

	int code{ -1 }, current_code{ 0 };
	int object_typeid{ 0 };
	std::string event_message;
	while (data_loader())
	{
		data_loader.clearWhitespaceAndComments(); //Clear whitespace and comments before each set of event messages
		data_loader >> code;
		data_loader.clearWhitespaceAndComments(); //And after each code
		if (code < current_code || (code - 1) > current_code)
		{
			throw std::runtime_error(std::string(filename) + ": Incorrect code number or code order read, game data cannot be loaded.");
		}
		if (code > current_code)
		{
			object_typeid = 0;
			current_code = code;
		}

		m_event_messages.resize(5); // Number of distinct classes of objects

		switch (current_code)
		{
		case 0: //Load events for a BaseItem
		{
			std::vector<std::string> base_item_event_messages;
			base_item_event_messages.reserve(static_cast<int>(BaseItemEvent::NUMBER_OF_EVENTS));
			std::string event_message;
			for (int i{ 0 }; i < static_cast<int>(BaseItemEvent::NUMBER_OF_EVENTS); ++i)
			{
				data_loader.getLine(event_message);
				data_loader.handleNewlines(event_message);
				base_item_event_messages.push_back(event_message);
			}
			data_loader.checkStatus(); //Throws if error
			m_event_messages[0].push_back(std::move(base_item_event_messages));
			break;
		}
		case 1: //Load events for an Item
		{
			std::vector<std::string> item_event_messages;
			item_event_messages.reserve(static_cast<int>(ItemEvent::NUMBER_OF_EVENTS));
			std::string event_message;
			for (int i{ 0 }; i < static_cast<int>(ItemEvent::NUMBER_OF_EVENTS); ++i)
			{
				data_loader.getLine(event_message);
				data_loader.handleNewlines(event_message);
				item_event_messages.push_back(event_message);
			}
			data_loader.checkStatus(); //Throws if error
			m_event_messages[1].push_back(std::move(item_event_messages));
			break;
		}
		case 2: //Load events for a Mob
		{
			std::vector<std::string> mob_event_messages;
			mob_event_messages.reserve(static_cast<int>(MobEvent::NUMBER_OF_EVENTS));
			std::string event_message;
			for (int i{ 0 }; i < static_cast<int>(MobEvent::NUMBER_OF_EVENTS); ++i)
			{
				data_loader.getLine(event_message);
				data_loader.handleNewlines(event_message);
				mob_event_messages.push_back(event_message);
			}
			data_loader.checkStatus(); //Throws if error
			m_event_messages[2].push_back(std::move(mob_event_messages));
			break;
		}
		case 3: //Load events for a Threat
		{
			std::vector<std::string> threat_event_messages;
			threat_event_messages.reserve(static_cast<int>(ThreatEvent::NUMBER_OF_EVENTS));
			std::string event_message;
			for (int i{ 0 }; i < static_cast<int>(ThreatEvent::NUMBER_OF_EVENTS); ++i)
			{
				data_loader.getLine(event_message);
				data_loader.handleNewlines(event_message);
				threat_event_messages.push_back(event_message);
			}
			data_loader.checkStatus(); //Throws if error
			m_event_messages[3].push_back(std::move(threat_event_messages));
			break;
		}
		case 4: //Load events for a Merchant
		{
			std::vector<std::string> merchant_event_messages;
			merchant_event_messages.reserve(static_cast<int>(MerchantEvent::NUMBER_OF_EVENTS));
			std::string event_message;
			for (int i{ 0 }; i < static_cast<int>(MerchantEvent::NUMBER_OF_EVENTS); ++i)
			{
				data_loader.getLine(event_message);
				data_loader.handleNewlines(event_message);
				merchant_event_messages.push_back(event_message);
			}
			data_loader.checkStatus(); //Throws if error
			m_event_messages[4].push_back(std::move(merchant_event_messages));
			break;
		}
		case 5:
			//Does not check if correct number of object's messages loaded, should be checked by the handler, Game class using getNumberOfObjectTypesLoaded
			//End of file, success load
			data_loader.close();
			return;
		default:
			throw std::runtime_error(std::string(filename) + ": Unknown code read, events cannot be loaded.");
		}
		++object_typeid;
	}
	throw std::runtime_error(std::string(filename) + ": Data loader has failed, events data cannot be loaded."); //Should return from function from loop if successful
}