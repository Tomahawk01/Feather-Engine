#include "RotateGizmo.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Entity.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Core/Camera2D.h"

#include "Editor/Utilities/EditorUtilities.h"

namespace Feather {

	RotateGizmo::RotateGizmo()
		: Gizmo(
			GizmoAxisParams{.axisColor = XAXIS_GIZMO_COLOR,
							.axisHoveredColor = XAXIS_HOVERED_GIZMO_COLOR,
							.axisDisabledColor = GRAYED_OUT_GIZMO_COLOR},
			GizmoAxisParams{}, true)
	{
		Init("rotate_tool", "");
	}

	void RotateGizmo::Update(Canvas& canvas)
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

		selectedTransform.rotation += GetDeltaX();

		if (selectedTransform.rotation < 0.0f)
			selectedTransform.rotation = 360.0f + selectedTransform.rotation;
		else if (selectedTransform.rotation > 360.0f)
			selectedTransform.rotation = selectedTransform.rotation - 360.0f;

		SetGizmoPosition(selectedEntity);

		ExamineMousePosition();
	}

	void RotateGizmo::Draw()
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

			const auto xAxisTexture = MAIN_REGISTRY().GetAssetManager().GetTexture(xAxisSprite.textureName);
			if (xAxisTexture)
				m_BatchRenderer->AddSprite(xAxisPosition, xAxisUVs, xAxisTexture->GetID(), 99, glm::mat4{ 1.0f }, xAxisSprite.color);
		}

		m_BatchRenderer->End();
		m_BatchRenderer->Render();

		shader->Disable();
	}

}
