#pragma once

#include <sol/sol.hpp>

namespace Feather {

	struct LuaFilesystem
	{
		static void CreateLuaFileSystemBind(sol::state& lua);
	};

}
