#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <sstream>

namespace Feather {

	struct Color
	{
		GLubyte r, g, b, a;
	};

	struct Vertex
	{
		glm::vec2 position{ 0.0f }, uvs{ 0.0f };
		Color color{ .r = 255, .g = 255,.b = 255, .a = 255 };

		void set_color(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
		{
			color.r = r;
			color.g = g;
			color.b = b;
			color.a = a;
		}

		/*
		* 0xff00ffac
		*       R         G         B         A
		* 0b1111 1111 0000 0000 1111 1111 1010 1100
		*/
		void set_color(GLuint newColor)
		{
			color.r = (newColor >> 24) & 0xFF;
			color.g = (newColor >> 16) & 0xFF;
			color.b = (newColor >> 8) & 0xFF;
			color.a = (newColor >> 0) & 0xFF;
		}

		[[nodiscard]] std::string to_string() const
		{
			std::stringstream ss;
			ss << "==== Color ==== \n" <<
				  "Red: " << color.r << "\n" <<
				  "Green: " << color.g << "\n" <<
				  "Blue: " << color.b << "\n" <<
				  "Alpha: " << color.a << "\n";

			return ss.str();
		}
	};

	struct CircleVertex
	{
		glm::vec2 position, uvs;
		Color color;
		float lineThickness;
	};

}
