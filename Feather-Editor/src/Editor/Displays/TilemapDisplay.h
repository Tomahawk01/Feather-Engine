#pragma once
#include "IDisplay.h"

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
	private:
		std::unique_ptr<Camera2D> m_TilemapCam;
	};

}