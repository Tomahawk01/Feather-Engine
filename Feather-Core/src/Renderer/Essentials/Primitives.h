#pragma once

#include "Vertex.h"
#include "Font.h"

#include <string>
#include <memory>

namespace Feather {

	struct Line
	{
		glm::vec2 p1{ 0.0f };
		glm::vec2 p2{ 0.0f };
		float lineWidth{ 1.0f };
		Color color{};
	};

	struct Rect
	{
		glm::vec2 position{ 0.0f };
		float width{ 0.0f };
		float height{ 0.0f };
		Color color{};
	};

	struct Circle
	{
		glm::vec2 position{ 0.0f };
		float lineThickness{ 1.0f };
		float radius{ 0.0f };
		Color color{};
	};

	struct Text
	{
		glm::vec2 position{ 0.0f };
		std::string textStr{};
		float wrap{ -1.0f };
		std::shared_ptr<Font> pFont{ nullptr };
		Color color{ 255, 255, 255, 255 };
	};

}
