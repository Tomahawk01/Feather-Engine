#include "PhysicsComponent.h"

#include "Logger/Logger.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Physics/RayCastCallback.h"
#include "Physics/BoxTraceCallback.h"

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
			// TODO: Add the ability to create various convex polygon shapes based on the number of vertices and their position.
			// Currently hardcoded to a 4 vertice shape, square, rect, diamond, etc.
			auto halfWidth = PIXELS_TO_METERS * m_InitialAttributes.boxSize.x * m_InitialAttributes.scale.x * 0.5f;
			auto halfHeight = PIXELS_TO_METERS * m_InitialAttributes.boxSize.y * m_InitialAttributes.scale.y * 0.5f;

			b2Vec2 vertices[4];

			vertices[0].Set(0.0f, 0.0f);
			vertices[1].Set(halfWidth, -halfHeight);
			vertices[2].Set(-halfWidth, -halfHeight);
			vertices[3].Set(0.0f, -halfHeight * 2.0f);

			polyShape.Set(vertices, 4);
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

	ObjectData PhysicsComponent::CastRay(const b2Vec2& point1, const b2Vec2& point2) const
	{
		if (!m_RigidBody)
			return {};

		// Get the world
		auto* pWorld = m_RigidBody->GetWorld();
		if (!pWorld)
			return {};

		RayCastCallback callback{};

		auto& coreGlobals = CORE_GLOBALS();
		const auto& M2P = coreGlobals.MetersToPixels();
		const auto& P2M = coreGlobals.PixelsToMeters();

		const auto& scaledWidth = coreGlobals.ScaledWidth();
		const auto& scaledHeight = coreGlobals.ScaledHeight();

		auto ax = (point1.x / M2P) - scaledWidth * 0.5f;
		auto ay = (point1.y / M2P) - scaledHeight * 0.5f;

		auto bx = (point2.x / M2P) - scaledWidth * 0.5f;
		auto by = (point2.y / M2P) - scaledHeight * 0.5f;

		pWorld->RayCast(&callback, b2Vec2{ ax, ay }, b2Vec2{ bx, by });

		if (callback.IsHit())
		{
			auto& userData = callback.HitFixture()->GetUserData();
			if (UserData* pData = reinterpret_cast<UserData*>(userData.pointer))
			{
				try
				{
					auto objectData = std::any_cast<ObjectData>(pData->userData);
					return objectData;
				}
				catch (const std::bad_any_cast& ex)
				{
					F_ERROR("Failed to cast to object data. Error: {}", ex.what());
				}
			}
		}

		return ObjectData{};
	}

	std::vector<ObjectData> PhysicsComponent::BoxTrace(const b2Vec2& lowerBounds, const b2Vec2& upperBounds) const
	{
		if (!m_RigidBody)
		{
			return {};
		}

		// Get the world
		auto* pWorld = m_RigidBody->GetWorld();
		if (!pWorld)
			return {};

		std::vector<ObjectData> objectDataVec{};

		BoxTraceCallback callback{};

		auto& coreGlobals = CORE_GLOBALS();
		const auto& M2P = coreGlobals.MetersToPixels();
		const auto& P2M = coreGlobals.PixelsToMeters();

		const auto& scaledWidth = coreGlobals.ScaledWidth();
		const auto& scaledHeight = coreGlobals.ScaledHeight();

		b2AABB aabb{};
		aabb.lowerBound =
			b2Vec2{ (lowerBounds.x / M2P) - scaledWidth * 0.5f, (lowerBounds.y / M2P) - scaledHeight * 0.5f };

		aabb.upperBound =
			b2Vec2{ (upperBounds.x / M2P) - scaledWidth * 0.5f, (upperBounds.y / M2P) - scaledHeight * 0.5f };

		pWorld->QueryAABB(&callback, aabb);

		const auto& hitBodies = callback.GetBodies();
		if (hitBodies.empty())
			return objectDataVec;

		for (const auto pBody : hitBodies)
		{
			auto& userData = pBody->GetFixtureList()->GetUserData();
			UserData* pData = reinterpret_cast<UserData*>(userData.pointer);

			try
			{
				auto objectData = std::any_cast<ObjectData>(pData->userData);
				objectDataVec.push_back(objectData);
			}
			catch (const std::bad_any_cast& e)
			{
				F_ERROR("Failed to cast to object data: " + std::string{ e.what() });
			}
		}

		return objectDataVec;
	}

	ObjectData PhysicsComponent::GetCurrentObjectData()
	{
		F_ASSERT(m_RigidBody);

		if (!m_RigidBody)
			return {};

		auto& userData = m_RigidBody->GetFixtureList()->GetUserData();

		UserData* data = reinterpret_cast<UserData*>(userData.pointer);

		try
		{
			auto objectData = std::any_cast<ObjectData>(data->userData);
			return objectData;
		}
		catch (const std::bad_any_cast& e)
		{
			F_ERROR("Failed to cast to object data: {}", e.what());
		}

		return {};
	}

	void PhysicsComponent::SetFilterCategory(uint16_t category)
	{
		if (!m_RigidBody)
		{
			m_InitialAttributes.filterCategory = category;
			return;
		}

		auto fixtureList = m_RigidBody->GetFixtureList();
		if (!fixtureList)
		{
			return;
		}

		b2Filter copyFilter{ fixtureList->GetFilterData() };
		copyFilter.categoryBits = category;
		fixtureList->SetFilterData(copyFilter);
		m_InitialAttributes.filterCategory = category;
	}

	void PhysicsComponent::SetFilterCategory()
	{
		SetFilterCategory(m_InitialAttributes.filterCategory);
	}

	void PhysicsComponent::SetFilterMask(uint16_t mask)
	{
		if (!m_RigidBody)
		{
			m_InitialAttributes.filterMask = mask;
			return;
		}

		auto fixtureList = m_RigidBody->GetFixtureList();
		if (!fixtureList)
		{
			return;
		}

		b2Filter copyFilter{ fixtureList->GetFilterData() };
		copyFilter.maskBits = mask;
		fixtureList->SetFilterData(copyFilter);
		m_InitialAttributes.filterMask = mask;
	}

	void PhysicsComponent::SetFilterMask()
	{
		SetFilterMask(m_InitialAttributes.filterMask);
	}

	void PhysicsComponent::SetGroupIndex(int index)
	{
		if (!m_RigidBody)
		{
			m_InitialAttributes.groupIndex = index;
			return;
		}

		auto pFixtureList = m_RigidBody->GetFixtureList();
		if (!pFixtureList)
		{
			return;
		}

		auto copyFilter = pFixtureList->GetFilterData();
		copyFilter.groupIndex = index;
		pFixtureList->SetFilterData(copyFilter);
		m_InitialAttributes.groupIndex = index;
	}

	void PhysicsComponent::SetGroupIndex()
	{
		SetGroupIndex(m_InitialAttributes.groupIndex);
	}

	void PhysicsComponent::CreatePhysicsLuaBind(sol::state& lua, entt::registry& registry)
	{
		lua.new_usertype<ObjectData>(
			"ObjectData",
			"type_id", entt::type_hash<ObjectData>::value,
			sol::call_constructor,
			sol::factories(
				[](const std::string& tag, const std::string& group, bool isCollider, bool isTrigger, bool isFriendly, std::uint32_t entityID)
				{
					return ObjectData{ tag, group, isCollider, isTrigger, isFriendly, entityID };
				},
				[](const sol::table& objectData)
				{
					return ObjectData{
						objectData["tag"].get_or(std::string{}),
						objectData["group"].get_or(std::string{}),
						objectData["isCollider"].get_or(false),
						objectData["isTrigger"].get_or(false),
						objectData["isFriendly"].get_or(false),
						objectData["entityID"].get_or((std::uint32_t)0)
					};
				}
			),
			"tag", &ObjectData::tag,
			"group", &ObjectData::group,
			"isCollider", &ObjectData::isCollider,
			"isTrigger", &ObjectData::isTrigger,
			"isFriendly", &ObjectData::isFriendly,
			"entityID", &ObjectData::entityID,
			"contactEntities", sol::readonly_property([](ObjectData& objData) { return objData.GetContactEntities(); }),
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
				[](const sol::table& physAttr)
				{
					return PhysicsAttributes{
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
							physAttr["objectData"]["tag"].get_or(std::string{}),
							physAttr["objectData"]["group"].get_or(std::string{}),
							physAttr["objectData"]["isCollider"].get_or(false),
							physAttr["objectData"]["isTrigger"].get_or(false),
							physAttr["objectData"]["isFriendly"].get_or(false),
							physAttr["objectData"]["entityID"].get_or((std::uint32_t)0)
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
		F_ASSERT(physicsWorld && "Physics World was not setup properly");

		if (!physicsWorld)
		{
			F_ERROR("Failed to create Physics Component Lua bind. Physics World was not set properly");
			return;
		}

		auto& coreGlobals = CORE_GLOBALS();

		lua.new_usertype<PhysicsComponent>(
			"PhysicsComponent",
			"type_id", &entt::type_hash<PhysicsComponent>::value,
			sol::call_constructor,
			sol::factories(
				[&](const PhysicsAttributes& attrs)
				{
					PhysicsComponent pc{ attrs };
					pc.Init(physicsWorld, coreGlobals.WindowWidth(), coreGlobals.WindowHeight());
					return pc;
				}
			),
			"linearImpulse", [](PhysicsComponent& pc, const glm::vec2& impulse)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->ApplyLinearImpulse(b2Vec2{ impulse.x, impulse.y }, body->GetPosition(), true);
			},
			"angularImpulse", [](PhysicsComponent& pc, float impulse)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->ApplyAngularImpulse(impulse, true);
			},
			"setLinearVelocity", [](PhysicsComponent& pc, const glm::vec2& velocity)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->SetLinearVelocity(b2Vec2{ velocity.x, velocity.y });
			},
			"getLinearVelocity", [](PhysicsComponent& pc)
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
			"setAngularVelocity", [](PhysicsComponent& pc, float angularVelocity)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->SetAngularVelocity(angularVelocity);
			},
			"getAngularVelocity", [](PhysicsComponent& pc)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return 0.0f;
				}

				return body->GetAngularVelocity();
			},
			"applyForceToCenter", [](PhysicsComponent& pc, const glm::vec2& force)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->ApplyForceToCenter(b2Vec2{ force.x, force.y }, true);
			},
			"setGravityScale", [](PhysicsComponent& pc, float gravityScale)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->SetGravityScale(gravityScale);
			},
			"getGravityScale", [](PhysicsComponent& pc)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return 0.0f;
				}

				return body->GetGravityScale();
			},
			"setTransform", [](PhysicsComponent& pc, const glm::vec2& position)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				auto& engineData = CoreEngineData::GetInstance();
				const auto p2m = engineData.PixelsToMeters();

				const auto scaleHalfHeight = engineData.ScaledHeight() * 0.5f;
				const auto scaleHalfWidth = engineData.ScaledWidth() * 0.5f;

				auto bx = position.x * p2m - scaleHalfWidth;
				auto by = position.y * p2m - scaleHalfHeight;

				body->SetTransform(b2Vec2{ bx, by }, 0.0f);
			},
			"getTransform", [](const PhysicsComponent& pc)
			{

			},
			"setBodyType", [&](PhysicsComponent& pc, RigidBodyType type)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				b2BodyType bodyType = b2_dynamicBody;

				switch (type)
				{
				case RigidBodyType::STATIC:
					bodyType = b2_staticBody;
					break;
				case RigidBodyType::KINEMATIC:
					bodyType = b2_kinematicBody;
					break;
				case RigidBodyType::DYNAMIC:
					bodyType = b2_dynamicBody;
					break;
				default:
					break;
				}

				body->SetType(bodyType);
			},
			"setAsBullet", [&](PhysicsComponent& pc, bool bullet)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return;
				}

				body->SetBullet(bullet);
			},
			"isBullet", [&](PhysicsComponent& pc)
			{
				auto body = pc.GetBody();
				if (!body)
				{
					// TODO: error
					return false;
				}

				return body->IsBullet();
			},
			"setFilterCategory",
			[](PhysicsComponent& pc)
			{
				auto body = pc.GetBody();
				if (!body)
					return;
			},
			"castRay",
			[](PhysicsComponent& pc, const glm::vec2& p1, const glm::vec2& p2, sol::this_state s)
			{
				auto objectData = pc.CastRay(b2Vec2{ p1.x, p1.y }, b2Vec2{ p2.x, p2.y });
				return objectData.entityID == entt::null ? sol::lua_nil_t{} : sol::make_object(s, objectData);
			},
			"boxTrace",
			[](PhysicsComponent& pc, const glm::vec2& lowerBounds, const glm::vec2& upperBounds, sol::this_state s)
			{
				auto vecObjectData = pc.BoxTrace(b2Vec2{ lowerBounds.x, lowerBounds.y }, b2Vec2{ upperBounds.x, upperBounds.y });
				return vecObjectData.empty() ? sol::lua_nil_t{} : sol::make_object(s, vecObjectData);
			},
			"objectData",
			[](PhysicsComponent& pc)
			{
				return pc.GetCurrentObjectData();
			}
		);
	}

}
