AssetDefs =
{
    textures =
    {
        { name = "ship", path = "../_Test_Games/Asteroids/textures/ship.png" , pixel_art = false },
        { name = "ast_1", path = "../_Test_Games/Asteroids/textures/meteor_big.png" , pixel_art = false },
        { name = "ast_2", path = "../_Test_Games/Asteroids/textures/meteor_small.png" , pixel_art = false }
    }
}

function LoadAssets()
    for k, v in pairs(AssetDefs.textures) do
        if not AssetManager.add_texture(v.name, v.path, v.pixel_art) then
            print("Failed to load texture '"..v.name.."' at path '"..v.path.."'")
        else
            print("Loaded texture '"..v.name.."'")
        end
    end

    -- TODO: Load other asset types
end
