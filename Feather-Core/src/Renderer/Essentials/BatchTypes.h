#pragma once

#include "Vertex.h"

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

}
