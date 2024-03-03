#include "RigidBodyComponent.h"

#include <entt.hpp>

namespace Feather {
	
	void RigidBodyComponent::CreateRigidBodyLuaBind(sol::state& lua)
	{
		lua.new_usertype<RigidBodyComponent>(
			"RigidBody",
			"type_id", entt::type_hash<RigidBodyComponent>::value,
			sol::call_constructor,
			sol::factories(
				[](const glm::vec2& velocity) { return RigidBodyComponent{ .velocity = velocity }; }
			),
			"velocity", &RigidBodyComponent::velocity
		);
	}

}
