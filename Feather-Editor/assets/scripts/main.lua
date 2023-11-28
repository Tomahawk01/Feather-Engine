-- Main Lua Script!

run_script("../_Test_Games/Asteroids/scripts/entityDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/assetDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/utilities.lua")
run_script("../_Test_Games/Asteroids/scripts/ship.lua")

LoadAssets()

local entity = LoadEntity(ShipDefs["red_ship"])
gShip = Ship:Create({id = entity})

main = {
	[1] = {
		update = function()
			gShip:UpdateShip()
		end
	},
	[2] = {
		render = function()
			
		end
	}
}
