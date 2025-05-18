#include "Font.h"

#include <iostream>

#include <stb_truetype.h>

namespace Feather {

	Font::Font(GLuint fontAtlasID, int widht, int height, float fontSize, void* data, float fontAscent)
		: m_FontAtlasID{ fontAtlasID }
		, m_Width{ widht }
		, m_Height{ height }
		, m_FontSize{ fontSize }
		, m_FontAscent{ fontAscent }
		, m_Data{ std::move(data) }
	{
		float x{ 0.0f }, y{ 0.0f };

		for (int i = 32; i < 128; i++)
		{
			stbtt_aligned_quad quad;
			stbtt_GetBakedQuad((stbtt_bakedchar*)(m_Data), m_Width, m_Height, i - 32, &x, &y, &quad, 1);

			m_mapPaddingInfo.emplace(static_cast<char>(i),
				PaddingInfo{ .paddingX = m_FontSize - (quad.x1 - quad.x0),
							 .paddingY = m_FontSize - (quad.y1 - quad.y0) });
		}

		float paddingX{ 0.0f }, paddingY{ 0.0f };
		for (const auto& [c, paddingInfo] : m_mapPaddingInfo)
		{
			paddingX += paddingInfo.paddingX;
			paddingY += paddingInfo.paddingY;
		}

		m_AveragePadding.paddingX = std::floor(paddingX / m_mapPaddingInfo.size());
		m_AveragePadding.paddingY = std::floor(paddingY / m_mapPaddingInfo.size());
	}

	Font::~Font()
	{
		if (m_FontAtlasID != 0)
			glDeleteTextures(1, &m_FontAtlasID);

		if (m_Data)
		{
			typedef stbtt_bakedchar(stbtt_bakedchar)[96];
			delete[](stbtt_bakedchar*)m_Data;
		}
	}

	FontGlyph Font::GetGlyph(char c, glm::vec2& pos)
	{
		FontGlyph glyph{};
		float y = pos.y + m_FontAscent;

		if (c >= 32 && c < 128)
		{
			stbtt_aligned_quad quad;
			stbtt_GetBakedQuad((stbtt_bakedchar*)(m_Data), m_Width, m_Height, c - 32, &pos.x, &y, &quad, 1);

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
