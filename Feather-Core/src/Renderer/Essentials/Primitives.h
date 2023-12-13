#pragma once

#include "Vertex.h"

namespace Feather {

	struct Line
	{
		glm::vec2 p1{ 0.0f }, p2{ 0.0f };
		float lineWidth{ 1.0f };
		Color color{};
	};

	struct Rect
	{
		glm::vec2 position{ 0.0f };
		float width{ 0.0f }, height{ 0.0f };
		Color color{};
	};

	struct Circle
	{
		glm::vec2 position{ 0.0f };
		float thickness{ 1.0f }, radius{ 0.0f };
		Color color{};
	};

}
