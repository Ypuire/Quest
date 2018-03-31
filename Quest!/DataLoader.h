#pragma once
#include <cctype>
#include <iostream>
#include <fstream>
#include <string>

class DataLoader
{
	std::ifstream m_filereader;
	std::string m_filename;
	std::string m_error_msg;

public:

	DataLoader(const std::string& filename)
		: m_filename{ filename }, m_filereader{ filename }
	{}

	DataLoader() {}

	~DataLoader()
	{
		m_filereader.close();
	}

	void open(std::string& filename) { 
		m_filereader.open(filename);
		m_filename = filename;
	}
	bool is_open() const { return m_filereader.is_open(); }
	void close(){m_filereader.close();}

	bool good() const { return m_filereader.good(); }
	bool eof() const { return m_filereader.eof(); }
	bool fail() const { return m_filereader.fail(); }
	bool bad() const { return m_filereader.bad(); }

	const std::string& getErrorMsg() const { return m_error_msg; }

	//ADD SOME ERROR HANDLING MSGS FOR GOOD/EOF/FAIL/BAD IN THE FUTURE TO MAKE THIS CLASS MORE STANDALONE

	//Returns if the data_loader is still in good condition
	bool operator()()
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

	void clearWhitespace()
	{
		while (isspace(m_filereader.peek())) //Ignore character if whitespace found
		{
			m_filereader.get();
		}
	}

	//Discards comments and whitespace from the filereader stream
	//Comment syntax #1: When double slashes "//" are encountered, discards all characters until a newline character
	//Comment syntax #2: If only a single slash is encountered, discard all characters until next slash
	void clearWhitespaceAndComments()
	{
		clearWhitespace();

		while (m_filereader.peek() == '/')
		{
			m_filereader.get(); //Get and discard this first opening /
			if (m_filereader.peek() == '/') //If two / in a row (Like in C++ Comments: "//")
			{
				m_filereader.ignore(10000, '\n');
				clearWhitespace();
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
				clearWhitespace();
			}
		}//Loop, look for any more / or // comments. If next character is not a slash, attempt to read it
	}

	//Appends a string within double quotation marks (Including whitespace) to the string variable passed in
	//Returns 0 on success, -1 on failure
	//Reasons for failure: Reached eof/next character after ignoring whitespace and comments is not double quotation mark
	int getWithinQuotes(std::string& string)
	{
		clearWhitespaceAndComments();

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

	template <typename T>
	friend DataLoader& operator>>(DataLoader& data_loader, T& storage_variable)
	{
		data_loader.clearWhitespaceAndComments();
		data_loader.m_filereader >> storage_variable;
		return data_loader;
	}
};