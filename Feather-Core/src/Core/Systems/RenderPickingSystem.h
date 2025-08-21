#pragma once

namespace Feather {

	class Camera2D;
	class PickingBatchRenderer;
	class Registry;

	class RenderPickingSystem
	{
	public:
		RenderPickingSystem();
		~RenderPickingSystem();

		void Update(Registry& registry, Camera2D& camera);

	private:
		std::unique_ptr<PickingBatchRenderer> m_BatchRenderer;
	};

}
