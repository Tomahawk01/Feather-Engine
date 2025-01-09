#include "ScaleGizmo.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Entity.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Core/Camera2D.h"

#include "Editor/Utilities/EditorUtilities.h"

// TODO: Add scaling factor for all gizmos
constexpr float SCALING_FACTOR = 0.01f;

namespace Feather {

	ScaleGizmo::ScaleGizmo()
		: Gizmo()
	{
		Init("x_axis_scale", "y_axis_scale");
	}

	void ScaleGizmo::Update(Canvas& canvas)
	{
		Gizmo::Update(canvas);

		if (m_SelectedEntity == entt::null || !m_Registry)
		{
			Hide();
			return;
		}

		Show();

		Entity selectedEntity{ *m_Registry, m_SelectedEntity };
		auto& selectedTransform = selectedEntity.GetComponent<TransformComponent>();

		selectedTransform.scale.x += GetDeltaX() * SCALING_FACTOR;
		selectedTransform.scale.y += GetDeltaY() * SCALING_FACTOR;

		SetGizmoPosition(selectedEntity);

		ExamineMousePosition();
	}

	void ScaleGizmo::Draw()
	{
		if (m_Hidden)
			return;

		auto shader = MAIN_REGISTRY().GetAssetManager().GetShader("basic");
		if (!shader)
			return;

		shader->Enable();
		auto camMat = m_Camera->GetCameraMatrix();
		shader->SetUniformMat4("uProjection", camMat);

		m_BatchRenderer->Begin();

		const auto& xAxisSprite = m_XAxisParams->sprite;
		const auto& xAxisTransform = m_XAxisParams->transform;
		if (!xAxisSprite.isHidden)
		{
			glm::vec4 xAxisPosition{ xAxisTransform.position.x, xAxisTransform.position.y,
									 xAxisSprite.width * xAxisTransform.scale.x, xAxisSprite.height * xAxisTransform.scale.y };

			glm::vec4 xAxisUVs{ xAxisSprite.uvs.u, xAxisSprite.uvs.v,
								xAxisSprite.uvs.uv_width, xAxisSprite.uvs.uv_height };

			const auto xAxisTexture = MAIN_REGISTRY().GetAssetManager().GetTexture(xAxisSprite.texture_name);
			if (xAxisTexture)
				m_BatchRenderer->AddSprite(xAxisPosition, xAxisUVs, xAxisTexture->GetID(), 99, glm::mat4{ 1.0f }, xAxisSprite.color);
		}

		const auto& yAxisSprite = m_YAxisParams->sprite;
		const auto& yAxisTransform = m_YAxisParams->transform;
		if (!yAxisSprite.isHidden)
		{
			glm::vec4 yAxisPosition{ yAxisTransform.position.x, yAxisTransform.position.y,
									 yAxisSprite.width * yAxisTransform.scale.x, yAxisSprite.height * yAxisTransform.scale.y };

			glm::vec4 yAxisUVs{ yAxisSprite.uvs.u, yAxisSprite.uvs.v,
								yAxisSprite.uvs.uv_width, yAxisSprite.uvs.uv_height };

			const auto yAxisTexture = MAIN_REGISTRY().GetAssetManager().GetTexture(yAxisSprite.texture_name);
			if (yAxisTexture)
				m_BatchRenderer->AddSprite(yAxisPosition, yAxisUVs, yAxisTexture->GetID(), 99, glm::mat4{ 1.0f }, yAxisSprite.color);
		}

		m_BatchRenderer->End();
		m_BatchRenderer->Render();

		shader->Disable();
	}

}
