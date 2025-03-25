#pragma once

#include <sol/sol.hpp>

namespace Feather {

	class Registry;
	class Camera2D;

	class AnimationSystem
	{
	public:
		AnimationSystem() = default;
		~AnimationSystem() = default;

		void Update(Registry& registry, Camera2D& camera);

		static void CreateAnimationSystemLuaBind(sol::state& lua, Registry& registry);
	};

}
