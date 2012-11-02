local _libs = {
	"table";
	"serial";
	"json";
	"conf";
	"net";
	"ascii85";
	"base64";
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
