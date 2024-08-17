#pragma once
#include "Core/ECS/Registry.h"
#include "Renderer/Core/BatchRenderer.h"

namespace Feather {

	class RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem() = default;

		void Update(Feather::Registry& registry);

	private:
		std::unique_ptr<Feather::SpriteBatchRenderer> m_BatchRenderer;
	};

}
