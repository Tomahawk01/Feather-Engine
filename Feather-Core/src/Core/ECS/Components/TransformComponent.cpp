#include "TransformComponent.h"

std::string Feather::TransformComponent::to_string()
{
	std::stringstream ss;
	ss << "==== Transform Component ==== \n" <<
		  "Position: x = " << position.x << ", y = " << position.y << "\n" <<
		  "Scale: x = " << scale.x << ", y = " << scale.y << "\n" <<
		  "Rotation: " << rotation << "\n";

	return ss.str();
}

void Feather::TransformComponent::CreateLuaTransformBind(sol::state& lua)
{
	lua.new_usertype<TransformComponent>(
		"Transform",
		"type_id", &entt::type_hash<TransformComponent>::value,
		sol::call_constructor,
		sol::factories(
			[](glm::vec2 position, glm::vec2 scale, float rotation)
			{
				return TransformComponent{
					.position = position,
					.scale = scale,
					.rotation = rotation };
			},
			[](float x, float y, float scale_x, float scale_y, float rotation)
			{
				return TransformComponent{
					.position = glm::vec2{x, y},
					.scale = glm::vec2{scale_x, scale_y},
					.rotation = rotation };
			}
		),
		"position", &TransformComponent::position,
		"localPosition", &TransformComponent::localPosition,
		"localRotation", &TransformComponent::localRotation,
		"scale", &TransformComponent::scale,
		"rotation", &TransformComponent::rotation,
		"setScale", // Should be used rather than directly accessing member
		[](TransformComponent& transform, const glm::vec2& scale)
		{
			transform.scale = scale;
			transform.isDirty = true;
		},
		"setRotation", // Should be used rather than directly accessing member
		[](TransformComponent& transform, const float rotation)
		{
			transform.rotation = rotation;
			transform.isDirty = true;
		},
		"to_string", &TransformComponent::to_string
	);
}
