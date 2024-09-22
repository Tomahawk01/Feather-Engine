#include "MenuDisplay.h"
#include "Logger/Logger.h"
#include "FileSystem/Dialogs/FileDialog.h"
#include "Core/Loaders/TilemapLoader.h"
#include "../Scene/SceneManager.h"
#include "../Scene/SceneObject.h"

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
					auto file = fd.OpenFileDialog("Open tilemap test", SDL_GetBasePath(), { "*.json" });
					if (!file.empty())
					{
						auto currentScene = SCENE_MANAGER().GetCurrentScene();
						if (currentScene)
						{
							TilemapLoader tl{};
							if (!tl.LoadTilemap(currentScene->GetRegistry(), file, true))
								F_ERROR("Failed to load tilemap");
						}
						else
						{
							F_ERROR("Failed to load tilemap. No active scene");
						}
					}
				}

				if (ImGui::MenuItem("Save", "Ctrl + S"))
				{
					FileDialog fd{};
					auto file = fd.SaveFileDialog("Save Tilemap Test", SDL_GetBasePath(), { "*.json" });
					if (!file.empty())
					{
						auto currentScene = SCENE_MANAGER().GetCurrentScene();
						if (currentScene)
						{
							TilemapLoader tl{};
							if (!tl.SaveTilemap(currentScene->GetRegistry(), file, true))
								F_ERROR("Failed to save tilemap");
						}
						else
						{
							F_ERROR("Failed to save tilemap. No active scene");
						}
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
