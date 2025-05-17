#pragma once
#include "IDisplay.h"

#include "glm/glm.hpp"

#include <string>

namespace Feather {

	struct KeyEvent;
	struct PrefabbedEntity;
	class Camera2D;
	class Entity;

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

		void HandleKeyPressedEvent(const KeyEvent& keyEvent);
		void AddPrefabbedEntityToScene(const PrefabbedEntity& prefabbed);

	protected:
		virtual void DrawToolbar() override;

	private:
		std::unique_ptr<Camera2D> m_TilemapCam;
		bool m_WindowActive;
	};

}