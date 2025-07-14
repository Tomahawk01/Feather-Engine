#include "PackageDisplay.h"

#include "Logger/Logger.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/ECS/MainRegistry.h"
#include "FileSystem/Dialogs/FileDialog.h"
#include "Utils/HelperUtilities.h"

#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/EditorUtilities.h"
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
		const auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		auto optScriptListPath = projectInfo->GetScriptListPath();
		F_ASSERT(optScriptListPath && "Script List path not set correctly in project info");

		m_ScriptListPath = optScriptListPath->string();
		m_ScriptListExist = fs::exists(*optScriptListPath);
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

		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();

		ImGui::SeparatorText("Package and Export Game");
		ImGui::NewLine();

		if (ImGui::BeginChild("Game Packager"))
		{
			ImGui::SeparatorText("File Information");
			ImGui::PushItemWidth(256.0f);
			ImGui::InlineLabel("Game Title");
			std::string projectName{ projectInfo->GetProjectName() };
			ImGui::InputTextReadOnly("##gameTitle", &projectName);

			static bool destinationError{ false };
			ImGui::InlineLabel("Destination");
			ImGui::InputTextReadOnly("##destination", &m_DestinationPath);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("...""##dest"))
			{
				FileDialog fd{};
				const auto filepath = fd.SelectFolderDialog("Choose Destination Folder", BASE_PATH);
				if (!filepath.empty())
				{
					if (!IsReservedPathOrFile(fs::path{ filepath }))
					{
						m_DestinationPath = filepath;
						destinationError = false;
					}
					else
					{
						F_ERROR("Failed to set destination. Destination '{}' is a reserved path. Please select a different path", filepath);
						destinationError = true;
					}
				}
				else
				{
					destinationError = false;
				}
			}

			if (destinationError)
			{
				ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, "Invalid Destination. Destinations cannot be reserved paths");
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
		}

		ImGui::End();
	}

}
