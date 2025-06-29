#include "RectBatchRenderer.h"
#include "../Essentials/Primitives.h"

namespace Feather {

	RectBatchRenderer::RectBatchRenderer()
		: Batcher(true)
	{
		Initialize();
	}

	void RectBatchRenderer::End()
	{
		if (m_Glyphs.empty())
			return;

		GenerateBatches();
	}

	void RectBatchRenderer::Render()
	{
		EnableVAO();

		for (const auto& batch : m_Batches)
		{
			glDrawElements(GL_TRIANGLES, batch->numIndices, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * batch->offset));
		}

		DisableVAO();
	}

	void RectBatchRenderer::AddRect(const glm::vec4& destRect, int layer, const Color& color, glm::mat4 model)
	{
		m_Glyphs.emplace_back(
			std::make_unique<RectGlyph>(
				RectGlyph{
					.topLeft = Vertex {
						.position = model * glm::vec4{ destRect.x, destRect.y + destRect.w, 0.0f, 1.0f },
						.color = color,
					},
					.bottomLeft = Vertex {
						.position = model * glm::vec4{ destRect.x, destRect.y, 0.0f, 1.0f },
						.color = color
					},
					.topRight = Vertex {
						.position = model * glm::vec4{ destRect.x + destRect.z, destRect.y + destRect.w, 0.0f, 1.0f },
						.color = color
					},
					.bottomRight = Vertex {
						.position = model * glm::vec4{ destRect.x + destRect.z, destRect.y, 0.0f, 1.0f },
						.color = color
					}
				}
			)
		);
	}

	void RectBatchRenderer::AddRect(const Rect& rect, glm::mat4 model)
	{
		m_Glyphs.emplace_back(
			std::make_unique<RectGlyph>(
				RectGlyph{
					.topLeft = Vertex {
						.position = model * glm::vec4{ rect.position.x, rect.position.y + rect.height, 0.0f, 1.0f },
						.color = rect.color,
					},
					.bottomLeft = Vertex {
						.position = model * glm::vec4{ rect.position.x, rect.position.y, 0.0f, 1.0f },
						.color = rect.color
					},
					.topRight = Vertex {
						.position = model * glm::vec4{ rect.position.x + rect.width, rect.position.y + rect.height, 0.0f, 1.0f },
						.color = rect.color
					},
					.bottomRight = Vertex {
						.position = model * glm::vec4{ rect.position.x + rect.width, rect.position.y, 0.0f, 1.0f },
						.color = rect.color
					}
				}
			)
		);
	}

	void RectBatchRenderer::AddIsoRect(const Rect& rect, glm::mat4 model)
	{
		m_Glyphs.emplace_back(
			std::make_unique<RectGlyph>(
				RectGlyph{
					.topLeft = Vertex {
						.position = model * glm::vec4{ rect.position.x, rect.position.y, 0.0f, 1.0f },
						.color = rect.color,
					},
					.bottomLeft = Vertex {
						.position = model * glm::vec4{ rect.position.x - rect.width / 2, rect.position.y + rect.height / 2, 0.0f, 1.0f },
						.color = rect.color
					},
					.topRight = Vertex {
						.position = model * glm::vec4{ rect.position.x + rect.width / 2, rect.position.y + rect.height / 2, 0.0f, 1.0f  },
						.color = rect.color
					},
					.bottomRight = Vertex {
						.position = model * glm::vec4{ rect.position.x, rect.position.y + rect.height, 0.0f, 1.0f },
						.color = rect.color
					}
				}
			)
		);
	}

	void RectBatchRenderer::GenerateBatches()
	{
		std::vector<Vertex> vertices;
		vertices.resize((m_Glyphs.size() > MAX_SPRITES ? MAX_SPRITES : m_Glyphs.size()) * 4);

		for (const auto& shape : m_Glyphs)
		{
			if (m_CurrentObject == 0)
			{
				m_Batches.push_back(std::make_unique<RectBatch>(RectBatch{ .numIndices = NUM_SPRITE_INDICES, .offset = 0 }));
			}
			else
			{
				m_Batches.back()->numIndices += NUM_SPRITE_INDICES;
			}

			vertices[m_CurrentVertex++] = shape->topLeft;
			vertices[m_CurrentVertex++] = shape->topRight;
			vertices[m_CurrentVertex++] = shape->bottomRight;
			vertices[m_CurrentVertex++] = shape->bottomLeft;

			m_CurrentObject++;
			m_Offset += NUM_SPRITE_INDICES;

			// If the number of objects are equal to max sprites, flush early
			if (m_CurrentObject == MAX_SPRITES)
			{
				Flush(vertices);
			}
		}

		// Buffer remaining data
		if (!vertices.empty() && !m_Batches.empty())
		{
			glBindBuffer(GL_ARRAY_BUFFER, GetVBO());
			// Orphan the buffer
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
			// Upload the data
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	void RectBatchRenderer::Initialize()
	{
		SetVertexAttribute(0, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
		SetVertexAttribute(1, 4, GL_UNSIGNED_BYTE, sizeof(Vertex), (void*)offsetof(Vertex, color), GL_TRUE);
	}

}
