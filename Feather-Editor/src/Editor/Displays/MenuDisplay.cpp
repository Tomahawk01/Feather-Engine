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
				/*auto& toolManager = TOOL_MANAGER();
				ImGui::InlineLabel(ICON_FA_ARROWS_ALT "Translate");*/

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_VIDEO " Scene"))
			{
				if (auto pCurrentScene = SCENE_MANAGER().GetCurrentScene())
				{
					ImGui::Text("Current Scene");
					if (ImGui::TreeNode("Canvas"))
					{
						auto& canvas = pCurrentScene->GetCanvas();

						ImGui::InlineLabel("width");
						if (ImGui::InputInt("##_width", &canvas.width, canvas.tileWidth, canvas.tileWidth))
						{
							if (canvas.width < 640)
								canvas.width = 640;
						}
						ImGui::ItemToolTip("Canvas Width - Clamped minimum = 640");

						ImGui::InlineLabel("height");
						if (ImGui::InputInt("##_height", &canvas.height, canvas.tileHeight, canvas.tileHeight))
						{
							if (canvas.height < 320)
								canvas.height = 320;
						}
						ImGui::ItemToolTip("Canvas Height - Clamped minimum = 320");

						ImGui::InlineLabel("tile width");
						if (ImGui::InputInt("##tile_width", &canvas.tileWidth, 8, 8))
						{
							canvas.tileWidth = std::clamp(canvas.tileWidth, 8, 128);
						}
						ImGui::ItemToolTip("Tile Width - Range [8 : 128]");

						ImGui::InlineLabel("tile height");
						if (ImGui::InputInt("##tile_height", &canvas.tileHeight, 8, 8))
						{
							canvas.tileHeight = std::clamp(canvas.tileHeight, 8, 128);
						}
						ImGui::ItemToolTip("Tile Height - Range [8 : 128]");

						ImGui::TreePop();
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_COG " Settings"))
			{
				if (ImGui::TreeNode("Physics"))
				{
					ImGui::Separator();
					auto& coreGlobals = CORE_GLOBALS();
					bool enablePhysics = coreGlobals.IsPhysicsEnabled();

					ImGui::InlineLabel("Enable Physics", 176.0f);
					if (ImGui::Checkbox("##enable_physics", &enablePhysics))
					{
						if (enablePhysics)
							coreGlobals.EnablePhysics();
						else
							coreGlobals.DisablePhysics();
					}

					int32 velocityIterations = coreGlobals.GetVelocityIterations();
					int32 positionIterations = coreGlobals.GetPositionIterations();
					float gravity = coreGlobals.GetGravity();
					ImGui::InlineLabel("Gravity", 176.0f);
					if (ImGui::InputFloat("##Gravity", &gravity, 0.1f, 0.1f, "%.1f"))
					{
						coreGlobals.SetGravity(gravity);
					}

					ImGui::InlineLabel("Velocity Iterations", 176.0f);
					if (ImGui::InputInt("##VelocityIterations", &velocityIterations, 1, 1))
					{
						coreGlobals.SetVelocityIterations(velocityIterations);
					}

					ImGui::InlineLabel("Position Iterations", 176.0f);
					if (ImGui::InputInt("##PositionIterations", &positionIterations, 1, 1))
					{
						coreGlobals.SetPositionIterations(positionIterations);
					}
					ImGui::TreePop();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_QUESTION_CIRCLE " Help"))
			{
				if (ImGui::TreeNode("About Feather Engine"))
				{
					ImGui::Separator();
					ImGui::Text("Feather is a 2D game engine written in C++");
					ImGui::Text("By Alex (Tomahawk)");
					ImGui::AddSpaces(2);
					ImGui::Text("Helpful Links: ");
					ImGui::Separator();
					ImGui::TreePop();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

}
