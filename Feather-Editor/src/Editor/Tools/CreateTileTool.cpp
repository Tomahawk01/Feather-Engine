#include "CreateTileTool.h"

#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Renderer/Core/Camera2D.h"
#include "Logger/Logger.h"

#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Commands/CommandManager.h"
#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"

namespace Feather {

	CreateTileTool::CreateTileTool()
		: TileTool()
	{}

	void CreateTileTool::Create()
	{
		if (OutOfBounds() || !IsActivated() || !SpriteValid() || !m_Registry)
			return;

		if (MouseButtonJustPressed(MouseButton::LEFT) || MouseButtonPressed(MouseButton::LEFT))
		{
			CreateNewTile();
		}
		else if (MouseButtonJustPressed(MouseButton::RIGHT) || MouseButtonPressed(MouseButton::RIGHT))
		{
			RemoveTile();
		}
	}

	void CreateTileTool::Draw()
	{
		if (OutOfBounds() || !IsActivated() || !SpriteValid() || !m_Camera)
			return;

		auto shader = MAIN_REGISTRY().GetAssetManager().GetShader("basic");
		if (!shader)
			return;

		shader->Enable();
		auto camMat = m_Camera->GetCameraMatrix();
		shader->SetUniformMat4("uProjection", camMat);
		DrawMouseSprite();
		shader->Disable();
	}

	void CreateTileTool::CreateNewTile()
	{
		const auto& mouseWorldCoords = GetMouseWorldCoords();

		if (auto id = CheckForTile(mouseWorldCoords); id != entt::null)
			return;

		Entity tile{ CreateEntity() };
		tile.AddComponent<TransformComponent>(m_MouseTile->transform);
		tile.AddComponent<SpriteComponent>(m_MouseTile->sprite);

		if (m_MouseTile->isCollider)
		{
			tile.AddComponent<BoxColliderComponent>(m_MouseTile->boxCollider);
		}
		if (m_MouseTile->isCircle)
		{
			tile.AddComponent<CircleColliderComponent>(m_MouseTile->circleCollider);
		}
		if (m_MouseTile->hasAnimation)
		{
			tile.AddComponent<AnimationComponent>(m_MouseTile->animation);
		}
		if (m_MouseTile->hasPhysics)
		{
			tile.AddComponent<PhysicsComponent>(m_MouseTile->physics);
		}

		tile.AddComponent<TileComponent>(static_cast<uint32_t>(tile.GetEntity()));

		auto createToolAddCmd = UndoableCommands
		{
			CreateTileToolAddCmd
			{
				.registry = SCENE_MANAGER().GetCurrentScene()->GetRegistryPtr(),
				.tile = std::make_shared<Tile>(*m_MouseTile)
			}
		};

		COMMAND_MANAGER().Execute(createToolAddCmd);
	}

	void CreateTileTool::RemoveTile()
	{
		const auto& mouseWorldCoords = GetMouseWorldCoords();

		if (auto id = CheckForTile(mouseWorldCoords); id != entt::null)
		{
			Entity tileToRemove{ CreateEntity(id) };
			Tile removedTile{};

			removedTile.transform = tileToRemove.GetComponent<TransformComponent>();
			removedTile.sprite= tileToRemove.GetComponent<SpriteComponent>();

			if (tileToRemove.HasComponent<BoxColliderComponent>())
			{
				removedTile.isCollider = true;
				removedTile.boxCollider = tileToRemove.GetComponent<BoxColliderComponent>();
			}
			if (tileToRemove.HasComponent<CircleColliderComponent>())
			{
				removedTile.isCircle = true;
				removedTile.circleCollider = tileToRemove.GetComponent<CircleColliderComponent>();
			}
			if (tileToRemove.HasComponent<AnimationComponent>())
			{
				removedTile.hasAnimation = true;
				removedTile.animation = tileToRemove.GetComponent<AnimationComponent>();
			}
			if (tileToRemove.HasComponent<PhysicsComponent>())
			{
				removedTile.hasPhysics = true;
				removedTile.physics = tileToRemove.GetComponent<PhysicsComponent>();
			}

			tileToRemove.Kill();

			auto createToolRemoveCmd = UndoableCommands
			{
				CreateTileToolRemoveCmd
				{
					.registry = SCENE_MANAGER().GetCurrentScene()->GetRegistryPtr(),
					.tile = std::make_shared<Tile>(removedTile)
				}
			};

			COMMAND_MANAGER().Execute(createToolRemoveCmd);
		}
	}

}
