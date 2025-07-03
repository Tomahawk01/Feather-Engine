#include "PackageDisplay.h"

#include "Logger/Logger.h"
#include "Core/CoreUtils/SaveProject.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/ECS/MainRegistry.h"
#include "Utils/HelperUtilities.h"

#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Scene/SceneManager.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <filesystem>

namespace fs = std::filesystem;

namespace Feather {

	PackageGameDisplay::PackageGameDisplay()
		: m_GameConfig{ std::make_unique<GameConfig>() }
		, m_DestinationPath{}
		, m_ScriptListPath{}
		, m_FileIconPath{}
		, m_Resizable{ false }
		, m_Borderless{ false }
		, m_FullScreen{ false }
		, m_Titlebar{ false }
		, m_ScriptListExist{ false }
	{
		const auto& saveProject = MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>();
		m_ScriptListPath = std::format("{0}{1}{2}{3}{2}{4}", saveProject->projectPath, "content", PATH_SEPARATOR, "scripts", "script_list.lua");

		m_ScriptListExist = fs::exists(fs::path{ m_ScriptListPath });
	}

	PackageGameDisplay::~PackageGameDisplay() = default;

	void PackageGameDisplay::Update()
	{
		// TODO: Handle packager
	}

	void PackageGameDisplay::Draw()
	{
		if (!ImGui::Begin("Package Game"))
		{
			ImGui::End();
			return;
		}

		ImGui::SeparatorText("Package and Export Game");
		ImGui::NewLine();

		if (ImGui::BeginChild("Game Packager"))
		{
			ImGui::SeparatorText("File Information");
			ImGui::PushItemWidth(256.0f);
			ImGui::InlineLabel("Game Title");
			ImGui::InputText("##gameTitle", &m_GameConfig->gameName);

			ImGui::InlineLabel("Destination");
			ImGui::InputText("##destination", &m_DestinationPath);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("...""##dest"))
			{
				// TODO: Open file dialog and set destination path
			}

			ImGui::InlineLabel("Icon");
			ImGui::PushItemWidth(256.0f);
			ImGui::InputText("##icon", &m_FileIconPath);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("...""##iconpath"))
			{
				// TODO: Open file dialog and set file icon path
			}

			ImGui::InlineLabel("Package Assets");
			ImGui::ItemToolTip("Convert assets into luac files and add them to zip archive");
			ImGui::Checkbox("##packageassets", &m_GameConfig->packageAssets);
			ImGui::AddSpaces(2);
			ImGui::Separator();
			ImGui::AddSpaces(3);

			ImGui::SeparatorText("Default Window Parameters");
			ImGui::AddSpaces(2);
			ImGui::InlineLabel("Window Size");
			ImGui::PushItemWidth(128.0f);
			ImGui::InputInt("##windowWidth", &m_GameConfig->windowWidth);
			ImGui::SameLine();
			ImGui::InputInt("##windowHeight", &m_GameConfig->windowHeight);
			ImGui::PopItemWidth();
			ImGui::Separator();
			ImGui::AddSpaces(2);
			ImGui::Text("Window Flags");
			ImGui::Separator();

			ImGui::InlineLabel("Resizable");
			if (ImGui::Checkbox("##resizable", &m_Resizable))
			{
				if (m_Resizable)
				{
					m_FullScreen = false;
				}
			}

			ImGui::InlineLabel("Fullscreen");
			if (ImGui::Checkbox("##fullscreen", &m_FullScreen))
			{
				if (m_FullScreen)
				{
					m_Resizable = false;
					m_Titlebar = false;
				}
			}

			ImGui::InlineLabel("Titlebar");
			if (ImGui::Checkbox("##titlebar", &m_Titlebar))
			{
				if (m_Titlebar)
				{
					m_FullScreen = false;
					m_Borderless = false;
				}
			}

			ImGui::InlineLabel("Borderless");
			if (ImGui::Checkbox("##borderless", &m_Borderless))
			{
				if (m_Borderless)
				{
					m_Titlebar = false;
				}
			}

			ImGui::Separator();
			ImGui::AddSpaces(3);
			ImGui::SeparatorText("Default Camera Parameters");
			ImGui::AddSpaces(3);

			ImGui::InlineLabel("Camera Size");
			ImGui::PushItemWidth(128.0f);
			ImGui::InputInt("##cameraWidth", &m_GameConfig->cameraWidth);
			ImGui::SameLine();
			ImGui::InputInt("##cameraHeight", &m_GameConfig->cameraHeight);

			ImGui::InlineLabel("Camera Scale");
			ImGui::InputFloat("##camerascale", &m_GameConfig->cameraScale);
			ImGui::PopItemWidth();
			ImGui::AddSpaces(3);

			ImGui::SeparatorText("Startup Options");
			ImGui::PushItemWidth(256.0f);

			if (ImGui::BeginCombo("##start_up_scenes", m_GameConfig->startupScene.c_str()))
			{
				for (const auto& sceneName : SCENE_MANAGER().GetSceneNames())
				{
					if (ImGui::Selectable(sceneName.c_str(), sceneName == m_GameConfig->startupScene))
					{
						m_GameConfig->startupScene = sceneName;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::PopItemWidth();

			ImGui::EndChild();
		}

		ImGui::Separator();
		ImGui::AddSpaces(3);

		if (ImGui::Button("Package Game"))
		{
			F_TRACE("PACKED THE GAME");
		}

		ImGui::End();
	}

}
