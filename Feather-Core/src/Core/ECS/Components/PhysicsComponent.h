#pragma once

#include "Physics/Box2DWrappers.h"
#include "Physics/UserData.h"
#include "Physics/PhysicsUtilities.h"

#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <entt.hpp>

namespace Feather {

	struct PhysicsAttributes
	{
		RigidBodyType eType{ RigidBodyType::STATIC };
		float density{ 1.0f }, friction{ 0.2f }, restitution{ 0.2f };
		float restitutionThreshold{ 1.0f }, radius{ 0.0f }, gravityScale{ 1.0f };

		glm::vec2 position{ 0.0f }, scale{ 1.0f }, boxSize{ 0.0f }, offset{ 0.0f };
		bool isCircle{ false }, isBoxShape{ true }, isFixedRotation{ true }, isTrigger{ false };

		bool isBullet{ false };

		// Used to filter collisions on shapes
		uint16_t filterCategory{ 0 };
		uint16_t filterMask{ 0 };
		int16_t groupIndex{ 0 };

		// User specified data for each body
		ObjectData objectData{};
	};

	class PhysicsComponent
	{
	public:
		PhysicsComponent();
		PhysicsComponent(const PhysicsAttributes& physicsAttrs);
		~PhysicsComponent() = default;

		void Init(PhysicsWorld physicsWorld, int windowWidth, int windowHeight);
		b2Body* GetBody() { return m_RigidBody.get(); }
		UserData* GetUserData() { return m_UserData.get(); }

		const bool IsTrigger() const;
		const PhysicsAttributes& GetAttributes() const { return m_InitialAttributes; }
		PhysicsAttributes& GetChangableAttributes() { return m_InitialAttributes; }
		static void CreatePhysicsLuaBind(sol::state& lua, entt::registry& registry);

	private:
		std::shared_ptr<b2Body> m_RigidBody;
		std::shared_ptr<UserData> m_UserData;

		PhysicsAttributes m_InitialAttributes;
	};

}
