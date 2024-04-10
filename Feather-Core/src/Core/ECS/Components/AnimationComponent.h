#pragma once

#include <sol/sol.hpp>
#include <SDL.h>

namespace Feather {

	struct AnimationComponent
	{
		int numFrames{ 1 }, frameRate{ 1 }, frameOffset{ 0 }, currentFrame{ 0 }, startTime{ static_cast<int>(SDL_GetTicks()) };
		bool isVertical{ false }, isLooped{ false };

		[[nodiscard]] std::string to_string() const;

		static void CreateAnimationLuaBind(sol::state& lua);
	};

}
