#include "ProjectSettingsDisplay.h"

#include "Logger/Logger.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Core/ECS/MainRegistry.h"
#include "Physics/PhysicsUtilities.h"
#include "FileSystem/Dialogs/FileDialog.h"
#include "Renderer/Essentials/Texture.h"

#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/EditorState.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Scene/SceneManager.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace Feather {

	ProjectSettingsDisplay::ProjectSettingsDisplay()
		: m_SettingsCategory{}
		, m_SelectedCategory{ "General" }
	{
		CreateProjectSettings();
	}

	ProjectSettingsDisplay::~ProjectSettingsDisplay() = default;

	void ProjectSettingsDisplay::Update()
	{
	}

	void ProjectSettingsDisplay::Draw()
	{
		if (auto& editorState = MAIN_REGISTRY().GetContext<EditorStatePtr>())
		{
			if (!editorState->IsDisplayOpen(EDisplay::GameSettingsView))
			{
				return;
			}
		}

		ImGui::Begin(ICON_FA_COG " Project Settings");
		const float leftWidth = 250.0f;

		ImGui::Columns(2, nullptr, true);
		ImGui::SetColumnWidth(0, leftWidth);

		// Create two child regions side-by-side
		ImGui::BeginChild("SettingsNav", ImVec2(leftWidth, 0), true); // Left: category tree
		DrawCategoryTree(m_SettingsCategory, m_SelectedCategory);
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("SettingsPanel", ImGui::GetContentRegionAvail(), true); // Right: settings panel
		DrawSettingsPanel(m_SettingsCategory, m_SelectedCategory);
		ImGui::EndChild();

		ImGui::Columns(1);

		ImGui::End();
	}

	void ProjectSettingsDisplay::DrawCategoryTree(const SettingCategory& category, std::string& selected)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

		if (!category.subCategories.empty())
		{
			bool bOpen = ImGui::TreeNodeEx(category.name.c_str(), flags);
			if (ImGui::IsItemClicked())
			{
				selected = category.name;
			}

			if (bOpen)
			{
				for (const auto& sub : category.subCategories)
				{
					DrawCategoryTree(sub, selected);
				}
				ImGui::TreePop();
			}
		}
		else
		{
			if (ImGui::Selectable(category.name.c_str(), selected == category.name))
			{
				selected = category.name;
			}
		}
	}

	void ProjectSettingsDisplay::DrawSettingsPanel(const SettingCategory& category, std::string& selected)
	{
		if (category.name == selected)
		{
			for (const auto& item : category.items)
			{
				item.drawFunc();
				ImGui::Separator();
			}
		}

		for (const auto& sub : category.subCategories)
		{
			DrawSettingsPanel(sub, selected);
		}
	}

	auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size()))
		{
			return false;
		}
		*out_text = vector[idx].c_str();
		return true;
	};

	void ProjectSettingsDisplay::CreateProjectSettings()
	{
		auto& coreGlobals = CORE_GLOBALS();
		auto& mainRegistry = MAIN_REGISTRY();
		auto& projectInfo = mainRegistry.GetContext<ProjectInfoPtr>();
		F_ASSERT(projectInfo && "Project Info was not setup correctly.");

		SettingCategory projectSettings{ .name = "Project" };
		projectSettings.subCategories.emplace_back(CreateGeneralSettings(coreGlobals, *projectInfo, mainRegistry));
		projectSettings.subCategories.emplace_back(CreatePhysicsSettings(coreGlobals, *projectInfo, mainRegistry));
		projectSettings.subCategories.emplace_back(CreateGraphicsSettings(coreGlobals, *projectInfo, mainRegistry));
		projectSettings.subCategories.emplace_back(CreateAudioSettings(coreGlobals, *projectInfo, mainRegistry));

		m_SettingsCategory = projectSettings;
	}

	ProjectSettingsDisplay::SettingCategory ProjectSettingsDisplay::CreateGeneralSettings(CoreEngineData& coreGlobals, ProjectInfo& projectInfo, MainRegistry& mainRegistry)
	{
		return SettingCategory{
			.name = "General",
			.items = {
				{
					"Project Name", [&]() {
						std::string gameName = projectInfo.GetProjectName();
						ImGui::InlineLabel("Project Name");
						if (ImGui::InputText("##project_name", &gameName))
						{
							projectInfo.SetProjectName(gameName);
						}
					}
				},
				{
					"Version", [&]() {
						std::string version{ projectInfo.GetProjectVersion() };
						ImGui::InlineLabel("Version");
						if (ImGui::InputText("##version", &version))
						{
							projectInfo.SetProjectVersion(version);
						}
					}
				},
				{
					"Copyright Notice", [&]() {
						std::string copyright{ projectInfo.GetCopyRightNotice() };
						ImGui::InlineLabel("Copyright Notice");
						if (ImGui::InputText("##copyright_notice", &copyright))
						{
							projectInfo.SetCopyRightNotice(copyright);
						}
					}
				},
				{
					"Description", [&]() {
						std::string description{ projectInfo.GetProjectDescription() };
						ImGui::InlineLabel("Description");
						if (ImGui::InputText("##description", &description))
						{
							projectInfo.SetProjectDescription(description);
						}
					}
				},
				{
					"Icon", [&]() {
						static bool hasError{ false };
						static std::string iconPath{ projectInfo.GetFileIconPath() ? projectInfo.GetFileIconPath()->string() : "" };
						ImGui::InlineLabel("Icon:");
						ImGui::PushItemWidth(256.0f);
						ImGui::InputText("##icon", &iconPath, ImGuiInputTextFlags_ReadOnly);
						ImGui::SameLine();
						if (ImGui::Button("..." "##iconpath"))
						{
							auto optContentPath = projectInfo.TryGetFolderPath(EProjectFolderType::Content);
							F_ASSERT(optContentPath && "Content folder not set correctly in project info.");
							FileDialog fd{};
						
							std::string openIcoPath = fd.OpenFileDialog(
								"Open Icon File", optContentPath->string(), { "*.ico" }, "Ico Files");
						
							if (!openIcoPath.empty())
							{
								if (openIcoPath.starts_with(optContentPath->string()))
								{
									if (openIcoPath != iconPath && fs::exists(fs::path{ openIcoPath }))
									{
										projectInfo.SetFileIconPath(fs::path{ openIcoPath });
										iconPath = openIcoPath;
										hasError = false;
									}
								}
								else
								{
									F_ERROR("Failed to change icon. Path '{}' is invalid. Icons must be in the project content", openIcoPath);					
									hasError = true;
								}
							}
							else
							{
								hasError = false;
							}
						}
						
						if (auto* pIconTexture = projectInfo.GetIconTexturePtr())
						{
							ImGui::SameLine(0.0f, 32.0f);
							ImGui::Image((ImTextureID)(intptr_t)pIconTexture->GetID(), ImVec2{ 64.0f, 64.0f });
						}
						
						if (hasError)
						{
							ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, "Invalid path. The game icon must be in the content folder for the project");
						}
					}
				},
				{
					"GameType", [&]() {
						std::string gameType{ coreGlobals.GetGameTypeStr(coreGlobals.GetGameType()) };
						ImGui::InlineLabel(ICON_FA_GAMEPAD " Game Type:");
						ImGui::ItemToolTip("The type of game this is going to be");
						if (ImGui::BeginCombo("##game_type", gameType.c_str()))
						{
							for (const auto& [eType, sTypeStr] : coreGlobals.GetGameTypesMap())
							{
								if (ImGui::Selectable(sTypeStr.c_str(), sTypeStr == gameType))
								{
									gameType = sTypeStr;
									coreGlobals.SetGameType(eType);
								}
							}
						
							ImGui::EndCombo();
						}
					}
				},
				{
					"Default Scene", [&]() {
						std::string defaultScene{ projectInfo.GetDefaultScene() };
						ImGui::InlineLabel(ICON_FA_IMAGE " Default Scene");
						ImGui::ItemToolTip("The default scene to be loaded when the project is loaded");
						if (ImGui::BeginCombo("##default_scene", defaultScene.c_str()))
						{
							for (const auto& sceneName : SCENE_MANAGER().GetSceneNames())
							{
								if (ImGui::Selectable(sceneName.c_str(), sceneName == defaultScene))
								{
									defaultScene = sceneName;
									projectInfo.SetDefaultScene(defaultScene);
								}
							}
						
							ImGui::EndCombo();
						}
					}
				}
			}
		};
	}

	ProjectSettingsDisplay::SettingCategory ProjectSettingsDisplay::CreatePhysicsSettings(CoreEngineData& coreGlobals, ProjectInfo& projectInfo, MainRegistry& mainRegistry)
	{
		return SettingCategory{
			.name = "Physics",
			.items = {
				{
					"Enabled", [&]() {
						static bool enabled{ coreGlobals.IsPhysicsEnabled() };
						ImGui::InlineLabel("Enable");
						if (ImGui::Checkbox("##physicsEnable", &enabled))
						{
							enabled ? coreGlobals.EnablePhysics() : coreGlobals.DisablePhysics();
						}
					}
				},
				{
					"Iterations", [&]() {
						static int velocity{ coreGlobals.GetVelocityIterations() };
						static int position{ coreGlobals.GetPositionIterations() };
						ImGui::InlineLabel("Position");
						if (ImGui::InputInt("##positionItr", &position))
						{
							coreGlobals.SetPositionIterations(position);
						}
						ImGui::InlineLabel("Velocity");
						if (ImGui::InputInt("##velocityItr", &velocity))
						{
							coreGlobals.SetVelocityIterations(velocity);
						}
					}
				},
				{
					"Gravity", [&]() {
						static float gravity{ coreGlobals.GetGravity() };
						ImGui::InlineLabel("Gravity");
						if (ImGui::InputFloat("##gravity", &gravity))
						{
							coreGlobals.SetGravity(gravity);
						}
					}
				},
				{
					"Collision Categories", [&]() {
						static float gravity{ coreGlobals.GetGravity() };
						static int currentIndex{ 0 };
						auto filters = GetFilterStrings();
						ImGui::InlineLabel("Collision Categories");
						if (ImGui::ListBox("##collisionCategories", &currentIndex, vector_getter, static_cast<void*>(&filters), static_cast<int>(filters.size())))
						{
						}
					}
				},
			}
		};
	}
				
	ProjectSettingsDisplay::SettingCategory ProjectSettingsDisplay::CreateGraphicsSettings(CoreEngineData& coreGlobals, ProjectInfo& projectInfo, MainRegistry& mainRegistry)
	{
		return SettingCategory{
			.name = "Graphics",
			.items = {
				{
					"Resolution", [&]() {
						ImGui::SeparatorText("Graphics Settings");

						int width{ coreGlobals.WindowWidth() };
						int height{ coreGlobals.WindowHeight() };
						
						ImGui::InlineLabel("Width");
						if (ImGui::InputInt("##windowwidth", &width))
						{
							coreGlobals.SetWindowWidth(width);
						}
						ImGui::InlineLabel("Height");
						if (ImGui::InputInt("##windowheight", &height))
						{
							coreGlobals.SetWindowHeight(height);
						}
					}
				},
			}
		};
	}

	ProjectSettingsDisplay::SettingCategory ProjectSettingsDisplay::CreateAudioSettings(CoreEngineData& coreGlobals, ProjectInfo& projectInfo, MainRegistry& mainRegistry)
	{
		return SettingCategory{
		"Audio",
		{},
		{
			{
				"Global Overrides",
				{
					{
						"Overrides",
						[&]
						{
							ImGui::SeparatorText("Global Sound Overrides");
							ImGui::InlineLabel("Enable");
							ImGui::Checkbox("##enable_override", &projectInfo.GetAudioConfig().globalOverrideEnabled);
							ImGui::InlineLabel("Volume");
							if (ImGui::InputInt("##override_volume", &projectInfo.GetAudioConfig().globalVolumeOverride))
							{
								projectInfo.GetAudioConfig().globalVolumeOverride = std::clamp(projectInfo.GetAudioConfig().globalVolumeOverride, 0, 100);
							}
						}
					}
				},
				{}
			},
			{
				"Music Overrides",
				{
					{
						"Music Overrides",
						[&]
						{
							ImGui::SeparatorText("Music Channel Overrides");
							ImGui::InlineLabel("Enable");
							ImGui::ItemToolTip("Enables music channel override. If enabled, the volume set here will override other settings");
							ImGui::Checkbox("##enable_music_override", &projectInfo.GetAudioConfig().musicVolumeOverrideEnabled);
							ImGui::InlineLabel("Volume");
							ImGui::ItemToolTip("Music channel volume override");
							if (ImGui::InputInt("##override_music_volume", &projectInfo.GetAudioConfig().musicVolumeOverride))
							{
								projectInfo.GetAudioConfig().musicVolumeOverride = std::clamp(projectInfo.GetAudioConfig().musicVolumeOverride, 0, 100);
							}
						}
					}
				},
				{}
			},
			{
				"Sound FX Overrides",
				{
					{
						"SoundFX Overrides",
						[&]
						{
							ImGui::SeparatorText("Sound FX Channel Settings");
							int allocatedChannels{ projectInfo.GetAudioConfig().GetAllocatedChannelCount() };
							ImGui::InlineLabel("Channels");
							ImGui::ItemToolTip("The number of allocated channels. Min 8 channels. Max 64 channels");
							if (ImGui::InputInt("##channels_allocated", &allocatedChannels))
							{
								int newChannelCount{ allocatedChannels - projectInfo.GetAudioConfig().GetAllocatedChannelCount() };
								if (newChannelCount != 0)
								{
									if (!projectInfo.GetAudioConfig().UpdateSoundChannels(newChannelCount))
									{
										F_ERROR("Failed to update sound channels");
									}
								}
							}

							if (ImGui::BeginTable("ChannelTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
							{
								ImGui::TableSetupColumn("Channel");
								ImGui::TableSetupColumn("Enabled");
								ImGui::TableSetupColumn("Volume");
								ImGui::TableHeadersRow();

								for (const auto& [channelId, state] : projectInfo.GetAudioConfig().GetSoundChannelMap())
								{
									bool enabled{ state.first };
									int volume{ state.second };

									ImGui::TableNextRow();

									ImGui::TableSetColumnIndex(0);
									ImGui::Text("%d", channelId);

									ImGui::TableSetColumnIndex(1);
									if (ImGui::Checkbox(("##enabled_" + std::to_string(channelId)).c_str(), &enabled))
									{
										projectInfo.GetAudioConfig().EnableChannelOverride(channelId, enabled);
									}

									ImGui::TableSetColumnIndex(2);
									if (ImGui::SliderInt(("##volume_" + std::to_string(channelId)).c_str(), &volume, 0, 100))
									{
										projectInfo.GetAudioConfig().SetChannelVolume(channelId, volume);
									}
								}

								ImGui::EndTable();
							}
						}
					}
				},
				{}
			}
		}
		};
	}

}
