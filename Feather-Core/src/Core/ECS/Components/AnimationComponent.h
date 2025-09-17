#pragma once

#include <sol/sol.hpp>
#include <SDL.h>

namespace Feather {

	struct AnimationComponent
	{
		int numFrames{ 1 };
		int frameRate{ 1 };
		int currentFrame{ 0 };
		int startTime{ static_cast<int>(SDL_GetTicks()) };

		bool isVertical{ false };
		bool isLooped{ false };

		[[nodiscard]] std::string to_string() const;

		static void CreateAnimationLuaBind(sol::state& lua);
	};

}
