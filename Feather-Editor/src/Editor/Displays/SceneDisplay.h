#pragma once

#include "Core/ECS/Registry.h"

namespace Feather {

	class SceneDisplay
	{
	public:
		SceneDisplay(Registry& registry);
		~SceneDisplay() = default;

		void Draw();

	private:
		Registry& m_Registry;
	};
}