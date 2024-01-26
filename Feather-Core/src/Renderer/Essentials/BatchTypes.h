#pragma once

#include "Vertex.h"
#include "Font.h"

#include <string>

namespace Feather {

	struct Batch
	{
		GLuint numIndices{ 0 }, offset{ 0 }, textureID{ 0 };
	};

	struct LineBatch
	{
		GLuint offset{ 2 }, numVertices{ 0 };
		float lineWidth{ 1.0f };
	};

	struct SpriteGlyph
	{
		Vertex topLeft, bottomLeft, topRight, bottomRight;
		int layer;
		GLuint textureID;
	};

	struct LineGlyph
	{
		Vertex p1, p2;
		float lineWidth;
	};

	struct RectBatch
	{
		GLuint numIndices{ 0 }, offset{ 0 };
	};

	struct RectGlyph
	{
		Vertex topLeft, bottomLeft, topRight, bottomRight;
	};

	struct CircleGlyph
	{
		CircleVertex topLeft, bottomLeft, topRight, bottomRight;
	};


	struct TextBatch
	{
		GLuint offset{ 0 }, numVertices{ 0 }, fontAtlasID{ 0 };
	};

	struct TextGlyph
	{
		std::string textStr{ "" };
		glm::vec2 position{ 0.0f };
		Color color{ 255, 255, 255, 255 };
		glm::mat4 model{ 1.0f };
		std::shared_ptr<Font> font{ nullptr };
		float wrap{ 0.0f };
		int padding{ 0 };
	};

}
