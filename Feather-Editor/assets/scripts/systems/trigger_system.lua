TriggerSystem = Subscriber:Create()

function TriggerSystem:Create()
	return CreateObject(TriggerSystem):new()
end 

function TriggerSystem:OnCollision(object_a, object_b)
	if object_a.isTrigger and object_b.tag == "player" then 
		self:OnPlayerTriggered(object_a, object_b)
	end 

	if object_b.isTrigger and object_a.tag == "player" then 
		self:OnPlayerTriggered(object_b, object_a)
	end 
end

function TriggerSystem:OnPlayerTriggered(trigger, player)
	print(trigger.group .. "has been activated by " ..player.tag)
	local playerEntity = Entity(player.entityID)
	local physics = playerEntity:get_component(PhysicsComponent)
	physics:set_transform(vec2(10 * 16, 33 * 16))
	physics:linear_impulse(vec2(0, 5))
end
