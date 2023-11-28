#include "CircleColliderComponent.h"

#include "entt.hpp"

void Feather::CircleColliderComponent::CreateLuaCircleColliderBind(sol::state& lua)
{
	lua.new_usertype<CircleColliderComponent>(
		"CircleCollider",
		"type_id", entt::type_hash<CircleColliderComponent>::value,
		sol::call_constructor,
		sol::factories(
			[](float radius) { return CircleColliderComponent{ .radius = radius }; },
			[](float radius, glm::vec2 offset) { return CircleColliderComponent{ .radius = radius, .offset = offset }; }
		),
		"radius", &CircleColliderComponent::radius,
		"offset", &CircleColliderComponent::offset,
		"colliding", &CircleColliderComponent::colliding
	);
}
