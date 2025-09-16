#include "CreateTileToolCmds.h"

#include "Logger/Logger.h"
#include "Core/ECS/Registry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Components/AllComponents.h"

#include "Editor/Utilities/EditorUtilities.h"

namespace Feather {

	void CreateTileToolAddCmd::undo()
	{
		F_ASSERT(registry && "The registry cannot be nullptr");
		if (!registry)
		{
			F_ERROR("Failed to undo create tile. Registry was not set correctly");
			return;
		}

		F_ASSERT(tile && "The tile cannot be nullptr");
		if (!tile)
		{
			F_ERROR("Failed to undo create tile. Tile was not set correctly");
			return;
		}

		auto tiles = registry->GetRegistry().view<TileComponent, TransformComponent>();
		const auto& tilePos = tile->transform.position;
		entt::entity entityToRemove{ entt::null };

		for (auto entity : tiles)
		{
			Entity t{ registry, entity };
			const auto& transform = t.GetComponent<TransformComponent>();
			const auto& sprite = t.GetComponent<SpriteComponent>();

			if (tilePos.x >= transform.position.x &&
				tilePos.x < transform.position.x + sprite.width * transform.scale.x &&
				tilePos.y >= transform.position.y &&
				tilePos.y < transform.position.y + sprite.height * transform.scale.y &&
				tile->sprite.layer == sprite.layer)
			{
				entityToRemove = entity;
				break;
			}
		}

		F_ASSERT(entityToRemove != entt::null && "Entity should not be null");
		if (entityToRemove != entt::null)
		{
			Entity ent{ registry, entityToRemove };
			ent.Destroy();
		}
	}

	void CreateTileToolAddCmd::redo()
	{
		F_ASSERT(registry && "The registry cannot be nullptr");
		if (!registry)
		{
			F_ERROR("Failed to undo create tile. Registry was not set correctly");
			return;
		}

		F_ASSERT(tile && "The tile cannot be nullptr");
		if (!tile)
		{
			F_ERROR("Failed to undo create tile. Tile was not set correctly");
			return;
		}

		Entity t{ registry, "", "" };
		t.AddComponent<TransformComponent>(tile->transform);
		t.AddComponent<SpriteComponent>(tile->sprite);
		t.AddComponent<TileComponent>(static_cast<std::uint32_t>(t.GetEntity()));

		if (tile->hasAnimation)
			t.AddComponent<AnimationComponent>(tile->animation);
		if (tile->isCollider)
			t.AddComponent<BoxColliderComponent>(tile->boxCollider);
		if (tile->isCircle)
			t.AddComponent<CircleColliderComponent>(tile->circleCollider);
		if (tile->hasPhysics)
			t.AddComponent<PhysicsComponent>(tile->physics);
	}

	void CreateTileToolRemoveCmd::undo()
	{
		F_ASSERT(registry && "The registry cannot be nullptr");
		if (!registry)
		{
			F_ERROR("Failed to undo create tile. Registry was not set correctly");
			return;
		}

		F_ASSERT(tile && "The tile cannot be nullptr");
		if (!tile)
		{
			F_ERROR("Failed to undo create tile. Tile was not set correctly");
			return;
		}

		Entity t{ registry, "", "" };
		t.AddComponent<TransformComponent>(tile->transform);
		t.AddComponent<SpriteComponent>(tile->sprite);
		t.AddComponent<TileComponent>(static_cast<std::uint32_t>(t.GetEntity()));

		if (tile->hasAnimation)
			t.AddComponent<AnimationComponent>(tile->animation);
		if (tile->isCollider)
			t.AddComponent<BoxColliderComponent>(tile->boxCollider);
		if (tile->isCircle)
			t.AddComponent<CircleColliderComponent>(tile->circleCollider);
		if (tile->hasPhysics)
			t.AddComponent<PhysicsComponent>(tile->physics);
	}

	void CreateTileToolRemoveCmd::redo()
	{
		F_ASSERT(registry && "The registry cannot be nullptr");
		if (!registry)
		{
			F_ERROR("Failed to undo create tile. Registry was not set correctly");
			return;
		}

		F_ASSERT(tile && "The tile cannot be nullptr");
		if (!tile)
		{
			F_ERROR("Failed to undo create tile. Tile was not set correctly");
			return;
		}

		auto tiles = registry->GetRegistry().view<TileComponent, TransformComponent>();
		const auto& tilePos = tile->transform.position;
		entt::entity entityToRemove{ entt::null };

		for (auto entity : tiles)
		{
			Entity t{ registry, entity };
			const auto& transform = t.GetComponent<TransformComponent>();
			const auto& sprite = t.GetComponent<SpriteComponent>();

			if (tilePos.x >= transform.position.x &&
				tilePos.x < transform.position.x + sprite.width * transform.scale.x &&
				tilePos.y >= transform.position.y &&
				tilePos.y < transform.position.y + sprite.height * transform.scale.y &&
				tile->sprite.layer == sprite.layer)
			{
				entityToRemove = entity;
				break;
			}
		}

		F_ASSERT(entityToRemove != entt::null && "Entity should not be null");
		if (entityToRemove != entt::null)
		{
			Entity ent{ registry, entityToRemove };
			ent.Destroy();
		}
	}

}
