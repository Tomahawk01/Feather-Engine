#include "PackageDisplay.h"

#include "Logger/Logger.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/ECS/MainRegistry.h"
#include "FileSystem/Dialogs/FileDialog.h"
#include "Utils/ThreadPool.h"
#include "Utils/HelperUtilities.h"

#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Loaders/ProjectLoader.h"
#include "Editor/Packaging/Packager.h"
#include "Editor/Scene/SceneManager.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace fs = std::filesystem;

namespace Feather {

	PackageGameDisplay::PackageGameDisplay()
		: m_GameConfig{ std::make_unique<GameConfig>() }
		, m_Packager{ nullptr }
		, m_DestinationPath{}
		, m_ScriptListPath{}
		, m_FileIconPath{}
		, m_Resizable{ false }
		, m_Borderless{ false }
		, m_FullScreen{ false }
		, m_Titlebar{ false }
		, m_ScriptListExist{ false }
		, m_PackageHasErrors{ false }
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
		if (!m_Packager)
			return;

		if (m_Packager->Completed())
		{
			m_Packager.reset(nullptr);
			return;
		}

		if (m_Packager->HasError())
		{
			m_PackageHasErrors = true;
			m_Packager.reset(nullptr);
		}
	}

	void PackageGameDisplay::Draw()
	{
		if (!ImGui::Begin(ICON_FA_ARCHIVE " Package Game"))
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

			ImGui::InlineLabel("Startup Scene");
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

		if (CanPackageGame())
		{
			if (m_Packager && !m_Packager->Completed())
			{
				ImGui::LoadingSpinner("##packaging", 10.0f, 3.0f, IM_COL32(32, 175, 32, 255));
				ImGui::SameLine(0.0f, 16.0f);
				const auto packageProgress = m_Packager->GetProgress();
				if (auto pFont = ImGui::GetFont("roboto-bold-24"))
				{
					ImGui::PushFont(pFont);
					ImGui::TextColored(
						m_PackageHasErrors ? ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f } : ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f },
						std::format("{}%% - {}", packageProgress.percent, packageProgress.message).c_str());
					ImGui::PopFont();
				}
			}
			else if (ImGui::Button("Package Game"))
			{
				// We want to ensure we are packaging the most current data.
				// Save all files, before packaging
				auto& pProjectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
				F_ASSERT(pProjectInfo && "Project Info must exist!");
				// Save entire project
				ProjectLoader pl{};
				if (!pl.SaveLoadedProject(*pProjectInfo))
				{
					auto optProjectFilePath = pProjectInfo->GetProjectFilePath();
					F_ASSERT(optProjectFilePath && "Project file path not set correctly in project info");

					F_ERROR("Failed to save project '{}' at file '{}'", pProjectInfo->GetProjectName(), optProjectFilePath->string());

					return;
				}

				const auto& coreGlobals = CORE_GLOBALS();

				// Set Physics data
				m_GameConfig->physicsEnabled = coreGlobals.IsPhysicsEnabled();
				m_GameConfig->positionIterations = coreGlobals.GetPositionIterations();
				m_GameConfig->velocityIterations = coreGlobals.GetVelocityIterations();
				m_GameConfig->gravity = coreGlobals.GetGravity();
				m_GameConfig->gameName = pProjectInfo->GetProjectName();

				// Set window flags
				uint32_t flags{ 0 };

				if (m_Borderless && !m_Titlebar)
					flags |= SDL_WINDOW_BORDERLESS;
				if (m_FullScreen)
					flags |= SDL_WINDOW_FULLSCREEN;
				if (m_Resizable)
					flags |= SDL_WINDOW_RESIZABLE;

				m_GameConfig->windowFlags = flags;

				std::string sFullDestination = std::format("{}{}{}", m_DestinationPath, PATH_SEPARATOR, m_GameConfig->gameName);

				auto pPackageData = std::make_unique<PackageData>();
				pPackageData->ProjectInfo = std::make_unique<ProjectInfo>(*pProjectInfo);
				pPackageData->GameConfig = std::make_unique<GameConfig>(*m_GameConfig);
				pPackageData->TempDataPath = fs::path{ pProjectInfo->GetProjectPath() / "tempData" }.string();
				pPackageData->FinalDestination = sFullDestination;
				pPackageData->AssetFilepath = pPackageData->TempDataPath + PATH_SEPARATOR + "assetDefs.lua";

				auto& pThreadPool = MAIN_REGISTRY().GetContext<SharedThreadPool>();
				F_ASSERT(pThreadPool && "Thread pool must exist and be valid");

				m_Packager = std::make_unique<Packager>(std::move(pPackageData), pThreadPool);

				ImGui::End();

				return;
			}
		}
		else if (!m_ScriptListExist)
		{
			ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, "Unable to package game. Script List does not exist");
		}

		ImGui::End();
	}

	bool PackageGameDisplay::CanPackageGame() const
	{
		return m_ScriptListExist && !m_DestinationPath.empty() && !m_GameConfig->startupScene.empty() &&
			fs::exists(fs::path{ m_DestinationPath }) && !m_DestinationPath.empty();
	}

}
