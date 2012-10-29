#pragma once
#include <string>

class Bot;
class SystemInterface
{
	Bot& bot;
public:
	SystemInterface(Bot& bot) : bot(bot)
	{}
	std::string exec(char* cmd);
};
