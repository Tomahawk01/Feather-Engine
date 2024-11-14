#include "PhysicsSystem.h"

#include "Logger/Logger.h"
#include "Core/ECS/Components/BoxColliderComponent.h"
#include "Core/ECS/Components/CircleColliderComponent.h"
#include "Core/ECS/Components/TransformComponent.h"
#include "Core/ECS/Components/PhysicsComponent.h"
#include "Core/ECS/Registry.h"
#include "Core/CoreUtils/CoreEngineData.h"

namespace Feather {

	PhysicsSystem::PhysicsSystem()
	{}

	void PhysicsSystem::Update(Registry& registry)
	{
		auto boxView = registry.GetRegistry().view<PhysicsComponent, TransformComponent, BoxColliderComponent>();
		auto& coreEngine = CoreEngineData::GetInstance();

		float scaledWidth = coreEngine.ScaledWidth() * 0.5f;
		float scaledHeight = coreEngine.ScaledHeight() * 0.5f;

		const float M2P = coreEngine.MetersToPixels();

		for (auto entity : boxView)
		{
			auto& physics = boxView.get<PhysicsComponent>(entity);
			auto RigidBody = physics.GetBody();
			if (!RigidBody)
				continue;

			// If the entity is static, we can skip it
			if (RigidBody->GetType() == b2BodyType::b2_staticBody)
				continue;

			auto& transform = boxView.get<TransformComponent>(entity);
			auto& boxCollider = boxView.get<BoxColliderComponent>(entity);

			const auto& bodyPosition = RigidBody->GetPosition();

			transform.position.x = (scaledWidth + bodyPosition.x) * M2P - (boxCollider.width * transform.scale.x) / 2.0f - boxCollider.offset.x;
			transform.position.y = (scaledHeight + bodyPosition.y) * M2P - (boxCollider.height * transform.scale.y) / 2.0f - boxCollider.offset.y;
			if (!RigidBody->IsFixedRotation())
				transform.rotation = glm::degrees(RigidBody->GetAngle());
		}

		auto circleView = registry.GetRegistry().view<PhysicsComponent, TransformComponent, CircleColliderComponent>();
		for (auto entity : circleView)
		{
			auto& physics = circleView.get<PhysicsComponent>(entity);
			auto RigidBody = physics.GetBody();
			if (!RigidBody)
				continue;

			// If the entity is static, we can skip it
			if (RigidBody->GetType() == b2BodyType::b2_staticBody)
				continue;

			auto& transform = circleView.get<TransformComponent>(entity);
			auto& circleCollider = circleView.get<CircleColliderComponent>(entity);

			const auto& bodyPosition = RigidBody->GetPosition();

			transform.position.x = (scaledWidth + bodyPosition.x) * M2P - (circleCollider.radius * transform.scale.x) - circleCollider.offset.x;
			transform.position.y = (scaledHeight + bodyPosition.y) * M2P - (circleCollider.radius * transform.scale.y) - circleCollider.offset.y;
			if (!RigidBody->IsFixedRotation())
				transform.rotation = glm::degrees(RigidBody->GetAngle());
		}
	}

}
