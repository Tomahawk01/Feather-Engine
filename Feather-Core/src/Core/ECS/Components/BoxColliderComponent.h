#pragma once

#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace Feather {

	struct BoxColliderComponent
	{
		int width{ 4 };
		int height{ 4 };
		glm::vec2 offset{ 0.0f };
		bool colliding{ false };

		[[nodiscard]] std::string to_string() const;

		static void CreateLuaBoxColliderBind(sol::state& lua);
	};

}
