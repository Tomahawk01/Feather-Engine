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
			ImGui::SameLine();
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
		ImGui::SeparatorText("Animation Component");
		ImGui::PushID(entt::type_hash<AnimationComponent>::value());
		if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("num frames");
			if (ImGui::InputInt("##1", &animation.numFrames, 1, 1))
				animation.numFrames = std::clamp(animation.numFrames, 1, 15);

			ImGui::InlineLabel("frame rate");
			if (ImGui::InputInt("##frame_rate", &animation.frameRate, 1, 1))
				animation.frameRate = std::clamp(animation.frameRate, 1, 25);

			ImGui::InlineLabel("frame offset");
			if (ImGui::InputInt("##frame_offset", &animation.frameOffset, 1, 1))
				animation.frameOffset = std::clamp(animation.frameOffset, 0, 15);

			ImGui::InlineLabel("vertical");
			ImGui::Checkbox("##vertical", &animation.isVertical);
			ImGui::InlineLabel("looped");
			ImGui::Checkbox("##looped", &animation.isLooped);
			ImGui::TreePop();
			ImGui::PopItemWidth();
		}
		ImGui::PopID();
	}

	void DrawComponentsUtil::DrawImGuiComponent(BoxColliderComponent& boxCollider)
	{
		ImGui::SeparatorText("Box Collider Component");
		ImGui::PushID(entt::type_hash<BoxColliderComponent>::value());
		if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("width");
			if (ImGui::InputInt("##width", &boxCollider.width, 4, 4))
				boxCollider.width = std::clamp(boxCollider.width, 4, 2000);

			ImGui::InlineLabel("height");
			if (ImGui::InputInt("##height", &boxCollider.height, 4, 4))
				boxCollider.height = std::clamp(boxCollider.height, 4, 2000);

			ImGui::InlineLabel("offset");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputFloat("##offset_x", &boxCollider.offset.x, 4.0f, 4.0f))
				boxCollider.offset.x = std::clamp(boxCollider.offset.x, 0.0f, 128.0f);
			ImGui::SameLine();
			ImGui::ColoredLabel("y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputFloat("##offset_y", &boxCollider.offset.y, 4.0f, 4.0f))
				boxCollider.offset.y = std::clamp(boxCollider.offset.y, 0.0f, 128.0f);
			ImGui::TreePop();
			ImGui::PopItemWidth();
		}
		ImGui::PopID();
	}

	void DrawComponentsUtil::DrawImGuiComponent(CircleColliderComponent& circleCollider)
	{
		ImGui::SeparatorText("Circle Collider Component");
		ImGui::PushID(entt::type_hash<CircleColliderComponent>::value());
		if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("radius");
			if (ImGui::InputFloat("##radius", &circleCollider.radius, 4, 4))
				circleCollider.radius = std::clamp(circleCollider.radius, 4.0f, 2000.0f);

			ImGui::InlineLabel("offset");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputFloat("##offset_x", &circleCollider.offset.x, 4.0f, 4.0f))
				circleCollider.offset.x = std::clamp(circleCollider.offset.x, 0.0f, 128.0f);
			ImGui::SameLine();
			ImGui::ColoredLabel("y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputFloat("##offset_y", &circleCollider.offset.y, 4.0f, 4.0f))
				circleCollider.offset.y = std::clamp(circleCollider.offset.y, 0.0f, 128.0f);
			ImGui::TreePop();
			ImGui::PopItemWidth();
		}
		ImGui::PopID();
	}

	void DrawComponentsUtil::DrawImGuiComponent(PhysicsComponent& physics)
	{
		ImGui::SeparatorText("Physics Component");
		ImGui::PushID(entt::type_hash<PhysicsComponent>::value());
		if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// TODO: Add thing
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	void DrawComponentsUtil::DrawImGuiComponent(RigidBodyComponent& rigidBody)
	{
		ImGui::SeparatorText("Rigidbody Component");
		ImGui::PushID(entt::type_hash<RigidBodyComponent>::value());
		if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// TODO: Add thing
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	void DrawComponentsUtil::DrawImGuiComponent(TextComponent& text)
	{
		ImGui::SeparatorText("Text Component");
		ImGui::PushID(entt::type_hash<TextComponent>::value());
		if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::string sTextBuffer{ text.textStr };
			ImGui::InlineLabel("text");
			if (ImGui::InputText("##_textStr", sTextBuffer.data(), sizeof(char) * 255, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				text.textStr = std::string{ sTextBuffer.data() };
			}

			std::string fontName{ text.fontName };
			ImGui::PushItemWidth(164.0f);
			ImGui::InlineLabel("font");
			if (ImGui::BeginCombo("##fontName", fontName.c_str()))
			{
				auto& assetManager = MAIN_REGISTRY().GetAssetManager();
				for (const auto& font : assetManager.GetAssetKeyNames(AssetType::FONT))
				{
					if (ImGui::Selectable(font.c_str(), font == fontName))
					{
						fontName = font;
						text.fontName = fontName;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("padding");
			if (ImGui::InputInt("##padding", &text.padding, 0, 0))
			{
				// TODO: Add padding
			}

			ImGui::InlineLabel("wrap");
			if (ImGui::InputFloat("##textWrap", &text.wrap, 0.0f, 0.0f))
			{
				// TODO: Add textWrap
			}

			ImGui::PopItemWidth();
			ImGui::PopItemWidth();
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	void DrawComponentsUtil::DrawImGuiComponent(Identification& identification)
	{
		ImGui::SeparatorText("Identificaton");
		ImGui::PushID(entt::type_hash<Identification>::value());
		if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::string sNameBuffer{ identification.name };
			ImGui::InlineLabel("name");
			if (ImGui::InputText("##_name", sNameBuffer.data(), sizeof(char) * 255, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				identification.name = std::string{ sNameBuffer.data() };
			}

			std::string sGroupBuffer{ identification.group };
			ImGui::InlineLabel("group");
			if (ImGui::InputText("##_group", sGroupBuffer.data(), sizeof(char) * 255, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				identification.group = std::string{ sGroupBuffer.data() };
			}

			ImGui::TreePop();
		}
		ImGui::PopID();
	}

}
