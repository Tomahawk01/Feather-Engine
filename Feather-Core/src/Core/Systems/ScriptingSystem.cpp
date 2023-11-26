#include "ScriptingSystem.h"

#include <Logger/Logger.h>
#include "Core/ECS/Components/ScriptComponent.h"
#include "Core/ECS/Entity.h"

namespace Feather {

	ScriptingSystem::ScriptingSystem(Feather::Registry& registry)
		: m_Registry(registry), m_MainLoaded{ false }
	{}

	bool ScriptingSystem::LoadMainScript(sol::state& lua)
	{
		try
		{
			auto result = lua.safe_script_file("assets/scripts/main.lua");
		}
		catch (const sol::error& e)
		{
			F_ERROR("Error loading the Main lua script: {0}", e.what());
			return false;
		}

		sol::table main_lua = lua["main"];

		sol::optional<sol::table> UpdateExists = main_lua[1];
		if (UpdateExists == sol::nullopt)
		{
			F_ERROR("There is no update function in main.lua!");
			return false;
		}
		sol::table update_script = main_lua[1];
		sol::function update = update_script["update"];

		sol::optional<sol::table> RenderExists = main_lua[2];
		if (RenderExists == sol::nullopt)
		{
			F_ERROR("There is no render function in main.lua!");
			return false;
		}
		sol::table render_script = main_lua[2];
		sol::function render = render_script["render"];

		Feather::Entity mainLuaScript{ m_Registry, "main_script", "" };
		mainLuaScript.AddComponent<Feather::ScriptComponent>(Feather::ScriptComponent{
					.update = update,
					.render = render });

		m_MainLoaded = true;

		return true;
	}

	void ScriptingSystem::Update()
	{
		if (!m_MainLoaded)
		{
			F_ERROR("Main lua script has not been loaded!");
			return;
		}

		auto view = m_Registry.GetRegistry().view<Feather::ScriptComponent>();
		for (const auto& entity : view)
		{
			Feather::Entity ent{ m_Registry, entity };
			if (ent.GetName() != "main_script")
				continue;
			
			auto& script = ent.GetComponent<Feather::ScriptComponent>();
			auto error = script.update(entity);
			if (!error.valid())
			{
				sol::error err = error;
				F_ERROR("Error running the Update script: {0}", err.what());
			}
		}
	}

	void ScriptingSystem::Render()
	{
		if (!m_MainLoaded)
		{
			F_ERROR("Main lua script has not been loaded!");
			return;
		}

		auto view = m_Registry.GetRegistry().view<Feather::ScriptComponent>();
		for (const auto& entity : view)
		{
			Feather::Entity ent{ m_Registry, entity };
			if (ent.GetName() != "main_script")
				continue;

			auto& script = ent.GetComponent<Feather::ScriptComponent>();
			auto error = script.render(entity);
			if (!error.valid())
			{
				sol::error err = error;
				F_ERROR("Error running the Render script: {0}", err.what());
			}
		}
	}

}
