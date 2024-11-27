#include "TileTool.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Core/Camera2D.h"

#include "Editor/Utilities/EditorUtilities.h"
#include "Editor/Scene/SceneObject.h"

constexpr int MOUSE_SPRITE_LAYER = 10;

namespace Feather {

	TileTool::TileTool()
		: AbstractTool(),
		m_MouseRect{ 32.0f }, m_GridCoords{ 0.0f }, m_GridSnap{ true },
		m_BatchRenderer{ std::make_shared<SpriteBatchRenderer>() },
		m_MouseTile{ std::make_shared<Tile>() }
	{}

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
			.width = m_MouseRect.x,
			.height = m_MouseRect.y,
			.color = Color{255, 255, 255, 255},
			.start_x = 0,
			.start_y = 0,
			.layer = currentLayer,
			.texture_name = textureName
		};

		auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(textureName);
		F_ASSERT(texture && "Texture must exist");
		GenerateUVs(m_MouseTile->sprite, texture->GetWidth(), texture->GetHeight());
	}

	const std::string& TileTool::GetSpriteTexture() const
	{
		return m_MouseTile->sprite.texture_name;
	}

	void TileTool::SetBoxCollider(int width, int height, const glm::vec2& offset)
	{
		m_MouseTile->boxCollider.width = width;
		m_MouseTile->boxCollider.height = height;
		m_MouseTile->boxCollider.offset = offset;
	}

	void TileTool::SetSpriteLayer(int layer)
	{
		m_MouseTile->sprite.layer = layer;
	}

	void TileTool::SetScale(const glm::vec2& scale)
	{
		m_MouseTile->transform.scale = scale;
	}

	void TileTool::SetAnimation(int numFrames, int frameRate, bool isVertical, bool isLooped, int frameOffset)
	{
		m_MouseTile->animation.numFrames = numFrames;
		m_MouseTile->animation.frameRate = frameRate;
		m_MouseTile->animation.isVertical = isVertical;
		m_MouseTile->animation.isLooped = isLooped;
		m_MouseTile->animation.frameOffset = frameOffset;
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
		if (m_MouseTile->sprite.texture_name.empty())
			return;

		m_MouseRect = spriteRect;
		auto& sprite = m_MouseTile->sprite;
		sprite.width = m_MouseRect.x;
		sprite.height = m_MouseRect.y;

		auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.texture_name);
		F_ASSERT(texture && "Texture must exist");
		GenerateUVs(sprite, texture->GetWidth(), texture->GetHeight());
	}

	void TileTool::SetCollider(bool isCollider)
	{
		m_MouseTile->isCollider = isCollider;
	}

	void TileTool::SetCircle(bool isCircle)
	{
		m_MouseTile->isCircle = isCircle;
	}

	void TileTool::SetAnimation(bool hasAnimation)
	{
		m_MouseTile->hasAnimation = hasAnimation;
	}

	void TileTool::SetPhysics(bool hasPhysics)
	{
		m_MouseTile->hasPhysics = hasPhysics;
	}

	const bool TileTool::SpriteValid() const
	{
		return !m_MouseTile->sprite.texture_name.empty();
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

			if (position.x >= transform.position.x && position.x < transform.position.x + sprite.width * transform.scale.x &&
				position.y >= transform.position.y && position.y < transform.position.y + sprite.height * transform.scale.y &&
				m_MouseTile->sprite.layer == sprite.layer)
			{
				return static_cast<uint32_t>(entity);
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

		const auto texture = MAIN_REGISTRY().GetAssetManager().GetTexture(sprite.texture_name);
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

		const float cameraScale = m_Camera->GetScale();
		bool isOffset{ true };

		if (m_GridSnap)
		{
			glm::vec2 mouseGrid{
				mouseWorldPos.x / (m_MouseRect.x * transform.scale.x) * cameraScale,
				mouseWorldPos.y / (m_MouseRect.y * transform.scale.y) * cameraScale
			};

			float scaledGridToCamX = std::floor(mouseGrid.x / cameraScale);
			float scaledGridToCamY = std::floor(mouseGrid.y / cameraScale);

			transform.position.x = scaledGridToCamX * m_MouseRect.x * transform.scale.x;
			transform.position.y = scaledGridToCamY * m_MouseRect.y * transform.scale.y;

			m_GridCoords.x = scaledGridToCamX;
			m_GridCoords.y = scaledGridToCamY;

			SetMouseWorldCoords(transform.position);
		}
		else
		{
			transform.position = isOffset ? mouseWorldPos - m_MouseRect * 0.5f : mouseWorldPos;
		}
	}

}
