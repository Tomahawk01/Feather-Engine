#include "GridSystem.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Renderer/Core/RectBatchRenderer.h"
#include "Renderer/Core/Camera2D.h"
#include "Renderer/Essentials/Primitives.h"
#include "Renderer/Essentials/Vertex.h"

#include "Editor/Scene/SceneObject.h"

namespace Feather {

	GridSystem::GridSystem()
		: m_BatchRenderer{ std::make_unique<RectBatchRenderer>() }
	{}

	void GridSystem::Update(SceneObject& currentScene, Camera2D& camera)
	{
		auto& assetManager = MAIN_REGISTRY().GetAssetManager();
		const auto& canvas = currentScene.GetCanvas();
		auto camMat = camera.GetCameraMatrix();
		auto colorShader = assetManager.GetShader("color");
		colorShader->Enable();
		colorShader->SetUniformMat4("uProjection", camMat);

		m_BatchRenderer->Begin();

		int tileWidth{ canvas.tileWidth }, tileHeight{ canvas.tileHeight };
		int canvasWidth{ canvas.width }, canvasHeight{ canvas.height };

		int cols = canvasWidth / tileWidth;
		int rows = canvasHeight / tileHeight;

		Color color{};

		for (int row = 0; row < rows; row++)
		{
			for (int col = 0; col < cols; col++)
			{
				// Create checkboard colors
				if ((col - row) % 2 == 0 && (row - col) % 2 == 0)
					color = { 125, 125, 125, 70 };
				else
					color = { 200, 200, 200, 70 };

				Rect rect{ 
					.position = glm::vec2{ static_cast<float>(col * tileWidth), static_cast<float>(row * tileHeight) },
					.width = static_cast<float>(tileWidth),
					.height = static_cast<float>(tileHeight),
					.color = color
				};
				m_BatchRenderer->AddRect(rect);
			}
		}

		m_BatchRenderer->End();
		m_BatchRenderer->Render();

		colorShader->Disable();
	}

}
