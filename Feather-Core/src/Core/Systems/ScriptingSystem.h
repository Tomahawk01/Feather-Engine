#pragma once

#include <sol/sol.hpp>

#include "Core/ECS/Registry.h"

namespace Feather {

	class ScriptingSystem
	{
	public:
		ScriptingSystem(Feather::Registry& registry);
		~ScriptingSystem() = default;

		bool LoadMainScript(sol::state& lua);
		void Update();
		void Render();

		static void RegisterLuaBindings(sol::state& lua, Feather::Registry& registry);

	private:
		Feather::Registry& m_Registry;
		bool m_MainLoaded;
	};

}
