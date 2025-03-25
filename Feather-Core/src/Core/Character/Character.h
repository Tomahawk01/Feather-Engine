#pragma once

#include "Core/ECS/Entity.h"

#include <optional>

namespace Feather {

	struct PhysicsAttributes;
	struct TransformComponent;
	struct SpriteComponent;
	struct AnimationComponent;
	struct BoxColliderComponent;
	struct CircleColliderComponent;
	class StateMachine;

	struct CharacterParams
	{
		std::string name{ "" };
		std::string group{ "" };

		std::optional<std::unique_ptr<AnimationComponent>> animation{ std::nullopt };
		std::optional<std::unique_ptr<SpriteComponent>> sprite{ std::nullopt };
		std::optional<std::unique_ptr<BoxColliderComponent>> boxCollider{ std::nullopt };
		std::optional<std::unique_ptr<CircleColliderComponent>> circleCollider{ std::nullopt };
		std::optional<std::unique_ptr<PhysicsAttributes>> physicsParams{ std::nullopt };
	};

	class Character : public Entity
	{
	public:
		Character(Registry& registry, const CharacterParams& params);
		Character(Registry& registry, entt::entity entity);
		Character(const Entity& entity);

		StateMachine& GetStateMachine();

		TransformComponent& GetTransformComponent();
		SpriteComponent& GetSpriteComponent();
		AnimationComponent& GetAnimationComponent();
		
		static void CreateCharacterLuaBind(sol::state& lua, Registry& registry);

	protected:
		std::shared_ptr<StateMachine> m_StateMachine;
	};

}
