#pragma once

class Bot;
class StorageInterface
{
	Bot& bot;
public:
	StorageInterface(Bot& bot) : bot(bot)
	{
	}
};
