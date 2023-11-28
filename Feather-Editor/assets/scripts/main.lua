-- Main Lua Script!

run_script("../_Test_Games/Asteroids/scripts/entityDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/assetDefs.lua")
run_script("../_Test_Games/Asteroids/scripts/utilities.lua")

LoadAssets()

local entity = LoadEntity(ShipDefs["red_ship"])

main = {
	[1] = {
		update = function()
			
		end
	},
	[2] = {
		render = function()
			
		end
	},
}
