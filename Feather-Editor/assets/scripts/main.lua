-- Main Lua Script!

gEntity = Entity("TestEntity", "GroupTest")
gEntity2 = Entity("Second_TestEntity", "GroupTest")

gEntity2:add_component(Transform(200, 100, 10, 10, 0))
local sprite2 = gEntity2:add_component(Sprite("gem_animation", 32.0, 32.0, 0, 0, 0))
gEntity2:add_component(Animation(18, 10, 0, false))
sprite2:generate_uvs()

local transform = gEntity:add_component(Transform(vec2(100, 100), vec2(10, 10), 0))
local sprite = gEntity:add_component(Sprite("gem", 32.0, 32.0, 0, 0, 0))
sprite:generate_uvs()

local rotation = 0.0
local x_pos = 10.0
local scale = 1.0
local move_right = true
local value = 0

local view = Registry.get_entities(Transform)

view:exclude(Sprite)

view:for_each(
	function (entity)
		print(entity:name())
	end
)

main = {
	[1] = {
		update = function()
			if rotation >= 360 then
				rotation = 0
			end

			if move_right and x_pos < 300 then
				x_pos = x_pos + 3
				value = value + 0.01
			elseif move_right and x_pos >= 300 then
				move_right = false
			end

			if not move_right and x_pos > 10 then
				x_pos = x_pos - 3
				value = value - 0.01
			elseif not move_right and x_pos <= 10 then
				move_right = true
			end

			transform.position.x = x_pos
			transform.rotation = rotation

			if move_right then
				rotation = rotation + 9
			else
				rotation = rotation - 9
			end

			scale = math.sin(value) * 10

			if scale >= 10 then
				scale = 10
			elseif scale <= 1 then
				scale = 1
			end

			if value >= 1 then
				value = 1
			elseif value <= 0 then
				value = 0
			end

			transform.scale = vec2(scale, scale)
			
			local r_stick_LR = Gamepad.get_axis_position(1, 2)
			print("r_stick LeftRight value: " ..r_stick_LR)
			local r_stick_UD = Gamepad.get_axis_position(1, 3)
			print("r_stick UpDown value: " ..r_stick_UD)

			local l_stick_LR = Gamepad.get_axis_position(1, 0)
			print("l_stick LeftRight value: " ..l_stick_LR)
			local l_stick_UD = Gamepad.get_axis_position(1, 1)
			print("l_stick UpDown value: " ..l_stick_UD)

			local l_trigger = Gamepad.get_axis_position(1, 4)
			print("l_trigger value: " ..l_trigger)
			local r_trigger = Gamepad.get_axis_position(1, 5)
			print("r_trigger value: " ..r_trigger)
		end
	},
	[2] = {
		render = function()
			
		end
	},
}
