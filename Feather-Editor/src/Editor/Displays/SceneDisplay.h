#pragma once

#include "IDisplay.h"
#include "Core/ECS/Registry.h"

namespace Feather {

	class SceneDisplay : public IDisplay
	{
	public:
		SceneDisplay(Registry& registry);
		~SceneDisplay() = default;

		virtual void Draw() override;
		virtual void Update() override;

	private:
		void LoadScene();
		void UnloadScene();

		void RenderScene();

	private:
		Registry& m_Registry;
		bool m_PlayScene;
		bool m_SceneLoaded;
	};
}