for i = 1, 10000 do
    -- When length is zero, we return the length of the data instead of the data itself.
    local length = (i % 1000) + 1
    local offset = i
    local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, length, offset)
    assert(not error)
    assert(#buf == length)
end
