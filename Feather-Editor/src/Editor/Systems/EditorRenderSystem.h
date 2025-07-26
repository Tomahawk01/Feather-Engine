#pragma once

#include "Core/ECS/Registry.h"

namespace Feather {

	struct SpriteLayerParams;
	class Registry;
	class Camera2D;
	class SpriteBatchRenderer;

	class EditorRenderSystem
	{
	public:
		EditorRenderSystem();
		~EditorRenderSystem();

		/*
		 * @brief Loops through all of the entities in the registry that have a sprite
		 * and transform component. Applies all the necessary transformations and adds them to a batch to be rendered.
		 */
		void Update(Registry& registry, Camera2D& camera, const std::vector<SpriteLayerParams>& layerFilters = {});

	private:
		std::unique_ptr<SpriteBatchRenderer> m_BatchRenderer;
	};

}

using EditorRenderSystemPtr = std::shared_ptr<Feather::EditorRenderSystem>;
