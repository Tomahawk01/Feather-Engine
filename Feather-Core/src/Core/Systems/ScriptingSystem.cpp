#include "ScriptingSystem.h"

#include "Logger/Logger.h"
#include "Utils/Timer.h"
#include "Utils/RandomGenerator.h"

#include "Core/ECS/Entity.h"

#include "Core/Scripting/GlmLuaBindings.h"
#include "Core/Scripting/InputManager.h"
#include "Core/Scripting/SoundBindings.h"
#include "Core/Scripting/RendererBindings.h"
#include "Core/Scripting/UserDataBindings.h"
#include "Core/Scripting/ContactListenerBindings.h"

#include "Core/Resources/AssetManager.h"

#include "Core/CoreUtils/FollowCamera.h"
#include "Core/CoreUtils/CoreUtilities.h"

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

		Entity mainLuaScript{ m_Registry, "main_script", "" };
		mainLuaScript.AddComponent<ScriptComponent>(ScriptComponent{
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

		auto view = m_Registry.GetRegistry().view<ScriptComponent>();
		for (const auto& entity : view)
		{
			Entity ent{ m_Registry, entity };
			if (ent.GetName() != "main_script")
				continue;
			
			auto& script = ent.GetComponent<ScriptComponent>();
			auto error = script.update(entity);
			if (!error.valid())
			{
				sol::error err = error;
				F_ERROR("Error running the Update script: {0}", err.what());
			}
		}

		auto& lua = m_Registry.GetContext<std::shared_ptr<sol::state>>();
		lua->collect_garbage();
	}

	void ScriptingSystem::Render()
	{
		if (!m_MainLoaded)
		{
			F_ERROR("Main lua script has not been loaded!");
			return;
		}

		auto view = m_Registry.GetRegistry().view<ScriptComponent>();
		for (const auto& entity : view)
		{
			Entity ent{ m_Registry, entity };
			if (ent.GetName() != "main_script")
				continue;

			auto& script = ent.GetComponent<ScriptComponent>();
			auto error = script.render(entity);
			if (!error.valid())
			{
				sol::error err = error;
				F_ERROR("Error running the Render script: {0}", err.what());
			}
		}

		auto& lua = m_Registry.GetContext<std::shared_ptr<sol::state>>();
		lua->collect_garbage();
	}

	auto create_timer = [](sol::state& lua){
		lua.new_usertype<Timer>(
			"Timer",
			sol::call_constructor,
			sol::factories([]() { return Timer{}; }),
			"start", &Timer::Start,
			"stop", &Timer::Stop,
			"pause", &Timer::Pause,
			"resume", &Timer::Resume,
			"is_paused", &Timer::IsPaused,
			"is_running", &Timer::IsRunning,
			"elapsed_ms", &Timer::ElapsedMS,
			"elapsed_sec", &Timer::ElapsedSec,
			"restart", [](Timer& timer)
			{
				if (timer.IsRunning())
					timer.Stop();
				timer.Start();
			}
		);
	};

	void ScriptingSystem::RegisterLuaBindings(sol::state& lua, Registry& registry)
	{
		GLMBinding::CreateGLMBindings(lua);
		InputManager::CreateLuaInputBindings(lua, registry);
		AssetManager::CreateLuaAssetManager(lua, registry);
		SoundBinder::CreateSoundBind(lua, registry);
		RendererBinder::CreateRenderingBind(lua, registry);
		UserDataBinder::CreateLuaUserData(lua);
		ContactListenerBinder::CreateLuaContactListener(lua, registry.GetRegistry());

		FollowCamera::CreateLuaFollowCamera(lua, registry);

		create_timer(lua);

		Registry::CreateLuaRegistryBind(lua, registry);
		Entity::CreateLuaEntityBind(lua, registry);
		TransformComponent::CreateLuaTransformBind(lua);
		SpriteComponent::CreateSpriteLuaBind(lua, registry);
		AnimationComponent::CreateAnimationLuaBind(lua);
		BoxColliderComponent::CreateLuaBoxColliderBind(lua);
		CircleColliderComponent::CreateLuaCircleColliderBind(lua);
		PhysicsComponent::CreatePhysicsLuaBind(lua, registry.GetRegistry());
		RigidBodyComponent::CreateRigidBodyLuaBind(lua);
		TextComponent::CreateLuaTextBindings(lua);

		Entity::RegisterMetaComponent<TransformComponent>();
		Entity::RegisterMetaComponent<SpriteComponent>();
		Entity::RegisterMetaComponent<AnimationComponent>();
		Entity::RegisterMetaComponent<BoxColliderComponent>();
		Entity::RegisterMetaComponent<CircleColliderComponent>();
		Entity::RegisterMetaComponent<PhysicsComponent>();
		Entity::RegisterMetaComponent<RigidBodyComponent>();
		Entity::RegisterMetaComponent<TextComponent>();

		Registry::RegisterMetaComponent<TransformComponent>();
		Registry::RegisterMetaComponent<SpriteComponent>();
		Registry::RegisterMetaComponent<AnimationComponent>();
		Registry::RegisterMetaComponent<BoxColliderComponent>();
		Registry::RegisterMetaComponent<CircleColliderComponent>();
		Registry::RegisterMetaComponent<PhysicsComponent>();
		Registry::RegisterMetaComponent<RigidBodyComponent>();
		Registry::RegisterMetaComponent<TextComponent>();

		// Register user data types
		UserDataBinder::register_user_meta_data<ObjectData>();
	}

	void ScriptingSystem::RegisterLuaFunctions(sol::state& lua, Registry& registry)
	{
		lua.set_function(
			"run_script", [&](const std::string& path)
			{
				try
				{
					lua.safe_script_file(path);
				}
				catch (const sol::error& error)
				{
					F_ERROR("Failed to load lua script: {0}", error.what());
					return false;
				}

				return true;
			}
		);

		lua.set_function("get_ticks", [] { return SDL_GetTicks(); });

		auto& assetManager = registry.GetContext<std::shared_ptr<AssetManager>>();
		lua.set_function(
			"measure_text", [&](const std::string& text, const std::string& fontName) {
				const auto& pFont = assetManager->GetFont(fontName);
				if (!pFont)
				{
					F_ERROR("Failed to get font '{}': Does not exist in asset manager!", fontName);
					return -1.f;
				}

				glm::vec2 position{ 0.f }, temp_pos{ position };
				for (const auto& character : text)
					pFont->GetNextCharPos(character, temp_pos);

				return std::abs((position - temp_pos).x);
			}
		);

		auto& engine = CoreEngineData::GetInstance();
		lua.set_function("GetDeltaTime", [&] { return engine.GetDeltaTime(); });
		lua.set_function("WindowWidth", [&] { return engine.WindowWidth(); });
		lua.set_function("WindowHeight", [&] { return engine.WindowHeight(); });

		// Physics Enable functions
		lua.set_function("DisablePhysics", [&] { engine.DisablePhysics(); });
		lua.set_function("EnablePhysics", [&] { engine.EnablePhysics(); });
		lua.set_function("IsPhysicsEnabled", [&] { return engine.IsPhysicsEnabled(); });

		// Render Colliders Enable functions
		lua.set_function("DisableRenderColliders", [&] { engine.DisableColliderRender(); });
		lua.set_function("EnableRenderColliders", [&] { engine.EnableColliderRender(); });
		lua.set_function("IsRenderCollidersEnabled", [&] { return engine.RenderCollidersEnabled(); });

		lua.new_usertype<RandomGenerator>(
			"Random",
			sol::call_constructor,
			sol::constructors<RandomGenerator(uint32_t, uint32_t), RandomGenerator()>(),
			"get_float", &RandomGenerator::GetFloat,
			"get_int", &RandomGenerator::GetInt
		);

		lua.set_function(
			"S2D_EntityInView",
			[&](const TransformComponent& transform, float width, float height) {
				auto& camera = registry.GetContext<std::shared_ptr<Camera2D>>();
				return EntityInView(transform, width, height, *camera);
			}
		);
	}

}
