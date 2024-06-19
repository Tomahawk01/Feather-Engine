#pragma once

#include <sol/sol.hpp>

#include "Core/ECS/Registry.h"

namespace Feather {

	struct SoundBinder
	{
		static void CreateSoundBind(sol::state& lua);
	};

}
