#pragma once

#include <vector>
#include <memory>

#include "Renderer/Essentials/Vertex.h"

namespace Feather {

	class BatchRenderer
	{
	public:
		BatchRenderer();
		~BatchRenderer();

		void Begin();
		void End();
		void Render();

		void AddSprite(const glm::vec4& spriteRect, const glm::vec4 uvRect, GLuint textureID, int layer = 0,
					   glm::mat4 model = glm::mat4{ 1.0f }, const Color& color = Color{ .r = 255,.g = 255,.b = 255,.a = 255 });

	private:
		struct Batch
		{
			GLuint numIndices{ 0 }, offset{ 0 }, textureID{ 0 };
		};
		struct Sprite
		{
			Vertex topLeft, bottomLeft, topRight, bottomRight;
			int layer;
			GLuint textureID;
		};
	private:
		GLuint m_VAO, m_VBO, m_IBO;
		std::vector<std::shared_ptr<Sprite>> m_Sprites;
		std::vector<std::shared_ptr<Batch>> m_Batches;
	private:
		void Initialize();
		void GenerateBatches();
	};

}
