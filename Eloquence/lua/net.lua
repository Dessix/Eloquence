LineReader = function(sock)
	return {
		sock = sock,
		currentData = "",
		receive = function(self)--Returns StreamState (Boolean), and Line (String/Nil)
			local function scanForNewline(currentData)
				local foundNewline = currentData:find('\n', 0, true)
				if(foundNewline)then
					local line = currentData:sub(1, foundNewline-1)
					if(currentData:len() > line:len() + 1)then--Is currentData of format "<msg>\n"?
						currentData = currentData:sub(foundNewline+1)--No, there is more data
					else
						currentData = ""--Yes, it did end on a newline
					end
					return currentData, line
				else
					return currentData, nil
				end
			end
			--Check the current buffer for a newline; If it has one, do not ask for new data
			self.currentData, line = scanForNewline(self.currentData)
			if(line)then
				return true, line
			end

			--Ask the socket for new data.
			success, message = pcall(self.sock.read, self.sock)
			if(success)then
				self.currentData = self.currentData .. (message or "")
			else
				return false, self.currentData
			end

			--Check the new data for a newline
			self.currentData, line = scanForNewline(self.currentData)
			if(line)then
				return true, line
			else
				return true, nil
			end
		end,
		rest = function(self)--Get the rest of the message until an empty string or server closed message arrives.
			while(true)do
				StreamState, Data = pcall(self.sock.read, self.sock)

				if(not StreamState or Data == "" or Data == nil)then
					break
				end

				self.currentData = self.currentData .. Data
			end
			return self.currentData
		end
	}
end

function CommandManager(sock)
	return {
		sock = sock;
		lr = LineReader(sock);
		get = function(self)--Returns json or false.
			local status, data = self.lr:receive()
			if not(status)then
				error("socket has closed.")
			end
			return data or nil;
		end;
		put = function(self, command, lineEnding)
			return self.sock:write(tostring(command)..tostring(lineEnding or "\r\n"))
		end;
		putRaw = function(self, command)
			return self:put(command, "")--Override lineEnding with "" for raw
		end;
		putJSON = function(self, command)--In lua table format
			return self:put(json.enc(command))
		end;
		blockget = function(self, timeout, incremental)--waits until data arrives, or up until timeout in incremental intervals
			if(timeout)then
				incremental = incremental or 50
				local timeWaited = 0
				for i = 0, timeout, incremental do
					local ret = self:get()
					if(ret)then
						return ret
					end
					sleep(incremental)
				end
			else
				while(true)do
					local ret = self:get()
					if(ret)then
						return ret
					end
					sleep(incremental)
				end
			end
		end;
	}
end
