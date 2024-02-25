-- Main Lua Script!

run_script("assets/scripts/TestProject/assetDefs.lua")
run_script("assets/scripts/TestProject/testmap.lua")
run_script("assets/scripts/utilities.lua")

-- Create a Gem
local gem = Entity("", "")
local circle = gem:add_component(CircleCollider(16.0))
local transform = gem:add_component(Transform(vec2(320, 64), vec2(2, 2), 0))

local physAttrs = PhysicsAttributes()
physAttrs.type = BodyType.Dynamic
physAttrs.density = 1000.0
physAttrs.friction = 0.5
physAttrs.restitution = 0.2
physAttrs.radius = circle.radius * (1.0 / 12.0)
physAttrs.gravityScale = 2.0
physAttrs.position = transform.position
physAttrs.scale = transform.scale
physAttrs.isCircle = true
physAttrs.isFixedRotation = false

gem:add_component(PhysicsComponent(physAttrs))

local sprite = gem:add_component(Sprite("TestGem", 32, 32, 0, 0, 0))
sprite:generate_uvs()

-- Create a bottom box
local bottomEnt = Entity("", "")
local bottomBox = bottomEnt:add_component(BoxCollider(624, 16, vec2(0, 0)))
local bottomTransform = bottomEnt:add_component(Transform(vec2(0, 464), vec2(1, 1), 0))

local bottomPhys = PhysicsAttributes()
bottomPhys.type = BodyType.Static
bottomPhys.density = 1000.0
bottomPhys.friction = 0.5
bottomPhys.restitution = 0.0
bottomPhys.position = bottomTransform.position
bottomPhys.scale = bottomTransform.scale
bottomPhys.boxSize = vec2(bottomBox.width, bottomBox.height)
bottomPhys.isBox = true
bottomPhys.isFixedRotation = true

bottomEnt:add_component(PhysicsComponent(bottomPhys))

-- Create a left box
local leftEnt = Entity("", "")
local leftBox = leftEnt:add_component(BoxCollider(16, 464, vec2(0, 0)))
local leftTransform = leftEnt:add_component(Transform(vec2(0, 0), vec2(1, 1), 0))

local leftPhys = PhysicsAttributes()
leftPhys.type = BodyType.Static
leftPhys.density = 1000.0
leftPhys.friction = 0.5
leftPhys.restitution = 0.0
leftPhys.position = leftTransform.position
leftPhys.scale = leftTransform.scale
leftPhys.boxSize = vec2(leftBox.width, leftBox.height)
leftPhys.isBox = true
leftPhys.isFixedRotation = true

leftEnt:add_component(PhysicsComponent(leftPhys))

-- Create a right box
local rightEnt = Entity("", "")
local rightBox = rightEnt:add_component(BoxCollider(16, 480, vec2(0, 0)))
local rightTransform = rightEnt:add_component(Transform(vec2(624, 0), vec2(1, 1), 0))

local rightPhys = PhysicsAttributes()
rightPhys.type = BodyType.Static
rightPhys.density = 1000.0
rightPhys.friction = 0.5
rightPhys.restitution = 0.0
rightPhys.position = rightTransform.position
rightPhys.scale = rightTransform.scale
rightPhys.boxSize = vec2(rightBox.width, rightBox.height)
rightPhys.isBox = true
rightPhys.isFixedRotation = true

rightEnt:add_component(PhysicsComponent(rightPhys))

-- Create a top box
local topEnt = Entity("", "")
local topBox = topEnt:add_component(BoxCollider(608, 16, vec2(0, 0)))
local topTransform = topEnt:add_component(Transform(vec2(16, 0), vec2(1, 1), 0))

local topPhys = PhysicsAttributes()
topPhys.type = BodyType.Static
topPhys.density = 1000.0
topPhys.friction = 0.5
topPhys.restitution = 0.0
topPhys.position = topTransform.position
topPhys.scale = topTransform.scale
topPhys.boxSize = vec2(topBox.width, topBox.height)
topPhys.isBox = true
topPhys.isFixedRotation = true

topEnt:add_component(PhysicsComponent(topPhys))
-- ============================================================================

local ballCount = 0
local countEnt = Entity("", "")
countEnt:add_component(Transform(vec2(10, 32), vec2(1, 1), 0))
countEnt:add_component(TextComponent("testFont", "Ball Count: ", Color(255, 255, 255, 255), 4, -1.0))

local valEnt = Entity("", "")
valEnt:add_component(Transform(vec2(150, 32), vec2(1, 1), 0))
local valText = valEnt:add_component(TextComponent("testFont", "0", Color(255, 255, 255, 255), 4, -1.0))

function createGem()
	if (Mouse.just_released(LEFT_BUTTON)) then
		local pos_x, pos_y = Mouse.screen_position()
		local gem = Entity("", "")
		local circle = gem:add_component(CircleCollider(16.0))
		local transform = gem:add_component(Transform(vec2(pos_x, pos_y), vec2(1, 1), 0))

		local physAttrs = PhysicsAttributes()
		physAttrs.type = BodyType.Dynamic
		physAttrs.density = 1000.0
		physAttrs.friction = 0.5
		physAttrs.restitution = 0.2
		physAttrs.radius = circle.radius * (1.0 / 12.0)
		physAttrs.gravityScale = 2.0
		physAttrs.position = transform.position
		physAttrs.scale = transform.scale
		physAttrs.isCircle = true
		physAttrs.isFixedRotation = false

		gem:add_component(PhysicsComponent(physAttrs))

		local sprite = gem:add_component(Sprite("TestGem", 32, 32, 0, 0, 0))
		sprite:generate_uvs()

		ballCount = ballCount + 1
	end
end

function updateEntity(entity)
	local physics = entity:get_component(PhysicsComponent)
	local transform = entity:get_component(Transform)

	local velocity = physics:get_linear_velocity()

	-- if velocity.y > 0.0 then
	-- 	physics:set_gravity_scale(15.0)
	-- else
	-- 	physics:set_gravity_scale(2.0)
	-- end

	if Keyboard.pressed(KEY_D) then
		physics:set_linear_velocity(vec2(25, velocity.y))
	elseif Keyboard.pressed(KEY_A) then
		physics:set_linear_velocity(vec2(-25, velocity.y))
	end

	if Keyboard.just_pressed(KEY_SPACE) then
		physics:set_linear_velocity(vec2(velocity.x, 0))
		physics:linear_impulse(vec2(0, -300000))
	end
end

main = {
	[1] = {
		update = function()
			createGem()
			updateEntity(gem)

			valText.textStr = tostring(ballCount)
		end
	},
	[2] = {
		render = function()

		end
	}
}
