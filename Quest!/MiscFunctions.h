#pragma once

//Utility functions
//

int getRandomInt(int min, int max);
std::string getTimeAndDate();

//General input and message output functions
//

void printMainMenuMsg();	

void printInvalidOption();

//General error handling functions
//

bool validInput();
void logError(const std::string& error_msg);