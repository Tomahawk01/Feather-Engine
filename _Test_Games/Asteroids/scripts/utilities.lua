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
