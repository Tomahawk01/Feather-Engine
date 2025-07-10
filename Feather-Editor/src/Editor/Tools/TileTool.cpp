#include "TileTool.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Essentials/Shader.h"
#include "Renderer/Essentials/Texture.h"
#include "Utils/MathUtilities.h"

#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Scene/SceneObject.h"

constexpr int MOUSE_SPRITE_LAYER = 10;

namespace Feather {

	TileTool::TileTool()
		: AbstractTool()
		, m_MouseRect{ 32.0f }
		, m_GridSnap{ true }
		, m_BatchRenderer{ std::make_shared<SpriteBatchRenderer>() }
		, m_MouseTile{ std::make_shared<Tile>() }
	{
		m_GridCoords = glm::vec2{ 0.0f };
	}

	void TileTool::Update(Canvas& canvas)
	{
		AbstractTool::Update(canvas);
		ExamineMousePosition();
	}

	void TileTool::ClearMouseTextureData()
	{
		m_MouseTile->sprite = SpriteComponent{};
	}

	void TileTool::LoadSpriteTextureData(const std::string& textureName)
	{
		// We need to get the current sprite layer to ensure that if we change tilesets, the layer we are drawing on does not reset
		int currentLayer = m_MouseTile->sprite.layer;

		m_MouseTile->sprite = SpriteComponent{
			.textureName = textureName,
			.width = m_MouseRect.x,
			.height = m_MouseRect.y,
			.color = Color{255, 255, 255, 255},
			.start_x = 0,
			.start_y = 0,
			.layer = currentLayer
		};

		auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(textureName);
		F_ASSERT(texture && "Texture must exist");
		GenerateUVs(m_MouseTile->sprite, texture->GetWidth(), texture->GetHeight());
	}

	const std::string& TileTool::GetSpriteTexture() const
	{
		return m_MouseTile->sprite.textureName;
	}

	void TileTool::SetSpriteUVs(int startX, int startY)
	{
		m_MouseTile->sprite.start_x = startX;
		m_MouseTile->sprite.start_y = startY;
		m_MouseTile->sprite.uvs.u = startX * m_MouseTile->sprite.uvs.uv_width;
		m_MouseTile->sprite.uvs.v = startY * m_MouseTile->sprite.uvs.uv_height;
	}

	void TileTool::SetSpriteRect(const glm::vec2& spriteRect)
	{
		if (m_MouseTile->sprite.textureName.empty())
			return;

		m_MouseRect = spriteRect;
		auto& sprite = m_MouseTile->sprite;
		sprite.width = m_MouseRect.x;
		sprite.height = m_MouseRect.y;

		auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.textureName);
		F_ASSERT(texture && "Texture must exist");
		GenerateUVs(sprite, texture->GetWidth(), texture->GetHeight());
	}

	const bool TileTool::SpriteValid() const
	{
		return !m_MouseTile->sprite.textureName.empty();
	}

	const bool TileTool::CanDrawOrCreate() const
	{
		return IsActivated() && !OutOfBounds() && IsOverTilemapWindow() && SpriteValid() && m_CurrentScene && m_CurrentScene->HasTileLayers();
	}

	uint32_t TileTool::CheckForTile(const glm::vec2& position)
	{
		if (!m_Registry)
			return entt::null;

		auto tileView = m_Registry->GetRegistry().view<TileComponent, TransformComponent>();

		for (auto entity : tileView)
		{
			Entity tile{ *m_Registry, entity };
			const auto& transform = tile.GetComponent<TransformComponent>();
			const auto& sprite = tile.GetComponent<SpriteComponent>();

			if (m_CurrentScene && m_CurrentScene->GetMapType() == EMapType::Grid)
			{
				if (position.x >= transform.position.x &&
					position.x < transform.position.x + sprite.width * transform.scale.x &&
					position.y >= transform.position.y &&
					position.y < transform.position.y + sprite.height * transform.scale.y &&
					m_MouseTile->sprite.layer == sprite.layer)
				{
					return static_cast<uint32_t>(entity);
				}
			}
			else // Iso Grids, we check at the center of the tile if there is an entity
			{
				// Get the center pos of the sprite
				int spriteCenterX = transform.position.x + ((sprite.width * transform.scale.x) / 2.0f);
				int spriteCenterY = transform.position.y + ((sprite.height * transform.scale.y) / 2.0f);

				// Get the offset of the position + sprite center
				int positionOffsetX = position.x + ((sprite.width * transform.scale.x) / 2.0f);
				int positionOffsetY = position.y + ((sprite.height * transform.scale.y) / 2.0f);

				if (positionOffsetX == spriteCenterX && positionOffsetY == spriteCenterY &&
					m_MouseTile->sprite.layer == sprite.layer)
				{
					return static_cast<uint32_t>(entity);
				}
			}
		}

		return entt::null;
	}

	Entity TileTool::CreateEntity()
	{
		F_ASSERT(m_Registry && "The registry must be valid to create an entity");
		return Entity{ *m_Registry, "", "" };
	}

	Entity TileTool::CreateEntity(uint32_t id)
	{
		F_ASSERT(m_Registry && "The registry must be valid to create an entity");
		return Entity{ *m_Registry, static_cast<entt::entity>(id) };
	}

	void TileTool::DrawMouseSprite()
	{
		m_BatchRenderer->Begin();
		const auto& sprite = m_MouseTile->sprite;
		const auto& transform = m_MouseTile->transform;

		glm::vec4 position{
			transform.position.x,
			transform.position.y,
			sprite.width * transform.scale.x,
			sprite.height * transform.scale.y
		};
		glm::vec4 uvs{
			sprite.uvs.u,
			sprite.uvs.v,
			sprite.uvs.uv_width,
			sprite.uvs.uv_height
		};

		const auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.textureName);
		if (texture)
			m_BatchRenderer->AddSprite(position, uvs, texture->GetID(), MOUSE_SPRITE_LAYER, glm::mat4{ 1.0f }, sprite.color);

		m_BatchRenderer->End();
		m_BatchRenderer->Render();
	}

	void TileTool::ExamineMousePosition()
	{
		if (!m_Camera)
			return;

		const auto& mouseWorldPos = GetMouseWorldCoords();
		auto& transform = m_MouseTile->transform;

		bool isOffset{ true };

		if (m_GridSnap)
		{
			if (m_CurrentScene->GetMapType() == EMapType::Grid)
			{
				glm::vec2 mouseGrid{ mouseWorldPos.x / (m_MouseRect.x * transform.scale.x),
									 mouseWorldPos.y / (m_MouseRect.y * transform.scale.y) };

				float scaledGridToCamX = glm::floor(mouseGrid.x);
				float scaledGridToCamY = glm::floor(mouseGrid.y);

				transform.position.x = scaledGridToCamX * m_MouseRect.x * transform.scale.x;
				transform.position.y = scaledGridToCamY * m_MouseRect.y * transform.scale.y;

				m_GridCoords.x = scaledGridToCamX;
				m_GridCoords.y = scaledGridToCamY;

				SetMouseWorldCoords(transform.position);
			}
			else
			{
				const auto& canvas = m_CurrentScene->GetCanvas();

				float doubleWidth = canvas.tileWidth * 2.0f;
				float doubleWidthOver4 = doubleWidth / 4.0f;
				float tileHeightOver4 = canvas.tileHeight / 4.0f;

				auto [cellX, cellY] = ConvertWorldPosToIsoCoords(mouseWorldPos, canvas);

				transform.position.x = ((doubleWidthOver4 * cellX) - (doubleWidthOver4 * cellY)) * 2.0f/* + canvas.offset.x*/;
				transform.position.y = ((tileHeightOver4 * cellX) + (tileHeightOver4 * cellY)) * 2.0f;

				m_GridCoords = glm::vec2{ cellX, cellY };
				SetMouseWorldCoords(transform.position);
			}
		}
		else
		{
			transform.position = isOffset ? mouseWorldPos - m_MouseRect * 0.5f : mouseWorldPos;
		}
	}

}
