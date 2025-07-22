#include "ScriptDisplay.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "FileSystem/Serializers/LuaSerializer.h"
#include "FileSystem/Utilities/DirectoryWatcher.h"
#include "Utils/HelperUtilities.h"
#include "Utils/FeatherUtilities.h"

#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"

#include <imgui.h>

#include <format>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace Feather {

	ScriptDisplay::ScriptDisplay()
		: m_ScriptsDirectory{}
		, m_Selected { -1 }
		, m_ScriptsChanged{ false }
		, m_ListScripts{ false }
		, m_DirWatcher{ nullptr }
		, m_FilesChanged{ false }
	{
		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		auto optScriptsPath = projectInfo->TryGetFolderPath(EProjectFolderType::Scripts);
		F_ASSERT(optScriptsPath && "Scipts Directory was not set correctly");
		F_ASSERT(fs::exists(*optScriptsPath) && "Scripts directory must exist");

		m_ScriptsDirectory = optScriptsPath->string();

		auto optScriptListPath = projectInfo->GetScriptListPath();
		F_ASSERT(optScriptListPath && "Script list path not set correctly in project info");

		if (!fs::exists(*optScriptListPath))
		{
			std::ofstream file{ optScriptListPath->string() };
			file.close();

			if (!fs::exists(*optScriptListPath))
			{
				F_ASSERT(false && "Failed to create script file");
				F_ERROR("Failed to create script list file at path: '{}'", optScriptListPath->parent_path().string());
				return;
			}
		}

		GenerateScriptList();

		m_DirWatcher = std::make_unique<DirectoryWatcher>(
			fs::path{ m_ScriptsDirectory },
			[this](const fs::path& file, bool modified) { OnFileChanged(file, modified); }
		);
	}

	ScriptDisplay::~ScriptDisplay() = default;

	void ScriptDisplay::Draw()
	{
		if (!ImGui::Begin(ICON_FA_CODE " Script List"))
		{
			ImGui::End();
			return;
		}

		ImGui::SeparatorText("Script List");

		ImGui::Text("Drag to reorder scripts:");
		ImGui::Separator();

		ImGui::Columns(2, nullptr, false);

		ImGui::BeginChild("ListBox", ImVec2{ 0.0f, 300.0f }, true);
		for (int i = 0; i < static_cast<int>(m_ScriptList.size()); i++)
		{
			const auto& script = m_ScriptList[i];
			if (ImGui::Selectable(script.c_str(), m_Selected == i))
			{
				m_Selected = i;
			}
			
			if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
			{
				int iNext = i + (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
				if (iNext >= 0 && iNext < m_ScriptList.size())
				{
					std::swap(m_ScriptList.at(i), m_ScriptList.at(iNext));
					ImGui::ResetMouseDragDelta();
					m_ScriptsChanged = true;
				}
			}
		}
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("Controls", ImVec2{ 0.0f, 300.0f }, false, ImGuiWindowFlags_NoBackground);
		float buttonHeight = ImGui::GetFrameHeight();
		float totalHeight = buttonHeight * 2 + ImGui::GetStyle().ItemSpacing.y;

		float available = ImGui::GetContentRegionAvail().y;
		float padding = (available - totalHeight) * 0.5f;

		if (padding > 0.0f)
		{
			ImGui::Dummy(ImVec2{ 0.0f, padding });
		}

		if (ImGui::ArrowButton("##Up", ImGuiDir_Up))
		{
			if (m_Selected >= 0 && !m_ScriptList.empty())
			{
				int newIndex = (m_Selected - 1 + m_ScriptList.size()) % m_ScriptList.size();
				std::swap(m_ScriptList[m_Selected], m_ScriptList[newIndex]);
				m_Selected = newIndex;
				m_ScriptsChanged = true;
			}
		}

		if (ImGui::ArrowButton("##Down", ImGuiDir_Down))
		{
			if (m_Selected >= 0 && !m_ScriptList.empty())
			{
				int newIndex = (m_Selected + 1) % m_ScriptList.size();
				std::swap(m_ScriptList[m_Selected], m_ScriptList[newIndex]);
				m_Selected = newIndex;
				m_ScriptsChanged = true;
			}
		}
		ImGui::EndChild();

		ImGui::Columns(1);

		if (m_ScriptsChanged)
		{
			if (ImGui::Button("Regenerate Script List"))
			{
				WriteScriptListToFile();
			}
		}

		ImGui::End();
	}

	void ScriptDisplay::Update()
	{
		if (m_FilesChanged.exchange(false, std::memory_order_acquire))
		{
			m_ListScripts = true;
			F_TRACE("File was changed or added to scripts directory");
		}

		if (m_ListScripts)
		{
			m_Scripts.clear();
			for (const auto& dirEntry : fs::recursive_directory_iterator(fs::path{ m_ScriptsDirectory }))
			{
				if (fs::is_directory(dirEntry) ||
					dirEntry.path().extension() != ".lua" ||
					dirEntry.path().filename().string() == "main.lua" ||
					dirEntry.path().filename().string() == "script_list.lua")
				{
					continue;
				}

				auto foundScript = GET_SUBSTRING(dirEntry.path().relative_path().string(), "scripts");
				if (!foundScript.empty())
				{
					m_Scripts.push_back(std::string{ foundScript });
				}
			}

			std::unordered_set<std::string> lookupSet{ m_Scripts.begin(), m_Scripts.end() };

			auto removeRange = 
				std::ranges::remove_if(m_ScriptList,
				[&lookupSet](const std::string& item)
				{
					return !lookupSet.contains(item);
				});

			if (removeRange.begin() != removeRange.end())
			{
				m_ScriptList.erase(removeRange.begin(), removeRange.end());
				m_ScriptsChanged = true;
			}

			for (const auto& script : m_Scripts)
			{
				if (std::ranges::find(m_ScriptList, script) == m_ScriptList.end())
				{
					m_ScriptList.push_back(script);
					m_ScriptsChanged = true;
				}
			}

			m_ListScripts = false;
		}
	}

	void ScriptDisplay::GenerateScriptList()
	{
		if (m_ScriptList.empty())
		{
			auto optScriptListPath = MAIN_REGISTRY().GetContext<ProjectInfoPtr>()->GetScriptListPath();
			F_ASSERT(optScriptListPath && "ScriptList Path not set correctly in project info");

			if (!optScriptListPath)
			{
				F_ERROR("Failed to load script list. Not set correctly in project info");
				return;
			}
			if (fs::exists(*optScriptListPath))
			{
				sol::state lua{};
				auto result = lua.safe_script_file(optScriptListPath->string());
				if (!result.valid())
				{
					sol::error err = result;
					F_ERROR("Failed to load script list. {}", err.what());
					return;
				}

				sol::optional<sol::table> scriptList = lua["ScriptList"];
				if (!scriptList)
				{
					F_ERROR("Failed to load script list. Missing \"ScriptList\" table");
					return;
				}

				std::string path{ m_ScriptsDirectory.substr(0, m_ScriptsDirectory.find("scripts")) };
				for (const auto& [_, script] : *scriptList)
				{
					std::string newScript{ script.as<std::string>() };
					if (fs::exists(fs::path{ path + newScript }))
					{
						m_ScriptList.push_back(newScript);
					}
				}
			}
		}
	}

	void ScriptDisplay::WriteScriptListToFile()
	{
		auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
		F_ASSERT(projectInfo && "Project info must exist");
		auto optScriptListPath = projectInfo->GetScriptListPath();
		F_ASSERT(optScriptListPath && "Script list path not setup correctly in project info");

		if (!fs::exists(*optScriptListPath))
		{
			F_ERROR("Failed to write script list. File '{}' does not exist", optScriptListPath->string());
			return;
		}

		std::unique_ptr<LuaSerializer> serializer{ nullptr };
		try
		{
			serializer = std::make_unique<LuaSerializer>(optScriptListPath->string());
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to write script list: {}", ex.what());
			return;
		}

		serializer->AddComment("Script List File")
			.AddComment("WARNING - THIS FILE IS EDITOR GENERATED!")
			.AddComment("DO NOT CHANGE UNLESS YOU KNOW WHAT YOU ARE DOING.")
			.AddComment("\n");

		serializer->StartNewTable("ScriptList");
		for (const auto& script : m_ScriptList)
		{
			serializer->AddValue(script, true, false, false, true);
		}

		serializer->EndTable().FinishStream();
		m_ScriptsChanged = false;
	}

	void ScriptDisplay::OnFileChanged(const std::filesystem::path& path, bool modified)
	{
		m_FilesChanged.store(true, std::memory_order_relaxed);
	}

}
