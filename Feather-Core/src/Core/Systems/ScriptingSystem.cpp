#include "ScriptingSystem.h"

#include "Logger/Logger.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/ECSUtils.h"

#include "Core/Scripting/GlmLuaBindings.h"
#include "Core/Scripting/InputManager.h"
#include "Core/Scripting/SoundBindings.h"
#include "Core/Scripting/RendererBindings.h"
#include "Core/Scripting/UserDataBindings.h"
#include "Core/Scripting/ContactListenerBindings.h"
#include "Core/Scripting/LuaFilesystemBindings.h"
#include "Core/Scripting/ScriptingUtilities.h"

#include "Core/Resources/AssetManager.h"

#include "Core/CoreUtils/FollowCamera.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/CoreUtils/ProjectInfo.h"

#include "Core/States/State.h"
#include "Core/States/StateStack.h"
#include "Core/States/StateMachine.h"

#include "Core/Events/EngineEventTypes.h"
#include "Core/Events/EventDispatcher.h"

#include "Core/Systems/RenderSystem.h"
#include "Core/Systems/RenderUISystem.h"
#include "Core/Systems/AnimationSystem.h"

#include "Core/Scene/Scene.h"
#include "Core/Character/Character.h"

#include "Renderer/Essentials/Texture.h"
#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Font.h"

#include "Utils/Timer.h"
#include "Utils/RandomGenerator.h"
#include "Utils/HelperUtilities.h"
#include "Utils/Tween.h"

#include <filesystem>

namespace Feather {

	ScriptingSystem::ScriptingSystem()
		: m_MainLoaded{ false }
	{}

	bool ScriptingSystem::LoadMainScript(const std::string& mainLuaFile, Registry& registry, sol::state& lua)
	{
		std::error_code ec;
		if (!std::filesystem::exists(mainLuaFile, ec))
		{
			F_ERROR("Error loading the main lua script: {}", ec.message());
			return false;
		}

		try
		{
			auto result = lua.safe_script_file(mainLuaFile);
		}
		catch (const sol::error& e)
		{
			F_ERROR("Error loading the Main lua script: {}", e.what());
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

		Entity mainLuaScript{ registry, "main_script", "" };
		mainLuaScript.AddComponent<ScriptComponent>(ScriptComponent{
					.update = update,
					.render = render });

		m_MainLoaded = true;

		return true;
	}

	bool ScriptingSystem::LoadMainScript(ProjectInfo& projectInfo, Registry& registry, sol::state& lua)
	{
		auto optScriptListPath = projectInfo.GetScriptListPath();
		F_ASSERT(optScriptListPath && "Script List path not setup correctly in project info");
		auto optContentPath = projectInfo.TryGetFolderPath(EProjectFolderType::Content);
		F_ASSERT(optContentPath && "Content path not setup correctly in project info");

		// Try to load script list files
		if (std::filesystem::exists(*optScriptListPath) && std::filesystem::exists(*optContentPath))
		{
			try
			{
				sol::state scriptLua;
				auto result = scriptLua.safe_script_file(optScriptListPath->string());
				if (!result.valid())
				{
					sol::error err = result;
					throw err;
				}

				sol::optional<sol::table> scriptList = scriptLua["ScriptList"];
				if (!scriptList)
				{
					F_ERROR("Failed to load script list. Missing \"ScriptList\" table");
					return false;
				}

				for (const auto& [_, script] : *scriptList)
				{
					try
					{
						std::filesystem::path scriptPath = *optContentPath / script.as<std::string>();
						auto result = lua.safe_script_file(scriptPath.string());
						if (!result.valid())
						{
							sol::error err = result;
							throw err;
						}
					}
					catch (const sol::error& error)
					{
						F_ERROR("Failed to load script {}. {}", script.as<std::string>(), error.what());
						return false;
					}
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to load script_list.lua: {}", error.what());
				return false;
			}
		}

		auto optMainLuaScript = projectInfo.GetMainLuaScriptPath();
		F_ASSERT(optMainLuaScript && "Main lua script has not been set correctly in project info");

		return LoadMainScript(optMainLuaScript->string(), registry, lua);
	}

	void ScriptingSystem::Update(Registry& registry)
	{
		if (!m_MainLoaded)
		{
			F_FATAL("Main lua script has not been loaded!");
			return;
		}

		auto mainScript = FindEntityByTag(registry, "main_script");
		if (mainScript == entt::null)
		{
			F_FATAL("Failed to run main Update script. Entity does not exist");
			return;
		}
			
		Entity scriptEnt{ registry, mainScript };

		if (auto* script = scriptEnt.TryGetComponent<ScriptComponent>())
		{
			auto error = script->update();
			if (!error.valid())
			{
				sol::error err = error;
				F_ERROR("Error running the Update script: {0}", err.what());
			}
		}

		if (auto* lua = registry.TryGetContext<std::shared_ptr<sol::state>>())
			(*lua)->collect_garbage();
	}

	void ScriptingSystem::Render(Registry& registry)
	{
		if (!m_MainLoaded)
		{
			F_FATAL("Main lua script has not been loaded!");
			return;
		}

		auto mainScript = FindEntityByTag(registry, "main_script");
		if (mainScript == entt::null)
		{
			F_FATAL("Failed to run main render script. Entity does not exist");
			return;
		}

		Entity scriptEnt{ registry, mainScript };

		if (auto* script = scriptEnt.TryGetComponent<ScriptComponent>())
		{
			auto error = script->render();
			if (!error.valid())
			{
				sol::error err = error;
				F_ERROR("Error running the Render script: {0}", err.what());
			}
		}

		if (auto* lua = registry.TryGetContext<std::shared_ptr<sol::state>>())
			(*lua)->collect_garbage();
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

	auto createTweenLuaBind = [](sol::state& lua)
		{
			// We only need to expose the easing function type to the user not how it was implemented
			lua.new_enum<EasingFunc>("EasingFuncType",
				{ { "Linear", EasingFunc::LINEAR },
				  { "EaseInQuad", EasingFunc::EASE_IN_QUAD },
				  { "EaseOutQuad", EasingFunc::EASE_OUT_QUAD },
				  { "EaseInSine", EasingFunc::EASE_IN_SINE },
				  { "EaseOutSine", EasingFunc::EASE_OUT_SINE },
				  { "EaseInOutSine", EasingFunc::EASE_IN_OUT_SINE },
				  { "EaseOutElastic", EasingFunc::EASE_OUT_ELASTIC },
				  { "EaseInElastic", EasingFunc::EASE_IN_ELASTIC },
				  { "EaseInOutElastic", EasingFunc::EASE_IN_OUT_ELASTIC },
				  { "EaseInExponential", EasingFunc::EASE_IN_EXPONENTIAL },
				  { "EaseOutExponential", EasingFunc::EASE_OUT_EXPONENTIAL },
				  { "EaseInOutExponential", EasingFunc::EASE_IN_OUT_EXPONENTIAL },
				  { "EaseInBound", EasingFunc::EASE_IN_BOUNCE },
				  { "EaseOutBound", EasingFunc::EASE_OUT_BOUNCE },
				  { "EaseInOutBound", EasingFunc::EASE_IN_OUT_BOUNCE },
				  { "EaseInCirc", EasingFunc::EASE_IN_CIRC },
				  { "EaseOutCirc", EasingFunc::EASE_OUT_CIRC },
				  { "EaseInOutCirc", EasingFunc::EASE_IN_OUT_CIRC } });

			lua.new_usertype<Tween>("Tween",
				sol::call_constructor,
				sol::constructors<Tween(), Tween(float, float, float, EasingFunc)>(),
				"update",
				&Tween::Update,
				"totalDistance",
				&Tween::TotalDistance,
				"currentValue",
				&Tween::CurrentValue,
				"isFinished",
				&Tween::IsFinished);
		};

	auto create_lua_logger = [](sol::state& lua) {
		auto& logger = Log::GetInstance();

		lua.new_usertype<Log>(
			"Logger",
			sol::no_constructor,
			"trace", [&](const std::string_view message) { logger.LuaTrace(message); },
			"info", [&](const std::string_view message) { logger.LuaInfo(message); },
			"warn", [&](const std::string_view message) { logger.LuaWarn(message); },
			"error", [&](const std::string_view message) { logger.LuaError(message); }
		);

		auto traceResult = lua.safe_script(R"(
				function XXX_Trace(message, ...)
					Logger.trace(string.format(message, ...))
				end
			)");
		if (!traceResult.valid())
			F_ERROR("Failed to initialize lua traces!");

		auto infoResult = lua.safe_script(R"(
				function XXX_Info(message, ...)
					Logger.info(string.format(message, ...))
				end
			)");
		if (!infoResult.valid())
			F_ERROR("Failed to initialize lua infos!");

		auto warnResult = lua.safe_script(R"(
				function XXX_Warn(message, ...)
					Logger.warn(string.format(message, ...))
				end
			)");
		if (!warnResult.valid())
			F_ERROR("Failed to initialize lua warnings!");

		auto errorResult = lua.safe_script(R"(
				function XXX_Error(message, ...)
					Logger.error(string.format(message, ...))
				end
			)");
		if (!errorResult.valid())
			F_ERROR("Failed to initialize lua errors!");

		lua.set_function("F_trace", [](const std::string& message, const sol::variadic_args& args, sol::this_state s) {
			try
			{
				sol::state_view L = s;
				sol::protected_function log = L["XXX_Trace"];
				auto result = log(message, args);
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to get lua traces: {}", error.what());
			}
		});

		lua.set_function("F_info", [](const std::string& message, const sol::variadic_args& args, sol::this_state s) {
			try
			{
				sol::state_view L = s;
				sol::protected_function info = L["XXX_Info"];
				auto result = info(message, args);
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to get lua infos: {}", error.what());
			}
		});

		lua.set_function("F_warn", [](const std::string& message, const sol::variadic_args& args, sol::this_state s) {
			try
			{
				sol::state_view L = s;
				sol::protected_function warn = L["XXX_Warn"];
				auto result = warn(message, args);
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to get lua warnings: {}", error.what());
			}
		});

		lua.set_function("F_error", [](const std::string& message, const sol::variadic_args& args, sol::this_state s) {
			try
			{
				sol::state_view L = s;
				sol::protected_function err = L["XXX_Error"];
				auto result = err(message, args);
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to get lua errors: {}", error.what());
			}
		});

		auto assertResult = lua.safe_script(R"(
				F_assert = assert
				assert = function(arg1, message, ...)
					if not arg1 then 
						Logger.error(string.format(message, ...))
					end 
					F_assert(arg1)
				end
			)");
	};

	void ScriptingSystem::RegisterLuaBindings(sol::state& lua, Registry& registry)
	{
		GLMBinding::CreateGLMBindings(lua);
		InputManager::CreateLuaInputBindings(lua, registry);
		AssetManager::CreateLuaAssetManager(lua);
		SoundBinder::CreateSoundBind(lua);
		RendererBinder::CreateRenderingBind(lua, registry);
		UserDataBinder::CreateLuaUserData(lua);
		ContactListenerBinder::CreateLuaContactListener(lua, registry.GetRegistry());
		LuaFilesystem::CreateLuaFileSystemBind(lua);
		ScriptingHelpers::CreateLuaHelpers(lua);

		FollowCamera::CreateLuaFollowCamera(lua, registry);
		Character::CreateCharacterLuaBind(lua, registry);

		create_timer(lua);
		create_lua_logger(lua);
		createTweenLuaBind(lua);

		State::CreateLuaStateBind(lua);
		StateStack::CreateLuaStateStackBind(lua);
		StateMachine::CreateLuaStateMachineBind(lua);

		Registry::CreateLuaRegistryBind(lua, registry);
		Entity::CreateLuaEntityBind(lua, registry);
		TransformComponent::CreateLuaTransformBind(lua);
		SpriteComponent::CreateSpriteLuaBind(lua);
		AnimationComponent::CreateAnimationLuaBind(lua);
		BoxColliderComponent::CreateLuaBoxColliderBind(lua);
		CircleColliderComponent::CreateLuaCircleColliderBind(lua);
		PhysicsComponent::CreatePhysicsLuaBind(lua, registry.GetRegistry());
		TextComponent::CreateLuaTextBindings(lua);
		RigidBodyComponent::CreateRigidBodyLuaBind(lua);
		UIComponent::CreateLuaBind(lua);
	}

	void ScriptingSystem::RegisterLuaFunctions(sol::state& lua, Registry& registry)
	{
		auto& mainRegistry = MAIN_REGISTRY();

		lua.set_function(
			"F_RunScript", [&](const std::string& path)
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

		lua.set_function("F_LoadScriptTable", [&](const sol::table& scriptList) {
			if (!scriptList.valid())
			{
				F_ERROR("Failed to load script list: Table is invalid");
				return;
			}

			for (const auto& [index, script] : scriptList)
			{
				try
				{
					auto result = lua.safe_script_file(script.as<std::string>());
					if (!result.valid())
					{
						sol::error error = result;
						throw error;
					}
				}
				catch (const sol::error& error)
				{
					F_ERROR("Failed to load script '{}', Error: {}", script.as<std::string>(), error.what());
					return;
				}
			}
		});

		lua.set_function("F_GetTicks", [] { return SDL_GetTicks(); });

		auto& assetManager = mainRegistry.GetAssetManager();
		lua.set_function("F_MeasureText",
			sol::overload(
				[&](const std::string& text, const std::string& fontName)
				{
					const auto& pFont = assetManager.GetFont(fontName);
					if (!pFont)
					{
						F_ERROR("Failed to get font '{}': Does not exist in asset manager!", fontName);
						return -1.0f;
					}

					glm::vec2 position{ 0.0f }, temp_pos{ position };
					for (const auto& character : text)
						pFont->GetNextCharPos(character, temp_pos);

					return std::abs((position - temp_pos).x);
				},
				[&](const TextComponent& text, const TransformComponent& transform)
				{
					return GetTextBlockSize(text, transform, assetManager);
				}
			)
		);

		auto& engine = CoreEngineData::GetInstance();
		lua.set_function("F_DeltaTime", [&] { return engine.GetDeltaTime(); });
		lua.set_function("F_WindowWidth", [&] { return engine.WindowWidth(); });
		lua.set_function("F_WindowHeight", [&] { return engine.WindowHeight(); });

		// Physics Enable functions
		lua.set_function("F_DisablePhysics", [&] { engine.DisablePhysics(); });
		lua.set_function("F_EnablePhysics", [&] { engine.EnablePhysics(); });
		lua.set_function("F_IsPhysicsEnabled", [&] { return engine.IsPhysicsEnabled(); });

		// Render Colliders Enable functions
		lua.set_function("F_DisableCollisionRendering", [&] { engine.DisableColliderRender(); });
		lua.set_function("F_EnableCollisionRendering", [&] { engine.EnableColliderRender(); });
		lua.set_function("F_CollisionRenderingEnabled", [&] { return engine.RenderCollidersEnabled(); });

		// Animation Enable functions
		lua.set_function("F_DisableAnimationRendering", [&] { engine.DisableAnimationRender(); });
		lua.set_function("F_EnableAnimationRendering", [&] { engine.EnableAnimationRender(); });
		lua.set_function("F_AnimationRenderingEnabled", [&] { return engine.AnimationRenderEnabled(); });

		lua.set_function("F_GetProjecPath", [&] { return engine.GetProjectPath(); });

		lua.new_usertype<RandomIntGenerator>(
			"RandomInt",
			sol::call_constructor,
			sol::constructors<RandomIntGenerator(uint32_t, uint32_t), RandomIntGenerator()>(),
			"get_value", &RandomIntGenerator::GetValue
		);

		lua.new_usertype<RandomFloatGenerator>(
			"RandomFloat",
			sol::call_constructor,
			sol::constructors<RandomFloatGenerator(float, float), RandomFloatGenerator()>(),
			"get_value", &RandomFloatGenerator::GetValue
		);

		lua.set_function(
			"F_EntityInView",
			[&](const TransformComponent& transform, float width, float height) {
				auto& camera = registry.GetContext<std::shared_ptr<Camera2D>>();
				return EntityInView(transform, width, height, *camera);
			}
		);

		Scene::CreateLuaBind(lua);
	}

	void ScriptingSystem::RegisterLuaEvents(sol::state& lua, Registry& registry)
	{
		auto* dispatcher = registry.TryGetContext<std::shared_ptr<EventDispatcher>>();
		F_ASSERT(dispatcher && "There must be at least one registered dispatcher");

		LuaEventBinder::CreateLuaEventBindings(lua);
		EventDispatcher::RegisterMetaEventFuncs<ContactEvent>();
		EventDispatcher::RegisterMetaEventFuncs<KeyEvent>();
		EventDispatcher::RegisterMetaEventFuncs<LuaEvent>();
		EventDispatcher::CreateEventDispatcherLuaBind(lua, **dispatcher);
	}

	void ScriptingSystem::RegisterLuaSystems(sol::state& lua, Registry& registry)
	{
		RenderSystem::CreateRenderSystemLuaBind(lua, registry);
		RenderUISystem::CreateRenderUISystemLuaBind(lua);
		AnimationSystem::CreateAnimationSystemLuaBind(lua, registry);
	}

}
