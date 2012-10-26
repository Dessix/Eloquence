#pragma once
#pragma warning( disable : 4482 )
#include <string>

class Bot;
class ScriptEnv;
class NetworkInterface;
struct lua_State;
class CommandInterface
{
	Bot& bot;
	lua_State* lua;
	void InitializeScripting();
	void CloseScripting();
public:
	enum Response
	{
		CONTINUE,
		EXIT
	};
	CommandInterface(Bot& bot);
	~CommandInterface();
	Response IssueCommand(const std::string& cmd);
	NetworkInterface* getNetworking();
};
