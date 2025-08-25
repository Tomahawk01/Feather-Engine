#include "Runtime.h"

#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/CoreUtils/EngineShaders.h"
#include "Core/Resources/AssetManager.h"
#include "Core/Events/EventDispatcher.h"
#include "Core/Events/EngineEventTypes.h"
#include "Core/Scripting/InputManager.h"
#include "Core/Scripting/CrashLoggerTestBindings.h"
#include "Core/Systems/AnimationSystem.h"
#include "Core/Systems/PhysicsSystem.h"
#include "Core/Systems/ScriptingSystem.h"
#include "Core/Systems/RenderSystem.h"
#include "Core/Systems/RenderUISystem.h"
#include "Core/Systems/RenderShapeSystem.h"
#include "Physics/Box2DWrappers.h"
#include "Physics/ContactListener.h"
#include "Logger/Logger.h"
#include "Logger/CrashLogger.h"
#include "Utils/HelperUtilities.h"
#include "Utils/FeatherUtilities.h"
#include "Windowing/Window/Window.h"
#include "Windowing/Input/Mouse.h"
#include "Windowing/Input/Keyboard.h"
#include "Windowing/Input/Gamepad.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/Renderer.h"
#include "Core/Loaders/TilemapLoader.h"
#include "Core/CoreUtils/ProjectInfo.h"

#include <SDL.h>
#include <sol/sol.hpp>
#include <glad/glad.h>
#include <libzippp/libzippp.h>

namespace Feather {

	RuntimeApp::RuntimeApp()
		: m_Window{ nullptr }
		, m_Event{}
		, m_Running{ true }
		, m_GameConfig{ std::make_unique<GameConfig>() }
	{}

	RuntimeApp::~RuntimeApp()
	{}

	void RuntimeApp::Run()
	{
		Initialize();

		while (m_Running)
		{
			ProcessEvents();
			Update();
			Render();
		}

		CleanUp();
	}

	void RuntimeApp::Initialize()
	{
		F_INIT_LOGS(true, false);

		FEATHER_INIT_CRASH_LOGS();

		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		{
			std::string error = SDL_GetError();
			throw std::runtime_error(std::format("Failed to initialize SDL: {}", error));
		}

		if (SDL_GL_LoadLibrary(NULL) != 0)
		{
			std::string error = SDL_GetError();
			throw std::runtime_error(std::format("Failed to load OpenGL Library: {}", error));
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

		auto luaState = std::make_shared<sol::state>();
		luaState->open_libraries(sol::lib::base,
								  sol::lib::math,
								  sol::lib::os,
								  sol::lib::table,
								  sol::lib::io,
								  sol::lib::string,
								  sol::lib::package,
								  sol::lib::coroutine);

		if (!luaState || !LoadConfig(*luaState))
		{
			throw std::runtime_error("Failed to initialize the game configuration");
		}

		FEATHER_CRASH_LOGGER().SetLuaState(luaState->lua_state());

		CrashLoggerTests::CreateLuaBind(*luaState);

		auto& coreGlobals = CORE_GLOBALS();

		m_Window = std::make_unique<Window>(m_GameConfig->gameName.c_str(),
											m_GameConfig->windowWidth,
											m_GameConfig->windowHeight,
											SDL_WINDOWPOS_CENTERED,
											SDL_WINDOWPOS_CENTERED,
											true,
											m_GameConfig->windowFlags | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

		m_Window->SetGLContext(SDL_GL_CreateContext(m_Window->GetWindow().get()));

		if (gladLoadGLLoader(SDL_GL_GetProcAddress) == 0)
		{
			throw std::runtime_error("Failed to initialize GLAD");
		}

		if (!m_Window->GetGLContext())
		{
			std::string error = SDL_GetError();
			throw std::runtime_error(std::format("Failed to create OpenGL context: {}", error));
		}

		if ((SDL_GL_MakeCurrent(m_Window->GetWindow().get(), m_Window->GetGLContext())) != 0)
		{
			std::string error = SDL_GetError();
			throw std::runtime_error(std::format("Failed to make OpenGL context current: {}", error));
		}

		SDL_GL_SetSwapInterval(1);

		auto& mainRegistry = MAIN_REGISTRY();
		if (!mainRegistry.Initialize())
		{
			throw std::runtime_error("Failed to initialize Main Registry");
		}

		if (!mainRegistry.GetAssetManager().CreateDefaultFonts())
		{
			throw std::runtime_error("Failed to create default fonts");
		}

		mainRegistry.AddToContext<std::shared_ptr<sol::state>>(std::move(luaState));

		if (!LoadShaders())
		{
			throw std::runtime_error("Failed to load game shaders");
		}

		LoadRegistryContext();
		LoadBindings();
		CoreEngineData::RegisterMetaFunctions();

		if (m_GameConfig->packageAssets && !LoadZip())
		{
			throw std::runtime_error("Failed to load game assets zip file");
		}

		if (!LoadScripts())
		{
			throw std::runtime_error("Failed to load game scripts");
		}

		TilemapLoader tl{};
		auto& lua = mainRegistry.GetContext<std::shared_ptr<sol::state>>();
		tl.LoadTilemapFromLuaTable(*mainRegistry.GetRegistry(), (*lua)[m_GameConfig->startupScene + "_tilemap"]);
		tl.LoadGameObjectsFromLuaTable(*mainRegistry.GetRegistry(), (*lua)[m_GameConfig->startupScene + "_objects"]);
		if (coreGlobals.IsPhysicsEnabled())
		{
			LoadPhysics();
		}
	}

	bool RuntimeApp::LoadShaders()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		if (!assetManager.AddShaderFromMemory("basic", basicShaderVert, basicShaderFrag))
		{
			F_ERROR("Failed to add the basic shader to the asset manager");
			return false;
		}

		if (!assetManager.AddShaderFromMemory("color", colorShaderVert, colorShaderFrag))
		{
			F_ERROR("Failed to add the color shader to the asset manager");
			return false;
		}

		if (!assetManager.AddShaderFromMemory("circle", circleShaderVert, circleShaderFrag))
		{
			F_ERROR("Failed to add the color shader to the asset manager");
			return false;
		}

		if (!assetManager.AddShaderFromMemory("font", fontShaderVert, fontShaderFrag))
		{
			F_ERROR("Failed to add the font shader to the asset manager");
			return false;
		}

		return true;
	}

	bool RuntimeApp::LoadConfig(sol::state& lua)
	{
		auto& coreGlobals = CORE_GLOBALS();
		const auto configFile = std::format("config{}config.luac", PATH_SEPARATOR);
		if (!fs::exists(fs::path{ configFile }))
		{
			F_ERROR("Failed to load configuration: '{}' config file does not exist", configFile);
			return false;
		}

		try
		{
			lua.safe_script_file(configFile);
		}
		catch (const sol::error& err)
		{
			throw std::runtime_error(std::format("Failed to load configuration: {}", err.what()));
		}

		sol::optional<sol::table> maybeConfig = lua["GameConfig"];
		if (!maybeConfig)
		{
			throw std::runtime_error("Game config file is missing the \"GameConfig\" table");
		}

		const std::string gameName = (*maybeConfig)["GameName"].get_or(std::string{ "GAME NAME NOT PROVIDED" });
		m_GameConfig->gameName = gameName;

		const std::string sStartScene = (*maybeConfig)["StartupScene"].get_or(std::string{ "" });

		if (sStartScene.empty())
		{
			throw std::runtime_error("Game config file is missing the \"StartScene\" entry");
		}

		m_GameConfig->startupScene = sStartScene;

		sol::optional<sol::table> maybeWindow = (*maybeConfig)["WindowParams"];
		if (!maybeWindow)
		{
			throw std::runtime_error("Game config file is missing the \"WindowParams\" table");
		}

		int windowWidth = (*maybeWindow)["width"].get_or(640);
		int windowHeight = (*maybeWindow)["height"].get_or(480);
		uint32_t windowFlags = (*maybeWindow)["flags"].get_or(0U);
		coreGlobals.SetWindowWidth(windowWidth);
		coreGlobals.SetWindowHeight(windowHeight);
		m_GameConfig->windowWidth = windowWidth;
		m_GameConfig->windowHeight = windowHeight;
		m_GameConfig->windowFlags = windowFlags;

		sol::optional<sol::table> maybePhysics = (*maybeConfig)["PhysicsParams"];
		if (maybePhysics)
		{
			bool physicsEnabled = (*maybePhysics)["enabled"].get_or(false);
			int32_t positionIterations = (*maybePhysics)["positionIterations"].get_or(0);
			int32_t velocityIterations = (*maybePhysics)["velocityIterations"].get_or(0);
			float gravity = (*maybePhysics)["gravity"].get_or(9.8f);
			physicsEnabled ? coreGlobals.EnablePhysics() : coreGlobals.DisablePhysics();

			coreGlobals.SetPositionIterations(positionIterations);
			coreGlobals.SetVelocityIterations(velocityIterations);
			coreGlobals.SetGravity(gravity);
		}

		// TODO: Flags

		m_GameConfig->packageAssets = (*maybeConfig)["PackageAssets"].get_or(false);

		return true;
	}

	bool RuntimeApp::LoadRegistryContext()
	{
		auto& coreGlobals = CORE_GLOBALS();
		auto& mainRegistry = MAIN_REGISTRY();
		mainRegistry.AddToContext<std::shared_ptr<Camera2D>>(std::make_shared<Camera2D>(coreGlobals.WindowWidth(), coreGlobals.WindowHeight()));

		if (coreGlobals.IsPhysicsEnabled())
		{
			auto physicsWorld = mainRegistry.AddToContext<PhysicsWorld>(std::make_shared<b2World>(b2Vec2{ 0.0f, coreGlobals.GetGravity() }));
			auto contactListener = mainRegistry.AddToContext<std::shared_ptr<ContactListener>>(std::make_shared<ContactListener>());

			physicsWorld->SetContactListener(contactListener.get());
		}

		mainRegistry.AddToContext<std::shared_ptr<ScriptingSystem>>(std::make_shared<ScriptingSystem>());

		return false;
	}

	void RuntimeApp::LoadBindings()
	{
		auto& mainRegistry = MAIN_REGISTRY();

		auto& luaState = mainRegistry.GetContext<std::shared_ptr<sol::state>>();
		auto* registry = mainRegistry.GetRegistry();

		ScriptingSystem::RegisterLuaBindings(*luaState, *registry);
		ScriptingSystem::RegisterLuaFunctions(*luaState, *registry);
		ScriptingSystem::RegisterLuaEvents(*luaState, *registry);
		ScriptingSystem::RegisterLuaSystems(*luaState, *registry);
	}

	bool RuntimeApp::LoadScripts()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& scriptSystem = mainRegistry.GetContext<std::shared_ptr<ScriptingSystem>>();
		auto& lua = mainRegistry.GetContext<std::shared_ptr<sol::state>>();

		return scriptSystem->LoadMainScript("assets/scripts/master.luac", *mainRegistry.GetRegistry(), *lua);
	}

	bool RuntimeApp::LoadPhysics()
	{
		auto& coreGlobals = CORE_GLOBALS();
		auto& mainRegistry = MAIN_REGISTRY();
		auto* registry = mainRegistry.GetRegistry();
		auto& physicsWorld = mainRegistry.GetContext<PhysicsWorld>();
		auto& camera = mainRegistry.GetContext<std::shared_ptr<Camera2D>>();

		auto physicsEntities = registry->GetRegistry().view<PhysicsComponent>();
		for (auto entity : physicsEntities)
		{
			Entity ent{ *registry, entity };

			bool bBoxCollider{ ent.HasComponent<BoxColliderComponent>() };
			bool bCircleCollider{ ent.HasComponent<CircleColliderComponent>() };

			if (!bBoxCollider && !bCircleCollider)
			{
				F_ERROR("Entity must have a box or circle collider component to initialize physics on it");
				continue;
			}

			auto& physics = ent.GetComponent<PhysicsComponent>();
			auto& physicsAttributes = physics.GetChangableAttributes();

			if (bBoxCollider)
			{
				const auto& boxCollider = ent.GetComponent<BoxColliderComponent>();
				physicsAttributes.boxSize = glm::vec2{ boxCollider.width, boxCollider.height };
				physicsAttributes.offset = boxCollider.offset;
			}

			else if (bCircleCollider)
			{
				const auto& circleCollider = ent.GetComponent<CircleColliderComponent>();
				physicsAttributes.radius = circleCollider.radius;
				physicsAttributes.offset = circleCollider.offset;
			}

			const auto& transform = ent.GetComponent<TransformComponent>();
			physicsAttributes.position = transform.position;
			physicsAttributes.scale = transform.scale;
			physicsAttributes.objectData.entityID = static_cast<std::int32_t>(entity);

			physics.Init(physicsWorld, camera->GetWidth(), camera->GetHeight());

			if (physics.UseFilters()) // TODO: Right now filters are disabled, since there is no way to set this from the editor
			{
				physics.SetFilterCategory();
				physics.SetFilterMask();

				// TODO: Should the group index be set based on the sprite layer?
				physics.SetGroupIndex();
			}
		}

		return true;
	}

	bool RuntimeApp::LoadZip()
	{
		auto& assetManager = MAIN_REGISTRY().GetAssetManager();
		auto& coreGlobals = CORE_GLOBALS();

		const std::string zipAssetsPath{ std::format("{}{}{}", "assets", PATH_SEPARATOR, "ScionAssets.zip") };

		if (!fs::exists(fs::path{ zipAssetsPath }))
		{
			throw std::runtime_error(std::format("Failed to load zipped assets at path: {}", zipAssetsPath));
		}

		libzippp::ZipArchive zipArchive{ zipAssetsPath };
		zipArchive.open(libzippp::ZipArchive::ReadOnly);
		std::vector<libzippp::ZipEntry> entries = zipArchive.getEntries();

		for (const auto& entry : entries)
		{
			auto text = entry.readAsText();
			sol::state lua;
			try
			{
				auto result = lua.safe_script(text);
				if (!result.valid())
				{
					sol::error error = result;
					throw error;
				}
			}
			catch (const sol::error& error)
			{
				F_ERROR("Failed to read in assets! {}", error.what());
			}

			sol::optional<sol::table> fAsset = lua["F_Assets"];
			if (fAsset)
			{
				for (const auto& [index, assetTable] : *fAsset)
				{
					sol::table asset = assetTable.as<sol::table>();
					auto featherAsset = std::make_unique<FAsset>();
					featherAsset->name = asset["assetName"].get_or(std::string{ "" });
					featherAsset->type = StringToAssetType(asset["assetType"].get_or(std::string{ "" }));
					featherAsset->assetSize = asset["dataSize"].get_or(0U);
					featherAsset->assetEnd = asset["dataEnd"].get_or(0U);

					// Get the asset data
					sol::table dataTable = asset["data"];
					for (const auto& [_, data] : dataTable)
					{
						auto value = data.as<unsigned char>();
						featherAsset->assetData.push_back(value);
					}

					m_mapFAssets[featherAsset->type].push_back(std::move(featherAsset));
				}
			}
		}

		for (const auto& [type, assets] : m_mapFAssets)
		{
			switch (type)
			{
				case AssetType::TEXTURE:
				{
					for (const auto& texAsset : assets)
					{
						if (!assetManager.AddTextureFromMemory(texAsset->name, texAsset->assetData.data(), texAsset->assetSize))
						{
							F_ERROR("Failed to add texture '{}' from memory", texAsset->name);
						}
					}
					break;
				}
				case AssetType::MUSIC:
				{
					for (const auto& musicAsset : assets)
					{
						if (!assetManager.AddMusicFromMemory(musicAsset->name, musicAsset->assetData.data(), musicAsset->assetSize))
						{
							F_ERROR("Failed to add music '{}' from memory", musicAsset->name);
						}
					}
					break;
				}
				case AssetType::SOUNDFX:
				{
					for (const auto& soundfxAsset : assets)
					{
						if (!assetManager.AddSoundFxFromMemory(soundfxAsset->name, soundfxAsset->assetData.data(), soundfxAsset->assetSize))
						{
							F_ERROR("Failed to add music '{}' from memory", soundfxAsset->name);
						}
					}
					break;
				}
				case AssetType::FONT:
				{
					for (const auto& fontAsset : assets)
					{
						if (!assetManager.AddFontFromMemory(fontAsset->name, fontAsset->assetData.data(), fontAsset->assetSize))
						{
							F_ERROR("Failed to add font '{}' from memory", fontAsset->name);
						}
					}
					break;
				}
			}
		}

		zipArchive.close();
		return true;
	}

	void RuntimeApp::ProcessEvents()
	{
		auto& inputManager = INPUT_MANAGER();
		auto& keyboard = inputManager.GetKeyboard();
		auto& mouse = inputManager.GetMouse();

		// Process Events
		while (SDL_PollEvent(&m_Event))
		{
			switch (m_Event.type)
			{
				case SDL_QUIT: m_Running = false; break;
				case SDL_KEYDOWN:
					keyboard.OnKeyPressed(m_Event.key.keysym.sym);
					EVENT_DISPATCHER().EmitEvent(KeyEvent{ .key = m_Event.key.keysym.sym, .type = EKeyEventType::Pressed });
					break;
				case SDL_KEYUP:
					keyboard.OnKeyReleased(m_Event.key.keysym.sym);
					EVENT_DISPATCHER().EmitEvent(KeyEvent{ .key = m_Event.key.keysym.sym, .type = EKeyEventType::Released });
					break;
				case SDL_MOUSEBUTTONDOWN:
					mouse.OnButtonPressed(m_Event.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					mouse.OnButtonReleased(m_Event.button.button);
					break;
				case SDL_MOUSEWHEEL:
					mouse.SetMouseWheelX(m_Event.wheel.x);
					mouse.SetMouseWheelY(m_Event.wheel.y);
					break;
				case SDL_MOUSEMOTION:
					mouse.SetMouseMoving(true);
					break;
				case SDL_CONTROLLERBUTTONDOWN:
					inputManager.GamepadButtonPressed(m_Event);
					break;
				case SDL_CONTROLLERBUTTONUP:
					inputManager.GamepadButtonReleased(m_Event);
					break;
				case SDL_CONTROLLERDEVICEADDED:
					inputManager.AddGamepad(m_Event.jdevice.which);
					break;
				case SDL_CONTROLLERDEVICEREMOVED:
					inputManager.RemoveGamepad(m_Event.jdevice.which);
					break;
				case SDL_JOYAXISMOTION:
					inputManager.GamepadAxisValues(m_Event);
					break;
				case SDL_JOYHATMOTION:
					inputManager.GamepadHatValues(m_Event);
					break;
				case SDL_WINDOWEVENT:
				{
					switch (m_Event.window.event)
					{
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							m_Window->SetSize(m_Event.window.data1, m_Event.window.data2);
							break;
						default:
							break;
					}
					break;
				}
				default:
					break;
			}
		}
	}

	void RuntimeApp::Update()
	{
		auto& coreGlobals = CORE_GLOBALS();
		auto& mainRegistry = MAIN_REGISTRY();
		auto* registry = mainRegistry.GetRegistry();

		double dt = coreGlobals.GetDeltaTime();
		coreGlobals.UpdateDeltaTime();

		// Clamp delta time to the target frame rate
		if (dt < TARGET_FRAME_TIME)
		{
			std::this_thread::sleep_for(std::chrono::duration<double>(TARGET_FRAME_TIME - dt));
		}

		auto& scriptSystem = mainRegistry.GetContext<std::shared_ptr<ScriptingSystem>>();
		scriptSystem->Update(*registry);

		if (coreGlobals.IsPhysicsEnabled() && !coreGlobals.IsPhysicsPaused())
		{
			auto& physicsWorld = mainRegistry.GetContext<PhysicsWorld>();
			physicsWorld->Step(TARGET_FRAME_TIME_F, coreGlobals.GetVelocityIterations(), coreGlobals.GetPositionIterations());
			physicsWorld->ClearForces();

			auto& dispatch = mainRegistry.GetContext<std::shared_ptr<EventDispatcher>>();

			// If there are no listeners for contact events, don't emit event
			if (dispatch->HasHandlers<ContactEvent>())
			{
				if (auto& contactListener = mainRegistry.GetContext<std::shared_ptr<ContactListener>>())
				{
					auto userDataA = contactListener->GetUserDataA();
					auto userDataB = contactListener->GetUserDataB();

					// Only emit contact event if both contacts are valid
					if (userDataA && userDataB)
					{
						try
						{
							auto ObjectA = std::any_cast<ObjectData>(userDataA->userData);
							auto ObjectB = std::any_cast<ObjectData>(userDataB->userData);

							dispatch->EmitEvent(ContactEvent{ .objectA = ObjectA, .objectB = ObjectB });
						}
						catch (const std::bad_any_cast& e)
						{
							F_ERROR("Failed to cast to object data: {}", e.what());
						}
					}
				}
			}

			auto& physicsSystem = mainRegistry.GetPhysicsSystem();
			physicsSystem.Update(*registry);
		}

		auto& camera = mainRegistry.GetContext<std::shared_ptr<Camera2D>>();
		mainRegistry.GetAnimationSystem().Update(*registry, *camera);

#ifdef DEBUG
		if (INPUT_MANAGER().GetKeyboard().IsKeyJustPressed(F_KEY_F2))
		{
			coreGlobals.ToggleRenderCollisions();
		}
#endif

		INPUT_MANAGER().UpdateInputs();
		camera->Update();
	}

	void RuntimeApp::Render()
	{
		auto& coreGlobals = CORE_GLOBALS();
		auto& mainRegistry = MAIN_REGISTRY();
		auto& renderer = mainRegistry.GetRenderer();
		auto* registry = mainRegistry.GetRegistry();

		int w, h;
		SDL_GetWindowSize(m_Window->GetWindow().get(), &w, &h);

		renderer.SetViewport(0, 0, w, h);
		renderer.SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		renderer.ClearBuffers(true, true, false);

		auto& camera = mainRegistry.GetContext<std::shared_ptr<Camera2D>>();
		mainRegistry.GetRenderSystem().Update(*mainRegistry.GetRegistry(), *camera);
		mainRegistry.GetRenderUISystem().Update(*mainRegistry.GetRegistry());

		if (coreGlobals.RenderCollidersEnabled())
		{
			mainRegistry.GetRenderShapeSystem().Update(*mainRegistry.GetRegistry(), *camera);
		}

		auto& scriptSystem = mainRegistry.GetContext<std::shared_ptr<ScriptingSystem>>();
		scriptSystem->Render(*registry);

		SDL_GL_SwapWindow(m_Window->GetWindow().get());
	}

	void RuntimeApp::CleanUp()
	{
		SDL_Quit();
	}

}
