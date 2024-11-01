#include "TileDetailsDisplay.h"

#include "../Utilities/ImGuiUtils.h"
#include "../Utilities/DrawComponentUtils.h"
#include "../Scene/SceneManager.h"
#include "../Scene/SceneObject.h"
#include "../Tools/ToolManager.h"
#include "../Tools/TileTool.h"

#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Logger/Logger.h"

#include <imgui.h>

#include <ranges>

namespace Feather {

	TileDetailsDisplay::TileDetailsDisplay()
		: m_SelectedLayer{ -1 }, m_RenameLayerBuff{ "" }, m_Renaming{ false }
	{}

	TileDetailsDisplay::~TileDetailsDisplay()
	{}

	void TileDetailsDisplay::Draw()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		auto& toolManager = SCENE_MANAGER().GetToolManager();

		auto activeTool = toolManager.GetActiveTool();
		if (!activeTool)
			return;

		if (!ImGui::Begin("Tile Details") || !currentScene)
		{
			ImGui::End();
			return;
		}

		auto& tileData = activeTool->GetTileData();

		// Transform
		DrawComponentsUtil::DrawComponentInfo(tileData.transform);
		ImGui::AddSpaces(2);

		// Sprite
		DrawSpriteComponent(tileData.sprite, currentScene.get());

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::Checkbox("Box Collider", &tileData.isCollider);
		if (tileData.isCollider)
			DrawComponentsUtil::DrawComponentInfo(tileData.boxCollider);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::Checkbox("Circle Collider", &tileData.isCircle);
		if (tileData.isCircle)
			DrawComponentsUtil::DrawComponentInfo(tileData.circleCollider);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::Checkbox("Animation", &tileData.hasAnimation);
		if (tileData.hasAnimation)
			DrawComponentsUtil::DrawComponentInfo(tileData.animation);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		ImGui::Checkbox("Physics", &tileData.hasPhysics);
		if (tileData.hasPhysics)
			DrawComponentsUtil::DrawComponentInfo(tileData.physics);

		ImGui::AddSpaces(2);
		ImGui::Separator();
		ImGui::AddSpaces(2);

		if (ImGui::BeginChild("Tile Layers"))
		{
			ImGui::SeparatorText("Tile Layers");

			auto& spriteLayers = currentScene->GetLayerParams();

			if (ImGui::Button("Add"))
				currentScene->AddNewLayer();

			ImGui::AddSpaces(2);
			ImGui::Separator();
			ImGui::AddSpaces(2);

			float itemWidth{ ImGui::GetWindowWidth() - 32.0f };
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
						std::swap(reverseView[n], reverseView[nNext]);

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

				if (m_Renaming && isSelected)
				{
					// TODO: Renaming
				}

				ImGui::SameLine();

				// Layer visibility
				ImGui::Checkbox(std::format("##visible_{}", spriteLayer.layerName).c_str(), &spriteLayer.isVisible);
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}

	void TileDetailsDisplay::DrawSpriteComponent(SpriteComponent& sprite, SceneObject* scene)
	{
		bool IsChanged{ false };

		ImGui::SeparatorText("Sprite");
		ImGui::PushID(entt::type_hash<SpriteComponent>::value());
		if (ImGui::TreeNodeEx("##SpriteTree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::AddSpaces(2);

			ImGui::InlineLabel("texture: ");
			ImGui::TextColored(ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f }, sprite.texture_name.c_str());

			std::string layer{ "" };

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
			auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.texture_name);
			if (!texture)
			{
				F_ERROR("Texture is not valid");
				return;
			}

			GenerateUVs(sprite, texture->GetWidth(), texture->GetHeight());
		}
	}

}