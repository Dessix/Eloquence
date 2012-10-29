_libs = {
	"table",
	"serial",
	"json",
	"conf"
}

function _loadLibs()
	for _,libname in ipairs(_libs) do
		result, msg = runfile("lib/"..libname..".lua")
		if not result then
			print("Error loading library \""..libname.."\":")
			print(msg)
		end
	end
end

function reloadLibs()
	return _loadLibs()
end

_loadLibs()
