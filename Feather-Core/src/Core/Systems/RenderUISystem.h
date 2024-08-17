#pragma once

#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Core/TextBatchRenderer.h"
#include "Renderer/Core/Camera2D.h"
#include "Core/ECS/Registry.h"

namespace Feather {
	
	class RenderUISystem
	{
	public:
		RenderUISystem();
		~RenderUISystem() = default;

		void Update(Registry& registry);

	private:
		std::unique_ptr<SpriteBatchRenderer> m_SpriteRenderer;
		std::unique_ptr<TextBatchRenderer> m_TextRenderer;
		std::unique_ptr<Camera2D> m_Camera2D;
	};

}
