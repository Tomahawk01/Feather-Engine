#pragma once

#include <string>
#include <format>

#include <imgui.h>

constexpr ImVec4 LABEL_RED = ImVec4{ 0.92f, 0.18f, 0.05f, 1.0f };
constexpr ImVec4 LABEL_GREEN = ImVec4{ 0.05f, 0.93f, 0.25f, 1.0f };
constexpr ImVec4 LABEL_BLUE = ImVec4{ 0.05f, 0.18f, 0.91f, 1.0f };

constexpr ImVec4 BUTTON_HELD = ImVec4{ 0.0f, 0.9f, 0.0f, 0.3f };

constexpr ImVec4 BLACK_TRANSPARENT = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };

constexpr ImVec2 LABEL_SINGLE_SIZE = ImVec2{ 20.0f, 20.0f };
constexpr ImVec2 TOOL_BUTTON_SIZE = ImVec2{ 32.0f, 32.0f };

constexpr ImGuiColorEditFlags IMGUI_COLOR_PICKER_FLAGS = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB;
constexpr ImGuiTableFlags IMGUI_NORMAL_TABLE_FLAGS = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;

namespace ImGui {

	void InitDefaultStyles();

	void ColoredLabel(const std::string& label, const ImVec2& size = ImVec2{ 32.0f, 32.0f }, const ImVec4& color = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f });
	void OffsetTextX(const std::string& label, float position);
	void AddSpaces(int numSpaces);
	void InlineLabel(const std::string& label, float spaceSize = 128.0f);

	void ActiveButton(const char* label, ImVec2 size = TOOL_BUTTON_SIZE);
	void DisabledButton(const char* label, ImVec2 size = TOOL_BUTTON_SIZE, const std::string& disabledMsg = "");

	void ActiveImageButton(const char* buttonId, ImTextureID textureID, ImVec2 size = TOOL_BUTTON_SIZE);
	void DisabledImageButton(const char* buttonId, ImTextureID textureID, ImVec2 size = TOOL_BUTTON_SIZE, const std::string& disabledMsg = "");

	void TextLinkOpenURL(const char* label, const char* url);

	template <typename... Args>
	void ItemToolTip(const std::string_view tipMsg, Args&&... args)
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(std::vformat(tipMsg, std::make_format_args(args...)).c_str());
			ImGui::EndTooltip();
		}
	}
	template <typename... Args>
	void TextFormatted(const std::string_view text, Args&&... args)
	{
		ImGui::TextUnformatted(std::vformat(text, std::make_format_args(args...)).c_str());
	}

}
