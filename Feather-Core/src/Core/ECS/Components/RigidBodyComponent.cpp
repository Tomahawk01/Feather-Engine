#include "RigidBodyComponent.h"

#include <entt.hpp>

namespace Feather {
	
	std::string RigidBodyComponent::to_string() const
	{
		std::stringstream ss;
		ss << "==== RigidBody Component ==== \n" <<
			  "Max velocity: x = " << maxVelocity.x << ", y = " << maxVelocity.y << "\n";

		return ss.str();
	}

	void RigidBodyComponent::CreateRigidBodyLuaBind(sol::state& lua)
	{
		lua.new_usertype<RigidBodyComponent>(
			"RigidBody",
			"type_id", entt::type_hash<RigidBodyComponent>::value,
			sol::call_constructor,
			sol::factories(
				[](const glm::vec2& velocity) { return RigidBodyComponent{ .maxVelocity = velocity }; }
			),
			"currentVelocity", &RigidBodyComponent::currentVelocity,
			"maxVelocity", &RigidBodyComponent::maxVelocity,
			"to_string", &RigidBodyComponent::to_string
		);
	}

}
