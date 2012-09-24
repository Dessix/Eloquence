#pragma once

class Bot;
class SystemInterface
{
	Bot& bot;
public:
	SystemInterface(Bot& bot) : bot(bot)
	{}
};
