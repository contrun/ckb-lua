
local m = {}

local s = "a quick brown fox jumps over the lazy dog."
for i = 0, 100000000 do
    m[i] = string.format("s: %d", i)
end

