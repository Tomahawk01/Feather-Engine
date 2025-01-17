#pragma once

#include <glm/glm.hpp>
#include <sol/sol.hpp>
#include <entt.hpp>

namespace Feather {

	struct TransformComponent
	{
		glm::vec2 position{ 0.0f };
		glm::vec2 localPosition{ 0.0f };
		glm::vec2 scale{ 1.0f };
		float rotation{ 0.0f };

		[[nodiscard]] std::string to_string();

		static void CreateLuaTransformBind(sol::state& lua);
	};

}
