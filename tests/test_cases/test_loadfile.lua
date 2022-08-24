
local f, msg = loadfile("mod1.lua")
assert(f ~= nil)
local m = f()
assert(m.a == 100)
print("OK")
