#pragma once
#include <stdio.h>
#include "Network.hpp"
#include "System.hpp"
#include "Dynamics.hpp"
#include "Storage.hpp"
#include "Command.hpp"

//Bot.h
class Bot
{
	NetworkInterface net;
	SystemInterface sys;
	DynamicsInterface dyn;
	CommandInterface cmd;
	StorageInterface sto;
public:
	NetworkInterface* getNet()	{return &net;}
	SystemInterface* getSys()	{return &sys;}
	DynamicsInterface* getDyn()	{return &dyn;}
	CommandInterface* getCmd()	{return &cmd;}
	StorageInterface* getSto()	{return &sto;}
	Bot();
	bool IssueCommand(const std::string cmd);
};
