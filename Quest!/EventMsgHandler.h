#pragma once
#include <iostream>
#include <string>
#include <vector>

class EventMsgHandler
{
	std::vector<std::string> m_event_msgs;

public:

	//Add an event message to be held
	void addEventMsg(const std::string& new_event_msg); 
	void addEventMsg(std::string&& new_event_msg);

	//Prints all event messages being held then clears them
	void printMsgs();
};