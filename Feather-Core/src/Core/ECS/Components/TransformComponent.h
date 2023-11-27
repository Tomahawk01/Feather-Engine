#pragma once

#include <glm/glm.hpp>
#include <sol/sol.hpp>
#include <entt.hpp>

namespace Feather {

	struct TransformComponent
	{
		glm::vec2 position{ glm::vec2{0.0f} }, scale{ glm::vec2 {1.0f} };
		float rotation{ 0.0f };

		static void CreateLuaTransformBind(sol::state& lua);
	};

}
