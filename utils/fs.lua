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

local function usage(msg)  
  if msg ~= nil then
    print(msg)
  end
  print(arg[0] .. ' pack output_file [files] | ' .. arg[0] ..  ' unpack input_file [directory]')
end

local function do_pack()  
  if #arg == 1 then
    usage('You must specify the output file.')
    os.exit()
  end

  local outfile = arg[2]
  local stream = assert(io.open(outfile, "w+"))
  local files = {}
  local n = 0
  if #arg ~= 2 then
    for i = 3,#arg do
      n = n+1
      file = arg[i]
      print("adding file " .. file)
      files[file] = file
    end
  else
    for file in io.lines() do
      n = n+1
      print("adding file " .. file)
      files[file] = file
    end
  end
  if n == 0 then
    usage('You must at least specify one file to pack')
    os.exit()
  end
  pack(files, stream)
end

local function do_pack()  
  if #arg == 1 then
    usage('You must specify the output file.')
    os.exit()
  end

  local outfile = arg[2]
  local stream = assert(io.open(outfile, "w+"))
  local files = {}
  local n = 0
  if #arg ~= 2 then
    for i = 3,#arg do
      n = n+1
      file = arg[i]
      print("adding file " .. file)
      files[file] = file
    end
  else
    print('No files given in the command line, reading files from stdin')
    for file in io.lines() do
      n = n+1
      print("adding file " .. file)
      files[file] = file
    end
  end
  if n == 0 then
    usage('You must at least specify one file to pack')
    os.exit()
  end
  pack(files, stream)
end

local function do_unpack()  
  print("unpack not implemented")
end

if #arg == 0 or ( arg[1] ~= 'pack' and arg[1] ~= 'unpack' ) then
  usage('Please specify whether to pack or unpack')
  os.exit()
end

if arg[1] == 'pack' then
  do_pack()
else 
  do_unpack()
end
