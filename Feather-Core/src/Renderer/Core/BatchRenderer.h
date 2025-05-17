#pragma once

#include "Batcher.h"
#include "../Essentials/BatchTypes.h"

namespace Feather {

	class SpriteBatchRenderer : public Batcher<Batch, SpriteGlyph>
	{
	public:
		SpriteBatchRenderer();
		virtual ~SpriteBatchRenderer() = default;

		virtual void End() override;
		virtual void Render() override;

		void AddSprite(const glm::vec4& spriteRect, const glm::vec4 uvRect, GLuint textureID, int layer = 0,
					   glm::mat4 model = glm::mat4{ 1.0f }, const Color& color = Color{ .r = 255, .g = 255, .b = 255, .a = 255 });

		void AddSpriteIso(const glm::vec4& spriteRect, const glm::vec4 uvRect, GLuint textureID, int cellX, int cellY, int layer = 0,
						  glm::mat4 model = glm::mat4{ 1.0f }, const Color& color = Color{ .r = 255, .g = 255, .b = 255, .a = 255 });

	private:
		void Initialize();
		virtual void GenerateBatches() override;
	};

}
