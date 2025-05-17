#pragma once

#include "Core/ECS/Components/AllComponents.h"

namespace Feather {

	class Registry;
	class Entity;

	enum class EPrefabType
	{
		Character,
		GameObject,
		/* TODO: Add more Prefab types */
		InvalidType
	};

	struct PrefabbedEntity;

	/* Prefabbed Relationships basically need to be able to build all of the necessary entities */
	struct PrefabbedRelationships
	{
		/* The first child of the entity */
		PrefabbedEntity* firstChild{ nullptr };
		/* The previous sibling entity in the hierarchy for the parent */
		PrefabbedEntity* prevSibling{ nullptr };
		/* The next sibling entity in the hierarchy for the parent */
		PrefabbedEntity* nextSibling{ nullptr };
		/* The parent entity. Not all entities will have parents */
		PrefabbedEntity* parent{ nullptr };
	};

	struct PrefabbedEntity
	{
		TransformComponent transform{};
		std::optional<AnimationComponent> animation{ std::nullopt };
		std::optional<SpriteComponent> sprite{ std::nullopt };
		std::optional<BoxColliderComponent> boxCollider{ std::nullopt };
		std::optional<CircleColliderComponent> circleCollider{ std::nullopt };
		std::optional<PhysicsComponent> physics{ std::nullopt };
		std::optional<RigidBodyComponent> rigidBody{ std::nullopt };
		std::optional<Identification> id{ std::nullopt };
		std::optional<TextComponent> textComp{ std::nullopt };
		std::optional<UIComponent> uiComp{ std::nullopt };
		std::optional<PrefabbedRelationships> relationships{ std::nullopt };
	};

	/* Prefabs cannot be associated with any registry. They need to be able to create entities when dragged into a level */
	class Prefab
	{
	public:
		Prefab();
		Prefab(EPrefabType eType, const PrefabbedEntity& prefabbed);
		Prefab(const std::string& prefabPath);
		~Prefab();

		bool Load(const std::string& prefabPath);
		bool Save();

		inline const PrefabbedEntity& GetPrefabbedEntity() const { return m_Entity; }
		inline const std::string& GetFilepath() const { return m_PrefabPath; }
		inline EPrefabType GetType() const { return m_Type; }

	private:
		void AddChild(const PrefabbedEntity& child);

	private:
		EPrefabType m_Type;
		PrefabbedEntity m_Entity;
		std::vector<std::shared_ptr<PrefabbedEntity>> m_RelatedPrefabs;
		std::string m_Name;
		std::string m_PrefabPath;

		friend class PrefabCreator;
	};

	class PrefabCreator
	{
	public:
		PrefabCreator() = delete;
		static std::shared_ptr<Prefab> CreatePrefab(EPrefabType type, Entity& entityToPrefab);
		static std::shared_ptr<Prefab> CreatePrefab(const std::string& prefabPath);
		static std::shared_ptr<Entity> AddPrefabToScene(const Prefab& prefab, Registry& registry);
		static bool DeletePrefab(Prefab& prefabToDelete);
	};

}
