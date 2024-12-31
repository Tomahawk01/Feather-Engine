#pragma once
#include "IDisplay.h"

#include "glm/glm.hpp"

#include <string>

namespace Feather {

	class Camera2D;

	class TilemapDisplay : public IDisplay
	{
	public:
		TilemapDisplay();
		~TilemapDisplay() = default;

		virtual void Draw() override;
		virtual void Update() override;

	private:
		void RenderTilemap();
		void LoadNewScene();
		void PanZoomCamera(const glm::vec2& mousePos);

	protected:
		virtual void DrawToolbar() override;

	private:
		std::unique_ptr<Camera2D> m_TilemapCam;
	};

}