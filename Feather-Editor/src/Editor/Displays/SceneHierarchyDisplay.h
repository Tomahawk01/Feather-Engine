#pragma once

#include "IDisplay.h"

#include <imgui.h>

namespace Feather {

	struct SwitchEntityEvent;
	struct KeyEvent;
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
		void DrawUneditableTypes();
		void DrawPlayerStart();
		void DrawEntityComponents();
		bool DeleteSelectedEntity();
		bool DuplicateSelectedEntity();

		void OnEntityChanged(SwitchEntityEvent& swEntEvent);
		void OnKeyPressed(KeyEvent& keyEvent);

		void OpenContext(class SceneObject* currentScene);

	private:
		std::shared_ptr<Entity> m_SelectedEntity{ nullptr };
		ImGuiTextFilter m_TextFilter;
		bool m_AddComponent{ false };
		bool m_WindowActive{ false };
	};

}
