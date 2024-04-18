#include "RenderShapeSystem.h"

#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/CoreUtilities.h"

#include "Renderer/Core/Camera2D.h"
#include "Renderer/Essentials/Primitives.h"

namespace Feather {

	RenderShapeSystem::RenderShapeSystem(Registry& registry)
		: m_Registry{ registry }, m_RectRenderer{ std::make_unique<RectBatchRenderer>() }, m_CircleRenderer{ std::make_unique<CircleBatchRenderer>() }
	{}

	void RenderShapeSystem::Update()
	{
		if (!CoreEngineData::GetInstance().RenderCollidersEnabled())
			return;

		auto& camera = m_Registry.GetContext<std::shared_ptr<Camera2D>>();
		auto& assetManager = m_Registry.GetContext<std::shared_ptr<AssetManager>>();

		// Box
		auto colorShader = assetManager->GetShader("color");
		auto cam_mat = camera->GetCameraMatrix();

		colorShader->Enable();
		colorShader->SetUniformMat4("uProjection", cam_mat);
		m_RectRenderer->Begin();

		auto boxView = m_Registry.GetRegistry().view<TransformComponent, BoxColliderComponent>();
		for (auto entity : boxView)
		{
			const auto& transform = boxView.get<TransformComponent>(entity);
			const auto& boxCollider = boxView.get<BoxColliderComponent>(entity);

			if (!EntityInView(transform, static_cast<float>(boxCollider.width), static_cast<float>(boxCollider.height), *camera))
				continue;

			glm::mat4 model = TRSModel(transform, boxCollider.width, boxCollider.height);

			auto color = Color{ 255, 0, 0, 100 };

			if (m_Registry.GetRegistry().all_of<PhysicsComponent>(entity))
			{
				auto& physics = m_Registry.GetRegistry().get<PhysicsComponent>(entity);
				if (physics.IsTrigger())
					color = Color{ 0, 255, 0, 100 };
			}

			Rect rect{
				.position = glm::vec2{ transform.position.x + boxCollider.offset.x, transform.position.y + boxCollider.offset.y },
				.width = boxCollider.width * transform.scale.x,
				.height = boxCollider.height * transform.scale.y,
				.color = color
			};
			m_RectRenderer->AddRect(rect, model);
		}

		m_RectRenderer->End();
		m_RectRenderer->Render();
		colorShader->Disable();

		// Circle
		auto circleShader = assetManager->GetShader("circle");

		circleShader->Enable();
		circleShader->SetUniformMat4("uProjection", cam_mat);
		m_CircleRenderer->Begin();

		auto circleView = m_Registry.GetRegistry().view<TransformComponent, CircleColliderComponent>();
		for (auto entity : circleView)
		{
			const auto& transform = circleView.get<TransformComponent>(entity);
			const auto& circleCollider = circleView.get<CircleColliderComponent>(entity);

			glm::vec4 circle{
				transform.position.x + circleCollider.offset.x,
				transform.position.y + circleCollider.offset.y,
				circleCollider.radius * transform.scale.x * 2,
				circleCollider.radius * transform.scale.y * 2
			};
			m_CircleRenderer->AddCircle(circle, Color{ 0, 0, 255, 100 }, 1.0f);
		}

		m_CircleRenderer->End();
		m_CircleRenderer->Render();
		circleShader->Disable();
	}

}
