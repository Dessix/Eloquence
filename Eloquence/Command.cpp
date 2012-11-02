#include "Command.hpp"
#include "Bot.hpp"
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/adopt_policy.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>

void PrintToLog(const luabind::object& arg)
{
	std::cout << luabind::tostring_operator(arg).c_str();
}

int Print(lua_State* L)
{
	int argc = lua_gettop(L);
	for(int i = 0; i < argc; ++i)
	{
		std::string output = std::string(luabind::call_function<std::string>(L, "tostring", luabind::object(luabind::from_stack(L, i+1))));
		std::cout.write(output.c_str(), output.length());
	}
	return 0;
}
int PrintLn(lua_State* L)
{
	Print(L);
	std::cout << std::endl;
	return 0;
}

bool writeFile(std::string filepath, std::string contents)
{
	try
	{
		std::ofstream file(filepath);
		file << contents;
		return true;
	} catch (std::exception&)
	{
		return false;
	}
}
std::string readFile(std::string filepath)
{
	try
	{
		std::ifstream file(filepath);
		std::stringstream buff;
		buff << file.rdbuf();
		return buff.str();
	} catch (std::exception&)
	{
		return "";
	}
}

CommandInterface::CommandInterface( Bot& bot ) : bot(bot)
{
	InitializeScripting();
	//luaL_dostring(lua, "log(\"Hello!\")");
}

CommandInterface::~CommandInterface()
{
	CloseScripting();
}

void CommandInterface::InitializeScripting()
{
	lua = luaL_newstate();
	luaL_openlibs(lua);
	luabind::open(lua);

	////////////
	//BINDINGS//
	////////////
#define BIND(name, func) lua_register(lua, name, func)
#define RUN(str) luaL_dostring(lua, str)
	luabind::module(lua, "file")
	[
		luabind::def("write", writeFile),
		luabind::def("read", readFile)
	];
	luabind::module(lua)
		[
		luabind::class_<CommandInterface>("CommandInterface")
			.def("getNet", &CommandInterface::getNetworking)
			.def("getSystem", &CommandInterface::getSystem),
		luabind::class_<NetworkInterface>("NetworkInterface")
			.def("connect", &NetworkInterface::CreateSocket, luabind::adopt(luabind::result)),
		luabind::class_<NetworkInterface::SockHolder>("Socket")
			.def("read", &NetworkInterface::SockHolder::Read)
			.def("write", &NetworkInterface::SockHolder::Write)
			.def("disconnect", &NetworkInterface::SockHolder::Disconnect),
		luabind::class_<SystemInterface>("SystemInterface")
			.def("exec", &SystemInterface::exec)
	];
	BIND("printraw", Print);
	BIND("print", PrintLn);
	BIND("log", PrintLn);
	RUN("exit = function() ___EXIT = 1 end");
	RUN("require('cmdline')");
	RUN("require('index')");
#undef RUN
#undef BIND
	luabind::globals(lua)["cmd"]=this;
	luabind::globals(lua)["net"]=bot.getNet();
}

void CommandInterface::CloseScripting()
{
	lua_close(lua);
}
CommandInterface::Response CommandInterface::IssueCommand( const std::string& cmd )
{
	lua_State* lua = this->lua;
	const auto handleError = [&lua](){
		std::string err(luabind::tostring_operator(luabind::object(luabind::from_stack(lua,-1))));
		std::cout << ((err.length() > 0) ? err.c_str() : "Unspecified Error") << std::endl;
	};
	const auto callFunc = [&handleError](luabind::object& func)->luabind::object{
		try
		{
			return luabind::call_function<luabind::object>(func);
		}
		catch (luabind::error&)
		{
			throw;
		}
	};
	const std::string trimmedCmd = boost::algorithm::trim_copy(cmd);
	try
	{
		luabind::object repl = luabind::globals(lua)["_repl"];
		if(luabind::type(repl) != LUA_TTABLE)
		{
			printf("REPL NOT FOUND; NAIVE OVERRIDE:\n");
			callFunc((luabind::object)luabind::call_function<luabind::object>(lua, "load", trimmedCmd));
		} else {
			luabind::object handleline = repl["handleline"];
			luabind::call_function<luabind::object>(handleline, repl, trimmedCmd);
		}
	}
	catch (luabind::error&)
	{
		handleError();
	}
	lua_gc(lua,LUA_GCCOLLECT,0);
	return luabind::type(luabind::globals(lua)["___EXIT"]) != LUA_TNIL ? Response::EXIT : Response::CONTINUE;
}

NetworkInterface* CommandInterface::getNetworking()
{
	return bot.getNet();
}

SystemInterface* CommandInterface::getSystem()
{
	return bot.getSys();
}
