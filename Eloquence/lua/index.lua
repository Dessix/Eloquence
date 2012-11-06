local _libs = {
	"table";
	"serial";
	"json";
	"conf";
	"net";
	"ascii85";
	"base64";
	"bot";
}

local function _loadLibs()
	for _,libname in ipairs(_libs) do
		require(libname)
	end
end

function reloadLibs()
	return _loadLibs()
end

reloadLibs()

pcall(require,"autorun")--To run as a replacement for user-run commands when on a bot configuration
