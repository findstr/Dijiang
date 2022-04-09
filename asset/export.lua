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
local function print(level,...)
	gprint(tab(level) .. concat({...}, " "))
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
	print(4, "-meshfilter:")
	print(5,    "-submesh:")
	print(6,        "mesh:", mesh.sub_meshes[1].obj_file_ref)
	print(6,	"material:", mesh.sub_meshes[1].material)
end

importer['MotorComponent'] = function(path)

end

importer['CameraComponent'] = function(path)

end

importer['RigidBodyComponent'] = function(path)

end


for k, obj in pairs(level.objects) do
	print(1, "-gameobject:")
	print(2,     "id:", id)
	print(2,     "name:", obj.name)
	print(2,     "components:")
	print(4,        "-transform:")
	print(5,            "position:")
	print(6,                "x:", obj.transform.position.x)
	print(6,                "y:", obj.transform.position.y)
	print(6,                "z:", obj.transform.position.z)
	print(5,            "rotation:")
	print(6,                "x:", obj.transform.rotation.x or 0)
	print(6,                "y:", obj.transform.rotation.y or 0)
	print(6,                "z:", obj.transform.rotation.z or 0)
	print(6,                "w:", obj.transform.rotation.w or 0)
	print(5,            "scale:")
	print(6,                "x:", obj.transform.scale.x or 1)
	print(6,                "y:", obj.transform.scale.y or 1)
	print(6,                "z:", obj.transform.scale.z or 1)
	print(6,                "w:", obj.transform.scale.w or 1)
	local def = parse(obj.definition)
	for i = 1, #def.components do
		local com = parse(def.components[i])
		importer[com.type_name](com.component)
	end
	id = id + 1
end
