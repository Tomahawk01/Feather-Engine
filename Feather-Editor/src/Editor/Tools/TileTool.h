#pragma once
#include "AbstractTool.h"
#include "Core/ECS/Entity.h"

namespace Feather {

	struct Tile;
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

		void SetSpriteUVs(int startX, int startY);
		void SetSpriteRect(const glm::vec2& spriteRect);

		const bool SpriteValid() const;
		const bool CanDrawOrCreate() const;

		inline void EnableGridSnap() { m_GridSnap = true; }
		inline void DisableGridSnap() { m_GridSnap = false; }

		inline Tile& GetTileData() { return *m_MouseTile; }

	private:
		glm::vec2 m_MouseRect;

		bool m_GridSnap;

	protected:
		uint32_t CheckForTile(const glm::vec2& position);

		Entity CreateEntity();
		Entity CreateEntity(uint32_t id);

		void DrawMouseSprite();

		virtual void ExamineMousePosition() override;
	protected:
		std::shared_ptr<SpriteBatchRenderer> m_BatchRenderer; // TODO: Overkill for one sprite. Remade it
		std::shared_ptr<Tile> m_MouseTile;
	};

}
