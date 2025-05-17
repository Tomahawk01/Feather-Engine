#include "PlayerStart.h"

#include "Core/Character/Character.h"
#include "Core/CoreUtils/Prefab.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/Scene/Scene.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"

constexpr const char* PlayerStartTag = "F_PlayerStart";

namespace Feather {

	PlayerStart::PlayerStart(Registry& registry, Scene& sceneRef)
		: m_SceneRef{ sceneRef }
		, m_VisualEntity{ registry, PlayerStartTag, "" }
		, m_CharacterPrefab{ nullptr }
		, m_Character { nullptr }
		, m_CharacterName{ "default" }
		, m_CharacterLoaded{ false }
		, m_VisualEntityCreated{ false }
	{
		LoadVisualEntity();
	}

	void PlayerStart::CreatePlayer(Registry& registry)
	{
		if (m_CharacterName != "default" && !m_CharacterPrefab && m_CharacterLoaded)
		{
			if (auto pPrefab = ASSET_MANAGER().GetPrefab(m_CharacterName))
			{
				SetCharacter(*pPrefab);
				auto pNewEntity = PrefabCreator::AddPrefabToScene(*m_CharacterPrefab, registry);
				auto& transform = pNewEntity->GetComponent<TransformComponent>();
				const auto& playerStartTransform = m_VisualEntity.GetComponent<TransformComponent>();
				transform.position = playerStartTransform.position;
			}
			else
			{
				F_ERROR("Failed to create player start character. '{}' prefabbed character does not exist", m_CharacterName);
				return;
			}
		}
		else if (m_CharacterPrefab)
		{
			auto pNewEntity = PrefabCreator::AddPrefabToScene(*m_CharacterPrefab, registry);
			auto& transform = pNewEntity->GetComponent<TransformComponent>();
			const auto& playerStartTransform = m_VisualEntity.GetComponent<TransformComponent>();
			transform.position = playerStartTransform.position;
		}
		else
		{
			Entity characterEnt{ registry, "Player", "" };
			auto& transform = characterEnt.AddComponent<TransformComponent>(m_VisualEntity.GetComponent<TransformComponent>());
			transform.scale = glm::vec2{ 1.0f }; // Should the scale be changed here?

			// This needs to be a default texture in the engine.
			// We should have a couple of different ones based on the type of game we want to make
			// Right now we just have a default player. Will add more later.
			auto& sprite = characterEnt.AddComponent<SpriteComponent>(SpriteComponent{ .textureName = "ZZ_F_default_player", .width = 16, .height = 16, .layer = 6 });

			sprite.isIsometric = m_SceneRef.GetMapType() == EMapType::IsoGrid;

			auto pTexture = ASSET_MANAGER().GetTexture("ZZ_F_default_player");
			F_ASSERT(pTexture && "The default player texture must exist");

			GenerateUVs(sprite, pTexture->GetWidth(), pTexture->GetHeight());

			float radius{ 6.0f };
			characterEnt.AddComponent<CircleColliderComponent>(CircleColliderComponent{ .radius = radius, .offset = glm::vec2{ 2.0f, 2.0f } });

			auto& coreGlobals = CORE_GLOBALS();
			if (coreGlobals.IsPhysicsEnabled())
			{
				// If the game type is not a platformer, set to zero
				float gravityScale = coreGlobals.GetGameType() == GameType::Platformer ? 1.0f : 0.0f;

				characterEnt.AddComponent<PhysicsComponent>(
					PhysicsComponent{ PhysicsAttributes{.eType = RigidBodyType::DYNAMIC,
														 .density = 100.0f,
														 .friction = 0.0f,
														 .restitution = 0.0f,
														 .radius = radius,
														 .gravityScale = gravityScale,
														 .isCircle = true } });
			}

			characterEnt.AddComponent<AnimationComponent>(AnimationComponent{ .numFrames = 4, .frameRate = 10, .isLooped = true });
		}
	}

	std::string PlayerStart::GetCharacterName()
	{
		if (!m_CharacterName.empty() && m_CharacterLoaded)
			return m_CharacterName;

		return std::string{ "default" };
	}

	void PlayerStart::SetCharacter(const Prefab& prefab)
	{
		if (m_CharacterPrefab)
			m_CharacterPrefab.reset();

		m_CharacterPrefab = std::make_shared<Prefab>(prefab);

		if (auto& id = m_CharacterPrefab->GetPrefabbedEntity().id)
		{
			m_CharacterName = id->name;
		}

		m_CharacterLoaded = true;
	}

	glm::vec2 PlayerStart::GetPosition()
	{
		const auto& transform = m_VisualEntity.GetComponent<TransformComponent>();
		return transform.position;
	}

	void PlayerStart::SetPosition(const glm::vec2& position)
	{
		auto* transform = m_VisualEntity.TryGetComponent<TransformComponent>();
		F_ASSERT(transform && "Visual entity was not setup correctly");
		transform->position = position;
	}

	void PlayerStart::Load(const std::string& prefabName)
	{
		m_CharacterName = prefabName;
		m_CharacterLoaded = true;
	}

	void PlayerStart::Unload()
	{
		m_VisualEntity.GetEntity() = entt::null;
		m_VisualEntityCreated = false;

		// m_Character.reset( );
		// m_CharacterPrefab.reset( );
		// m_CharacterLoaded = false;
		// m_CharacterName.clear();
	}

	void PlayerStart::LoadVisualEntity()
	{
		if (m_VisualEntityCreated)
		{
			F_ERROR("Failed to load visual entity. Already created");
			return;
		}

		if (m_VisualEntity.GetEntity() == entt::null)
		{
			m_VisualEntity = Entity{ m_SceneRef.GetRegistry(), PlayerStartTag, "" };
		}

		m_VisualEntity.AddComponent<TransformComponent>(TransformComponent{});
		m_VisualEntity.AddComponent<UneditableComponent>(UneditableComponent{ .type = EUneditableType::PlayerStart });
		auto& sprite = m_VisualEntity.AddComponent<SpriteComponent>(SpriteComponent{ .textureName = "ZZ_F_PlayerStart", .width = 64, .height = 64, .layer = 999999 });

		auto pTexture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.textureName);
		F_ASSERT(pTexture && "ZZ_F_PlayerStart texture must be loaded into the asset manager!");

		GenerateUVs(sprite, pTexture->GetWidth(), pTexture->GetHeight());

		m_VisualEntityCreated = true;
	}

}
