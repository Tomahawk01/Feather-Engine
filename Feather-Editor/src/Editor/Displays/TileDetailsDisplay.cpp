#include "TileDetailsDisplay.h"

#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Utilities/DrawComponentUtils.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"
#include "Editor/Tools/ToolManager.h"
#include "Editor/Tools/TileTool.h"
#include "Editor/Commands/CommandManager.h"

#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Essentials/Texture.h"
#include "Logger/Logger.h"

#include <imgui.h>

namespace Feather {

	TileDetailsDisplay::TileDetailsDisplay()
		: m_SelectedLayer{ -1 }
		, m_RenameLayerBuff{}
		, m_Renaming{ false }
		, m_bDeleteLayer{ false }
	{}

	TileDetailsDisplay::~TileDetailsDisplay()
	{}

	void TileDetailsDisplay::Draw()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentSceneObject();
		auto& toolManager = SCENE_MANAGER().GetToolManager();

		auto activeTool = toolManager.GetActiveTool();
		if (!activeTool)
			return;

		if (!ImGui::Begin(ICON_FA_CLIPBOARD_LIST " Tile Details") || !currentScene)
		{
			ImGui::End();
			return;
		}

		auto& tileData = activeTool->GetTileData();

		// Transform
		DrawComponentsUtil::DrawComponentInfo(tileData.transform);
		ImGui::AddSpaces(2);

		// Sprite
		DrawSpriteComponent(tileData.sprite, currentScene);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::InlineLabel("Box Collider");
		ImGui::Checkbox("##Box_Collider", &tileData.isCollider);
		if (tileData.isCollider)
			DrawComponentsUtil::DrawComponentInfo(tileData.boxCollider);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::InlineLabel("Circle Collider");
		ImGui::Checkbox("##Cicle_Collider", &tileData.isCircle);
		if (tileData.isCircle)
			DrawComponentsUtil::DrawComponentInfo(tileData.circleCollider);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::InlineLabel("Animation");
		ImGui::Checkbox("##Animation", &tileData.hasAnimation);
		if (tileData.hasAnimation)
			DrawComponentsUtil::DrawComponentInfo(tileData.animation);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::InlineLabel("Physics");
		ImGui::Checkbox("##Physics", &tileData.hasPhysics);
		if (tileData.hasPhysics)
			DrawComponentsUtil::DrawComponentInfo(tileData.physics);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::End();

		if (ImGui::Begin(ICON_FA_LAYER_GROUP " Tile Layers"))
		{
			ImGui::SeparatorText("Tile Layers");

			auto& spriteLayers = currentScene->GetLayerParams();
			std::string checkName{ m_RenameLayerBuff.data() };

			if (ImGui::Button(ICON_FA_PLUS_CIRCLE " Add Layer"))
				currentScene->AddNewLayer();

			ImGui::ItemToolTip("Add Layer");

			ImGui::AddSpaces(2);
			ImGui::Separator();
			ImGui::AddSpaces(2);

			float itemWidth{ ImGui::GetWindowWidth() - 96.0f };
			F_ASSERT(itemWidth > 0 && "Item width must not be less than zero");

			auto reverseView = spriteLayers | std::ranges::views::reverse;

			for (auto rit = reverseView.begin(); rit < reverseView.end(); rit++)
			{
				int n = std::distance(rit, reverseView.end()) - 1;

				auto& spriteLayer = *rit;
				bool isSelected = m_SelectedLayer == n;

				ImGui::Selectable(spriteLayer.layerName.c_str(), isSelected, 0, ImVec2{ itemWidth, 20.0f });

				if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
				{
					int nNext = n + (ImGui::GetMouseDragDelta(0).y < 0.0f ? 1 : -1);
					if (nNext >= 0 && nNext < reverseView.size())
					{
						const int nextLayer = spriteLayers[n].layer;
						spriteLayers[n].layer = spriteLayers[nNext].layer;
						spriteLayers[nNext].layer = nextLayer;

						std::swap(spriteLayers[n], spriteLayers[nNext]);

						auto spriteView = currentScene->GetRegistry().GetRegistry().view<SpriteComponent, TileComponent>();

						for (auto entity : spriteView)
						{
							auto& sprite = spriteView.get<SpriteComponent>(entity);

							if (sprite.layer == n)
								sprite.layer = nNext;
							else if (sprite.layer == nNext)
								sprite.layer = n;
						}

						m_SelectedLayer = nNext;
						tileData.sprite.layer = nNext;

						auto moveTileLayerCmd = UndoableCommands{ MoveTileLayerCmd{.sceneObject = currentScene, .from = n, .to = nNext} };
						COMMAND_MANAGER().Execute(moveTileLayerCmd);

						ImGui::ResetMouseDragDelta();
					}
				}

				// Set the current selected layer
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !m_Renaming)
				{
					m_SelectedLayer = n;
					tileData.sprite.layer = n;
				}

				// Start the rename
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && !m_Renaming && isSelected)
				{
					m_Renaming = true;
					m_RenameLayerBuff.clear();
					m_RenameLayerBuff = spriteLayer.layerName;
				}

				bool checkPassed{ currentScene->CheckLayerName(checkName) };

				ImGui::SameLine();
				ImGui::PushID(n);
				ImGui::PushStyleColor(ImGuiCol_Button, BLACK_TRANSPARENT);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, BLACK_TRANSPARENT);
				if (ImGui::Button(spriteLayer.isVisible ? ICON_FA_EYE : ICON_FA_EYE_SLASH, { 24.0f, 24.0f }))
					spriteLayer.isVisible= !spriteLayer.isVisible;

				ImGui::SameLine();

				if (ImGui::Button(ICON_FA_TRASH, { 24.0f, 24.0f }))
				{
					m_bDeleteLayer = true;
					m_DeleteLayer = n;
				}

				ImGui::PopStyleColor(2);

				if (m_Renaming && isSelected)
				{
					ImGui::SetKeyboardFocusHere();
					if (ImGui::InputText("##rename", m_RenameLayerBuff.data(), 255, ImGuiInputTextFlags_EnterReturnsTrue) && checkPassed)
					{
						ChangeTileLayerNameCmd changeNameCmd{ .sceneObject = currentScene, .oldName = spriteLayer.layerName, .newName = checkName };

						spriteLayer.layerName = checkName;
						m_RenameLayerBuff.clear();
						m_Renaming = false;

						auto changeLayerNameCmd = UndoableCommands{ changeNameCmd };
						COMMAND_MANAGER().Execute(changeLayerNameCmd);
					}
					else if (m_Renaming && ImGui::IsKeyPressed(ImGuiKey_Escape))
					{
						m_RenameLayerBuff.clear();
						m_Renaming = false;
					}
				}

				// Display an error if the name already exists
				if (!checkPassed && isSelected)
					ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, std::format("{} - Already exists", checkName).c_str());

				ImGui::PopID();
			}

			if (m_bDeleteLayer)
			{
				ImGui::OpenPopup("Delete Layer");
			}

			if (ImGui::BeginPopupModal("Delete Layer"))
			{
				ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Are you sure?\nThis will delete all tiles at this layer");

				if (ImGui::Button("Ok"))
				{
					if (m_DeleteLayer >= 0)
					{
						if (!currentScene->DeleteLayer(m_DeleteLayer))
						{
							F_WARN("Failed to delete layer: '{}'", m_DeleteLayer);
						}

						m_bDeleteLayer = false;
						m_DeleteLayer = -1;
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel"))
				{
					m_bDeleteLayer = false;
					m_DeleteLayer = -1;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::End();
		}
	}

	void TileDetailsDisplay::DrawSpriteComponent(SpriteComponent& sprite, Scene* scene)
	{
		bool IsChanged{ false };

		ImGui::SeparatorText("Sprite");
		ImGui::PushID(entt::type_hash<SpriteComponent>::value());
		if (ImGui::TreeNodeEx("##SpriteTree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::AddSpaces(2);

			ImGui::InlineLabel("texture: ");
			ImGui::TextColored(ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f }, sprite.textureName.c_str());

			std::string layer{};

			// Set layer description
			if (sprite.layer >= 0 && sprite.layer < scene->GetLayerParams().size())
				layer = scene->GetLayerParams()[sprite.layer].layerName;

			ImGui::InlineLabel("layer");
			ImGui::TextColored(ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f }, layer.c_str());

			ImGui::AddSpaces(2);

			// Color picker
			ImVec4 color = { sprite.color.r / 255.0f, sprite.color.g / 255.0f, sprite.color.b / 255.0f, sprite.color.a / 255.0f };
			ImGui::InlineLabel("color");
			if (ImGui::ColorEdit4("##color", &color.x, IMGUI_COLOR_PICKER_FLAGS))
			{
				sprite.color.r = static_cast<GLubyte>(color.x * 255.0f);
				sprite.color.g = static_cast<GLubyte>(color.y * 255.0f);
				sprite.color.b = static_cast<GLubyte>(color.z * 255.0f);
				sprite.color.a = static_cast<GLubyte>(color.w * 255.0f);
			}

			ImGui::AddSpaces(2);

			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("width");
			if (ImGui::InputFloat("##width", &sprite.width, 1.0f, 8.0f))
			{
				sprite.width = glm::clamp(sprite.width, 8.0f, 2000.0f);
				IsChanged = true;
			}
			ImGui::InlineLabel("height");
			if (ImGui::InputFloat("##height", &sprite.height, 1.0f, 8.0f))
			{
				sprite.height = glm::clamp(sprite.height, 8.0f, 2000.0f);
				IsChanged = true;
			}

			ImGui::TreePop();
			ImGui::PopItemWidth();
		}

		ImGui::PopID();

		if (IsChanged)
		{
			auto tileTool = SCENE_MANAGER().GetToolManager().GetActiveTool();
			if (tileTool)
			{
				tileTool->SetSpriteRect(glm::vec2{ sprite.width, sprite.height });
			}
			else
			{
				auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.textureName);
				if (!texture)
					return;

				GenerateUVs(sprite, texture->GetWidth(), texture->GetHeight());
			}
		}
	}

}