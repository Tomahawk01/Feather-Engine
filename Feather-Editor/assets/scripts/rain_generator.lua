Rain = {}
Rain.__index = Rain

function Rain:Create(params)
    params = params or {}
    local this =
    {
        m_InitialPosition = params.position or vec2(0, 0),
		m_Velocity = params.velocity or vec2(200, 200),
		m_MinVelocity = params.min_velocity or 100,
		m_MaxVelocity = params.max_velocity or 400,
		m_LifeTime = params.life_time or 1000, -- In ms 
		m_Texture = params.texture or "rain",
		m_Width = params.width or 32,
		m_Height = params.height or 16,
		m_Alpha = params.alpha or 135, -- semi-transparent
		m_Scale = params.scale or 1,
		m_NumFrames = params.num_frames or 5, 
		m_MinLifeTime = params.min_life_time or 0,
		m_MaxLifeTime = params.max_life_time or 1500,
		m_LifeTimer = Timer()
    }

    this.m_StartFinish = false

    -- Create entity
    this.m_Entity = Entity("", "rain")
    this.m_Entity:add_component(Transform(this.m_InitialPosition, vec2(this.m_Scale, this.m_Scale), 0))
    local sprite = this.m_Entity:add_component(Sprite(this.m_Texture, this.m_Width, this.m_Height, 0, 0, 6))
    sprite.color = Color(255, 255, 255, this.m_Alpha)
    sprite:generate_uvs()

    local animation = this.m_Entity:add_component(Animation(0, 10, 0, false, false))
	this.m_Entity:add_component(RigidBody(this.m_Velocity))

    this.m_LifeTimer:start() -- Start Lifetime Timer

    setmetatable(this, self)
	return this
end

function Rain:LifeOver()
	return self.m_LifeTimer:elapsed_ms() > self.m_LifeTime
end

function Rain:Update(dt)
	if not self.m_Entity then
		return 
	end

	local transform = self.m_Entity:get_component(Transform)
	local rigid_body = self.m_Entity:get_component(RigidBody)

	transform.position = transform.position + (rigid_body.velocity * dt)
end

function Rain:Finish()
	if not self.m_Entity then
		return 
	end

	local rigid_body = self.m_Entity:get_component(RigidBody)
	local animation = self.m_Entity:get_component(Animation)

	if not self.m_StartFinish then
		rigid_body.velocity = vec2(0, 0) -- stop moving
		animation.num_frames = self.m_NumFrames
		self.m_StartFinish = true
	else 
		-- Reset rain to the begining position
		if animation.current_frame == animation.num_frames - 1 then
			animation:reset()
			animation.num_frames = 0 
			local transform = self.m_Entity:get_component(Transform)
			local sprite = self.m_Entity:get_component(Sprite)
			sprite.uvs.u = 0
			transform.position = self.m_InitialPosition 

			-- Change velocity to a random value between the min and max
			local rain_velocity = Random(self.m_MinVelocity, self.m_MaxVelocity):get_float()
			rigid_body.velocity = vec2(rain_velocity, rain_velocity)

			-- Adjust lifetime to a random value between the min and max
			self.m_LifeTime = Random(self.m_MinLifeTime, self.m_MaxLifeTime):get_float()

			-- Reset the Lifetime timer
			self.m_LifeTimer:stop()
			self.m_LifeTimer:start() 
			self.m_StartFinish = false
		end
	end
end

function Rain:Destroy()
	self.m_Entity:kill()
	self.m_Entity = nil
end

RainGenerator = {}
RainGenerator.__index = RainGenerator

function RainGenerator:Create(params)
	params = params or {}

	local this = 
	{
		m_xOffset = params.x_offset or 32,
		m_yOffset = params.y_offset or 32,
		m_RainVelMin = params.rain_vel_min or 100,
		m_RainVelMax = params.rain_vel_max or 200,
		m_RainLifeMin = params.rain_life_min or 500,
		m_RainLifeMax = params.rain_life_max or 1500,
		m_Scale = params.scale or 1
	}

	this.m_RainTable = {}

	setmetatable(this, self)

	-- Generate the rain drops
	this:ReGenerate()

	return this 
end

function RainGenerator:Update(dt)
	for k, v in pairs(self.m_RainTable) do
		v:Update(dt)

		if v:LifeOver() then
			v:Finish()
		end
	end 
end

function RainGenerator:ReGenerate()
	if #self.m_RainTable == 0 then
		local camScale = Camera.get():scale()
		local maxWidth = WindowWidth() * camScale
		local maxHeight = WindowHeight() * camScale

		local rows = maxHeight / self.m_yOffset
		local cols = maxWidth / self.m_xOffset

		-- Populate rain
		for i = 0, rows do
			for j = 0, cols do
				local start_x = clamp(self.m_xOffset * j, 0, maxWidth)
				local start_y = clamp(self.m_yOffset * i, 0, maxHeight)
				local rain_velocity = Random(self.m_RainVelMin, self.m_RainVelMax):get_float()
				local rain = Rain:Create(
					{
						position = vec2(start_x, start_y),
						velocity = vec2(rain_velocity, rain_velocity),
						life_time = Random(self.m_RainLifeMin, self.m_RainLifeMax):get_float(),
						scale = self.m_Scale,
						num_frames = 1
					}
				)
				table.insert(self.m_RainTable, rain)
			end
		end
	end
end

function RainGenerator:Destroy()
	for k, v in ipairs(self.m_RainTable) do
		v:Destroy()
		self.m_RainTable[k] = nil
	end
end
