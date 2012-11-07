#pragma once
#include <string>

class Bot;
class SystemInterface
{
	Bot& bot;
public:
	SystemInterface(Bot& bot) : bot(bot)
	{}
	std::string exec(const char* cmd);
	std::string cwd();
	bool setcwd(std::string path);
};
