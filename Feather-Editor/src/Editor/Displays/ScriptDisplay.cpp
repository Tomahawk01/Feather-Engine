#include "ScriptDisplay.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtils/SaveProject.h"
#include "FileSystem/Serializers/LuaSerializer.h"
#include "Utils/HelperUtilities.h"
#include "Utils/FeatherUtilities.h"

#include <imgui.h>

#include <format>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace Feather {

	ScriptDisplay::ScriptDisplay()
		: m_ScriptsDirectory{ std::format("{}{}{}{}", MAIN_REGISTRY().GetContext<std::shared_ptr<SaveProject>>()->projectPath, "content", PATH_SEPARATOR, "scripts") }
		, m_Selected { -1 }
		, m_ScriptsChanged{ false }
	{
		F_ASSERT(fs::exists(fs::path{ m_ScriptsDirectory }) && "Scripts directory must exist");
		const std::string scriptListPath = m_ScriptsDirectory + PATH_SEPARATOR + "script_list.lua";

		if (!fs::exists(fs::path{ scriptListPath }))
		{
			std::ofstream file{ scriptListPath };
			file.close();

			if (!fs::exists(fs::path{ scriptListPath }))
			{
				F_ASSERT(false && "Failed to create script file");
				F_ERROR("Failed to create script list file at path: {}", m_ScriptsDirectory);
				return;
			}
		}

		GenerateScriptList();
	}

	ScriptDisplay::~ScriptDisplay() = default;

	void ScriptDisplay::Draw()
	{
		if (!ImGui::Begin("Script List"))
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
		// TODO: Handle directory changes
	}

	void ScriptDisplay::GenerateScriptList()
	{
		if (m_ScriptList.empty())
		{
			// TODO: Delete this test data!!!
			m_ScriptList.push_back("scripts\\character.lua");
			m_ScriptList.push_back("scripts\\states\\move_state.lua");
			m_ScriptList.push_back("scripts\\states\\damage_state.lua");

			//const std::string scriptListPath = m_ScriptsDirectory + PATH_SEPARATOR + "script_list.lua";
			//if (fs::exists(fs::path{ scriptListPath }))
			//{
			//	sol::state lua{};
			//	auto result = lua.safe_script_file(scriptListPath);
			//	if (!result.valid())
			//	{
			//		sol::error err = result;
			//		F_ERROR("Failed to load script list. {}", err.what());
			//		return;
			//	}

			//	sol::optional<sol::table> scriptList = lua["ScriptList"];
			//	if (!scriptList)
			//	{
			//		F_ERROR("Failed to load script list. Missing \"ScriptList\" table");
			//		return;
			//	}

			//	std::string path{ m_ScriptsDirectory.substr(0, m_ScriptsDirectory.find("scripts")) };
			//	for (const auto& [_, script] : *scriptList)
			//	{
			//		std::string newScript{ script.as<std::string>() };
			//		if (fs::exists(fs::path{ path + newScript }))
			//		{
			//			m_ScriptList.push_back(newScript);
			//		}
			//	}
			//}
		}
	}

	void ScriptDisplay::WriteScriptListToFile()
	{
		const std::string scriptListPath = m_ScriptsDirectory + PATH_SEPARATOR + "script_list.lua";
		if (!fs::exists(fs::path{ scriptListPath }))
		{
			F_ERROR("Failed to write script list. File '{}' does not exist", scriptListPath);
			return;
		}

		std::unique_ptr<LuaSerializer> serializer{ nullptr };
		try
		{
			serializer = std::make_unique<LuaSerializer>(scriptListPath);
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

}
