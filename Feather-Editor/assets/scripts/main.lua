-- Main Lua Script!

run_script("../_Test_Games/Asteroids/scripts/script_list.lua")
run_script("../_Test_Games/Asteroids/scripts/main.lua")

Music.play("bg_music")

main = {
	[1] = {
		update = function()
			if AsteroidsLoaded then
				RunAsteroids()
			end
		end
	},
	[2] = {
		render = function()
		end
	}
}
