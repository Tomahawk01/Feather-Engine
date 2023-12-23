#pragma once

#include <sol/sol.hpp>

#include "Core/ECS/Registry.h"

namespace Feather {

	struct RendererBinder
	{
		static void CreateRenderingBind(sol::state& lua, Registry& registry);
	};

}
