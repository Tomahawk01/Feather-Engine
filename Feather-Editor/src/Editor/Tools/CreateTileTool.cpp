#include "CreateTileTool.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/MainRegistry.h"
#include "Renderer/Core/Camera2D.h"
#include "Logger/Logger.h"
#include "../Utilities/EditorUtilities.h"

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
	}

	void CreateTileTool::RemoveTile()
	{
		const auto& mouseWorldCoords = GetMouseWorldCoords();

		if (auto id = CheckForTile(mouseWorldCoords); id != entt::null)
		{
			Entity tileToRemove{ CreateEntity(id) };
			tileToRemove.Kill();
		}
	}

}
