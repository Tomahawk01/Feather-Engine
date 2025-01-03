#include "CoreEngineData.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Registry.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/Scripting/UserDataBindings.h"

namespace Feather {

	constexpr float METERS_TO_PIXELS = 12.0f;
	constexpr float PIXELS_TO_METERS = 1.0f / METERS_TO_PIXELS;

	void CoreEngineData::RegisterMetaFunctions()
	{
		Entity::RegisterMetaComponent<Identification>();
		Entity::RegisterMetaComponent<TransformComponent>();
		Entity::RegisterMetaComponent<SpriteComponent>();
		Entity::RegisterMetaComponent<AnimationComponent>();
		Entity::RegisterMetaComponent<BoxColliderComponent>();
		Entity::RegisterMetaComponent<CircleColliderComponent>();
		Entity::RegisterMetaComponent<PhysicsComponent>();
		Entity::RegisterMetaComponent<RigidBodyComponent>();
		Entity::RegisterMetaComponent<TextComponent>();
		Entity::RegisterMetaComponent<TileComponent>();

		Registry::RegisterMetaComponent<Identification>();
		Registry::RegisterMetaComponent<TransformComponent>();
		Registry::RegisterMetaComponent<SpriteComponent>();
		Registry::RegisterMetaComponent<AnimationComponent>();
		Registry::RegisterMetaComponent<BoxColliderComponent>();
		Registry::RegisterMetaComponent<CircleColliderComponent>();
		Registry::RegisterMetaComponent<PhysicsComponent>();
		Registry::RegisterMetaComponent<RigidBodyComponent>();
		Registry::RegisterMetaComponent<TextComponent>();
		Registry::RegisterMetaComponent<TileComponent>();

		// Register user data types
		UserDataBinder::register_user_meta_data<ObjectData>();
	}

	CoreEngineData::CoreEngineData()
		: m_DeltaTime{ 0.0f }, m_ScaledWidth{ 0.0f }, m_ScaledHeight{ 0.0f }, m_Gravity{ 9.8f },
		m_WindowWidth{ 640 }, m_WindowHeight{ 480 },
		m_VelocityIterations{ 10 }, m_PositionIterations{ 8 },
		m_PhysicsEnabled{ true }, m_PhysicsPaused{ false }, m_RenderColliders{ false }
	{
		m_ScaledWidth = m_WindowWidth / METERS_TO_PIXELS;
		m_ScaledHeight = m_WindowHeight / METERS_TO_PIXELS;
	}

	CoreEngineData& CoreEngineData::GetInstance()
	{
		static CoreEngineData instance{};
		return instance;
	}

	void CoreEngineData::UpdateDeltaTime()
	{
		auto now = std::chrono::steady_clock::now();
		m_DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_LastUpdate).count() / 1000000.0f;
		m_LastUpdate = now;
	}

	void CoreEngineData::SetWindowWidth(int windowWidth)
	{
		m_WindowWidth = windowWidth;
		m_ScaledWidth = m_WindowWidth / METERS_TO_PIXELS;
	}

	void CoreEngineData::SetWindowHeight(int windowHeight)
	{
		m_WindowHeight = windowHeight;
		m_ScaledHeight = m_WindowHeight / METERS_TO_PIXELS;
	}

	const float CoreEngineData::MetersToPixels() const
	{
		return METERS_TO_PIXELS;
	}

	const float CoreEngineData::PixelsToMeters() const
	{
		return PIXELS_TO_METERS;
	}

}
