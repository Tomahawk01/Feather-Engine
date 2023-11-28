#pragma once

#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace Feather {

	struct BoxColliderComponent
	{
		int width{ 0 }, height{ 0 };
		glm::vec2 offset{ glm::vec2{0} };
		bool colliding{ false };

		static void CreateLuaBoxColliderBind(sol::state& lua);
	};

}
