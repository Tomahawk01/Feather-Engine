#include "PhysicsComponent.h"

#include "Logger/Logger.h"
#include "Core/CoreUtils/CoreEngineData.h"

namespace Feather {

	PhysicsComponent::PhysicsComponent()
		: PhysicsComponent(PhysicsAttributes{})
	{}

	PhysicsComponent::PhysicsComponent(const PhysicsAttributes& physicsAttrs)
		: m_RigidBody{ nullptr }, m_UserData{ nullptr }, m_InitialAttributes { physicsAttrs }
	{}

	void PhysicsComponent::Init(PhysicsWorld physicsWorld, int windowWidth, int windowHeight)
	{
		if (!physicsWorld)
		{
			F_FATAL("Failed to initialize physics component: Physics world is nullptr!");
			return;
		}

		bool isCircle{ m_InitialAttributes.isCircle };

		auto PIXELS_TO_METERS = CoreEngineData::GetInstance().PixelsToMeters();

		b2BodyDef bodyDef{};
		bodyDef.type = static_cast<b2BodyType>(m_InitialAttributes.eType);
		bodyDef.position.Set(
			(m_InitialAttributes.position.x + m_InitialAttributes.offset.x - (windowWidth * 0.5f) + ((isCircle ? m_InitialAttributes.radius : m_InitialAttributes.boxSize.x) * m_InitialAttributes.scale.x) * 0.5f) * PIXELS_TO_METERS,
			(m_InitialAttributes.position.y + m_InitialAttributes.offset.y - (windowHeight * 0.5f) + ((isCircle ? m_InitialAttributes.radius : m_InitialAttributes.boxSize.y) * m_InitialAttributes.scale.y) * 0.5f) * PIXELS_TO_METERS
		);
		bodyDef.gravityScale = m_InitialAttributes.gravityScale;
		bodyDef.fixedRotation = m_InitialAttributes.isFixedRotation;

		m_RigidBody = MakeSharedBody(physicsWorld->CreateBody(&bodyDef));
		if (!m_RigidBody)
		{
			F_ERROR("Failed to create RigidBody");
			return;
		}

		b2PolygonShape polyShape;
		b2CircleShape circleShape;
		if (isCircle)
		{
			circleShape.m_radius = PIXELS_TO_METERS * m_InitialAttributes.radius * m_InitialAttributes.scale.x;
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

		// Create user data
		m_UserData = std::make_shared<UserData>();
		m_UserData->userData = m_InitialAttributes.objectData;
		m_UserData->type_id = entt::type_hash<ObjectData>::value();

		// Create fixture definition
		b2FixtureDef fixtureDef{};
		if (isCircle)
			fixtureDef.shape = &circleShape;
		else
			fixtureDef.shape = &polyShape;

		fixtureDef.density = m_InitialAttributes.density;
		fixtureDef.friction = m_InitialAttributes.friction;
		fixtureDef.restitution = m_InitialAttributes.restitution;
		fixtureDef.restitutionThreshold = m_InitialAttributes.restitutionThreshold;
		fixtureDef.isSensor = m_InitialAttributes.isTrigger;
		fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(m_UserData.get());
		
		auto fixture = m_RigidBody->CreateFixture(&fixtureDef);
		if (!fixture)
		{
			F_ERROR("Failed to create the RigidBody fixture!");
		}
	}

	const bool PhysicsComponent::IsTrigger() const
	{
		if (!m_RigidBody)
			return false;

		return m_RigidBody->GetFixtureList()->IsSensor();
	}

	void PhysicsComponent::CreatePhysicsLuaBind(sol::state& lua, entt::registry& registry)
	{
		lua.new_usertype<ObjectData>(
			"ObjectData",
			"type_id", entt::type_hash<ObjectData>::value,
			sol::call_constructor,
			sol::factories(
				[](const std::string& tag, const std::string& group, bool isCollider, bool isTrigger, std::uint32_t entityID)
				{
					return ObjectData {
						.tag = tag,
						.group = group,
						.isCollider = isCollider,
						.isTrigger = isTrigger,
						.entityID = entityID
					};
				},
				[](const sol::table& objectData)
				{
					return ObjectData{
						.tag = objectData["tag"].get_or(std::string{""}),
						.group = objectData["group"].get_or(std::string{""}),
						.isCollider = objectData["isCollider"].get_or(false),
						.isTrigger = objectData["isTrigger"].get_or(false),
						.entityID = objectData["entityID"].get_or((std::uint32_t)0)
					};
				}
			),
			"to_string", &ObjectData::to_string
		);

		lua.new_enum<RigidBodyType>(
			"BodyType", {
				{ "Static", RigidBodyType::STATIC },
				{ "Kinematic", RigidBodyType::KINEMATIC },
				{ "Dynamic", RigidBodyType::DYNAMIC }
			}
		);

		lua.new_usertype<PhysicsAttributes>(
			"PhysicsAttributes",
			sol::call_constructor,
			sol::factories(
				[] { return PhysicsAttributes{}; },
				[](const sol::table& physAttr) {
					return PhysicsAttributes {
						.eType = physAttr["type"].get_or(RigidBodyType::STATIC),
						.density = physAttr["density"].get_or(100.0f),
						.friction = physAttr["friction"].get_or(0.2f),
						.restitution = physAttr["restitution"].get_or(0.2f),
						.restitutionThreshold = physAttr["restitutionThreshold"].get_or(0.2f),
						.radius = physAttr["radius"].get_or(0.0f),
						.gravityScale = physAttr["gravityScale"].get_or(1.0f),
						.position = glm::vec2 {
							physAttr["position"]["x"].get_or(0.0f),
							physAttr["position"]["y"].get_or(0.0f)
						},
						.scale = glm::vec2 {
							physAttr["scale"]["x"].get_or(0.0f),
							physAttr["scale"]["y"].get_or(0.0f)
						},
						.boxSize = glm::vec2 {
							physAttr["boxSize"]["x"].get_or(0.0f),
							physAttr["boxSize"]["y"].get_or(0.0f)
						},
						.offset = glm::vec2 {
							physAttr["offset"]["x"].get_or(0.0f),
							physAttr["offset"]["y"].get_or(0.0f)
						},
						.isCircle = physAttr["isCircle"].get_or(false),
						.isBoxShape = physAttr["isBoxShape"].get_or(true),
						.isFixedRotation = physAttr["isFixedRotation"].get_or(true),
						.isTrigger = physAttr["isTrigger"].get_or(false),
						.filterCategory = physAttr["filterCategory"].get_or((uint16_t)0),
						.filterMask = physAttr["filterMask"].get_or((uint16_t)0),
						.objectData = ObjectData {
							.tag = physAttr["objectData"]["tag"].get_or(std::string{""}),
							.group = physAttr["objectData"]["group"].get_or(std::string{""}),
							.isCollider = physAttr["objectData"]["isCollider"].get_or(false),
							.isTrigger = physAttr["objectData"]["isTrigger"].get_or(false),
							.entityID = physAttr["objectData"]["entityID"].get_or((std::uint32_t)0)
						}
					};
				}
			),
			"type", &PhysicsAttributes::eType,
			"density", &PhysicsAttributes::density,
			"friction", &PhysicsAttributes::friction,
			"restitution", &PhysicsAttributes::restitution,
			"restitutionThreshold", &PhysicsAttributes::restitutionThreshold,
			"radius", &PhysicsAttributes::radius,
			"gravityScale", &PhysicsAttributes::gravityScale,
			"position", &PhysicsAttributes::position,
			"scale", &PhysicsAttributes::scale,
			"boxSize", &PhysicsAttributes::boxSize,
			"offset", &PhysicsAttributes::offset,
			"isCircle", &PhysicsAttributes::isCircle,
			"isBoxShape", &PhysicsAttributes::isBoxShape,
			"isFixedRotation", &PhysicsAttributes::isFixedRotation,
			"isTrigger", &PhysicsAttributes::isTrigger,
			"objectData", &PhysicsAttributes::objectData
			// TODO: Add filters and other properties as needed
		);

		auto& physicsWorld = registry.ctx().get<PhysicsWorld>();
		if (!physicsWorld)
			return;
		
		lua.new_usertype<PhysicsComponent>(
			"PhysicsComponent",
			"type_id", &entt::type_hash<PhysicsComponent>::value,
			sol::call_constructor,
			sol::factories(
				[&](const PhysicsAttributes& attrs)
				{
					PhysicsComponent pc{ attrs };
					pc.Init(physicsWorld, 640, 480); // TODO: Change based on window values
					return pc;
				}
			),
			"linear_impulse", [](PhysicsComponent& pc, const glm::vec2& impulse)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->ApplyLinearImpulse(b2Vec2{ impulse.x, impulse.y }, body->GetPosition(), true);
			},
			"angular_impulse", [](PhysicsComponent& pc, float impulse)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->ApplyAngularImpulse(impulse, true);
			},
			"set_linear_velocity", [](PhysicsComponent& pc, const glm::vec2& velocity)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->SetLinearVelocity(b2Vec2{velocity.x, velocity.y});
			},
			"get_linear_velocity", [](PhysicsComponent& pc)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return glm::vec2{ 0.0f };
				}

				const auto& linearVelocity = body->GetLinearVelocity();
				return glm::vec2{ linearVelocity.x, linearVelocity.y };
			},
			"set_angular_velocity", [](PhysicsComponent& pc, float angularVelocity)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->SetAngularVelocity(angularVelocity);
			},
			"get_angular_velocity", [](PhysicsComponent& pc)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return 0.0f;
				}

				return body->GetAngularVelocity();
			},
			"set_gravity_scale", [](PhysicsComponent& pc, float gravityScale)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->SetGravityScale(gravityScale);
			}
		);
	}

}
