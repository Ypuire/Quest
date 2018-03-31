#pragma once
#include <vector>
#include <string>
#include <type_traits>
#include <cassert>
#include <exception>
#include <utility>
#include "DataLoader.h"

enum class GameState : int
{
	ONGOING,
	SAVING,
	EXITING,
	ENCOUNTER_MOB,
	ENCOUNTER_THREAT,
	ENCOUNTER_NPC,
	WON,
	LOST
};

enum class Action : int
{
	MOVE_UP,
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_DOWN,
	INVENTORY1,
	INVENTORY2,
	INVENTORY3,
	INVENTORY4,
	INVENTORY5,
	INVENTORY6,
	INVENTORY7,
	INVENTORY8,
	INVENTORY9,
	SWAP_ITEM1,
	SWAP_ITEM2,
	SWAP_ITEM3,
	SWAP_ITEM4,
	SWAP_ITEM5,
	SWAP_ITEM6,
	SWAP_ITEM7,
	SWAP_ITEM8,
	SWAP_ITEM9,
	CHECK_SURROUNDINGS,
	SAVE,
	EXIT,
	RUN
};

enum class BaseItemEvent : int
{
	INSPECT_DESCRIPTION = 0,
	USE,
	NUMBER_OF_EVENTS
};

enum class ItemEvent : int
{
	USE_SUCCESS = 0,
	USE_FAILURE,
	ITEM_USED_UP,
	INSPECT_DESCRIPTION,
	NUMBER_OF_EVENTS
};

enum class MobEvent : int
{
	ATTACK = 0,
	DEATH,
	INSPECT_DESCRIPTION,
	NUMBER_OF_EVENTS
};

enum class ThreatEvent : int
{
	ATTACK = 0,
	INSPECT_DESCRIPTION,
	NUMBER_OF_EVENTS
};

enum class MerchantEvent : int
{
	INSPECT_DESCRIPTION = 0,
	NUMBER_OF_EVENTS
};


class EventList final
{
private:
	std::vector<std::vector<std::vector<std::string>>> m_event_messages;

public:

	template <typename T1, typename T2>
	const std::string& getMessage(T1 object_type, int type_id, T2 event_type) const //Gets the corresponding string for an object of type_id's event
	{
		static_assert(std::is_enum<T1>::value, "EventList.getMessage(T1, int, T2): T1 object_type parameter must be of enumeration type");
		static_assert(std::is_enum<T2>::value, "EventList.getMessage(T1, int, T2): T2 event_type parameter must be of enumeration type");

		return m_event_messages[static_cast<int>(object_type)][type_id][static_cast<int>(event_type)];
	}

	template <typename T>
	int getNumberOfObjectTypesLoaded(T object_type) const //Returns number of object types loaded for a particular object for Game class to verify
	{
		static_assert(std::is_enum<T>::value, "EventList.getNumberOfObjectTypesLoaded(T): T object_type parameter must be of enumeration type");

		return m_event_messages[static_cast<int>(object_type)].size();
	}

	//Loads all the event messages associated with each object (item/entity)
	void loadEvents(const char* filename);

};