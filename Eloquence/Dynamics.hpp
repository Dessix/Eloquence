#pragma once

class Bot;
class DynamicsInterface
{
	Bot& bot;
public:
	DynamicsInterface(Bot& bot) : bot(bot)
	{
	}
};
