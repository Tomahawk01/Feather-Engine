#include "SceneObject.h"
#include "Utils/FeatherUtilities.h"

#include <format>

namespace Feather {

	SceneObject::SceneObject(const std::string& sceneName)
		: m_Registry{}, m_RuntimeRegistry{}, m_SceneName{ sceneName }, m_Canvas{}, m_CurrentLayer{ 0 }
	{}

	void SceneObject::CopySceneToRuntime()
	{
	}

	void SceneObject::ClearRuntimeScene()
	{
	}

	void SceneObject::AddNewLayer()
	{
		m_LayerParams.emplace_back(SpriteLayerParams{ .layerName = std::format("NewLayer_{}", m_CurrentLayer++) });
	}

	bool SceneObject::CheckLayerName(const std::string& layerName)
	{
		return CheckContainsValue(m_LayerParams,
			[&](SpriteLayerParams& spriteLayer) { return spriteLayer.layerName == layerName; }
		);
	}

}