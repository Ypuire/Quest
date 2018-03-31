#pragma once

enum class GameState
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

enum class Action
{
	MOVE_UP,
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_DOWN,
	INVENTORY1,
	INVENTORY2,
	INVENTORY3,
	INVENTORY4,
	SWAP_ITEM1,
	SWAP_ITEM2,
	SWAP_ITEM3,
	SWAP_ITEM4,
	CHECK_SURROUNDINGS,
	SAVE,
	EXIT,
	RUN
};

//not used
enum class EncounterType
{
	NONE,
	SHOP,
	MOB,
	THREAT
};

//Future implement, customised messages from attacking/defending/etc
class EventList
{
	//encounter
	//kill
	//killed player
	//kill (miss)
	//attacked player
	//load from file
};