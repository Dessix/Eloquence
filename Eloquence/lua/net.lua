LineReader = function(sock)
	return {
		sock = sock,
		currentData = "",
		receiveLine = function(self)--Returns StreamState (Boolean), and Line (String/Nil)
			local function scanForNewline(currentData)
				local foundNewline = currentData:find('\n', 0, true)
				if(foundNewline)then
					local line = currentData:sub(1, foundNewline-1)
					if(currentData:len() > line:len() + 1)then--Is currentData of format "<msg>\n"?
						currentData = currentData:sub(foundNewline+1)--No, there is more data
					else
						currentData = ""--Yes, it ended on a newline
					end
					return currentData, line
				else
					return currentData, nil
				end
			end
			--Check the current buffer for a newline; If it has one, don't ask for new data
			self.currentData, line = scanForNewline(self.currentData)
			if(line)then
				return true, line
			end

			--Ask the socket for new data.
			success, message = pcall(self.sock.read, self.sock)
			if(success)then
				self.currentData = self.currentData .. (message or "")
			else
				return false, nil
			end

			--Check the new data for a newline
			self.currentData, line = scanForNewline(self.currentData)
			if(line)then
				return true, line
			else
				return true, nil
			end
		end
	}
end
