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
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/Resources/AssetManager.h"
#include "Sounds/MusicPlayer/MusicPlayer.h"
#include "Sounds/SoundPlayer/SoundFXPlayer.h"
#include "Physics/Box2DWrappers.h"
#include "Physics/ContactListener.h"
#include "Logger/Logger.h"

#include "Editor/Utilities/EditorFramebuffers.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"

#include <imgui.h>

constexpr float one_over_sixty = 1.0f / 60.0f;

namespace Feather {

	SceneDisplay::SceneDisplay()
		: m_PlayScene{ false }, m_SceneLoaded{ false }
	{}

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
			auto& editorFramebuffers = MAIN_REGISTRY().GetContext<std::shared_ptr<EditorFramebuffers>>();
			const auto& fb = editorFramebuffers->mapFramebuffers[FramebufferType::SCENE];

			ImGui::SetCursorPos(ImVec2{ 0.0f, 0.0f });

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

		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		if (!currentScene)
			return;

		auto& runtimeRegistry = currentScene->GetRuntimeRegistry();
		auto& mainRegistry = MAIN_REGISTRY();
		auto& coreGlobals = CORE_GLOBALS();

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
			physicsWorld->Step(one_over_sixty, coreGlobals.GetVelocityIterations(), coreGlobals.GetPositionIterations());
			physicsWorld->ClearForces();
		}

		auto& physicsSystem = mainRegistry.GetPhysicsSystem();
		physicsSystem.Update(runtimeRegistry);

		auto& animationSystem = mainRegistry.GetAnimationSystem();
		animationSystem.Update(runtimeRegistry, *camera);
	}

	void SceneDisplay::LoadScene()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		if (!currentScene)
			return;

		currentScene->CopySceneToRuntime();
		auto& runtimeRegistry = currentScene->GetRuntimeRegistry();

		const auto& canvas = currentScene->GetCanvas();
		runtimeRegistry.AddToContext<std::shared_ptr<Camera2D>>(std::make_shared<Camera2D>(canvas.width, canvas.height));

		auto physicsWorld = runtimeRegistry.AddToContext<PhysicsWorld>(std::make_shared<b2World>(b2Vec2{ 0.0f, 9.8f }));
		auto contactListener = runtimeRegistry.AddToContext<std::shared_ptr<ContactListener>>(std::make_shared<ContactListener>());
		physicsWorld->SetContactListener(contactListener.get());

		// Add necessary systems
		auto scriptSystem = runtimeRegistry.AddToContext<std::shared_ptr<ScriptingSystem>>(std::make_shared<ScriptingSystem>());

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

			// TODO: Set Filters/Masks/Group Index

			physics.Init(physicsWorld, 640, 480);
		}

		if (!scriptSystem->LoadMainScript(runtimeRegistry, *lua))
		{
			F_FATAL("Failed to load main lua script");
			return;
		}

		m_SceneLoaded = true;
		m_PlayScene = true;
	}

	void SceneDisplay::UnloadScene()
	{
		m_PlayScene = false;
		m_SceneLoaded = false;
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		auto& runtimeRegistry = currentScene->GetRuntimeRegistry();

		runtimeRegistry.ClearRegistry();
		runtimeRegistry.RemoveContext<std::shared_ptr<Camera2D>>();
		runtimeRegistry.RemoveContext<std::shared_ptr<sol::state>>();
		runtimeRegistry.RemoveContext<std::shared_ptr<PhysicsWorld>>();
		runtimeRegistry.RemoveContext<std::shared_ptr<ContactListener>>();
		runtimeRegistry.RemoveContext<std::shared_ptr<ScriptingSystem>>();

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

		auto currentScene = SCENE_MANAGER().GetCurrentScene();

		if (currentScene && m_PlayScene)
		{
			auto& runtimeRegistry = currentScene->GetRuntimeRegistry();
			auto& camera = runtimeRegistry.GetContext<std::shared_ptr<Camera2D>>();
			renderSystem.Update(runtimeRegistry, *camera);
			if (CORE_GLOBALS().RenderCollidersEnabled())
				renderShapeSystem.Update(runtimeRegistry, *camera);
			renderUISystem.Update(runtimeRegistry);
		}
		fb->Unbind();
		fb->CheckResize();
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
			ImGui::ActiveImageButton(playTextureID);
		}
		else
		{
			if (ImGui::ImageButton(playTextureID, TOOL_BUTTON_SIZE))
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
			ImGui::ActiveImageButton(stopTextureID);
		}
		else
		{
			if (ImGui::ImageButton(stopTextureID, TOOL_BUTTON_SIZE))
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
