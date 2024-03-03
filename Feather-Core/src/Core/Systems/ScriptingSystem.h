#pragma once

#include <sol/sol.hpp>

#include "Core/ECS/Registry.h"

namespace Feather {

	class ScriptingSystem
	{
	public:
		ScriptingSystem(Registry& registry);
		~ScriptingSystem() = default;

		bool LoadMainScript(sol::state& lua);
		void Update();
		void Render();

		static void RegisterLuaBindings(sol::state& lua, Registry& registry);

		static void RegisterLuaFunctions(sol::state& lua, Registry& registry);

	private:
		Registry& m_Registry;
		bool m_MainLoaded;
	};

}
