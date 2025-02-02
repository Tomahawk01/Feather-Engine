ProjectileDefs = 
{
    regular_shot = {
        tag = "regular_shot",
        group = "projectile",
        components = {
            transform = {
                position = vec2(0, 0),
                scale = vec2(1, 1),
                rotation = 0.0
            },
            sprite = {
                texture = "projectile_atlas",
                width = 32,
                height = 32,
                start_x = 0,
                start_y = 2,
                layer = 3
            },
            animation = {
                num_frames = 4,
                frame_rate = 20,
                frame_offset = 0,
                is_vertical = false,
                is_looped = true
            },
            circle_collider = {
                radius = 8,
                offset = vec2(8, 12)
            },
            physics_attributes = {
                type = BodyType.Dynamic,
                density = 1.0,
                friction = 1.0,
                restitution = 0.0,
                position = vec2(0, 0),
                radius = 8,
                isCircle = true,
                isTrigger = true,
                isFixedRotation = true,
                object_data = {
                    tag = "shot",
                    group = "projectile",
                    isCollider = false,
                    isTrigger = true,
                    isFriendly = true
                }
            }
        }
    }
}