#pragma once
#include <memory>

namespace Feather {

	class RectBatchRenderer;
	class Camera2D;
	class Scene;

	class GridSystem
	{
	public:
		GridSystem();
		~GridSystem() = default;

		void Update(Scene& currentScene, Camera2D& camera);

	private:
		void UpdateIso(Scene& currentScene, Camera2D& camera);

	private:
		std::unique_ptr<RectBatchRenderer> m_BatchRenderer;
	};

}
