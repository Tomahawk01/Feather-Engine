#pragma once
#include <memory>

namespace Feather {

	class RectBatchRenderer;
	class Camera2D;

	class GridSystem
	{
	public:
		GridSystem();
		~GridSystem() = default;

		void Update(/*class SceneObject& currentScene, */ Camera2D& camera);

	private:
		std::unique_ptr<RectBatchRenderer> m_BatchRenderer;
	};

}
