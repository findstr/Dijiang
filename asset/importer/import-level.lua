local quaternion = require "quaternion"
local type = type
local format = string.format
local concat = table.concat
local json = require "json"

local function read_file(path)
	assert(path)
	local f<close> = io.open(path, "rb")
	local dat = f:read("a")
	return dat
end
local gprint = print
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

local id = 1
local level = json.decode(read_file(...))
print(0,"---")
print(0,"root:")

local function parse(path)
	return json.decode(read_file(path))
end

local importer = {}

importer['AnimationComponent'] = function(path)

end

importer['MeshComponent'] = function(path)
	local mesh = parse(path)
	print(4, "- meshfilter:")
	print(5,    "- submesh:")
	local m = mesh.sub_meshes[1].obj_file_ref
	m = m:match("([^/]+)$")
	print(6,        "mesh:", "asset/models/"..m)
	print(4, "- meshrender:")
	local mat = mesh.sub_meshes[1].material
	local x = mat:match("(%w+).material.json$")
	print(5,	"material: asset/material/"..x..".mat")
	print(5,	"shadowcaster: asset/material/shadowcaster.mat")
end

importer['MotorComponent'] = function(path)

end

importer['CameraComponent'] = function(path)

end

importer['RigidBodyComponent'] = function(path)

end

print(1, "- gameobject:")
print(2,     "id:", id)
print(2,     "parent:", 0)
print(2,     "name:", "Camera")
print(2,     "components:")
print(4,        "- transform:")
print(5,            "position:")
print(6,                "x:", 0)
print(6,                "y:", 1)
print(6,                "z:", 5)
print(5,            "rotation:")
print(6,                "x:", 0)
print(6,                "y:", 180)
print(6,                "z:", 0)
print(5,            "scale:")
print(6,                "x:", 1)
print(6,                "y:", 1)
print(6,                "z:", 1)
print(4,        "- camera:")
print(5,		"fov:", 60)
print(5,		"aspect:", 1)
print(5,		"clip_near_plane:", 1.0)
print(5,		"clip_far_plane:", 100.0)
print(5,		"perspective: true")
print(5,		"orthographic_size:", 10.0)
print(4,        "- component.main:")
print(5,		"speed:", 0.0)

id = id + 1
print(1, "- gameobject:")
print(2,     "id:", id)
print(2,     "parent:", 0)
print(2,     "name:", "Directional Light")
print(2,     "components:")
print(4,        "- transform:")
print(5,            "position:")
print(6,                "x:", 0)
print(6,                "y:", 1)
print(6,                "z:", -5)
print(5,            "rotation:")
print(6,                "x:", 0)
print(6,                "y:", 45)
print(6,                "z:", 0)
print(5,            "scale:")
print(6,                "x:", 1)
print(6,                "y:", 1)
print(6,                "z:", 1)
print(4,        "- light:")
print(5,            "type: directional")
print(5,			"color:")
print(6,                "r:", 1)
print(6,                "g:", 1)
print(6,                "b:", 1)
print(6,                "a:", 1)
print(5,			"intensity: 1")

id = id + 1
print(1, "- gameobject:")
print(2,     "id:", id)
print(2,     "parent:", 0)
print(2,     "name:", "Player")
print(2,     "components:")
print(4,        "- transform:")
print(5,            "position:")
print(6,                "x:", 0)
print(6,                "y:", 0)
print(6,                "z:", 0)
print(5,            "rotation:")
print(6,                "x:", -90)
print(6,                "y:", 0)
print(6,                "z:", 0)
print(5,            "scale:")
print(6,                "x:", 1)
print(6,                "y:", 1)
print(6,                "z:", 1)
print(4,        "- skinrender:")
print(5,			"mesh: asset/models/robot.obj")
print(5,			"skin: asset/models/robot.skin")
print(5,            "material: asset/material/robot.mat")
print(5,			"shadowcaster: asset/material/shadowcaster.mat")
print(4,        "- animator:")
print(5,			"skeleton:", "asset/skeleton/avatar.skeleton")


for k, obj in pairs(level.objects) do
	if obj.name ~= "Player" then
		id = id + 1
		print(1, "- gameobject:")
		print(2,     "id:", id)
		print(2,     "parent:", 0)
		print(2,     "name:", obj.name)
		print(2,     "components:")
		print(4,        "- transform:")
		print(5,            "position:")
		print(6,                "x:", obj.transform.position.x)
		print(6,                "y:", obj.transform.position.z)
		print(6,                "z:", -obj.transform.position.y)
		local rot = Quaternion.New(0, 0, 0, 1)
		local angles = rot:ToEulerAngles()
		angles.x = -90
		print(5,            "rotation:")
		print(6,                "x:", angles.x or 0)
		print(6,                "y:", angles.z or 0)
		print(6,                "z:", -angles.y or 0)
		print(5,            "scale:")
		print(6,                "x:", obj.transform.scale.x or 1)
		print(6,                "y:", obj.transform.scale.z or 1)
		print(6,                "z:", obj.transform.scale.y or 1)
		local def = parse(obj.definition)
		for i = 1, #def.components do
			local com = parse(def.components[i])
			importer[com.type_name](com.component)
		end
	end
end
id = id + 1
print(1, "- gameobject:")
print(2,     "id:", id)
print(2,     "parent:", 0)
print(2,     "name:", "Skybox")
print(2,     "components:")
print(4,        "- transform:")
print(5,            "position:")
print(6,                "x:", 0)
print(6,                "y:", 0)
print(6,                "z:", 0)
print(5,            "rotation:")
print(6,                "x:", 0)
print(6,                "y:", 0)
print(6,                "z:", 0)
print(5,            "scale:")
print(6,                "x:", 10)
print(6,                "y:", 10)
print(6,                "z:", 10)
print(4,        "- meshfilter:")
print(5,            "- submesh:")
print(6,                "mesh: asset/models/skybox.obj")
print(4,        "- meshrender:")
print(5,            "material: asset/material/skybox.mat")

