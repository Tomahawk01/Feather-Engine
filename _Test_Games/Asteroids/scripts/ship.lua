Ship = {}
Ship.__index = Ship

function Ship:Create(def)
    local this =
    {
        m_EntityID = def.id,
        m_ForwardSpeed = def.fwd_speed or 7,
        m_AngularSpeed = def.ang_speed or 4,
        m_DriftSpeed = def.drift_speed or 1,
        m_DriftAngle = vec2(math.cos(0), math.sin(0)),
        m_Cooldown = def.cool_down or 160,
        m_CooldownTimer = Timer(),
        m_DeathTimer = Timer(),
        m_InvincibleTimer = Timer(),
        m_Explode = false,
        m_Dead = false,
        m_NumLives = gData:NumLives()
    }

    setmetatable(this, self)
    return this
end

function Ship:Update()
    if self.m_Dead then
        return
    end

    if self.m_Explode then
        self:UpdateExplosion()
        return
    end

    local ship = Entity(self.m_EntityID)
    local transform = ship:get_component(Transform)
    local sprite = ship:get_component(Sprite)

    local forward = vec2(
        math.cos(math.rad(transform.rotation)),     -- X
        math.sin(math.rad(transform.rotation))      -- Y
    )
    local backward = vec2(
        -math.cos(math.rad(transform.rotation)),    -- X
        -math.sin(math.rad(transform.rotation))     -- Y
    )

    local speedUp = 1.0
    if Keyboard.pressed(KEY_LSHIFT) then
        speedUp = 1.5
    end

    if Keyboard.pressed(KEY_A) then
        transform.rotation = transform.rotation - self.m_AngularSpeed * speedUp
    elseif Keyboard.pressed(KEY_D) then
        transform.rotation = transform.rotation + self.m_AngularSpeed * speedUp
    end

    if Keyboard.pressed(KEY_W) then
        transform.position = transform.position + forward * self.m_ForwardSpeed * speedUp
        self.m_DriftAngle = forward
    elseif Keyboard.pressed(KEY_S) then
        transform.position = transform.position - forward * self.m_ForwardSpeed * speedUp
        self.m_DriftAngle = backward
    else
        transform.position = transform.position + self.m_DriftAngle * self.m_DriftSpeed
    end

    if not self.m_CooldownTimer:is_running() then
        if Keyboard.just_pressed(KEY_SPACE) or Mouse.just_pressed(LEFT_BUTTON) then
            local projectile = Projectile:Create(
                {
                    def = "proj_1",
                    dir = forward,
                    start_pos = vec2(
                        transform.position.x + sprite.width / 2,
                        transform.position.y + sprite.height / 2
                    ),
                    rotation = transform.rotation
                }
            )
            self.m_CooldownTimer:start()
            AddProjectile(projectile)
            Sound.play("laser")
        end
    elseif self.m_CooldownTimer:elapsed_ms() >= self.m_Cooldown then
        self.m_CooldownTimer:stop()
    end

    self:CheckDeath()

    CheckPos(transform.position, sprite.width, sprite.height)
end

function Ship:CheckDeath()
    local numLives = gData:NumLives()

    if self.m_NumLives ~= numLives and not self.m_Explode then
        self.m_Explode = true

        local ship = Entity(self.m_EntityID)
        -- Set Animation
        local animation = ship:get_component(Animation)
        animation.num_frames = 54
        animation.frame_rate = 35
        animation:reset()

        -- Set Sprite
        local sprite = ship:get_component(Sprite)
        sprite.texture_name = "ship_explosion"
        sprite.width = 100
        sprite.height = 100
        sprite:generate_uvs()

        -- Set Scale
        local transform = ship:get_component(Transform)
        transform.scale = vec2(2, 2)

        self.m_NumLives = numLives
        self.m_DeathTimer:start()
        Sound.play("ship_explosion")
        Sound.set_volume(-1, 40)
    end

    if self.m_InvincibleTimer:is_running() then
        local ship = Entity(self.m_EntityID)
        -- "Ghost" ship
        local sprite = ship:get_component(Sprite)
        sprite.color.a = 150

        if self.m_InvincibleTimer:elapsed_ms() > 3000 then
            local collider = ship:get_component(CircleCollider)
            collider.colliding = false
            self.m_InvincibleTimer:stop()
            sprite.color.a = 255
        end
    end
end

function Ship:UpdateExplosion()
    if self.m_Explode and self.m_DeathTimer:elapsed_ms() > 1000 then
        local ship = Entity(self.m_EntityID)
        local animation = ship:get_component(Animation)

        if animation.current_frame >= animation.num_frames - 1 then
            -- Reset Animations
            animation.num_frames = 0
            animation.frame_rate = 0
            animation.current_frame = 0

            -- Reset Sprite
            local sprite = ship:get_component(Sprite)
            sprite.texture_name = "ship"
            sprite.width = 84
            sprite.height = 136
            sprite:generate_uvs()
            sprite.start_x = 0

            -- Reset Scale
            local transform = ship:get_component(Transform)
            transform.scale = vec2(1, 1)

            -- Reset explosion
            self.m_Explode = false
            self.m_DeathTimer:stop()
            self.m_InvincibleTimer:start()

            if gData:NumLives() == 0.0 then
                if not self.m_Dead then
                    sprite.hidden = true
                end
                self.m_Dead = true
            end
        end
    end
end

function Ship:Reset()
    self.m_Explode = false
    self.m_Dead = false
    self.m_NumLives = gData:NumLives()
    self.m_DeathTimer:stop()
    self.m_InvincibleTimer:stop()
    self.m_CooldownTimer:stop()

    -- Reset Sprite and Circle collider
    local ship = Entity(self.m_EntityID)
    local sprite = ship:get_component(Sprite)
    sprite.hidden = false

    local circle_collider = ship:get_component(CircleCollider)
    circle_collider.colliding = false
end
