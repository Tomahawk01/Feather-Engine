#include "Font.h"

#include <stb_truetype.h>

namespace Feather {

	Font::Font(GLuint fontAtlasID, int widht, int height, float fontSize, void* data)
		: m_FontAtlasID{ fontAtlasID }, m_Width{ widht }, m_Height{ height }, m_FontSize{ fontSize }, m_Data{ data }
	{}

	Font::~Font()
	{
		if (m_FontAtlasID != 0)
			glDeleteTextures(1, &m_FontAtlasID);

		if (m_Data)
			delete m_Data;
	}

	FontGlyph Font::GetGlyph(char c, glm::vec2& pos)
	{
		FontGlyph glyph{};

		if (c >= 32 && c < 128)
		{
			stbtt_aligned_quad quad;
			stbtt_GetBakedQuad((stbtt_bakedchar*)(m_Data), m_Width, m_Height, c - 32, &pos.x, &pos.y, &quad, 1);

			glyph.min = Vertex{ .position = glm::vec2{quad.x0, quad.y0}, .uvs = glm::vec2{quad.s0, quad.t0} };
			glyph.max = Vertex{ .position = glm::vec2{quad.x1, quad.y1}, .uvs = glm::vec2{quad.s1, quad.t1} };
		}

		return glyph;
	}

	void Font::GetNextCharPos(char c, glm::vec2& pos)
	{
		if (c >= 32 && c < 128)
		{
			stbtt_aligned_quad quad;
			stbtt_GetBakedQuad((stbtt_bakedchar*)(m_Data), m_Width, m_Height, c - 32, &pos.x, &pos.y, &quad, 1);
		}
	}

}
