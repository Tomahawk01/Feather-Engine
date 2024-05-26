-- Helper functions

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
        sprite.hidden = def.components.sprite.hidden or false
    end

    if def.components.circle_collider then
        newEntity:add_component(
            CircleCollider(
                def.components.circle_collider.radius
            )
        )
    end

    if def.components.animation then
        newEntity:add_component(
            Animation(
                def.components.animation.num_frames,
                def.components.animation.frame_rate,
                def.components.animation.frame_offset,
                def.components.animation.is_vertical,
                def.components.animation.is_looped
            )
        )
    end

    return newEntity:id()
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

function GetDigit(num, digit)
    local n = 10 ^ digit
    local n1 = 10 ^ (digit - 1)
    return math.floor((num % n) / n1)
end

Tileset = {}
Tileset.__index = Tileset
function Tileset:Create(params)
    local this =
    {
        name = params.name,
        columns = params.columns,
        width = params.width,
        height = params.height,
        tilewidth = params.tilewidth,
        tileheight = params.tileheight,
        firstgid = params.firstgid
    }

    this.rows = params.height / params.tileheight
    this.lastgid = math.floor(((this.rows * this.columns) + this.firstgid) - 1)

    setmetatable(this, self)
    return this
end

function Tileset:TileIdExists(id)
    return id >= self.firstgid and id <= self.lastgid
end

function Tileset:GetTileStartXY(id)
    assert(self:TileIdExists(id), "Tile ID '"..id.."' does not exist in tileset '"..self.name.."'")
    local actualTileID = id - self.firstgid
    local start_y = math.floor(actualTileID / self.columns)
    local start_x = math.floor(actualTileID % self.columns)
    return start_x, start_y
end

function GetTileset(tilesets, id)
    for k, v in pairs(tilesets) do
        if v:TileIdExists(id) then
            return v
        end
    end
    return nil
end

function LoadMap(mapDef)
    assert(mapDef)
    local tilesets = {}
    for k, v in pairs(mapDef.tilesets) do
        local newTileset = Tileset:Create(
            {
                name = v.name,
                columns = v.columns,
                width = v.imagewidth,
                height = v.imageheight,
                tilewidth = v.tilewidth,
                tileheight = v.tileheight,
                firstgid = v.firstgid
            }
        )

        table.insert(tilesets, newTileset)
    end

    for k, v in pairs(mapDef.layers) do
        local rows = v.height - 1
        local cols = v.width
        local layer = k - 1

        for row = 0, rows do
            for col = 1, cols do
                local id = v.data[row * cols + col]

                if id == 0 then
                    goto continue
                end

                local tileset = GetTileset(tilesets, id);
                assert(tileset, "Tileset does not exist with ID '"..id.."'")
                local start_x, start_y = tileset:GetTileStartXY(id)

                local scale = 1
                local position = vec2(((col - 1) * tileset.tilewidth * scale), (row * tileset.tileheight * scale))

                local tile = Entity("", "tiles")
                local transform = tile:add_component(Transform(position, vec2(scale, scale), 0))

                local isCollider = false

                if tileset.name == "collider" or tileset.name == "trigger" then
                    local width = tileset.tilewidth / scale
                    local height = tileset.tileheight / scale
                    tile:add_component(
                        BoxCollider(
                            math.floor(width),
                            math.floor(height),
                            vec2(0, 0)
                        )
                    )

                    if IsPhysicsEnabled() then
                        local physicsAttribs = PhysicsAttributes()

                        physicsAttribs.type = BodyType.Static
                        physicsAttribs.density = 1000.0
                        physicsAttribs.friction = 0.0
                        physicsAttribs.restitution = 0.0
                        physicsAttribs.gravityScale = 0.0
                        physicsAttribs.position = transform.position
                        physicsAttribs.isFixedRotation = true
                        physicsAttribs.boxSize = vec2(tileset.tilewidth, tileset.tileheight)
                        physicsAttribs.isBoxShape = true

                        if tileset.name == "trigger" then
                            physicsAttribs.isTrigger = true
                            physicsAttribs.objectData = ObjectData("", "hole_triggers", false, true, tile:id())
                        else
                            physicsAttribs.objectData = ObjectData("", "colliders", true, false, tile:id())
                        end

                        tile:add_component(PhysicsComponent(physicsAttribs))
                    end

                    isCollider = true
                end

                if not isCollider then
                    local sprite = tile:add_component(
                        Sprite(
                            tileset.name,
                            tileset.tilewidth,
                            tileset.tileheight,
                            start_x, start_y,
                            layer
                        )
                    )
                    sprite:generate_uvs()
                end

                ::continue::
            end
        end
    end
end

function LoadAssets(assets)
    for k, v in pairs(assets.textures) do
        if not AssetManager.add_texture(v.name, v.path, v.pixel_art) then
            print("Failed to load texture file '"..v.name.."' at path '"..v.path.."'")
        else
            print("Loaded texture '"..v.name.."'")
        end
    end

    for k, v in pairs(assets.music) do
        if not AssetManager.add_music(v.name, v.path) then
            print("Failed to load music file '"..v.name.."' at path '"..v.path.."'")
        else
            print("Loaded music '"..v.name.."'")
        end
    end

    for k, v in pairs(assets.sound_fx) do
        if not AssetManager.add_sound(v.name, v.path) then
            print("Failed to load sound effect file '"..v.name.."' at path '"..v.path.."'")
        else
            print("Loaded sound effect '"..v.name.."'")
        end
    end
end

function Debug()
    if Keyboard.just_pressed(KEY_F1) then
        if IsRenderCollidersEnabled() then
            DisableRenderColliders()
        else
            EnableRenderColliders()
        end
    end
end
