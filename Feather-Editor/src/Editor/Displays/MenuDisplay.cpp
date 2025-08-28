#include "MenuDisplay.h"

#include "Logger/Logger.h"
#include "FileSystem/Dialogs/FileDialog.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/Prefab.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Events/EventDispatcher.h"
#include "Utils/FeatherUtilities.h"

#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"
#include "Editor/Tools/ToolManager.h"
#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Utilities/EditorState.h"
#include "Editor/Loaders/ProjectLoader.h"
#include "Editor/Events/EditorEventTypes.h"
#include "Editor/Loaders/TiledMapImporter.h"

#include <imgui.h>
#include <SDL.h>

namespace Feather {

	void MenuDisplay::Draw()
	{
		auto& sceneManager = SCENE_MANAGER();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu(ICON_FA_FILE " File"))
			{
				ImGui::SeparatorText("Project");
				ImGui::InlineLabel(ICON_FA_FILE_ALT, 32.0f);
				if (ImGui::MenuItem("New", "Ctrl + N"))
				{
					F_ERROR("New not implemented yet!");
				}

				ImGui::InlineLabel(ICON_FA_FOLDER_OPEN, 32.0f);
				if (ImGui::MenuItem("Open", "Ctrl + O"))
				{
					F_ERROR("Open not implemented yet!");
				}

				ImGui::InlineLabel(ICON_FA_SAVE, 32.0f);
				if (ImGui::MenuItem("Save All", "Ctrl + S"))
				{
					auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
					F_ASSERT(projectInfo && "Project Info must exist!");

					// Save entire project
					ProjectLoader pl{};
					if (!pl.SaveLoadedProject(*projectInfo))
					{
						auto optProjectFilePath = projectInfo->GetProjectFilePath();
						F_ASSERT(optProjectFilePath && "Project file path not setup correctly");
						F_ERROR("Failed to save project '{}' at file '{}'", projectInfo->GetProjectName(), optProjectFilePath->string());
					}
				}
				ImGui::ItemToolTip("Saves entire project to disk");

				ImGui::SeparatorText("Scenes");

				if (auto currentScene = sceneManager.GetCurrentScene())
				{
					ImGui::InlineLabel(ICON_FA_SAVE, 32.0f);
					if (ImGui::MenuItem("Save Current Scene As..."))
					{
						// TODO: Save a copy of the scene and all of it's entities under a new name
					}
				}

				ImGui::SeparatorText("Import");
				ImGui::InlineLabel(ICON_FA_FILE_IMPORT, 32.0f);
				if (ImGui::MenuItem("Import Tiled Map"))
				{
					FileDialog fd{};
					const auto filepath = fd.OpenFileDialog("Import Tiled Map", BASE_PATH, { "*.lua" });
					if (!filepath.empty())
					{
						if (!TiledMapImporter::ImportTilemapFromTiled(&SCENE_MANAGER(), filepath))
						{
							F_ERROR("Failed to import tiled map as new scene");
						}
					}
				}

				ImGui::ItemToolTip("Import a map from the Tiled Map Editor. **Must be a lua export**");

				ImGui::SeparatorText("Exit");
				ImGui::InlineLabel(ICON_FA_WINDOW_CLOSE, 32.0f);
				if (ImGui::MenuItem("Exit"))
				{
					EVENT_DISPATCHER().EmitEvent(CloseEditorEvent{});
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_EDIT " Edit"))
			{
				ImGui::SeparatorText("Configuration");

				auto& coreGlobals = CORE_GLOBALS();
				auto& toolManager = TOOL_MANAGER();

				bool gridSnap{ toolManager.IsGridSnapEnabled() };
				if (ImGui::Checkbox("Enable Gridsnap", &gridSnap))
					toolManager.EnableGridSnap(gridSnap);

				bool showCollision{ coreGlobals.RenderCollidersEnabled() };
				if (ImGui::Checkbox("Show Collision", &showCollision))
				{
					showCollision ? coreGlobals.EnableColliderRender() : coreGlobals.DisableColliderRender();
				}

				bool showAnimations{ coreGlobals.AnimationRenderEnabled() };
				if (ImGui::Checkbox("Show Animations", &showAnimations))
				{
					showAnimations ? coreGlobals.EnableAnimationRender() : coreGlobals.DisableAnimationRender();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_WINDOW_MAXIMIZE " Displays"))
			{
				ImGui::SeparatorText("Displays");

				auto& editorState = MAIN_REGISTRY().GetContext<EditorStatePtr>();
				DrawDisplayItem(*editorState, ICON_FA_FILE_ALT " Asset Browser", EDisplay::AssetBrowser);
				DrawDisplayItem(*editorState, ICON_FA_FOLDER " Content Browser", EDisplay::ContentBrowser);
				DrawDisplayItem(*editorState, ICON_FA_CODE " Script List", EDisplay::ScriptListView);
				DrawDisplayItem(*editorState, ICON_FA_ARCHIVE " Package Game", EDisplay::PackagerView);
				DrawDisplayItem(*editorState, ICON_FA_TERMINAL " Console Logger", EDisplay::Console);
				DrawDisplayItem(*editorState, ICON_FA_COG " Project Settings", EDisplay::GameSettingsView);

				ImGui::Separator();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FA_VIDEO " Scene"))
			{
				if (auto pCurrentScene = SCENE_MANAGER().GetCurrentScene())
				{
					ImGui::Text("Current Scene");
					ImGui::Separator();
					if (ImGui::TreeNode(ICON_FA_FILE_IMAGE " Canvas"))
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
					ImGui::Separator();
					if (ImGui::TreeNode(ICON_FA_COG " Settings"))
					{
						bool isChanged{ false };
						bool isPlayerStartEnabled{ pCurrentScene->IsPlayerStartEnabled() };
						ImGui::InlineLabel("Enable Player Start:");
						ImGui::SetCursorPosX(250.0f);

						ImGui::ItemToolTip("Enable or Disable the player start.\n"
										   "The player start is the character that we want to use when the scene is played.");

						if (ImGui::Checkbox("##_enablePlayerStart", &isPlayerStartEnabled))
						{
							pCurrentScene->SetPlayerStartEnabled(isPlayerStartEnabled);
							if (isPlayerStartEnabled)
							{
								pCurrentScene->GetPlayerStart().LoadVisualEntity();
							}
							else
							{
								pCurrentScene->GetPlayerStart().Unload();
							}
						}

						std::string sPlayerStartCharacter{ pCurrentScene->GetPlayerStart().GetCharacterName() };
						auto prefabs = GetKeys(ASSET_MANAGER().GetAllPrefabs()/*, [](auto& prefab) {
							return prefab.second->GetType() == EPrefabType::Character;
						} */);

						ImGui::InlineLabel(ICON_FA_FLAG ICON_FA_GAMEPAD " Player Start Character:");
						ImGui::SetCursorPosX(250.0f);
						ImGui::ItemToolTip("The default player to spawn when starting the scene");
						if (ImGui::BeginCombo("##DefaultPlayerStart", sPlayerStartCharacter.c_str()))
						{
							for (const auto& sPrefabName : prefabs)
							{
								if (ImGui::Selectable(sPrefabName.c_str(), sPrefabName == sPlayerStartCharacter))
								{
									sPlayerStartCharacter = sPrefabName;

									isChanged = true;
								}
							}

							ImGui::EndCombo();
						}

						if (isChanged)
						{
							if (auto pPrefab = ASSET_MANAGER().GetPrefab(sPlayerStartCharacter))
							{
								pCurrentScene->GetPlayerStart().SetCharacter(*pPrefab);
							}
						}

						isChanged = true;

						auto musicNames = GetKeys(ASSET_MANAGER().GetAllMusic());
						musicNames.push_back("None");

						std::string sDefaultSceneMusic{ pCurrentScene->GetDefaultMusicName() };
						if (sDefaultSceneMusic.empty())
							sDefaultSceneMusic = "None";

						ImGui::InlineLabel(ICON_FA_MUSIC " Default Music:");
						ImGui::SetCursorPosX(250.0f);
						ImGui::ItemToolTip("Music to play when the scene starts");
						if (ImGui::BeginCombo("##DefaultMusic", sDefaultSceneMusic.c_str()))
						{
							for (const auto& sMusicName : musicNames)
							{
								if (ImGui::Selectable(sMusicName.c_str(), sMusicName == sDefaultSceneMusic))
								{
									sDefaultSceneMusic = sMusicName;
									isChanged = true;
								}
							}

							ImGui::EndCombo();
						}

						if (isChanged)
						{
							pCurrentScene->SetDefaultMusic(sDefaultSceneMusic);
						}

						ImGui::TreePop();
					}
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
					ImGui::SeparatorText("Helpful Links: ");
					ImGui::TextLinkOpenURL("Github", "https://github.com/Tomahawk01/Feather-Engine");
					ImGui::Separator();
					ImGui::TreePop();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void MenuDisplay::DrawDisplayItem(EditorState& editorState, const std::string& displayName, const EDisplay display)
	{
		bool displayEnabled{ editorState.IsDisplayOpen(display) };
		if (ImGui::Selectable(displayName.c_str(), false, ImGuiSelectableFlags_DontClosePopups))
		{
			displayEnabled = !displayEnabled;
			editorState.SetDisplay(display, displayEnabled);
		}

		if (displayEnabled)
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(150.0f);
			ImGui::Text(ICON_FA_CHECK);
		}
	}

}
