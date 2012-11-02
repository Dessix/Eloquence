function serialize(dat) return table.val_to_str(dat) end
function unserialize(dat) return load("return ("..dat..")")() end
