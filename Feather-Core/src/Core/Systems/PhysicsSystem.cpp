#include "PhysicsSystem.h"

#include "Logger/Logger.h"
#include "Core/ECS/Components/BoxColliderComponent.h"
#include "Core/ECS/Components/CircleColliderComponent.h"
#include "Core/ECS/Components/TransformComponent.h"
#include "Core/ECS/Components/PhysicsComponent.h"

namespace Feather {

	PhysicsSystem::PhysicsSystem(Registry& registry)
		: m_Registry{ registry }
	{}

	void PhysicsSystem::Update(entt::registry& registry)
	{
		auto boxView = registry.view<PhysicsComponent, TransformComponent, BoxColliderComponent>();
		auto scaledWidth = 640.0f / METERS_TO_PIXELS;
		auto scaledHeight = 480.0f / METERS_TO_PIXELS;

		for (auto entity : boxView)
		{
			auto& physics = boxView.get<PhysicsComponent>(entity);
			auto RigidBody = physics.GetBody();
			if (!RigidBody)
				continue;

			auto& transform = boxView.get<TransformComponent>(entity);
			auto& boxCollider = boxView.get<BoxColliderComponent>(entity);

			const auto& bodyPosition = RigidBody->GetPosition();

			transform.position.x = ((scaledWidth / 2.0f) + bodyPosition.x) * METERS_TO_PIXELS - (boxCollider.width * transform.scale.x) / 2.0f - boxCollider.offset.x;
			transform.position.y = ((scaledHeight / 2.0f) + bodyPosition.y) * METERS_TO_PIXELS - (boxCollider.height * transform.scale.y) / 2.0f - boxCollider.offset.y;
			if (!RigidBody->IsFixedRotation())
				transform.rotation = glm::degrees(RigidBody->GetAngle());
		}

		auto circleView = registry.view<PhysicsComponent, TransformComponent, CircleColliderComponent>();
		for (auto entity : circleView)
		{
			auto& physics = circleView.get<PhysicsComponent>(entity);
			auto RigidBody = physics.GetBody();
			if (!RigidBody)
				continue;

			auto& transform = circleView.get<TransformComponent>(entity);
			auto& circleCollider = circleView.get<CircleColliderComponent>(entity);

			const auto& bodyPosition = RigidBody->GetPosition();

			transform.position.x = ((scaledWidth / 2.0f) + bodyPosition.x) * METERS_TO_PIXELS - (circleCollider.radius * transform.scale.x) - circleCollider.offset.x;
			transform.position.y = ((scaledHeight / 2.0f) + bodyPosition.y) * METERS_TO_PIXELS - (circleCollider.radius * transform.scale.y) - circleCollider.offset.y;
			if (!RigidBody->IsFixedRotation())
				transform.rotation = glm::degrees(RigidBody->GetAngle());
		}
	}

}
