#include "RectFillTool.h"
#include "Renderer/Core/RectBatchRenderer.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Essentials/Primitives.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Logger/Logger.h"

#include "../Utilities/EditorUtilities.h"

constexpr int PREVIEW_LAYER = 10;

namespace Feather {

	RectFillTool::RectFillTool()
		: TileTool(),
		m_RectRenderer{ std::make_unique<RectBatchRenderer>() },
		m_TileFillRect{ std::make_unique<Rect>() },
		m_StartPressPos{ 0.0f }
	{}

	RectFillTool::~RectFillTool()
	{}

	void RectFillTool::Create()
	{
		if (!CanDrawOrCreate())
			return;

		if (MouseButtonJustPressed(MouseButton::LEFT) || MouseButtonJustPressed(MouseButton::RIGHT))
			m_StartPressPos = GetMouseWorldCoords();

		if (MouseButtonJustReleased(MouseButton::LEFT))
		{
			CreateTiles();
			ResetTile();
		}
		else if (MouseButtonJustReleased(MouseButton::RIGHT))
		{
			RemoveTiles();
			ResetTile();
		}
	}

	void RectFillTool::Draw()
	{
		if (!CanDrawOrCreate())
			return;

		auto shader = MAIN_REGISTRY().GetAssetManager().GetShader("basic");
		auto colorShader = MAIN_REGISTRY().GetAssetManager().GetShader("color");
		if (!shader || !colorShader)
			return;

		shader->Enable();
		auto camMat = m_Camera->GetCameraMatrix();
		shader->SetUniformMat4("uProjection", camMat);
		DrawMouseSprite();

		bool leftMousePressed{ MouseButtonPressed(MouseButton::LEFT) };
		bool rightMousePressed{ MouseButtonPressed(MouseButton::RIGHT) };
		if (!leftMousePressed && !rightMousePressed)
			return;

		const auto& mouseWorldCoords = GetMouseWorldCoords();
		auto dx = mouseWorldCoords.x - m_StartPressPos.x;
		auto dy = mouseWorldCoords.y - m_StartPressPos.y;

		const auto& sprite = m_MouseTile->sprite;
		const auto& transform = m_MouseTile->transform;

		auto spriteWidth = sprite.width * transform.scale.x;
		auto spriteHeight = sprite.height * transform.scale.y;

		dx += dx > 0 ? spriteWidth : -spriteWidth;
		dy += dy > 0 ? spriteHeight : -spriteHeight;

		auto newPosX = m_StartPressPos.x + (dx > 0 ? 0 : spriteWidth);
		auto newPosY = m_StartPressPos.y + (dy > 0 ? 0 : spriteHeight);

		Color color{};

		if (leftMousePressed)
		{
			color = { 125, 250, 255, 100 };
			m_BatchRenderer->Begin();
			DrawPreview(dx, dy);
			m_BatchRenderer->End();
			m_BatchRenderer->Render();
			shader->Disable();
		}
		else if (rightMousePressed)
		{
			color = { 255, 115, 120, 100 };
		}

		colorShader->Enable();
		m_RectRenderer->Begin();
		m_RectRenderer->AddRect(Rect{ .position = {newPosX, newPosY}, .width = dx, .height = dy, .color = color });
		m_RectRenderer->End();
		m_RectRenderer->Render();
		colorShader->Disable();

		m_TileFillRect->position = { newPosX, newPosY };
		m_TileFillRect->width = dx;
		m_TileFillRect->height = dy;
	}

	void RectFillTool::CreateTiles()
	{
		const auto& sprite = m_MouseTile->sprite;
		const auto& transform = m_MouseTile->transform;
		auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.texture_name);
		if (!texture)
			return;

		const int dx = static_cast<int>(m_TileFillRect->width);
		const int dy = static_cast<int>(m_TileFillRect->height);

		auto spriteWidth = static_cast<int>(sprite.width * transform.scale.x * (dx > 0 ? 1.0f : -1.0f));
		auto spriteHeight = static_cast<int>(sprite.height * transform.scale.y * (dy > 0 ? 1.0f : -1.0f));

		for (int y = 0; (dy > 0 ? y < dy : y > dy); y += spriteHeight)
		{
			for (int x = 0; (dx > 0 ? x < dx : x > dx); x += spriteWidth)
			{
				glm::vec2 newTilePosition{ m_StartPressPos.x + x, m_StartPressPos.y + y };

				if (auto id = CheckForTile(newTilePosition); id != entt::null)
					continue;

				Entity tile{ CreateEntity() };
				auto& newTransform = tile.AddComponent<TransformComponent>(transform);
				newTransform.position = newTilePosition;
				tile.AddComponent<SpriteComponent>(sprite);

				if (m_MouseTile->isCollider)
				{
					// TODO: Add collider component
				}
				if (m_MouseTile->hasAnimation)
				{
					// TODO: Add animation component
				}
				if (m_MouseTile->hasPhysics)
				{
					// TODO: Add physics component
				}

				tile.AddComponent<TileComponent>(static_cast<uint32_t>(tile.GetEntity()));
			}
		}
	}

	void RectFillTool::RemoveTiles()
	{
		const auto& sprite = m_MouseTile->sprite;
		const auto& transform = m_MouseTile->transform;
		const int dx = static_cast<int>(m_TileFillRect->width);
		const int dy = static_cast<int>(m_TileFillRect->height);

		auto spriteWidth = static_cast<int>(sprite.width * transform.scale.x * (dx > 0 ? 1.0f : -1.0f));
		auto spriteHeight = static_cast<int>(sprite.height * transform.scale.y * (dy > 0 ? 1.0f : -1.0f));

		std::set<std::uint32_t> entitiesToRemove{};

		for (int y = 0; (dy > 0 ? y < dy : y > dy); y += spriteHeight)
		{
			for (int x = 0; (dx > 0 ? x < dx : x > dx); x += spriteWidth)
			{
				if (auto id = CheckForTile(glm::vec2{ m_StartPressPos.x + x, m_StartPressPos.y + y }); id != entt::null)
					entitiesToRemove.insert(id);
			}
		}

		for (auto id : entitiesToRemove)
		{
			Entity removedTile{ CreateEntity(id) };
			removedTile.Kill();
		}
	}

	void RectFillTool::DrawPreview(int dx, int dy)
	{
		const auto& sprite = m_MouseTile->sprite;
		const auto& transform = m_MouseTile->transform;
		auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.texture_name);
		if (!texture)
			return;

		auto spriteWidth = static_cast<int>(sprite.width * transform.scale.x * (dx > 0 ? 1.0f : -1.0f));
		auto spriteHeight = static_cast<int>(sprite.height * transform.scale.y * (dy > 0 ? 1.0f : -1.0f));

		for (int y = 0; (dy > 0 ? y < dy : y > dy); y += spriteHeight)
		{
			for (int x = 0; (dx > 0 ? x < dx : x > dx); x += spriteWidth)
			{
				glm::vec4 tilePosition{
					m_StartPressPos.x + x,
					m_StartPressPos.y + y,
					sprite.width * transform.scale.x,
					sprite.height * transform.scale.y
				};
				glm::vec4 uvs{
					sprite.uvs.u,
					sprite.uvs.v,
					sprite.uvs.uv_width,
					sprite.uvs.uv_height
				};

				m_BatchRenderer->AddSprite(tilePosition, uvs, texture->GetID(), PREVIEW_LAYER, glm::mat4{ 1.0f }, sprite.color);
			}
		}
	}

	void RectFillTool::ResetTile()
	{
		*m_TileFillRect = Rect{};
	}

}
