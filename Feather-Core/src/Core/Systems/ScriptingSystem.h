#pragma once

#include <sol/sol.hpp>

namespace Feather {

	class Registry;

	class ScriptingSystem
	{
	public:
		ScriptingSystem();
		~ScriptingSystem() = default;

		bool LoadMainScript(Registry& registry, sol::state& lua);
		void Update(Registry& registry);
		void Render(Registry& registry);

		static void RegisterLuaBindings(sol::state& lua, Registry& registry);

		static void RegisterLuaFunctions(sol::state& lua, Registry& registry);

	private:
		bool m_MainLoaded;
	};

}
