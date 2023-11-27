#pragma once

#include "Core/ECS/Registry.h"
#include "Renderer/Core/BatchRenderer.h"

namespace Feather {

	class RenderSystem
	{
	public:
		RenderSystem(Feather::Registry& registry);
		~RenderSystem() = default;

		void Update();

	private:
		Feather::Registry& m_Registry;
		std::unique_ptr<Feather::BatchRenderer> m_BatchRenderer;
	};

}
