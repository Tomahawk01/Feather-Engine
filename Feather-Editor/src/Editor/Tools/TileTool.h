#pragma once
#include "AbstractTool.h"
#include "Core/ECS/Entity.h"

namespace Feather {

	class SpriteBatchRenderer;

	class TileTool : public AbstractTool
	{
	public:
		TileTool();
		virtual ~TileTool() = default;

		virtual void Create() = 0;
		virtual void Draw() = 0;

		virtual void Update(Canvas& canvas) override;

		void ClearMouseTextureData();
		void LoadSpriteTextureData(const std::string& textureName);
		const std::string& GetSpriteTexture() const;

		void SetBoxCollider(int width, int height, const glm::vec2& offset);
		void SetSpriteLayer(int layer);
		void SetScale(const glm::vec2& scale);
		void SetAnimation(int numFrames, int frameRate, bool isVertical, bool isLooped, int frameOffset);
		void SetSpriteUVs(int startX, int startY);
		// TODO: Set physics props
		void SetSpriteRect(const glm::vec2& spriteRect);

		void SetCollider(bool isCollider);
		void SetCircle(bool isCircle);
		void SetAnimation(bool hasAnimation);
		void SetPhysics(bool hasPhysics);

		const bool SpriteValid() const;
		const bool CanDrawOrCreate() const;

		inline void EnableGridSnap() { m_GridSnap = true; }
		inline void DisableGridSnap() { m_GridSnap = false; }

	private:
		glm::vec2 m_MouseRect;
		glm::vec2 m_GridCoords;

		bool m_GridSnap;

	protected:
		uint32_t CheckForTile(const glm::vec2& position);

		Entity CreateEntity();
		Entity CreateEntity(uint32_t id);

		void DrawMouseSprite();

		virtual void ExamineMousePosition() override;
	protected:
		std::shared_ptr<SpriteBatchRenderer> m_BatchRenderer; // TODO: Overkill for one sprite. Remade it
		std::shared_ptr<struct Tile> m_MouseTile;
	};

}
