conf={save,load}
conf.save=function(dat) return file.write("conf", json.enc(dat)) end
conf.load=function() return json.dec(file.read("conf")) end
