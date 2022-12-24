local status, mymodule = pcall(require, "mymodule")
if status then
  print("mymodule should not exist before mounting")
  ckb.exit(1)
end

local function check(source, index, expected_return_value)
  local err = ckb.mount(source, index)
  if err ~= nil then
    print("mounting source " .. source " index " .. index .. "failed: " .. err)
    ckb.exit(1)
  end

  -- Make sure mymodule will be reloaded.
  package.loaded.mymodule = nil
  local mymodule = require'mymodule'
  local magic = mymodule.magic()
  if magic ~= expected_return_value then
    print("expecting " .. expected_return_value .. " from mymodule, but " .. magic .. " returned")
    ckb.exit(1)
  end
end

check(2, 0, 42)
check(2, 1, 43)
