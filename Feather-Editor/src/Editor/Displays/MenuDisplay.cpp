#include "MenuDisplay.h"
#include "Logger/Logger.h"
#include "FileSystem/Dialogs/FileDialog.h"
#include "Core/Loaders/TilemapLoader.h"
#include "Core/CoreUtils/CoreEngineData.h"

#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"
#include "Editor/Tools/ToolManager.h"
#include "Editor/Utilities/ImGuiUtils.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"

#include <imgui.h>
#include <SDL.h>

namespace Feather {

	void MenuDisplay::Draw()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu(ICON_FA_FILE " File"))
			{
				ImGui::InlineLabel(ICON_FA_FILE_ALT, 32.0f);
				if (ImGui::MenuItem("New", "Ctrl + N"))
				{
					F_TRACE("New pressed");
				}

				ImGui::InlineLabel(ICON_FA_FOLDER_OPEN, 32.0f);
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

				ImGui::InlineLabel(ICON_FA_SAVE, 32.0f);
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

				ImGui::InlineLabel(ICON_FA_WINDOW_CLOSE, 32.0f);
				if (ImGui::MenuItem("Exit"))
				{
					F_TRACE("Exit pressed");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_EDIT " Edit"))
			{
				auto& coreGlobals = CORE_GLOBALS();

				static bool gridSnap{ true };
				if (ImGui::Checkbox("Enable Gridsnap", &gridSnap))
					SCENE_MANAGER().GetToolManager().EnableGridSnap(gridSnap);

				static bool showCollision{ false };
				if (ImGui::Checkbox("Show Collision", &showCollision))
				{
					if (showCollision)
						coreGlobals.EnableColliderRender();
					else
						coreGlobals.DisableColliderRender();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_TOOLS " Tools"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_COG " Settings"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_QUESTION_CIRCLE " Help"))
			{
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

}
