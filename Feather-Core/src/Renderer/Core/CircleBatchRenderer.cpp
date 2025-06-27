#include "CircleBatchRenderer.h"
#include "../Essentials/Primitives.h"

namespace Feather {

	CircleBatchRenderer::CircleBatchRenderer()
		: Batcher(true)
	{
		Initialize();
	}

	void CircleBatchRenderer::End()
	{
		if (m_Glyphs.empty())
			return;

		GenerateBatches();
	}

	void CircleBatchRenderer::Render()
	{
		if (m_Batches.empty())
			return;

		EnableVAO();

		for (const auto& batch : m_Batches)
		{
			glDrawElements(GL_TRIANGLES, batch->numIndices, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * batch->offset));
		}

		DisableVAO();
	}

	void CircleBatchRenderer::AddCircle(const glm::vec4& destRect, const Color& color, float thickness, glm::mat4 model)
	{
		auto newCircle = std::make_shared<CircleGlyph>(
			CircleGlyph{
				.topLeft = CircleVertex {
					.position = model * glm::vec4{ destRect.x, destRect.y + destRect.w, 0.0f, 1.0f},
					.uvs = glm::vec2{1.0f, 1.0f},
					.color = color,
					.lineThickness = thickness
				},
				.bottomLeft = CircleVertex {
					.position = model * glm::vec4{ destRect.x, destRect.y, 0.0f, 1.0f},
					.uvs = glm::vec2{1.0f, -1.0f},
					.color = color,
					.lineThickness = thickness
				},
				.topRight = CircleVertex {
					.position = model * glm::vec4{ destRect.x + destRect.z, destRect.y + destRect.w, 0.0f, 1.0f},
					.uvs = glm::vec2{-1.0f, 1.0f},
					.color = color,
					.lineThickness = thickness
				},
				.bottomRight = CircleVertex {
					.position = model * glm::vec4{ destRect.x + destRect.z, destRect.y, 0.0f, 1.0f},
					.uvs = glm::vec2{-1.0f, -1.0f},
					.color = color,
					.lineThickness = thickness
				},
			}
		);
		m_Glyphs.emplace_back(std::move(newCircle));
	}

	void CircleBatchRenderer::AddCircle(const Circle& circle)
	{
		glm::mat4 model{ 1.0f };
		auto newCircle = std::make_shared<CircleGlyph>(
			CircleGlyph{
				.topLeft = CircleVertex {
					.position = model * glm::vec4{ circle.position.x, circle.position.y + circle.radius, 0.0f, 1.0f},
					.uvs = glm::vec2{1.0f, 1.0f},
					.color = circle.color,
					.lineThickness = circle.lineThickness
				},
				.bottomLeft = CircleVertex {
					.position = model * glm::vec4{ circle.position.x, circle.position.y, 0.0f, 1.0f},
					.uvs = glm::vec2{1.0f, -1.0f},
					.color = circle.color,
					.lineThickness = circle.lineThickness
				},
				.topRight = CircleVertex {
					.position = model * glm::vec4{ circle.position.x + circle.radius, circle.position.y + circle.radius, 0.0f, 1.0f},
					.uvs = glm::vec2{-1.0f, 1.0f},
					.color = circle.color,
					.lineThickness = circle.lineThickness
				},
				.bottomRight = CircleVertex {
					.position = model * glm::vec4{ circle.position.x + circle.radius, circle.position.y, 0.0f, 1.0f},
					.uvs = glm::vec2{-1.0f, -1.0f},
					.color = circle.color,
					.lineThickness = circle.lineThickness
				},
			}
		);

		m_Glyphs.emplace_back(std::move(newCircle));
	}

	void CircleBatchRenderer::GenerateBatches()
	{
		std::vector<CircleVertex> vertices;
		vertices.resize(m_Glyphs.size() * NUM_SPRITE_VERTICES);

		for (const auto& circle : m_Glyphs)
		{
			if (m_CurrentObject == 0)
				m_Batches.emplace_back(std::make_shared<RectBatch>(RectBatch{ .numIndices = NUM_SPRITE_INDICES, .offset = m_Offset }));
			else
				m_Batches.back()->numIndices += NUM_SPRITE_INDICES;

			vertices[m_CurrentVertex++] = circle->topLeft;
			vertices[m_CurrentVertex++] = circle->topRight;
			vertices[m_CurrentVertex++] = circle->bottomRight;
			vertices[m_CurrentVertex++] = circle->bottomLeft;

			m_Offset += NUM_SPRITE_INDICES;
			m_CurrentObject++;

			if (m_CurrentObject == MAX_SPRITES)
			{
				Flush(vertices);
			}
		}

		if (!vertices.empty() && !m_Batches.empty())
		{
			glBindBuffer(GL_ARRAY_BUFFER, GetVBO());
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CircleVertex), nullptr, GL_DYNAMIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(CircleVertex), vertices.data());
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	void CircleBatchRenderer::Initialize()
	{
		SetVertexAttribute(0, 2, GL_FLOAT, sizeof(CircleVertex), (void*)offsetof(CircleVertex, position));
		SetVertexAttribute(1, 2, GL_FLOAT, sizeof(CircleVertex), (void*)offsetof(CircleVertex, uvs));
		SetVertexAttribute(2, 4, GL_UNSIGNED_BYTE, sizeof(CircleVertex), (void*)offsetof(CircleVertex, color), GL_TRUE);
		SetVertexAttribute(3, 1, GL_FLOAT, sizeof(CircleVertex), (void*)offsetof(CircleVertex, lineThickness));
	}

}
