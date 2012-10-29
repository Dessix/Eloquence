libs = {
	"table",
	"serial",
	"json",
	"conf"
}
for _,libname in ipairs(libs) do
	print(runfile("lib/"..libname..".lua")
end
