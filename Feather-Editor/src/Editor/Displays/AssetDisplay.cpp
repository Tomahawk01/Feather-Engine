#include "AssetDisplay.h"

#include "AssetDisplayUtils.h"
#include "Utils/FeatherUtilities.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Scripting/InputManager.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/Prefab.h"
#include "Core/CoreUtils/ProjectInfo.h"
#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Texture.h"
#include "Renderer/Essentials/Font.h"
#include "Sounds/Essentials/Music.h"
#include "Sounds/Essentials/SoundFX.h"
#include "Sounds/MusicPlayer/MusicPlayer.h"
#include "Sounds/SoundPlayer/SoundFXPlayer.h"
#include "FileSystem/Process/FileProcessor.h"
#include "FileSystem/Utilities/FileSystemUtilities.h"
#include "Logger/Logger.h"

#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Utilities/EditorState.h"
#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Loaders/ProjectLoader.h"

#include <imgui.h>

constexpr float DEFAULT_ASSET_SIZE = 64.0f;
constexpr ImVec2 DRAG_ASSET_SIZE = ImVec2{ 32.0f, 32.0f };

namespace Feather {

	AssetDisplay::AssetDisplay()
		: m_AssetTypeChanged{ true },
		m_Rename{ false },
		m_WindowSelected{ false },
		m_WindowHovered{ false },
		m_OpenAddAssetModal{ false },
		m_SelectedAssetName{},
		m_SelectedType{ "Textures" },
		m_DragSource{},
		m_RenameBuf{},
		m_eSelectedType{ AssetType::TEXTURE },
		m_AssetSize{ DEFAULT_ASSET_SIZE },
		m_SelectedID{ -1 }
	{
		SetAssetType();
	}

	void AssetDisplay::Draw()
	{
		if (auto& editorState = MAIN_REGISTRY().GetContext<EditorStatePtr>())
		{
			if (!editorState->IsDisplayOpen(EDisplay::AssetBrowser))
			{
				return;
			}
		}

		if (!ImGui::Begin(ICON_FA_FILE_ALT " Assets"))
		{
			ImGui::End();
			return;
		}

		DrawToolbar();

		if (ImGui::BeginChild("##AssetTable", ImVec2{ 0.0f, 0.0f }, ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_ChildWindow))
		{
			m_WindowHovered = ImGui::IsWindowHovered();
			m_WindowSelected = ImGui::IsWindowFocused();

			DrawSelectedAssets();

			if (m_SelectedID == -1 && ImGui::BeginPopupContextWindow("##AddAsset"))
			{
				if (ImGui::Selectable(AssetDisplayUtils::AddAssetBasedOnType(m_eSelectedType).c_str()))
					m_OpenAddAssetModal = true;

				ImGui::EndPopup();
			}

			if (m_OpenAddAssetModal)
				AssetDisplayUtils::OpenAddAssetModalBasedOnType(m_eSelectedType, &m_OpenAddAssetModal);

			ImGui::EndChild();
		}

		ImGui::End();
	}

	void AssetDisplay::Update()
	{
	}

	void AssetDisplay::SetAssetType()
	{
		if (!m_AssetTypeChanged)
			return;

		if (m_SelectedType == "Textures")
		{
			m_eSelectedType = AssetType::TEXTURE;
			m_DragSource = std::string{ DROP_TEXTURE_SRC };
		}
		else if (m_SelectedType == "Fonts")
		{
			m_eSelectedType = AssetType::FONT;
			m_DragSource = std::string{ DROP_FONT_SRC };
		}
		else if (m_SelectedType == "Music")
		{
			m_eSelectedType = AssetType::MUSIC;
			m_DragSource = std::string{ DROP_MUSIC_SRC };
		}
		else if (m_SelectedType == "SoundFX")
		{
			m_eSelectedType = AssetType::SOUNDFX;
			m_DragSource = std::string{ DROP_SOUNDFX_SRC };
		}
		else if (m_SelectedType == "Scenes")
		{
			m_eSelectedType = AssetType::SCENE;
			m_DragSource = std::string{ DROP_SCENE_SRC };
		}
		else if (m_SelectedType == "Prefabs")
		{
			m_eSelectedType = AssetType::PREFAB;
			m_DragSource = std::string{ DROP_PREFAB_SRC };
		}
		else
		{
			m_eSelectedType = AssetType::NO_TYPE;
			m_DragSource = "NO_ASSET_TYPE";
		}

		m_AssetTypeChanged = false;
	}

	void AssetDisplay::DrawSelectedAssets()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		std::vector<std::string> assetNames{};

		if (m_eSelectedType == AssetType::SCENE)
		{
			assetNames = SCENE_MANAGER().GetSceneNames();
		}
		else
		{
			assetNames = assetManager.GetAssetKeyNames(m_eSelectedType);
		}

		if (assetNames.empty())
			return;

		const float avail = ImGui::GetContentRegionAvail().x;
		const ImGuiStyle& style = ImGui::GetStyle();

		// one cell = image width + left/right cell padding
		const float cellW = m_AssetSize + style.CellPadding.x * 2.0f;

		// number of columns that actually fit in the visible region
		int numCols = (int)std::floor((avail + 0.5f) / cellW);
		if (numCols < 1)
			numCols = 1;

		ImGuiTableFlags tableFlags{ 0 };
		tableFlags |= ImGuiTableFlags_SizingFixedFit;

		int k{ 0 }, id{ 0 };

		auto assetItr = assetNames.begin();

		if (ImGui::BeginTable("Assets", numCols, tableFlags))
		{
			for (int c = 0; c < numCols; ++c)
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, cellW);

			for (auto assetItr = assetNames.begin(); assetItr != assetNames.end(); ++assetItr, ++id)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(k++);

				bool selectedAsset = (m_SelectedID == id);

				if (selectedAsset)
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4{ 0.0f, 0.9f, 0.0f, 0.3f }));
				GLuint textureID = GetTextureID(*assetItr);
				std::string checkName{ m_RenameBuf.data() };
				std::string assetBtn = "##asset" + std::to_string(id);

				if (m_eSelectedType == AssetType::PREFAB)
				{
					if (auto prefab = assetManager.GetPrefab(*assetItr))
					{
						auto& sprite = prefab->GetPrefabbedEntity().sprite;
						if (textureID && sprite)
						{
							ImGui::ImageButton(
								assetBtn.c_str(),
								(ImTextureID)(intptr_t)textureID,
								ImVec2{ m_AssetSize, m_AssetSize },
								ImVec2{ sprite->uvs.u, sprite->uvs.v },
								ImVec2{ sprite->uvs.u + sprite->uvs.uv_width, sprite->uvs.v + sprite->uvs.uv_height });
						}
						else
						{
							ImGui::Button(assetBtn.c_str(), ImVec2{ m_AssetSize, m_AssetSize });
						}
					}
				}
				else
				{
					if (textureID == 0)
					{
						ImGui::PopID();
						continue;
					}

					ImGui::ImageButton(assetBtn.c_str(), (ImTextureID)(intptr_t)textureID, ImVec2{ m_AssetSize, m_AssetSize });
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !m_Rename)
				{
					m_SelectedID = id;
				}

				const char* assetName = (*assetItr).c_str();

				if (selectedAsset && ImGui::BeginPopupContextItem())
				{
					OpenAssetContext(*assetItr);
					ImGui::EndPopup();
				}

				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload(m_DragSource.c_str(), assetName, (strlen(assetName) + 1) * sizeof(char), ImGuiCond_Once);
					ImGui::Image((ImTextureID)(intptr_t)textureID, DRAG_ASSET_SIZE);
					ImGui::EndDragDropSource();
				}

				if (!m_Rename || !selectedAsset)
				{
					ImGui::TextWrapped(assetName);
				}

				if (m_Rename && selectedAsset)
				{
					ImGui::SetKeyboardFocusHere();
					if (ImGui::InputText("##rename", m_RenameBuf.data(), 255, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						if (!DoRenameAsset(*assetItr, checkName))
							F_ERROR("Failed to change asset name");

						m_RenameBuf.clear();
						m_Rename = false;
					}
					else if (m_Rename && ImGui::IsKeyPressed(ImGuiKey_Escape))
					{
						m_RenameBuf.clear();
						m_Rename = false;
					}
				}

				if (!m_Rename && selectedAsset && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					m_RenameBuf.clear();
					m_RenameBuf = *assetItr;
					m_Rename = true;
				}

				if (m_Rename && selectedAsset)
				{
					if (std::string{ assetName } != checkName)
						CheckRename(checkName);
				}

				ImGui::PopID();
			}

			ImGui::EndTable();
		}

		// NOTE: Click empty space to clear selection
		if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(0) && !m_Rename)
		{
			m_SelectedID = -1;
		}
	}

	unsigned int AssetDisplay::GetTextureID(const std::string& assetName) const
	{
		auto& assetManager = MAIN_REGISTRY().GetAssetManager();

		switch (m_eSelectedType)
		{
		case Feather::AssetType::TEXTURE:
		{
			auto texture = assetManager.GetTexture(assetName);
			if (texture)
				return texture->GetID();
			break;
		}
		case Feather::AssetType::FONT:
		{
			auto font = assetManager.GetFont(assetName);
			if (font)
				return font->GetFontAtlasID();
			break;
		}
		case Feather::AssetType::SOUNDFX:
		case Feather::AssetType::MUSIC:
		{
			auto texture = assetManager.GetTexture("music_icon");
			if (texture)
				return texture->GetID();
			break;
		}
		case Feather::AssetType::SCENE:
		{
			auto texture = assetManager.GetTexture("scene_icon");
			if (texture)
				return texture->GetID();
			break;
		}
		case Feather::AssetType::PREFAB:
		{
			if (auto prefab = assetManager.GetPrefab(assetName))
			{
				if (auto& sprite = prefab->GetPrefabbedEntity().sprite)
				{
					if (auto texture = assetManager.GetTexture(sprite->textureName))
					{
						return texture->GetID();
					}
				}
			}

			break;
		}
		}

		return 0;
	}

	bool AssetDisplay::DoRenameAsset(const std::string& oldName, const std::string& newName) const
	{
		if (newName.empty())
			return false;

		if (m_eSelectedType == AssetType::SCENE)
		{
			return SCENE_MANAGER().ChangeSceneName(oldName, newName);
		}
		else
		{
			return ASSET_MANAGER().ChangeAssetName(oldName, newName, m_eSelectedType);
		}

		F_ASSERT(false && "How did it get here?");
		return false;
	}

	void AssetDisplay::CheckRename(const std::string& checkName) const
	{
		if (checkName.empty())
		{
			ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, "Rename text cannot be blank!");
			return;
		}

		bool hasAsset{ false };

		if (m_eSelectedType == AssetType::SCENE)
		{
			if (SCENE_MANAGER().HasScene(checkName))
				hasAsset = true;
		}
		else
		{
			auto& assetManager = MAIN_REGISTRY().GetAssetManager();
			if (assetManager.HasAsset(checkName, m_eSelectedType))
				hasAsset = true;
		}

		if (hasAsset)
			ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, std::format("Asset name '{}' already exists!", checkName).c_str());
	}

	void AssetDisplay::OpenAssetContext(const std::string& assetName)
	{
		DrawSoundContext(assetName);

		ImGui::SeparatorText("Edit");
		if (ImGui::Selectable(ICON_FA_PEN " Rename"))
		{
			m_Rename = true;
		}
		if (ImGui::Selectable(ICON_FA_TRASH " Delete"))
		{
			bool isSuccess{ false };
			if (m_eSelectedType == AssetType::SCENE)
			{
				if (!SCENE_MANAGER().DeleteScene(assetName))
				{
					F_ERROR("Failed to delete scene '{}'", assetName);
				}

				isSuccess = true;
			}
			else
			{
				if (!ASSET_MANAGER().DeleteAsset(assetName, m_eSelectedType))
				{
					F_ERROR("Failed to delete asset '{}'", assetName);
				}

				isSuccess = true;
			}

			// Whenever an asset is deleted, we want to save the project.
			// There should be some sort of message to the user before deleting??
			if (isSuccess)
			{
				auto& projectInfo = MAIN_REGISTRY().GetContext<ProjectInfoPtr>();
				F_ASSERT(projectInfo && "Project Info must exist!");
				// Save entire project
				ProjectLoader pl{};
				if (!pl.SaveLoadedProject(*projectInfo))
				{
					auto optProjectFilePath = projectInfo->GetProjectFilePath();
					F_ASSERT(optProjectFilePath && "Project file path not set correctly in project info");

					F_ERROR("Failed to save project '{}' at file '{}' after deleting asset '{}'",
							projectInfo->GetProjectName(),
							optProjectFilePath->string(),
							assetName);
				}
			}
		}

		ImGui::SeparatorText("File Explorer");
		if (ImGui::Selectable(ICON_FA_FILE_ALT " Open File Location"))
		{
			if (m_eSelectedType == AssetType::SCENE)
			{
				std::string scenePath = SCENE_MANAGER().GetSceneFilepath(assetName);
				if (!scenePath.empty())
				{
					FileProcessor fp{};
					if (!fp.OpenFileLocation(NormalizePath(scenePath)))
					{
						F_ERROR("Failed to open file location '{}'", scenePath);
					}
				}
			}
			else
			{
				std::string assetPath = ASSET_MANAGER().GetAssetFilepath(assetName, m_eSelectedType);
				if (!assetPath.empty())
				{
					FileProcessor fp{};
					if (!fp.OpenFileLocation(NormalizePath(assetPath)))
					{
						F_ERROR("Failed to open file location '{}'", assetPath);
					}
				}
			}
		}
	}

	void AssetDisplay::DrawSoundContext(const std::string& assetName)
	{
		if (m_eSelectedType == AssetType::MUSIC)
		{
			ImGui::SeparatorText("Music Controls");
			auto& musicPlayer = MAIN_REGISTRY().GetMusicPlayer();

			if (!musicPlayer.IsPlaying())
			{
				if (ImGui::Selectable(ICON_FA_PLAY " Play"))
				{
					auto& assetManager = MAIN_REGISTRY().GetAssetManager();
					if (auto music = assetManager.GetMusic(assetName))
					{
						musicPlayer.Play(*music);
					}
				}

				ImGui::BeginDisabled();
				ImGui::Selectable(ICON_FA_STOP " Stop");
				ImGui::EndDisabled();
			}
			else
			{
				ImGui::BeginDisabled();
				ImGui::Selectable(ICON_FA_PLAY " Play");
				ImGui::EndDisabled();

				if (ImGui::Selectable(ICON_FA_STOP " Stop"))
				{
					musicPlayer.Stop();
				}
			}
		}
		else if (m_eSelectedType == AssetType::SOUNDFX)
		{
			ImGui::SeparatorText("Sound Controls");
			auto& soundPlayer = MAIN_REGISTRY().GetSoundPlayer();

			if (!soundPlayer.IsPlaying(0))
			{
				if (ImGui::Selectable(ICON_FA_PLAY " Play"))
				{
					auto& assetManager = MAIN_REGISTRY().GetAssetManager();
					if (auto soundfx = assetManager.GetSoundFx(assetName))
					{
						soundPlayer.Play(*soundfx, 0, 0);
					}
				}

				ImGui::BeginDisabled();
				ImGui::Selectable(ICON_FA_STOP " Stop");
				ImGui::EndDisabled();
			}
			else
			{
				ImGui::BeginDisabled();
				ImGui::Selectable(ICON_FA_PLAY " Play");
				ImGui::EndDisabled();

				if (ImGui::Selectable(ICON_FA_STOP " Stop"))
				{
					soundPlayer.Stop(0);
				}
			}
		}
	}

	void AssetDisplay::DrawToolbar()
	{
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_Button, BLACK_TRANSPARENT);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BLACK_TRANSPARENT);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, BLACK_TRANSPARENT);
		ImGui::Button("Asset Type");
		ImGui::PopStyleColor(3);

		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::BeginCombo("##AssetType", m_SelectedType.c_str()))
		{
			for (const auto& assetType : m_SelectableTypes)
			{
				bool IsSelected = m_SelectedType == assetType;
				if (ImGui::Selectable(assetType.c_str(), IsSelected))
				{
					m_AssetTypeChanged = true;
					m_SelectedType = assetType;
					m_SelectedID = -1;
					SetAssetType();
				}

				if (IsSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Separator();
	}

}
