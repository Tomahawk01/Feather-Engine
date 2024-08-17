#include "AssetDisplay.h"
#include "Utils/FeatherUtilities.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Scripting/InputManager.h"
#include "Core/Resources/AssetManager.h"
#include "Logger/Logger.h"

#include "../Utilities/EditorUtilities.h"
#include "../Scene/SceneManager.h"

#include <imgui.h>

constexpr float DEFAULT_ASSET_SIZE = 128.0f;
constexpr ImVec2 DRAG_ASSET_SIZE = ImVec2{ 32.0f, 32.0f };

namespace Feather {

	AssetDisplay::AssetDisplay()
		: m_ItemHovered{ false },
		m_AssetTypeChanged{ true },
		m_Rename{ false },
		m_WindowSelected{ false },
		m_WindowHovered{ false },
		m_SelectedAssetName{ "" },
		m_SelectedType{ "Textures" },
		m_DragSource{ "" },
		m_RenameBuf{ "" },
		m_eSelectedType{ AssetType::TEXTURE },
		m_AssetSize{ DEFAULT_ASSET_SIZE },
		m_SelectedID{ -1 }
	{
		SetAssetType();
	}

	void AssetDisplay::Draw()
	{
		if (!ImGui::Begin("Assets"))
		{
			ImGui::End();
			return;
		}

		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		ImGui::Text("Asset Type");
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

		if (ImGui::BeginChild("##AssetTable", ImVec2{ 0.0f, 0.0f }, NULL, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_ChildWindow))
		{
			m_WindowHovered = ImGui::IsWindowHovered();
			m_WindowSelected = ImGui::IsWindowFocused();

			DrawSelectedAssets();

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

		float windowWidth = ImGui::GetWindowWidth();
		int numCols = static_cast<int>((windowWidth - m_AssetSize) / m_AssetSize);
		int numRows = static_cast<int>(assetNames.size() / (numCols <= 1 ? 1 : numCols) + 1);

		if (!numCols || !numRows)
			return;

		ImGuiTableFlags tableFlags{ 0 };
		tableFlags |= ImGuiTableFlags_SizingFixedFit;

		int k{ 0 }, id{ 0 };

		auto assetItr = assetNames.begin();

		if (ImGui::BeginTable("Assets", numCols, tableFlags))
		{
			for (int row = 0; row < numRows; row++)
			{
				ImGui::TableNextRow();
				for (int col = 0; col < numCols; col++)
				{
					if (assetItr == assetNames.end())
						break;

					ImGui::PushID(k++);
					ImGui::TableSetColumnIndex(col);

					bool IsSelectedAsset{ m_SelectedID == id };
					if (IsSelectedAsset)
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4{ 0.0f, 0.9f, 0.0f, 0.5f }));

					GLuint textureID{ GetTextureID(*assetItr) };
					std::string checkName{ m_RenameBuf.data() };

					if (textureID == 0)
						break;

					ImGui::ImageButton((ImTextureID)textureID, ImVec2{ m_AssetSize, m_AssetSize });

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !m_Rename)
						m_SelectedID = id;

					const char* assetName = (*assetItr).c_str();

					if (IsSelectedAsset && ImGui::BeginPopupContextItem())
					{
						OpenAssetContext(*assetItr);
						ImGui::EndPopup();
					}

					if (ImGui::BeginDragDropSource())
					{
						ImGui::SetDragDropPayload(m_DragSource.c_str(), assetName, (strlen(assetName) + 1) * sizeof(char), ImGuiCond_Once);
						ImGui::Image((ImTextureID)textureID, DRAG_ASSET_SIZE);
						ImGui::EndDragDropSource();
					}

					if (!m_Rename || !IsSelectedAsset)
						ImGui::Text(assetName);

					if (m_Rename && IsSelectedAsset)
					{
						ImGui::SetKeyboardFocusHere();
						if (ImGui::InputText("##rename", m_RenameBuf.data(), 255, ImGuiInputTextFlags_EnterReturnsTrue))
						{
							if (!DoRenameAsset(*assetItr, checkName))
								F_ERROR("Failed to change asset name");

							m_RenameBuf.clear();
							m_Rename = false;
						}
						else if (m_Rename && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
						{
							m_RenameBuf.clear();
							m_Rename = false;
						}
					}

					if (!m_Rename && IsSelectedAsset && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					{
						m_RenameBuf.clear();
						m_RenameBuf = *assetItr;
						m_Rename = true;
					}

					if (m_Rename && IsSelectedAsset)
					{
						if (assetName != checkName)
							CheckRename(checkName);
					}

					++id;
					++assetItr;
					ImGui::PopID();
				}
			}

			ImGui::EndTable();
		}
	}

	unsigned int AssetDisplay::GetTextureID(const std::string& assetName)
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
		}

		return 0;
	}

	bool AssetDisplay::DoRenameAsset(const std::string& oldName, const std::string& newName)
	{
		if (newName.empty())
			return false;

		if (m_eSelectedType == AssetType::SCENE)
		{
			// TODO: Change scene name
		}
		else
		{
			auto& assetManager = MAIN_REGISTRY().GetAssetManager();
			return assetManager.ChangeAssetName(oldName, newName, m_eSelectedType);
		}

		F_ASSERT(false && "How did it get here?");
		return false;
	}

	void AssetDisplay::CheckRename(const std::string& checkName)
	{
		if (checkName.empty())
		{
			ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, "Rename text cannot be blank!");
			return;
		}

		bool hasAsset{ false };

		if (m_eSelectedType == AssetType::SCENE)
		{
			// TODO: Check if scene name already exists
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
		if (ImGui::Selectable("rename"))
		{
			m_Rename = true;
		}
		if (ImGui::Selectable("delete"))
		{
			if (m_eSelectedType == AssetType::SCENE)
			{
				// TODO: Check if scene name already exists
			}
			else
			{
				auto& assetManager = MAIN_REGISTRY().GetAssetManager();
				if (!assetManager.DeleteAsset(assetName, m_eSelectedType))
				{
					F_ERROR("Failed to delete asset '{}'", assetName);
				}
			}
		}
	}

}
