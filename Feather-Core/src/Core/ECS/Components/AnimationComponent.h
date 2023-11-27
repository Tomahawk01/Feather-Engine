#pragma once

#include <sol/sol.hpp>

namespace Feather {

	struct AnimationComponent
	{
		int numFrames{ 1 }, frameRate{ 1 }, frameOffset{ 0 }, currentFrame{ 0 };
		bool isVertical{ false };

		static void CreateAnimationLuaBind(sol::state& lua);
	};

}
