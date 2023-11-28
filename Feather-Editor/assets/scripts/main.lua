-- Main Lua Script!

run_script("../_Test_Games/Asteroids/scripts/entityDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/assetDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/utilities.lua")
run_script("../_Test_Games/Asteroids/scripts/ship.lua")
run_script("../_Test_Games/Asteroids/scripts/asteroid.lua")
run_script("../_Test_Games/Asteroids/scripts/collision_system.lua")

math.randomseed(os.time())

LoadAssets()
LoadBackground()

local entity = LoadEntity(ShipDefs["red_ship"])
gShip = Ship:Create({id = entity})
gCollisionSystem = CollisionSystem:Create()

main = {
	[1] = {
		update = function()
			gShip:Update()
			UpdateAsteroids()
			gCollisionSystem:Update()
			SpawnAsteroid()
		end
	},
	[2] = {
		render = function()
		end
	}
}
