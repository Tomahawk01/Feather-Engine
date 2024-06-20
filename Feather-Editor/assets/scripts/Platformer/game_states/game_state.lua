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
        local character = Character:Create({name = "player"})
		gPlayer = Entity(character.m_EntityID)
		AddActiveCharacter(gPlayer:id(), character)
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
    self:UpdateContacts()
	UpdateActiveCharacters(dt)
    gFollowCam:update()
    self.m_SceneDimmer.m_OnUpdate()
    self.m_SceneDimmer:UpdateRainGen(dt)
	gTriggerSystem:Update()
end

function GameState:OnRender()
end

function GameState:HandleInputs()
	if Keyboard.just_released(KEY_BACKSPACE) then
		self.m_Stack:pop()
		return
	end
end

function GameState:UpdateContacts()
    local uda, udb = ContactListener.get_user_data()
	if uda and udb then
		gCollisionEvent:EmitEvent(uda, udb)
	end
end
