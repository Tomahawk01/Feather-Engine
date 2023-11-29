GameData = {}
GameData.__index = GameData

function GameData:Create()
    local this =
    {
        m_MaxLives = 5,
        m_NumLives = 3,
        m_NumAsteroids = 0,
        m_Score = 0
    }

    setmetatable(this, self)
    return this
end

function GameData:IsGameOver()
    return self.m_NumLives == 0
end

function GameData:MaxLives()
    return self.m_MaxLives
end

function GameData:NumLives()
    return self.m_NumLives
end

function GameData:AddLife()
    self.m_NumLives = clamp(self.m_NumLives + 1, 0, self.m_MaxLives)
end

function GameData:RemoveLife()
    self.m_NumLives = clamp(self.m_NumLives - 1, 0, self.m_MaxLives)
end

gData = GameData:Create()
