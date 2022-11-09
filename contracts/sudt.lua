ERROR_LOAD_SCRIPT = 1
ERROR_INVALID_SCRIPT = 2
ERROR_LOAD_LOCK_HASH = 3
ERROR_LOAD_CELL_DATA = 4
ERROR_INVALID_CELL_DATA = 5
ERROR_OVERFLOWING = 6
ERROR_INVALID_AMOUNT = 7

OWNER_LOCK_HASH_SIZE = 32
LUA_LOADER_ARGS_SIZE = 35
AMOUNT_BITS = 128
AMOUNT_BYTES = AMOUNT_BITS/16

local bn = {}

local bn_mt = {}
local MAX_INTEGER = 1 << 32

local load = nil
local save = nil
local tostring = nil

local function bind_methods(t)
    assert(#t > 0)
    t.load = load
    return setmetatable(t, bn_mt)
end

bn_mt.__tostring = function(self)
    local res = {}
    for k, v in ipairs(self) do table.insert(res, string.format("%d", v)) end
    return "[" .. table.concat(res, ",") .. "]"
end

bn_mt.__add = function(a, b)
    assert(#a == #b)
    local carry = 0
    local res = {}
    res.overflow = false
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
    if carry > 0 then res.overflow = true end
    return bind_methods(res)
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

load = function(self, raw)
    assert(raw:len() > 0)
    assert(raw:len() % 4 == 0)
    local index = 1
    local fmt = "<I4"
    for i = 1, raw:len(), 4 do
        local num = fmt:unpack(raw, i)
        self[index] = num
        index = index + 1
    end
end

function get_owner_lock_hash() 
  local _code_hash, _hash_type, args, err = ckb.load_and_unpack_script()
  if err ~= nil then
    return ERROR_LOAD_SCRIPT
  end

  -- args must be a hash of the owner private key
  if #args ~= (OWNER_LOCK_HASH_SIZE + LUA_LOADER_ARGS_SIZE) then
    return ERROR_INVALID_SCRIPT
  end

  owner_lock_hash = string.sub(args, LUA_LOADER_ARGS_SIZE+1, -1)
  print("owner pk hash")
  ckb.dump(owner_lock_hash)

  return owner_lock_hash
end

function main()
  local owner_lock_hash = get_owner_lock_hash()

  local index = 0
  while true do
    local data, err = ckb.load_cell_by_field(index, ckb.SOURCE_INPUT, ckb.CELL_FIELD_LOCK_HASH)
    if err == ckb.INDEX_OUT_OF_BOUND then
      break
    end
    if err ~= nil then
      return ERROR_LOAD_LOCK_HASH
    end
    print("lock hash")
    ckb.dump(data)
    if data == owner_lock_hash then
      return 0
    end
    index = index + 1
  end

  local tmp_number = bn.new(AMOUNT_BITS, 0)

  local index = 0
  local input_sum = bn.new(AMOUNT_BITS, 0)
  while true do
    local data, err = ckb.load_cell_data(index, ckb.SOURCE_GROUP_INPUT, AMOUNT_BYTES)
    if err == ckb.INDEX_OUT_OF_BOUND then
      break
    end
    if err ~= nil then
      return ERROR_LOAD_CELL_DATA
    end
    if #data < AMOUNT_BYTES then
      return ERROR_INVALID_CELL_DATA
    end
    tmp_number:load(data)
    input_sum = input_sum + tmp_number
    if input_sum.overflow then
      return ERROR_OVERFLOWING
    end
    index = index + 1
  end

  local index = 0
  local output_sum = bn.new(AMOUNT_BITS, 0)
  while true do
    local data, err = ckb.load_cell_data(index, ckb.SOURCE_GROUP_OUTPUT, AMOUNT_BYTES)
    if err == ckb.INDEX_OUT_OF_BOUND then
      break
    end
    if err ~= nil then
      return ERROR_LOAD_CELL_DATA
    end
    if #data < AMOUNT_BYTES then
      return ERROR_INVALID_CELL_DATA
    end
    tmp_number:load(string.sub(data, 1, AMOUNT_BYTES))
    output_sum = output_sum + tmp_number
    if output_sum.overflow then
      return ERROR_OVERFLOWING
    end
    index = index + 1
  end

  print("input_sum", input_sum, "output_sum", output_sum, "\n")
  if input_sum < output_sum then
    return ERROR_INVALID_AMOUNT
  end

  return 0
end

ckb.exit(main())
