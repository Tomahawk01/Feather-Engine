#pragma once

#include "Core/ECS/Registry.h"

#include <sol/sol.hpp>

namespace Feather {
	
	class Camera2D;
	class SpriteBatchRenderer;
	class TextBatchRenderer;

	class RenderUISystem
	{
	public:
		RenderUISystem();
		~RenderUISystem();

		void Update(Registry& registry);

		static void CreateRenderUISystemLuaBind(sol::state& lua);

	private:
		std::unique_ptr<SpriteBatchRenderer> m_SpriteRenderer;
		std::unique_ptr<TextBatchRenderer> m_TextRenderer;
		std::unique_ptr<Camera2D> m_Camera2D;
	};

}
