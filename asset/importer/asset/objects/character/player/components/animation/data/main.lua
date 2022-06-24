local json = require "json"
local f = io.open("skeleton_data_root.skeleton.json", "r")
local t = f:read("a")
f:close()
local obj = json.decode(t)
print(#obj['bones_map'])


