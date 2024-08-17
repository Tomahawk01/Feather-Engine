#include "SceneManager.h"
#include "SceneObject.h"
#include "Utils/FeatherUtilities.h"
#include "Logger/Logger.h"

namespace Feather {

	SceneManager& SceneManager::GetInstance()
	{
		static SceneManager instance{};
		return instance;
	}

	bool SceneManager::AddScene(const std::string& sceneName)
	{
		if (m_mapScenes.contains(sceneName))
		{
			F_ERROR("Failed to add new scene object: '{}' already exists", sceneName);
			return false;
		}

		auto [itr, isSuccess] = m_mapScenes.emplace(sceneName, std::move(std::make_shared<SceneObject>(sceneName)));

		return isSuccess;
	}

	std::shared_ptr<SceneObject> SceneManager::GetScene(const std::string& sceneName)
	{
		auto sceneItr = m_mapScenes.find(sceneName);
		if (sceneItr == m_mapScenes.end())
		{
			F_ERROR("Failed to get scene object: '{}' does not exist", sceneName);
			return nullptr;
		}

		return sceneItr->second;
	}

	std::shared_ptr<SceneObject> SceneManager::GetCurrentScene()
	{
		if (m_CurrentScene.empty())
			return nullptr;

		auto sceneItr = m_mapScenes.find(m_CurrentScene);
		if (sceneItr == m_mapScenes.end())
		{
			F_ERROR("Failed to get scene object: '{}' does not exist", m_CurrentScene);
			return nullptr;
		}

		return sceneItr->second;
	}

	std::vector<std::string> SceneManager::GetSceneNames() const
	{
		return GetKeys(m_mapScenes);
	}

}
