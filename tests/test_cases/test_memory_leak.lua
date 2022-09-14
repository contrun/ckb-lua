for i = 1, 10000 do
  local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 10)
  assert(not error)
end
