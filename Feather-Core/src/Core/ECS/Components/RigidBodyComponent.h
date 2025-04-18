#pragma once

#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace Feather {

	struct RigidBodyComponent
	{
		glm::vec2 currentVelocity{ 0.0f };
		glm::vec2 maxVelocity{ 0.0f };

		[[nodiscard]] std::string to_string() const;

		static void CreateRigidBodyLuaBind(sol::state& lua);
	};

}
