#include "SceneObject.h"

#include <format>

namespace Feather {

	SceneObject::SceneObject(const std::string& sceneName)
		: m_Registry{}, m_RuntimeRegistry{}, m_SceneName{ sceneName }, m_Canvas{}
	{}

	void SceneObject::CopySceneToRuntime()
	{
	}

	void SceneObject::ClearRuntimeScene()
	{
	}

	void SceneObject::AddNewLayer()
	{
		static int number{ 0 };
		m_LayerParams.emplace_back(SpriteLayerParams{
			.layerName = std::format("NewLayer_{}", number++)
		});
	}

}