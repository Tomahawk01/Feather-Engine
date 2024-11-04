#pragma once
#include <memory>

namespace Feather {

	class Camera2D;
	class RectBatchRenderer;
	class CircleBatchRenderer;
	class Registry;

	class RenderShapeSystem
	{
	public:
		RenderShapeSystem();
		~RenderShapeSystem() = default;

		void Update(Registry& registry, Camera2D& camera);

	private:
		std::unique_ptr<Feather::RectBatchRenderer> m_RectRenderer;
		std::unique_ptr<Feather::CircleBatchRenderer> m_CircleRenderer;
	};

}
