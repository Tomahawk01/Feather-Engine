#pragma once

namespace Feather {

	class Registry;
	class Camera2D;

	class AnimationSystem
	{
	public:
		AnimationSystem() = default;
		~AnimationSystem() = default;

		void Update(Registry& registry, Camera2D& camera);
	};

}
