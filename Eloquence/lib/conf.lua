
conf={save,load}
conf.save=function(dat) return file.write("conf", serialize(dat)) end
conf.load=function() return unserialize(file.read("conf")) end
