#pragma once
#include "Core/ECS/Entity.h"
#include "Utils/HelperUtilities.h"
#include "../Utilities/EditorUtilities.h"

namespace Feather {

	class SceneObject
	{
	public:
		SceneObject(const std::string& sceneName);
		~SceneObject() = default;

		void CopySceneToRuntime();
		void ClearRuntimeScene();

		void AddNewLayer();
		inline std::vector<SpriteLayerParams>& GetLayerParams() { return m_LayerParams; }

		inline Canvas& GetCanvas() { return m_Canvas; }
		inline const std::string& GetSceneName() { return m_SceneName; }
		inline Registry& GetRegistry() { return m_Registry; }
		inline Registry* GetRegistryPtr() { return &m_Registry; }
		inline Registry& GetRuntimeRegistry() { return m_RuntimeRegistry; }

	private:
		Registry m_Registry, m_RuntimeRegistry;
		std::string m_SceneName;
		Canvas m_Canvas;
		std::vector<SpriteLayerParams> m_LayerParams;
	};

}
