function LoadEntity(def)
    assert(def, "Definition does not exist!")

    local tag = ""
    if def.tag then
        tag = def.tag
    end

    local group = ""
    if def.group then
        group = def.group
    end

    local newEntity = Entity(tag, group)

    if def.components.transform then
        newEntity:add_component(
            Transform(
                vec2(
                    def.components.transform.position.x,
                    def.components.transform.position.y
                ),
                vec2(
                    def.components.transform.scale.x,
                    def.components.transform.scale.y
                ),
                def.components.transform.rotation
            )
        )
    end

    if def.components.sprite then
        local sprite = newEntity:add_component(
            Sprite(
                def.components.sprite.asset_name,
                def.components.sprite.width,
                def.components.sprite.height,
                def.components.sprite.start_x,
                def.components.sprite.start_y,
                def.components.sprite.layer
            )
        )
        sprite:generate_uvs()
    end

    return newEntity:id()
end

function LoadBackground()
    for i = 0, 2 do
        for j = 0, 3 do
            local bgTile = Entity("", "bg")
            bgTile:add_component(Transform(vec2(j * 256, i * 256), vec2(1, 1), 0))
            local sprite = bgTile:add_component(Sprite("bg", 256.0, 256.0, 0, 0, 0))
            sprite:generate_uvs()
        end
    end
end

WINDOW_WIDTH = 640
WINDOW_HEIGHT = 480

function CheckPos(position, width, height)
    local min_x = 0
    local min_y = 0
    local max_x = WINDOW_WIDTH
    local max_y = WINDOW_HEIGHT

    if position.x + width < min_x then
        position.x = position.x + WINDOW_WIDTH + width
    elseif position.x > max_x + width then
        position.x = -width
    end
    if position.y + height < min_y then
        position.y = position.y + WINDOW_HEIGHT + height
    elseif position.y > max_y + height then
        position.y = -height
    end
end

function GetRandomVelocity(min_speed, max_speed)
    return vec2(
        math.random(min_speed, max_speed),
        math.random(min_speed, max_speed)
    )
end

function GetRandomPosition()
    return vec2(
        math.random(WINDOW_WIDTH) + WINDOW_WIDTH,
        math.random(WINDOW_HEIGHT) + WINDOW_HEIGHT
    )
end

Asteroids = {}

function AddAsteroid(asteroid)
    table.insert(Asteroids, asteroid)
end

function UpdateAsteroids()
    for k, v in pairs(Asteroids) do
        v:Update()
    end
end

gSpawnTimer = Timer()
function SpawnAsteroid()
    if not gSpawnTimer:is_running() then
        gSpawnTimer:start()
    end

    if gSpawnTimer:elapsed_sec() > 2 then
        local val = math.random(1, 3)
        if val == 1 then
            local asteroid = Asteroid:Create("asteroid_big")
            AddAsteroid(asteroid)
            print("big asteroid spawned")
        elseif val == 2 then
            local asteroid = Asteroid:Create("asteroid_small")
            AddAsteroid(asteroid)
            print("small asteroid spawned")
        elseif val == 3 then
            -- TODO: Create new enemies
        end
        gSpawnTimer:stop()
    end
end
