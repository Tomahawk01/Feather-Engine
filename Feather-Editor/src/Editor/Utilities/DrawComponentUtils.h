#pragma once
#include "Core/ECS/Components/AllComponents.h"

namespace Feather {

	class Entity;

	class DrawComponentsUtil
	{
	public:
		DrawComponentsUtil() = delete;
		
		template<typename TComponent>
		static void DrawEntityComponentInfo(Entity& entity);

		template<typename TComponent>
		static void DrawComponentInfo(TComponent& component);

		template<typename TComponent>
		static void RegisterUIComponent();

	private:
		static void DrawImGuiComponent(TransformComponent& transform);
		static void DrawImGuiComponent(SpriteComponent& sprite);
		static void DrawImGuiComponent(AnimationComponent& animation);
		static void DrawImGuiComponent(BoxColliderComponent& boxCollider);
		static void DrawImGuiComponent(CircleColliderComponent& circleCollider);
		static void DrawImGuiComponent(PhysicsComponent& physics);
		static void DrawImGuiComponent(RigidBodyComponent& rigidBody);
		static void DrawImGuiComponent(TextComponent& text);
		static void DrawImGuiComponent(Identification& identification);
	};

}

#include "DrawComponentUtils.inl"
