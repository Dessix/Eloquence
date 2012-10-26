#include "Command.h"
#include "Bot.h"
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>

#include "jsonliblua.h"
#include "tableliblua.h"

void PrintToLog(const luabind::object& arg)
{
	std::cout << luabind::tostring_operator(arg).c_str();
}

int Print(lua_State* L)
{
	int argc = lua_gettop(L);
	for(int i = 0; i < argc; ++i)
	{
		std::cout << std::string(luabind::call_function<std::string>(L, "tostring", luabind::object(luabind::from_stack(L, i+1))));
	}
	return 0;
}
int PrintLn(lua_State* L)
{
	int argc = lua_gettop(L);
	for(int i = 0; i < argc; ++i)
	{
		std::cout << std::string(luabind::call_function<std::string>(L, "tostring", luabind::object(luabind::from_stack(L, i+1))));
	}
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
			.def("getNet", &CommandInterface::getNetworking),
		luabind::class_<NetworkInterface>("NetworkInterface")
			.def("connect", &NetworkInterface::CreateSocket),
		luabind::class_<NetworkInterface::SockHolder>("Socket")
			.def("disconnect", &NetworkInterface::SockHolder::Disconnect)
			.def("read", &NetworkInterface::SockHolder::Read)
			.def("write", &NetworkInterface::SockHolder::Write)
	];
	BIND("print", Print);
	BIND("println", PrintLn);
	BIND("log", PrintLn);
	RUN("exit = function() ___EXIT = 1 end");
	RUN("function serialize(dat) return table.val_to_str(dat) end");
	RUN("function unserialize(dat) return load(\"return (\"..dat..\")\")() end");
	RUN("conf={save,load}");
	RUN("conf.save=function(dat) return file.write(\"conf\", serialize(dat)) end");
	RUN("conf.load=function() return unserialize(file.read(\"conf\")) end");
	RUN(TABLELIBLUA);
	RUN(JSONLIBLUA);
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
	const auto callFunc = [&handleError](luabind::object& cmdFunc){
		try
		{
			luabind::object retval = luabind::call_function<luabind::object>(cmdFunc);
			if(luabind::type(retval) != LUA_TNIL)
			{
				std::cout<< std::string(luabind::call_function<std::string>(cmdFunc.interpreter(), "tostring", retval)).c_str() << std::endl;
			}
		}
		catch (luabind::error&)
		{
			handleError();
		}
	};
	const std::string trimmedCmd = boost::algorithm::trim_copy(cmd);
	luabind::object cmdFunc = luabind::globals(lua)[trimmedCmd.c_str()];
	try
	{
		if(luabind::type(cmdFunc) == LUA_TNIL)
		{
			std::string newcmd(trimmedCmd);
			if(cmd[0] == '=')
			{
				newcmd = std::string("return (") + trimmedCmd.substr(1, trimmedCmd.length() - 1) + ")";
			}
			cmdFunc = luabind::call_function<luabind::object>(lua, "load", newcmd);
		}
		if(luabind::type(cmdFunc) == LUA_TFUNCTION)
		{
			callFunc(cmdFunc);
		} else if(luabind::type(cmdFunc) != LUA_TNIL) {
			std::cout << std::string(luabind::call_function<std::string>(cmdFunc.interpreter(), "tostring", cmdFunc)).c_str() << std::endl;
		}
	}
	catch (luabind::error&)
	{
		handleError();
	}
	return luabind::type(luabind::globals(lua)["___EXIT"]) != LUA_TNIL ? Response::EXIT : Response::CONTINUE;
}

NetworkInterface* CommandInterface::getNetworking()
{
	return bot.getNet();
}
