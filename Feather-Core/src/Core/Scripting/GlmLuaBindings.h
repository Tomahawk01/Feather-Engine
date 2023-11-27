#pragma once

#include "sol/sol.hpp"

namespace Feather {

	struct GLMBinding
	{
		static void CreateGLMBindings(sol::state& lua);
	};

}
