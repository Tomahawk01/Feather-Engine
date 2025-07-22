#include "LogDisplay.h"

#include "Logger/Logger.h"

#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"

#include <ranges>

#include <imgui_stdlib.h>

namespace Feather {

	LogDisplay::LogDisplay()
		: m_SearchQuery{}
		, m_AutoScroll{ true }
		, m_ShowTrace{ true }
		, m_ShowInfo{ true }
		, m_ShowWarn{ true }
		, m_ShowError{ true }
		, m_LogIndex{ 0 }
	{}

	void LogDisplay::Clear()
	{
		m_TextBuffer.clear();
		m_TextOffsets.clear();
		m_LogIndex = 0;
	}

	void LogDisplay::Draw()
	{
		if (!ImGui::Begin(ICON_FA_TERMINAL " Logs"))
		{
			ImGui::End();
			return;
		}

		GetLogs();

		// Logging Controls
		ImGui::Text("Search:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputText("##Search", &m_SearchQuery);
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::Checkbox(" TRACE", &m_ShowTrace);
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::Checkbox(" INFO", &m_ShowInfo);
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::Checkbox(" WARN", &m_ShowWarn);
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::Checkbox(" ERROR", &m_ShowError);
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::Checkbox(" Auto scroll", &m_AutoScroll);
		ImGui::SameLine(0.0f, 16.0f);
		if (ImGui::Button("Clear"))
		{
			Clear();
			F_CLEAR_LOGS();
		}
		ImGui::SameLine(0.0f, 16.0f);

		if (ImGui::Button("Copy"))
		{
			ImGui::LogToClipboard();
		}

		ImGui::Separator();

		if (ImGui::BeginChild("scrolling", ImVec2{ 0.0f, 0.0f }, false, ImGuiWindowFlags_HorizontalScrollbar))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });

			for (int i = 0; i < m_TextOffsets.Size; i++)
			{
				const char* line_start = m_TextBuffer.begin() + m_TextOffsets[i];
				const char* line_end = (i + 1 < m_TextOffsets.Size)
										? (m_TextBuffer.begin() + m_TextOffsets[i + 1] - 1)
										: m_TextBuffer.end();

				std::string_view text{ line_start, line_end };

				// Apply filters
				bool isTrace{ text.find("TRACE") != std::string_view::npos };
				bool isInfo{ text.find("INFO") != std::string_view::npos };
				bool isWarn{ text.find("WARN") != std::string_view::npos };
				bool isError{ text.find("ERROR") != std::string_view::npos };

				if ((isTrace && !m_ShowTrace) || (isInfo && !m_ShowInfo) || (isWarn && !m_ShowWarn) || (isError && !m_ShowError))
					continue;

				if (!m_SearchQuery.empty() && text.find(m_SearchQuery) == std::string_view::npos)
					continue;

				ImVec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
				if (isTrace)
					color = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };	// white
				else if (isInfo)
					color = ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f };	// green
				else if (isWarn)
					color = ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f };	// yellow
				else if (isError)
					color = ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f };	// red

				ImGui::PushStyleColor(ImGuiCol_Text, color);
				ImGui::TextUnformatted(line_start, line_end);
				ImGui::PopStyleColor();
			}
			
			ImGui::PopStyleVar();

			if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
		}

		ImGui::End();
	}

	void LogDisplay::GetLogs()
	{
		const auto& logs{ F_GET_LOGS() };
		if (logs.size() == m_LogIndex)
			return;

		for (size_t i = m_LogIndex; i < logs.size(); i++)
		{
			const auto& log = logs[i];
			int oldTextSize = m_TextBuffer.size();
			m_TextBuffer.append(log.log.c_str());
			m_TextBuffer.append("\n");
			m_TextOffsets.push_back(oldTextSize);
		}

		m_LogIndex = static_cast<int>(logs.size());
	}

}
