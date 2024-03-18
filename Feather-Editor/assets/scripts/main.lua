-- Main Lua Script!

run_script("assets/scripts/TestProject/assetDefs.lua")
run_script("assets/scripts/TestProject/testmap.lua")
run_script("assets/scripts/TestProject/test_platformer.lua")
run_script("assets/scripts/utilities.lua")
run_script("assets/scripts/rain_generator.lua")

local tilemap = CreateTestPlatformerMap()
assert(tilemap)
LoadAssets(AssetDefs)
LoadMap(tilemap)

local rainGen = RainGenerator:Create()
Sound.play("rain", -1, 1)
Sound.set_volume(1, 50)

Music.play("ambience_song", -1)
Music.set_volume(40)

-- Create lightning and darkness
local darkness = Entity("", "")
darkness:add_component(Transform(vec2(0, 0), vec2(WindowWidth(), WindowHeight()), 0))

local darkSprite = darkness:add_component(Sprite("white_box", 16, 16, 0, 0, 4))
darkSprite.color = Color(0, 0, 0, 100)
darkSprite:generate_uvs()

gTimer = Timer()
gTimer:start()

function UpdateLightning(entity)
	local darkSprite = entity:get_component(Sprite)

	if gTimer:elapsed_ms() > 3000 then
		darkSprite.color = Color(255, 255, 255, 25)
	end

	if gTimer:elapsed_ms() > 3100 and gTimer:elapsed_ms() < 3200 then
		darkSprite.color = Color(0, 0, 0, 100)
	elseif gTimer:elapsed_ms() > 3200 and gTimer:elapsed_ms() < 3400 then
		darkSprite.color = Color(255, 255, 255, 25)
	elseif gTimer:elapsed_ms() > 3400 then
		darkSprite.color = Color(0, 0, 0, 100)
		gTimer:stop()
		gTimer:start()
	end
end

-- Create Player
gPlayer = Entity("player", "")
local playerTransform = gPlayer:add_component(Transform(vec2(20 * 16, 32 * 16), vec2(1, 1), 0))
local sprite = gPlayer:add_component(Sprite("player", 16, 16, 0, 0, 2))
sprite:generate_uvs()
gPlayer:add_component(Animation(4, 6, 0, false, true))
local circleCollider = gPlayer:add_component(CircleCollider(8))

local playerPhysAttrs = PhysicsAttributes()
playerPhysAttrs.type = BodyType.Dynamic
playerPhysAttrs.density = 75.0
playerPhysAttrs.friction = 1.0
playerPhysAttrs.restitution = 0.0
playerPhysAttrs.position = playerTransform.position
playerPhysAttrs.radius = circleCollider.radius
playerPhysAttrs.isCircle = true
playerPhysAttrs.isFixedRotation = true
gPlayer:add_component(PhysicsComponent(playerPhysAttrs))

isFacingLeft = false

function UpdatePlayer(player)
	local physics = player:get_component(PhysicsComponent)
	local velocity = physics:get_linear_velocity()
	local sprite = player:get_component(Sprite)

	physics:set_linear_velocity(vec2(0, velocity.y))

	if Keyboard.pressed(KEY_A) then
		physics:set_linear_velocity(vec2(-10, velocity.y))
		isFacingLeft = true
		sprite.start_y = 3
	elseif Keyboard.pressed(KEY_D) then
		physics:set_linear_velocity(vec2(10, velocity.y))
		isFacingLeft = false
		sprite.start_y = 2
	end

	if Keyboard.just_pressed(KEY_SPACE) then
		physics:set_linear_velocity(vec2(velocity.x, 0))
		physics:linear_impulse(vec2(velocity.x, -1250))
	end

	if velocity.y < 0 then
		physics:set_gravity_scale(2)
	elseif velocity.y > 0 then
		physics:set_gravity_scale(5)
	end

	-- Reset to Idle animation
	if velocity.x == 0.0 then
		if isFacingLeft then
			sprite.start_y = 1
		else
			sprite.start_y = 0
		end
	end

	sprite.uvs.v = sprite.start_y * sprite.uvs.uv_height
end

-- Follow camera
Camera.get().set_scale(2.0)
gFollowCam = FollowCamera(
	FollowCamParams(
		{
			min_x = 0,
			min_y = 0,
			max_x = 960,
			max_y = 960,
			scale = 2
		}
	), gPlayer
)

main = {
	[1] = {
		update = function()
			UpdatePlayer(gPlayer)
			gFollowCam:update()
			rainGen:Update(0.016)
			UpdateLightning(darkness)

			Debug()
		end
	},
	[2] = {
		render = function()

		end
	}
}
