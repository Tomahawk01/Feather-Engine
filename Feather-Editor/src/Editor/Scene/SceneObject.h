#pragma once
#include "Core/ECS/Entity.h"
#include "../Utilities/EditorUtilities.h"

namespace Feather {

	class SceneObject
	{
	public:
		SceneObject(const std::string& sceneName);
		~SceneObject() = default;

		void CopySceneToRuntime();
		void ClearRuntimeScene();

		inline Canvas& GetCanvas() { return m_Canvas; }
		inline const std::string& GetSceneName() { return m_SceneName; }
		inline Registry& GetRegistry() { return m_Registry; }
		inline Registry& GetRuntimeRegistry() { return m_RuntimeRegistry; }

	private:
		Registry m_Registry, m_RuntimeRegistry;
		std::string m_SceneName;
		Canvas m_Canvas;
	};

}
