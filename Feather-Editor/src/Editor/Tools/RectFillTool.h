#pragma once
#include "TileTool.h"

namespace Feather {

	class RectBatchRenderer;
	struct Rect;

	class RectFillTool : public TileTool
	{
	public:
		RectFillTool();
		~RectFillTool();

		virtual void Create() override;
		virtual void Draw() override;

	private:
		void CreateTiles();
		void RemoveTiles();
		void DrawPreview(int dx, int dy);
		void ResetTile();
	private:
		std::unique_ptr<RectBatchRenderer> m_RectRenderer;
		std::unique_ptr<Rect> m_TileFillRect;
		glm::vec2 m_StartPressPos;
	};

}
