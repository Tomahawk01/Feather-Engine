#pragma once

#include <sol/sol.hpp>

namespace Feather {

	class Registry;
	class Camera2D;
	class SpriteBatchRenderer;

	class RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem();

		void Update(Registry& registry, Camera2D& camera);
		static void CreateRenderSystemLuaBind(sol::state& lua, Registry& registry);

	private:
		std::unique_ptr<SpriteBatchRenderer> m_BatchRenderer;
	};

}
