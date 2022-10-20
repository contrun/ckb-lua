local spack, sunpack = string.pack, string.unpack

local function get_file_size(path)
  local f = assert(io.open(path))
  local size = f:seek("end")
  f:close()
  return size
end

local function append_to_stream(str, stream)
  stream:write(str)
end

local function append_file_to_stream(path, stream)
  local infile = assert(io.open(path, "r"))
  local s = infile:read("*a")
  infile:close()
  append_to_stream(s, stream)
end

local function append_string_null_to_stream(str, stream)
  local s = spack("z", str)
  append_to_stream(s, stream)
end

local function append_integer_to_stream(num, stream)
  local s = spack("<I4", num)
  append_to_stream(s, stream)
end

function tablelength(t)
  local count = 0
  for _ in pairs(t) do count = count + 1 end
  return count
end

local function pack(files, stream)  
  local num = tablelength(files)
  append_integer_to_stream(num, stream)

  local offset = 0
  local length = 0
  for name, path in pairs(files) do
    append_integer_to_stream(offset, stream)
    length = #name + 1
    append_integer_to_stream(length, stream)
    offset = offset + length
    append_integer_to_stream(offset, stream)
    length = get_file_size(path)
    append_integer_to_stream(length, stream)
    offset = offset + length
  end

  for name, path in pairs(files) do
    append_string_null_to_stream(name, stream)
    append_file_to_stream(path, stream)
  end
end

local outfile = "lfs"
local stream = assert(io.open(outfile, "w+"))
local files = {["main.lua"] = "../contracts/hello.lua"}
pack(files, stream)
