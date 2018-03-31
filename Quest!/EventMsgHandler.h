#pragma once
#include <iostream>
#include <string>

class EventMsgHandler
{
	std::string m_event_msg;

public:


	//Add an event message to be held
	void addEventMsg(std::string&& new_event_msg)
	{
		if (m_event_msg.size() > 0) //Already has an event message being held
		{
			m_event_msg += new_event_msg + '\n'; //Append
		}
		else
		{
			m_event_msg = new_event_msg + '\n';
		}
	}

	//Prints all event messages being held then clears them
	void printMsgs()
	{
		std::cout << m_event_msg;
		m_event_msg.clear();
	}
};