#include "RenderShapeSystem.h"

#include "Core/ECS/Components/BoxColliderComponent.h"
#include "Core/ECS/Components/CircleColliderComponent.h"
#include "Core/ECS/Components/TransformComponent.h"
#include "Core/Resources/AssetManager.h"

#include "Renderer/Core/Camera2D.h"
#include "Renderer/Essentials/Primitives.h"

namespace Feather {

	RenderShapeSystem::RenderShapeSystem(Registry& registry)
		: m_Registry{ registry }, m_RectRenderer{ std::make_unique<RectBatchRenderer>() }, m_CircleRenderer{ std::make_unique<CircleBatchRenderer>() }
	{}

	void RenderShapeSystem::Update()
	{
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

			glm::mat4 model{ 1.0f };
			if (transform.rotation > 0.0f || transform.rotation < 0.0f ||
				transform.scale.x > 1.0f || transform.scale.x < 1.0f ||
				transform.scale.y > 1.0f || transform.scale.y < 1.0f)
			{
				model = glm::translate(model, glm::vec3{ transform.position, 0.0f });
				model = glm::translate(model, glm::vec3{ (boxCollider.width * transform.scale.x) * 0.5f, (boxCollider.height * transform.scale.y) * 0.5f, 0.0f });

				model = glm::rotate(model, glm::radians(transform.rotation), glm::vec3{ 0.0f, 0.0f, 1.0f });
				model = glm::translate(model, glm::vec3{ (boxCollider.width * transform.scale.x) * -0.5f, (boxCollider.height * transform.scale.y) * -0.5f, 0.0f });

				model = glm::scale(model, glm::vec3{ transform.scale, 1.0f });

				model = glm::translate(model, glm::vec3{ -transform.position, 0.0f });
			}

			Rect rect{
				.position = glm::vec2{transform.position.x + boxCollider.offset.x, transform.position.y + boxCollider.offset.y},
				.width = boxCollider.width * transform.scale.x,
				.height = boxCollider.height * transform.scale.y,
				.color = Color{ 255, 0, 0, 100 }
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
			m_CircleRenderer->AddCircle(circle, Color{ 255, 0, 0, 100 }, 1.0f);
		}

		m_CircleRenderer->End();
		m_CircleRenderer->Render();
		circleShader->Disable();
	}

}
