#pragma once

#include "Core/ECS/Registry.h"

namespace Feather {

	class AnimationSystem
	{
	public:
		AnimationSystem(Registry& registry);
		~AnimationSystem() = default;

		void Update();

	private:
		Registry& m_Registry;
	};

}
