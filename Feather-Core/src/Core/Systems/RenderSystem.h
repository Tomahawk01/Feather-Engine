#pragma once
#include "Core/ECS/Registry.h"

namespace Feather {

	class Camera2D;
	class SpriteBatchRenderer;

	class RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem() = default;

		void Update(Feather::Registry& registry, Camera2D& camera);

	private:
		std::unique_ptr<Feather::SpriteBatchRenderer> m_BatchRenderer;
	};

}
