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

	DataLoader(const char* filename)
		: m_filename{ filename }, m_filereader{ filename }
	{}

	DataLoader(const std::string& filename)
		: m_filename{ filename }, m_filereader{ filename }
	{}

	DataLoader() {}

	~DataLoader()
	{
		m_filereader.close();
	}

	void open(const char* filename);
	void open(const std::string& filename);
	bool is_open() const { return m_filereader.is_open(); }
	void close(){m_filereader.close();}

	bool good() const { return m_filereader.good(); }
	bool eof() const { return m_filereader.eof(); }
	bool fail() const { return m_filereader.fail(); }
	bool bad() const { return m_filereader.bad(); }

	const std::string& getErrorMsg() const { return m_error_msg; }

	//ADD SOME ERROR HANDLING MSGS FOR GOOD/EOF/FAIL/BAD IN THE FUTURE TO MAKE THIS CLASS MORE STANDALONE

	//Returns if the data_loader is still in good condition
	bool operator()();

	void clearWhitespace();

	//Discards comments and whitespace from the filereader stream
	//Comment syntax #1: When double slashes "//" are encountered, discards all characters until a newline character
	//Comment syntax #2: If only a single slash is encountered, discard all characters until next slash
	void clearWhitespaceAndComments();

	//Appends a string within double quotation marks (Including whitespace) to the string variable passed in
	//Returns 0 on success, -1 on failure
	//Reasons for failure: Reached eof/next character after ignoring whitespace and comments is not double quotation mark
	int getWithinQuotes(std::string& string);

	//Gets a line of characters and sets the string passed in to be equal to it
	void getLine(std::string& string) { std::getline(m_filereader, string); }
	void ignore(std::streamsize count, int delim) { m_filereader.ignore(count, delim); }

	//When "\n" is read from a file, it is read as two characters, '\\' and 'n'
	//This function removes \\n from a string and replaces it with \n so that newlines can be streamed properly
	void handleNewlines(std::string& string) const;

	//Throws if the filereader failed or is at eof
	//Note: This function is designed to specifically throw, use only if you are sure that you can't continue if filereader fails or eof
	void checkStatus() const;

	template <typename T>
	friend DataLoader& operator>>(DataLoader& data_loader, T& storage_variable)
	{
		data_loader.clearWhitespaceAndComments();
		data_loader.m_filereader >> storage_variable;
		return data_loader;
	}
};
