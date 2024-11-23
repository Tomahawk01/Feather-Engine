#include "ImGuiUtils.h"
#include "Logger/Logger.h"

namespace ImGui {

	void InitDefaultStyles()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.TabRounding = 4.0f;
		style.TabBorderSize = 1.0f;
		style.FrameBorderSize = 0.01f;
		style.FramePadding.x = 4.0f;
		style.FramePadding.y = 3.0f;
		style.FrameRounding = 2.0f;
		style.ScrollbarSize = 10.0f;
		style.ScrollbarRounding = 6.0f;
		style.WindowBorderSize = 1.0f;
		style.WindowPadding.x = 10.0f;
		style.WindowPadding.y = 10.0f;
		style.WindowRounding = 8.0f;
		style.WindowTitleAlign.x = 0.5f;
		style.ItemSpacing.x = 3.0f;
		style.ItemInnerSpacing.x = 0.0f;
		style.ItemInnerSpacing.y = 4.0f;
		style.ButtonTextAlign.x = 0.5f;
		style.ButtonTextAlign.y = 0.5f;

		// TODO: Adjust colors
	}

	void ColoredLabel(const std::string& label, const ImVec2& size, const ImVec4& color)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
		ImGui::Button(label.c_str(), size);
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	void OffsetTextX(const std::string& label, float position)
	{
		ImGui::SetCursorPosX(position);
		ImGui::Text(label.c_str());
	}

	void AddSpaces(int numSpaces)
	{
		F_ASSERT(numSpaces > 0 && "Number of spaces must be a positive number!");
		for (int i = 0; i < numSpaces; ++i)
			ImGui::Spacing();
	}

	void InlineLabel(const std::string& label, float spaceSize)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine();
		ImGui::SetCursorPosX(spaceSize);
	}

}
