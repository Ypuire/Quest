#include "EventMsgHandler.h"

void EventMsgHandler::addEventMsg(const std::string& new_event_msg)
{
	m_event_msgs.push_back(new_event_msg);
}

void EventMsgHandler::addEventMsg(std::string&& new_event_msg)
{
	m_event_msgs.push_back(std::move(new_event_msg));
}

void EventMsgHandler::printMsgs()
{
	for (const auto& i : m_event_msgs)
	{
		std::cout << i << '\n';
	}
	m_event_msgs.clear();
}