#include "PhysicsComponent.h"

#include "Logger/Logger.h"

namespace Feather {

	PhysicsComponent::PhysicsComponent(PhysicsWorld physicsWorld, const PhysicsAttributes& physicsAttribs)
		: m_PhysicsWorld{ physicsWorld }, m_RigidBody{ nullptr }, m_InitialAttributes{ physicsAttribs }
	{}

	void PhysicsComponent::Init(int windowWidth, int windowHeight)
	{
		if (!m_PhysicsWorld)
		{
			F_FATAL("Failed to initialize physics component: Physics world is nullptr!");
			return;
		}

		bool isCircle{ m_InitialAttributes.isCircle };

		b2BodyDef bodyDef{};
		bodyDef.type = static_cast<b2BodyType>(m_InitialAttributes.eType);
		bodyDef.position.Set(
			(m_InitialAttributes.position.x + m_InitialAttributes.offset.x - (windowWidth * 0.5f) + ((isCircle ? m_InitialAttributes.radius : m_InitialAttributes.boxSize.x) * m_InitialAttributes.scale.x) * 0.5f) * PIXELS_TO_METERS,
			(m_InitialAttributes.position.y + m_InitialAttributes.offset.y - (windowHeight * 0.5f) + ((isCircle ? m_InitialAttributes.radius : m_InitialAttributes.boxSize.y) * m_InitialAttributes.scale.y) * 0.5f) * PIXELS_TO_METERS
		);
		bodyDef.gravityScale = m_InitialAttributes.gravityScale;
		bodyDef.fixedRotation = m_InitialAttributes.isFixedRotation;

		m_RigidBody = MakeSharedBody(m_PhysicsWorld->CreateBody(&bodyDef));
		if (!m_RigidBody)
		{
			F_ERROR("Failed to create RigidBody");
			return;
		}

		b2PolygonShape polyShape;
		b2CircleShape circleShape;
		if (isCircle)
		{
			circleShape.m_radius = m_InitialAttributes.radius;
		}
		else if (m_InitialAttributes.isBoxShape)
		{
			polyShape.SetAsBox(
				PIXELS_TO_METERS * m_InitialAttributes.boxSize.x * m_InitialAttributes.scale.x * 0.5f,
				PIXELS_TO_METERS * m_InitialAttributes.boxSize.y * m_InitialAttributes.scale.y * 0.5f
			);
		}
		else
		{
			// TODO: Polygon shape
		}

		b2FixtureDef fixtureDef{};
		if (isCircle)
			fixtureDef.shape = &circleShape;
		else
			fixtureDef.shape = &polyShape;

		fixtureDef.density = m_InitialAttributes.density;
		fixtureDef.friction = m_InitialAttributes.friction;
		fixtureDef.restitution = m_InitialAttributes.restitution;
		fixtureDef.restitutionThreshold = m_InitialAttributes.restitutionThreshold;

		auto fixture = m_RigidBody->CreateFixture(&fixtureDef);
		if (!fixture)
		{
			F_ERROR("Failed to create the RigidBody fixture!");
		}
	}

	b2Body* PhysicsComponent::GetBody()
	{
		return m_RigidBody.get();
	}

	void PhysicsComponent::CreatePhysicsLuaBind(sol::state& lua, entt::registry& registry)
	{
		// TODO: Create lua bindings
	}

}
