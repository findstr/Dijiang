local json = require "json"
local selector, file = ...
local f = io.open(file)
local txt = f:read("a")
local obj, err = json.decode(txt)
local concat = table.concat

if selector == "mesh" then
	local vertices = obj.vertex_buffer
	local indices = obj.index_buffer
	local v = {}
	local vt = {}
	local vn = {}
	local face = {}
	for i = 1, #vertices do
		local x = vertices[i]
		print("v", x.px, x.py, x.pz)
	end
	for i = 1, #vertices do
		local x = vertices[i]
		print("vt",  x.u, 1.0 - x.v)

	end
	for i = 1, #vertices do
		local x = vertices[i] 
		print("vn", x.nx or 0, x.ny or 0, x.nz or 0)	
	end
	for i = 1, #indices, 3 do
		local a = indices[i] + 1
		local b = indices[i+1] + 1
		local c = indices[i+2] + 1
		print("f", a.."/"..a.."/"..a, b.."/"..b.."/"..b, c.."/"..c.."/"..c)
	end
else
	local index = {"index0", "index1", "index2", "index3"}
	local weight = {"weight0", "weight1", "weight2", "weight3"}
	local bind = obj.bind
	print(#bind)
	local buf = {}
	for i = 1, #bind do
		local b = bind[i]
		local vertex = {}
		for i = 1, 4 do
			local iname = index[i]
			local wname = weight[i]
			local iv = b[iname]
			local wv = b[wname]
			if iv and wv then
				vertex[#vertex + 1] = string.format("%s:%s", iv, wv)
			else
				assert(not iv)
				assert(not wv)
				break
			end
		end
		buf[i] = table.concat(vertex, ",")
	end
	print(table.concat(buf, "\n"))
end
