#pragma once

#include "BoxColliderComponent.h"
#include "CircleColliderComponent.h"
#include "AnimationComponent.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "PhysicsComponent.h"
#include "RigidBodyComponent.h"
#include "ScriptComponent.h"
#include "Identification.h"
#include "TextComponent.h"
#include "TileComponent.h"
#include "Relationship.h"
#include "UIComponent.h"

namespace Feather {

	enum class EUneditableType
	{
		PlayerStart,
		/* TODO: add more uneditable types as needed */
		NoType
	};
	/*
	 * UneditableComponent
	 */

	struct UneditableComponent
	{
		EUneditableType type{ EUneditableType::PlayerStart };
	};

}
