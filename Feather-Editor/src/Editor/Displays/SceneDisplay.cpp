#include "SceneDisplay.h"

#include "Renderer/Buffers/Framebuffer.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Core/Renderer.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/Systems/AnimationSystem.h"
#include "Core/Systems/PhysicsSystem.h"
#include "Core/Systems/ScriptingSystem.h"
#include "Core/Systems/RenderSystem.h"
#include "Core/Systems/RenderUISystem.h"
#include "Core/Systems/RenderShapeSystem.h"
#include "Core/Scripting/CrashLoggerTestBindings.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/SaveProject.h"
#include "Core/Resources/AssetManager.h"
#include "Core/Events/EventDispatcher.h"
#include "Core/Events/EngineEventTypes.h"
#include "Windowing/Input/Keys.h"
#include "Sounds/MusicPlayer/MusicPlayer.h"
#include "Sounds/SoundPlayer/SoundFXPlayer.h"
#include "Physics/Box2DWrappers.h"
#include "Physics/ContactListener.h"
#include "Logger/Logger.h"
#include "Logger/CrashLogger.h"

#include "Editor/Utilities/EditorFramebuffers.h"
#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Scripting/EditorCoreLuaWrappers.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"

#include <imgui.h>
#include <thread>

constexpr float TARGET_FRAME_TIME_F = 1.0f / 60.0f;
constexpr double TARGET_FRAME_TIME = 1.0 / 60.0;

namespace Feather {

	SceneDisplay::SceneDisplay()
		: m_PlayScene{ false }
		, m_WindowActive{ false }
		, m_SceneLoaded{ false }
	{
		ADD_EVENT_HANDLER(KeyEvent, &SceneDisplay::HandleKeyEvent, *this);
	}

	void SceneDisplay::Draw()
	{
		static bool isOpen{ true };
		if (!ImGui::Begin("Scene", &isOpen))
		{
			ImGui::End();
			return;
		}

		DrawToolbar();
		RenderScene();

		if (ImGui::BeginChild("##SceneChild", ImVec2{ 0.0f, 0.0f }, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollWithMouse))
		{
			m_WindowActive = ImGui::IsWindowFocused();

			auto& editorFramebuffers = MAIN_REGISTRY().GetContext<std::shared_ptr<EditorFramebuffers>>();
			const auto& fb = editorFramebuffers->mapFramebuffers[FramebufferType::SCENE];

			if (auto currentScene = SCENE_MANAGER().GetCurrentSceneObject())
			{
				auto& runtimeRegistry = currentScene->GetRuntimeRegistry();
				// NOTE: We need to set the relative mouse window, so that any scripts will take into account
				// the position of the imgui window relative to the actual window position, size, etc.
				if (auto* mouseInfo = runtimeRegistry.TryGetContext<std::shared_ptr<MouseGuiInfo>>())
				{
					ImGuiIO io = ImGui::GetIO();
					auto relativePos = ImGui::GetCursorScreenPos();
					ImVec2 windowSize{ ImGui::GetWindowSize() };

					(*mouseInfo)->position = glm::vec2{ io.MousePos.x - relativePos.x, io.MousePos.y - relativePos.y };
					(*mouseInfo)->windowSize = glm::vec2{ fb->GetWidth(), fb->GetHeight() };
				}
			}

			ImGui::Image(
				(ImTextureID)(intptr_t)fb->GetTextureID(),
				ImVec2{
					 static_cast<float>(fb->GetWidth()),
					 static_cast<float>(fb->GetHeight())
				},
				ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f }
			);

			ImGui::EndChild();

			// Check for resize based on window size
			ImVec2 windowSize{ ImGui::GetWindowSize() };
			if (fb->GetWidth() != static_cast<int>(windowSize.x) || fb->GetHeight() != static_cast<int>(windowSize.y))
				fb->Resize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
		}

		ImGui::End();
	}

	void SceneDisplay::Update()
	{
		if (!m_PlayScene)
			return;

		auto currentScene = SCENE_MANAGER().GetCurrentSceneObject();
		if (!currentScene)
			return;

		auto& runtimeRegistry = currentScene->GetRuntimeRegistry();
		auto& mainRegistry = MAIN_REGISTRY();
		auto& coreGlobals = CORE_GLOBALS();

		double dt = coreGlobals.GetDeltaTime();
		coreGlobals.UpdateDeltaTime();

		// NOTE: Clamp delta time to the target frame rate
		if (dt < TARGET_FRAME_TIME)
		{
			std::this_thread::sleep_for(std::chrono::duration<double>(TARGET_FRAME_TIME - dt));
		}

		auto& camera = runtimeRegistry.GetContext<std::shared_ptr<Camera2D>>();
		if (!camera)
		{
			F_FATAL("Failed to get the camera from the registry context!");
			return;
		}
		camera->Update();

		auto& scriptSystem = runtimeRegistry.GetContext<std::shared_ptr<ScriptingSystem>>();
		scriptSystem->Update(runtimeRegistry);

		if (coreGlobals.IsPhysicsEnabled())
		{
			auto& physicsWorld = runtimeRegistry.GetContext<PhysicsWorld>();
			physicsWorld->Step(TARGET_FRAME_TIME_F, coreGlobals.GetVelocityIterations(), coreGlobals.GetPositionIterations());
			physicsWorld->ClearForces();

			auto& dispatch = runtimeRegistry.GetContext<std::shared_ptr<EventDispatcher>>();

			// If there are no listeners for contact events, don't emit event
			if (dispatch->HasHandlers<ContactEvent>())
			{
				if (auto& contactListener = runtimeRegistry.GetContext<std::shared_ptr<ContactListener>>())
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
		}

		auto& physicsSystem = mainRegistry.GetPhysicsSystem();
		physicsSystem.Update(runtimeRegistry);

		auto& animationSystem = mainRegistry.GetAnimationSystem();
		animationSystem.Update(runtimeRegistry, *camera);
	}

	void SceneDisplay::LoadScene()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentSceneObject();
		if (!currentScene)
			return;

		currentScene->CopySceneToRuntime();
		auto& runtimeRegistry = currentScene->GetRuntimeRegistry();

		const auto& canvas = currentScene->GetCanvas();
		auto camera = runtimeRegistry.AddToContext<std::shared_ptr<Camera2D>>(std::make_shared<Camera2D>(canvas.width, canvas.height));

		auto physicsWorld = runtimeRegistry.AddToContext<PhysicsWorld>(std::make_shared<b2World>(b2Vec2{ 0.0f, CORE_GLOBALS().GetGravity() }));
		auto contactListener = runtimeRegistry.AddToContext<std::shared_ptr<ContactListener>>(std::make_shared<ContactListener>());
		physicsWorld->SetContactListener(contactListener.get());

		// Add the temporary event dispatcher
		runtimeRegistry.AddToContext<std::shared_ptr<EventDispatcher>>(std::make_shared<EventDispatcher>());

		// Add necessary systems
		auto scriptSystem = runtimeRegistry.AddToContext<std::shared_ptr<ScriptingSystem>>(std::make_shared<ScriptingSystem>());
		runtimeRegistry.AddToContext<std::shared_ptr<MouseGuiInfo>>(std::make_shared<MouseGuiInfo>());

		auto lua = runtimeRegistry.AddToContext<std::shared_ptr<sol::state>>(std::make_shared<sol::state>());

		if (!lua)
			lua = std::make_shared<sol::state>();

		lua->open_libraries(sol::lib::base,
							sol::lib::math,
							sol::lib::os,
							sol::lib::table,
							sol::lib::io,
							sol::lib::string,
							sol::lib::package);

		ScriptingSystem::RegisterLuaBindings(*lua, runtimeRegistry);
		ScriptingSystem::RegisterLuaFunctions(*lua, runtimeRegistry);
		ScriptingSystem::RegisterLuaEvents(*lua, runtimeRegistry);
		ScriptingSystem::RegisterLuaSystems(*lua, runtimeRegistry);
		LuaCoreBinder::CreateLuaBind(*lua, runtimeRegistry);

		EditorSceneManager::CreateSceneManagerLuaBind(*lua);

		// Initialize all of the physics entities
		auto physicsEntities = runtimeRegistry.GetRegistry().view<PhysicsComponent>();
		
		for (auto entity : physicsEntities)
		{
			Entity ent{ runtimeRegistry, entity };

			bool boxCollider{ ent.HasComponent<BoxColliderComponent>() };
			bool circleCollider{ ent.HasComponent<CircleColliderComponent>() };

			if (!boxCollider && !circleCollider)
			{
				F_ERROR("Entity must have a box or circle collider component to initialize physics on it");
				continue;
			}

			auto& physics = ent.GetComponent<PhysicsComponent>();
			auto& physicsAttributes = physics.GetChangableAttributes();

			if (boxCollider)
			{
				const auto& boxCollider = ent.GetComponent<BoxColliderComponent>();
				physicsAttributes.boxSize = glm::vec2{ boxCollider.width, boxCollider.height };
				physicsAttributes.offset = boxCollider.offset;
			}
			else if (circleCollider)
			{
				const auto& circleCollider = ent.GetComponent<CircleColliderComponent>();
				physicsAttributes.radius = circleCollider.radius;
				physicsAttributes.offset = circleCollider.offset;
			}

			const auto& transform = ent.GetComponent<TransformComponent>();
			physicsAttributes.position = transform.position;
			physicsAttributes.scale = transform.scale;
			physicsAttributes.objectData.entityID = static_cast<int32_t>(entity);

			physics.Init(physicsWorld, camera->GetWidth(), camera->GetHeight());

			// Set Filters/Masks/Group Index
			if (physics.UseFilters()) // Right now filters are disabled, since there is no way to set this from the editor
			{
				physics.SetFilterCategory();
				physics.SetFilterMask();

				// Should the group index be set based on the sprite layer?
				physics.SetGroupIndex();
			}
		}

		// Get the main script path
		auto& saveProject = MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>();
		if (!scriptSystem->LoadMainScript(saveProject->mainLuaScript, runtimeRegistry, *lua))
		{
			F_FATAL("Failed to load main lua script");
			return;
		}

		// Setup Crash Tests
		CrashLoggerTests::CreateLuaBind(*lua);

		// Set the lua state for the crash logger
		// This is used to log the lua stack trace in case of a crash
		FEATHER_CRASH_LOGGER().SetLuaState(lua->lua_state());

		m_SceneLoaded = true;
		m_PlayScene = true;
	}

	void SceneDisplay::UnloadScene()
	{
		m_PlayScene = false;
		m_SceneLoaded = false;
		auto currentScene = SCENE_MANAGER().GetCurrentSceneObject();
		F_ASSERT(currentScene && "Current Scene must be valid");

		auto& runtimeRegistry = currentScene->GetRuntimeRegistry();

		runtimeRegistry.ClearRegistry();
		runtimeRegistry.RemoveContext<std::shared_ptr<Camera2D>>();
		runtimeRegistry.RemoveContext<PhysicsWorld>();
		runtimeRegistry.RemoveContext<std::shared_ptr<ContactListener>>();
		runtimeRegistry.RemoveContext<std::shared_ptr<ScriptingSystem>>();
		runtimeRegistry.RemoveContext<std::shared_ptr<EventDispatcher>>();
		runtimeRegistry.RemoveContext<std::shared_ptr<sol::state>>();

		auto& mainRegistry = MAIN_REGISTRY();
		mainRegistry.GetMusicPlayer().Stop();
		mainRegistry.GetSoundPlayer().Stop(-1);
	}

	void SceneDisplay::RenderScene() const
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& editorFramebuffers = mainRegistry.GetContext<std::shared_ptr<EditorFramebuffers>>();
		auto& renderer = mainRegistry.GetContext<std::shared_ptr<Renderer>>();

		auto& renderSystem = mainRegistry.GetRenderSystem();
		auto& renderUISystem = mainRegistry.GetRenderUISystem();
		auto& renderShapeSystem = mainRegistry.GetRenderShapeSystem();

		const auto& fb = editorFramebuffers->mapFramebuffers[FramebufferType::SCENE];

		fb->Bind();
		renderer->SetViewport(0, 0, fb->GetWidth(), fb->GetHeight());
		renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		renderer->ClearBuffers(true, true, false);

		auto currentScene = SCENE_MANAGER().GetCurrentSceneObject();

		if (currentScene && m_PlayScene)
		{
			auto& runtimeRegistry = currentScene->GetRuntimeRegistry();
			auto& camera = runtimeRegistry.GetContext<std::shared_ptr<Camera2D>>();
			renderSystem.Update(runtimeRegistry, *camera);
			if (CORE_GLOBALS().RenderCollidersEnabled())
				renderShapeSystem.Update(runtimeRegistry, *camera);
			renderUISystem.Update(runtimeRegistry);

			auto& scriptSystem = runtimeRegistry.GetContext<std::shared_ptr<ScriptingSystem>>();
			scriptSystem->Render(runtimeRegistry);
		}
		fb->Unbind();
		fb->CheckResize();
	}

	void SceneDisplay::HandleKeyEvent(const KeyEvent keyEvent)
	{
		if (m_SceneLoaded)
		{
			if (keyEvent.type == EKeyEventType::Released)
			{
				if (keyEvent.key == F_KEY_ESCAPE)
				{
					UnloadScene();
				}
			}
		}

		// Send double dispatch events to the scene dispatcher
		auto currentScene = SCENE_MANAGER().GetCurrentSceneObject();
		if (!currentScene)
			return;

		auto& runtimeRegistry = currentScene->GetRuntimeRegistry();

		if (auto* eventDispatcher = runtimeRegistry.TryGetContext<std::shared_ptr<EventDispatcher>>())
		{
			if (!eventDispatcher->get()->HasHandlers<KeyEvent>())
				return;

			eventDispatcher->get()->EmitEvent(keyEvent);
		}
	}

	void SceneDisplay::DrawToolbar()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		auto playTexture = assetManager.GetTexture("play_button");
		auto stopTexture = assetManager.GetTexture("stop_button");

		F_ASSERT(playTexture && stopTexture);

		ImGui::Separator();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });

		// Play button
		auto playTextureID = (ImTextureID)(intptr_t)playTexture->GetID();
		if (m_PlayScene && m_SceneLoaded)
		{
			ImGui::ActiveImageButton("##playButton", playTextureID);
		}
		else
		{
			if (ImGui::ImageButton("##playButton", playTextureID, TOOL_BUTTON_SIZE))
			{
				LoadScene();
			}
		}

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Play Scene");

		ImGui::SameLine();

		// Stop button
		auto stopTextureID = (ImTextureID)(intptr_t)stopTexture->GetID();
		if (!m_PlayScene && !m_SceneLoaded)
		{
			ImGui::ActiveImageButton("##stopButton", stopTextureID);
		}
		else
		{
			if (ImGui::ImageButton("##stopButton", stopTextureID, TOOL_BUTTON_SIZE))
			{
				UnloadScene();
			}
		}

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Stop Scene");

		ImGui::Separator();
		ImGui::PopStyleVar(1);
	}

}
