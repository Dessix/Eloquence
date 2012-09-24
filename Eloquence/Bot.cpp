#include "Bot.h"
#include "Command.h"

#pragma warning ( push )
#pragma warning ( disable : 4355 )
Bot::Bot() : net(*this), sys(*this), dyn(*this), cmd(*this), sto(*this)
#pragma warning ( pop )
{
}

bool Bot::IssueCommand( const std::string cmd )
{
	switch(getCmd()->IssueCommand(cmd))
	{
	case CommandInterface::Response::EXIT:
		{
			return false;
		} break;
	case CommandInterface::Response::CONTINUE:
		{
			return true;
		} break;
	default:
		return false;
	}
}
