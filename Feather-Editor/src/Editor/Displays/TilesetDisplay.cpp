#include "TilesetDisplay.h"
#include "Logger/Logger.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Utils/FeatherUtilities.h"

#include "Editor/Scene/SceneManager.h"
#include "Editor/Tools/ToolManager.h"
#include "Editor/Tools/TileTool.h"

#include <imgui.h>

namespace Feather {

	TilesetDisplay::~TilesetDisplay()
	{}

	void TilesetDisplay::Draw()
	{
		if (!ImGui::Begin("Tileset"))
		{
			ImGui::End();
			return;
		}

		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		auto tilesets = GetKeys(assetManager.GetAllTextures(), [](const auto& pair) { return pair.second->IsTileset(); });

		if (ImGui::BeginCombo("Choose Tileset", m_Tileset.c_str()))
		{
			for (const auto& tileset : assetManager.GetTilesetNames())
			{
				bool IsSelected = m_Tileset == tileset;
				if (ImGui::Selectable(tileset.c_str(), IsSelected))
				{
					m_Tileset = tileset;
					SCENE_MANAGER().SetTileset(tileset);
				}

				if (IsSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		if (m_Tileset.empty())
		{
			ImGui::End();
			return;
		}

		auto texture = assetManager.GetTexture(m_Tileset);
		if (!texture)
		{
			ImGui::End();
			return;
		}

		int textureWidth = texture->GetWidth();
		int textureHeight = texture->GetHeight();

		int cols = textureWidth / 32;
		int rows = textureHeight / 32;

		float uvW = 32 / static_cast<float>(textureWidth);
		float uvH = 32 / static_cast<float>(textureHeight);

		float ux{ 0.0f }, uy{ 0.0f }, vx{ uvW }, vy{ uvH };

		ImGuiTableFlags tableFlags{ 0 };
		tableFlags |= ImGuiTableFlags_SizingFixedFit;
		tableFlags |= ImGuiTableFlags_ScrollX;

		int k{ 0 }, id{ 0 };

		if (ImGui::BeginTable("Tileset", cols, tableFlags))
		{
			for (int i = 0; i < rows; i++)
			{
				ImGui::TableNextRow();
				for (int j = 0; j < cols; j++)
				{
					ImGui::TableSetColumnIndex(j);

					if (m_Selected == id)
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4{ 0.0f, 0.9f, 0.0f, 0.5f }));

					// Create unique id for the buttons
					ImGui::PushID(k++);

					if (ImGui::ImageButton((ImTextureID)(intptr_t)texture->GetID(), ImVec2{ 32.0f * 1.5f, 32.0f * 1.5f }, ImVec2{ ux, uy }, ImVec2{ vx, vy }))
					{
						m_Selected = id;
						auto activeTool = SCENE_MANAGER().GetToolManager().GetActiveTool();
						if (activeTool)
							activeTool->SetSpriteUVs(j, i);
					}

					ImGui::PopID();

					// Advance the UVs to the next column
					ux += uvW;
					vx += uvW;
					++id;
				}
				// Put the UVs back to start column of the next row
				ux = 0.0f;
				vx = uvW;
				uy += uvH;
				vy += uvH;
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}

}
