#pragma once

#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace Feather {

	struct RigidBodyComponent
	{
		glm::vec2 velocity{ 0.0f };

		static void CreateRigidBodyLuaBind(sol::state& lua);
	};

}
