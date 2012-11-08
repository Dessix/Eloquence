function control(host, port)
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
		blockread = function(self)--Reads until a message arrives
			local dat = self.cmdMgr:blockget()
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
			print(self:commandStringify(data))
		end;
		toserver = function(self, data)--Display something nicely to console sent to server
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
			self:send(self:CMD("IDENTIFY", genIdentController(authcode)),false)
			self:notify("Waiting for identification status...")
			return self:blockread()
		end;
	}
	return ctrl
end
