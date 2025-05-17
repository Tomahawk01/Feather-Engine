#include "ImGuiUtils.h"

#include "Logger/Logger.h"

#include <Windows.h>
#include <shellapi.h>

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

	void ActiveButton(const char* label, ImVec2 size)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_HELD);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BUTTON_HELD);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, BUTTON_HELD);
		ImGui::Button(label, size);
		ImGui::PopStyleColor(3);
	}

	void DisabledButton(const char* label, ImVec2 size, const std::string& disabledMsg)
	{
		ImGui::BeginDisabled();
		ImGui::Button(label, size);

		if (!disabledMsg.empty())
			ImGui::SetItemTooltip(disabledMsg.c_str());

		ImGui::EndDisabled();
	}

	void ActiveImageButton(const char* buttonId, ImTextureID textureID, ImVec2 size)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_HELD);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BUTTON_HELD);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, BUTTON_HELD);
		ImGui::ImageButton(buttonId, textureID, size);
		ImGui::PopStyleColor(3);
	}

	void DisabledImageButton(const char* buttonId, ImTextureID textureID, ImVec2 size, const std::string& disabledMsg)
	{
		ImGui::BeginDisabled();
		ImGui::ImageButton(buttonId, textureID, size);

		if (!disabledMsg.empty())
			ImGui::SetItemTooltip(disabledMsg.c_str());
		
		ImGui::EndDisabled();
	}

	void TextLinkOpenURL(const char* label, const char* url)
	{
		ImVec2 startPos = ImGui::GetCursorPos();
		ImVec2 textSize = ImGui::CalcTextSize(label);

		ImGui::InvisibleButton(label, textSize);

		bool hovered = ImGui::IsItemHovered();
		bool clicked = ImGui::IsItemClicked();

		ImGui::SetCursorPos(startPos);

		ImVec4 textColor = hovered ?
			ImVec4(0.4f, 0.7f, 1.0f, 1.0f) :  // Light blue when hovered
			ImVec4(0.2f, 0.5f, 0.9f, 1.0f);   // Blue when not hovered

		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(textColor));
		ImGui::Text(label);
		ImGui::PopStyleColor();

		// Change cursor to hand when hovering
		if (hovered)
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

		// Open URL when clicked
		if (clicked)
		{
			ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
		}
	}

}
