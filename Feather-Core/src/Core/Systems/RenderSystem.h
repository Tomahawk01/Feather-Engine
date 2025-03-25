#pragma once

#include "Core/ECS/Registry.h"

namespace Feather {

	struct SpriteLayerParams;
	class Camera2D;
	class SpriteBatchRenderer;

	class RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem() = default;

		void Update(Feather::Registry& registry, Camera2D& camera, const std::vector<SpriteLayerParams>& layerFilters = {});

		static void CreateRenderSystemLuaBind(sol::state& lua, Registry& registry);

	private:
		std::unique_ptr<Feather::SpriteBatchRenderer> m_BatchRenderer;
	};

}
