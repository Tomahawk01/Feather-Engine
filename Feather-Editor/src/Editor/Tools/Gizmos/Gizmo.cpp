#include "Gizmo.h"

#include "Logger/Logger.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Renderer/Core/BatchRenderer.h"
#include "Renderer/Essentials/Vertex.h"

#include "Editor/Utilities/EditorUtilities.h"

namespace Feather {

	Gizmo::Gizmo()
		: Gizmo(GizmoAxisParams{ .transform = TransformComponent{.scale = glm::vec2{ 1.0f, 0.5f }},
								 .axisColor = XAXIS_GIZMO_COLOR,
								 .axisHoveredColor = XAXIS_HOVERED_GIZMO_COLOR,
								 .axisDisabledColor = GRAYED_OUT_GIZMO_COLOR },
				GizmoAxisParams{ .transform = TransformComponent{.scale = glm::vec2{ 0.5f, 1.0f }},
								 .axisOffset = glm::vec2{ -4 },
								 .axisColor = YAXIS_GIZMO_COLOR,
								 .axisHoveredColor = YAXIS_HOVERED_GIZMO_COLOR,
								 .axisDisabledColor = GRAYED_OUT_GIZMO_COLOR },
				false)
	{}

	Gizmo::Gizmo(const GizmoAxisParams& xAxisParams, const GizmoAxisParams& yAxisParams, bool oneAxis)
		: AbstractTool()
		, m_XAxisParams{ nullptr }
		, m_YAxisParams{ nullptr }
		, m_BatchRenderer{ std::make_unique<SpriteBatchRenderer>() }
		, m_SelectedEntity{ entt::null }
		, m_LastMousePos{ 0.0f }
		, m_OverXAxis{ false }
		, m_OverYAxis{ false }
		, m_HoldingX{ false }
		, m_HoldingY{ false }
		, m_Hidden{ false }
		, m_OnlyOneAxis{ oneAxis }
	{
		m_XAxisParams = std::make_unique<GizmoAxisParams>(xAxisParams);

		if (m_OnlyOneAxis)
			return;

		m_YAxisParams = std::make_unique<GizmoAxisParams>(yAxisParams);
	}

	void Gizmo::Update(Canvas& canvas)
	{
		m_LastMousePos = GetMouseScreenCoords();
		AbstractTool::Update(canvas);
	}

	void Gizmo::SetSelectedEntity(entt::entity entity)
	{
		m_SelectedEntity = entity;
		if (m_SelectedEntity != entt::null && m_Registry)
		{
			Entity ent{ *m_Registry, entity };
			SetGizmoPosition(ent);
		}
	}

	void Gizmo::Hide()
	{
		if (m_XAxisParams)
			m_XAxisParams->sprite.isHidden = true;
		if (m_YAxisParams)
			m_YAxisParams->sprite.isHidden = true;

		m_Hidden = true;
	}

	void Gizmo::Show()
	{
		if (m_Hidden && m_SelectedEntity != entt::null)
		{
			if (m_XAxisParams)
				m_XAxisParams->sprite.isHidden = false;
			if (m_YAxisParams)
				m_YAxisParams->sprite.isHidden = false;

			m_Hidden = false;
		}
	}

	void Gizmo::Init(const std::string& xAxisTexture, const std::string& yAxisTexture)
	{
		// Setup x-axis
		m_XAxisParams->sprite.texture_name = xAxisTexture;
		auto xAxisTex = MAIN_REGISTRY().GetAssetManager().GetTexture(xAxisTexture);
		F_ASSERT(xAxisTex && "Texture must exist!");
		m_XAxisParams->sprite.width = xAxisTex->GetWidth();
		m_XAxisParams->sprite.height = xAxisTex->GetHeight();
		m_XAxisParams->sprite.color = m_XAxisParams->axisColor;
		GenerateUVs(m_XAxisParams->sprite, xAxisTex->GetWidth(), xAxisTex->GetHeight());

		// Setup y-axis
		if (!m_OnlyOneAxis)
		{
			m_YAxisParams->sprite.texture_name = yAxisTexture;
			auto yAxisTex = MAIN_REGISTRY().GetAssetManager().GetTexture(yAxisTexture);
			F_ASSERT(yAxisTex && "Texture must exist!");
			m_YAxisParams->sprite.width = yAxisTex->GetWidth();
			m_YAxisParams->sprite.height = yAxisTex->GetHeight();
			m_YAxisParams->sprite.color = m_YAxisParams->axisColor;
			GenerateUVs(m_YAxisParams->sprite, yAxisTex->GetWidth(), yAxisTex->GetHeight());
		}

		Hide();
	}

	void Gizmo::ExamineMousePosition()
	{
		const auto& mousePos = GetMouseWorldCoords();

		const auto& xAxisTransform = m_XAxisParams->transform;
		auto& xAxisSprite = m_XAxisParams->sprite;

		if (mousePos.x >= xAxisTransform.position.x &&
			mousePos.x <= (xAxisTransform.position.x + (xAxisSprite.width * xAxisTransform.scale.x)) &&
			mousePos.y >= xAxisTransform.position.y &&
			mousePos.y <= (xAxisTransform.position.y + (xAxisSprite.height * xAxisTransform.scale.y)))
		{
			xAxisSprite.color = m_XAxisParams->axisHoveredColor;
			m_OverXAxis = true;
		}
		else if (m_HoldingX)
		{
			xAxisSprite.color = m_XAxisParams->axisHoveredColor;
			m_OverXAxis = true;
		}
		else if (m_HoldingY)
		{
			xAxisSprite.color = m_XAxisParams->axisDisabledColor;
			m_OverXAxis = false;
		}
		else
		{
			xAxisSprite.color = m_XAxisParams->axisColor;
			m_OverXAxis = false;
		}

		if (m_OnlyOneAxis)
			return;

		const auto& yAxisTransform = m_YAxisParams->transform;
		auto& yAxisSprite = m_YAxisParams->sprite;

		if (mousePos.x >= yAxisTransform.position.x &&
			mousePos.x <= (yAxisTransform.position.x + (yAxisSprite.width * yAxisTransform.scale.x)) &&
			mousePos.y >= yAxisTransform.position.y &&
			mousePos.y <= (yAxisTransform.position.y + (yAxisSprite.height * yAxisTransform.scale.y)))
		{
			yAxisSprite.color = m_YAxisParams->axisHoveredColor;
			m_OverYAxis = true;
		}
		else if (m_HoldingY)
		{
			yAxisSprite.color = m_YAxisParams->axisHoveredColor;
			m_OverYAxis = true;
		}
		else if (m_HoldingX)
		{
			yAxisSprite.color = m_YAxisParams->axisDisabledColor;
			m_OverYAxis = false;
		}
		else
		{
			yAxisSprite.color = m_YAxisParams->axisColor;
			m_OverYAxis = false;
		}
	}

	float Gizmo::GetDeltaX()
	{
		if (!IsOverTilemapWindow() || OutOfBounds())
			return 0.0f;
		if (!m_OverXAxis && !m_HoldingX)
			return 0.0f;

		if (MouseButtonPressed(AbstractTool::MouseButton::LEFT) && MouseMoving())
		{
			m_HoldingX = true;
			return std::ceil((GetMouseScreenCoords().x - m_LastMousePos.x) / m_Camera->GetScale());
		}

		if (MouseButtonJustReleased(AbstractTool::MouseButton::LEFT))
		{
			m_HoldingX = false;
		}

		return 0.0f;
	}

	float Gizmo::GetDeltaY()
	{
		if (!IsOverTilemapWindow() || OutOfBounds() || m_OnlyOneAxis)
			return 0.0f;
		if (!m_OverYAxis && !m_HoldingY)
			return 0.0f;

		if (MouseButtonPressed(AbstractTool::MouseButton::LEFT) && MouseMoving())
		{
			m_HoldingY = true;
			return std::ceil((GetMouseScreenCoords().y - m_LastMousePos.y) / m_Camera->GetScale());
		}

		if (MouseButtonJustReleased(AbstractTool::MouseButton::LEFT))
		{
			m_HoldingY = false;
		}

		return 0.0f;
	}

	void Gizmo::SetGizmoPosition(Entity& selectedEntity)
	{
		float spriteWidth{ 0.0f };
		float spriteHeight{ 0.0f };

		if (const auto* sprite = selectedEntity.TryGetComponent<SpriteComponent>())
		{
			spriteWidth = sprite->width;
			spriteHeight = sprite->height;
		}

		const auto& selectedTransform = selectedEntity.GetComponent<TransformComponent>();
		if (!m_OnlyOneAxis)
		{
			m_XAxisParams->transform.position =
				selectedTransform.position + m_XAxisParams->axisOffset +
				glm::vec2{
					spriteWidth * selectedTransform.scale.x * 0.5f,
					(spriteHeight * selectedTransform.scale.y * 0.5f) - (m_XAxisParams->sprite.height * m_XAxisParams->transform.scale.y)
				};

			m_YAxisParams->transform.position =
				selectedTransform.position + m_YAxisParams->axisOffset +
				glm::vec2{
					(spriteWidth * selectedTransform.scale.x * 0.5f) - (m_YAxisParams->sprite.width * m_YAxisParams->transform.scale.x),
					(spriteHeight * selectedTransform.scale.y * 0.5f) - (m_YAxisParams->sprite.height * m_YAxisParams->transform.scale.y)
				};
		}
		else
		{
			m_XAxisParams->transform.position =
				selectedTransform.position + m_XAxisParams->axisOffset +
				glm::vec2{
					(spriteWidth * selectedTransform.scale.x * 0.5f) - (m_XAxisParams->sprite.width * m_XAxisParams->transform.scale.x),
					(spriteHeight * selectedTransform.scale.y * 0.5f) - (m_XAxisParams->sprite.height * m_XAxisParams->transform.scale.y)
				};
		}
	}

}
