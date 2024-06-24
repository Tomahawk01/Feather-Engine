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

	private:
		Registry& m_Registry;
	};
}