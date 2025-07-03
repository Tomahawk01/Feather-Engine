#include "TextBatchRenderer.h"

#include "Logger/Logger.h"

/* If the loop is more than 100, fail and let the user know */
constexpr int MAX_LOOP_FAIL_CHECK = 100;
constexpr GLuint NUM_VERTICES = 6;
constexpr float MIN_TEXT_WRAP = 100.0f;

namespace Feather {

	TextBatchRenderer::TextBatchRenderer()
		: Batcher(false)
	{
		Initialize();
	}

	void TextBatchRenderer::End()
	{
		if (m_Glyphs.empty())
			return;

		GenerateBatches();
	}

	void TextBatchRenderer::Render()
	{
		if (m_Batches.empty())
			return;

		EnableVAO();
		for (const auto& batch : m_Batches)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, batch->fontAtlasID);
			glDrawArrays(GL_TRIANGLES, batch->offset, batch->numVertices);
		}
		DisableVAO();
	}

	void TextBatchRenderer::AddText(const std::string& text, const std::shared_ptr<Font>& font, const glm::vec2& position, int padding, float wrap, Color color, glm::mat4 model)
	{
		if (!font)
			return;

		m_Glyphs.emplace_back(
			std::make_unique<TextGlyph>(
				TextGlyph{
					.textStr = text,
					.position = position,
					.color = color,
					.model = model,
					.font = font,
					.wrap = wrap,
					.padding = padding
				}
			)
		);
	}

	void TextBatchRenderer::Initialize()
	{
		SetVertexAttribute(0, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
		SetVertexAttribute(1, 4, GL_UNSIGNED_BYTE, sizeof(Vertex), (void*)offsetof(Vertex, color), GL_TRUE);
		SetVertexAttribute(2, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, uvs));
	}

	void TextBatchRenderer::GenerateBatches()
	{
		GLuint offset{ 0 }, prevFontID{ 0 };
		int currentFont{ 0 };

		size_t total{ 0 }, currentVertex{ 0 };

		// Add up the total characters
		for (const auto& textGlpyh : m_Glyphs)
			total += textGlpyh->textStr.size();

		std::vector<Vertex> vertices;
		vertices.resize(total * NUM_VERTICES);

		for (const auto& textGlyph : m_Glyphs)
		{
			std::vector<std::string> textChunks{};
			std::string text_holder{};
			glm::vec2 temp_pos = textGlyph->position;
			auto fontSize = textGlyph->font->GetFontSize();
			int infiniteLoopCheck{ 0 };

			if (textGlyph->wrap > MIN_TEXT_WRAP)
			{
				// Create the text chunks for each line
				for (int i = 0; i < textGlyph->textStr.size(); i++)
				{
					if (infiniteLoopCheck >= MAX_LOOP_FAIL_CHECK)
					{
						F_ERROR("Failed to draw text batch correctly. Please check your text wrap, padding, textStr, etc");
						return;
					}

					auto character = textGlyph->textStr[i];
					text_holder += character;
					bool newLine = character == '\n';
					size_t text_size = text_holder.size();
					// Move temp_pos with each character
					textGlyph->font->GetNextCharPos(character, temp_pos);

					if (text_size > 0 && (temp_pos.x > (textGlyph->wrap + textGlyph->position.x) || character == '\0' || newLine))
					{
						if (!newLine)
						{
							while (textGlyph->textStr[i] != ' ' && textGlyph->textStr[i] != '.' && textGlyph->textStr[i] != '!' && textGlyph->textStr[i] != '?' && text_size > 0)
							{
								i--;
								infiniteLoopCheck++;

								if (i < 0)
								{
									F_ERROR("Failed to draw text '{0}': Wrap '{1}', is too small for the text to wrap successfully!", textGlyph->textStr, textGlyph->wrap);
									return;
								}

								if (!text_holder.empty())
								{
									text_holder.pop_back();
									text_size = text_holder.size();
									temp_pos.x -= fontSize;
								}
							}
						}
						else
						{
							text_holder.pop_back();
						}

						if (text_size > 0)
						{
							if (std::isalpha(text_holder[0]))
							{
								textChunks.push_back(text_holder);
								temp_pos = textGlyph->position;
								text_holder.clear();
								infiniteLoopCheck = 0;
							}
							else
							{
								text_holder.erase(0, 1);
								temp_pos.x -= fontSize;
							}
						}
					}
				}

				if (!text_holder.empty())
				{
					textChunks.push_back(text_holder);
					text_holder.clear();
				}
			}
			else
			{
				textChunks.push_back(textGlyph->textStr);
			}

			// Reset the text position
			temp_pos = textGlyph->position;

			// Add new Text Sprite
			for (const auto& textStr : textChunks)
			{
				for (const auto& character : textStr)
				{
					auto glyph = textGlyph->font->GetGlyph(character, temp_pos);

					// First Triangle
					vertices[currentVertex++] = Vertex{
						.position = textGlyph->model * glm::vec4{glyph.min.position.x, glyph.min.position.y, 0.0f, 1.0f},
						.uvs = glm::vec2{glyph.min.uvs.x, glyph.min.uvs.y},
						.color = textGlyph->color
					};

					vertices[currentVertex++] = Vertex{
						.position = textGlyph->model * glm::vec4{glyph.max.position.x, glyph.max.position.y, 0.0f, 1.0f},
						.uvs = glm::vec2{glyph.max.uvs.x, glyph.max.uvs.y},
						.color = textGlyph->color
					};

					vertices[currentVertex++] = Vertex{
						.position = textGlyph->model * glm::vec4{glyph.max.position.x, glyph.min.position.y, 0.0f, 1.0f},
						.uvs = glm::vec2{glyph.max.uvs.x, glyph.min.uvs.y},
						.color = textGlyph->color
					};

					// Second Triangle
					vertices[currentVertex++] = Vertex{
						.position = textGlyph->model * glm::vec4{glyph.min.position.x, glyph.min.position.y, 0.0f, 1.0f},
						.uvs = glm::vec2{glyph.min.uvs.x, glyph.min.uvs.y},
						.color = textGlyph->color
					};

					vertices[currentVertex++] = Vertex{
						.position = textGlyph->model * glm::vec4{glyph.min.position.x, glyph.max.position.y, 0.0f, 1.0f},
						.uvs = glm::vec2{glyph.min.uvs.x, glyph.max.uvs.y},
						.color = textGlyph->color
					};

					vertices[currentVertex++] = Vertex{
						.position = textGlyph->model * glm::vec4{glyph.max.position.x, glyph.max.position.y, 0.0f, 1.0f},
						.uvs = glm::vec2{glyph.max.uvs.x, glyph.max.uvs.y},
						.color = textGlyph->color
					};

					if (currentFont == 0)
					{
						m_Batches.push_back(
							std::make_unique<TextBatch>(
								TextBatch{
									.offset = offset,
									.numVertices = NUM_VERTICES,
									.fontAtlasID = textGlyph->font->GetFontAtlasID()
								}
							)
						);
					}
					else if (textGlyph->font->GetFontAtlasID() != prevFontID)
					{
						m_Batches.push_back(
							std::make_unique<TextBatch>(
								TextBatch{
									.offset = offset,
									.numVertices = NUM_VERTICES,
									.fontAtlasID = textGlyph->font->GetFontAtlasID()
								}
							)
						);
					}
					else
					{
						m_Batches.back()->numVertices += NUM_VERTICES;
					}

					currentFont++;
					prevFontID = textGlyph->font->GetFontAtlasID();
					offset += NUM_VERTICES;
				}

				// Move to the next Line
				temp_pos.x = textGlyph->position.x;
				temp_pos.y += textGlyph->font->GetFontSize() + textGlyph->padding;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, GetVBO());
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

}
