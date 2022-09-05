local bn = {}

local bn_mt = {}
local MAX_INTEGER = 2 ^ 32

local dump = nil

local function bind_methods(t)
    assert(#t > 0)
    t.dump = dump
    return setmetatable(t, bn_mt)
end

bn_mt.__add = function(a, b)
    assert(#a == #b)
    local carry = 0
    local res = {}
    for i = 1, #a do
        local temp = a[i] + b[i] + carry
        if temp >= MAX_INTEGER then
            temp = temp - MAX_INTEGER
            carry = 1
        else
            carry = 0
        end
        res[i] = temp
    end
    return bind_methods(res), carry > 0
end

bn_mt.__eq = function(a, b)
    assert(#a == #b)
    for i = 1, #a do if a[i] ~= b[i] then return false end end
    return true
end

bn_mt.__lt = function(a, b)
    assert(#a == #b)
    for i = #a, 1, -1 do
        if a[i] < b[i] then
            return true
        elseif a[i] > b[i] then
            return false
        end
    end
    return false
end

bn_mt.__le = function(a, b)
    assert(#a == #b)
    for i = #a, 1, -1 do
        if a[i] < b[i] then
            return true
        elseif a[i] > b[i] then
            return false
        end
    end
    return true
end

bn.new = function(bits, u64_value)
    assert(bits % 64 == 0)
    local limbs_count = bits // 32
    local res = {}
    for _ = 1, limbs_count, 1 do table.insert(res, 0) end
    res[1] = u64_value % MAX_INTEGER
    res[2] = u64_value // MAX_INTEGER
    return bind_methods(res)
end

dump = function(bn, name)
    local prefix = ""
    if name then
        prefix = string.format("%s =", name)
    else
        prefix = "<no name> ="
    end
    local res = {}
    for k, v in ipairs(bn) do table.insert(res, string.format("%d", v)) end
    print(prefix, table.concat(res, " "))
end

local function test()
    local a = bn.new(256, MAX_INTEGER + 1)
    local b = bn.new(256, MAX_INTEGER + 2)
    local sum = a + b
    assert(sum[1] == 3)
    assert(sum[2] == 2)
    assert(a < b)
    assert(a <= b)
    assert(a <= a)
    assert(b <= b)
    assert(a == a)
    assert(b > a)
    assert(b >= a)
    local a = bn.new(256, MAX_INTEGER - 1)
    local b = bn.new(256, 1)
    local sum, overflow = a + b
    assert(sum[1] == 0)
    assert(sum[2] == 1)
    assert(not overflow)
    sum:dump("sum")
end

test()

