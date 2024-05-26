GameState = {}
GameState.__index = GameState

function GameState:Create(stack)
	local this =
	{
		m_Stack = stack,
        m_IsFacingLeft = false,
        m_SceneDimmer = SceneDimmer:Create(nil)
	}

	local state = State("game state")
	state:set_variable_table(this)
	state:set_on_enter(function() this:OnEnter() end)
	state:set_on_exit(function() this:OnExit() end)
	state:set_on_update(function(dt) this:OnUpdate(dt) end)
	state:set_on_render(function() this:OnRender() end)
	state:set_handle_inputs(function() this:HandleInputs() end)

	setmetatable(this, self)

    this:Initialize()

	return state
end

function GameState:Initialize()
    -- Create events
    if not gCollisionEvent then
        gCollisionEvent = CollisionEvent:Create()
    end

    if not gTriggerSystem then
        gTriggerSystem = TriggerSystem:Create()
    end

    gCollisionEvent:SubscribeToEvent(gTriggerSystem)

    -- Create the player
    if not gPlayer then
        gPlayer = Entity("player", "")

        local playerTransform = gPlayer:add_component(Transform(vec2(10 * 16, 33 * 16), vec2(1, 1), 0))
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
        playerPhysAttrs.objectData = (ObjectData("player", "", true, false, gPlayer:id()))
        gPlayer:add_component(PhysicsComponent(playerPhysAttrs))
    end

    if not gFollowCam then
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
    end
end

function GameState:OnEnter()
    LoadMap(CreateTestPlatformerMap())

    Sound.play("rain", -1, 1)
    Sound.set_volume(1, 50)

    Music.play("ambience_song", -1)
    Music.set_volume(40)
end

function GameState:OnExit()
end

function GameState:OnUpdate(dt)
    self:UpdatePlayer(gPlayer)
    self:UpdateContacts()
    gFollowCam:update()
    self.m_SceneDimmer.m_OnUpdate()
    self.m_SceneDimmer:UpdateRainGen(dt)
end

function GameState:OnRender()
end

function GameState:HandleInputs()
	if Keyboard.just_released(KEY_BACKSPACE) then
		self.m_Stack:pop()
		return
	end
end

function GameState:UpdatePlayer(player)
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

	sprite:inpect_y()
end

function GameState:UpdateContacts()
    local uda, udb = ContactListener.get_user_data()
	if uda and udb then
		gCollisionEvent:EmitEvent(uda, udb)
	end
end
