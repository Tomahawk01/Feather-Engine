#include "CoreEngineData.h"

#include "Core/ECS/Entity.h"
#include "Core/ECS/Registry.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/Scripting/UserDataBindings.h"

namespace Feather {

	constexpr float METERS_TO_PIXELS = 12.0f;
	constexpr float PIXELS_TO_METERS = 1.0f / METERS_TO_PIXELS;

	static std::unordered_map<GameType, std::string> g_mapGameTypeToStr
	{
		{ GameType::TopDown,		"Top Down" },
		{ GameType::Platformer,		"Platformer" },
		{ GameType::Fighting,		"Fighting" },
		{ GameType::Puzzle,			"Puzzle" },
		{ GameType::Rougelike,		"Rougelike" },
		{ GameType::NoType,			"No Type" }
	};

	static std::unordered_map<std::string, GameType> g_mapStrToGameType
	{
		{ "Top Down",				GameType::TopDown },
		{ "Platformer" ,			GameType::Platformer},
		{ "Fighting",				GameType::Fighting },
		{ "Puzzle",					GameType::Puzzle},
		{ "Rougelike",				GameType::Rougelike },
		{ "No Type",				GameType::NoType }
	};

	std::string CoreEngineData::GetGameTypeStr(GameType eType)
	{
		if (auto itr = g_mapGameTypeToStr.find(eType); itr != g_mapGameTypeToStr.end())
		{
			return itr->second;
		}

		return {};
	}

	GameType CoreEngineData::GetGameTypeFromStr(const std::string& sType)
	{
		if (auto itr = g_mapStrToGameType.find(sType); itr != g_mapStrToGameType.end())
		{
			return itr->second;
		}

		return GameType::NoType;
	}

	const std::unordered_map<GameType, std::string>& CoreEngineData::GetGameTypesMap()
	{
		return g_mapGameTypeToStr;
	}

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
		Entity::RegisterMetaComponent<Relationship>();
		Entity::RegisterMetaComponent<UIComponent>();

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
		Registry::RegisterMetaComponent<Relationship>();
		Registry::RegisterMetaComponent<UIComponent>();

		// Register user data types
		UserDataBinder::register_user_meta_data<ObjectData>();
	}

	CoreEngineData::CoreEngineData()
		: m_DeltaTime{ 0.0f }
		, m_ScaledWidth{ 0.0f }
		, m_ScaledHeight{ 0.0f }
		, m_Gravity{ 9.8f }
		, m_WindowWidth{ 640 }
		, m_WindowHeight{ 480 }
		, m_VelocityIterations{ 10 }
		, m_PositionIterations{ 8 }
		, m_PhysicsEnabled{ true }
		, m_PhysicsPaused{ false }
		, m_RenderColliders{ false }
		, m_RenderAnimations{ false }
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
		m_DeltaTime = std::chrono::duration<double>(now - m_LastUpdate).count();
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

	void CoreEngineData::SetScaledWidth(float newWidth)
	{
		// TODO: Add some clamps
		m_ScaledWidth = newWidth / METERS_TO_PIXELS;
	}

	void CoreEngineData::SetScaledHeight(float newHeight)
	{
		// TODO: Add some clamps
		m_ScaledHeight = newHeight / METERS_TO_PIXELS;
	}

}
