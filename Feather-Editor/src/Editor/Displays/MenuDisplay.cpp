#include "MenuDisplay.h"
#include "Logger/Logger.h"
#include "FileSystem/Dialogs/FileDialog.h"

#include <imgui.h>
#include <SDL.h>

namespace Feather {

	void MenuDisplay::Draw()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl + N"))
				{
					F_TRACE("New pressed");
				}

				if (ImGui::MenuItem("Open", "Ctrl + O"))
				{
					FileDialog fd{};
					auto file = fd.OpenFileDialog("Open tests", SDL_GetBasePath(), { "*.png", "*.jpg" });
					if (!file.empty())
					{
						F_TRACE("File opened: {}", file);
					}
				}

				if (ImGui::MenuItem("Save", "Ctrl + S"))
				{
					FileDialog fd{};
					auto file = fd.SaveFileDialog("Save Tilemap Test", SDL_GetBasePath(), { "*.json" });
					if (!file.empty())
					{
						// TODO: Save Tilemap
					}
				}

				if (ImGui::MenuItem("Exit"))
				{
					F_TRACE("Exit pressed");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Settings"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

}
