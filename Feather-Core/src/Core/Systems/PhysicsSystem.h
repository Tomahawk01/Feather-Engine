#pragma once

namespace Feather {

	class Registry;

	class PhysicsSystem
	{
	public:
		PhysicsSystem();
		~PhysicsSystem() = default;

		void Update(Registry& registry);
	};

}
