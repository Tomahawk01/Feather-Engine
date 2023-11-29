Hud = {}
Hud.__index = Hud

function Hud:Create()
    local this =
    {
        m_Lives = {},
        m_CurrentLives = gData:MaxLives(),
        m_Score = 0
    }

    -- Create Lives entities
    for i = 0, gData:MaxLives() - 1 do
        local livesID = LoadEntity(HudDefs["lives"])
        local livesEntity = Entity(livesID)
        local transform = livesEntity:get_component(Transform)
        transform.position = vec2(40 * i, 0)
        table.insert(this.m_Lives, livesID)
    end

    -- Score entity data
    this.m_ScoreID = LoadEntity(HudDefs["score"])
    this.m_TenThousandID = LoadEntity(HudDefs["number"])
    this.m_ThousandID = LoadEntity(HudDefs["number"])
    this.m_HundredID = LoadEntity(HudDefs["number"])
    this.m_TensID = LoadEntity(HudDefs["number"])
    this.m_OnesID = LoadEntity(HudDefs["number"])

    -- Game over entity data
    this.m_GameOver = false
    this.m_ShowGameOver = false
    this.m_GameOverID = LoadEntity(HudDefs["game_over"])

    setmetatable(this, self)

    this:SetNumberPositions()

    return this
end

function Hud:SetNumberPositions()
    local transform1 = Entity(self.m_TenThousandID):get_component(Transform)
    local transform2 = Entity(self.m_ThousandID):get_component(Transform)
    transform2.position.x = transform1.position.x + (19 * transform2.scale.x)
    local transform3 = Entity(self.m_HundredID):get_component(Transform)
    transform3.position.x = transform2.position.x + (19 * transform3.scale.x)
    local transform4 = Entity(self.m_TensID):get_component(Transform)
    transform4.position.x = transform3.position.x + (19 * transform4.scale.x)
    local transform5 = Entity(self.m_OnesID):get_component(Transform)
    transform5.position.x = transform4.position.x + (19 * transform5.scale.x)
end

function Hud:Update()
    if not self.m_GameOver then
        self:UpdateLives()
        self:UpdateScore()
    elseif not self.m_ShowGameOver then
        local entity = Entity(self.m_GameOverID)
        local sprite = entity:get_component(Sprite)
        sprite.hidden = false
        self.m_ShowGameOver = true
    end
end

function Hud:UpdateLives()
    local numLives = gData:NumLives()
    if self.m_CurrentLives ~= numLives then
        for k, v in pairs(self.m_Lives) do
            local sprite = Entity(self.m_Lives[k]):get_component(Sprite)
            if k <= numLives then
                sprite.hidden = false
            else
                sprite.hidden = true
            end
        end
        self.m_CurrentLives = numLives
    end

    if numLives == 0.0 then
        self.m_GameOver = true
    end
end

function Hud:UpdateScore()
    local current_score = gData:GetScore()
    if current_score == self.m_Score then
        return
    end

    self:SetScore(current_score)
end

function Hud:SetScore(current_score)
    local ten_thousands = GetDigit(current_score, 5)
    local thousands = GetDigit(current_score, 4)
    local hundreds = GetDigit(current_score, 3)
    local tens = GetDigit(current_score, 2)
    local ones = GetDigit(current_score, 1)

    -- Set sprite UVs
    local sprite1 = Entity(self.m_TenThousandID):get_component(Sprite)
    sprite1.start_x = ten_thousands
    sprite1.uvs.u = sprite1.uvs.uv_width * sprite1.start_x

    local sprite2 = Entity(self.m_ThousandID):get_component(Sprite)
    sprite2.start_x = thousands
    sprite2.uvs.u = sprite2.uvs.uv_width * sprite2.start_x

    local sprite3 = Entity(self.m_HundredID):get_component(Sprite)
    sprite3.start_x = hundreds
    sprite3.uvs.u = sprite3.uvs.uv_width * sprite3.start_x

    local sprite4 = Entity(self.m_TensID):get_component(Sprite)
    sprite4.start_x = tens
    sprite4.uvs.u = sprite4.uvs.uv_width * sprite4.start_x

    local sprite5 = Entity(self.m_OnesID):get_component(Sprite)
    sprite5.start_x = ones
    sprite5.uvs.u = sprite5.uvs.uv_width * sprite5.start_x

    -- Set the score
    self.m_Score = current_score
end

function Hud:Reset()
    self.m_GameOver = false
    self.m_ShowGameOver = false
    self:SetScore(0)

    -- Hide game over sprite
    local sprite = Entity(self.m_GameOverID):get_component(Sprite)
    sprite.hidden = true
end
