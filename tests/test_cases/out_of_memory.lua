
local m = {}

local s = "a quick brown fox jumps over the lazy dog."
for i = 0, 10000 do
    m[i] = string.rep(s, i*1000)
end

