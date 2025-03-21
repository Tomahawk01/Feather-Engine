#pragma once

#include <sol/sol.hpp>

namespace Feather {

	struct LuaFilesystem
	{
		static void CreateLuaFileSytemBind(sol::state& lua);
	};

}
