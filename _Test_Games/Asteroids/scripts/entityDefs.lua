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
            },
            animation =
            {
                num_frames = 0,
                frame_rate = 0,
                frame_offset = 0,
                is_vertical = false,
                is_looped = false
            }
        }
    }
}

ProjectileDefs =
{
    proj_1 =
    {
        group = "projectiles",
        components =
        {
            transform = 
            {
                position = {x = 0, y = 0},
                rotation = 0,
                scale = {x = 1, y = 1}
            },
            sprite =
            {
                asset_name = "proj_1",
                width = 37, height = 13,
                start_x = 0, start_y = 0,
                layer = 2
            },
            circle_collider =
            {
                radius = 10
            }
        },
        life_time = 2000,
        proj_speed = 10
    }
}

HudDefs =
{
    lives =
    {
        group = "lives",
        components =
        {
            transform = 
            {
                position = {x = 0, y = 0},
                rotation = 0,
                scale = {x = 1, y = 1}
            },
            sprite =
            {
                asset_name = "lives",
                width = 37, height = 26,
                start_x = 0, start_y = 0,
                layer = 10
            }
        }
    },
    score =
    {
        tag = "score",
        components =
        {
            transform = 
            {
                position = {x = 320, y = 0},
                rotation = 0,
                scale = {x = 1, y = 1}
            },
            sprite =
            {
                asset_name = "score",
                width = 100, height = 40,
                start_x = 0, start_y = 0,
                layer = 10
            }
        }
    },
    number =
    {
        group = "numbers",
        components =
        {
            transform = 
            {
                position = {x = 440, y = 2},
                rotation = 0,
                scale = {x = 1.8, y = 1.8}
            },
            sprite =
            {
                asset_name = "numbers",
                width = 19, height = 19,
                start_x = 0, start_y = 0,
                layer = 10
            }
        }
    },
    game_over =
    {
        tag = "game_over",
        components =
        {
            transform = 
            {
                position = {x = (WINDOW_WIDTH / 2) - 115, y = (WINDOW_HEIGHT / 2) - 85},
                rotation = 0,
                scale = {x = 1, y = 1}
            },
            sprite =
            {
                asset_name = "game_over",
                width = 230, height = 170,
                start_x = 0, start_y = 0,
                layer = 10,
                hidden = true
            }
        }
    }
}
