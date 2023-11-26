#pragma once

#include <glm/glm.hpp>

namespace Feather {

	struct TransformComponent
	{
		glm::vec2 position{ glm::vec2{0.0f} }, scale{ glm::vec2 {1.0f} };
		float rotation{ 0.0f };
	};

}
