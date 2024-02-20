#pragma once

#include "Core/ECS/Registry.h"
#include "Renderer/Core/RectBatchRenderer.h"
#include "Renderer/Core/CircleBatchRenderer.h"

namespace Feather {

	class RenderShapeSystem
	{
	public:
		RenderShapeSystem(Feather::Registry& registry);
		~RenderShapeSystem() = default;

		void Update();

	private:
		Feather::Registry& m_Registry;
		std::unique_ptr<Feather::RectBatchRenderer> m_RectRenderer;
		std::unique_ptr<Feather::CircleBatchRenderer> m_CircleRenderer;
	};

}
