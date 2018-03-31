#pragma once
#include <iostream>
#include <string>

class EventMsgHandler
{
	std::string m_event_msg;

public:

	//Add an event message to be held
	void addEventMsg(std::string&& new_event_msg);

	//Prints all event messages being held then clears them
	void printMsgs();
};