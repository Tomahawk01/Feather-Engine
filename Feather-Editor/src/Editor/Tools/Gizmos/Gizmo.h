#pragma once
#include "Editor/Tools/AbstractTool.h"

#include <entt.hpp>

#include <memory>

namespace Feather {

	class Entity;
	class SpriteBatchRenderer;
	struct GizmoAxisParams;

	class Gizmo : public AbstractTool
	{
	public:
		Gizmo();
		Gizmo(const GizmoAxisParams& xAxisParams, const GizmoAxisParams& yAxisParams, bool oneAxis);
		virtual ~Gizmo();

		virtual void Update(Canvas& canvas) override;
		virtual void Draw() = 0;

		void SetSelectedEntity(entt::entity entity);
		void Hide();
		void Show();

		inline bool OverGizmo() const { return false; }

	protected:
		void Init(const std::string& xAxisTexture, const std::string& yAxisTexture);
		virtual void ExamineMousePosition() override;
		
		float GetDeltaX();
		float GetDeltaY();

		void SetGizmoPosition(Entity& selectedEntity);
		virtual void ResetSelectedEntity() override { m_SelectedEntity = entt::null; }

	protected:
		std::unique_ptr<GizmoAxisParams> m_XAxisParams;
		std::unique_ptr<GizmoAxisParams> m_YAxisParams;
		std::unique_ptr<SpriteBatchRenderer> m_BatchRenderer;

		entt::entity m_SelectedEntity;
		glm::vec2 m_LastMousePos;

		bool m_OverXAxis;
		bool m_OverYAxis;
		bool m_HoldingX;
		bool m_HoldingY;
		bool m_Hidden;
		bool m_OnlyOneAxis;
	};

}
