#include "EventMsgHandler.h"

void EventMsgHandler::addEventMsg(std::string&& new_event_msg)
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

void EventMsgHandler::printMsgs()
{
	std::cout << m_event_msg;
	m_event_msg.clear();
}