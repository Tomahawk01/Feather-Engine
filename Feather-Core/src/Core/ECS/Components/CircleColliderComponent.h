#pragma once

#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace Feather {

	struct CircleColliderComponent
	{
		float radius{ 0.0f };
		glm::vec2 offset{ glm::vec2{0} };
		bool colliding{ false };

		static void CreateLuaCircleColliderBind(sol::state& lua);
	};

}
