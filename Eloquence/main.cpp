#include <stdio.h>
#include <iostream>
#include "Bot.hpp"

int main()
{
	Bot bot;
	std::string str = "";
	while(!([&str]() -> std::istream& { std::cout << ">> "; return std::getline(std::cin, str); }().eof() || std::cin.fail() || std::cin.bad()))
	{
		if(bot.IssueCommand(str) == false)
		{
			break;
		}
	}
	return 0;
}
