script_list = 
{
    "../_Test_Games/Asteroids/scripts/utilities.lua",
    "../_Test_Games/Asteroids/scripts/entityDefs.lua",
    "../_Test_Games/Asteroids/scripts/assetDefs.lua",
    "../_Test_Games/Asteroids/scripts/ship.lua",
    "../_Test_Games/Asteroids/scripts/asteroid.lua",
    "../_Test_Games/Asteroids/scripts/collision_system.lua",
    "../_Test_Games/Asteroids/scripts/projectile.lua",
    "../_Test_Games/Asteroids/scripts/game_data.lua",
    "../_Test_Games/Asteroids/scripts/hud.lua"
}

function LoadScripts()
    for k, v in ipairs(script_list) do
		if not run_script(v) then
			print("Failed to load '"..v.."'")
		end
	end
end
