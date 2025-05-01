#pragma once

#include <sol/sol.hpp>

namespace Feather {

	class Registry;

	struct LuaCoreBinder
	{
		static void CreateLuaBind(sol::state& lua, Registry& registry);
	};

}
