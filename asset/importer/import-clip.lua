local json = require "json"
local concat = table.concat
local format = string.format
local clip_file, convert_file = ...
local clip_obj, err = json.decode(io.open(clip_file):read("a"))
local convert_obj, err = json.decode(io.open(convert_file):read("a"))

local convert = convert_obj.convert
local clip_data = clip_obj.clip_data

local list = {}

local function parse_frame(id, channel)
	local frame = {}
	local bone = {
		id = id,
		frames = frame,
	}
	local position_keys = channel.position_keys
	for i = 1, #position_keys do
		frame[i] = { position = position_keys[i], rotation = nil, scale = nil}
	end
	local rotation_keys = channel.rotation_keys
	for i = 1, #rotation_keys do
		frame[i].rotation = rotation_keys[i]
	end
	local scaling_keys = channel.scaling_keys
	for i = 1, #scaling_keys do
		frame[i].scale = scaling_keys[i]
	end
	return bone
end

local channels = clip_data.node_channels
for i = 1, #channels do
	list[i] = parse_frame(convert[i], channels[i])
end
table.sort(list, function(a, b) return a.id < b.id end)

print(format("frame_count:%s", clip_data.total_frame))
print(format("bone_count:%s", clip_data.node_count))
local buf = {}
local line = {}
for i = 1, #list do
		line[i] = list[i].id
end
print(concat(line, ","))
for i = 1, clip_data.total_frame do
	local line = {}
	for j = 1, #list do
		local frame = list[j].frames[i]
		if not frame then
			frame = {position = {}, rotation = {}, scale = {}}
		end
		line[j] = string.format("%s:%s:%s&%s:%s:%s&%s:%s:%s:%s", 
			frame.position.x or 0, frame.position.y or 0, frame.position.z or 0,
			frame.scale.x or 1, frame.scale.y or 1, frame.scale.z or 1,
			frame.rotation.x or 0, frame.rotation.y or 0, frame.rotation.z or 0, frame.rotation.w or 0)
	end	
	print(concat(line, ","))
end


