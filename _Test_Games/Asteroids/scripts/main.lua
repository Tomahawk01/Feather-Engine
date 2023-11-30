-- Asteroids game loader
math.randomseed(os.time())

LoadScripts()
LoadAssets()
LoadBackground()

local entity = LoadEntity(ShipDefs["red_ship"])
gShip = Ship:Create({id = entity})
gCollisionSystem = CollisionSystem:Create()
gHud = Hud:Create()

AsteroidsLoaded = true

Music.play("bg_music")
Music.set_volume(30)

function RunAsteroids()
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
