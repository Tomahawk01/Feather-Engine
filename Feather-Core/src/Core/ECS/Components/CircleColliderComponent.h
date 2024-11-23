#pragma once

#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace Feather {

	struct CircleColliderComponent
	{
		float radius{ 4.0f };
		glm::vec2 offset{ glm::vec2{0} };
		bool colliding{ false };

		[[nodiscard]] std::string to_string() const;

		static void CreateLuaCircleColliderBind(sol::state& lua);
	};

}
