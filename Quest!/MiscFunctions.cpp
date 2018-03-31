#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <fstream>
#include <random>
#include <string>
#include "MiscFunctions.h"

//Constants
//
namespace CONSTANTS
{
	constexpr char* VERSION_NUMBER = "v0.3.1";
}

//Utility functions
//

int getRandomInt(int min, int max)
{
	static std::mt19937 mersenne(std::random_device{}());
	std::uniform_int_distribution<int> distri(min, max);
	return distri(mersenne);
}

std::string getTimeAndDate()
{
	time_t time = std::time(nullptr);
	std::tm timeinfo;
	localtime_s(&timeinfo, &time);
	std::stringstream ss;
	ss << std::put_time(&timeinfo, "%Y %B %d %A %T");
	return ss.str();
}


//General input and message output functions
//

void printMainMenuMsg()
{
	std::cout << "Welcome to Mini Quest main menu, version " << CONSTANTS::VERSION_NUMBER << " " << __DATE__ << " " << __TIME__ << std::endl;
	std::cout << "1)Start quick game (With default settings)\n" << "2)Start new game\n" << "3)Load saved game\n" << "e)Exit" << std::endl;
}

void printInvalidOption()
{
	std::cout << "Invalid option inputted, please try again\n";
}

//General error handling functions
//

bool validInput()
{
	if (!std::cin || !(std::cin.peek() == '\n'))
	{
		std::cout << "Invalid input, please try again\n" << std::endl;
		std::cin.clear();
		std::cin.ignore(1024, '\n');
		return false;
	}
	return true;
}

void logError(const std::string& error_msg)
{
	std::ofstream filewriter;
	filewriter.open("ErrorLog.txt", std::ios::app);
	filewriter << getTimeAndDate() << ": FATAL ERROR. " << error_msg << '\n';
	filewriter.close();
}