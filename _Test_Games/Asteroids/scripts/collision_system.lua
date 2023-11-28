CollisionSystem = {}
CollisionSystem.__index = CollisionSystem

function CollisionSystem:Create()
    local this =
    {
    }

    setmetatable(this, self)
    return this
end

function CollisionSystem:Update()
    self:UpdateCircleCollision()
end

function CollisionSystem:UpdateCircleCollision()
    local entities = Registry.get_entities(CircleCollider)

    entities:for_each(
        function(entity_a)
            local group_a = entity_a:group()
            local name_a = entity_a:name()
            local collider_a = entity_a:get_component(CircleCollider)

            entities:for_each(
                function(entity_b)
                    if entity_a:id() == entity_b:id() then
                        goto continue
                    end

                    local group_b = entity_b:group()
                    if group_a == group_b then
                        goto continue
                    end

                    local name_b = entity_b:name()
                    local collider_b = entity_b:get_component(CircleCollider)

                    if collider_a.colliding or collider_b.colliding then
                        goto continue
                    end

                    if self:Intersect(entity_a, entity_b) then
                        print("ID: " ..entity_a:id()..", is colliding with ID: " ..entity_b:id())
                    end

                    ::continue::
                end
            )
        end
    )
end

function CollisionSystem:GetCenter(entity)
    local transform = entity:get_component(Transform)
    local sprite = entity:get_component(Sprite)

    local centerSprite = vec2(sprite.width / 2, sprite.height / 2)
    local center = transform.position + centerSprite

    return center
end

function CollisionSystem:Intersect(entity_a, entity_b)
    local a_center = self:GetCenter(entity_a)
    local b_center = self:GetCenter(entity_b)

    local difference = a_center - b_center
    local distanceSq = difference:lengthSq()

    local circle_a = entity_a:get_component(CircleCollider)
    local circle_b = entity_b:get_component(CircleCollider)

    local radSum = circle_a.radius + circle_b.radius
    local radSq = radSum * radSum

    return distanceSq <= radSq
end
