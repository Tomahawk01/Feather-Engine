#include "DrawComponentUtils.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Utils/FeatherUtilities.h"
#include "Logger/Logger.h"

#include "ImGuiUtils.h"

namespace Feather {

	void DrawComponentsUtil::DrawImGuiComponent(TransformComponent& transform)
	{
		ImGui::SeparatorText("Transform");
		ImGui::PushID(entt::type_hash<TransformComponent>::value());
		if (ImGui::TreeNodeEx("##TransformTree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushItemWidth(120.0f);

			ImGui::InlineLabel("position");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			ImGui::InputFloat("##position_x", &transform.position.x, 1.0f, 10.0f, "%.2f");
			ImGui::SameLine();
			ImGui::ColoredLabel("y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			ImGui::InputFloat("##position_y", &transform.position.y, 1.0f, 10.0f, "%.2f");

			ImGui::InlineLabel("scale");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputFloat("##scale_x", &transform.scale.x, 0.1f, 1.0f, "%.2f"))
				transform.scale.x = glm::clamp(transform.scale.x, 0.1f, 500.0f);
			ImGui::SameLine();
			ImGui::ColoredLabel("y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputFloat("##scale_y", &transform.scale.y, 0.1f, 1.0f, "%.2f"))
				transform.scale.y = glm::clamp(transform.scale.y, 0.1f, 500.0f);

			ImGui::InlineLabel("rotation");
			ImGui::InputFloat("##rotation", &transform.rotation, 1.0f, 2.0f, "%.2f");

			ImGui::PopItemWidth();
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void DrawComponentsUtil::DrawImGuiComponent(SpriteComponent& sprite)
	{
		bool IsChanged{ false };

		ImGui::SeparatorText("Sprite");
		ImGui::PushID(entt::type_hash<SpriteComponent>::value());
		if (ImGui::TreeNodeEx("##SpriteTree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::AddSpaces(2);
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetDisplayTexture");
				if (payload)
				{
					const char* texture = (const char*)payload->Data;
					std::string textureStr{ texture };
					F_ASSERT(!textureStr.empty() && "Texture name is empty!");
					if (!textureStr.empty())
						sprite.texture_name = textureStr;
				}
				ImGui::EndDragDropTarget();
			}

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

			auto& assetManager = MAIN_REGISTRY().GetAssetManager();
			std::string selectedTexture{ sprite.texture_name };
			ImGui::InlineLabel("texture");
			if (ImGui::BeginCombo("##texture", selectedTexture.c_str()))
			{
				for (const std::string& textureName : assetManager.GetAssetKeyNames(AssetType::TEXTURE))
				{
					if (ImGui::Selectable(textureName.c_str(), textureName == selectedTexture))
					{
						selectedTexture = textureName;
						sprite.texture_name = selectedTexture;
						IsChanged = true;
					}
				}
				ImGui::EndCombo();
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

			ImGui::InlineLabel("layer");
			if (ImGui::InputInt("##layer", &sprite.layer, 1.0f, 1.0f))
			{
				sprite.layer = glm::clamp(sprite.layer, 0, 99);
			}

			ImGui::InlineLabel("Sprite Sheet Position");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputInt("##start_x", &sprite.start_x, 1, 1))
			{
				sprite.start_x = glm::clamp(sprite.start_x, 0, 32); // TODO: Get texure and divide by width to get maxvalue
				IsChanged = true;
			}
			ImGui::ColoredLabel("y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputInt("##start_y", &sprite.start_y, 1, 1))
			{
				sprite.start_y = glm::clamp(sprite.start_y, 0, 32); // TODO: Get texure and divide by width to get maxvalue
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

	void DrawComponentsUtil::DrawImGuiComponent(AnimationComponent& animation)
	{
	}

	void DrawComponentsUtil::DrawImGuiComponent(BoxColliderComponent& boxCollider)
	{
	}

	void DrawComponentsUtil::DrawImGuiComponent(CircleColliderComponent& cirlceCollider)
	{
	}

	void DrawComponentsUtil::DrawImGuiComponent(PhysicsComponent& physics)
	{
	}

	void DrawComponentsUtil::DrawImGuiComponent(RigidBodyComponent& rigidBody)
	{
	}

	void DrawComponentsUtil::DrawImGuiComponent(TextComponent& text)
	{
	}

	void DrawComponentsUtil::DrawImGuiComponent(Identification& identification)
	{
	}

}
