#include "DrawComponentUtils.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/Events/EventDispatcher.h"
#include "Physics/PhysicsUtilities.h"
#include "Utils/FeatherUtilities.h"

#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Events/EditorEventTypes.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"

#include <map>

static constexpr std::string GetPhysicsBodyDescription(Feather::RigidBodyType type)
{
	switch (type)
	{
	case Feather::RigidBodyType::STATIC: return "Zero mass, zero velocity, may be manually moved";
	case Feather::RigidBodyType::KINEMATIC: return "Zero mass, velocity set by user, moved by solver";
	case Feather::RigidBodyType::DYNAMIC: return "Positive mass, velocity determined by forces, moved by solver";
	default: return "";
	}
}

namespace Feather {

	void DrawComponentsUtil::DrawImGuiComponent(TransformComponent& transform)
	{
		ImGui::SeparatorText("Transform");
		ImGui::PushID(entt::type_hash<TransformComponent>::value());
		if (ImGui::TreeNodeEx("##TransformTree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushItemWidth(120.0f);

			ImGui::InlineLabel("position");
			ImGui::ColoredLabel("x" "##pos_x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			ImGui::InputFloat("##position_x", &transform.position.x, 1.0f, 10.0f, "%.2f");
			ImGui::SameLine();
			ImGui::ColoredLabel("y" "##pos_y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			ImGui::InputFloat("##position_y", &transform.position.y, 1.0f, 10.0f, "%.2f");

			ImGui::InlineLabel("scale" "##scl_x");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputFloat("##scale_x", &transform.scale.x, 0.1f, 1.0f, "%.2f"))
			{
				transform.scale.x = glm::clamp(transform.scale.x, 0.1f, 500.0f);
				transform.isDirty = true;
			}
			ImGui::SameLine();
			ImGui::ColoredLabel("y" "##scl_y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputFloat("##scale_y", &transform.scale.y, 0.1f, 1.0f, "%.2f"))
			{
				transform.scale.y = glm::clamp(transform.scale.y, 0.1f, 500.0f);
				transform.isDirty = true;
			}

			ImGui::InlineLabel("rotation");
			if (ImGui::InputFloat("##rotation", &transform.rotation, 1.0f, 2.0f, "%.2f"))
			{
				transform.isDirty = true;
			}

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
						sprite.textureName = textureStr;
				}
				ImGui::EndDragDropTarget();
			}

			// Color picker
			ImVec4 color = { sprite.color.r / 255.0f, sprite.color.g / 255.0f, sprite.color.b / 255.0f, sprite.color.a / 255.0f };
			ImGui::InlineLabel("color");
			ImGui::ItemToolTip("Sprite color override");
			if (ImGui::ColorEdit4("##color", &color.x, IMGUI_COLOR_PICKER_FLAGS))
			{
				sprite.color.r = static_cast<GLubyte>(color.x * 255.0f);
				sprite.color.g = static_cast<GLubyte>(color.y * 255.0f);
				sprite.color.b = static_cast<GLubyte>(color.z * 255.0f);
				sprite.color.a = static_cast<GLubyte>(color.w * 255.0f);
			}

			auto& assetManager = MAIN_REGISTRY().GetAssetManager();
			std::string selectedTexture{ sprite.textureName };
			ImGui::InlineLabel("texture");
			ImGui::ItemToolTip("The current active texture of the sprite to be drawn");
			if (ImGui::BeginCombo("##texture", selectedTexture.c_str()))
			{
				for (const std::string& textureName : assetManager.GetAssetKeyNames(AssetType::TEXTURE))
				{
					if (ImGui::Selectable(textureName.c_str(), textureName == selectedTexture))
					{
						selectedTexture = textureName;
						sprite.textureName = selectedTexture;
						IsChanged = true;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::AddSpaces(2);

			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("width");
			ImGui::ItemToolTip("The width of the sprite");
			if (ImGui::InputFloat("##width", &sprite.width, 1.0f, 8.0f))
			{
				sprite.width = glm::clamp(sprite.width, 8.0f, 2000.0f);
				IsChanged = true;
			}
			ImGui::InlineLabel("height");
			ImGui::ItemToolTip("The height of the sprite");
			if (ImGui::InputFloat("##height", &sprite.height, 1.0f, 8.0f))
			{
				sprite.height = glm::clamp(sprite.height, 8.0f, 2000.0f);
				IsChanged = true;
			}

			ImGui::InlineLabel("layer");
			ImGui::ItemToolTip("Z-Index in which to draw the sprite");
			if (ImGui::InputInt("##layer", &sprite.layer, 1.0f, 1.0f))
			{
				sprite.layer = glm::clamp(sprite.layer, 0, 255);
			}

			ImGui::InlineLabel("start pos");
			ImGui::ItemToolTip("The index positions where we want to start our UV calculations");
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

			ImGui::InlineLabel("Iso Sorting");
			ImGui::Checkbox("##isoSorting", &sprite.isIsometric);
			ImGui::ItemToolTip("If the scene is Isometric, the sprite should use iso sorting.");

			ImGui::TreePop();
			ImGui::PopItemWidth();
		}

		ImGui::PopID();

		if (IsChanged)
		{
			auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.textureName);
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
			ImGui::ItemToolTip("Does the sprite animations scroll vertically");
			ImGui::Checkbox("##vertical", &animation.isVertical);
			ImGui::InlineLabel("looped");
			ImGui::ItemToolTip("Are the sprite animatons to be looped");
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
			ImGui::ItemToolTip("The offset of the box collider from the origin. Origin is the Top Left corner");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputFloat("##offset_x", &boxCollider.offset.x, 1.0f, 4.0f))
				boxCollider.offset.x = std::clamp(boxCollider.offset.x, -256.0f, 256.0f);
			ImGui::SameLine();
			ImGui::ColoredLabel("y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputFloat("##offset_y", &boxCollider.offset.y, 1.0f, 4.0f))
				boxCollider.offset.y = std::clamp(boxCollider.offset.y, -256.0f, 256.0f);
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
			ImGui::ItemToolTip("The radius of the circle component. Clamped to [4 - 2000]");
			if (ImGui::InputFloat("##radius", &circleCollider.radius, 4, 4))
				circleCollider.radius = std::clamp(circleCollider.radius, 4.0f, 2000.0f);

			ImGui::InlineLabel("offset");
			ImGui::ItemToolTip("The offset of the circle collider from the origin. Origin is the Top Left corner");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputFloat("##offset_x", &circleCollider.offset.x, 1.0f, 4.0f))
				circleCollider.offset.x = std::clamp(circleCollider.offset.x, 0.0f, 128.0f);
			ImGui::SameLine();
			ImGui::ColoredLabel("y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputFloat("##offset_y", &circleCollider.offset.y, 1.0f, 4.0f))
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
			ImGui::AddSpaces(2);

			PhysicsAttributes& physicsAttributes = physics.GetChangableAttributes();
			std::string selectedBodyType{ GetRigidBodyTypeString(physicsAttributes.eType) };
			ImGui::InlineLabel("body type");
			ImGui::ItemToolTip("The body type: static, kinematic, or dynamic");
			if (ImGui::BeginCombo("##body_type", selectedBodyType.c_str()))
			{
				for (const auto& [bodyType, bodyStr] : GetRigidBodyStringMap())
				{
					if (ImGui::Selectable(bodyStr.c_str(), bodyStr == selectedBodyType))
					{
						selectedBodyType = bodyStr;
						physicsAttributes.eType = bodyType;
					}

					ImGui::ItemToolTip("{}", GetPhysicsBodyDescription(bodyType));
				}

				ImGui::EndCombo();
			}

			std::string selectedCategoryType{ GetFilterCategoryString(static_cast<FilterCategory>(physicsAttributes.filterCategory)) };
			ImGui::InlineLabel("category type");
			ImGui::ItemToolTip("The collision category bits. Bodies will usually only use one category type");
			if (ImGui::BeginCombo("##category_type", selectedCategoryType.c_str()))
			{
				for (const auto& [categoryType, categoryStr] : GetFilterCategoryToStringMap())
				{
					if (ImGui::Selectable(categoryStr.c_str(), categoryStr == selectedCategoryType))
					{
						selectedCategoryType = categoryStr;
						physicsAttributes.filterCategory = static_cast<uint16_t>(categoryType);
					}
				}

				ImGui::EndCombo();
			}

			ImGui::Separator();
			ImGui::AddSpaces(2);

			std::string selectedMaskBit{ "" };
			FilterCategory maskCategory{ FilterCategory::NO_CATEGORY };

			ImGui::InlineLabel("masks");
			ImGui::ItemToolTip("The collision mask bits. This states the categories that this shape would accept for collision");
			if (ImGui::BeginCombo("##mask_bits", selectedMaskBit.c_str()))
			{
				for (const auto& [categoryType, categoryStr] : GetFilterCategoryToStringMap())
				{
					if (ImGui::Selectable(categoryStr.c_str(), categoryStr == selectedMaskBit))
					{
						selectedMaskBit = categoryStr;
						maskCategory = categoryType;
					}
				}

				if (!selectedMaskBit.empty() && maskCategory != FilterCategory::NO_CATEGORY)
				{
					uint16_t filterCat = static_cast<uint16_t>(maskCategory);
					if (!(IsBitSet(physicsAttributes.filterMask, filterCat)))
					{
						physicsAttributes.filterMask += filterCat;
						maskCategory = FilterCategory::NO_CATEGORY;
						selectedMaskBit.clear();
					}
					else
					{
						F_ERROR("Masks already contain '{}'", selectedMaskBit);
					}
				}

				ImGui::EndCombo();
			}

			if (physicsAttributes.filterMask > 0)
			{
				ImGui::InlineLabel("applied masks");
				if (ImGui::BeginListBox("##applied_masks", ImVec2{ 0.0f, 32.0f }))
				{
					auto setMaskBits{ GetAllSetBits(physicsAttributes.filterMask) };
					for (auto mask : setMaskBits)
					{
						ImGui::Selectable(GetFilterCategoryString(static_cast<FilterCategory>(Bit(mask))).c_str());
					}

					ImGui::EndListBox();
				}

				ImGui::SetCursorPosX(128.0f);
				if (ImGui::Button("clear masks"))
				{
					physicsAttributes.filterMask = 0;
					maskCategory = FilterCategory::NO_CATEGORY;
					selectedMaskBit.clear();
				}
			}

			ImGui::AddSpaces(2);
			ImGui::Separator();
			ImGui::AddSpaces(2);

			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("density");
			ImGui::ItemToolTip("The density, usually in kg/m^2");
			ImGui::InputFloat("##density", &physicsAttributes.density, 1.0f, 1.0f, "%.1f");

			ImGui::InlineLabel("friction");
			ImGui::ItemToolTip("The Coulomb friction coefficient, usually in the range [0,1]");
			if (ImGui::InputFloat("##friction", &physicsAttributes.friction, 0.1f, 0.1f, "%.1f"))
				physicsAttributes.friction = std::clamp(physicsAttributes.friction, 0.0f, 1.0f);

			ImGui::InlineLabel("restitution");
			ImGui::ItemToolTip("The restitution (bounce) usually in the range [0,1]");
			if (ImGui::InputFloat("##restitution", &physicsAttributes.restitution, 0.1f, 0.1f, "%.1f"))
				physicsAttributes.restitution = std::clamp(physicsAttributes.restitution, 0.0f, 1.0f);

			ImGui::InlineLabel("gravityScale");
			ImGui::ItemToolTip("Scale of gravity applied to this body");
			ImGui::InputFloat("##gravityScale", &physicsAttributes.gravityScale, 1.0f, 1.0f, "%.1f");

			ImGui::AddSpaces(2);
			ImGui::Separator();
			ImGui::AddSpaces(2);

			ImGui::InlineLabel("is a box?");
			ImGui::ItemToolTip("If the entity is a box, the box collider is used in construction for the size");
			if (ImGui::Checkbox("##boxShape", &physicsAttributes.isBoxShape))
			{
				if (physicsAttributes.isCircle)
					physicsAttributes.isCircle = false;
			}

			if (physicsAttributes.isBoxShape && !physicsAttributes.isCircle)
			{
				ImGui::SameLine(0, 32.0f);
				ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Must have a Box Collider Component");
			}

			ImGui::InlineLabel("is a circle?");
			ImGui::ItemToolTip("If the entity is a circle, the circle collider is used in construction for the size");
			if (ImGui::Checkbox("##circle", &physicsAttributes.isCircle))
			{
				if (physicsAttributes.isBoxShape)
					physicsAttributes.isBoxShape = false;
			}

			if (!physicsAttributes.isBoxShape && physicsAttributes.isCircle)
			{
				ImGui::SameLine(0, 32.0f);
				ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Must have a Circle Collider Component");
			}

			ImGui::AddSpaces(2);
			ImGui::Separator();

			ImGui::InlineLabel("fixed rotation");
			ImGui::ItemToolTip("Should the body be prevented from rotating?");
			ImGui::Checkbox("##fixedRotation", &physicsAttributes.isFixedRotation);

			ImGui::InlineLabel("sensor");
			ImGui::ItemToolTip("A sensor shape generates overlap events but never generates a collision response.\n"
				"Sensors do not collide with other sensors and do not have continuous collision");
			ImGui::Checkbox("##sensor", &physicsAttributes.isTrigger);

			ImGui::InlineLabel("bullet");
			ImGui::ItemToolTip("Treat this body as high speed object that performs continuous collision detection against "
								"dynamic and kinematic bodies, but not other bullet bodies.\n"
								"Warning - Bullets should be used sparingly. They are not a solution for general "
								"dynamic-versus-dynamic continuous collision. They may interfere with joint constraints.");
			ImGui::Checkbox("##bullet", &physicsAttributes.isBullet);

			ImGui::SeparatorText("Physics Object Data");
			ImGui::AddSpaces(2);

			ImGui::PopItemWidth();
			auto& objectData = physicsAttributes.objectData;

			std::string tagBuffer{ objectData.tag };
			ImGui::InlineLabel("tag");
			if (ImGui::InputText("##_tag", tagBuffer.data(), sizeof(char) * 255, ImGuiInputTextFlags_EnterReturnsTrue))
				objectData.tag = std::string{ tagBuffer.data() };

			std::string groupBuffer{ objectData.group };
			ImGui::InlineLabel("group");
			if (ImGui::InputText("##_group", groupBuffer.data(), sizeof(char) * 255, ImGuiInputTextFlags_EnterReturnsTrue))
				objectData.group = std::string{ groupBuffer.data() };

			ImGui::InlineLabel("is collider?");
			ImGui::Checkbox("##objectDataCollider", &objectData.isCollider);

			ImGui::InlineLabel("is trigger?");
			ImGui::Checkbox("##objectDataTrigger", &objectData.isTrigger);

			ImGui::InlineLabel("is friendly?");
			ImGui::Checkbox("##objectDataFriendly", &objectData.isFriendly);

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
			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("max velocity");
			ImGui::ColoredLabel("x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			ImGui::InputFloat("##maxVelocity_x", &rigidBody.maxVelocity.x, 1.0f, 10.0f, "%.2f");
			ImGui::SameLine();
			ImGui::ColoredLabel("y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			ImGui::InputFloat("##maxVelocity_y", &rigidBody.maxVelocity.y, 1.0f, 10.0f, "%.2f");

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
			if (ImGui::InputText("##_textStr", sTextBuffer.data(), sizeof(char) * 1024, 0/*ImGuiInputTextFlags_EnterReturnsTrue*/))
			{
				text.textStr = std::string{ sTextBuffer.data() };
				text.isDirty = true;
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
						text.isDirty = true;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel("padding");
			if (ImGui::InputInt("##padding", &text.padding, 0, 0))
			{
				text.isDirty = true;
			}

			ImGui::InlineLabel("wrap");
			if (ImGui::InputFloat("##textWrap", &text.wrap, 0.0f, 0.0f))
			{
				text.isDirty = true;
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

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, TransformComponent& transform)
	{
		ImGui::SeparatorText("Transform");
		ImGui::PushID(entt::type_hash<TransformComponent>::value());
		if (ImGui::TreeNodeEx("##TransformTree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto& relations = entity.GetComponent<Relationship>();
			bool hasParent{ relations.parent != entt::null };
			bool positionChanged{ false };

			ImGui::PushItemWidth(120.0f);
			ImGui::InlineLabel(hasParent ? "relative pos" : "position");
			ImGui::ItemToolTip(hasParent ? "Game object has a parent. This is the relative position based on the parent's position"
										 : "World or absolute position of the game object");

			ImGui::ColoredLabel("x##pos_x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputFloat(
				"##position_x", hasParent ? &transform.localPosition.x : &transform.position.x, 1.0f, 10.0f, "%.1f"))
			{
				positionChanged = true;
			}

			ImGui::SameLine();
			ImGui::ColoredLabel("y##pos_y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputFloat(
				"##position_y", hasParent ? &transform.localPosition.y : &transform.position.y, 1.0f, 10.0f, "%.1f"))
			{
				positionChanged = true;
			}
			if (positionChanged)
			{
				entity.UpdateTransform();
			}

			ImGui::InlineLabel("scale");
			ImGui::ColoredLabel("x##scl_x", LABEL_SINGLE_SIZE, LABEL_RED);
			ImGui::SameLine();
			if (ImGui::InputFloat("##scale_x", &transform.scale.x, 1.0f, 1.0f, "%.1f"))
			{
				transform.scale.x = std::clamp(transform.scale.x, 0.01f, 150.0f);
				transform.isDirty = true;
			}
			ImGui::SameLine();
			ImGui::ColoredLabel("y##scl_y", LABEL_SINGLE_SIZE, LABEL_GREEN);
			ImGui::SameLine();
			if (ImGui::InputFloat("##scale_y", &transform.scale.y, 1.0f, 1.0f, "%.1f"))
			{
				transform.scale.y = std::clamp(transform.scale.y, 0.01f, 150.0f);
				transform.isDirty = true;
			}

			ImGui::InlineLabel("rotation");
			if (ImGui::InputFloat("##rotation", &transform.rotation, 1.0f, 1.0f, "%.1f"))
			{
				transform.isDirty = true;
			}

			ImGui::PopItemWidth();
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, SpriteComponent& sprite)
	{
		DrawImGuiComponent(sprite);
	}

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, AnimationComponent& animation)
	{
		DrawImGuiComponent(animation);
	}

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, BoxColliderComponent& boxCollider)
	{
		DrawImGuiComponent(boxCollider);
	}

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, CircleColliderComponent& circleCollider)
	{
		DrawImGuiComponent(circleCollider);
	}

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, PhysicsComponent& physics)
	{
		DrawImGuiComponent(physics);
	}

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, RigidBodyComponent& rigidbody)
	{
		DrawImGuiComponent(rigidbody);
	}

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, TextComponent& textComponent)
	{
		DrawImGuiComponent(textComponent);
	}

	void DrawComponentsUtil::DrawImGuiComponent(Entity& entity, Identification& identification)
	{
		ImGui::SeparatorText("Identificaton");
		ImGui::PushID(entt::type_hash<Identification>::value());
		if (ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::string error{ "" };
			std::string nameBuffer{ identification.name };
			bool nameError{ false };

			ImGui::InlineLabel("name");
			if (ImGui::InputText("##_name", nameBuffer.data(), sizeof(char) * 255, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string sBufferStr{ nameBuffer.data() };
				if (!sBufferStr.empty() && !SCENE_MANAGER().CheckTagName(sBufferStr))
				{
					std::string sOldName{ identification.name };
					identification.name = std::string{ nameBuffer.data() };
					EVENT_DISPATCHER().EmitEvent(NameChangeEvent{ .oldName = sOldName, .newName = identification.name, .entity = &entity });
				}
			}

			// We need to display an error if the name is already in the scene or empty.
			// Entity names need to be unique
			if (ImGui::IsItemActive())
			{
				std::string sBufferStr{ nameBuffer.data() };
				if (sBufferStr.empty())
				{
					error = "Name cannot be empty";
				}
				else if (SCENE_MANAGER().CheckTagName(sBufferStr))
				{
					error = std::format("{} already exists!", sBufferStr);
				}

				if (!error.empty())
				{
					ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, error.c_str());
				}
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
