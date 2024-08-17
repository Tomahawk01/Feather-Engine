#pragma once

#include "IDisplay.h"
#include "Core/ECS/Registry.h"

namespace Feather {

	class SceneDisplay : public IDisplay
	{
	public:
		SceneDisplay();
		~SceneDisplay() = default;

		virtual void Draw() override;
		virtual void Update() override;

	private:
		void LoadScene();
		void UnloadScene();

		void RenderScene();

	private:
		bool m_PlayScene;
		bool m_SceneLoaded;
	};
}