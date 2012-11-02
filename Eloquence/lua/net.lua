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
		await = function(self)--Wait until a line successfully arrives, or fail if the server disconnected.
			while(true)do
				StreamState, Line = self:receive()
				if(not StreamState)then
					return Line
				else
					if(StreamState and Line)then
						return Line
					end
				end
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

function downloadFile(host, file, port)--Returns filedata as a string, or nil on failure --Broken to an extreme degree.
	port = port or 80
	local sock = net:connect(host, port)
	sock:write("GET "..file.." HTTP/1.0\n\n")
	local lr = LineReader(sock)

	local line = lr:await()
	if(not line:find("200"))then
		return nil
	end

	while(true)do
		line = lr:await()
		if(line:find("-Length"))then
			break
		end
	end
	local filedat = lr:rest()
	return filedat
end
