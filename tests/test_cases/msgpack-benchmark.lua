local msgpack = require('msgpack')
do
	local bytes = assert(msgpack.encode(1, -5, math.pi, 'Test!', true, false, { a = 1, b = 2 }))
	local a, b, c, d, e, f, g = assert(msgpack.decode(bytes))
	assert(a == 1)
	assert(b == -5)
	assert(c == math.pi)
	assert(d == 'Test!')
	assert(e == true)
	assert(f == false)
	assert(type(g) == 'table')
	assert(g.a == 1)
	assert(g.b == 2)
end

-- test fixmap
for i = 1, 15 do -- map with size 0 will be encoded as an array, so we start at 1
	local array = {}; for j = 1, i do array['item_' .. j] = j end -- prepare test array
	local bytes = assert(msgpack.encode(array))
	assert(string.byte(bytes) == 0x80 + i)
	array = assert(msgpack.decode(bytes))
	for j = 1, i do assert(array['item_' .. j] == j) end
end

-- test fixarray
for i = 0, 15 do
	local array = {}; for j = 1, i do array[j] = j end -- prepare test array
	local bytes = assert(msgpack.encode(array))
	assert(string.byte(bytes) == 0x90 + i)
	array = assert(msgpack.decode(bytes))
	for j = 1, i do assert(array[j] == j) end
end

