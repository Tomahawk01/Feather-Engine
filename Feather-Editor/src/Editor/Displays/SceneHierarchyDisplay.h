#pragma once
#include "IDisplay.h"

#include <imgui.h>

namespace Feather {

	struct SwitchEntityEvent;
	class Entity;

	class SceneHierarchyDisplay : public IDisplay
	{
	public:
		SceneHierarchyDisplay();
		~SceneHierarchyDisplay();

		virtual void Update() override;
		virtual void Draw() override;

	private:
		bool OpenTreeNode(Entity& entity);
		void AddComponent(Entity& entity, bool* addComponent);
		void DrawGameObjectDetails();
		void DrawEntityComponents();

		void OnEntityChanged(SwitchEntityEvent& swEntEvent);

	private:
		std::shared_ptr<Entity> m_SelectedEntity{ nullptr };
		ImGuiTextFilter m_TextFilter;
		bool m_AddComponent{ false };
	};

}
