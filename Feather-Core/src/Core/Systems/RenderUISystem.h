#pragma once

#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Core/TextBatchRenderer.h"
#include "Renderer/Core/Camera2D.h"
#include "Core/ECS/Registry.h"

namespace Feather {
	
	class RenderUISystem
	{
	public:
		RenderUISystem(Registry& registry);
		~RenderUISystem() = default;

		void Update(entt::registry& registry);

	private:
		Registry& m_Registry;
		std::unique_ptr<SpriteBatchRenderer> m_SpriteRenderer;
		std::unique_ptr<TextBatchRenderer> m_TextRenderer;
		std::unique_ptr<Camera2D> m_Camera2D;
	};

}
