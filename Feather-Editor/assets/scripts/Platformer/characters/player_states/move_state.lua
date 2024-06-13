MoveState = {}
MoveState.__index = MoveState

function MoveState:Create(character)
    assert(character)
    local this =
    {
        m_Character = character,
        m_EntityID = character.m_EntityID,
        m_Controller = character.m_Controller,
        m_JumpSteps = 10,
        m_NumJumps = 0,
        m_IsAttacking = false
    }

    local state = State("move")
    state:set_variable_table(this)
    state:set_on_enter(function(...) this:OnEnter(...) end)
    state:set_on_exit(function(...) this:OnExit(...) end)
    state:set_on_update(function(dt) this:OnUpdate(dt) end)
    state:set_on_render(function() end)

    setmetatable(this, self)
    return state
end

function MoveState:OnEnter()
end

function MoveState:OnExit()
end

function MoveState:OnUpdate(dt)
    local player = Entity(self.m_EntityID)
    local physics = player:get_component(PhysicsComponent)
	local velocity = physics:get_linear_velocity()
	local sprite = player:get_component(Sprite)

	physics:set_linear_velocity(vec2(0, velocity.y))

	if Keyboard.pressed(KEY_A) then
		physics:set_linear_velocity(vec2(-10, velocity.y))
		self.m_Character.m_IsFacingLeft = true
		sprite.start_y = 3
	elseif Keyboard.pressed(KEY_D) then
		physics:set_linear_velocity(vec2(10, velocity.y))
		self.m_Character.m_IsFacingLeft = false
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
		if self.m_Character.m_IsFacingLeft then
			sprite.start_y = 1
		else
			sprite.start_y = 0
		end
	end

	sprite:inpect_y()
end
