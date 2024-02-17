#pragma once

#include "Physics/Box2DWrappers.h"

#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <entt.hpp>

namespace Feather {

	constexpr float METERS_TO_PIXELS = 12.0f;
	constexpr float PIXELS_TO_METERS = 1.0f / METERS_TO_PIXELS;

	enum class RigidBodyType
	{
		STATIC = 0,
		KINEMATIC,
		DYNAMIC
	};

	struct PhysicsAttributes
	{
		RigidBodyType eType{ RigidBodyType::STATIC };
		float density{ 1.0f }, friction{ 0.2f }, restitution{ 0.2f };
		float restitutionThreshold{ 1.0f }, radius{ 0.0f }, gravityScale{ 1.0f };

		glm::vec2 position{ 0.0f }, scale{ 1.0f }, boxSize{ 0.0f }, offset{ 0.0f };
		bool isCircle{ false }, isBoxShape{ true }, isFixedRotation{ false };

		uint16_t filterCategory{ 0 }, filterMask{ 0 };
		int16_t groupIndex{ 0 };
	};

	class PhysicsComponent
	{
	public:
		PhysicsComponent(PhysicsWorld physicsWorld, const PhysicsAttributes& physicsAttribs);
		~PhysicsComponent() = default;

		void Init(int windowWidth, int windowHeight);
		b2Body* GetBody();

		static void CreatePhysicsLuaBind(sol::state& lua, entt::registry& registry);

	private:
		PhysicsWorld m_PhysicsWorld;
		std::shared_ptr<b2Body> m_RigidBody;

		PhysicsAttributes m_InitialAttributes;
	};

}
