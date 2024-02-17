#pragma once

#include "Core/ECS/Registry.h"

namespace Feather {

	class PhysicsSystem
	{
	public:
		PhysicsSystem(Registry& registry);
		~PhysicsSystem() = default;

		void Update(entt::registry& registry);

	private:
		Registry& m_Registry;
	};

}
