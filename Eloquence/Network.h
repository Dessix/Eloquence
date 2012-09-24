#pragma once

class Bot;
class NetworkInterface
{
	Bot& bot;
public:
	NetworkInterface(Bot& bot) : bot(bot)
	{
	}
};
