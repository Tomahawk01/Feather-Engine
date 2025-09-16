#include "TileCmds.h"

#include "Logger/Logger.h"
#include "Core/ECS/Registry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Components/AllComponents.h"

#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Scene/SceneObject.h"

namespace Feather {

	void RemoveTileLayerCmd::undo()
	{
		F_ASSERT(sceneObject && "The SceneObject cannot be nullptr");

		if (!sceneObject)
		{
			F_ERROR("Failed to undo remove tile layer. SceneObject was not set correctly");
			return;
		}

		auto* registry = sceneObject->GetRegistryPtr();
		if (!registry)
		{
			F_ERROR("Failed to undo remove tile layer. Scene Registry was invalid");
			return;
		}

		// Push each layer above this up one layer
		for (auto& layerParam : sceneObject->GetLayerParams())
		{
			if (layerParam.layer >= spriteLayerParams.layer)
			{
				layerParam.layer++;
			}
		}

		sceneObject->AddLayer(spriteLayerParams);

		// Push each sprite up one layer
		auto tileView = registry->GetRegistry().view<TileComponent, SpriteComponent>();
		for (auto entity : tileView)
		{
			Entity ent{ registry, entity };
			if (auto* pSprite = ent.TryGetComponent<SpriteComponent>())
			{
				if (pSprite->layer >= spriteLayerParams.layer)
				{
					pSprite->layer++;
				}
			}
		}

		// Add the tiles back into the registry
		for (const auto& tile : tilesRemoved)
		{
			Entity ent{ registry, "", "" };
			ent.AddComponent<TransformComponent>(tile.transform);
			ent.AddComponent<SpriteComponent>(tile.sprite);
			ent.AddComponent<TileComponent>(static_cast<std::uint32_t>(ent.GetEntity()));

			if (tile.isCollider)
			{
				ent.AddComponent<BoxColliderComponent>(tile.boxCollider);
			}

			if (tile.hasPhysics)
			{
				ent.AddComponent<PhysicsComponent>(tile.physics);
			}

			if (tile.hasAnimation)
			{
				ent.AddComponent<AnimationComponent>(tile.animation);
			}

			if (tile.isCircle)
			{
				ent.AddComponent<CircleColliderComponent>(tile.circleCollider);
			}
		}
	}

	void RemoveTileLayerCmd::redo()
	{
		F_ASSERT(sceneObject && "The SceneObject cannot be nullptr");

		if (!sceneObject)
		{
			F_ERROR("Failed to undo remove tile layer. SceneObject was not set correctly");
			return;
		}

		auto* registry = sceneObject->GetRegistryPtr();
		if (!registry)
		{
			F_ERROR("Failed to undo remove tile layer. Scene Registry was invalid");
			return;
		}

		auto& layerParams = sceneObject->GetLayerParams();

		layerParams.erase(layerParams.begin() + spriteLayerParams.layer);

		// Drop all layers above removed layer down by 1
		for (auto& spriteLayer : layerParams)
		{
			if (spriteLayer.layer > spriteLayerParams.layer)
			{
				spriteLayer.layer--;
			}
		}

		auto view = registry->GetRegistry().view<TileComponent, SpriteComponent>();
		for (auto entity : view)
		{
			Entity ent{ registry, entity };
			auto& sprite = ent.GetComponent<SpriteComponent>();
			if (sprite.layer == spriteLayerParams.layer)
			{
				ent.Destroy();
			}
			else if (sprite.layer > spriteLayerParams.layer) // Drop the layer down if greater
			{
				sprite.layer--;
			}
		}
	}

	void AddTileLayerCmd::undo()
	{
		F_ASSERT(sceneObject && "The SceneObject cannot be nullptr");

		if (!sceneObject)
		{
			F_ERROR("Failed to undo add tile layer. SceneObject was not set correctly");
			return;
		}

		auto& layerParams = sceneObject->GetLayerParams();
		layerParams.erase(layerParams.begin() + spriteLayerParams.layer);
	}

	void AddTileLayerCmd::redo()
	{
		F_ASSERT(sceneObject && "The SceneObject cannot be nullptr");

		if (!sceneObject)
		{
			F_ERROR("Failed to redo add tile layer. SceneObject was not set correctly");
			return;
		}

		auto& layerParams = sceneObject->GetLayerParams();
		layerParams.push_back(spriteLayerParams);
	}

	void MoveTileLayerCmd::undo()
	{
		F_ASSERT(sceneObject && "The SceneObject cannot be nullptr");

		if (!sceneObject)
		{
			F_ERROR("Failed to redo add tile layer. SceneObject was not set correctly");
			return;
		}

		auto* registry = sceneObject->GetRegistryPtr();
		F_ASSERT(registry && "Registry must be valid");

		// Change the layers
		auto& layerParams = sceneObject->GetLayerParams();
		const int nextLayer = layerParams[to].layer;
		layerParams[to].layer = layerParams[from].layer;
		layerParams[from].layer = nextLayer;
		std::swap(layerParams[from], layerParams[to]);

		auto tileView = registry->GetRegistry().view<TileComponent, SpriteComponent>();
		for (auto entity : tileView)
		{
			Entity ent{ registry, entity };
			if (auto* sprite = ent.TryGetComponent<SpriteComponent>())
			{
				if (sprite->layer == to)
				{
					sprite->layer = from;
				}
				else if (sprite->layer == from)
				{
					sprite->layer = to;
				}
			}
		}
	}

	void MoveTileLayerCmd::redo()
	{
		F_ASSERT(sceneObject && "The SceneObject cannot be nullptr");

		if (!sceneObject)
		{
			F_ERROR("Failed to redo add tile layer. SceneObject was not set correctly");
			return;
		}

		auto* registry = sceneObject->GetRegistryPtr();
		F_ASSERT(registry && "Registry must be valid");

		// Change the layers
		auto& layerParams = sceneObject->GetLayerParams();
		const int nextLayer = layerParams[from].layer;
		layerParams[from].layer = layerParams[to].layer;
		layerParams[to].layer = nextLayer;
		std::swap(layerParams[from], layerParams[to]);

		auto tileView = registry->GetRegistry().view<TileComponent, SpriteComponent>();
		for (auto entity : tileView)
		{
			Entity ent{ registry, entity };
			if (auto* sprite = ent.TryGetComponent<SpriteComponent>())
			{
				if (sprite->layer == to)
				{
					sprite->layer = from;
				}
				else if (sprite->layer == from)
				{
					sprite->layer = to;
				}
			}
		}
	}

	void ChangeTileLayerNameCmd::undo()
	{
		F_ASSERT(sceneObject && "The SceneObject cannot be nullptr");

		if (!sceneObject)
		{
			F_ERROR("Failed to undo change tile layer name. SceneObject was not set correctly");
			return;
		}

		auto& layerParams = sceneObject->GetLayerParams();
		auto layerItr = std::ranges::find(layerParams, newName, &SpriteLayerParams::layerName);
		F_ASSERT(layerItr != layerParams.end() && "Failed to find layer");
		layerItr->layerName = oldName;
	}

	void ChangeTileLayerNameCmd::redo()
	{
		F_ASSERT(sceneObject && "The SceneObject cannot be nullptr");

		if (!sceneObject)
		{
			F_ERROR("Failed to redo change tile layer name. SceneObject was not set correctly");
			return;
		}

		auto& layerParams = sceneObject->GetLayerParams();
		auto layerItr = std::ranges::find(layerParams, oldName, &SpriteLayerParams::layerName);
		F_ASSERT(layerItr != layerParams.end() && "Failed to find layer");
		layerItr->layerName = newName;
	}

}
