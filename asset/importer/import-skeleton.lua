local json = require "json"
local quaternion = require "quaternion"
require "Vector3"
local f = io.open(...)
local txt = f:read("a")
local obj, err = json.decode(txt)
local bones = obj.bones_map
local gprint = print
local concat = table.concat
local function tab(n)
	local buf = {}
	for i = 1, n do
		buf[i] = "    "
	end
	return concat(buf)
end
local neg = "-"
neg = neg:byte(1)
local function print(level,...)
	local x = {...}
	for i = 1, #x do
		if type(x[i]) == "number" then
			local s = tostring(x[i])
			if s:byte(1) ~= neg then
				s = "+" .. s
			end
			x[i] = s
		end
	end
	gprint(tab(level) .. concat(x, " "))
end
print(0, "skeleton:")
for i = 1, #bones do
	local joint = bones[i]
	print(1, "- bone:")
		print(2, "id:", joint.index)
		print(2, "name:", joint.name)
		print(2, "parent:", joint.parent_index or -1)
		print(2, "position:")
			print(3, "x:", joint.binding_pose.position.x or 0)
			print(3, "y:", joint.binding_pose.position.y or 0)
			print(3, "z:", joint.binding_pose.position.z or 0)
		print(2, "rotation:")
			local rot
			local x, y, z, w = 
								joint.binding_pose.rotation.x or 0,
								joint.binding_pose.rotation.y or 0,
								joint.binding_pose.rotation.z or 0,
								joint.binding_pose.rotation.w or 0
			if x == 0 and y == 0 and z == 0 and w == 0 then
				rot = Quaternion.New(0, 0, 0, 1)
			else
				rot = Quaternion.New(x, y, z, w)
			end
			print(3, "x:", rot.x or 0)
			print(3, "y:", rot.y or 0)
			print(3, "z:", rot.z or 0)
			print(3, "w:", rot.w or 0)
		print(2, "scale:")
			print(3, "x:", joint.binding_pose.scale.x or 1.0)
			print(3, "y:", joint.binding_pose.scale.y or 1.0)
			print(3, "z:", joint.binding_pose.scale.z or 1.0)
		print(2, "tpose:")
		for k, v in pairs(joint.tpose_matrix) do
			print(3, k..":", v)
		end
end


