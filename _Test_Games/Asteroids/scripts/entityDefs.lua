AsteroidDefs =
{
    asteroid_big = 
    {
        group = "asteroids",
        components =
        {
            transform = 
            {
                position = {x = -100, y = -100},
                rotation = 0,
                scale = {x = 1, y = 1}
            },
            sprite =
            {
                asset_name = "ast_1",
                width = 120, height = 98,
                start_x = 0, start_y = 0,
                layer = 2
            },
            circle_collider =
            {
                radius = 48
            }
        },
        type = "big",
        min_speed = -2,
        max_speed = 4
    },
    asteroid_small = 
    {
        group = "asteroids",
        components =
        {
            transform = 
            {
                position = {x = -100, y = -100},
                rotation = 0,
                scale = {x = 1, y = 1}
            },
            sprite =
            {
                asset_name = "ast_2",
                width = 28, height = 28,
                start_x = 0, start_y = 0,
                layer = 2
            },
            circle_collider =
            {
                radius = 14
            }
        },
        type = "small",
        min_speed = -5,
        max_speed = 5
    }
}

ShipDefs =
{
    red_ship =
    {
        tag = "ship",
        components =
        {
            transform = 
            {
                position = {x = 320, y = 240},
                rotation = -90,
                scale = {x = 1, y = 1}
            },
            sprite =
            {
                asset_name = "ship",
                width = 84, height = 136,
                start_x = 0, start_y = 0,
                layer = 2
            },
            circle_collider =
            {
                radius = 42
            }
        }
    }
}
