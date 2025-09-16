#include "RectToolCmds.h"

#include "Logger/Logger.h"
#include "Core/ECS/Registry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Components/AllComponents.h"

#include "Editor/Utilities/EditorUtilities.h"

namespace Feather {

	void RectToolAddTilesCmd::undo()
	{
		F_ASSERT(registry && "The registry cannot be nullptr");
		if (!registry)
		{
			F_ERROR("Failed to undo create tile. Registry was not set correctly");
			return;
		}

		auto tileView = registry->GetRegistry().view<TileComponent, TransformComponent>();
		for (const auto& tile : tiles)
		{
			const auto& tilePos = tile.transform.position;
			const auto& layer = tile.sprite.layer;

			for (auto entity : tileView)
			{
				Entity checkedTile{ registry, entity };
				const auto& transform = checkedTile.GetComponent<TransformComponent>();
				const auto& sprite = checkedTile.GetComponent<SpriteComponent>();

				if (tilePos.x >= transform.position.x &&
					tilePos.x < transform.position.x + sprite.width * transform.scale.x &&
					tilePos.y >= transform.position.y &&
					tilePos.y < transform.position.y + sprite.height * transform.scale.y &&
					layer == sprite.layer)
				{
					if (entity != entt::null)
						registry->GetRegistry().destroy(entity);

					break;
				}
			}
		}
	}

	void RectToolAddTilesCmd::redo()
	{
		F_ASSERT(registry && "The registry cannot be nullptr");
		if (!registry)
		{
			F_ERROR("Failed to undo create tile. Registry was not set correctly");
			return;
		}

		for (const auto& tile : tiles)
		{
			Entity addedTile{ registry, "", "" };
			addedTile.AddComponent<TransformComponent>(tile.transform);
			addedTile.AddComponent<SpriteComponent>(tile.sprite);
			addedTile.AddComponent<TileComponent>(static_cast<std::uint32_t>(addedTile.GetEntity()));

			if (tile.hasAnimation)
				addedTile.AddComponent<AnimationComponent>(tile.animation);
			if (tile.isCollider)
				addedTile.AddComponent<BoxColliderComponent>(tile.boxCollider);
			if (tile.isCircle)
				addedTile.AddComponent<CircleColliderComponent>(tile.circleCollider);
			if (tile.hasPhysics)
				addedTile.AddComponent<PhysicsComponent>(tile.physics);
		}
	}

	void RectToolRemoveTilesCmd::undo()
	{
		F_ASSERT(registry && "The registry cannot be nullptr");
		if (!registry)
		{
			F_ERROR("Failed to undo create tile. Registry was not set correctly");
			return;
		}

		for (const auto& tile : tiles)
		{
			Entity addedTile{ registry, "", "" };
			addedTile.AddComponent<TransformComponent>(tile.transform);
			addedTile.AddComponent<SpriteComponent>(tile.sprite);
			addedTile.AddComponent<TileComponent>(static_cast<std::uint32_t>(addedTile.GetEntity()));

			if (tile.hasAnimation)
				addedTile.AddComponent<AnimationComponent>(tile.animation);
			if (tile.isCollider)
				addedTile.AddComponent<BoxColliderComponent>(tile.boxCollider);
			if (tile.isCircle)
				addedTile.AddComponent<CircleColliderComponent>(tile.circleCollider);
			if (tile.hasPhysics)
				addedTile.AddComponent<PhysicsComponent>(tile.physics);
		}
	}

	void RectToolRemoveTilesCmd::redo()
	{
		F_ASSERT(registry && "The registry cannot be nullptr");
		if (!registry)
		{
			F_ERROR("Failed to undo create tile. Registry was not set correctly");
			return;
		}

		auto tileView = registry->GetRegistry().view<TileComponent, TransformComponent>();
		for (const auto& tile : tiles)
		{
			const auto& tilePos = tile.transform.position;
			const auto& layer = tile.sprite.layer;

			for (auto entity : tileView)
			{
				Entity checkedTile{ registry, entity };
				const auto& transform = checkedTile.GetComponent<TransformComponent>();
				const auto& sprite = checkedTile.GetComponent<SpriteComponent>();

				if (tilePos.x >= transform.position.x &&
					tilePos.x < transform.position.x + sprite.width * transform.scale.x &&
					tilePos.y >= transform.position.y &&
					tilePos.y < transform.position.y + sprite.height * transform.scale.y &&
					layer == sprite.layer)
				{
					if (entity != entt::null)
						registry->GetRegistry().destroy(entity);

					break;
				}
			}
		}
	}

}
