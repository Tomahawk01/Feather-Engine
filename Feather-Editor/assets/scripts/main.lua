-- Main Lua Script!

run_script("../_Test_Games/Asteroids/scripts/entityDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/assetDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/utilities.lua")
run_script("../_Test_Games/Asteroids/scripts/ship.lua")
run_script("../_Test_Games/Asteroids/scripts/asteroid.lua")

math.randomseed(os.time())

LoadAssets()
LoadBackground()

local entity = LoadEntity(ShipDefs["red_ship"])
gShip = Ship:Create({id = entity})

main = {
	[1] = {
		update = function()
			gShip:Update()
			UpdateAsteroids()

			SpawnAsteroid()
		end
	},
	[2] = {
		render = function()
		end
	}
}
