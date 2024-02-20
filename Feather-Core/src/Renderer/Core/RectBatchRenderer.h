#pragma once

#include "Batcher.h"
#include "../Essentials/BatchTypes.h"

namespace Feather {

	class RectBatchRenderer : public Batcher<RectBatch, RectGlyph>
	{
	public:
		RectBatchRenderer();
		~RectBatchRenderer() = default;

		virtual void End() override;
		virtual void Render() override;

		void AddRect(const glm::vec4& destRect, int layer, const Color& color, glm::mat4 model = glm::mat4{ 1.0f });
		void AddRect(const struct Rect& rect, glm::mat4 model = glm::mat4{ 1.0f });

	private:
		virtual void GenerateBatches() override;
		void Initialize();
	};

}
