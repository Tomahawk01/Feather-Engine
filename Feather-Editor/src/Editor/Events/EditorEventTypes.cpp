#include "EditorEventTypes.h"

#include <map>

namespace Feather {

	static std::map<ComponentType, std::string> g_mapComponentToStr
	{
		{ ComponentType::Transform, "TransformComponent" },
		{ ComponentType::Sprite, "SpriteComponent" },
		{ ComponentType::Animation, "AnimationComponent" },
		{ ComponentType::Text, "TextComponent" },
		{ ComponentType::UI, "UIComponent" },
		{ ComponentType::Physics, "PhysicsComponent" },
		{ ComponentType::RigidBody, "RigidBodyComponent" },
		{ ComponentType::BoxCollider, "BoxColliderComponent" },
		{ ComponentType::CircleCollider, "CircleColliderComponent" },
		{ ComponentType::Tile, "TileComponent" }
	};

	ComponentType GetComponentTypeFromStr(const std::string& componentStr)
	{
		auto itr = std::ranges::find_if(g_mapComponentToStr, [&](const auto& pair) { return pair.second == componentStr; });
		if (itr == g_mapComponentToStr.end())
			return ComponentType::NoType;

		return itr->first;
	}

	std::string GetComponentStrFromType(ComponentType type)
	{
		auto itr = g_mapComponentToStr.find(type);
		if (itr == g_mapComponentToStr.end())
			return {};

		return itr->second;
	}

}
