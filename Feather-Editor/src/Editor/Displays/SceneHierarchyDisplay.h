#pragma once
#include "IDisplay.h"

namespace Feather {

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

	private:
		std::shared_ptr<Entity> m_SelectedEntity{ nullptr };
		bool m_AddComponent{ false };
	};

}
