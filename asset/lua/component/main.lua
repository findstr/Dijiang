local component = require "engine.component"
local input = require "engine.input"
local mathx = require "engine.math"
local M = {}
local touch = {}

local camera_up = {x = 0, y = 0, z = 0}
local camera_forward = {x = 0, y = 0, z = 0}
local camera_right = {x = 0, y = 0, z = 0}

function M:start()
	local stk<close> = mathx.begin()
	print("rotation", component.get_quaternion(self))
	local rot = stk:quaternion(component.get_quaternion(self))
	local up = stk:mul(rot, stk:vector3f_up())
	local forward = stk:mul(rot, stk:vector3f_forward())
	local right = stk:mul(rot, stk:vector3f_right())
	stk:save(up, camera_up)
	stk:save(right, camera_right)
	stk:save(forward, camera_forward)
	print("===================== camera up", camera_up.x, camera_up.y, camera_up.z)
	print("===================== camera right", camera_right.x, camera_right.y, camera_right.z)
	print("===================== camera forward", camera_forward.x, camera_forward.y, camera_forward.z)
end

function M:pre_tick(delta)

end

function M:tick(delta)
	local speed = 300.0 
	local rot_speed = 50.0
	local touch_count = input.touch_count()
	if touch_count > 0 then
		local stk<close> = mathx.begin()
		input.touch_info(0, touch)
		local dx = math.abs(touch.delta_x)
		local dy = math.abs(touch.delta_y)
		if dx > dy and dx > 0.0001 then
			local rot = stk:quaternion_from_axis_angle(stk:vector3f_up(), touch.delta_x * rot_speed)
			local up = stk:mul(rot, stk:vector3f(camera_up))
			local right = stk:mul(rot, stk:vector3f(camera_right))
			local forward = stk:mul(rot, stk:vector3f(camera_forward))
			local newq = stk:quaternion_look_at(forward, up)
			component.set_quaternion(self, stk:save(newq))
			stk:save(up, camera_up)
			stk:save(right, camera_right)
			stk:save(forward, camera_forward)
			print("===================== camera forward", camera_forward.x, camera_forward.y, camera_forward.z)
		elseif dx < dy and dy > 0.0001 then
			local right = stk:vector3f(camera_right) 
			local rot = stk:quaternion_from_axis_angle(right, touch.delta_y * rot_speed)
			local up = stk:mul(rot, stk:vector3f(camera_up))
			local forward = stk:mul(rot, stk:vector3f(camera_forward))
			local newq = stk:quaternion_look_at(forward, up)
			component.set_quaternion(self, stk:save(newq))
			stk:save(up, camera_up)
			stk:save(forward, camera_forward)
			print("===================== camera up", camera_up.x, camera_up.y, camera_up.z)
		end
	end
	if input.get_key(input.A) == "down" then
		delta = delta
		component.move(self,  delta * camera_right.x, delta * camera_right.y, delta * camera_right.z)
	end
	if input.get_key(input.D) == "down" then
		delta = -1 * delta
		component.move(self,  delta * camera_right.x, delta * camera_right.y, delta * camera_right.z)
	end
	if input.get_key(input.W) == "down" then
		component.move(self,  delta * camera_forward.x, delta * camera_forward.y, delta * camera_forward.z)
	end
	if input.get_key(input.S) == "down" then
		delta = -1 * delta
		component.move(self,  delta * camera_forward.x, delta * camera_forward.y, delta * camera_forward.z)
	end
end

function M:post_tick(delta)

end

return M

