AssetDefs =
{
    textures =
    {
        { name = "ship", path = "../_Test_Games/Asteroids/textures/ship.png" , pixel_art = false },
        { name = "ast_1", path = "../_Test_Games/Asteroids/textures/meteor_big.png" , pixel_art = false },
        { name = "ast_2", path = "../_Test_Games/Asteroids/textures/meteor_small.png" , pixel_art = false },
        { name = "bg", path = "../_Test_Games/Asteroids/textures/background.png" , pixel_art = false },
        { name = "proj_1", path = "../_Test_Games/Asteroids/textures/projectile_1.png" , pixel_art = false },
        { name = "lives", path = "../_Test_Games/Asteroids/textures/lives.png" , pixel_art = false },
        { name = "game_over", path = "../_Test_Games/Asteroids/textures/gameover.png" , pixel_art = true },
        { name = "score", path = "../_Test_Games/Asteroids/textures/score_text.png" , pixel_art = true },
        { name = "numbers", path = "../_Test_Games/Asteroids/textures/score_nums.png" , pixel_art = true },
        { name = "ship_explosion", path = "../_Test_Games/Asteroids/textures/ship_explosion.png" , pixel_art = false }
    },
    music =
    {
        { name = "bg_music", path = "../_Test_Games/Asteroids/sounds/bg_song.mp3" }
    },
    sound_fx =
    {
        { name = "big_ast_break", path = "../_Test_Games/Asteroids/sounds/sfx_asteroid_break_big.wav" },
        { name = "small_ast_break", path = "../_Test_Games/Asteroids/sounds/sfx_asteroid_break_small.wav" },
        { name = "ship_explosion", path = "../_Test_Games/Asteroids/sounds/sfx_explosion.wav" },
        { name = "laser", path = "../_Test_Games/Asteroids/sounds/sfx_laser.ogg" }
    }
}

function LoadAssets()
    for k, v in pairs(AssetDefs.textures) do
        if not AssetManager.add_texture(v.name, v.path, v.pixel_art) then
            print("Failed to load texture file '"..v.name.."' at path '"..v.path.."'")
        else
            print("Loaded texture '"..v.name.."'")
        end
    end

    for k, v in pairs(AssetDefs.music) do
        if not AssetManager.add_music(v.name, v.path) then
            print("Failed to load music file '"..v.name.."' at path '"..v.path.."'")
        else
            print("Loaded music '"..v.name.."'")
        end
    end

    for k, v in pairs(AssetDefs.sound_fx) do
        if not AssetManager.add_sound(v.name, v.path) then
            print("Failed to load sound effect file '"..v.name.."' at path '"..v.path.."'")
        else
            print("Loaded sound effect '"..v.name.."'")
        end
    end
end
