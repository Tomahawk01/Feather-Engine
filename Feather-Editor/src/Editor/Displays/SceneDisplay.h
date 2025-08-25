#pragma once

#include "IDisplay.h"

namespace Feather {

	struct KeyEvent;

	class SceneDisplay : public IDisplay
	{
	public:
		SceneDisplay();
		~SceneDisplay() = default;

		virtual void Draw() override;
		virtual void Update() override;

	protected:
		virtual void DrawToolbar() override;

	private:
		void LoadScene();
		void UnloadScene();
		void RenderScene() const;

		void HandleKeyEvent(const KeyEvent keyEvent);

	private:
		bool m_PlayScene;
		bool m_WindowActive;
		bool m_SceneLoaded;
	};
}