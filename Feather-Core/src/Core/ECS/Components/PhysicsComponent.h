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
		float density{ 1.0f };
		float friction{ 0.2f };
		float restitution{ 0.2f };
		float restitutionThreshold{ 1.0f };
		float radius{ 0.0f };
		float gravityScale{ 1.0f };

		glm::vec2 position{ 0.0f };
		glm::vec2 scale{ 1.0f };
		glm::vec2 boxSize{ 0.0f };
		glm::vec2 offset{ 0.0f };

		bool isCircle{ false };
		bool isBoxShape{ true };
		bool isFixedRotation{ true };
		bool isTrigger{ false };
		bool isBullet{ false };
		bool useFilters{ false };

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
		const bool IsTrigger() const;

		ObjectData CastRay(const b2Vec2& point1, const b2Vec2& point2) const;
		std::vector<ObjectData> BoxTrace(const b2Vec2& lowerBounds, const b2Vec2& upperBounds) const;
		ObjectData GetCurrentObjectData();

		void SetFilterCategory(uint16_t category);
		void SetFilterCategory();
		void SetFilterMask(uint16_t mask);
		void SetFilterMask();
		void SetGroupIndex(int index);
		void SetGroupIndex();

		bool UseFilters() const { return m_InitialAttributes.useFilters; }

		inline b2Body* GetBody() { return m_RigidBody.get(); }
		inline UserData* GetUserData() { return m_UserData.get(); }

		inline const PhysicsAttributes& GetAttributes() const { return m_InitialAttributes; }
		inline PhysicsAttributes& GetChangableAttributes() { return m_InitialAttributes; }

		static void CreatePhysicsLuaBind(sol::state& lua, entt::registry& registry);

	private:
		std::shared_ptr<b2Body> m_RigidBody;
		std::shared_ptr<UserData> m_UserData;

		PhysicsAttributes m_InitialAttributes;
	};

}
