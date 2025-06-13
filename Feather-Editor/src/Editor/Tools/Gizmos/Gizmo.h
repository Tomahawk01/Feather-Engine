#pragma once
#include "Editor/Tools/AbstractTool.h"

#include <entt.hpp>

#include <memory>

namespace Feather {

	class Entity;
	class SpriteBatchRenderer;
	class EventDispatcher;
	struct GizmoAxisParams;
	struct AddComponentEvent;

	class Gizmo : public AbstractTool
	{
	public:
		Gizmo();
		Gizmo(const GizmoAxisParams& xAxisParams, const GizmoAxisParams& yAxisParams, bool oneAxis);
		virtual ~Gizmo();

		virtual void Update(Canvas& canvas) override;
		virtual void Draw(Camera2D* camera) = 0;

		void SetSelectedEntity(entt::entity entity);
		void Hide();
		void Show();

		EventDispatcher& GetDispatcher();
		inline bool OverGizmo() const { return !(!m_HoldingX && !m_HoldingY && !m_OverXAxis && !m_OverYAxis); }

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
		std::unique_ptr<EventDispatcher> m_EventDispatcher{ nullptr };

		entt::entity m_SelectedEntity;
		glm::vec2 m_LastMousePos;

		bool m_OverXAxis;
		bool m_OverYAxis;
		bool m_HoldingX;
		bool m_HoldingY;
		bool m_Hidden;
		bool m_OnlyOneAxis;
		bool m_UIComponent;

	private:
		void OnAddComponent(const AddComponentEvent& addCompEvent);
	};

}
