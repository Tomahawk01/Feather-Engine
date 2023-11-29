-- Main Lua Script!

run_script("../_Test_Games/Asteroids/scripts/utilities.lua")
run_script("../_Test_Games/Asteroids/scripts/entityDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/assetDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/ship.lua")
run_script("../_Test_Games/Asteroids/scripts/asteroid.lua")
run_script("../_Test_Games/Asteroids/scripts/collision_system.lua")
run_script("../_Test_Games/Asteroids/scripts/projectile.lua")
run_script("../_Test_Games/Asteroids/scripts/game_data.lua")
run_script("../_Test_Games/Asteroids/scripts/hud.lua")

math.randomseed(os.time())

LoadAssets()
LoadBackground()

local entity = LoadEntity(ShipDefs["red_ship"])
gShip = Ship:Create({id = entity})
gCollisionSystem = CollisionSystem:Create()
gHud = Hud:Create()

main = {
	[1] = {
		update = function()
			gShip:Update()
			UpdateAsteroids()
			UpdateProjectiles()
			gCollisionSystem:Update()
			gHud:Update()
			if not gData:IsGameOver() then
				SpawnAsteroid()
			else
				if Keyboard.just_pressed(KEY_ENTER) then
					gData:Reset()
					gHud:Reset()
					gShip:Reset()
					ResetAsteroids()
					ResetProjectiles()
				end
			end
		end
	},
	[2] = {
		render = function()
		end
	}
}
