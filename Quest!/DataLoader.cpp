#include "DataLoader.h"

void DataLoader::open(std::string& filename)
{
	m_filereader.open(filename);
	m_filename = filename;
}

bool DataLoader::operator()()
{
	if (good())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DataLoader::clearWhitespace()
{
	while (isspace(m_filereader.peek())) //Ignore character if whitespace found
	{
		m_filereader.get();
	}
}

void DataLoader::clearWhitespaceAndComments()
{
	DataLoader::clearWhitespace();

	while (m_filereader.peek() == '/')
	{
		m_filereader.get(); //Get and discard this first opening /
		if (m_filereader.peek() == '/') //If two / in a row (Like in C++ Comments: "//")
		{
			m_filereader.ignore(10000, '\n');
			DataLoader::clearWhitespace();
			continue; //Restart the check, don't continue the check for comment syntax #2
		}
		else if (m_filereader.peek() != '/')
		{
			m_filereader.ignore(10000, '/'); //Get and discard a character as long as it is after a single / until the second / is found
			if (eof())
			{
				m_error_msg = "Data loader unexpectedly reached end of file. Did you forget the closing slash for a comment?";
				return;
			}
			DataLoader::clearWhitespace();
		}
	}//Loop, look for any more / or // comments. If next character is not a slash, attempt to read it
}

int DataLoader::getWithinQuotes(std::string& string)
{
	DataLoader::clearWhitespaceAndComments();

	if (m_filereader.peek() != '\"') //If first character after clearing whitespace is not double quotation marks, fail
	{
		m_error_msg = "Data loader cannot find data in double quotation marks. Please check the syntax of the data file.";
		return -1;
	}

	m_filereader.get(); //Skip the first double quote and get into the actual string to be read
	while (true)
	{
		if (eof())
		{
			m_error_msg = "Data loader unexpectedly reached end of file. Did you forget the closing \" for data within \"\"s?";
			return -1;
		}
		if (m_filereader.peek() == '\"') //Closing double quotation mark found, stop reading
		{
			break;
		}
		string.push_back(m_filereader.get());
	}
	m_filereader.get(); //Discard the closing double quotation mark
	return 0;
}
