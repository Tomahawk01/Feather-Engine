#include "SceneDisplay.h"
#include "Renderer/Buffers/Framebuffer.h"
#include "Renderer/Core/Camera2D.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Systems/AnimationSystem.h"
#include "Core/Systems/PhysicsSystem.h"
#include "Core/Systems/ScriptingSystem.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/Resources/AssetManager.h"
#include "Sounds/MusicPlayer/MusicPlayer.h"
#include "Sounds/SoundPlayer/SoundFXPlayer.h"
#include "Physics/Box2DWrappers.h"
#include "Logger/Logger.h"

#include <imgui.h>

constexpr float one_over_sixty = 1.0f / 60.0f;

namespace Feather {

	SceneDisplay::SceneDisplay(Registry& registry)
		: m_Registry{ registry }, m_PlayScene{ false }, m_SceneLoaded{ false }
	{}

	void SceneDisplay::Draw()
	{
		static bool isOpen{ true };
		if (!ImGui::Begin("Scene", &isOpen))
		{
			ImGui::End();
			return;
		}

		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		auto playTexture = assetManager.GetTexture("play_button");
		auto stopTexture = assetManager.GetTexture("stop_button");

		// Play button
		if (m_PlayScene)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.9f, 0.0f, 0.5f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.0f, 0.9f, 0.0f, 0.5f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.0f, 0.9f, 0.0f, 0.5f });
		}

		if (ImGui::ImageButton((ImTextureID)playTexture->GetID(), ImVec2{ (float)playTexture->GetWidth() * 0.5f, (float)playTexture->GetHeight() * 0.5f }) && !m_SceneLoaded)
		{
			LoadScene();
		}
		if (ImGui::GetColorStackSize() > 0)
			ImGui::PopStyleColor(ImGui::GetColorStackSize());
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Play Scene");

		ImGui::SameLine();

		// Stop button
		if (!m_PlayScene)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.9f, 0.0f, 0.5f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.0f, 0.9f, 0.0f, 0.5f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.0f, 0.9f, 0.0f, 0.5f });
		}

		if (ImGui::ImageButton((ImTextureID)stopTexture->GetID(), ImVec2{ (float)playTexture->GetWidth() * 0.5f, (float)playTexture->GetHeight() * 0.5f }) && m_SceneLoaded)
		{
			UnloadScene();
		}
		if (ImGui::GetColorStackSize() > 0)
			ImGui::PopStyleColor(ImGui::GetColorStackSize());
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Stop Scene");

		if (ImGui::BeginChild("##SceneChild", ImVec2{ 0.0f, 0.0f }, NULL, ImGuiWindowFlags_NoScrollWithMouse))
		{
			const auto& fb = m_Registry.GetContext<std::shared_ptr<Framebuffer>>();

			ImGui::SetCursorPos(ImVec2{ 0.0f, 0.0f });

			ImGui::Image(
				(ImTextureID)fb->GetTextureID(),
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

		auto& mainRegistry = MAIN_REGISTRY();
		auto& coreGlobals = CORE_GLOBALS();

		auto& camera = m_Registry.GetContext<std::shared_ptr<Camera2D>>();
		if (!camera)
		{
			F_FATAL("Failed to get the camera from the registry context!");
			return;
		}
		camera->Update();

		auto& scriptSystem = m_Registry.GetContext<std::shared_ptr<ScriptingSystem>>();
		scriptSystem->Update();

		if (coreGlobals.IsPhysicsEnabled())
		{
			auto& physicsWorld = m_Registry.GetContext<PhysicsWorld>();
			physicsWorld->Step(one_over_sixty, coreGlobals.GetVelocityIterations(), coreGlobals.GetPositionIterations());
			physicsWorld->ClearForces();
		}

		auto& physicsSystem = m_Registry.GetContext<std::shared_ptr<PhysicsSystem>>();
		physicsSystem->Update(m_Registry.GetRegistry());

		auto& animationSystem = m_Registry.GetContext<std::shared_ptr<AnimationSystem>>();
		animationSystem->Update();
	}

	void SceneDisplay::LoadScene()
	{
		auto& scriptSystem = m_Registry.GetContext<std::shared_ptr<ScriptingSystem>>();
		auto& lua = m_Registry.GetContext<std::shared_ptr<sol::state>>();

		if (!lua)
			lua = std::make_shared<sol::state>();

		lua->open_libraries(sol::lib::base,
							sol::lib::math,
							sol::lib::os,
							sol::lib::table,
							sol::lib::io,
							sol::lib::string,
							sol::lib::package);

		ScriptingSystem::RegisterLuaBindings(*lua, m_Registry);
		ScriptingSystem::RegisterLuaFunctions(*lua, m_Registry);

		if (!scriptSystem->LoadMainScript(*lua))
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
		m_Registry.GetRegistry().clear();

		auto& lua = m_Registry.GetContext<std::shared_ptr<sol::state>>();
		lua.reset();

		auto& mainRegistry = MAIN_REGISTRY();
		mainRegistry.GetMusicPlayer().Stop();
		mainRegistry.GetSoundPlayer().Stop(-1);
	}
}
